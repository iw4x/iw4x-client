#include <STDInclude.hpp>
#include <Utils/InfoString.hpp>

#include <Components/Modules/Events.hpp>

#include "Script.hpp"
#include "UserInfo.hpp"

namespace Components::GSC
{
	std::unordered_map<int, UserInfo::userInfoMap> UserInfo::UserInfoOverrides;

	void UserInfo::SV_GetUserInfo_Stub(int index, char* buffer, int bufferSize)
	{
		Utils::Hook::Call<void(int, char*, int)>(0x49A160)(index, buffer, bufferSize);

		Utils::InfoString map(buffer);

		if (!UserInfoOverrides.contains(index))
		{
			UserInfoOverrides[index] = {};
		}

		for (const auto& [key, val] : UserInfoOverrides[index])
		{
			if (val.empty())
			{
				map.remove(key);
			}
			else
			{
				map.set(key, val);
			}
		}

		const auto userInfo = map.build();
		strncpy_s(buffer, bufferSize, userInfo.data(), _TRUNCATE);
	}

	void UserInfo::ClearClientOverrides(const int clientNum)
	{
		UserInfoOverrides[clientNum].clear();
	}

	void UserInfo::ClearAllOverrides()
	{
		UserInfoOverrides.clear();
	}

	void UserInfo::AddScriptMethods()
	{
		Script::AddMethod("SetName", [](Game::scr_entref_t entref)  // gsc: self SetName(<string>)
		{
			const auto* ent = Script::Scr_GetPlayerEntity(entref);
			const auto* name = Game::Scr_GetString(0);

			if (!name)
			{
				Game::Scr_ParamError(0, "SetName: Illegal parameter!");
				return;
			}

			Logger::Debug("Setting name of {} to {}", ent->s.number, name);
			UserInfoOverrides[ent->s.number]["name"] = name;
			Game::ClientUserinfoChanged(ent->s.number);
		});

		Script::AddMethod("ResetName", [](Game::scr_entref_t entref)  // gsc: self ResetName()
		{
			const auto* ent = Script::Scr_GetPlayerEntity(entref);

			Logger::Debug("Resetting name of {}", ent->s.number);
			UserInfoOverrides[ent->s.number].erase("name");
			Game::ClientUserinfoChanged(ent->s.number);
		});

		Script::AddMethod("SetClanTag", [](Game::scr_entref_t entref)  // gsc: self SetClanTag(<string>)
		{
			const auto* ent = Script::Scr_GetPlayerEntity(entref);
			const auto* clanName = Game::Scr_GetString(0);

			if (!clanName)
			{
				Game::Scr_ParamError(0, "SetClanTag: Illegal parameter!");
				return;
			}

			Logger::Debug("Setting clanName of {} to {}", ent->s.number, clanName);
			UserInfoOverrides[ent->s.number]["clanAbbrev"] = clanName;
			Game::ClientUserinfoChanged(ent->s.number);
		});

		Script::AddMethod("ResetClanTag", [](Game::scr_entref_t entref)  // gsc: self ResetClanTag()
		{
			const auto* ent = Script::Scr_GetPlayerEntity(entref);
			
			Logger::Debug("Resetting clanName of {}", ent->s.number);
			UserInfoOverrides[ent->s.number].erase("clanAbbrev");
			Game::ClientUserinfoChanged(ent->s.number);
		});
	}

	UserInfo::UserInfo()
	{
		Utils::Hook(0x445268, SV_GetUserInfo_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x478B04, SV_GetUserInfo_Stub, HOOK_CALL).install()->quick();

		AddScriptMethods();

		Events::OnVMShutdown(ClearAllOverrides);
		Events::OnClientDisconnect(ClearClientOverrides);
	}
}
