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

		static void InsertBotName(const char* arg);
		static void BuildConnectStringStub();
	};
}
