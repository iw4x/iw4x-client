#pragma once

namespace Utils
{
	std::string GetMimeType(std::string url);
	std::string ParseChallenge(std::string data);
	void OutputDebugLastError();

	bool IsWineEnvironment();

	template <typename T> void Merge(std::vector<T>* target, T* source, size_t length)
	{
		if (source)
		{
			for (size_t i = 0; i < length; ++i)
			{
				target->push_back(source[i]);
			}
		}
	}

	template <typename T> void Merge(std::vector<T>* target, std::vector<T> source)
	{
		for (auto &entry : source)
		{
			target->push_back(entry);
		}
	}

	template <typename T> using Slot = std::function<T>;
	template <typename T>
	class Signal
	{
	public:
		Signal()
		{
			this->slots.clear();
		}

		Signal(Signal& obj) : Signal()
		{
			Utils::Merge(&this->slots, obj.getSlots());
		}

		void connect(Slot<T> slot)
		{
			if (slot)
			{
				this->slots.push_back(slot);
			}
			else
			{
				__debugbreak();
			}
		}

		void clear()
		{
			this->slots.clear();
		}

		std::vector<Slot<T>>& getSlots()
		{
			return this->slots;
		}

		template <class ...Args>
		void operator()(Args&&... args) const
		{
			std::vector<Slot<T>> copiedSlots;
			Utils::Merge(&copiedSlots, this->slots);

			for (auto slot : copiedSlots)
			{
				if (slot)
				{
					slot(std::forward<Args>(args)...);
				}
			}
		}

	private:
		std::vector<Slot<T>> slots;
	};

	// TODO: Replace with std::optional, once C++17 is fully available!
	template <typename T>
	class Value
	{
	public:
		Value() : hasValue(false) {}
		Value(T _value) { this->set(_value); }

		void set(T _value)
		{
			this->value = _value;
			this->hasValue = true;
		}

		bool isValid()
		{
			return this->hasValue;
		}

		T get()
		{
			return this->value;
		}

	private:
		bool hasValue;
		T value;
	};
}
