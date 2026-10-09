#include "XenonModule.hpp"
#include "Scheduler.hpp"

namespace Components
{
	static float g_xenonHudElemFontScaleLarge = __XENON_UI_BIGFONT;
	static float g_xenonHudElemFontScaleSmall = __XENON_UI_SMALLFONT;

#ifdef __XENON_UI_THUMBSTICK
	Utils::Hook Xenon::s_uiReplaceConversionsHook;

	void Xenon::UI_ReplaceConversions_Hk(const char* sourceString, Game::ConversionArguments* arguments, char* outputString, size_t outputStringSize)
	{
		s_uiReplaceConversionsHook.uninstall();
		Game::UI_ReplaceConversions(sourceString, arguments, outputString, outputStringSize);
		s_uiReplaceConversionsHook.install();
		Game::UI_FilterStringForButtonAnimation(outputString, static_cast<unsigned int>(outputStringSize));
	}
#endif

#ifdef __JSON_FONTLOADER
	std::unordered_map<std::string, std::vector<Game::Glyph>> Xenon::s_fontGlyphStorage;
	std::unordered_map<std::string, int> Xenon::s_fontPixelHeightStorage;
	Utils::Hook Xenon::s_registerFontHook;

	void Xenon::ApplyFontJson(Game::Font_s* font, const char* jsonPath)
	{
		// R_RegisterFont calls on every map load etc — cache the built glyph array so we
		// only read/parse the JSON once, then just re-point the font on subsequent calls.
		const auto cached = s_fontGlyphStorage.find(jsonPath);
		if (cached != s_fontGlyphStorage.end())
		{
			if (const auto cachedPixelHeight = s_fontPixelHeightStorage.find(jsonPath); cachedPixelHeight != s_fontPixelHeightStorage.end())
			{
				font->pixelHeight = cachedPixelHeight->second;
			}

			font->glyphs = cached->second.data();
			font->glyphCount = static_cast<int>(cached->second.size());
			XLOG("Font JSON '{}' served from cache ({} glyphs)\n", jsonPath, font->glyphCount);
			return;
		}

		char* buffer = nullptr;
		const auto len = Game::FS_ReadFile(jsonPath, &buffer);
		if (len <= 0 || !buffer)
			return;

		try
		{
			const auto json = nlohmann::json::parse(buffer, buffer + len);

			if (json.contains("pixelHeight"))
			{
				const auto pixelHeight = json.at("pixelHeight").get<int>();
				font->pixelHeight = pixelHeight;
				s_fontPixelHeightStorage[jsonPath] = pixelHeight;
			}

			const auto& glyphArray = json.at("glyphs");

			//build a working copy of the current glyphs
			std::vector<Game::Glyph> glyphs(font->glyphs, font->glyphs + font->glyphCount);

			for (const auto& entry : glyphArray)
			{
				const unsigned short letter = entry.at("letter").get<unsigned short>();

				auto it = std::find_if(glyphs.begin(), glyphs.end(), [letter](const Game::Glyph& g)
				{
					return g.letter == letter;
				});

				Game::Glyph g{};
				if (it != glyphs.end())
					g = *it;

				g.letter = letter;

				auto tryGet = [&](const char* key, auto& field)
				{
					if (entry.contains(key))
						field = entry[key].get<std::decay_t<decltype(field)>>();
				};

				tryGet("x0", g.x0);
				tryGet("y0", g.y0);
				tryGet("dx", g.dx);
				tryGet("pixelWidth", g.pixelWidth);
				tryGet("pixelHeight", g.pixelHeight);
				tryGet("s0", g.s0);
				tryGet("t0", g.t0);
				tryGet("s1", g.s1);
				tryGet("t1", g.t1);

				if (it != glyphs.end())
					*it = g;
				else
					glyphs.push_back(g);
			}
			auto& stored = (s_fontGlyphStorage[jsonPath] = std::move(glyphs));
			font->glyphs = stored.data();
			font->glyphCount = static_cast<int>(stored.size());

			XLOG("Applied font JSON '{}' ({} glyphs total)\n", jsonPath, font->glyphCount);
		}
		catch (const std::exception& e)
		{
			XLOG("Failed to parse font JSON '{}': {}\n", jsonPath, e.what());
		}

		Game::FS_FreeFile(buffer);
	}

