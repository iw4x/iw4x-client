#pragma once

typedef LONG NTSTATUS;
typedef NTSTATUS(NTAPI *NtCreateThreadEx_t)(PHANDLE hThread, ACCESS_MASK DesiredAccess, LPVOID ObjectAttributes, HANDLE ProcessHandle, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, BOOL CreateSuspended, DWORD StackZeroBits, DWORD SizeOfStackCommit, DWORD SizeOfStackReserve, LPVOID lpBytesBuffer);
typedef NTSTATUS(NTAPI* NtQueryInformationThread_t)(HANDLE ThreadHandle, LONG ThreadInformationClass, PVOID ThreadInformation, ULONG ThreadInformationLength, PULONG ReturnLength);
#define ThreadQuerySetWin32StartAddress 9

namespace Utils
{
	std::string GetMimeType(const std::string& url);
	std::string ParseChallenge(const std::string& data);
	void OutputDebugLastError();
	std::string GetLastWindowsError();

	bool IsWineEnvironment();

	unsigned long GetParentProcessId();
	std::size_t GetModuleSize(HMODULE);
	void* GetThreadStartAddress(HANDLE hThread);
	HMODULE GetNTDLL();

	void SetEnvironment();

	void OpenUrl(const std::string& url);

	bool HasIntersection(unsigned int base1, unsigned int len1, unsigned int base2, unsigned int len2);

	template <typename T>
	void RotLeft(T& object, std::size_t bits)
	{
		bits %= sizeof(T) * 8;

		T sign = 1;
		sign = sign << (sizeof(T) * 8 - 1);

		bool negative = (object & sign) != 0;
		object &= ~sign;
		object = (object << bits) | (object >> (sizeof(T) * 8 - bits));
		object |= T(negative) << ((sizeof(T) * 8 - 1 + bits) % (sizeof(T) * 8));
	}

	template <typename T>
	void RotRight(T& object, std::size_t bits)
	{
		bits %= (sizeof(T) * 8);
		RotLeft<T>(object, ((sizeof(T) * 8) - bits));
	}

	template <typename T>
	void Merge(std::vector<T>* target, T* source, std::size_t length)
	{
		if (source)
		{
			for (std::size_t i = 0; i < length; ++i)
			{
				target->push_back(source[i]);
			}
		}
	}

	template <typename T>
	void Merge(std::vector<T>* target, std::vector<T> source)
	{
		for (auto& entry : source)
		{
			target->push_back(entry);
		}
	}

	template <typename T> using Slot = std::function<T>;
	template <typename T>
	class Signal
	{
	public:
		Signal() = default;

		Signal(Signal& obj) : Signal()
		{
			std::lock_guard<std::recursive_mutex> _(this->mutex);
			std::lock_guard<std::recursive_mutex> __(obj.mutex);

			Utils::Merge(&this->slots, obj.getSlots());
		}

		void connect(const Slot<T> slot)
		{
			std::lock_guard<std::recursive_mutex> _(this->mutex);

			if (slot)
			{
				this->slots.emplace_back(slot);
			}
		}

		void clear()
		{
			std::lock_guard<std::recursive_mutex> _(this->mutex);

			this->slots.clear();
		}

		std::vector<Slot<T>>& getSlots()
		{
			return this->slots;
		}

		template <class ...Args>
		void operator()(Args&&... args) const
		{
			std::lock_guard<std::recursive_mutex> _(this->mutex);

			std::vector<Slot<T>> copiedSlots;
			Utils::Merge(&copiedSlots, this->slots);

			for (auto& slot : copiedSlots)
			{
				if (slot)
				{
					slot(std::forward<Args>(args)...);
				}
			}
		}

	private:
		mutable std::recursive_mutex mutex;
		std::vector<Slot<T>> slots;
	};
}
