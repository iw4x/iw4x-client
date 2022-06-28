#pragma once

namespace Utils
{
	class NamedMutex
	{
	public:
		explicit NamedMutex(const std::string& name);
		~NamedMutex();

		NamedMutex(NamedMutex&&) = delete;
		NamedMutex(const NamedMutex&) = delete;
		NamedMutex& operator=(NamedMutex&&) = delete;
		NamedMutex& operator=(const NamedMutex&) = delete;

		void lock() const;
		// Lockable requirements
		[[nodiscard]] bool try_lock(std::chrono::milliseconds timeout = std::chrono::milliseconds{0}) const;
		void unlock() const noexcept;

	private:
		void* handle_{};
	};
}