	Game::Font_s* Xenon::R_RegisterFont_Hk(const char* asset, int safe)
	{
		s_registerFontHook.uninstall();
		auto* font = Utils::Hook::Call<Game::Font_s*(const char*, int)>(0x505670)(asset, safe);
		s_registerFontHook.install();

		if (!font || !asset || !*asset)
			return font;

		// A translation's font (LanguageFonts) has its own glyph table for its own texture
		if (font->fontName && Utils::String::ToLower(font->fontName) != Utils::String::ToLower(asset))
			return font;

		const auto jsonPath = std::string(asset) + ".json";
		ApplyFontJson(font, jsonPath.c_str());

		return font;
	}
#endif

	void Xenon::PatchDvars()
	{
		PATCH_DVAR("cg_scoreboardFont", integer, 3);
		PATCH_DVAR("cg_scoreboardHeaderFontScale", value, 0.30000001f);
		PATCH_DVAR("cg_scoreboardRankFontScale", value, 0.30000001f);
		PATCH_DVAR("cg_overheadNamesFarScale", value, 0.5f);
		PATCH_DVAR("perk_extendedMeleeRange", value, 192.0f);
		PATCH_DVAR("cl_packetdup", integer, 2);
		PATCH_DVAR("rate", integer, 20000);
		PATCH_DVAR("sv_floodProtect", enabled, false);
		PATCH_DVAR("sv_timeout", integer, 10);
		PATCH_DVAR("sv_connectTimeout", integer, 40);
		
	}
	void Xenon::PatchMemory()
	{
#ifdef __XENON_SCOREBOARD
		// remove pc-only +15 y offset added in sub_591B70 before drawing scoreboard rows
		Utils::Hook::Nop(0x591C3D, 6);

		// Console draws only up/down arrows. These two PC-only calls draw PageUp/PageDown key icons.
		Utils::Hook::Nop(0x5919F5, 5); // UI_DrawHandlePic("hudscoreboardscroll_upkey")
		Utils::Hook::Nop(0x591B58, 5); // UI_DrawHandlePic("hudscoreboardscroll_downkey")
#endif
		
		// apply hud elem font scale values from console
		Utils::Hook::Set<DWORD>(0x587F01, reinterpret_cast<DWORD>(&g_xenonHudElemFontScaleLarge));
		Utils::Hook::Set<DWORD>(0x587F0E, reinterpret_cast<DWORD>(&g_xenonHudElemFontScaleSmall));
		Utils::Hook::Set<DWORD>(0x587F30, reinterpret_cast<DWORD>(&g_xenonHudElemFontScaleLarge));
		Utils::Hook::Set<DWORD>(0x587F3D, reinterpret_cast<DWORD>(&g_xenonHudElemFontScaleSmall));
	}

	Xenon::Xenon()
	{
		PatchMemory();

#ifdef __XENON_UI_THUMBSTICK
		// hook UI_ReplaceConversions to call UI_FilterStringForButtonAnimation on the output,
		// animating lstick/rstick glyphs the same way the console client does
		s_uiReplaceConversionsHook.initialize(0x4E9740, UI_ReplaceConversions_Hk, HOOK_JUMP)->install();
#endif

#ifdef __JSON_FONTLOADER
		// hook R_RegisterFont so JSON overrides are applied via the internal file path system
		s_registerFontHook.initialize(0x505670, R_RegisterFont_Hk, HOOK_JUMP)->install();
#endif

		Scheduler::Once([] { PatchDvars(); }, Scheduler::Pipeline::MAIN);
	}
}