#pragma once

namespace Components
{
	class PlayerName : public Component
	{
	public:
		PlayerName();

		static void UserInfoCopy(char* buffer, const char* name, int size);

		static int GetClientName(int localClientNum, int index, char* buf, int size);

	private:
		static Dvar::Var sv_allowColoredNames;

		static char* CleanStrStub(char* string);
		static void ClientCleanName();

		static bool IsBadChar(int c);
		static bool CopyClientNameCheck(char* dest, const char* source, int size);
		static void DropClient(Game::client_s* drop);
		static void SV_UserinfoChangedStub();
	};
}
