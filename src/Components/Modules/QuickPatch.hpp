namespace Components
{
	class QuickPatch : public Component
	{
	public:
		typedef void(Callback)();

		QuickPatch();
		~QuickPatch();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "QuickPatch"; };
#endif

		bool unitTest();

		static void UnlockStats();
		static void OnShutdown(Callback* callback);

		static void OnFrame(Callback* callback);
		static void Once(Callback* callback);

	private:
		static wink::signal<wink::slot<Callback>> ShutdownSignal;

		static int64_t* GetStatsID();
		static void ShutdownStub(int num);

		static void SelectStringTableEntryInDvarStub();

		static int MsgReadBitsCompressCheckSV(const char *from, char *to, int size);
		static int MsgReadBitsCompressCheckCL(const char *from, char *to, int size);
		static void CL_HandleRelayPacketCheck(Game::msg_t* msg, int client);

#ifdef DEBUG
        static FILE* readLog;
        static Utils::Hook beginLoggingHook;
        static int* g_streamPos;
        static void logReads(bool flush);
        static void logXAssetRead(int len);
        static void logXAssetVirtualRead(int len);
#endif
	};
}
