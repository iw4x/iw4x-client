#pragma once

namespace Components
{
	enum TextColor
	{
	    TEXT_COLOR_BLACK = 0,
		TEXT_COLOR_RED = 1,
		TEXT_COLOR_GREEN = 2,
		TEXT_COLOR_YELLOW = 3,
		TEXT_COLOR_BLUE = 4,
		TEXT_COLOR_LIGHT_BLUE = 5,
		TEXT_COLOR_PINK = 6,
		TEXT_COLOR_DEFAULT = 7,
		TEXT_COLOR_AXIS = 8,
		TEXT_COLOR_ALLIES = 9,
		TEXT_COLOR_RAINBOW = 10,
		TEXT_COLOR_SERVER = 11, // using that color in infostrings (e.g. your name) fails, ';' is an illegal character!

		TEXT_COLOR_COUNT
	};

	constexpr unsigned int ColorRgba(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
	{
		return (r) | (g << 8) | (b << 16) | (a << 24);
	}

	constexpr unsigned int ColorRgb(const uint8_t r, const uint8_t g, const uint8_t b)
	{
		return ColorRgba(r, g, b, 0xFF);
	}

	constexpr char CharForColorIndex(const int colorIndex)
	{
		return static_cast<char>('0' + colorIndex);
	}

	constexpr int ColorIndexForChar(const char colorChar)
	{
		return colorChar - '0';
	}

	class TextRenderer : public Component
	{
		struct FontIconInfo
		{
            Game::Material* material;
			bool flipHorizontal;
			bool flipVertical;
			bool big;
		};

		struct HsvColor
		{
			unsigned char h;
			unsigned char s;
			unsigned char v;
		};

		enum FontIconAutocompleteInstance
		{
		    FONT_ICON_ACI_CONSOLE,
		    FONT_ICON_ACI_CHAT,

			FONT_ICON_ACI_COUNT
		};

		class FontIconAutocompleteResult
		{
		public:
			std::string fontIconName;
			std::string materialName;
		};

		class FontIconAutocompleteContext
		{
		public:
			static constexpr auto MAX_RESULTS = 10;

			bool autocompleteActive;
			unsigned int lastHash;
			std::string lastQuery;
			FontIconAutocompleteResult results[MAX_RESULTS];
			size_t resultCount;
			bool hasMoreResults;
			size_t resultOffset;
			size_t lastResultOffset;
			size_t selectedOffset;
			float maxFontIconWidth;
			float maxMaterialNameWidth;
		};

		static constexpr char COLOR_FIRST_CHAR = '0';
		static constexpr char COLOR_LAST_CHAR = CharForColorIndex(TEXT_COLOR_COUNT - 1);
		static constexpr unsigned MY_ALTCOLOR_TWO = 0x0DCE6FFE6;
		static constexpr unsigned COLOR_MAP_HASH = 0xA0AB1041;
		static constexpr auto FONT_ICON_AUTOCOMPLETE_BOX_PADDING = 6.0f;
		static constexpr auto FONT_ICON_AUTOCOMPLETE_BOX_BORDER = 2.0f;
		static constexpr auto FONT_ICON_AUTOCOMPLETE_COL_SPACING = 12.0f;
		static constexpr auto FONT_ICON_AUTOCOMPLETE_ARROW_SIZE = 12.0f;
		static constexpr float MY_OFFSETS[4][2]
		{
			{-1.0f, -1.0f},
			{-1.0f, 1.0f},
			{1.0f, -1.0f},
			{1.0f, 1.0f},
		};

		static unsigned colorTableDefault[TEXT_COLOR_COUNT];
		static unsigned colorTableNew[TEXT_COLOR_COUNT];
		static unsigned(*currentColorTable)[TEXT_COLOR_COUNT];
		static FontIconAutocompleteContext autocompleteContextArray[FONT_ICON_ACI_COUNT];

		static Dvar::Var cg_newColors;
		static Game::dvar_t* sv_customTextColor;
		static Dvar::Var r_colorBlind;
		static Game::dvar_t* g_ColorBlind_MyTeam;
		static Game::dvar_t* g_ColorBlind_EnemyTeam;
		static Game::dvar_t** con_inputBoxColor;

	public:
		static void DrawText2D(const char* text, float x, float y, Game::Font_s* font, float xScale, float yScale, float sinAngle, float cosAngle, Game::GfxColor color, int maxLength, int renderFlags, int cursorPos, char cursorLetter, float padding, Game::GfxColor glowForcedColor, int fxBirthTime, int fxLetterTime, int fxDecayStartTime, int fxDecayDuration, Game::Material* fxMaterial, Game::Material* fxMaterialGlow);
		static int R_TextWidth_Hk(const char* text, int maxChars, Game::Font_s* font);
		static unsigned int ColorIndex(char index);
		static void StripColors(const char* in, char* out, size_t max);
		static std::string StripColors(const std::string& in);
		static void StripMaterialTextIcons(const char* in, char* out, size_t max);
		static std::string StripMaterialTextIcons(const std::string& in);
		static void StripAllTextIcons(const char* in, char* out, size_t max);
		static std::string StripAllTextIcons(const std::string& in);

		TextRenderer();

	private:
		static unsigned HsvToRgb(HsvColor hsv);

		static void DrawAutocompleteBox(const FontIconAutocompleteContext& context, float x, float y, float w, float h, const float* color);
		static void DrawAutocomplete(const FontIconAutocompleteContext& context, float x, float y, Game::Font_s* font);
		static void UpdateAutocompleteContextResults(FontIconAutocompleteContext& context, Game::Font_s* font);
		static void UpdateAutocompleteContext(FontIconAutocompleteContext& context, Game::field_t* edit, Game::Font_s* font);
		static void Field_Draw_Say(int localClientNum, Game::field_t* edit, int x, int y, int horzAlign, int vertAlign);
		static void Con_DrawInput_Hk(int localClientNum);

		static void AutocompleteUp(FontIconAutocompleteContext& context);
		static void AutocompleteDown(FontIconAutocompleteContext& context);
		static void AutocompleteFill(const FontIconAutocompleteContext& context, Game::ScreenPlacement* scrPlace, Game::field_t* edit);
		static bool AutocompleteHandleKeyDown(FontIconAutocompleteContext& context, int key, Game::ScreenPlacement* scrPlace, Game::field_t* edit);
		static void Console_Key_Hk(int localClientNum, int key);
		static bool ChatHandleKeyDown(int localClientNum, int key);
		static void Message_Key_Stub();

		static int SEH_PrintStrlenWithCursor(const char* string, const Game::field_t* field);
		static void Field_AdjustScroll_PrintLen_Stub();

		static void PatchColorLimit(char limit);
		static bool Dvar_GetUnpackedColorByName(const char* name, float* expandedColor);
		static void GetUnpackedColorByNameStub();

		static Game::GfxImage* GetFontIconColorMap(const Game::Material* fontIconMaterial);
		static bool IsFontIcon(const char*& text, FontIconInfo& fontIcon);
		static float GetFontIconWidth(const FontIconInfo& fontIcon, const Game::Font_s* font, float xScale);
		static float DrawFontIcon(const FontIconInfo& fontIcon, float x, float y, float sinAngle, float cosAngle, const Game::Font_s* font, float xScale, float yScale, unsigned color);

		static float GetMonospaceWidth(Game::Font_s* font, int rendererFlags);
		static void GlowColor(Game::GfxColor* result, Game::GfxColor baseColor, Game::GfxColor forcedGlowColor, int renderFlags);
        static unsigned R_FontGetRandomLetter(int seed);
		static void DrawTextFxExtraCharacter(Game::Material* material, int charIndex, float x, float y, float w, float h, float sinAngle, float cosAngle, unsigned color);
		static float DrawHudIcon(const char*& text, float x, float y, float sinAngle, float cosAngle, const Game::Font_s* font, float xScale, float yScale, unsigned color);
		static void RotateXY(float cosAngle, float sinAngle, float pivotX, float pivotY, float x, float y, float* outX, float* outY);
		static void UpdateColorTable();
	};
}
