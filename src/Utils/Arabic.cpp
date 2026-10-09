#include "Arabic.hpp"

#include <algorithm>
#include <cctype>
#include <optional>
#include <vector>

namespace Utils::Arabic
{
	namespace
	{
		enum class Joining
		{
			None,        // Does not connect to neighbors
			Right,       // Connects only to the previous letter
			Dual,        // Connects to both neighbors
			Transparent, // Skipped when deciding joins (harakat)
		};

		enum class BidiClass
		{
			L,       // Strong left-to-right
			R,       // Strong right-to-left
			Number,  // European or Arabic-Indic digits
			Neutral, // Spaces, punctuation and inline objects
		};

		struct LetterForms
		{
			std::uint32_t isolated, final, initial, medial;
		};

		struct LetterEntry
		{
			std::uint32_t codepoint;
			LetterForms forms;
		};

		// Unicode presentation forms; letters without initial/medial forms are right-joining
		constexpr LetterEntry LETTERS[]
		{
			{0x0621, {0xFE80, 0, 0, 0}},
			{0x0622, {0xFE81, 0xFE82, 0, 0}},
			{0x0623, {0xFE83, 0xFE84, 0, 0}},
			{0x0624, {0xFE85, 0xFE86, 0, 0}},
			{0x0625, {0xFE87, 0xFE88, 0, 0}},
			{0x0626, {0xFE89, 0xFE8A, 0xFE8B, 0xFE8C}},
			{0x0627, {0xFE8D, 0xFE8E, 0, 0}},
			{0x0628, {0xFE8F, 0xFE90, 0xFE91, 0xFE92}},
			{0x0629, {0xFE93, 0xFE94, 0, 0}},
			{0x062A, {0xFE95, 0xFE96, 0xFE97, 0xFE98}},
			{0x062B, {0xFE99, 0xFE9A, 0xFE9B, 0xFE9C}},
			{0x062C, {0xFE9D, 0xFE9E, 0xFE9F, 0xFEA0}},
			{0x062D, {0xFEA1, 0xFEA2, 0xFEA3, 0xFEA4}},
			{0x062E, {0xFEA5, 0xFEA6, 0xFEA7, 0xFEA8}},
			{0x062F, {0xFEA9, 0xFEAA, 0, 0}},
			{0x0630, {0xFEAB, 0xFEAC, 0, 0}},
			{0x0631, {0xFEAD, 0xFEAE, 0, 0}},
			{0x0632, {0xFEAF, 0xFEB0, 0, 0}},
			{0x0633, {0xFEB1, 0xFEB2, 0xFEB3, 0xFEB4}},
			{0x0634, {0xFEB5, 0xFEB6, 0xFEB7, 0xFEB8}},
			{0x0635, {0xFEB9, 0xFEBA, 0xFEBB, 0xFEBC}},
			{0x0636, {0xFEBD, 0xFEBE, 0xFEBF, 0xFEC0}},
			{0x0637, {0xFEC1, 0xFEC2, 0xFEC3, 0xFEC4}},
			{0x0638, {0xFEC5, 0xFEC6, 0xFEC7, 0xFEC8}},
			{0x0639, {0xFEC9, 0xFECA, 0xFECB, 0xFECC}},
			{0x063A, {0xFECD, 0xFECE, 0xFECF, 0xFED0}},
			{0x0640, {0x0640, 0x0640, 0x0640, 0x0640}},
			{0x0641, {0xFED1, 0xFED2, 0xFED3, 0xFED4}},
			{0x0642, {0xFED5, 0xFED6, 0xFED7, 0xFED8}},
			{0x0643, {0xFED9, 0xFEDA, 0xFEDB, 0xFEDC}},
			{0x0644, {0xFEDD, 0xFEDE, 0xFEDF, 0xFEE0}},
			{0x0645, {0xFEE1, 0xFEE2, 0xFEE3, 0xFEE4}},
			{0x0646, {0xFEE5, 0xFEE6, 0xFEE7, 0xFEE8}},
			{0x0647, {0xFEE9, 0xFEEA, 0xFEEB, 0xFEEC}},
			{0x0648, {0xFEED, 0xFEEE, 0, 0}},
			{0x0649, {0xFEEF, 0xFEF0, 0, 0}},
			{0x064A, {0xFEF1, 0xFEF2, 0xFEF3, 0xFEF4}},
			{0x067E, {0xFB56, 0xFB57, 0xFB58, 0xFB59}},
			{0x0686, {0xFB7A, 0xFB7B, 0xFB7C, 0xFB7D}},
			{0x0698, {0xFB8A, 0xFB8B, 0, 0}},
			{0x06A9, {0xFB8E, 0xFB8F, 0xFB90, 0xFB91}},
			{0x06AF, {0xFB92, 0xFB93, 0xFB94, 0xFB95}},
			{0x06CC, {0xFBFC, 0xFBFD, 0xFBFE, 0xFBFF}},
		};

