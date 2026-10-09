#pragma once

namespace Components
{
	class Xenon : public Component
	{
	public:
		Xenon();

	private:
		static void PatchDvars();
		static void PatchMemory();

#ifdef __XENON_UI_THUMBSTICK
		// Hooks UI_ReplaceConversions to animate lstick/rstick glyphs, matching console behavior
		static Utils::Hook s_uiReplaceConversionsHook;
		static void UI_ReplaceConversions_Hk(const char* sourceString, Game::ConversionArguments* arguments, char* outputString, size_t outputStringSize);
#endif

#ifdef __JSON_FONTLOADER
		// Per-font extended glyph storage (outlives the Font_s pointer)
		static std::unordered_map<std::string, std::vector<Game::Glyph>> s_fontGlyphStorage;
		static std::unordered_map<std::string, int> s_fontPixelHeightStorage;
		static Utils::Hook s_registerFontHook;

		static Game::Font_s* R_RegisterFont_Hk(const char* asset, int safe);
		static void ApplyFontJson(Game::Font_s* font, const char* jsonPath);
#endif
	};
}