#pragma once

namespace Components
{
	union Skin
	{
		struct
		{
			unsigned int headIndex : 8;
			unsigned int bodyIndex : 8;
			unsigned int enableHead : 1;
			unsigned int enableBody : 1;
			unsigned int reserved : 6; // Unused for now
		};

		int intValue;
	};

	class PlayerSkins : public Component
	{

	public:
		PlayerSkins();

		static Skin GetSkin() { return currentSkin; };

		static void GScr_GetPlayerHead(Game::gentity_s* entRef);
		static void GScr_GetPlayerBody(Game::gentity_s* entRef);

	private:
		static const std::string heads[];
		static const std::string bodies[];
		static Game::scr_string_t headsScriptStrings[];
		static Game::scr_string_t bodiesScriptStrings[];

		static Dvar::Var localHeadIndexDvar;
		static Dvar::Var localBodyIndexDvar;
		static Dvar::Var localEnableHeadDvar;
		static Dvar::Var localEnableBodyDvar;
		static Dvar::Var skinTryOut;
		static Dvar::Var sv_allowSkins;
		static Dvar::Var sv_overrideTeamSkins;

		static std::unordered_set<int> forbiddenHeadBodyCombinations;

		static uint32_t GetTrueSkillForGametype(int localClientIndex, char* gametype);
		static void RefreshPlayerSkinFromDvars();
		static void SanitizeSkin(Skin& skin);
		static void CheckForbiddenHeadBodyCombinations();
		static void RegisterConstantStrings();

		static void CheckStrings();

		static void Info_SetValueForKey(const char* infoString, const char* key, const char* data);

		static bool HasAuthorizedBoneCount(const Skin& skin, std::string& err);
		static Game::scr_string_t GetHeadName(const Skin& skin);
		static Game::scr_string_t GetBodyName(const Skin& skin);
		
		static bool GetPlayerSkinInternal(Game::gentity_s* entRef, OUT Game::scr_string_t& head, OUT Game::scr_string_t& body);

		static Skin currentSkin;
	};
}
