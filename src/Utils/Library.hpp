#pragma once

namespace Utils
{
  // Pointer to a function, the type of an exported procedure.
  //
  template <typename T>
  concept FunctionPointer = std::is_pointer_v<T> &&
                            std::is_function_v<std::remove_pointer_t<T>>;

  // Return the procedure that the module exports under the name as the
  // function pointer type F, or nullptr if there is no such export.
  //
  // Note that GetProcAddress() returns FARPROC, which GCC considers
  // incompatible with most function pointer types (-Wcast-function-type).
  // So we cast through void (*) (), which it treats as a generic function
  // pointer type.
  //
  template <FunctionPointer F>
  [[nodiscard]] F GetProc(HMODULE module, const char* name)
  {
    return reinterpret_cast<F>(reinterpret_cast<void (*)()>(GetProcAddress(module, name)));
  }

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

    template <FunctionPointer T>
    [[nodiscard]] T getProc(const std::string& process) const
    {
      if (!this->isValid()) return T{};
      return GetProc<T>(this->module_, process.data());
    }

    template <typename T>
    [[nodiscard]] std::function<T> get(const std::string& process) const
    {
      if (!this->isValid()) return std::function<T>();
      return this->getProc<T*>(process);
    }

    template <typename T, typename... Args>
    T invoke(const std::string& process, Args ... args) const
    {
      auto method = this->getProc<T(__cdecl*)(Args ...)>(process);
      if (method) return method(args...);
      return T();
    }

    template <typename T, typename... Args>
    T invokePascal(const std::string& process, Args ... args) const
    {
      auto method = this->getProc<T(__stdcall*)(Args ...)>(process);
      if (method) return method(args...);
      return T();
    }

    template <typename T, typename... Args>
    T invokeThis(const std::string& process, void* this_ptr, Args ... args) const
    {
      auto method = this->getProc<T(__thiscall*)(void*, Args ...)>(process);
      if (method) return method(this_ptr, args...);
      return T();
    }

    static void LaunchProcess(const std::wstring& process, const std::wstring& commandLine, const std::filesystem::path& currentDir);

  private:
    HMODULE module_;
    bool freeOnDestroy_;
  };
}
