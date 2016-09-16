namespace Components
{
	class Colors : public Component
	{
	public:
		Colors();
		~Colors();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* GetName() { return "Colors"; };
#endif

		static void Strip(const char* in, char* out, int max);
		static std::string Strip(std::string in);

		static char Add(uint8_t r, uint8_t g, uint8_t b);

	private:
		struct HsvColor
		{
			unsigned char h;
			unsigned char s;
			unsigned char v;
		};

		static DWORD HsvToRgb(HsvColor hsv);

		static Dvar::Var NewColors;

		static void ClientUserinfoChanged();
		static char* GetClientName(int localClientNum, int index, char *buf, size_t size);
		static void PatchColorLimit(char limit);

		static unsigned int ColorIndex(unsigned char);
		static void LookupColor(DWORD* color, char index);
		static void LookupColorStub();
		static char* CleanStrStub(char* string);
		static std::vector<DWORD> ColorTable;
	};
}
