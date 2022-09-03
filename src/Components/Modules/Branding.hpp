#pragma once

namespace Components
{
	class Branding : public Component
	{
	public:
		Branding();

		static const char* GetBuildNumber();
		static const char* GetVersionString();

	private:
		static Dvar::Var CGDrawVersion;
		static Dvar::Var CGDrawVersionX;
		static Dvar::Var CGDrawVersionY;

		static void CG_DrawVersion();
		static void CG_DrawVersion_Hk(int localClientNum);

		// Use IW4x Branding
		static void Dvar_SetVersionString(const Game::dvar_t* dvar, const char* value);
		static void MSG_WriteVersionStringHeader(Game::msg_t* msg, const char* string);

		static Game::dvar_t* Dvar_RegisterUIBuildLocation(const char* dvarName, float x, float y, float min, float max, int flags, const char* description);

		static void RegisterBrandingDvars();
	};
}
