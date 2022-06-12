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

	Library::Library(const std::string& name, bool _freeOnDestroy) : module_(nullptr), freeOnDestroy(_freeOnDestroy)
	{
		this->module_ = LoadLibraryExA(name.data(), nullptr, 0);
	}

	Library::Library(const HMODULE handle)
	{
		this->module_ = handle;
		this->freeOnDestroy = true;
	}

	Library::~Library()
	{
		if (this->freeOnDestroy)
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

	void Library::free()
	{
		if (this->isValid())
		{
			FreeLibrary(this->module_);
		}

		this->module_ = nullptr;
	}
}
