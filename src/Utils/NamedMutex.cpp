#include <STDInclude.hpp>

namespace Utils
{
	NamedMutex::NamedMutex(const std::string& name)
	{
		this->handle_ = CreateMutexA(nullptr, FALSE, name.data());
	}

	NamedMutex::~NamedMutex()
	{
		if (this->handle_)
		{
			CloseHandle(this->handle_);
		}
	}

	void NamedMutex::lock() const
	{
		if (this->handle_)
		{
			WaitForSingleObject(this->handle_, INFINITE);
		}
	}

	bool NamedMutex::try_lock(const std::chrono::milliseconds timeout) const
	{
		if (this->handle_)
		{
			return WAIT_OBJECT_0 == WaitForSingleObject(this->handle_, static_cast<DWORD>(timeout.count()));
		}

		return false;
	}

	void NamedMutex::unlock() const noexcept
	{
		if (this->handle_)
		{
			ReleaseMutex(this->handle_);
		}
	}
}
