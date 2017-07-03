#pragma once

namespace Components
{
	class Scheduler : public Component
	{
	public:
		typedef void(Callback)();

		Scheduler();
		~Scheduler();

		void preDestroy() override;

		static void OnShutdown(Utils::Slot<Callback> callback);
		static void OnFrame(Utils::Slot<Callback> callback, bool clientOnly = false);
		static void OnReady(Utils::Slot<Callback> callback, bool clientOnly = false);
		static void Once(Utils::Slot<Callback> callback, bool clientOnly = false);
		static void OnDelay(Utils::Slot<Callback> callback, std::chrono::nanoseconds delay, bool clientOnly = false);

		static void OnFrameAsync(Utils::Slot<Callback> callback);
		static void OnceAsync(Utils::Slot<Callback> callback);

		static void FrameHandler();

	private:
		class DelayedSlot
		{
		public:
			std::chrono::nanoseconds delay;
			Utils::Time::Interval interval;
			Utils::Slot<Callback> callback;
		};

		static bool AsyncTerminate;
		static std::thread AsyncThread;

		static Utils::Signal<Callback> FrameSignal;
		static Utils::Signal<Callback> FrameOnceSignal;
		static std::vector<DelayedSlot> DelayedSlots;

		static bool ReadyPassed;
		static Utils::Signal<Callback> ReadySignal;
		static Utils::Signal<Callback> ShutdownSignal;

		static Utils::Signal<Callback> AsyncFrameSignal;
		static Utils::Signal<Callback> AsyncFrameOnceSignal;

		static void ReadyHandler();
		static void DelaySignal();

		static void ShutdownStub(int num);
	};
}
