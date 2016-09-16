namespace Components
{
	class Discovery : public Component
	{
	public:
		Discovery();
		~Discovery();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* GetName() { return "Discovery"; };
#endif

		static void Perform();

	private:
		class Container
		{
		public:
			bool Perform;
			bool Terminate;
			std::thread Thread;
			std::string Challenge;
		};

		static Container DiscoveryContainer;
	};
}
