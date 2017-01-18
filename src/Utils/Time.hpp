namespace Utils
{
	namespace Time
	{
		class Interval
		{
		private:
			std::chrono::high_resolution_clock::time_point lastPoint;

		public:
			Interval() : lastPoint(std::chrono::high_resolution_clock::now()) {}

			void update();
			bool elapsed(std::chrono::nanoseconds nsecs);
		};
	}
}
