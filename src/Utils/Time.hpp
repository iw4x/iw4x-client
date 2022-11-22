#pragma once

namespace Utils::Time
{
	class Interval
	{
	protected:
		std::chrono::high_resolution_clock::time_point lastPoint;

	public:
		Interval() : lastPoint(std::chrono::high_resolution_clock::now()) {}

		void update();
		bool elapsed(std::chrono::nanoseconds nsecs) const;
	};

	class Point
	{
	public:
		Point();

		void update();
		int diff(Point point) const;
		bool after(Point point) const;
		bool elapsed(int milliseconds) const;

	private:
		int lastPoint;
	};
}
