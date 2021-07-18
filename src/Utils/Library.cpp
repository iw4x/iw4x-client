#include "STDInclude.hpp"

namespace Utils
{
	Library::Library(const std::string& buffer, bool _freeOnDestroy) : _module(nullptr), freeOnDestroy(_freeOnDestroy)
	{
		this->_module = LoadLibraryExA(buffer.data(), nullptr, 0);
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
			FreeLibrary(this->getModule());
		}

		this->_module = nullptr;
	}
}
