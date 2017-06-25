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

		std::chrono::high_resolution_clock::duration Point::diff(Point point)
		{
			return point.lastPoint - this->lastPoint;
		}

		bool Point::after(Point point)
		{
			return this->diff(point).count() < 0;
		}
	}
}
