#pragma once

namespace Components
{
	class Dvar : public Component
	{
	public:
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

		// Only strings and bools use this type of declaration
		template<typename T> static Var Register(const char* dvarName, T value, std::uint16_t flag, const char* description);
		template<typename T> static Var Register(const char* dvarName, T value, T min, T max, std::uint16_t flag, const char* description);

		static Var Name;

	private:

		static const Game::dvar_t* Dvar_RegisterName(const char* dvarName, const char* value, std::uint16_t flags, const char* description);
		static const Game::dvar_t* Dvar_RegisterSVNetworkFps(const char* dvarName, int value, int min, int max, std::uint16_t flags, const char* description);
		static const Game::dvar_t* Dvar_RegisterPerkExtendedMeleeRange(const char* dvarName, float value, float min, float max, std::uint16_t flags, const char* description);

		static void SetFromStringByNameExternal(const char* dvarName, const char* string);
		static void SetFromStringByNameSafeExternal(const char* dvarName, const char* string);

		static bool AreArchiveDvarsUnprotected();
		static bool IsSettingDvarsDisabled();
		static void DvarSetFromStringByName_Stub(const char* dvarName, const char* value);

		static void OnRegisterVariant(Game::dvar_t* dvar);
		static void Dvar_RegisterVariant_Stub();

		static const char* Dvar_EnumToString_Stub(const Game::dvar_t* dvar);
	};
}
