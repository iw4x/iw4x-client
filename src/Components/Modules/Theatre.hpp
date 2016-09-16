namespace Components
{
	class Theatre : public Component
	{
	public:
		Theatre();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* GetName() { return "Theatre"; };
#endif

	private:
		class Container
		{
		public:
			class DemoInfo
			{
			public:
				std::string Name;
				std::string Mapname;
				std::string Gametype;
				std::string Author;
				int Length;
				std::time_t TimeStamp;

				json11::Json to_json() const
				{
					return json11::Json::object
					{
						{ "mapname", Mapname },
						{ "gametype", Gametype },
						{ "author", Author },
						{ "length", Length },
						{ "timestamp", fmt::sprintf("%lld", TimeStamp) } //Ugly, but prevents information loss
					};
				}
			};

			DemoInfo CurrentInfo;
			unsigned int CurrentSelection;
			std::vector<DemoInfo> Demos;
		};

		static Container DemoContainer;

		static char BaselineSnapshot[131072];
		static int BaselineSnapshotMsgLen;
		static int BaselineSnapshotMsgOff;

		static void WriteBaseline();
		static void StoreBaseline(PBYTE snapshotMsg);

		static void LoadDemos();
		static void DeleteDemo();
		static void PlayDemo();

		static unsigned int GetDemoCount();
		static const char* GetDemoText(unsigned int item, int column);
		static void SelectDemo(unsigned int index);

		static void GamestateWriteStub(Game::msg_t* msg, char byte);
		static void RecordGamestateStub();
		static void BaselineStoreStub();
		static void BaselineToFileStub();
		static void AdjustTimeDeltaStub();
		static void ServerTimedOutStub();
		static void UISetActiveMenuStub();

		static uint32_t InitCGameStub();
		static void MapChangeStub();
		static void MapChangeSVStub(char* a1, char* a2);

		static void RecordStub(int channel, char* message, char* file);
		static void StopRecordStub(int channel, char* message);
	};
}