		constexpr std::uint32_t LAM = 0x0644;
		constexpr std::uint32_t ZWNJ = 0x200C;
		constexpr std::uint32_t ZWJ = 0x200D;
		constexpr std::uint32_t LRM = 0x200E;
		constexpr std::uint32_t RLM = 0x200F;

		struct Item
		{
			std::uint32_t codepoint; // 0 for inline objects and raw bytes
			std::string raw;         // Bytes to emit as-is when codepoint is 0
			std::string color;       // Color code active for this item, empty for the base color
			bool object;
			BidiClass bidi;
			int level;
			std::size_t source;  // Byte offset in the input text
			std::size_t logical; // Position before reordering
		};

		const LetterForms* FindLetter(const std::uint32_t codepoint)
		{
			const auto* end = std::end(LETTERS);
			const auto* entry = std::lower_bound(std::begin(LETTERS), end, codepoint, [](const LetterEntry& e, const std::uint32_t c)
			{
				return e.codepoint < c;
			});

			return entry != end && entry->codepoint == codepoint ? &entry->forms : nullptr;
		}

		bool IsTransparent(const std::uint32_t c)
		{
			return (c >= 0x064B && c <= 0x065F) || c == 0x0670 || (c >= 0x06D6 && c <= 0x06ED);
		}

		Joining GetJoining(const Item& item)
		{
			const auto c = item.codepoint;
			if (IsTransparent(c)) return Joining::Transparent;
			if (c == ZWJ) return Joining::Dual;

			const auto* forms = FindLetter(c);
			if (!forms || !forms->final) return Joining::None;
			return forms->initial ? Joining::Dual : Joining::Right;
		}

		bool IsDigit(const std::uint32_t c)
		{
			return (c >= '0' && c <= '9') || (c >= 0x0660 && c <= 0x0669) || (c >= 0x06F0 && c <= 0x06F9);
		}

		bool IsRtl(const std::uint32_t c)
		{
			return (c >= 0x0590 && c <= 0x08FF) || (c >= 0xFB1D && c <= 0xFDFF) || (c >= 0xFE70 && c <= 0xFEFF) || c == RLM;
		}

		BidiClass Classify(const Item& item)
		{
			const auto c = item.codepoint;
			if (item.object) return BidiClass::Neutral;
			if (c == 0) return BidiClass::L; // Legacy single-byte characters
			if (IsDigit(c)) return BidiClass::Number;
			if (IsRtl(c)) return BidiClass::R;
			if (c == LRM) return BidiClass::L;
			if (c < 0x80) return std::isalpha(static_cast<int>(c)) ? BidiClass::L : BidiClass::Neutral;
			if (c >= 0x2000 && c <= 0x206F) return BidiClass::Neutral; // General punctuation
			return BidiClass::L;
		}

		std::uint32_t Mirror(const std::uint32_t c)
		{
			switch (c)
			{
			case '(': return ')';
			case ')': return '(';
			case '[': return ']';
			case ']': return '[';
			case '{': return '}';
			case '}': return '{';
			case '<': return '>';
			case '>': return '<';
			case 0xAB: return 0xBB;
			case 0xBB: return 0xAB;
			default: return c;
			}
		}

		std::uint32_t LamAlefLigature(const std::uint32_t alef)
		{
			switch (alef)
			{
			case 0x0622: return 0xFEF5;
			case 0x0623: return 0xFEF7;
			case 0x0625: return 0xFEF9;
			case 0x0627: return 0xFEFB;
			default: return 0;
			}
		}

