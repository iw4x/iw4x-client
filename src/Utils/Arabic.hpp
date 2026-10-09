#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace Utils::Arabic
{
	enum class TokenKind
	{
		None,
		Color,  // Changes the color of the following text (e.g. "^1")
		Object, // Inline object drawn as one unit (e.g. a font icon)
	};

	struct Token
	{
		TokenKind kind;
		std::size_t length;
	};

	// Identifies engine markup at the given position so it survives reordering.
	using Tokenizer = std::function<Token(const char* text)>;

	// Decodes a valid multi-byte UTF-8 sequence. Returns its byte length, or 0 for ASCII and invalid sequences.
	std::size_t DecodeUtf8(const char* text, std::uint32_t* codepoint);
	void EncodeUtf8(std::uint32_t codepoint, std::string& out);

	// Presentation forms ProcessForDisplay shapes letters into, the letters an Arabic font needs
	std::vector<std::uint32_t> GetPresentationForms();

	// True if the text contains right-to-left characters encoded as UTF-8.
	bool ContainsRtl(const char* text);

	// Shapes Arabic letters into their joined presentation forms and reorders each line
	// into visual (left-to-right) order, so it can be drawn glyph by glyph.
	// A cursor byte offset into text is updated to where it belongs in the result.
	std::string ProcessForDisplay(const char* text, const Tokenizer& tokenizer, const std::string& resetColorCode, int* cursor = nullptr);
}
