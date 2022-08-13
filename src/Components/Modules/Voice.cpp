#include <STDInclude.hpp>

namespace Components
{
	Game::VoicePacket_t Voice::voicePackets[Game::MAX_CLIENTS][MAX_SERVER_QUEUED_VOICE_PACKETS];
	int Voice::voicePacketCount[Game::MAX_CLIENTS];

	const Game::dvar_t* Voice::sv_voice;

	bool Voice::SV_VoiceEnabled()
	{
		return sv_voice->current.enabled;
	}

	void Voice::SV_WriteVoiceDataToClient(const int clientNum, Game::msg_t* msg)
	{
		assert(voicePacketCount[clientNum] >= 0);
		assert(voicePacketCount[clientNum] <= MAX_SERVER_QUEUED_VOICE_PACKETS);

		Game::MSG_WriteByte(msg, voicePacketCount[clientNum]);
		for (auto packet = 0; packet < voicePacketCount[clientNum]; ++packet)
		{
			Game::MSG_WriteByte(msg, voicePackets[clientNum][packet].talker);

			assert(voicePackets[clientNum][packet].dataSize < (2 << 15));

			Game::MSG_WriteByte(msg, voicePackets[clientNum][packet].dataSize);
			Game::MSG_WriteData(msg, voicePackets[clientNum][packet].data, voicePackets[clientNum][packet].dataSize);
		}

		assert(!msg->overflowed);
	}

	void Voice::SV_SendClientVoiceData(Game::client_t* client)
	{
		const auto msg_buf = std::make_unique<unsigned char[]>(0x10000);
		Game::msg_t msg{};
		const auto clientNum = client - Game::svs_clients;

		assert(voicePacketCount[clientNum] >= 0);

		if (client->state == Game::CS_ACTIVE && voicePacketCount[clientNum])
		{
			Game::MSG_Init(&msg, msg_buf.get(), 0x10000);

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
				Game::NET_OutOfBandVoiceData(Game::NS_SERVER, client->netchan.remoteAddress, msg.data, msg.cursize, true);
				voicePacketCount[clientNum] = 0;
			}
		}
	}

	void Voice::SV_SendClientMessages_Stub(Game::client_t* client, Game::msg_t* msg, unsigned char* snapshotMsgBuf)
	{
		// SV_EndClientSnapshot
		Utils::Hook::Call<void(Game::client_t*, Game::msg_t*, unsigned char*)>(0x4F5300)(client, msg, snapshotMsgBuf);

		SV_SendClientVoiceData(client);
	}

	bool Voice::OnSameTeam(Game::gentity_s* ent1, Game::gentity_s* ent2)
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

	void Voice::SV_QueueVoicePacket(int talkerNum, int clientNum, Game::VoicePacket_t* voicePacket)
	{
		assert(talkerNum >= 0);
		assert(clientNum >= 0);
		assert(talkerNum < (*Game::sv_maxclients)->current.integer);
		assert(clientNum < (*Game::sv_maxclients)->current.integer);

		if (voicePacketCount[clientNum] < MAX_SERVER_QUEUED_VOICE_PACKETS)
		{
			voicePackets[clientNum][voicePacketCount[clientNum]].dataSize = voicePacket->dataSize;
			std::memcpy(voicePackets[clientNum][voicePacketCount[clientNum]].data, voicePacket->data, voicePacket->dataSize);

			assert(talkerNum == static_cast<byte>(talkerNum));
			voicePackets[clientNum][voicePacketCount[clientNum]].talker = static_cast<char>(talkerNum);
			++voicePacketCount[clientNum];
		}
	}

	void Voice::G_BroadcastVoice(Game::gentity_s* talker, Game::VoicePacket_t* voicePacket)
	{
		for (auto otherPlayer = 0; otherPlayer < (*Game::sv_maxclients)->current.integer; ++otherPlayer)
		{
			auto* ent = &Game::g_entities[otherPlayer];
			auto* client = ent->client;

			if (ent->r.isInUse && client && (client->sess.sessionState == Game::SESS_STATE_INTERMISSION || OnSameTeam(talker, ent) || talker->client->sess.cs.team == Game::TEAM_FREE) &&
				(ent->client->sess.sessionState == talker->client->sess.sessionState || (ent->client->sess.sessionState == Game::SESS_STATE_DEAD || talker->client->sess.sessionState == Game::SESS_STATE_DEAD) &&
					(*Game::g_deadChat)->current.enabled) && (talker != ent))
			{
				SV_QueueVoicePacket(talker->s.number, otherPlayer, voicePacket);
			}
		}
	}

	void Voice::SV_UserVoice(Game::client_t* cl, Game::msg_t* msg)
	{
		Game::VoicePacket_t voicePacket{};

		if (!SV_VoiceEnabled())
		{
			return;
		}

		const auto packetCount = Game::MSG_ReadByte(msg);

		assert(cl->gentity);

		for (int packet = 0; packet < packetCount; ++packet)
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

	void Voice::SV_VoicePacket(Game::netadr_t from, Game::msg_t* msg)
	{
		auto qport = Game::MSG_ReadShort(msg);
		auto* cl = Game::SV_FindClientByAddress(from, qport, 0);
		if (!cl || cl->state == Game::CS_ZOMBIE)
		{
			return;
		}

		if (cl->state == Game::CS_ACTIVE)
		{
			assert(cl->gentity);
			SV_UserVoice(cl, msg);
		}
	}

	void Voice::CL_WriteVoicePacket_Hk(int localClientNum)
	{
		const auto connstate = Game::CL_GetLocalClientConnectionState(localClientNum);
		const auto clc = Game::CL_GetLocalClientConnection(localClientNum);
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

	void Voice::CL_VoicePacket_Hk(const int localClientNum, Game::msg_t* msg)
	{
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

			Game::SessionData* session{};
			if (Game::Party_InParty(Game::g_lobbyData))
			{
				session = Game::g_lobbyData->session;
			}
			else if (Game::Party_InParty(Game::g_partyData))
			{
				session = Game::g_partyData->session;
			}
			else
			{
				session = Game::g_serverSession;
			}

			if (!Game::CL_IsPlayerMuted(session, localClientNum, voicePacket.talker))
			{
				if ((*Game::cl_voice)->current.enabled)
				{
					Game::Voice_IncomingVoiceData(session, voicePacket.talker, reinterpret_cast<unsigned char*>(voicePacket.data), voicePacket.dataSize);
				}
			}
		}
	}

	Voice::Voice()
	{
		AssertOffset(Game::clientUIActive_t, connectionState, 0x9B8);

		// Write voice packets to the server instead of other clients
		Utils::Hook(0x487935, CL_WriteVoicePacket_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x5AD945, CL_WriteVoicePacket_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x5A9E06, CL_VoicePacket_Hk, HOOK_CALL).install()->quick();

		Utils::Hook(0x4519F5, SV_SendClientMessages_Stub, HOOK_CALL).install()->quick();

		// Recycle packet handler for 'icanthear'
		Utils::Hook::Set<const char*>(0x62673F, "v");
		Utils::Hook(0x626787, SV_VoicePacket, HOOK_CALL).install()->quick();

		sv_voice = Game::Dvar_RegisterBool("sv_voice", false, Game::DVAR_NONE, "Use server side voice communications");
	}
}
