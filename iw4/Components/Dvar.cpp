#include "..\STDInclude.hpp"

namespace Components
{
	Dvar::Var::Var(std::string dvarName) : Var()
	{
		this->dvar = Game::Dvar_FindVar(dvarName.data());

		if (!this->dvar)
		{
			// Register the dvar?
		}
	}

	template <> char* Dvar::Var::Get()
	{
		if (this->dvar && this->dvar->type == Game::dvar_type::DVAR_TYPE_STRING && this->dvar->current.string)
		{
			return this->dvar->current.string;
		}

		return "";
	}

	template <> const char* Dvar::Var::Get()
	{
		return this->Get<char*>();
	}

	template <> int Dvar::Var::Get()
	{
		if (this->dvar && this->dvar->type == Game::dvar_type::DVAR_TYPE_INT)
		{
			return this->dvar->current.integer;
		}

		return 0;
	}

	template <> float Dvar::Var::Get()
	{
		if (this->dvar && this->dvar->type == Game::dvar_type::DVAR_TYPE_FLOAT)
		{
			return this->dvar->current.value;
		}

		return 0;
	}

	template <> float* Dvar::Var::Get()
	{
		static float val[4] = { 0 };

		if (this->dvar && (this->dvar->type == Game::dvar_type::DVAR_TYPE_FLOAT_2 || this->dvar->type == Game::dvar_type::DVAR_TYPE_FLOAT_3 || this->dvar->type == Game::dvar_type::DVAR_TYPE_FLOAT_4))
		{
			return this->dvar->current.vec4;
		}

		return val;
	}

	template <> bool Dvar::Var::Get()
	{
		if (this->dvar && this->dvar->type == Game::dvar_type::DVAR_TYPE_BOOL)
		{
			return this->dvar->current.boolean;
		}

		return false;
	}

	void Dvar::Var::Set(char* string)
	{
		this->Set(reinterpret_cast<const char*>(string));
	}

	void Dvar::Var::Set(const char* string)
	{
		if (this->dvar && this->dvar->name)
		{
			Game::Dvar_SetCommand(this->dvar->name, string);
		}
	}

	void Dvar::Var::Set(std::string string)
	{
		this->Set(string.data());
	}

	void Dvar::Var::Set(int integer)
	{
		if (this->dvar && this->dvar->name)
		{
			Game::Dvar_SetCommand(this->dvar->name, Utils::VA("%i", integer));
		}
	}

	void Dvar::Var::Set(float value)
	{
		if (this->dvar && this->dvar->name)
		{
			Game::Dvar_SetCommand(this->dvar->name, Utils::VA("%f", value));
		}
	}

	template<> static Dvar::Var Dvar::Var::Register(const char* name, bool value, Game::dvar_flag flag, const char* description)
	{
		return Game::Dvar_RegisterBool(name, value, flag, description);
	}

	template<> static Dvar::Var Dvar::Var::Register(const char* name, const char* value, Game::dvar_flag flag, const char* description)
	{
		return Game::Dvar_RegisterString(name, value, flag, description);
	}

	Dvar::Dvar()
	{
		// set flags of cg_drawFPS to archive
		*(BYTE*)0x4F8F69 |= Game::dvar_flag::DVAR_FLAG_SAVED;

		// un-cheat cg_fov and add archive flags
		*(BYTE*)0x4F8E35 ^= Game::dvar_flag::DVAR_FLAG_CHEAT | Game::dvar_flag::DVAR_FLAG_SAVED;

		// set cg_fov max to 90.0
		static float cgFov90 = 90.0f;
		*(DWORD*)0x4F8E28 = (DWORD)&cgFov90;

		Dvar::Var::Register<bool>("zob", true, Game::dvar_flag::DVAR_FLAG_NONE, "test dvar");
		//Dvar::Var::Register<const char*>("zob2", "test", Game::dvar_flag::DVAR_FLAG_NONE, "test dvar3");
	}
}
