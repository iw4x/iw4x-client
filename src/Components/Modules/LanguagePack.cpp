#include "LanguagePack.hpp"
#include "ZoneBuilder.hpp"

#include <stb_truetype.hpp>

namespace Components
{
	namespace
	{
		namespace fs = std::filesystem;

		struct Language
		{
			const char* folder;
			const char* prefix;
			unsigned int codePage; // 0 keeps the glyph codes: the game's English text is single-byte Windows-1252
		};

		// English and British are the game's own text, only the English console fonts are imported.
		// British is a copy of English.
		constexpr Language LANGUAGES[]
		{
			{ "english", "en", 0 },
			{ "german", "de", 1252 },
			{ "italian", "it", 1252 },
			{ "spanish", "es", 1252 },
			{ "polish", "pl", 1250 },
			{ "russian", "ru", 1251 },
			{ "japanese", "ja", 932 },
			{ "korean", "ko", 949 },
		};

		// Mistakes in the official translations
		const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> FIXES
		{
			// The stance hints are shifted by one in the Polish strings (jump says "lie down" and so on)
			{ "polish", {
				{ "PLATFORM_STANCEHINT_JUMP", "Naciśnij &&1, aby skoczyć" },
				{ "PLATFORM_STANCEHINT_STAND", "Naciśnij &&1, aby wstać" },
				{ "PLATFORM_STANCEHINT_PRONE", "Naciśnij &&1, aby się położyć" },
			} },
		};

		const char* DEFAULT_PACK = "languagepack";
		const char* FONT_DIR = "userraw/fonts";
		const char* IMAGE_DIR = "userraw/images";
		const char* STRING_DIR = "userraw/localizedstrings";

		// Codes the console fonts use for controller buttons rather than letters (0x01-0x1F are buttons too).
		// The game sends them as raw bytes, so they keep their code in every language.
		bool IsButtonCode(const std::uint32_t code)
		{
			return code == 0xBC || code == 0xBD;
		}

		bool IsButtonPicture(const std::uint32_t code)
		{
			return code < 0x20 || IsButtonCode(code);
		}

		std::optional<std::uint32_t> DecodeLegacy(const std::string& bytes, const unsigned int codePage)
		{
			wchar_t wide[4];
			if (MultiByteToWideChar(codePage, MB_ERR_INVALID_CHARS, bytes.data(), static_cast<int>(bytes.size()), wide, 4) != 1)
			{
				return {};
			}

			return static_cast<std::uint32_t>(wide[0]);
		}

		std::string ToUtf8(const std::string& bytes, const unsigned int codePage)
		{
			if (bytes.empty())
			{
				return {};
			}

			const auto wideLength = MultiByteToWideChar(codePage, 0, bytes.data(), static_cast<int>(bytes.size()), nullptr, 0);
			std::wstring wide(wideLength, L'\0');
			MultiByteToWideChar(codePage, 0, bytes.data(), static_cast<int>(bytes.size()), wide.data(), wideLength);

			const auto length = WideCharToMultiByte(CP_UTF8, 0, wide.data(), wideLength, nullptr, 0, nullptr, nullptr);
			std::string text(length, '\0');
			WideCharToMultiByte(CP_UTF8, 0, wide.data(), wideLength, text.data(), length, nullptr, nullptr);
			return text;
		}

		std::string ReadFile(const fs::path& path)
		{
			std::ifstream stream(path, std::ios::binary);
			return { std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };
		}

		void WriteJson(const fs::path& path, const nlohmann::json& json)
		{
			fs::create_directories(path.parent_path());
			Utils::IO::WriteFile(path.string(), json.dump(1, ' ', false, nlohmann::json::error_handler_t::replace));
		}

		nlohmann::json ReadJson(const fs::path& path)
		{
			auto text = ReadFile(path);
			if (text.starts_with("\xEF\xBB\xBF"))
			{
				text.erase(0, 3);
			}

			return nlohmann::json::parse(text);
		}

		std::string Trimmed(std::string text)
		{
			Utils::String::Trim(text);
			return text;
		}

