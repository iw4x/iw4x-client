namespace Components
{
	class ConnectProtocol : public Component
	{
	public:
		ConnectProtocol();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "ConnectProtocol"; };
#endif

		static bool IsEvaluated();
		static bool Used();

	private:
		static bool Evaluated;
		static std::string ConnectString;

		static void EvaluateProtocol();
		static bool InstallProtocol();

		static void Invocation();
	};
}
