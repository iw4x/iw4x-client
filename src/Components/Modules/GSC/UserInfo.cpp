#include <Utils/InfoString.hpp>

#include <Components/Modules/Events.hpp>
#include <Components/Modules/ClanTags.hpp>

#include "UserInfo.hpp"
#include "Script.hpp"

namespace Components::GSC
{
	std::unordered_map<std::uint64_t, UserInfo::userInfoMap> UserInfo::UserInfoOverrides;

	std::uint64_t UserInfo::GetOverrideKey(const int clientNum)
	{
		const auto steamID = Game::svs_clients[clientNum].steamID;
		return steamID != 0 ? steamID : static_cast<std::uint64_t>(clientNum);
	}

	void UserInfo::SV_GetUserInfo_Stub(int index, char* buffer, int bufferSize)
	{
		Utils::Hook::Call<void(int, char*, int)>(0x49A160)(index, buffer, bufferSize);

		const auto it = UserInfoOverrides.find(GetOverrideKey(index));

		if (it == UserInfoOverrides.end() || it->second.empty())
		{
			return;
		}

		Utils::InfoString map(buffer);

		for (const auto& [key, val] : it->second)
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
		const auto steamID = Game::svs_clients[clientNum].steamID;

		if (steamID == 0)
		{
			UserInfoOverrides.erase(static_cast<std::uint64_t>(clientNum));
		}
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
			UserInfoOverrides[GetOverrideKey(ent->s.number)]["name"] = name;
			Game::ClientUserinfoChanged(ent->s.number);
		});

		Script::AddMethod("ResetName", [](Game::scr_entref_t entref)  // gsc: self ResetName()
		{
			const auto* ent = Script::Scr_GetPlayerEntity(entref);

			Logger::Debug("Resetting name of {}", ent->s.number);
			UserInfoOverrides[GetOverrideKey(ent->s.number)].erase("name");
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
			UserInfoOverrides[GetOverrideKey(ent->s.number)]["clanAbbrev"] = clanName;
			ClanTags::SetClanName(clanName);
			Game::ClientUserinfoChanged(ent->s.number);
		});

		Script::AddMethod("ResetClanTag", [](Game::scr_entref_t entref)  // gsc: self ResetClanTag()
		{
			const auto* ent = Script::Scr_GetPlayerEntity(entref);

			Logger::Debug("Resetting clanName of {}", ent->s.number);
			UserInfoOverrides[GetOverrideKey(ent->s.number)].erase("clanAbbrev");
			ClanTags::SetClanName("");
			Game::ClientUserinfoChanged(ent->s.number);
		});
	}

	UserInfo::UserInfo()
	{
		Utils::Hook(0x445268, SV_GetUserInfo_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x478B04, SV_GetUserInfo_Stub, HOOK_CALL).install()->quick();

		AddScriptMethods();

		Events::OnClientDisconnect(ClearClientOverrides);
	}
}
