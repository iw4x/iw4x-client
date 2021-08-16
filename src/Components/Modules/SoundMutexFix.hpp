#pragma once
#include <mutex>

namespace Components
{
	class SoundMutexFix : public Component
	{
	public:
		SoundMutexFix();
		
	private:
		static std::mutex snd_mutex;
		static void _stdcall LockSoundMutex(int unk);
	};
}