		std::vector<Item> Tokenize(const char* text, const Tokenizer& tokenizer, const std::string& resetColorCode, std::string& color)
		{
			std::vector<Item> items;
			const auto* start = text;

			while (*text)
			{
				if (const auto token = tokenizer(text); token.kind != TokenKind::None && token.length > 0)
				{
					if (token.kind == TokenKind::Color)
					{
						color.assign(text, token.length);
						if (color == resetColorCode) color.clear();
					}
					else
						items.push_back({0, std::string(text, token.length), color, true, {}, 0, static_cast<std::size_t>(text - start)});

					text += token.length;
					continue;
				}

				std::uint32_t codepoint;
				if (const auto length = DecodeUtf8(text, &codepoint); length > 0)
				{
					items.push_back({codepoint, {}, color, false, {}, 0, static_cast<std::size_t>(text - start)});
					text += length;
				}
				else
				{
					const auto byte = static_cast<unsigned char>(*text);
					if (byte < 0x80)
						items.push_back({byte, {}, color, false, {}, 0, static_cast<std::size_t>(text - start)});
					else
						items.push_back({0, std::string(1, *text), color, false, {}, 0, static_cast<std::size_t>(text - start)});
					++text;
				}
			}

			return items;
		}

		void Shape(std::vector<Item>& items)
		{
			auto neighborJoining = [&](std::size_t index, const int step) -> Joining
			{
				for (index += step; index < items.size(); index += step)
				{
					const auto joining = GetJoining(items[index]);
					if (joining != Joining::Transparent) return joining;
				}
				return Joining::None;
			};

			std::vector<Item> shaped;
			shaped.reserve(items.size());

			for (std::size_t i = 0; i < items.size(); ++i)
			{
				auto item = items[i];
				const auto* forms = FindLetter(item.codepoint);
				if (!forms)
				{
					shaped.push_back(std::move(item));
					continue;
				}

				const auto joining = GetJoining(item);
				const auto previous = neighborJoining(i, -1);
				const auto joinsPrevious = joining != Joining::None && (previous == Joining::Dual);

				if (item.codepoint == LAM && i + 1 < items.size())
				{
					if (const auto ligature = LamAlefLigature(items[i + 1].codepoint))
					{
						item.codepoint = joinsPrevious ? ligature + 1 : ligature;
						shaped.push_back(std::move(item));
						++i;
						continue;
					}
				}

				const auto next = neighborJoining(i, 1);
				const auto joinsNext = joining == Joining::Dual && (next == Joining::Dual || next == Joining::Right);

				if (joinsPrevious && joinsNext) item.codepoint = forms->medial;
				else if (joinsPrevious) item.codepoint = forms->final;
				else if (joinsNext) item.codepoint = forms->initial;
				else item.codepoint = forms->isolated;

				shaped.push_back(std::move(item));
			}

			items = std::move(shaped);
		}

