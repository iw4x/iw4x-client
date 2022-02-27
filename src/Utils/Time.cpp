#include <STDInclude.hpp>

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

		Point::Point() : lastPoint(Game::Sys_Milliseconds())
		{

		}

		void Point::update()
		{
			this->lastPoint = Game::Sys_Milliseconds();
		}

		int Point::diff(Point point)
		{
			return point.lastPoint - this->lastPoint;
		}

		bool Point::after(Point point)
		{
			return this->diff(point) < 0;
		}

		bool Point::elapsed(int milliseconds)
		{
			return (Game::Sys_Milliseconds() - this->lastPoint) >= milliseconds;
		}
	}
}
