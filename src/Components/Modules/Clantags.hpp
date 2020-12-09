#pragma once

namespace Components
{
	class ClanTags : public Component
	{
	public:
		static void ParseClantags(const char * infoString);
		static void SendClantagsToClients();
		static const char* GetUserClantag(std::uint32_t clientnum, const char * playername);

		ClanTags();
		~ClanTags();

	private:
		static std::string Tags[18];

		static void DrawPlayerNameOnScoreboard();

	};
}
