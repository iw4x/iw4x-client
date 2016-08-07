namespace Components
{
	class QuickPatch : public Component
	{
	public:
		typedef void(Callback)();

		QuickPatch();
		~QuickPatch();
		const char* GetName() { return "QuickPatch"; };

		static void UnlockStats();
		static void OnShutdown(Callback* callback);

		static void OnFrame(Callback* callback);
		static void Once(Callback* callback);

	private:
		static wink::signal<wink::slot<Callback>> ShutdownSignal;

		static int64_t* GetStatsID();
		static void ShutdownStub(int channel, const char* message);

		static void SelectStringTableEntryInDvarStub();

		static int MsgReadBitsCompressCheckSV(const char *from, char *to, int size);
		static int MsgReadBitsCompressCheckCL(const char *from, char *to, int size);
		static void CL_HandleRelayPacketCheck(Game::msg_t* msg, int client);
	};
}
