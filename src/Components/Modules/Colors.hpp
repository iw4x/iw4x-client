#pragma once

namespace Components
{
	class Colors : public Component
	{
	public:
		Colors();
		~Colors();

		static void Strip(const char* in, char* out, int max);
		static std::string Strip(const std::string& in);

		static char Add(uint8_t r, uint8_t g, uint8_t b);

	private:
		struct HsvColor
		{
			unsigned char h;
			unsigned char s;
			unsigned char v;
		};

		static Dvar::Var NewColors;
		static Dvar::Var ColorBlind;
		static Game::dvar_t* ColorAllyColorBlind;
		static Game::dvar_t* ColorEnemyColorBlind;

		static DWORD HsvToRgb(HsvColor hsv);

		static void UserInfoCopy(char* buffer, const char* name, size_t size);

		static void ClientUserinfoChanged();
		static char* GetClientName(int localClientNum, int index, char *buf, size_t size);
		static void PatchColorLimit(char limit);

		static unsigned int ColorIndex(char index);
		static void LookupColor(DWORD* color, char index);
		static void LookupColorStub();
		static char* CleanStrStub(char* string);
		static bool Dvar_GetUnpackedColorByName(const char* name, float* expandedColor);
		static void GetUnpackedColorByNameStub();
		static std::vector<DWORD> ColorTable;
	};
}
