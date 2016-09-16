namespace Utils
{
	class Library
	{
	public:
		Library() : Module(nullptr), FreeOnDestroy(false) {};
		Library(std::string buffer, bool freeOnDestroy = true);
		~Library();

		bool Valid();
		HMODULE GetModule();

		template <typename T>
		std::function<T> Get(std::string process)
		{
			if (!this->Valid())
			{
				throw new std::runtime_error("Library not loaded!");
			}

			return reinterpret_cast<T*>(GetProcAddress(this->GetModule(), process.data()));
		}

	private:
		HMODULE Module;
		bool FreeOnDestroy;
	};
}
