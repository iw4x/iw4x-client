#include "STDInclude.hpp"

namespace Components
{
	// This component is a workaround for issue https://github.com/XLabsProject/iw4x-client/issues/80
	// In case the link goes down, this is a "game hangs randomly" issue:
	// 
	// Investigations on the issue pointed out it comes from a situation on Intel processors where
	//		WaitForSingleObjectA is ignored by a thread, for some (?) reason.
	// 
	// This locks up the game randomly, mostly at the end of rounds or when too many things happen at
	//		once, due to trying to stop sounds (AIL_Stop_sounds) and playing streams at the same time,
	//		rushing for the same resource via AIL_lock_mutex. 
	// 
	// This bug has been reproduced on	mp_terminal, mp_overgrown, mp_rust, with and without bots,
	//		and so far this has been the only way to circumvent it afaik. This component wraps
	//		miles' mutex into another mutex, created below, and for some reason (?) that mutex is 
	//		respected when miles' is not.
	// 
	// As soon as a real fix is found, please discard this fix. In the meantime, it should not
	//		have side effects too bad - worst case it might cause a slight performance drop during
	//		team switch and intermission.
	// 

	std::mutex SoundMutexFix::SNDMutex;

	void __stdcall SoundMutexFix::LockSoundMutex(int unk)
	{
		std::lock_guard lock(SoundMutexFix::SNDMutex);

		DWORD funcPtr = *reinterpret_cast<DWORD*>(0x6D7554); // AIL_close_stream
		Utils::Hook::Call<void __stdcall(int)>(funcPtr)(unk);
	}

	SoundMutexFix::SoundMutexFix()
	{
		Utils::Hook(0x689EFE, &SoundMutexFix::LockSoundMutex, HOOK_JUMP).install()->quick();
	}
}