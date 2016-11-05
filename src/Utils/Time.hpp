namespace Utils
{
	namespace Time
	{
		class Interval
		{
		private:
			std::chrono::high_resolution_clock::time_point LastPoint;

		public:
			Interval() : LastPoint(std::chrono::high_resolution_clock::now()) {}

			void Set();
			bool Elapsed(std::chrono::nanoseconds nsecs);
		};
	}
}
