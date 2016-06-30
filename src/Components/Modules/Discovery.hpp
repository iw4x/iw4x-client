namespace Components
{
	class Discovery : public Component
	{
	public:
		Discovery();
		~Discovery();
		const char* GetName() { return "Discovery"; };

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