		void ReplaceAll(std::string& text, const std::string& from, const std::string& to)
		{
			for (auto pos = text.find(from); pos != std::string::npos; pos = text.find(from, pos + to.size()))
			{
				text.replace(pos, from.size(), to);
			}
		}

		bool EndsWithClosingQuote(const std::string& value)
		{
			const auto end = value.find_last_not_of(" \t");
			return end != std::string::npos && value[end] == '"' && (end == 0 || value[end - 1] != '\\');
		}

		// A .str file: REFERENCE <key> followed by LANG_<language> "<text>", the text can go on over several lines
		std::map<std::string, std::string> ParseStr(const fs::path& path, const unsigned int codePage)
		{
			std::vector<std::string> lines;
			std::istringstream stream(ReadFile(path));
			for (std::string line; std::getline(stream, line);)
			{
				if (!line.empty() && line.back() == '\r')
				{
					line.pop_back();
				}
				lines.push_back(std::move(line));
			}

			std::map<std::string, std::string> strings;
			std::string key;
			for (std::size_t i = 0; i < lines.size();)
			{
				const auto& line = lines[i++];
				if (line.starts_with("REFERENCE"))
				{
					key = Trimmed(line.substr(std::strlen("REFERENCE")));
					continue;
				}

				const auto quote = line.find('"');
				if (!line.starts_with("LANG_") || key.empty() || quote == std::string::npos)
				{
					continue;
				}

				auto value = line.substr(quote + 1);
				while (!EndsWithClosingQuote(value) && i < lines.size() && !lines[i].starts_with("REFERENCE") && !lines[i].starts_with("LANG_"))
				{
					value += "\n" + lines[i++];
				}

				// A double-byte letter can end in 0x5C ('\') before the closing quote (Shift-JIS), which reads as an
				// escaped quote and pulls in the empty line after the entry
				value.erase(value.find_last_not_of(" \t\r\n") + 1);
				if (value.ends_with('"'))
				{
					value.pop_back();
				}

				auto text = ToUtf8(value, codePage);
				ReplaceAll(text, "\\n", "\n");
				ReplaceAll(text, "\\\"", "\"");
				strings[key] = text;
				key.clear();
			}

			return strings;
		}

		void AddCodepoints(const std::string& text, std::set<std::uint32_t>& codepoints)
		{
			for (const auto* c = text.data(); *c;)
			{
				std::uint32_t codepoint;
				if (const auto length = Utils::Arabic::DecodeUtf8(c, &codepoint))
				{
					codepoints.insert(codepoint);
					c += length;
				}
				else
				{
					codepoints.insert(static_cast<unsigned char>(*c++));
				}
			}
		}

		// The game indexes 32-127 directly and binary searches the rest
		nlohmann::json GameLayout(const std::map<std::uint32_t, nlohmann::json>& glyphs)
		{
			auto layout = nlohmann::json::array();
			for (std::uint32_t letter = 32; letter <= 127; ++letter)
			{
				if (const auto glyph = glyphs.find(letter); glyph != glyphs.end())
				{
					layout.push_back(glyph->second);
				}
			}

			for (const auto& [letter, glyph] : glyphs)
			{
				if (letter < 32 || letter > 127)
				{
					layout.push_back(glyph);
				}
			}

			return layout;
		}

		// Official glyph table with Unicode letters, for the font's own texture
		nlohmann::json ConvertFont(const nlohmann::json& font, const std::string& stockName, const Language& language)
		{
			std::map<std::uint32_t, nlohmann::json> glyphs;
			for (const auto& glyph : font["glyphs"])
			{
				const auto code = glyph["letter"].get<std::uint32_t>();

				std::optional<std::uint32_t> letter = code;
				if (code >= 0x80 && !IsButtonCode(code) && language.codePage)
				{
					const auto bytes = code <= 0xFF ? std::string(1, static_cast<char>(code)) : std::string{ static_cast<char>(code >> 8), static_cast<char>(code & 0xFF) };
					letter = DecodeLegacy(bytes, language.codePage);
				}

				if (letter && *letter <= 0xFFFF)
				{
					auto converted = glyph;
					converted["letter"] = *letter;
					glyphs[*letter] = converted;
				}
			}

			return {
				{ "baseFont", std::format("fonts/{}", stockName) },
				{ "image", std::format("gamefonts_{}", language.prefix) },
				{ "pixelHeight", font["pixelHeight"] },
				{ "glyphs", GameLayout(glyphs) },
			};
		}

