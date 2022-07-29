#pragma once

namespace Components
{
	class PlayerName : public Component
	{
	public:
		PlayerName();

		static void UserInfoCopy(char* buffer, const char* name, size_t size);

		static int GetClientName(int localClientNum, int index, char* buf, int size);

	private:
		static Dvar::Var sv_allowColoredNames;
		// Message used when kicking players
		static constexpr auto INVALID_NAME_MSG = "Invalid name detected";

		static char* CleanStrStub(char* string);
		static void ClientCleanName();

		static bool CopyClientNameCheck(char* dest, const char* source, int size);
		static void SV_UserinfoChangedStub();
	};
}
