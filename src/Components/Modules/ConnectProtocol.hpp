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
		class Container
		{
		public:
			bool Evaluated;
			std::string ConnectString;
		};

		static Container ConnectContainer;

		static void EvaluateProtocol();
		static bool InstallProtocol();
	};
}