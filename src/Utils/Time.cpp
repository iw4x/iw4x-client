#include "STDInclude.hpp"

namespace Utils
{
	namespace Time
	{
		void Interval::update()
		{
			this->lastPoint = std::chrono::high_resolution_clock::now();
		}

		bool Interval::elapsed(std::chrono::nanoseconds nsecs)
		{
			return ((std::chrono::high_resolution_clock::now() - this->lastPoint) >= nsecs);
		}
	}
}
