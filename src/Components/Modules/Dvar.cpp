#include <STDInclude.hpp>

#include "Friends.hpp"
#include "TextRenderer.hpp"

namespace Components
{
	Dvar::Var Dvar::Name;

	Dvar::Var::Var(const std::string& dvarName)
		: dvar_(Game::Dvar_FindVar(dvarName.data()))
	{
		// If the dvar can't be found it will be registered as an empty string dvar
		if (!this->dvar_)
		{
			this->dvar_ = const_cast<Game::dvar_t*>(Game::Dvar_SetFromStringByNameFromSource(dvarName.data(), "", Game::DVAR_SOURCE_INTERNAL));
		}
	}

	template <> Game::dvar_t* Dvar::Var::get()
	{
		return this->dvar_;
	}

	template <> const char* Dvar::Var::get()
	{
		if (!this->dvar_)
		{
			return "";
		}

		if (this->dvar_->type == Game::DVAR_TYPE_STRING || this->dvar_->type == Game::DVAR_TYPE_ENUM)
		{
			if (this->dvar_->current.string)
			{
				return this->dvar_->current.string;
			}
		}

		return "";
	}

	template <> int Dvar::Var::get()
	{
		if (!this->dvar_)
		{
			return 0;
		}

		if (this->dvar_->type == Game::DVAR_TYPE_INT || this->dvar_->type == Game::DVAR_TYPE_ENUM)
		{
			return this->dvar_->current.integer;
		}

		return 0;
	}

	template <> unsigned int Dvar::Var::get()
	{
		if (!this->dvar_)
		{
			return 0;
		}

		if (this->dvar_->type == Game::DVAR_TYPE_INT)
		{
			return this->dvar_->current.unsignedInt;
		}

		return 0;
	}

	template <> float Dvar::Var::get()
	{
		if (!this->dvar_)
		{
			return 0.f;
		}

		if (this->dvar_->type == Game::DVAR_TYPE_FLOAT)
		{
			return this->dvar_->current.value;
		}

		return 0.f;
	}

	template <> bool Dvar::Var::get()
	{
		if (!this->dvar_)
		{
			return false;
		}

		if (this->dvar_->type == Game::DVAR_TYPE_BOOL)
		{
			return this->dvar_->current.enabled;
		}

		return false;
	}

	template <> std::string Dvar::Var::get()
	{
		return this->get<const char*>();
	}

	void Dvar::Var::set(const char* string)
	{
		assert(string);
		assert(this->dvar_->type == Game::DVAR_TYPE_STRING);

		if (this->dvar_)
		{
			Game::Dvar_SetString(this->dvar_, string);
		}
	}

	void Dvar::Var::set(const std::string& string)
	{
		this->set(string.data());
	}

	void Dvar::Var::set(int integer)
	{
		assert(this->dvar_->type == Game::DVAR_TYPE_INT);

		if (this->dvar_)
		{
			Game::Dvar_SetInt(this->dvar_, integer);
		}
	}

	void Dvar::Var::set(float value)
	{
		assert(this->dvar_->type == Game::DVAR_TYPE_FLOAT);

		if (this->dvar_)
		{
			Game::Dvar_SetFloat(this->dvar_, value);
		}
	}

	void Dvar::Var::set(bool enabled)
	{
		assert(this->dvar_->type == Game::DVAR_TYPE_BOOL);

		if (this->dvar_)
		{
			Game::Dvar_SetBool(this->dvar_, enabled);
		}
	}

	void Dvar::Var::setRaw(int integer)
	{
		assert(this->dvar_->type == Game::DVAR_TYPE_INT);

		if (this->dvar_)
		{
			this->dvar_->current.integer = integer;
			this->dvar_->latched.integer = integer;
		}
	}

	void Dvar::Var::setRaw(float value)
	{
		assert(this->dvar_->type == Game::DVAR_TYPE_FLOAT);

		if (this->dvar_)
		{
			this->dvar_->current.value = value;
			this->dvar_->latched.value = value;
		}
	}

	void Dvar::Var::setRaw(bool enabled)
	{
		assert(this->dvar_->type == Game::DVAR_TYPE_BOOL);

		if (this->dvar_)
		{
			this->dvar_->current.enabled = enabled;
			this->dvar_->latched.enabled = enabled;
		}
	}