		// Characters a legacy double-byte code page encodes in the given lead and trail byte ranges
		std::set<std::uint32_t> LegacyCharset(const unsigned int codePage, const int firstLead, const int lastLead, const int firstTrail, const int lastTrail)
		{
			std::set<std::uint32_t> chars;
			for (auto lead = firstLead; lead <= lastLead; ++lead)
			{
				for (auto trail = firstTrail; trail <= lastTrail; ++trail)
				{
					if (const auto letter = DecodeLegacy({ static_cast<char>(lead), static_cast<char>(trail) }, codePage))
					{
						chars.insert(*letter);
					}
				}
			}

			return chars;
		}

		// Font definition for ZoneBuilder that packs a TrueType font, keeping only letters the font has
		nlohmann::json TrueTypeFont(const fs::path& fontFile, std::set<std::uint32_t> charset, nlohmann::json definition)
		{
			const auto data = ReadFile(fontFile);
			stbtt_fontinfo info{};
			if (!stbtt_InitFont(&info, reinterpret_cast<const unsigned char*>(data.data()), stbtt_GetFontOffsetForIndex(reinterpret_cast<const unsigned char*>(data.data()), 0)))
			{
				throw std::runtime_error(std::format("{} is not a font", fontFile.string()));
			}

			// ZoneBuilder requires 32-127, DEL falls back to .notdef and is never drawn
			for (std::uint32_t c = 32; c < 128; ++c)
			{
				charset.insert(c);
			}

			auto letters = nlohmann::json::array();
			for (const auto letter : charset)
			{
				if (letter <= 0xFFFF && (letter < 128 || stbtt_FindGlyphIndex(&info, static_cast<int>(letter))))
				{
					letters.push_back(letter);
				}
			}

			definition["charset"] = letters;
			return definition;
		}

		void InsertRange(std::set<std::uint32_t>& set, const std::uint32_t first, const std::uint32_t last)
		{
			for (auto c = first; c <= last; ++c)
			{
				set.insert(c);
			}
		}

#ifdef __XENON_UI_BINDS
		bool HasButton(const std::string& text)
		{
			return std::ranges::any_of(text, [](const char c) { return (c >= 0x01 && c <= 0x06) || (c >= 0x0E && c <= 0x17); });
		}

		std::string Buttons(const std::string& text)
		{
			std::string buttons;
			for (const auto c : text)
			{
				if (!((c >= 0x01 && c <= 0x06) || (c >= 0x0E && c <= 0x17)))
				{
					break;
				}
				buttons += c;
			}
			return buttons;
		}

		// Prompts whose Arabic text talks about the mouse, written again for the gamepad
		const std::unordered_map<std::string, std::string> WRITTEN_PROMPTS
		{
			{ "PLATFORM_LOCSEL_DIR_CONTROLS", "\xD8\xAD\xD8\xAF\xD8\xAF \xD8\xA7\xD9\x84\xD8\xA7\xD8\xAA\xD8\xAC\xD8\xA7\xD9\x87 \xD8\xA8\xD8\xA7\xD8\xB3\xD8\xAA\xD8\xAE\xD8\xAF\xD8\xA7\xD9\x85 \x11" },
			{ "PLATFORM_LOCSEL_POSITION_CONTROLS", "\xD8\xAD\xD8\xAF\xD8\xAF \xD8\xA7\xD9\x84\xD9\x85\xD9\x88\xD9\x82\xD8\xB9 \xD8\xA8\xD8\xA7\xD8\xB3\xD8\xAA\xD8\xAE\xD8\xAF\xD8\xA7\xD9\x85 \x10" },
		};

