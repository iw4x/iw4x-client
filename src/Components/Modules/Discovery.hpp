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
		struct Container
		{
			bool Perform;
			bool Terminate;
			std::thread* Thread;
		};

		static Container DiscoveryContainer;
	};
}
