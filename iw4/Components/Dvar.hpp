namespace Components
{
	class Dvar : public Component
	{
	public:
		struct Flag
		{
			Flag(Game::dvar_flag flag) : val(flag){};
			Flag(int flag) : Flag((Game::dvar_flag)flag) {};

			Game::dvar_flag val;
		};

		class Var
		{
		public:
			Var() : dvar(0) {};
			Var(const Var &obj) { this->dvar = obj.dvar; };
			Var(Game::dvar_t* _dvar) : dvar(_dvar) {};
			Var(std::string dvarName);
			Var(std::string dvarName, std::string value);

			template<typename T> T Get();

			void Set(char* string);
			void Set(const char* string);
			void Set(std::string string);

			void Set(int integer);
			void Set(float value);

			// TODO: Add others
			void SetRaw(int integer);

		private:
			Game::dvar_t* dvar;
		};

		Dvar();
		const char* GetName() { return "Dvar"; };

		// Only strings and bools use this type of declaration
		template<typename T> static Var Register(const char* name, T value, Flag flag, const char* description);
		template<typename T> static Var Register(const char* name, T value, T min, T max, Flag flag, const char* description);

	private:
		static Game::dvar_t* RegisterName(const char* name, const char* default, Game::dvar_flag flag, const char* description);
	};
}