		// The Arabic prompt with its keyboard key turned into the button the Xbox string shows
		std::optional<std::string> XboxArabicPrompt(const std::string& arabic, const std::string& xboxEnglish)
		{
			// Bindings (&&1, [{+activate}]) already show the button the player uses
			if (arabic.find("&&") != std::string::npos || arabic.find("[{") != std::string::npos)
			{
				return {};
			}

			// A button after the text, e.g. "text    <B>", goes right next to it on the right side
			if (HasButton(arabic))
			{
				std::string reversedArabic(arabic.rbegin(), arabic.rend());
				auto buttons = Buttons(reversedArabic);
				std::ranges::reverse(buttons);
				const auto text = Trimmed(arabic.substr(0, arabic.size() - buttons.size()));
				if (buttons.empty() || text.empty() || HasButton(text))
				{
					return {};
				}
				return std::format("{} {}", buttons, text);
			}

			const auto leading = Buttons(xboxEnglish);
			std::string reversed(xboxEnglish.rbegin(), xboxEnglish.rend());
			auto trailing = Buttons(reversed);
			std::ranges::reverse(trailing);
			if (leading.empty() && trailing.empty())
			{
				return {};
			}

			static const std::regex keys(R"((ESCAPE|ESC|F\d{1,2}|DEL|ENTER|BACKSPACE)(?![A-Za-z0-9]))");
			static const std::regex colors(R"(\^\d)");
			static const std::regex dashes(R"(^\s*-\s*|\s*-\s*$)");
			auto core = std::regex_replace(std::regex_replace(arabic, keys, ""), colors, "");
			core = Trimmed(std::regex_replace(Trimmed(core), dashes, ""));
			if (core.empty())
			{
				return {};
			}

			// The button comes first, which right-to-left text shows on the right, right next to the word
			return std::format("{} {}", leading.empty() ? trailing : leading, core);
		}
#endif
	}

	void LanguagePack::BuildZone(const std::string& zone, const std::vector<std::string>& fonts)
	{
		std::string csv;
		for (const auto& font : fonts)
		{
			csv += std::format("font,fonts/{}\n", font);
		}

		fs::create_directories("zone_source");
		Utils::IO::WriteFile(std::format("zone_source/{}.csv", zone), csv);

		ZoneBuilder::Zone(zone).build();

		const auto built = fs::path("zonebuilder_out") / (zone + ".ff");
		const auto target = fs::path("zone/english") / (zone + ".ff");
		fs::create_directories(target.parent_path());
		fs::copy_file(built, target, fs::copy_options::overwrite_existing);

		RemoveTemporaryFiles();

		Logger::Print("Installed {}\n", target.string());
	}

	void LanguagePack::RemoveTemporaryFiles()
	{
		// Font sources and the zone build, the game only needs the zones and the textures
		std::error_code error;
		fs::remove_all(FONT_DIR, error);
		fs::remove_all("zone_source", error);
		fs::remove_all("zonebuilder_out", error);
	}

