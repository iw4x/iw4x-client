#pragma once

namespace Utils
{
	class Library
	{
	public:
		static Library Load(const std::filesystem::path& path);
		static Library GetByAddress(void* address);

		Library() : module_(nullptr), freeOnDestroy_(false) {}
		Library(const std::string& name, bool freeOnDestroy);
		explicit Library(const std::string& name) : module_(GetModuleHandleA(name.data())), freeOnDestroy_(true) {}
		explicit Library(HMODULE handle);
		~Library();

		bool operator!=(const Library& obj) const { return !(*this == obj); }
		bool operator==(const Library& obj) const;

		operator bool() const;
		operator HMODULE() const;

		[[nodiscard]] bool isValid() const;
		[[nodiscard]] HMODULE getModule() const;
		[[nodiscard]] std::string getName() const;
		[[nodiscard]] std::filesystem::path getPath() const;
		[[nodiscard]] std::filesystem::path getFolder() const;
		void free();

		template <typename T>
		[[nodiscard]] T getProc(const std::string& process) const
		{
			if (!this->isValid()) T{};
			return reinterpret_cast<T>(GetProcAddress(this->module_, process.data()));
		}

		template <typename T>
		[[nodiscard]] std::function<T> get(const std::string& process) const
		{
			if (!this->isValid()) return std::function<T>();
			return static_cast<T*>(this->getProc<void*>(process));
		}

		template <typename T, typename... Args>
		T invoke(const std::string& process, Args ... args) const
		{
			auto method = this->get<T(__cdecl)(Args ...)>(process);
			if (method) return method(args...);
			return T();
		}

		template <typename T, typename... Args>
		T invokePascal(const std::string& process, Args ... args) const
		{
			auto method = this->get<T(__stdcall)(Args ...)>(process);
			if (method) return method(args...);
			return T();
		}

		template <typename T, typename... Args>
		T invokeThis(const std::string& process, void* this_ptr, Args ... args) const
		{
			auto method = this->get<T(__thiscall)(void*, Args ...)>(this_ptr, process);
			if (method) return method(args...);
			return T();
		}

		static void LaunchProcess(const std::string& process, const std::string& commandLine, const std::string& currentDir);

	private:
		HMODULE module_;
		bool freeOnDestroy_;
	};
}
