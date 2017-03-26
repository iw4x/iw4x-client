#pragma once

namespace Utils
{
	class Library
	{
	public:
		Library() : module(nullptr), freeOnDestroy(false) {};
		Library(std::string buffer, bool freeOnDestroy = true);
		~Library();

		bool valid();
		HMODULE getModule();

		template <typename T>
		std::function<T> get(std::string process)
		{
			if (!this->valid())
			{
				throw std::runtime_error("Library not loaded!");
			}

			return reinterpret_cast<T*>(GetProcAddress(this->getModule(), process.data()));
		}

		void free();

	private:
		HMODULE module;
		bool freeOnDestroy;
	};
}