	void LanguagePack::BuildLanguages(const fs::path& pack, const fs::path& noto)
	{
		if (!fs::exists(pack / "english" / "fonts"))
		{
			throw std::runtime_error(std::format("{} is not a language pack (no english\\fonts folder)", pack.string()));
		}

		for (const auto* file : { "NotoSans.ttf", "NotoSansJP-Regular.otf", "NotoSansKR-Regular.otf" })
		{
			if (!fs::exists(noto / file))
			{
				throw std::runtime_error(std::format("{} is missing", (noto / file).string()));
			}
		}

		fs::create_directories(FONT_DIR);
		fs::create_directories(IMAGE_DIR);

		std::vector<std::string> fonts;
		std::set<std::uint32_t> japaneseChars, koreanChars;

		for (const auto& language : LANGUAGES)
		{
			const auto folder = pack / language.folder;
			if (!fs::exists(folder))
			{
				Logger::Print("Skipping {}, not found\n", language.folder);
				continue;
			}

			if (language.codePage)
			{
				auto strings = ParseStr(folder / "localizedstrings" / "iw4mp.str", language.codePage);
				if (const auto fixes = FIXES.find(language.folder); fixes != FIXES.end())
				{
					for (const auto& [key, text] : fixes->second)
					{
						strings[key] = text;
					}
				}

				auto& usedChars = language.codePage == 932 ? japaneseChars : koreanChars;
				if (language.codePage == 932 || language.codePage == 949)
				{
					for (const auto& [key, text] : strings)
					{
						AddCodepoints(text, usedChars);
					}
				}

				WriteJson(fs::path(STRING_DIR) / std::format("{}.json", language.folder), strings);
				Logger::Print("{}: {} strings\n", language.folder, strings.size());
			}

			fs::copy_file(folder / "images" / "gamefonts_pc.iwi", fs::path(IMAGE_DIR) / std::format("gamefonts_{}.iwi", language.prefix), fs::copy_options::overwrite_existing);

			for (const auto& entry : fs::directory_iterator(folder / "fonts"))
			{
				if (entry.path().extension() != ".json")
				{
					continue;
				}

				const auto stockName = entry.path().stem().string();
				const auto name = std::format("{}_{}", language.prefix, stockName);
				WriteJson(fs::path(FONT_DIR) / (name + ".json"), ConvertFont(ReadJson(entry.path()), stockName, language));
				fonts.push_back(name);
			}
		}

		// Button pictures of the English console font, for fonts without them (Arabic, backup fonts)
		{
			const auto english = pack / "english";
			const auto font = ReadJson(english / "fonts" / "normalFont.json");

			std::map<std::uint32_t, nlohmann::json> glyphs;
			for (const auto& glyph : font["glyphs"])
			{
				const auto letter = glyph["letter"].get<std::uint32_t>();
				if ((letter >= 32 && letter <= 127) || IsButtonPicture(letter))
				{
					glyphs[letter] = glyph;
				}
			}

			fs::copy_file(english / "images" / "gamefonts_pc.iwi", fs::path(IMAGE_DIR) / "gamefonts_buttons.iwi", fs::copy_options::overwrite_existing);
			WriteJson(fs::path(FONT_DIR) / "fb_buttons.json", {
				{ "image", "gamefonts_buttons" },
				{ "pixelHeight", font["pixelHeight"] },
				{ "glyphs", GameLayout(glyphs) },
			});
			fonts.emplace_back("fb_buttons");
		}

		// Noto backup fonts cover what players type in chat as well as letters the official fonts don't have
		{
			const nlohmann::json definition{ { "size", 28 }, { "glyphScale", 1.0 }, { "yOffset", 0 } };

			std::set<std::uint32_t> latin;
			InsertRange(latin, 0xA0, 0x24F);  // Latin
			InsertRange(latin, 0x370, 0x52F); // Greek, Cyrillic
			InsertRange(latin, 0x1E00, 0x1EFF);
			InsertRange(latin, 0x2000, 0x206F); // Punctuation
			InsertRange(latin, 0x20A0, 0x20BF); // Currency

			// Shift-JIS symbols, kana and level 1 kanji; KS X 1001 symbols and the 2350 common syllables
			auto japanese = LegacyCharset(932, 0x81, 0x98, 0x40, 0xFC);
			japanese.insert(japaneseChars.begin(), japaneseChars.end());
			auto korean = LegacyCharset(949, 0xA1, 0xC8, 0xA1, 0xFE);
			korean.insert(koreanChars.begin(), koreanChars.end());

			const std::tuple<const char*, const char*, const std::set<std::uint32_t>&> backups[]
			{
				{ "fb_latin", "NotoSans.ttf", latin },
				{ "fb_ja", "NotoSansJP-Regular.otf", japanese },
				{ "fb_ko", "NotoSansKR-Regular.otf", korean },
			};

			for (const auto& [name, file, charset] : backups)
			{
				const auto source = noto / file;
				fs::copy_file(source, fs::path(FONT_DIR) / std::format("{}.ttf", name), fs::copy_options::overwrite_existing);
				const auto backup = TrueTypeFont(source, charset, definition);
				WriteJson(fs::path(FONT_DIR) / std::format("{}.json", name), backup);
				fonts.emplace_back(name);
				Logger::Print("{}: {} letters\n", name, backup["charset"].size());
			}
		}

#ifdef __XENON_UI_BINDS
		// Xbox button prompts: the Xbox English strings that show buttons, and the Arabic prompts made the same way
		{
			const auto xbox = ParseStr(pack / "english" / "localizedstrings" / "iw4mp.str", 1252);
			nlohmann::json english = nlohmann::json::object();
			for (const auto& [key, text] : xbox)
			{
				if (HasButton(text))
				{
					english[key] = text;
				}
			}
			WriteJson(fs::path(STRING_DIR) / "english.json", english);

			const auto arabicPath = fs::path(STRING_DIR) / "arabic.json";
			if (fs::exists(arabicPath))
			{
				auto arabic = ReadJson(arabicPath);
				auto changed = 0;
				for (const auto& [key, text] : english.items())
				{
					if (const auto written = WRITTEN_PROMPTS.find(key); written != WRITTEN_PROMPTS.end())
					{
						arabic[key] = written->second;
						++changed;
					}
					else if (arabic.contains(key))
					{
						if (const auto prompt = XboxArabicPrompt(arabic[key].get<std::string>(), text.get<std::string>()))
						{
							arabic[key] = *prompt;
							++changed;
						}
					}
				}
				WriteJson(arabicPath, arabic);
				Logger::Print("{} Xbox prompts, {} Arabic prompts turned into button prompts\n", english.size(), changed);
			}
		}
#endif

		BuildZone("iw4x_languages", fonts);
	}

