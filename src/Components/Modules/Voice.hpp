#pragma once

namespace Components
{
	class Voice : public Component
	{
	public:
		Voice();

		static bool SV_VoiceEnabled();

		static void SV_ClearMutedList();
		static void SV_MuteClient(int muteClientIndex);
		static void SV_UnmuteClient(int muteClientIndex);

	private:
		static constexpr auto MAX_VOICE_PACKET_DATA = 256;
		static constexpr auto MAX_SERVER_QUEUED_VOICE_PACKETS = 40;

		static Game::VoicePacket_t VoicePackets[Game::MAX_CLIENTS][MAX_SERVER_QUEUED_VOICE_PACKETS];
		static int VoicePacketCount[Game::MAX_CLIENTS];

		static bool MuteList[Game::MAX_CLIENTS];
		static bool S_PlayerMute[Game::MAX_CLIENTS];

		static const Game::dvar_t* sv_voice;

		static void SV_WriteVoiceDataToClient(int clientNum, Game::msg_t* msg);
		static void SV_SendClientVoiceData(Game::client_s* client);
		static void SV_SendClientMessages_Stub(Game::client_s* client, Game::msg_t* msg, unsigned char* snapshotMsgBuf);

		static bool SV_ServerHasClientMuted(int talker);

		static bool OnSameTeam(const Game::gentity_s* ent1, const Game::gentity_s* ent2);
		static void SV_QueueVoicePacket(int talkerNum, int clientNum, const Game::VoicePacket_t* voicePacket);
		static void G_BroadcastVoice(Game::gentity_s* talker, const Game::VoicePacket_t* voicePacket);
		static void SV_UserVoice(Game::client_s* cl, Game::msg_t* msg);
		static void SV_PreGameUserVoice(Game::client_s* cl, Game::msg_t* msg);
		static void SV_VoicePacket(Game::netadr_t from, Game::msg_t* msg);

		static void CL_ClearMutedList();
		static bool CL_IsPlayerTalking_Hk(Game::SessionData* session, int localClientNum, int talkingClientIndex);
		static bool CL_IsPlayerMuted_Hk(Game::SessionData* session, int localClientNum, int muteClientIndex);
		static void CL_MutePlayer_Hk(Game::SessionData* session, int muteClientIndex);
		static void Voice_UnmuteMember_Hk(Game::SessionData* session, int clientNum);
		static void CL_TogglePlayerMute(int localClientNum, int muteClientIndex);

		static void CL_WriteVoicePacket_Hk(int localClientNum);
		static void CL_VoicePacket(Game::netadr_t* address, Game::msg_t* msg);

		static void UI_Mute_player(int clientNum, int localClientNum);
		static void UI_Mute_Player_Stub();
	};
}
