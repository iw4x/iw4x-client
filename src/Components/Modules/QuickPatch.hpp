#pragma once

namespace Components
{
	class QuickPatch : public Component
	{
	public:
		QuickPatch();
		~QuickPatch();

		bool unitTest() override;

		static void UnlockStats();
		static int GetFrameTime() { return FrameTime; }

	private:
		static int FrameTime;

		static void SelectStringTableEntryInDvarStub();

		static int SVCanReplaceServerCommand(Game::client_t *client, const char *cmd);
		static int G_GetClientScore();

		static int MsgReadBitsCompressCheckSV(const char *from, char *to, int size);
		static int MsgReadBitsCompressCheckCL(const char *from, char *to, int size);

		static long AtolAdjustPlayerLimit(const char* string);

		static void JavelinResetHookStub();

		static bool InvalidNameCheck(char* dest, const char* source, int size);
		static void InvalidNameStub();

		static Game::dvar_t* sv_enableBounces;
		static void BounceStub();

		static Dvar::Var r_customAspectRatio;
		static Game::dvar_t* Dvar_RegisterAspectRatioDvar(const char* name, char** enumValues, int defaultVal, int flags, const char* description);
		static void SetAspectRatioStub();
		static void SetAspectRatio();

		static Game::dvar_t* g_antilag;
		static void ClientEventsFireWeaponStub();
		static void ClientEventsFireWeaponMeleeStub();

		static Game::dvar_t* g_playerCollision;
		static void PlayerCollisionStub();
		static Game::dvar_t* g_playerEjection;
		static void PlayerEjectionStub();
		static BOOL IsDynClassnameStub(char* a1);

		static void CL_KeyEvent_OnEscape();
		static void CL_KeyEvent_ConsoleEscape_Stub();
	};
}
