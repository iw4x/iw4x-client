#include <STDInclude.hpp>

#include "Chat.hpp"
#include "Events.hpp"
#include "Voice.hpp"

namespace Components
{
	Game::VoicePacket_t Voice::VoicePackets[Game::MAX_CLIENTS][MAX_SERVER_QUEUED_VOICE_PACKETS];
	int Voice::VoicePacketCount[Game::MAX_CLIENTS];

	bool Voice::MuteList[Game::MAX_CLIENTS];
	bool Voice::S_PlayerMute[Game::MAX_CLIENTS];

	const Game::dvar_t* Voice::sv_voice;

	bool Voice::SV_VoiceEnabled()
	{
		return sv_voice->current.enabled;
	}

	void Voice::SV_WriteVoiceDataToClient(const int clientNum, Game::msg_t* msg)
	{
		assert(VoicePacketCount[clientNum] >= 0);
		assert(VoicePacketCount[clientNum] <= MAX_SERVER_QUEUED_VOICE_PACKETS);

		Game::MSG_WriteByte(msg, VoicePacketCount[clientNum]);
		for (auto packet = 0; packet < VoicePacketCount[clientNum]; ++packet)
		{
			Game::MSG_WriteByte(msg, VoicePackets[clientNum][packet].talker);

			assert(VoicePackets[clientNum][packet].dataSize < (2 << 15));

			Game::MSG_WriteByte(msg, VoicePackets[clientNum][packet].dataSize);
			Game::MSG_WriteData(msg, VoicePackets[clientNum][packet].data, VoicePackets[clientNum][packet].dataSize);
		}

		assert(!msg->overflowed);
	}

	void Voice::SV_SendClientVoiceData(Game::client_s* client)
	{
		Game::msg_t msg{};
		const auto clientNum = client - Game::svs_clients;

		const auto msg_buf_large = std::make_unique<unsigned char[]>(0x20000);
		auto* msg_buf = msg_buf_large.get();

		assert(VoicePacketCount[clientNum] >= 0);

		if (client->header.state == Game::CS_ACTIVE && VoicePacketCount[clientNum])
		{
			Game::MSG_Init(&msg, msg_buf, 0x20000);

			assert(msg.cursize == 0);
			assert(msg.bit == 0);

			Game::MSG_WriteString(&msg, "v");
			SV_WriteVoiceDataToClient(clientNum, &msg);

			if (msg.overflowed)
			{
				Logger::Warning(Game::CON_CHANNEL_SERVER, "WARNING: voice msg overflowed for {}\n", client->name);
			}
			else
			{
				Game::NET_OutOfBandVoiceData(Game::NS_SERVER, client->header.netchan.remoteAddress, msg.data, msg.cursize, true);
				VoicePacketCount[clientNum] = 0;
			}
		}
	}

	void Voice::SV_SendClientMessages_Stub(Game::client_s* client, Game::msg_t* msg, unsigned char* snapshotMsgBuf)
	{
		// SV_EndClientSnapshot
		Utils::Hook::Call<void(Game::client_s*, Game::msg_t*, unsigned char*)>(0x4F5300)(client, msg, snapshotMsgBuf);

		SV_SendClientVoiceData(client);
	}

	void Voice::SV_ClearMutedList()
	{
		std::memset(MuteList, 0, sizeof(MuteList));
	}

	void Voice::SV_MuteClient(const int muteClientIndex)
	{
		AssertIn(muteClientIndex, Game::MAX_CLIENTS);
		MuteList[muteClientIndex] = true;
	}

	void Voice::SV_UnmuteClient(const int muteClientIndex)
	{
		AssertIn(muteClientIndex, Game::MAX_CLIENTS);
		MuteList[muteClientIndex] = false;
	}

	bool Voice::SV_ServerHasClientMuted(const int talker)
	{
		AssertIn(talker, (*Game::sv_maxclients)->current.integer);
		return MuteList[talker];
	}

	bool Voice::OnSameTeam(const Game::gentity_s* ent1, const Game::gentity_s* ent2)
	{
		if (!ent1->client || !ent2->client)
		{
			return false;
		}

		if (ent1->client->sess.cs.team)
		{
			return ent1->client->sess.cs.team == ent2->client->sess.cs.team;
		}

		return false;
	}

