namespace Components
{
	class Bots : public Component
	{
	public:
		Bots();
		~Bots();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "Bots"; };
#endif

	private:
		static std::vector<std::string> BotNames;

		static void BuildConnectString(char* buffer, const char* connectString, int num, int, int protocol, int checksum, int statVer, int statStuff, int port);
	};
}
