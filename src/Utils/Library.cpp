#include <STDInclude.hpp>

namespace Utils
{
	Library Library::Load(const std::string& name)
	{
		return Library(LoadLibraryA(name.data()));
	}

	Library Library::Load(const std::filesystem::path& path)
	{
		return Library::Load(path.generic_string());
	}

	Library Library::GetByAddress(void* address)
	{
		HMODULE handle = nullptr;
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, static_cast<LPCSTR>(address), &handle);
		return Library(handle);
	}

	Library::Library(const std::string& name, bool freeOnDestroy) : module_(nullptr), freeOnDestroy_(freeOnDestroy)
	{
		this->module_ = LoadLibraryExA(name.data(), nullptr, 0);
	}

	Library::Library(const HMODULE handle) : module_(handle), freeOnDestroy_(true)
	{
	}

	Library::~Library()
	{
		if (this->freeOnDestroy_)
		{
			this->free();
		}
	}

	bool Library::operator==(const Library& obj) const
	{
		return this->module_ == obj.module_;
	}

	Library::operator bool() const
	{
		return this->isValid();
	}

	Library::operator HMODULE() const
	{
		return this->getModule();
	}

	bool Library::isValid() const
	{
		return this->module_ != nullptr;
	}

	HMODULE Library::getModule() const
	{
		return this->module_;
	}

	std::string Library::getName() const
	{
		if (!this->isValid())
			return {};

		auto path = this->getPath();
		const auto pos = path.find_last_of("/\\");
		if (pos == std::string::npos)
			return path;

		return path.substr(pos + 1);
	}

	std::string Library::getPath() const
	{
		if (!this->isValid())
			return {};

		char name[MAX_PATH] = {0};
		GetModuleFileNameA(this->module_, name, sizeof(name));

		return name;
	}

	std::string Library::getFolder() const
	{
		if (!this->isValid())
			return {};

		const auto path = std::filesystem::path(this->getPath());
		return path.parent_path().generic_string();
	}

	void Library::free()
	{
		if (this->isValid())
		{
			FreeLibrary(this->module_);
		}

		this->module_ = nullptr;
	}

	void Library::LaunchProcess(const std::string& process, const std::string& commandLine, const std::string& currentDir)
	{
		STARTUPINFOA startup_info;
		PROCESS_INFORMATION process_info;

		ZeroMemory(&startup_info, sizeof(startup_info));
		ZeroMemory(&process_info, sizeof(process_info));
		startup_info.cb = sizeof(startup_info);

		CreateProcessA(process.data(), const_cast<char*>(commandLine.data()), nullptr,
			nullptr, false, NULL, nullptr, currentDir.data(),
			&startup_info, &process_info);

		if (process_info.hThread && process_info.hThread != INVALID_HANDLE_VALUE)
			CloseHandle(process_info.hThread);
		if (process_info.hProcess && process_info.hProcess != INVALID_HANDLE_VALUE)
			CloseHandle(process_info.hProcess);
	}
}
