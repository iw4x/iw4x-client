#pragma once

namespace Components
{
	class PlayerName : public Component
	{
	public:
		PlayerName();

		static void UserInfoCopy(char* buffer, const char* name, size_t size);

	private:
		static Dvar::Var sv_allowColoredNames;

		static char* CleanStrStub(char* string);
		static void ClientUserinfoChanged();
		static char* GetClientName(int localClientNum, int index, char* buf, size_t size);
	};
}
