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
		void connect(Slot<T> slot)
		{
			slots.push_back(slot);
		}

		void clear()
		{
			slots.clear();
		}

		template <class ...Args>
		void operator()(Args&&... args) const
		{
			for (auto& slot : slots)
			{
				slot(std::forward<Args>(args)...);
			}
		}

	private:
		std::vector<Slot<T>> slots;
	};

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
