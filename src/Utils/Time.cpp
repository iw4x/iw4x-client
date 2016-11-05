#include "STDInclude.hpp"

namespace Utils
{
	namespace Time
	{
		void Interval::Set()
		{
			this->LastPoint = std::chrono::high_resolution_clock::now();
		}

		bool Interval::Elapsed(std::chrono::nanoseconds nsecs)
		{
			return ((std::chrono::high_resolution_clock::now() - this->LastPoint) >= nsecs);
		}
	}
}
