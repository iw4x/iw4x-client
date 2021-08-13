#pragma once

namespace Components
{
	class SoundMutexFix : public Component
	{
	public:
		SoundMutexFix();
		~SoundMutexFix();

		static void SND_StopStreamChannelHook(int channel);
		static std::mutex snd_mutex;
	};
}
