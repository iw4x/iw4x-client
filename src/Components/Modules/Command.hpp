namespace Components
{
	class Command : public Component
	{
	public:
		class Params
		{
		public:
			Params(bool sv, DWORD id) : CommandId(id), IsSV(sv) {};
			Params(bool sv) : Params(sv, (sv ? *Game::cmd_id_sv : *Game::cmd_id)) {};
			Params(const Params &obj) : CommandId(obj.CommandId), IsSV(obj.IsSV) {};
			Params() : Params(false, *Game::cmd_id) {};

			char* operator[](size_t index);
			size_t Length();

			std::string Join(size_t startIndex);

		private:
			bool IsSV;
			DWORD CommandId;
		};

		typedef void(Callback)(Command::Params params);

		Command();
		~Command();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* GetName() { return "Command"; };
#endif

		static Game::cmd_function_t* Allocate();

		static void Add(const char* name, Callback* callback);
		static void AddSV(const char* name, Callback* callback);
		static void AddRaw(const char* name, void(*callback)(), bool key = false);
		static void AddRawSV(const char* name, void(*callback)());
		static void Execute(std::string command, bool sync = true);

		static Game::cmd_function_t* Find(std::string command);

	private:
		static Utils::Memory::Allocator MemAllocator;
		static std::map<std::string, wink::slot<Callback>> FunctionMap;
		static std::map<std::string, wink::slot<Callback>> FunctionMapSV;

		static void MainCallback();
		static void MainCallbackSV();
	};
}
