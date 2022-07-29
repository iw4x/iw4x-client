#include <STDInclude.hpp>

namespace Components
{
	const char* Dvar::ArchiveDvarPath = "userraw/archivedvars.cfg";

	Dvar::Var::Var(const std::string& dvarName) : Var()
	{
		this->dvar = Game::Dvar_FindVar(dvarName.data());

		// If the dvar can't be found it will be registered as an empty string dvar
		if (this->dvar == nullptr)
		{
			this->dvar = const_cast<Game::dvar_t*>(Game::Dvar_SetFromStringByNameFromSource(dvarName.data(), "",
				Game::DvarSetSource::DVAR_SOURCE_INTERNAL));
		}
	}

	template <> Game::dvar_t* Dvar::Var::get()
	{
		return this->dvar;
	}

	template <> const char* Dvar::Var::get()
	{
		if (this->dvar == nullptr)
			return "";

		if (this->dvar->type == Game::dvar_type::DVAR_TYPE_STRING
			|| this->dvar->type == Game::dvar_type::DVAR_TYPE_ENUM)
		{
			if (this->dvar->current.string != nullptr)
				return this->dvar->current.string;
		}

		return "";
	}

	template <> int Dvar::Var::get()
	{
		if (this->dvar == nullptr)
			return 0;

		if (this->dvar->type == Game::dvar_type::DVAR_TYPE_INT || this->dvar->type == Game::dvar_type::DVAR_TYPE_ENUM)
		{
			return this->dvar->current.integer;
		}

		return 0;
	}

	template <> unsigned int Dvar::Var::get()
	{
		if (this->dvar == nullptr)
			return 0;

		if (this->dvar->type == Game::dvar_type::DVAR_TYPE_INT)
		{
			return this->dvar->current.unsignedInt;
		}

		return 0;
	}

	template <> float Dvar::Var::get()
	{
		if (this->dvar == nullptr)
			return 0.f;

		if (this->dvar->type == Game::dvar_type::DVAR_TYPE_FLOAT)
		{
			return this->dvar->current.value;
		}

		return 0.f;
	}

	template <> float* Dvar::Var::get()
	{
		static Game::vec4_t vector{0.f, 0.f, 0.f, 0.f};

		if (this->dvar == nullptr)
			return vector;

		if (this->dvar->type == Game::dvar_type::DVAR_TYPE_FLOAT_2 || this->dvar->type == Game::dvar_type::DVAR_TYPE_FLOAT_3
			|| this->dvar->type == Game::dvar_type::DVAR_TYPE_FLOAT_4)
		{
			return this->dvar->current.vector;
		}

		return vector;
	}

	template <> bool Dvar::Var::get()
	{
		if (this->dvar == nullptr)
			return false;

		if (this->dvar->type == Game::dvar_type::DVAR_TYPE_BOOL)
		{
			return this->dvar->current.enabled;
		}

		return false;
	}

	template <> std::string Dvar::Var::get()
	{
		return this->get<const char*>();
	}

	void Dvar::Var::set(const char* string)
	{
		assert(this->dvar->type == Game::DVAR_TYPE_STRING);
		if (this->dvar)
		{
			Game::Dvar_SetString(this->dvar, string);
		}
	}

	void Dvar::Var::set(const std::string& string)
	{
		this->set(string.data());
	}

	void Dvar::Var::set(int integer)
	{
		assert(this->dvar->type == Game::DVAR_TYPE_INT);
		if (this->dvar)
		{
			Game::Dvar_SetInt(this->dvar, integer);
		}
	}

	void Dvar::Var::set(float value)
	{
		assert(this->dvar->type == Game::DVAR_TYPE_FLOAT);
		if (this->dvar)
		{
			Game::Dvar_SetFloat(this->dvar, value);
		}
	}

	void Dvar::Var::set(bool enabled)
	{
		assert(this->dvar->type == Game::DVAR_TYPE_BOOL);
		if (this->dvar)
		{
			Game::Dvar_SetBool(this->dvar, enabled);
		}
	}

	void Dvar::Var::setRaw(int integer)
	{
		assert(this->dvar->type == Game::DVAR_TYPE_INT);
		if (this->dvar)
		{
			this->dvar->current.integer = integer;
			this->dvar->latched.integer = integer;
		}
	}

	void Dvar::Var::setRaw(float value)
	{
		assert(this->dvar->type == Game::DVAR_TYPE_FLOAT);
		if (this->dvar)
		{
			this->dvar->current.value = value;
			this->dvar->latched.value = value;
		}
	}

	void Dvar::Var::setRaw(bool enabled)
	{
		assert(this->dvar->type == Game::DVAR_TYPE_BOOL);
		if (this->dvar)
		{
			this->dvar->current.enabled = enabled;
			this->dvar->latched.enabled = enabled;
		}
	}

	template<> Dvar::Var Dvar::Register(const char* dvarName, bool value, Dvar::Flag flag, const char* description)
	{
		return Game::Dvar_RegisterBool(dvarName, value, flag.val, description);
	}