	void Voice::SV_QueueVoicePacket(const int talkerNum, const int clientNum, const Game::VoicePacket_t* voicePacket)
	{
		assert(talkerNum >= 0);
		assert(clientNum >= 0);
		assert(talkerNum < (*Game::sv_maxclients)->current.integer);
		assert(clientNum < (*Game::sv_maxclients)->current.integer);

		if (VoicePacketCount[clientNum] < MAX_SERVER_QUEUED_VOICE_PACKETS)
		{
			VoicePackets[clientNum][VoicePacketCount[clientNum]].dataSize = voicePacket->dataSize;
			std::memcpy(VoicePackets[clientNum][VoicePacketCount[clientNum]].data, voicePacket->data, voicePacket->dataSize);

			assert(talkerNum == static_cast<std::uint8_t>(talkerNum));
			VoicePackets[clientNum][VoicePacketCount[clientNum]].talker = static_cast<char>(talkerNum);
			++VoicePacketCount[clientNum];
		}
	}

	void Voice::G_BroadcastVoice(Game::gentity_s* talker, const Game::VoicePacket_t* voicePacket)
	{
		for (auto otherPlayer = 0; otherPlayer < (*Game::sv_maxclients)->current.integer; ++otherPlayer)
		{
			auto* ent = &Game::g_entities[otherPlayer];
			auto* client = ent->client;

			if (ent->r.isInUse && client && (client->sess.sessionState == Game::SESS_STATE_INTERMISSION || OnSameTeam(talker, ent) || talker->client->sess.cs.team == Game::TEAM_FREE) &&
				(ent->client->sess.sessionState == talker->client->sess.sessionState || (ent->client->sess.sessionState == Game::SESS_STATE_DEAD || talker->client->sess.sessionState == Game::SESS_STATE_DEAD) &&
					(*Game::g_deadChat)->current.enabled) && (talker != ent) && !SV_ServerHasClientMuted(talker->s.number))
			{
				SV_QueueVoicePacket(talker->s.number, otherPlayer, voicePacket);
			}
		}
	}

	void Voice::SV_UserVoice(Game::client_s* cl, Game::msg_t* msg)
	{
		Game::VoicePacket_t voicePacket{};

		if (!SV_VoiceEnabled())
		{
			return;
		}

		const auto packetCount = Game::MSG_ReadByte(msg);

		assert(cl->gentity);

		for (auto packet = 0; packet < packetCount; ++packet)
		{
			voicePacket.dataSize = Game::MSG_ReadByte(msg);
			if (voicePacket.dataSize <= 0 || voicePacket.dataSize > MAX_VOICE_PACKET_DATA)
			{
				Logger::Print(Game::CON_CHANNEL_SERVER, "Received invalid voice packet of size {} from {}\n", voicePacket.dataSize, cl->name);
				return;
			}

			assert(voicePacket.dataSize <= MAX_VOICE_PACKET_DATA);
			assert(msg->data);
			assert(voicePacket.data);

			Game::MSG_ReadData(msg, voicePacket.data, voicePacket.dataSize);
			G_BroadcastVoice(cl->gentity, &voicePacket);
		}
	}

	void Voice::SV_PreGameUserVoice(Game::client_s* cl, Game::msg_t* msg)
	{
		Game::VoicePacket_t voicePacket{};

		if (!SV_VoiceEnabled())
		{
			return;
		}

		const auto talker = cl - Game::svs_clients;

		AssertIn(talker, (*Game::sv_maxclients)->current.integer);

		const auto packetCount = Game::MSG_ReadByte(msg);
		for (auto packet = 0; packet < packetCount; ++packet)
		{
			voicePacket.dataSize = Game::MSG_ReadShort(msg);
			if (voicePacket.dataSize <= 0 || voicePacket.dataSize > MAX_VOICE_PACKET_DATA)
			{
				Logger::Print(Game::CON_CHANNEL_SERVER, "Received invalid voice packet of size {} from {}\n", voicePacket.dataSize, cl->name);
				return;
			}

			assert(voicePacket.dataSize <= MAX_VOICE_PACKET_DATA);
			assert(msg->data);
			assert(voicePacket.data);

			Game::MSG_ReadData(msg, voicePacket.data, voicePacket.dataSize);
			for (auto otherPlayer = 0; otherPlayer < (*Game::sv_maxclients)->current.integer; ++otherPlayer)
			{
				if (otherPlayer != talker && Game::svs_clients[otherPlayer].header.state >= Game::CS_CONNECTED && !SV_ServerHasClientMuted(talker))
				{
					SV_QueueVoicePacket(talker, otherPlayer, &voicePacket);
				}
			}
		}
	}

