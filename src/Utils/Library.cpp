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

	Library::Library(const std::string& name, bool _freeOnDestroy) : _module(nullptr), freeOnDestroy(_freeOnDestroy)
	{
		this->_module = LoadLibraryExA(name.data(), nullptr, 0);
	}

	Library::Library(const HMODULE handle)
	{
		this->_module = handle;
		this->freeOnDestroy = true;
	}

	Library::~Library()
	{
		if (this->freeOnDestroy)
		{
			this->free();
		}
	}

	bool Library::isValid() const
	{
		return this->_module != nullptr;
	}

	HMODULE Library::getModule() const
	{
		return this->_module;
	}

	void Library::free()
	{
		if (this->isValid())
		{
			FreeLibrary(this->_module);
		}

		this->_module = nullptr;
	}
}
