#pragma once

namespace Utils
{
	class Library
	{
	public:
		static Library load(const std::string& name);
		static Library load(const std::filesystem::path& path);
		static Library get_by_address(void* address);

		Library() : _module(nullptr), freeOnDestroy(false) {};
		Library(const std::string& buffer, bool freeOnDestroy);
		explicit Library(const std::string& name);
		explicit Library(HMODULE handle);
		~Library();

		bool is_valid() const;
		HMODULE getModule();

		template <typename T>
		T get_proc(const std::string& process) const
		{
			if (!this->is_valid()) T{};
			return reinterpret_cast<T>(GetProcAddress(this->_module, process.data()));
		}

		template <typename T>
		std::function<T> get(const std::string& process) const
		{
			if (!this->is_valid()) return std::function<T>();
			return static_cast<T*>(this->get_proc<void*>(process));
		}

		template <typename T, typename... Args>
		T invoke(const std::string& process, Args ... args) const
		{
			auto method = this->get<T(__cdecl)(Args ...)>(process);
			if (method) return method(args...);
			return T();
		}

		template <typename T, typename... Args>
		T invoke_pascal(const std::string& process, Args ... args) const
		{
			auto method = this->get<T(__stdcall)(Args ...)>(process);
			if (method) return method(args...);
			return T();
		}

		template <typename T, typename... Args>
		T invoke_this(const std::string& process, void* this_ptr, Args ... args) const
		{
			auto method = this->get<T(__thiscall)(void*, Args ...)>(this_ptr, process);
			if (method) return method(args...);
			return T();
		}

		void free();

	private:
		HMODULE _module;
		bool freeOnDestroy;
	};
}
