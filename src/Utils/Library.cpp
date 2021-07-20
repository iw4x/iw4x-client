#include "STDInclude.hpp"

namespace Utils
{
	Library Library::load(const std::string& name)
	{
		return Library(LoadLibraryA(name.data()));
	}

	Library Library::load(const std::filesystem::path& path)
	{
		return Library::load(path.generic_string());
	}

	Library Library::get_by_address(void* address)
	{
		HMODULE handle = nullptr;
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, static_cast<LPCSTR>(address), &handle);
		return Library(handle);
	}

	Library::Library(const std::string& buffer, bool _freeOnDestroy) : _module(nullptr), freeOnDestroy(_freeOnDestroy)
	{
		this->_module = LoadLibraryExA(buffer.data(), nullptr, 0);
	}

	Library::Library(const std::string& buffer)
	{
		this->_module = GetModuleHandleA(buffer.data());
		this->freeOnDestroy = true;
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

	bool Library::is_valid() const
	{
		return this->_module != nullptr;
	}

	HMODULE Library::getModule()
	{
		return this->_module;
	}

	void Library::free()
	{
		if (this->is_valid())
		{
			FreeLibrary(this->_module);
		}

		this->_module = nullptr;
	}
}