		void ResolveLevels(std::vector<Item>& items, const bool rtlParagraph)
		{
			const auto baseLevel = rtlParagraph ? 1 : 0;
			const auto levelL = rtlParagraph ? 2 : 0;
			constexpr auto levelR = 1;

			for (auto& item : items)
				item.bidi = Classify(item);

			// Combining marks take the class of the character they attach to
			for (std::size_t i = 1; i < items.size(); ++i)
			{
				if (IsTransparent(items[i].codepoint))
					items[i].bidi = items[i - 1].bidi;
			}

			// Separators between digits are part of the number (1,000 / 3.5 / 10:30)
			for (std::size_t i = 1; i + 1 < items.size(); ++i)
			{
				const auto c = items[i].codepoint;
				if ((c == '.' || c == ',' || c == ':' || c == '/') && items[i - 1].bidi == BidiClass::Number && items[i + 1].bidi == BidiClass::Number)
					items[i].bidi = BidiClass::Number;
			}

			// Numbers act as left-to-right after Latin text, otherwise as right-to-left for neutral resolution
			std::vector<BidiClass> strong(items.size());
			auto lastStrong = rtlParagraph ? BidiClass::R : BidiClass::L;
			for (std::size_t i = 0; i < items.size(); ++i)
			{
				const auto bidi = items[i].bidi;
				if (bidi == BidiClass::L || bidi == BidiClass::R)
				{
					lastStrong = bidi;
					strong[i] = bidi;
				}
				else if (bidi == BidiClass::Number)
				{
					strong[i] = lastStrong == BidiClass::L ? BidiClass::L : BidiClass::R;
				}
				else
				{
					strong[i] = BidiClass::Neutral;
				}
			}

			// Neutrals between two runs of the same direction take that direction, otherwise the paragraph's
			for (std::size_t i = 0; i < items.size();)
			{
				if (strong[i] != BidiClass::Neutral)
				{
					++i;
					continue;
				}

				auto end = i;
				while (end < items.size() && strong[end] == BidiClass::Neutral) ++end;

				const auto before = i > 0 ? strong[i - 1] : (rtlParagraph ? BidiClass::R : BidiClass::L);
				const auto after = end < items.size() ? strong[end] : (rtlParagraph ? BidiClass::R : BidiClass::L);
				const auto resolved = before == after ? before : (rtlParagraph ? BidiClass::R : BidiClass::L);

				for (; i < end; ++i) strong[i] = resolved;
			}

			for (std::size_t i = 0; i < items.size(); ++i)
			{
				if (items[i].bidi == BidiClass::Number)
					items[i].level = strong[i] == BidiClass::L ? levelL : 2;
				else
					items[i].level = strong[i] == BidiClass::L ? levelL : levelR;
			}

			// Trailing whitespace stays at the paragraph level
			for (auto i = items.size(); i > 0 && items[i - 1].codepoint == ' '; --i)
				items[i - 1].level = baseLevel;
		}

		void Reorder(std::vector<Item>& items)
		{
			auto maxLevel = 0;
			for (const auto& item : items) maxLevel = std::max(maxLevel, item.level);

			for (auto level = maxLevel; level >= 1; --level)
			{
				for (std::size_t i = 0; i < items.size();)
				{
					if (items[i].level < level)
					{
						++i;
						continue;
					}

					auto end = i;
					while (end < items.size() && items[end].level >= level) ++end;
					std::reverse(items.begin() + i, items.begin() + end);
					i = end;
				}
			}
		}

		bool IsRtlParagraph(const std::vector<Item>& items, const bool fallback)
		{
			for (const auto& item : items)
			{
				const auto bidi = Classify(item);
				if (bidi == BidiClass::R) return true;
				if (bidi == BidiClass::L) return false;
			}
			return fallback;
		}

		// spans receives the output range of every item, so positions can be mapped back to the input
		void Emit(const std::vector<Item>& items, std::string& color, const std::string& resetColorCode, std::string& out,
			std::vector<std::pair<std::size_t, std::size_t>>* spans = nullptr)
		{
			for (const auto& item : items)
			{
				const auto c = item.codepoint;
				if (c == ZWNJ || c == ZWJ || c == LRM || c == RLM)
				{
					if (spans) spans->emplace_back(out.size(), out.size());
					continue;
				}

				if (item.color != color)
				{
					out += item.color.empty() ? resetColorCode : item.color;
					color = item.color;
				}

				const auto itemStart = out.size();

				if (c == 0)
					out += item.raw;
				else if (c < 0x80)
					out += static_cast<char>(item.level % 2 ? Mirror(c) : c);
				else
					EncodeUtf8(item.level % 2 ? Mirror(c) : c, out);

				if (spans) spans->emplace_back(itemStart, out.size());
			}
		}

		// Where a cursor before the input byte at position cursor appears in the emitted line
		std::optional<std::size_t> MapCursor(const std::vector<Item>& line, const std::vector<std::pair<std::size_t, std::size_t>>& spans,
			const std::size_t cursor, const bool lastLine)
		{
			// The cursor sits before the first character at or after it in reading order
			std::optional<std::size_t> next;
			std::optional<std::size_t> last;
			for (std::size_t i = 0; i < line.size(); ++i)
			{
				if (line[i].source >= cursor && (!next || line[i].logical < line[*next].logical)) next = i;
				if (!last || line[i].logical > line[*last].logical) last = i;
			}

			// Before a right-to-left character means on its right side
			if (next) return line[*next].level % 2 ? spans[*next].second : spans[*next].first;
			if (!lastLine) return {};

			// At the end of the text, after the last character in reading order
			if (last) return line[*last].level % 2 ? spans[*last].first : spans[*last].second;
			return {};
		}
	}

