#pragma once

namespace Components
{
	class Branding : public Component
	{
	public:
		Branding();

	private:
		static Dvar::Var CGDrawVersion;
		static Dvar::Var CGDrawVersionX;
		static Dvar::Var CGDrawVersionY;
		static Game::dvar_t** Version;

		static void CG_DrawVersion();
		static void CG_DrawVersion_Hk(int localClientNum);

		static const char* GetBuildNumber();
		static void Dvar_SetVersionString(const Game::dvar_t* dvar, const char* value);

		static Game::dvar_t* Dvar_RegisterUIBuildLocation(const char* dvarName, float x, float y, float min, float max, int flags, const char* description);

		static void RegisterBrandingDvars();
	};
}
