#pragma once

namespace Components
{
	class QuickPatch : public Component
	{
	public:
		QuickPatch();

		bool unitTest() override;

		static void UnlockStats();

	private:
		static void JavelinResetHookStub();

		static Dvar::Var r_customAspectRatio;
		static Game::dvar_t* Dvar_RegisterAspectRatioDvar(const char* dvarName, const char** valueList, int defaultIndex, unsigned __int16 flags, const char* description);
		static void SetAspectRatioStub();
		static void SetAspectRatio();

		static Game::dvar_t* g_antilag;
		static void ClientEventsFireWeaponStub();
		static void ClientEventsFireWeaponMeleeStub();

		static BOOL IsDynClassnameStub(char* a1);

		static void CL_KeyEvent_OnEscape();
		static void CL_KeyEvent_ConsoleEscape_Stub();

		static Game::dvar_t* Dvar_RegisterConMinicon(const char* dvarName, bool value, unsigned __int16 flags, const char* description);
	};
}