	void Voice::SV_VoicePacket(Game::netadr_t from, Game::msg_t* msg)
	{
		const auto qport = Game::MSG_ReadShort(msg);
		auto* cl = Game::SV_FindClientByAddress(from, qport, 0);
		if (!cl || cl->header.state == Game::CS_ZOMBIE)
		{
			return;
		}

		cl->lastPacketTime = *Game::svs_time;
		if (cl->header.state < Game::CS_ACTIVE)
		{
			SV_PreGameUserVoice(cl, msg);
		}
		else
		{
			assert(cl->gentity);
			SV_UserVoice(cl, msg);
		}
	}

	void Voice::CL_WriteVoicePacket_Hk(const int localClientNum)
	{
		const auto connstate = Game::CL_GetLocalClientConnectionState(localClientNum);
		const auto* clc = Game::CL_GetLocalClientConnection(localClientNum);
		const auto* vc = Game::CL_GetLocalClientVoiceCommunication(localClientNum);
		if (clc->demoplaying || (connstate < Game::CA_LOADING))
		{
			return;
		}

		unsigned char voicePacketBuf[0x800]{};
		Game::msg_t msg{};

		Game::MSG_Init(&msg, voicePacketBuf, sizeof(voicePacketBuf));
		Game::MSG_WriteString(&msg, "v");
		Game::MSG_WriteShort(&msg, clc->qport);
		Game::MSG_WriteByte(&msg, vc->voicePacketCount);

		for (auto voicePacket = 0; voicePacket < vc->voicePacketCount; ++voicePacket)
		{
			assert(vc->voicePackets[voicePacket].dataSize > 0);
			assert(vc->voicePackets[voicePacket].dataSize < (2 << 15));

			Game::MSG_WriteByte(&msg, vc->voicePackets[voicePacket].dataSize);
			Game::MSG_WriteData(&msg, vc->voicePackets[voicePacket].data, vc->voicePackets[voicePacket].dataSize);
		}

		Game::NET_OutOfBandVoiceData(clc->netchan.sock, clc->serverAddress, msg.data, msg.cursize, true);
		if ((*Game::cl_showSend)->current.enabled)
		{
			Logger::Print(Game::CON_CHANNEL_CLIENT, "voice: {}\n", msg.cursize);
		}
	}

	void Voice::CL_ClearMutedList()
	{
		std::memset(S_PlayerMute, 0, sizeof(S_PlayerMute));
	}

	bool Voice::CL_IsPlayerTalking_Hk([[maybe_unused]] Game::SessionData* session, [[maybe_unused]] const int localClientNum, const int talkingClientIndex)
	{
		// Skip all the Party related code
		return Game::Voice_IsClientTalking(talkingClientIndex);
	}

	bool Voice::CL_IsPlayerMuted_Hk([[maybe_unused]] Game::SessionData* session, [[maybe_unused]] const int localClientNum, const int muteClientIndex)
	{
		AssertIn(muteClientIndex, Game::MAX_CLIENTS);
		return S_PlayerMute[muteClientIndex];
	}

	void Voice::CL_MutePlayer_Hk([[maybe_unused]] Game::SessionData* session, const int muteClientIndex)
	{
		AssertIn(muteClientIndex, Game::MAX_CLIENTS);
		S_PlayerMute[muteClientIndex] = true;
	}

	void Voice::Voice_UnmuteMember_Hk([[maybe_unused]] Game::SessionData* session, const int clientNum)
	{
		AssertIn(clientNum, Game::MAX_CLIENTS);
		S_PlayerMute[clientNum] = false;
	}

	void Voice::CL_TogglePlayerMute(const int localClientNum, const int muteClientIndex)
	{
		AssertIn(muteClientIndex, Game::MAX_CLIENTS);

		if (CL_IsPlayerMuted_Hk(nullptr, localClientNum, muteClientIndex))
		{
			Voice_UnmuteMember_Hk(nullptr, muteClientIndex);
		}
		else
		{
			CL_MutePlayer_Hk(nullptr, muteClientIndex);
		}
	}

