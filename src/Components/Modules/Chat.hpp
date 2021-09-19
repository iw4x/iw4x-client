#pragma once

namespace Components
{
	class Chat : public Component
	{
	public:
		Chat();

	private:
		static bool SendChat;

		static const char* EvaluateSay(char* text, Game::gentity_t* player);

		static void PreSayStub();
		static void PostSayStub();
	};
}
