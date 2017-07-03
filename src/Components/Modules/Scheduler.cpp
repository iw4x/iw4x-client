#include "STDInclude.hpp"

namespace Components
{
	bool Scheduler::AsyncTerminate;
	std::thread Scheduler::AsyncThread;

	bool Scheduler::ReadyPassed = false;
	Utils::Signal<Scheduler::Callback> Scheduler::ReadySignal;
	Utils::Signal<Scheduler::Callback> Scheduler::ShutdownSignal;

	Utils::Signal<Scheduler::Callback> Scheduler::FrameSignal;
	Utils::Signal<Scheduler::Callback> Scheduler::FrameOnceSignal;
	std::vector<Scheduler::DelayedSlot> Scheduler::DelayedSlots;

	Utils::Signal<Scheduler::Callback> Scheduler::AsyncFrameSignal;
	Utils::Signal<Scheduler::Callback> Scheduler::AsyncFrameOnceSignal;

	void Scheduler::Once(Utils::Slot<Scheduler::Callback> callback, bool clientOnly)
	{
		if (clientOnly && (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())) return;
		Scheduler::FrameOnceSignal.connect(callback);
	}

	void Scheduler::OnShutdown(Utils::Slot<Scheduler::Callback> callback)
	{
		Scheduler::ShutdownSignal.connect(callback);
	}

	void Scheduler::OnFrame(Utils::Slot<Scheduler::Callback> callback, bool clientOnly)
	{
		if (clientOnly && (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())) return;
		Scheduler::FrameSignal.connect(callback);
	}

	void Scheduler::OnReady(Utils::Slot<Scheduler::Callback> callback, bool clientOnly)
	{
		if (clientOnly && (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())) return;
		if (Scheduler::ReadyPassed) Scheduler::Once(callback);
		else Scheduler::ReadySignal.connect(callback);
	}

	void Scheduler::ReadyHandler()
	{
		if (!FastFiles::Ready())
		{
			Scheduler::Once(Scheduler::ReadyHandler);
		}
		else
		{
			Scheduler::ReadyPassed = true;
			Scheduler::ReadySignal();
			Scheduler::ReadySignal.clear();
		}
	}

	void Scheduler::FrameHandler()
	{
		Scheduler::DelaySignal();
		Scheduler::FrameSignal();

		Utils::Signal<Scheduler::Callback> copy(Scheduler::FrameOnceSignal);
		Scheduler::FrameOnceSignal.clear();
		copy();
	}

	void Scheduler::OnDelay(Utils::Slot<Scheduler::Callback> callback, std::chrono::nanoseconds delay, bool clientOnly)
	{
		if (clientOnly && (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())) return;

		Scheduler::DelayedSlot slot;
		slot.callback = callback;
		slot.delay = delay;

		Scheduler::DelayedSlots.push_back(slot);
	}

	void Scheduler::DelaySignal()
	{
		Utils::Signal<Scheduler::Callback> signal;

		for (auto i = Scheduler::DelayedSlots.begin(); i != Scheduler::DelayedSlots.end();)
		{
			if (i->interval.elapsed(i->delay))
			{
				signal.connect(i->callback);
				i = Scheduler::DelayedSlots.erase(i);
				continue;
			}

			++i;
		}

		signal();
	}

	void Scheduler::ShutdownStub(int num)
	{
		Scheduler::ShutdownSignal();
		Utils::Hook::Call<void(int)>(0x46B370)(num);
	}

	void Scheduler::OnFrameAsync(Utils::Slot<Scheduler::Callback> callback)
	{
		Scheduler::AsyncFrameSignal.connect(callback);
	}

	void Scheduler::OnceAsync(Utils::Slot<Scheduler::Callback> callback)
	{
		Scheduler::AsyncFrameOnceSignal.connect(callback);
	}

	Scheduler::Scheduler()
	{
		Scheduler::ReadyPassed = false;
		Scheduler::Once(Scheduler::ReadyHandler);

		Utils::Hook(0x4D697A, Scheduler::ShutdownStub, HOOK_CALL).install()->quick();

		if (!Loader::PerformUnitTests())
		{
			Scheduler::AsyncTerminate = false;
			Scheduler::AsyncThread = std::thread([]()
			{
				while (!Scheduler::AsyncTerminate)
				{
					Scheduler::AsyncFrameSignal();

					Utils::Signal<Scheduler::Callback> copy(Scheduler::AsyncFrameOnceSignal);
					Scheduler::AsyncFrameOnceSignal.clear();
					copy();

					std::this_thread::sleep_for(16ms);
				}
			});
		}
	}

	Scheduler::~Scheduler()
	{
		Scheduler::ReadySignal.clear();
		Scheduler::ShutdownSignal.clear();

		Scheduler::FrameSignal.clear();
		Scheduler::FrameOnceSignal.clear();
		Scheduler::DelayedSlots.clear();

		Scheduler::AsyncFrameSignal.clear();
		Scheduler::AsyncFrameOnceSignal.clear();

		Scheduler::ReadyPassed = false;
	}

	void Scheduler::preDestroy()
	{
		Scheduler::AsyncTerminate = true;
		if (Scheduler::AsyncThread.joinable())
		{
			Scheduler::AsyncThread.join();
		}
	}
}
