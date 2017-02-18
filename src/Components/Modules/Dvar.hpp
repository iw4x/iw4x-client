#pragma once

namespace Components
{
	class Dvar : public Component
	{
	public:
		typedef void(Callback)();

		class Flag
		{
		public:
			Flag(Game::dvar_flag flag) : val(flag){};
			Flag(int flag) : Flag(static_cast<Game::dvar_flag>(flag)) {};

			Game::dvar_flag val;
		};

		class Var
		{
		public:
			Var() : dvar(nullptr) {};
			Var(const Var &obj) { this->dvar = obj.dvar; };
			Var(Game::dvar_t* _dvar) : dvar(_dvar) {};
			Var(DWORD ppdvar) : Var(*reinterpret_cast<Game::dvar_t**>(ppdvar)) {};
			Var(std::string dvarName);

			template<typename T> T get();

			void set(char* string);
			void set(const char* string);
			void set(std::string string);

			void set(int integer);
			void set(float value);

			// TODO: Add others
			void setRaw(int integer);

		private:
			Game::dvar_t* dvar;
		};

		Dvar();
		~Dvar();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "Dvar"; };
#endif

		static void OnInit(Utils::Slot<Callback> callback);

		// Only strings and bools use this type of declaration
		template<typename T> static Var Register(const char* name, T value, Flag flag, const char* description);
		template<typename T> static Var Register(const char* name, T value, T min, T max, Flag flag, const char* description);

	private:
		static Utils::Signal<Callback> RegistrationSignal;

		static Game::dvar_t* RegisterName(const char* name, const char* defaultVal, Game::dvar_flag flag, const char* description);

		static Game::dvar_t* SetFromStringByNameExternal(const char* dvar, const char* value);
	};
}
