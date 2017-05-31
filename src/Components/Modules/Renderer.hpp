#pragma once

namespace Components
{
	class Renderer : public Component
	{
	public:
		typedef void(Callback)();
		typedef void(BackendCallback)(IDirect3DDevice9*);

		Renderer();
		~Renderer();

		static int Width();
		static int Height();

		static void Once(Utils::Slot<Callback> callback);
		static void OnFrame(Utils::Slot<Callback> callback);
		static void OnBackendFrame(Utils::Slot<BackendCallback> callback);
		static void OnDelay(Utils::Slot<Callback> callback, std::chrono::nanoseconds delay);

		static void OnDeviceRecoveryEnd(Utils::Slot<Callback> callback);
		static void OnDeviceRecoveryBegin(Utils::Slot<Callback> callback);

	private:
		class DelayedSlot
		{
		public:
			std::chrono::nanoseconds delay;
			Utils::Time::Interval interval;
			Utils::Slot<Callback> callback;
		};

		static void FrameStub();
		static void FrameHandler();

		static void BackendFrameStub();
		static void BackendFrameHandler();

		static void DelaySignal();

		static Utils::Signal<Callback> FrameSignal;
		static Utils::Signal<Callback> FrameOnceSignal;

		static Utils::Signal<Callback> EndRecoverDeviceSignal;
		static Utils::Signal<Callback> BeginRecoverDeviceSignal;

		static Utils::Signal<BackendCallback> BackendFrameSignal;

		static std::vector<DelayedSlot> DelayedSlots;
	};
}
