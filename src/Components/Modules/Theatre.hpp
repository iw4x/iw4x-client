namespace Components
{
	class Theatre : public Component
	{
	public:
		Theatre();
		const char* GetName() { return "Theatre"; };

	private:
		static char BaselineSnapshot[131072];
		static PBYTE BaselineSnapshotMsg;
		static int BaselineSnapshotMsgLen;
		static int BaselineSnapshotMsgOff;

		static void WriteBaseline();

		static void GamestateWriteStub(Game::msg_t* msg, char byte);
		static void RecordGamestateStub();
		static void BaselineStoreStub();
		static void BaselineToFileStub();
		static void AdjustTimeDeltaStub();
		static void ServerTimedOutStub();
		static void UISetActiveMenuStub();
	};
}