	template<> Dvar::Var Dvar::Register(const char* dvarName, bool value, std::uint16_t flag, const char* description)
	{
		return Game::Dvar_RegisterBool(dvarName, value, flag, description);
	}

	template<> Dvar::Var Dvar::Register(const char* dvarName, const char* value, std::uint16_t flag, const char* description)
	{
		return Game::Dvar_RegisterString(dvarName, value, flag, description);
	}

	template<> Dvar::Var Dvar::Register(const char* dvarName, int value, int min, int max, std::uint16_t flag, const char* description)
	{
		return Game::Dvar_RegisterInt(dvarName, value, min, max, flag, description);
	}

	template<> Dvar::Var Dvar::Register(const char* dvarName, float value, float min, float max, std::uint16_t flag, const char* description)
	{
		return Game::Dvar_RegisterFloat(dvarName, value, min, max, flag, description);
	}

	const Game::dvar_t* Dvar::Dvar_RegisterName(const char* dvarName, const char* /*value*/, std::uint16_t flags, const char* description)
	{
		// Name watcher
		if (!Dedicated::IsEnabled() && !ZoneBuilder::IsEnabled())
		{
			Scheduler::Loop([]
			{
				static std::string lastValidName = "Unknown Soldier";
				auto name = Name.get<std::string>();

				// Don't perform any checks if name didn't change
				if (name == lastValidName) return;

				Utils::String::Trim(name);
				auto saneName = TextRenderer::StripAllTextIcons(TextRenderer::StripColors(name));
				if (saneName.size() < 3 || (saneName[0] == '[' && saneName[1] == '{'))
				{
					Logger::PrintError(Game::CON_CHANNEL_ERROR, "Username '{}' is invalid. It must at least be 3 characters long and not appear empty!\n", name);
					Name.set(lastValidName);
				}
				else
				{
					lastValidName = name;
					Friends::UpdateName();
				}
			}, Scheduler::Pipeline::CLIENT, 3s); // Don't need to do this every frame
		}

		std::string username = "Unknown Soldier";

		if (Steam::Proxy::SteamFriends)
		{
			const char* steamName = Steam::Proxy::SteamFriends->GetPersonaName();

			if (steamName && *steamName)
			{
				username = steamName;
			}
		}

		Name = Register<const char*>(dvarName, username.data(), flags | Game::DVAR_ARCHIVE, description);
		return Name.get<Game::dvar_t*>();
	}

	const Game::dvar_t* Dvar::Dvar_RegisterSVNetworkFps(const char* dvarName, int /*value*/, int min, int /*max*/, std::uint16_t /*flags*/, const char* description)
	{
		return Game::Dvar_RegisterInt(dvarName, 1000, min, 1000, Game::DVAR_NONE, description);
	}

	const Game::dvar_t* Dvar::Dvar_RegisterPerkExtendedMeleeRange(const char* dvarName, float value, float min, float /*max*/, std::uint16_t flags, const char* description)
	{
		return Game::Dvar_RegisterFloat(dvarName, value, min, 10000.0f, flags, description);
	}

	void Dvar::SetFromStringByNameSafeExternal(const char* dvarName, const char* string)
	{
		static std::array exceptions =
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

		for (const auto& entry : exceptions)
		{
			if (!_stricmp(dvarName, entry))
			{
				Game::Dvar_SetFromStringByNameFromSource(dvarName, string, Game::DVAR_SOURCE_INTERNAL);
				return;
			}
		}

		SetFromStringByNameExternal(dvarName, string);
	}

	void Dvar::SetFromStringByNameExternal(const char* dvarName, const char* string)
	{
		Game::Dvar_SetFromStringByNameFromSource(dvarName, string, Game::DVAR_SOURCE_EXTERNAL);
	}

	bool Dvar::AreArchiveDvarsUnprotected()
	{
		static std::optional<bool> flag;

		if (!flag.has_value())
		{
			flag.emplace(Flags::HasFlag("unprotect-dvars"));
		}

		return flag.value();
	}

	bool Dvar::IsSettingDvarsDisabled()
	{
		static std::optional<bool> flag;

		if (!flag.has_value())
		{
			flag.emplace(Flags::HasFlag("protect-dvars"));
		}

		return flag.value();
	}

