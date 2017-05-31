#include "STDInclude.hpp"

namespace Components
{
	bool Scheduler::ReadyPassed = false;
	Utils::Signal<Scheduler::Callback> Scheduler::ReadySignal;
	Utils::Signal<Scheduler::Callback> Scheduler::ShutdownSignal;

	Utils::Signal<Scheduler::Callback> Scheduler::FrameSignal;
	Utils::Signal<Scheduler::Callback> Scheduler::FrameOnceSignal;
	std::vector<Scheduler::DelayedSlot> Scheduler::DelayedSlots;

	void Scheduler::Once(Utils::Slot<Scheduler::Callback> callback)
	{
		Scheduler::FrameOnceSignal.connect(callback);
	}

	void Scheduler::OnFrame(Utils::Slot<Scheduler::Callback> callback)
	{
		Scheduler::FrameSignal.connect(callback);
	}

	void Scheduler::ReadyHandler()
	{
		if (!FastFiles::Ready()) Scheduler::Once(Scheduler::ReadyHandler);
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

	void Scheduler::OnDelay(Utils::Slot<Scheduler::Callback> callback, std::chrono::nanoseconds delay)
	{
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

	Scheduler::Scheduler()
	{
		Scheduler::ReadyPassed = false;
		Scheduler::Once(Scheduler::ReadyHandler);
	}

	Scheduler::~Scheduler()
	{
		Scheduler::FrameOnceSignal.clear();
		Scheduler::FrameSignal.clear();
	}
}
