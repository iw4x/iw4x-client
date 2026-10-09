#pragma once

namespace Components
{
	// Lets the chat field take characters outside ASCII, like Arabic, and stores them as UTF-8
	class ChatInput : public Component
	{
	public:
		ChatInput();

		// Moves and deletes whole UTF-8 characters, returns true when the key was handled
		static bool HandleKey(int localClientNum, int key);

	private:
		static bool IsChatActive();
		static unsigned int GetInputCodePage();

		static std::size_t PreviousCharLength(const Game::field_t* field);
		static std::size_t NextCharLength(const Game::field_t* field);
		static void Erase(Game::field_t* field, std::size_t position, std::size_t length);
		static void Insert(Game::field_t* field, const std::string& text);

		static BOOL OnChar(WPARAM wParam, LPARAM lParam);
	};
}
