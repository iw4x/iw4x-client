namespace Components
{
	class ConnectProtocol : public Component
	{
	public:
		ConnectProtocol();
		const char* GetName() { return "ConnectProtocol"; };

		static bool Evaluated();
		static bool Used();

	private:
		struct Container
		{
			bool Evaluated;
			bool Invoked;
			std::string ConnectString;
		};

		static Container ConnectContainer;

		static void EvaluateProtocol();
		static bool InstallProtocol();
	};
}