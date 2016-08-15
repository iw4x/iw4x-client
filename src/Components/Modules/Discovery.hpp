namespace Components
{
	class Discovery : public Component
	{
	public:
		Discovery();
		~Discovery();

#ifdef DEBUG
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