	template<> Dvar::Var Dvar::Register(const char* dvarName, const char* value, Dvar::Flag flag, const char* description)
	{
		return Game::Dvar_RegisterString(dvarName, value, flag.val, description);
	}

	template<> Dvar::Var Dvar::Register(const char* dvarName, int value, int min, int max, Dvar::Flag flag, const char* description)
	{
		return Game::Dvar_RegisterInt(dvarName, value, min, max, flag.val, description);
	}

	template<> Dvar::Var Dvar::Register(const char* dvarName, float value, float min, float max, Dvar::Flag flag, const char* description)
	{
		return Game::Dvar_RegisterFloat(dvarName, value, min, max, flag.val, description);
	}

	void Dvar::ResetDvarsValue()
	{
		if (!Utils::IO::FileExists(Dvar::ArchiveDvarPath))
			return;

		Command::Execute("exec archivedvars.cfg", true);
		// Clean up
		Utils::IO::RemoveFile(Dvar::ArchiveDvarPath);
	}

	Game::dvar_t* Dvar::Dvar_RegisterName(const char* name, const char* /*default*/, unsigned __int16 flags, const char* description)
	{
		// Name watcher
		if (!Dedicated::IsEnabled() && !ZoneBuilder::IsEnabled())
		{
			Scheduler::Loop([]
			{
				static std::string lastValidName = "Unknown Soldier";
				auto name = Dvar::Var("name").get<std::string>();

				// Don't perform any checks if name didn't change
				if (name == lastValidName) return;

				std::string saneName = TextRenderer::StripAllTextIcons(TextRenderer::StripColors(Utils::String::Trim(name)));
				if (saneName.size() < 3 || (saneName[0] == '[' && saneName[1] == '{'))
				{
					Logger::Print("Username '{}' is invalid. It must at least be 3 characters long and not appear empty!\n", name);
					Dvar::Var("name").set(lastValidName);
				}
				else
				{
					lastValidName = name;
					Friends::UpdateName();
				}
			}, Scheduler::CLIENT, 3s); // Don't need to do this every frame
		}

		std::string username = "Unknown Soldier";

		if (Steam::Proxy::SteamFriends)
		{
			const char* steamName = Steam::Proxy::SteamFriends->GetPersonaName();

			if (steamName && !std::string(steamName).empty())
			{
				username = steamName;
			}
		}

		return Dvar::Register<const char*>(name, username.data(), flags | Game::DVAR_ARCHIVE, description).get<Game::dvar_t*>();
	}

	void Dvar::SetFromStringByNameSafeExternal(const char* dvarName, const char* string)
	{
		static const char* exceptions[] =
		{
			"ui_showEndOfGame",
			"systemlink",
			"splitscreen",
			"onlinegame",
			"party_maxplayers",
			"xblive_privateserver",
			"xblive_rankedmatch",
			"ui_mptype",
		};

		for (std::size_t i = 0; i < ARRAYSIZE(exceptions); ++i)
		{
			if (Utils::String::ToLower(dvarName) == Utils::String::ToLower(exceptions[i]))
			{
				Game::Dvar_SetFromStringByNameFromSource(dvarName, string, Game::DvarSetSource::DVAR_SOURCE_INTERNAL);
				return;
			}
		}

		Dvar::SetFromStringByNameExternal(dvarName, string);
	}

	void Dvar::SetFromStringByNameExternal(const char* dvarName, const char* string)
	{
		Game::Dvar_SetFromStringByNameFromSource(dvarName, string, Game::DvarSetSource::DVAR_SOURCE_EXTERNAL);
	}

	bool Dvar::AreArchiveDvarsProtected()
	{
		static std::optional<bool> flag;

		if (!flag.has_value())
		{
			flag.emplace(Flags::HasFlag("protect-saved-dvars"));
		}

		return flag.value();
	}

	void Dvar::SaveArchiveDvar(const Game::dvar_t* var)
	{
		if (!Utils::IO::FileExists(Dvar::ArchiveDvarPath))
		{
			Utils::IO::WriteFile(Dvar::ArchiveDvarPath,
				"// generated by IW4x, do not modify\n");
		}

		Utils::IO::WriteFile(Dvar::ArchiveDvarPath,
			Utils::String::VA("seta %s \"%s\"\n", var->name, Game::Dvar_DisplayableValue(var)), true);
	}

	void Dvar::DvarSetFromStringByNameStub(const char* dvarName, const char* value)
	{
		// Save the dvar original value if it has the archive flag
		const auto* dvar = Game::Dvar_FindVar(dvarName);
		if (dvar != nullptr && dvar->flags & Game::DVAR_ARCHIVE)
		{
			if (Dvar::AreArchiveDvarsProtected())
			{
				Logger::Print(Game::CON_CHANNEL_CONSOLEONLY, "Not allowing server to override saved dvar '{}'\n", dvarName);
				return;
			}

#ifdef DEBUG_DVARS
			Logger::Print(Game::CON_CHANNEL_CONSOLEONLY, "Server is overriding saved dvar '{}'\n", dvarName);
#endif
			Dvar::SaveArchiveDvar(dvar);
		}

		Utils::Hook::Call<void(const char*, const char*)>(0x4F52E0)(dvarName, value);
	}

