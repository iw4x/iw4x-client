#pragma once

namespace Components
{
	class Clantags : public Component
	{
	public:
		static std::vector < std::string > Clantags::Tags;
		static void ParseClantags(const char * infoString);
		static void SendClantagsToClients();

		Clantags();
		~Clantags();

	private:
		static void GetUserClantag(std::uint32_t clientnum, const char * playername);
		static void DrawPlayerNameOnScoreboard();

	};
}