	void Dvar::DvarSetFromStringByName_Stub(const char* dvarName, const char* value)
	{
		if (IsSettingDvarsDisabled())
		{
			Logger::Debug("Not allowing server to set '{}'", dvarName);
			return;
		}

		// Save the dvar original value if it has the archive flag
		const auto* dvar = Game::Dvar_FindVar(dvarName);
		if (dvar && dvar->flags & Game::DVAR_ARCHIVE)
		{
			if (!AreArchiveDvarsUnprotected())
			{
				Logger::Print(Game::CON_CHANNEL_CONSOLEONLY, "Not allowing server to override saved dvar '{}'\n", dvar->name);
				return;
			}

			Logger::Print(Game::CON_CHANNEL_CONSOLEONLY, "Server is overriding saved dvar '{}'\n", dvarName);
		}

		if (dvar && std::strcmp(dvar->name, "com_errorResolveCommand") == 0)
		{
			Logger::Print(Game::CON_CHANNEL_CONSOLEONLY, "Not allowing server to set '{}'\n", dvar->name);
			return;
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
			call OnRegisterVariant
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

	const char* Dvar::Dvar_EnumToString_Stub(const Game::dvar_t* dvar)
	{
		assert(dvar);
		assert(dvar->name);
		assert(dvar->type == Game::DVAR_TYPE_ENUM);
		assert(dvar->domain.enumeration.strings);
		assert(dvar->current.integer >= 0 && dvar->current.integer < dvar->domain.enumeration.stringCount || dvar->current.integer == 0);

		// Fix nullptr crash
		if (!dvar || dvar->domain.enumeration.stringCount == 0)
		{
			return "";
		}

		return dvar->domain.enumeration.strings[dvar->current.integer];
	}

	Dvar::Dvar()
	{
		// set flags of cg_drawFPS to archive
		Utils::Hook::Or<std::uint8_t>(0x4F8F69, Game::DVAR_ARCHIVE);

		// un-cheat camera_thirdPersonCrosshairOffset and add archive flags
		Utils::Hook::Xor<std::uint8_t>(0x447B41, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE);
		
		// un-cheat cg_fov and add archive flags
		Utils::Hook::Xor<std::uint8_t>(0x4F8E35, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE);
		
		// un-cheat cg_fovscale and add archive flags
		Utils::Hook::Xor<std::uint8_t>(0x4F8E68, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE);

		// un-cheat cg_fovMin and add archive flags
		Utils::Hook::Xor<std::uint8_t>(0x4F8E9D, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE);

		// un-cheat cg_debugInfoCornerOffset and add archive flags
		Utils::Hook::Xor<std::uint8_t>(0x4F8FC2, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE);

		// un-cheat cg_drawGun
		Utils::Hook::Set<std::uint8_t>(0x4F8DC6, Game::DVAR_NONE);

		// un-cheat cg_draw2D
		Utils::Hook::Set<std::uint8_t>(0x4F8EEE, Game::DVAR_NONE);

		// un-cheat cg_overheadNamesFarScale
		Utils::Hook::Set<std::uint8_t>(0x4FA7C4, Game::DVAR_NONE);

		// un-cheat cg_overheadNamesSize
		Utils::Hook::Set<std::uint8_t>(0x4FA7F9, Game::DVAR_NONE);

		// un-cheat cg_overheadRankSize
		Utils::Hook::Set<std::uint8_t>(0x4FA863, Game::DVAR_NONE);

		// un-cheat cg_overheadIconSize
		Utils::Hook::Set<std::uint8_t>(0x4FA833, Game::DVAR_NONE);

		// un-cheat cg_overheadTitleSize
		Utils::Hook::Set<std::uint8_t>(0x4FA898, Game::DVAR_NONE);

		// un-cheat cg_overheadNamesGlow
		Utils::Hook::Set<std::uint8_t>(0x4FA8C9, Game::DVAR_NONE);

		// remove archive flags for cg_hudChatPosition
		Utils::Hook::Xor<std::uint8_t>(0x4F9992, Game::DVAR_ARCHIVE);

		// remove archive flags for sv_hostname
		Utils::Hook::Xor<std::uint32_t>(0x4D3786, Game::DVAR_ARCHIVE);

		// remove write protection from fs_game
		Utils::Hook::Xor<std::uint32_t>(0x6431EA, Game::DVAR_INIT);

		// set cg_fov max to 160.0
		// because that's the max on SP
		static float cg_Fov = 160.0f;
		Utils::Hook::Set<float*>(0x4F8E28, &cg_Fov);

		// set max volume to 1
		static float volume = 1.0f;
		Utils::Hook::Set<float*>(0x408078, &volume);

		// un-cheat ui_showList
		Utils::Hook::Xor<std::uint8_t>(0x6310DC, Game::DVAR_CHEAT);

		// un-cheat ui_debugMode
		Utils::Hook::Xor<std::uint8_t>(0x6312DE, Game::DVAR_CHEAT);

		// un-cheat jump_slowdownEnable
		Utils::Hook::Xor<std::uint32_t>(0x4EFABE, Game::DVAR_CHEAT);

		// un-cheat jump_height
		Utils::Hook::Xor<std::uint32_t>(0x4EFA5C, Game::DVAR_CHEAT);

		// un-cheat player_breath_fire_delay
		Utils::Hook::Xor<std::uint32_t>(0x448646, Game::DVAR_CHEAT);

		// un-cheat player_breath_gasp_scale
		Utils::Hook::Xor<std::uint32_t>(0x448678, Game::DVAR_CHEAT);

		// un-cheat player_breath_gasp_lerp
		Utils::Hook::Xor<std::uint32_t>(0x4486E4, Game::DVAR_CHEAT);

		// un-cheat player_breath_gasp_time
		Utils::Hook::Xor<std::uint32_t>(0x448612, Game::DVAR_CHEAT);

		// Hook dvar 'name' registration
		Utils::Hook(0x40531C, Dvar_RegisterName, HOOK_CALL).install()->quick();

		// Hook dvar 'sv_network_fps' registration
		Utils::Hook(0x4D3C7B, Dvar_RegisterSVNetworkFps, HOOK_CALL).install()->quick();

		// Hook dvar 'perk_extendedMeleeRange' and set a higher max, better than having people force this with external programs
		Utils::Hook(0x492D2F, Dvar_RegisterPerkExtendedMeleeRange, HOOK_CALL).install()->quick();

		// un-cheat safeArea_* and add archive flags
		Utils::Hook::Xor<std::uint32_t>(0x42E3F5, Game::DVAR_ROM | Game::DVAR_ARCHIVE); //safeArea_adjusted_horizontal
		Utils::Hook::Xor<std::uint32_t>(0x42E423, Game::DVAR_ROM | Game::DVAR_ARCHIVE); //safeArea_adjusted_vertical
		Utils::Hook::Xor<std::uint8_t>(0x42E398, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE); //safeArea_horizontal
		Utils::Hook::Xor<std::uint8_t>(0x42E3C4, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE); //safeArea_vertical

		// Don't allow setting cheat protected dvars via menus
		Utils::Hook(0x63C897, SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x63CA96, SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x63CDB5, SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x635E47, SetFromStringByNameExternal, HOOK_CALL).install()->quick();

		// Script_SetDvar
		Utils::Hook(0x63444C, SetFromStringByNameSafeExternal, HOOK_CALL).install()->quick();

		// Slider
		Utils::Hook(0x636159, SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x636189, SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x6364EA, SetFromStringByNameExternal, HOOK_CALL).install()->quick();

		Utils::Hook(0x636207, SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x636608, SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x636695, SetFromStringByNameExternal, HOOK_CALL).install()->quick();

		// Hook Dvar_SetFromStringByName inside CG_SetClientDvarFromServer so we can protect dvars
		Utils::Hook(0x59386A, DvarSetFromStringByName_Stub, HOOK_CALL).install()->quick();

		// For debugging
		Utils::Hook(0x6483FA, Dvar_RegisterVariant_Stub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x648438, Dvar_RegisterVariant_Stub, HOOK_JUMP).install()->quick();

		// Fix crash
		Utils::Hook(0x4B7120, Dvar_EnumToString_Stub, HOOK_JUMP).install()->quick();
	}
}
