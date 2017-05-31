#pragma once

namespace Components
{
	class Scheduler : public Component
	{
	public:
		typedef void(Callback)();

		Scheduler();
		~Scheduler();

		static void OnShutdown(Utils::Slot<Callback> callback);
		static void OnFrame(Utils::Slot<Callback> callback);
		static void OnReady(Utils::Slot<Callback> callback);
		static void Once(Utils::Slot<Callback> callback);
		static void OnDelay(Utils::Slot<Callback> callback, std::chrono::nanoseconds delay);

		static void FrameHandler();

	private:
		class DelayedSlot
		{
		public:
			std::chrono::nanoseconds delay;
			Utils::Time::Interval interval;
			Utils::Slot<Callback> callback;
		};

		static Utils::Signal<Callback> FrameSignal;
		static Utils::Signal<Callback> FrameOnceSignal;
		static std::vector<DelayedSlot> DelayedSlots;

		static bool ReadyPassed;
		static Utils::Signal<Callback> ReadySignal;
		static Utils::Signal<Callback> ShutdownSignal;

		static void ReadyHandler();
		static void DelaySignal();
	};
}
