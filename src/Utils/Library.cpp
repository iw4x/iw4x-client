#include "STDInclude.hpp"

namespace Utils
{
	Library::Library(std::string buffer, bool _freeOnDestroy) : freeOnDestroy(_freeOnDestroy), module(nullptr)
	{
		this->module = LoadLibraryExA(buffer.data(), NULL, 0);
	}

	Library::~Library()
	{
		if (this->freeOnDestroy && this->valid())
		{
			FreeLibrary(this->getModule());
		}

		this->module = nullptr;
	}

	bool Library::valid()
	{
		return (this->getModule() != nullptr);
	}

	HMODULE Library::getModule()
	{
		return this->module;
	}
}
