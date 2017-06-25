#pragma once

namespace Utils
{
	namespace Time
	{
		class Interval
		{
		protected:
			std::chrono::high_resolution_clock::time_point lastPoint;

		public:
			Interval() : lastPoint(std::chrono::high_resolution_clock::now()) {}

			void update();
			bool elapsed(std::chrono::nanoseconds nsecs);
		};

		class Point : public Interval
		{
		public:
			Point() : Interval() {}

			std::chrono::high_resolution_clock::duration diff(Point point);
			bool after(Point point);
		};
	}
}
