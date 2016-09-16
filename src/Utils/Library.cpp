#include "STDInclude.hpp"

namespace Utils
{
	Library::Library(std::string buffer, bool freeOnDestroy) : FreeOnDestroy(freeOnDestroy), Module(nullptr)
	{
		this->Module = LoadLibraryExA(buffer.data(), NULL, 0);
	}

	Library::~Library()
	{
		if (this->FreeOnDestroy && this->Valid())
		{
			FreeLibrary(this->GetModule());
		}

		this->Module = nullptr;
	}

	bool Library::Valid()
	{
		return (this->GetModule() != nullptr);
	}

	HMODULE Library::GetModule()
	{
		return this->Module;
	}
}
