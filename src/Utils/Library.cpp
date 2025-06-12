#include "Library.hpp"

namespace Utils
{
	Library Library::Load(const std::filesystem::path& path)
	{
		return Library(LoadLibraryA(path.generic_string().data()));
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

		const auto path = this->getPath();
		const auto generic_path = path.generic_string();
		const auto pos = generic_path.find_last_of("/\\");
		if (pos == std::string::npos)
		{
			return generic_path;
		}

		return generic_path.substr(pos + 1);
	}

	std::filesystem::path Library::getPath() const
	{
		if (!this->isValid())
			return {};

		wchar_t name[MAX_PATH] = {0};
		GetModuleFileNameW(this->module_, name, MAX_PATH);

		return {name};
	}

	std::filesystem::path Library::getFolder() const
	{
		if (!this->isValid())
			return {};

		const auto path = this->getPath();
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

	void Library::LaunchProcess(const std::wstring& process, const std::wstring& commandLine, const std::filesystem::path& currentDir)
	{
		STARTUPINFOW startupInfo;
		PROCESS_INFORMATION processInfo;

		ZeroMemory(&startupInfo, sizeof(startupInfo));
		ZeroMemory(&processInfo, sizeof(processInfo));
		startupInfo.cb = sizeof(startupInfo);

		CreateProcessW(process.data(), const_cast<wchar_t*>(commandLine.data()), nullptr,
			nullptr, false, NULL, nullptr, currentDir.wstring().data(),
			&startupInfo, &processInfo);

		if (processInfo.hThread && processInfo.hThread != INVALID_HANDLE_VALUE)
			CloseHandle(processInfo.hThread);
		if (processInfo.hProcess && processInfo.hProcess != INVALID_HANDLE_VALUE)
			CloseHandle(processInfo.hProcess);
	}
}
