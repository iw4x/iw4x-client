#pragma once

namespace Utils
{
	class Library
	{
	public:
		static Library Load(const std::string& name);
		static Library Load(const std::filesystem::path& path);
		static Library GetByAddress(void* address);

		Library() : _module(nullptr), freeOnDestroy(false) {};
		Library(const std::string& buffer, bool freeOnDestroy);
		explicit Library(const std::string& name);
		explicit Library(HMODULE handle);
		~Library();

		bool IsValid() const;
		HMODULE GetModule();

		template <typename T>
		T GetProc(const std::string& process) const
		{
			if (!this->IsValid()) T{};
			return reinterpret_cast<T>(GetProcAddress(this->_module, process.data()));
		}

		template <typename T>
		std::function<T> Get(const std::string& process) const
		{
			if (!this->IsValid()) return std::function<T>();
			return static_cast<T*>(this->GetProc<void*>(process));
		}

		template <typename T, typename... Args>
		T Invoke(const std::string& process, Args ... args) const
		{
			auto method = this->Get<T(__cdecl)(Args ...)>(process);
			if (method) return method(args...);
			return T();
		}

		template <typename T, typename... Args>
		T InvokePascal(const std::string& process, Args ... args) const
		{
			auto method = this->Get<T(__stdcall)(Args ...)>(process);
			if (method) return method(args...);
			return T();
		}

		template <typename T, typename... Args>
		T InvokeThis(const std::string& process, void* this_ptr, Args ... args) const
		{
			auto method = this->Get<T(__thiscall)(void*, Args ...)>(this_ptr, process);
			if (method) return method(args...);
			return T();
		}

		void Free();

	private:
		HMODULE _module;
		bool freeOnDestroy;
	};
}
