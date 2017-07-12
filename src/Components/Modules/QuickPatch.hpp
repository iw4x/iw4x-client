#pragma once

namespace Components
{
	class QuickPatch : public Component
	{
	public:
		QuickPatch();
		~QuickPatch();

		bool unitTest() override;

		static void UnlockStats();
		static int GetFrameTime() { return FrameTime; }

	private:
		static int FrameTime;

		static void SelectStringTableEntryInDvarStub();

		static int SVCanReplaceServerCommand(Game::client_t *client, const char *cmd);

		static int MsgReadBitsCompressCheckSV(const char *from, char *to, int size);
		static int MsgReadBitsCompressCheckCL(const char *from, char *to, int size);

		static void CompareMaterialStateBits();
	};
}