	void LanguagePack::BuildArabicFonts(const fs::path& font)
	{
		fs::create_directories(FONT_DIR);

		std::set<std::uint32_t> charset;
		InsertRange(charset, 0xA0, 0xFF);
		for (const auto c : { 0x060C, 0x061B, 0x061F, 0x0640, 0x0670, 0x00AB, 0x00BB })
		{
			charset.insert(c);
		}
		InsertRange(charset, 0x064B, 0x0652); // Harakat
		InsertRange(charset, 0x0660, 0x066D); // Arabic-Indic digits and number signs
		InsertRange(charset, 0x06F0, 0x06F9); // Persian digits
		for (const auto form : Utils::Arabic::GetPresentationForms())
		{
			charset.insert(form);
		}

		// Kufi's capitals line up with the stock font's, a little smaller so its deep descenders stay clear of the line below
		const nlohmann::json definition{ { "alignToBaseFont", true }, { "capitalScale", 0.9 } };

		std::vector<std::string> fonts;
		for (const auto* stockName : { "smallFont", "normalFont", "boldFont", "bigFont", "extraBigFont", "objectiveFont", "hudBigFont", "hudSmallFont" })
		{
			const auto name = std::format("ar_{}", stockName);
			auto fontDefinition = TrueTypeFont(font, charset, definition);
			fontDefinition["baseFont"] = std::format("fonts/{}", stockName);

			fs::copy_file(font, fs::path(FONT_DIR) / (name + ".ttf"), fs::copy_options::overwrite_existing);
			WriteJson(fs::path(FONT_DIR) / (name + ".json"), fontDefinition);
			fonts.push_back(name);
		}

		BuildZone("iw4x_arabic", fonts);
	}

	LanguagePack::LanguagePack()
	{
		if (!ZoneBuilder::IsEnabled())
		{
			return;
		}

		Command::Add("buildlanguages", [](const Command::Params* params)
		{
			// Default: the pack in <game>\languagepack, the Noto fonts in its noto folder
			const fs::path pack = params->size() > 1 ? params->get(1) : DEFAULT_PACK;
			const fs::path noto = params->size() > 2 ? fs::path(params->get(2)) : pack / "noto";

			try
			{
				BuildLanguages(pack, noto);
			}
			catch (const std::exception& ex)
			{
				RemoveTemporaryFiles();
				Logger::PrintError(Game::CON_CHANNEL_ERROR, "buildlanguages failed: {}\n", ex.what());
			}
		});

		Command::Add("buildarabicfonts", [](const Command::Params* params)
		{
			// Default: <game>\languagepack\NotoKufiArabic.ttf
			const fs::path font = params->size() > 1 ? fs::path(params->join(1)) : fs::path(DEFAULT_PACK) / "NotoKufiArabic.ttf";

			try
			{
				BuildArabicFonts(font);
			}
			catch (const std::exception& ex)
			{
				RemoveTemporaryFiles();
				Logger::PrintError(Game::CON_CHANNEL_ERROR, "buildarabicfonts failed: {}\n", ex.what());
			}
		});
	}
}
