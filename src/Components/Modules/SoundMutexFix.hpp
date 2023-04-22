#pragma once
#include <mutex>

namespace Components
{
	class SoundMutexFix : public Component
	{
	public:
		SoundMutexFix();
		
	private:
		static std::mutex CloseStreamMutex;
		static void WINAPI AIL_close_stream_Stub(int h_stream);
	};
}
