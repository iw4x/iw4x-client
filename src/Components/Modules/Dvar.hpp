#pragma once

namespace Components
{
	class Dvar : public Component
	{
	public:
		class Flag
		{
		public:
			Flag(Game::dvar_flag flag) : val(flag) {}
			Flag(unsigned __int16 flag) : Flag(static_cast<Game::dvar_flag>(flag)) {}

			Game::dvar_flag val;
		};

		class Var
		{
		public:
			Var() : dvar(nullptr) {}
			Var(const Var& obj) { this->dvar = obj.dvar; }
			Var(Game::dvar_t* _dvar) : dvar(_dvar) {}
			Var(DWORD ppdvar) : Var(*reinterpret_cast<Game::dvar_t**>(ppdvar)) {}
			Var(const std::string& dvarName);

			template<typename T> T get();

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

		typedef void(Callback)();

		static void OnInit(Utils::Slot<Dvar::Callback> callback);

		// Only strings and bools use this type of declaration
		template<typename T> static Var Register(const char* dvarName, T value, Flag flag, const char* description);
		template<typename T> static Var Register(const char* dvarName, T value, T min, T max, Flag flag, const char* description);

		static void ResetDvarsValue();

	private:
		static Utils::Signal<Dvar::Callback> RegistrationSignal;
		static const char* ArchiveDvarPath;

		static Game::dvar_t* Dvar_RegisterName(const char* name, const char* defaultVal, unsigned __int16 flags, const char* description);

		static void SetFromStringByNameExternal(const char* dvar, const char* value);
		static void SetFromStringByNameSafeExternal(const char* dvar, const char* value);

		static void SaveArchiveDvar(const Game::dvar_t* var);
		static void DvarSetFromStringByNameStub(const char* dvarName, const char* value);

		static void CL_InitOnceForAllClients_Hk();
	};
}