	std::size_t DecodeUtf8(const char* text, std::uint32_t* codepoint)
	{
		const auto* s = reinterpret_cast<const unsigned char*>(text);
		const auto lead = s[0];

		std::size_t length;
		std::uint32_t value;
		if (lead >= 0xC2 && lead <= 0xDF) { length = 2; value = lead & 0x1F; }
		else if (lead >= 0xE0 && lead <= 0xEF) { length = 3; value = lead & 0x0F; }
		else if (lead >= 0xF0 && lead <= 0xF4) { length = 4; value = lead & 0x07; }
		else return 0;

		for (std::size_t i = 1; i < length; ++i)
		{
			if ((s[i] & 0xC0) != 0x80) return 0;
			value = (value << 6) | (s[i] & 0x3F);
		}

		// Reject overlong encodings, surrogates and out of range values
		if ((length == 3 && value < 0x800) || (length == 4 && (value < 0x10000 || value > 0x10FFFF)) || (value >= 0xD800 && value <= 0xDFFF))
			return 0;

		*codepoint = value;
		return length;
	}

	void EncodeUtf8(const std::uint32_t codepoint, std::string& out)
	{
		if (codepoint < 0x80)
		{
			out += static_cast<char>(codepoint);
		}
		else if (codepoint < 0x800)
		{
			out += static_cast<char>(0xC0 | (codepoint >> 6));
			out += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
		else if (codepoint < 0x10000)
		{
			out += static_cast<char>(0xE0 | (codepoint >> 12));
			out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
			out += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
		else
		{
			out += static_cast<char>(0xF0 | (codepoint >> 18));
			out += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
			out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
			out += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
	}

	bool ContainsRtl(const char* text)
	{
		if (!text) return false;

		while (*text)
		{
			std::uint32_t codepoint;
			if (const auto length = DecodeUtf8(text, &codepoint); length > 0)
			{
				if (IsRtl(codepoint)) return true;
				text += length;
			}
			else
			{
				++text;
			}
		}

		return false;
	}

	std::string ProcessForDisplay(const char* text, const Tokenizer& tokenizer, const std::string& resetColorCode, int* cursor)
	{
		std::string result;
		std::string inputColor, outputColor;

		// Tokenize before splitting lines, markup such as material icons can contain line break bytes
		const auto items = Tokenize(text, tokenizer, resetColorCode, inputColor);

		// Lines without strong characters inherit the direction of the text before them
		auto rtlParagraph = IsRtlParagraph(items, false);

		auto cursorPlaced = false;
		std::size_t mappedCursor = 0;

		auto lineStart = items.begin();
		while (true)
		{
			const auto lineEnd = std::find_if(lineStart, items.end(), [](const Item& item)
			{
				return !item.object && (item.codepoint == '\r' || item.codepoint == '\n');
			});

			std::vector<Item> line(lineStart, lineEnd);
			rtlParagraph = IsRtlParagraph(line, rtlParagraph);

			Shape(line);
			for (std::size_t i = 0; i < line.size(); ++i)
			{
				line[i].logical = i;
			}

			ResolveLevels(line, rtlParagraph);
			Reorder(line);

			const auto lineOffset = result.size();
			std::vector<std::pair<std::size_t, std::size_t>> spans;
			Emit(line, outputColor, resetColorCode, result, &spans);

			if (cursor && !cursorPlaced)
			{
				if (const auto position = MapCursor(line, spans, static_cast<std::size_t>(*cursor), lineEnd == items.end()))
				{
					mappedCursor = *position;
					cursorPlaced = true;
				}
				else if (line.empty() && lineEnd == items.end())
				{
					mappedCursor = lineOffset;
					cursorPlaced = true;
				}
			}

			if (lineEnd == items.end())
			{
				break;
			}

			// The line break carries the color for the text after it
			Emit({*lineEnd}, outputColor, resetColorCode, result);
			lineStart = lineEnd + 1;
		}

		if (cursor)
		{
			*cursor = static_cast<int>(cursorPlaced ? mappedCursor : result.size());
		}

		return result;
	}
}
