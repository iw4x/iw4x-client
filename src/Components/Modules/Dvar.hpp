#pragma once

namespace Components
{
	class Dvar : public Component
	{
	public:
		class Flag
		{
		public:
			Flag(Game::dvar_flag flag) : val(flag) {};
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
			Var(const std::string& dvarName);

			template<typename T> T get();

			void set(char* string);
			void set(const char* string);
			void set(const std::string& string);

			void set(int integer);
			void set(float value);
			void set(bool enabled);

			void setRaw(int integer);
			void setRaw(float value);
			void setRaw(bool enabled);

		private:
			Game::dvar_t* dvar;
		};

		Dvar();
		~Dvar();

		static void OnInit(Utils::Slot<Scheduler::Callback> callback);

		// Only strings and bools use this type of declaration
		template<typename T> static Var Register(const char* name, T value, Flag flag, const char* description);
		template<typename T> static Var Register(const char* name, T value, T min, T max, Flag flag, const char* description);

		static void ResetDvarsValue();

	private:
		static Utils::Signal<Scheduler::Callback> RegistrationSignal;

		static Game::dvar_t* RegisterName(const char* name, const char* defaultVal, Game::dvar_flag flag, const char* description);

		static Game::dvar_t* SetFromStringByNameExternal(const char* dvar, const char* value);
		static Game::dvar_t* SetFromStringByNameSafeExternal(const char* dvar, const char* value);

		static std::vector<std::string> ChangedDvars;
	};
}
