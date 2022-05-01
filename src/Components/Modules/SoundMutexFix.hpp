#pragma once
#include <mutex>

namespace Components
{
	class SoundMutexFix : public Component
	{
	public:
		SoundMutexFix();
		
	private:
		static std::mutex SNDMutex;
		static void _stdcall LockSoundMutex(int unk);
	};
}
