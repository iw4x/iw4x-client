namespace Components
{
	class ConnectProtocol : public Component
	{
	public:
		ConnectProtocol();

#ifdef DEBUG
		const char* GetName() { return "ConnectProtocol"; };
#endif

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
