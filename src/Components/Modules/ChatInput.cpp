#include "ChatInput.hpp"
#include "Window.hpp"
#include "ZoneBuilder.hpp"

namespace Components
{
	bool ChatInput::IsChatActive()
	{
		// The console takes the input while it is open
		return Game::Key_IsCatcherActive(0, Game::KEYCATCH_CHAT) && !Game::Key_IsCatcherActive(0, Game::KEYCATCH_CONSOLE);
	}

	unsigned int ChatInput::GetInputCodePage()
	{
		// The game window is ANSI, so typed characters arrive in the code page of the keyboard layout (1256 for Arabic)
		char codePage[8]{};
		const auto language = LOWORD(GetKeyboardLayout(0));
		if (GetLocaleInfoA(MAKELCID(language, SORT_DEFAULT), LOCALE_IDEFAULTANSICODEPAGE, codePage, sizeof(codePage)) > 0)
		{
			return std::strtoul(codePage, nullptr, 10);
		}

		return CP_ACP;
	}

	std::size_t ChatInput::PreviousCharLength(const Game::field_t* field)
	{
		// Step back over continuation bytes to the start of the UTF-8 character
		auto start = field->cursor;
		while (start > 0 && (static_cast<unsigned char>(field->buffer[start - 1]) & 0xC0) == 0x80)
		{
			--start;
		}

		if (start > 0)
		{
			--start;
		}

		return static_cast<std::size_t>(field->cursor - start);
	}

	std::size_t ChatInput::NextCharLength(const Game::field_t* field)
	{
		std::uint32_t codepoint;
		const auto length = Utils::Arabic::DecodeUtf8(&field->buffer[field->cursor], &codepoint);
		return length ? length : (field->buffer[field->cursor] ? 1 : 0);
	}

	void ChatInput::Erase(Game::field_t* field, const std::size_t position, const std::size_t length)
	{
		const auto size = std::strlen(field->buffer);
		std::memmove(&field->buffer[position], &field->buffer[position + length], size - position - length + 1);
	}

	void ChatInput::Insert(Game::field_t* field, const std::string& text)
	{
		const auto size = std::strlen(field->buffer);
		if (size + text.size() >= sizeof(field->buffer))
		{
			return;
		}

		auto* position = &field->buffer[field->cursor];
		std::memmove(position + text.size(), position, size - field->cursor + 1);
		std::memcpy(position, text.data(), text.size());
		field->cursor += static_cast<int>(text.size());
	}

	bool ChatInput::HandleKey(const int localClientNum, const int key)
	{
		auto* field = &Game::playerKeys[localClientNum].chatField;

		// Single byte characters are left to the game
		switch (key)
		{
		case Game::K_LEFTARROW:
		{
			const auto length = field->cursor > 0 ? PreviousCharLength(field) : 0;
			if (length < 2) return false;
			field->cursor -= static_cast<int>(length);
			field->scroll = std::min(field->scroll, field->cursor);
			return true;
		}

		case Game::K_RIGHTARROW:
		{
			const auto length = NextCharLength(field);
			if (length < 2) return false;
			field->cursor += static_cast<int>(length);
			return true;
		}

		case Game::K_DEL:
		{
			const auto length = NextCharLength(field);
			if (length < 2) return false;
			Erase(field, field->cursor, length);
			return true;
		}

		default:
			return false;
		}
	}

	BOOL ChatInput::OnChar(const WPARAM wParam, const LPARAM lParam)
	{
		if (IsChatActive())
		{
			auto* field = &Game::playerKeys[0].chatField;
			const auto character = static_cast<char>(wParam);

			// Backspace would only remove the last byte of a multi-byte character
			if (wParam == '\b' && field->cursor > 0)
			{
				const auto length = PreviousCharLength(field);
				if (length > 1)
				{
					field->cursor -= static_cast<int>(length);
					field->scroll = std::min(field->scroll, field->cursor);
					Erase(field, field->cursor, length);
					return 0;
				}
			}

			if (wParam >= 0x80 && wParam <= 0xFF)
			{
				wchar_t wide;
				if (MultiByteToWideChar(GetInputCodePage(), 0, &character, 1, &wide, 1) == 1)
				{
					std::string utf8;
					Utils::Arabic::EncodeUtf8(wide, utf8);
					Insert(field, utf8);
					return 0;
				}
			}
		}

		return Utils::Hook::Call<BOOL(__stdcall)(HWND, UINT, WPARAM, LPARAM)>(0x4731F0)(Window::GetWindow(), WM_CHAR, wParam, lParam);
	}

	ChatInput::ChatInput()
	{
		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())
		{
			return;
		}

		// Window passes the message parameters as (lParam, wParam)
		Window::OnWndMessage(WM_CHAR, [](const WPARAM lParam, const LPARAM wParam)
		{
			return OnChar(static_cast<WPARAM>(wParam), static_cast<LPARAM>(lParam));
		});
	}
}