	void Voice::CL_VoicePacket(Game::netadr_t* address, Game::msg_t* msg)
	{
		auto* clc = Game::CL_GetLocalClientConnection(0);
		if (!Game::NET_CompareBaseAdr(clc->serverAddress, *address))
		{
			Logger::Debug("Ignoring stray 'v' network message from '{}'", Game::NET_AdrToString(*address));
			return;
		}

		const auto numPackets = Game::MSG_ReadByte(msg);
		if (numPackets < 0 || numPackets > MAX_SERVER_QUEUED_VOICE_PACKETS)
		{
			return;
		}

		Game::VoicePacket_t voicePacket{};
		for (auto packet = 0; packet < numPackets; ++packet)
		{
			voicePacket.talker = static_cast<char>(Game::MSG_ReadByte(msg));
			voicePacket.dataSize = Game::MSG_ReadByte(msg);
			if (voicePacket.dataSize <= 0 || voicePacket.dataSize > MAX_VOICE_PACKET_DATA)
			{
				Logger::Print(Game::CON_CHANNEL_CLIENT, "Invalid server voice packet of {} bytes\n", voicePacket.dataSize);
				return;
			}

			Game::MSG_ReadData(msg, voicePacket.data, voicePacket.dataSize);

			if (static_cast<unsigned char>(voicePacket.talker) >= Game::MAX_CLIENTS)
			{
				Logger::Print(Game::CON_CHANNEL_CLIENT, "Invalid voice packet - talker was {}\n", voicePacket.talker);
				return;
			}

			if (!CL_IsPlayerMuted_Hk(nullptr, 0, voicePacket.talker))
			{
				if ((*Game::cl_voice)->current.enabled)
				{
					Game::Voice_IncomingVoiceData(nullptr, voicePacket.talker, reinterpret_cast<unsigned char*>(voicePacket.data), voicePacket.dataSize);
				}
			}
		}
	}

	void Voice::UI_Mute_player(const int clientNum, const int localClientNum)
	{
		CL_TogglePlayerMute(localClientNum, Game::sharedUiInfo->playerClientNums[clientNum]);
	}

	__declspec(naked) void Voice::UI_Mute_Player_Stub()
	{
		__asm
		{
			push eax
			call UI_Mute_player
			add esp, 8 // Game already pushed localClientNum

			pop edi
			pop esi
			add esp, 0xC00
			ret
		}
	}

	Voice::Voice()
	{
		AssertOffset(Game::clientUIActive_t, connectionState, 0x9B8);

		std::memset(VoicePackets, 0, sizeof(VoicePackets));
		std::memset(VoicePacketCount, 0, sizeof(VoicePacketCount));

		SV_ClearMutedList();
		CL_ClearMutedList();

		Events::OnSteamDisconnect(CL_ClearMutedList);
		Events::OnClientDisconnect(SV_UnmuteClient);
		Events::OnClientConnect([](const Game::client_s* cl) -> void
		{
			if (Chat::IsMuted(cl))
			{
				SV_MuteClient(cl - Game::svs_clients);
			}
		});

		// Write voice packets to the server instead of other clients
		Utils::Hook(0x487935, CL_WriteVoicePacket_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x5AD945, CL_WriteVoicePacket_Hk, HOOK_CALL).install()->quick();

		// Disable 'v' OOB handler and use our own
		Utils::Hook::Set<std::uint8_t>(0x5A9E02, 0xEB);
		Network::OnClientPacketRaw("v", CL_VoicePacket);

		Utils::Hook(0x4AE740, CL_IsPlayerTalking_Hk, HOOK_JUMP).install()->quick();
		Utils::Hook(0x4B6250, CL_IsPlayerMuted_Hk, HOOK_JUMP).install()->quick();

		Utils::Hook(0x4519F5, SV_SendClientMessages_Stub, HOOK_CALL).install()->quick();

		// Recycle packet handler for 'icanthear'
		Utils::Hook::Set<const char*>(0x62673F, "v");
		Utils::Hook(0x626787, SV_VoicePacket, HOOK_CALL).install()->quick();

		Utils::Hook(0x45F041, UI_Mute_Player_Stub, HOOK_JUMP).install()->quick();

		Utils::Hook(0x4C6B50, Voice_UnmuteMember_Hk, HOOK_JUMP).install()->quick();
		Utils::Hook(0x43F460, CL_MutePlayer_Hk, HOOK_JUMP).install()->quick();

		sv_voice = Game::Dvar_RegisterBool("sv_voice", false, Game::DVAR_NONE, "Use server side voice communications");
	}
}
