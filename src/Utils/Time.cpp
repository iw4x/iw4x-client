#include <STDInclude.hpp>

namespace Utils::Time
{
	void Interval::update()
	{
		this->lastPoint = std::chrono::high_resolution_clock::now();
	}

	bool Interval::elapsed(std::chrono::nanoseconds nsecs) const
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

	int Point::diff(Point point) const
	{
		return point.lastPoint - this->lastPoint;
	}

	bool Point::after(Point point) const
	{
		return this->diff(point) < 0;
	}

	bool Point::elapsed(int milliseconds) const
	{
		return (Game::Sys_Milliseconds() - this->lastPoint) >= milliseconds;
	}
}
