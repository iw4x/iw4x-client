#pragma once

namespace Components
{
	class Clantags : public Component
	{
	public:
		static void ParseClantags(const char * infoString);
		static void SendClantagsToClients();
		static const char* GetUserClantag(std::uint32_t clientnum, const char * playername);

		Clantags();
		~Clantags();

	private:
		static std::string Clantags::Tags[18];

		static void DrawPlayerNameOnScoreboard();

	};
}
