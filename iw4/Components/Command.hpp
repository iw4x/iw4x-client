#define Q_IsColorString( p )  ( ( p ) && *( p ) == '^' && *( ( p ) + 1 ) && isdigit( *( ( p ) + 1 ) ) ) // ^[0-9]


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

			const char* operator[](size_t index);
			size_t Length();

		private:
			DWORD CommandId;
		};

		typedef void(*Callback)(Command::Params params);

		Command();
		~Command();
		const char* GetName() { return "Command"; };

		static void Add(const char* name, Callback callback);
		static int ArgCount();

	private:
		static Game::cmd_function_t* Allocate();
		static std::vector<Game::cmd_function_t*> Functions;
		static std::map<std::string, Callback> FunctionMap;
		static void MainCallback();
	};
}
