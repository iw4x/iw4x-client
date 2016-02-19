namespace Components
{
	class Command : public Component
	{
	public:
		class Params
		{
		public:
			Params(DWORD id) : CommandId(id) {};
			Params(const Params &obj) { this->CommandId = obj.CommandId; };
			Params() : Params(*Game::cmd_id) {};

			char* operator[](size_t index);
			size_t Length();

			std::string Join(size_t startIndex);

		private:
			DWORD CommandId;
		};

		typedef void(Callback)(Command::Params params);

		Command();
		~Command();
		const char* GetName() { return "Command"; };

		static void Add(const char* name, Callback* callback);
		static void Execute(std::string command, bool sync = true);

	private:
		static Game::cmd_function_t* Allocate();
		static std::vector<Game::cmd_function_t*> Functions;
		static std::map<std::string, wink::slot<Callback>> FunctionMap;
		static void MainCallback();
	};
}
