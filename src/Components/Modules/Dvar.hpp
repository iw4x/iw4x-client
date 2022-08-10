#pragma once

namespace Components
{
	class Dvar : public Component
	{
	public:
		class Flag
		{
		public:
			Flag(Game::DvarFlags flag) : val(flag) {}
			Flag(unsigned __int16 flag) : Flag(static_cast<Game::DvarFlags>(flag)) {}

			Game::DvarFlags val;
		};

		class Var
		{
		public:
			Var() : dvar_(nullptr) {}
			Var(const Var& obj) { this->dvar_ = obj.dvar_; }
			Var(Game::dvar_t* dvar) : dvar_(dvar) {}
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
			Game::dvar_t* dvar_;
		};

		Dvar();
		~Dvar();

		// Only strings and bools use this type of declaration
		template<typename T> static Var Register(const char* dvarName, T value, Flag flag, const char* description);
		template<typename T> static Var Register(const char* dvarName, T value, T min, T max, Flag flag, const char* description);

		static void ResetDvarsValue();

	private:
		static const char* ArchiveDvarPath;

		static Game::dvar_t* Dvar_RegisterName(const char* name, const char* defaultVal, unsigned __int16 flags, const char* description);

		static void SetFromStringByNameExternal(const char* dvar, const char* value);
		static void SetFromStringByNameSafeExternal(const char* dvar, const char* value);

		static bool AreArchiveDvarsProtected();
		static void SaveArchiveDvar(const Game::dvar_t* var);
		static void DvarSetFromStringByNameStub(const char* dvarName, const char* value);

		static void OnRegisterVariant(Game::dvar_t* dvar);
		static void Dvar_RegisterVariant_Stub();
	};
}
