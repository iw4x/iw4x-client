namespace Components
{
	class Dvar : public Component
	{
	public:
		class Var
		{
		public:
			Var() : dvar(0) {};
			Var(const Var &obj) { this->dvar = obj.dvar; };
			Var(Game::dvar_t* _dvar) : dvar(_dvar) {};
			Var(std::string dvarName);

			template<typename T> T Get();

			void Set(char* string);
			void Set(const char* string);
			void Set(std::string string);

			void Set(int integer);
			void Set(float value);

			// Only strings and bools use this type of declaration
			template<typename T> static Var Register(const char* name, T value, Game::dvar_flag flag, const char* description);

		private:
			Game::dvar_t* dvar;
		};

		Dvar();
		const char* GetName() { return "Dvar"; };

	private:
		static Game::dvar_t* RegisterName(const char* name, const char* default, Game::dvar_flag flag, const char* description);
	};
}
