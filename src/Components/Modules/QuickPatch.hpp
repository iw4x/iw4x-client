#pragma once

namespace Components
{
	class QuickPatch : public Component
	{
	public:
		typedef void(Callback)();

		QuickPatch();
		~QuickPatch();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "QuickPatch"; };
#endif

		bool unitTest() override;

		static void UnlockStats();
		static void OnShutdown(Utils::Slot<Callback> callback);

		static void OnFrame(Utils::Slot<Callback> callback);
		static void Once(Utils::Slot<Callback> callback);

	private:
		static Utils::Signal<Callback> ShutdownSignal;

		static int64_t* GetStatsID();
		static void ShutdownStub(int num);

		static void SelectStringTableEntryInDvarStub();

		static int MsgReadBitsCompressCheckSV(const char *from, char *to, int size);
		static int MsgReadBitsCompressCheckCL(const char *from, char *to, int size);

		static void CompareMaterialStateBits();
	};
}
