namespace Components
{
	class Party : public Component
	{
	public:
		Party();
		~Party();
		const char* GetName() { return "Party"; };

		static void Connect(Network::Address target);

	private:
		struct JoinContainer
		{
			Network::Address Target;
			std::string Challenge;
			DWORD JoinTime;
			bool Valid;
		};

		static JoinContainer Container;
	};
}