	void Dvar::OnRegisterVariant([[maybe_unused]] Game::dvar_t* dvar)
	{
#ifdef _DEBUG
		dvar->flags &= ~Game::DVAR_CHEAT;
#endif
	}

	__declspec(naked) void Dvar::Dvar_RegisterVariant_Stub()
	{
		__asm
		{
			pushad

			push eax
			call Dvar::OnRegisterVariant
			add esp, 0x4

			popad

			// Game's code
			pop edi
			pop esi
			pop ebp
			pop ebx
			ret
		}
	}

	Dvar::Dvar()
	{
		// set flags of cg_drawFPS to archive
		Utils::Hook::Or<BYTE>(0x4F8F69, Game::DVAR_ARCHIVE);

		// un-cheat camera_thirdPersonCrosshairOffset and add archive flags
		Utils::Hook::Xor<BYTE>(0x447B41, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE);
		
		// un-cheat cg_fov and add archive flags
		Utils::Hook::Xor<BYTE>(0x4F8E35, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE);
		
		// un-cheat cg_fovscale and add archive flags
		Utils::Hook::Xor<BYTE>(0x4F8E68, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE);

		// un-cheat cg_debugInfoCornerOffset and add archive flags
		Utils::Hook::Xor<BYTE>(0x4F8FC2, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE);

		// remove archive flags for cg_hudchatposition
		Utils::Hook::Xor<BYTE>(0x4F9992, Game::DVAR_ARCHIVE);

		// remove write protection from fs_game
		Utils::Hook::Xor<DWORD>(0x6431EA, Game::DVAR_INIT);

		// set cg_fov max to 160.0
		// because that's the max on SP
		static float cg_Fov = 160.0f;
		Utils::Hook::Set<float*>(0x4F8E28, &cg_Fov);

		// set max volume to 1
		static float volume = 1.0f;
		Utils::Hook::Set<float*>(0x408078, &volume);

		// Uncheat ui_showList
		Utils::Hook::Xor<BYTE>(0x6310DC, Game::DVAR_CHEAT);

		// Uncheat ui_debugMode
		Utils::Hook::Xor<BYTE>(0x6312DE, Game::DVAR_CHEAT);

		// Hook dvar 'name' registration
		Utils::Hook(0x40531C, Dvar::Dvar_RegisterName, HOOK_CALL).install()->quick();

		// un-cheat safeArea_* and add archive flags
		Utils::Hook::Xor<INT>(0x42E3F5, Game::DVAR_ROM | Game::DVAR_ARCHIVE); //safeArea_adjusted_horizontal
		Utils::Hook::Xor<INT>(0x42E423, Game::DVAR_ROM | Game::DVAR_ARCHIVE); //safeArea_adjusted_vertical
		Utils::Hook::Xor<BYTE>(0x42E398, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE); //safeArea_horizontal
		Utils::Hook::Xor<BYTE>(0x42E3C4, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE); //safeArea_vertical

		// Don't allow setting cheat protected dvars via menus
		Utils::Hook(0x63C897, Dvar::SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x63CA96, Dvar::SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x63CDB5, Dvar::SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x635E47, Dvar::SetFromStringByNameExternal, HOOK_CALL).install()->quick();

		// Script_SetDvar
		Utils::Hook(0x63444C, Dvar::SetFromStringByNameSafeExternal, HOOK_CALL).install()->quick();

		// Slider
		Utils::Hook(0x636159, Dvar::SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x636189, Dvar::SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x6364EA, Dvar::SetFromStringByNameExternal, HOOK_CALL).install()->quick();

		Utils::Hook(0x636207, Dvar::SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x636608, Dvar::SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x636695, Dvar::SetFromStringByNameExternal, HOOK_CALL).install()->quick();

		// Entirely block setting cheat dvars internally without sv_cheats
		//Utils::Hook(0x4F52EC, Dvar::SetFromStringByNameExternal, HOOK_CALL).install()->quick();

		// Hook Dvar_SetFromStringByName inside CG_SetClientDvarFromServer so we can reset dvars when the player leaves the server
		Utils::Hook(0x59386A, Dvar::DvarSetFromStringByNameStub, HOOK_CALL).install()->quick();

		// If the game closed abruptly, the dvars would not have been restored
		Scheduler::Once(Dvar::ResetDvarsValue, Scheduler::Pipeline::MAIN);

		// Reset archive dvars when client leaves a server
		Events::OnSteamDisconnect(Dvar::ResetDvarsValue);

		// For debugging
		Utils::Hook(0x6483FA, Dvar::Dvar_RegisterVariant_Stub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x648438, Dvar::Dvar_RegisterVariant_Stub, HOOK_JUMP).install()->quick();
	}

	Dvar::~Dvar()
	{
		Utils::IO::RemoveFile(Dvar::ArchiveDvarPath);
	}
}
