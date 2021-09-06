#include "STDInclude.hpp"

namespace Components
{
    unsigned TextRenderer::colorTableDefault[TEXT_COLOR_COUNT]
    {
        ColorRgb(0, 0, 0),          // TEXT_COLOR_BLACK
        ColorRgb(255, 92, 92),      // TEXT_COLOR_RED
        ColorRgb(0, 255, 0),        // TEXT_COLOR_GREEN
        ColorRgb(255, 255, 0),      // TEXT_COLOR_YELLOW
        ColorRgb(0, 0, 255),        // TEXT_COLOR_BLUE
        ColorRgb(0, 255, 255),      // TEXT_COLOR_LIGHT_BLUE
        ColorRgb(255, 92, 255),     // TEXT_COLOR_PINK
        ColorRgb(255, 255, 255),    // TEXT_COLOR_DEFAULT
        ColorRgb(255, 255, 255),    // TEXT_COLOR_AXIS
        ColorRgb(255, 255, 255),    // TEXT_COLOR_ALLIES
        ColorRgb(255, 255, 255),    // TEXT_COLOR_RAINBOW
        ColorRgb(255, 255, 255),    // TEXT_COLOR_SERVER
    };

    unsigned TextRenderer::colorTableNew[TEXT_COLOR_COUNT]
    {
        ColorRgb(0, 0, 0),          // TEXT_COLOR_BLACK
        ColorRgb(255, 49, 49),      // TEXT_COLOR_RED
        ColorRgb(134, 192, 0),      // TEXT_COLOR_GREEN
        ColorRgb(255, 173, 34),     // TEXT_COLOR_YELLOW
        ColorRgb(0, 135, 193),      // TEXT_COLOR_BLUE
        ColorRgb(32, 197, 255),     // TEXT_COLOR_LIGHT_BLUE
        ColorRgb(151, 80, 221),     // TEXT_COLOR_PINK
        ColorRgb(255, 255, 255),    // TEXT_COLOR_DEFAULT
        ColorRgb(255, 255, 255),    // TEXT_COLOR_AXIS
        ColorRgb(255, 255, 255),    // TEXT_COLOR_ALLIES
        ColorRgb(255, 255, 255),    // TEXT_COLOR_RAINBOW
        ColorRgb(255, 255, 255),    // TEXT_COLOR_SERVER
    };

    unsigned(*TextRenderer::currentColorTable)[TEXT_COLOR_COUNT];

    Dvar::Var TextRenderer::cg_newColors;
    Game::dvar_t* TextRenderer::sv_customTextColor;

    unsigned TextRenderer::HsvToRgb(HsvColor hsv)
    {
        unsigned rgb;
        unsigned char region, p, q, t;
        unsigned int h, s, v, remainder;

        if (hsv.s == 0)
        {
            rgb = ColorRgb(hsv.v, hsv.v, hsv.v);
            return rgb;
        }

        // converting to 16 bit to prevent overflow
        h = hsv.h;
        s = hsv.s;
        v = hsv.v;

        region = static_cast<uint8_t>(h / 43);
        remainder = (h - (region * 43)) * 6;

        p = static_cast<uint8_t>((v * (255 - s)) >> 8);
        q = static_cast<uint8_t>((v * (255 - ((s * remainder) >> 8))) >> 8);
        t = static_cast<uint8_t>((v * (255 - ((s * (255 - remainder)) >> 8))) >> 8);

        switch (region)
        {
        case 0:
            rgb = ColorRgb(static_cast<uint8_t>(v), t, p);
            break;
        case 1:
            rgb = ColorRgb(q, static_cast<uint8_t>(v), p);
            break;
        case 2:
            rgb = ColorRgb(p, static_cast<uint8_t>(v), t);
            break;
        case 3:
            rgb = ColorRgb(p, q, static_cast<uint8_t>(v));
            break;
        case 4:
            rgb = ColorRgb(t, p, static_cast<uint8_t>(v));
            break;
        default:
            rgb = ColorRgb(static_cast<uint8_t>(v), p, q);
            break;
        }

        return rgb;
    }

    float TextRenderer::GetMonospaceWidth(Game::Font_s* font, int rendererFlags)
    {
        if(rendererFlags & Game::TEXT_RENDERFLAG_FORCEMONOSPACE)
            return Game::R_GetCharacterGlyph(font, 'o')->dx;

        return 0.0f;
    }

    void TextRenderer::GlowColor(Game::GfxColor* result, const Game::GfxColor baseColor, const Game::GfxColor forcedGlowColor, int renderFlags)
    {
        if (renderFlags & Game::TEXT_RENDERFLAG_GLOW_FORCE_COLOR)
        {
            result->array[0] = forcedGlowColor.array[0];
            result->array[1] = forcedGlowColor.array[1];
            result->array[2] = forcedGlowColor.array[2];
        }
        else
        {
            result->array[0] = static_cast<char>(std::floor(static_cast<float>(static_cast<uint8_t>(baseColor.array[0])) * 0.06f));
            result->array[1] = static_cast<char>(std::floor(static_cast<float>(static_cast<uint8_t>(baseColor.array[1])) * 0.06f));
            result->array[2] = static_cast<char>(std::floor(static_cast<float>(static_cast<uint8_t>(baseColor.array[2])) * 0.06f));
        }
    }

    unsigned TextRenderer::R_FontGetRandomLetter(const int seed)
    {
        static constexpr char RANDOM_CHARACTERS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";
        return RANDOM_CHARACTERS[seed % (std::extent_v<decltype(RANDOM_CHARACTERS)> -1)];
    }

    void TextRenderer::DrawTextFxExtraCharacter(Game::Material* material, const int charIndex, const float x, const float y, const float w, const float h, const float sinAngle, const float cosAngle, const unsigned color)
    {
        Game::RB_DrawStretchPicRotate(material, x, y, w, h, static_cast<float>(charIndex % 16) * 0.0625f, 0.0f, static_cast<float>(charIndex % 16) * 0.0625f + 0.0625f, 1.0f, sinAngle, cosAngle, color);
    }

    Game::GfxImage* TextRenderer::GetFontIconColorMap(Game::Material* fontIconMaterial)
    {
        for (auto i = 0u; i < fontIconMaterial->textureCount; i++)
        {
            if (fontIconMaterial->textureTable[i].nameHash == COLOR_MAP_HASH)
                return fontIconMaterial->textureTable[i].u.image;
        }

        return nullptr;
    }

    bool TextRenderer::IsFontIcon(const char*& text, FontIconInfo& fontIcon)
    {
        const auto* curPos = text;

        while (*curPos != ' ' && *curPos != ':' && *curPos != 0 && *curPos != '+')
            curPos++;

        const auto* nameEnd = curPos;
        
        if(*curPos == '+')
        {
            auto breakArgs = false;
            while(!breakArgs)
            {
                curPos++;
                switch(*curPos)
                {
                case 'h':
                    fontIcon.flipHorizontal = true;
                    break;

                case 'v':
                    fontIcon.flipVertical = true;
                    break;

                case ':':
                    breakArgs = true;
                    break;

                default:
                    return false;
                }
            }
        }

        if (*curPos != ':')
            return false;

        const std::string fontIconName(text, nameEnd - text);

        auto* materialEntry = Game::DB_FindXAssetEntry(Game::XAssetType::ASSET_TYPE_MATERIAL, fontIconName.data());
        if (materialEntry == nullptr)
            return false;
        auto* material = materialEntry->asset.header.material;
        if (material == nullptr || material->techniqueSet == nullptr || material->techniqueSet->name == nullptr || strcmp(material->techniqueSet->name, "2d") != 0)
            return false;

        text = curPos + 1;
        fontIcon.material = material;
        return true;
    }

    float TextRenderer::DrawFontIcon(const FontIconInfo& fontIcon, const float x, const float y, const float sinAngle, const float cosAngle, const Game::Font_s* font, const float xScale, const float yScale, const unsigned color)
    {
        const auto* colorMap = GetFontIconColorMap(fontIcon.material);
        if (colorMap == nullptr)
            return 0;

        float s0, t0, s1, t1;
        if(fontIcon.flipHorizontal)
        {
            s0 = 1.0f;
            s1 = 0.0f;
        }
        else
        {
            s0 = 0.0f;
            s1 = 1.0f;
        }
        if(fontIcon.flipVertical)
        {
            t0 = 1.0f;
            t1 = 0.0f;
        }
        else
        {
            t0 = 0.0f;
            t1 = 1.0f;
        }

        const auto h = static_cast<float>(font->pixelHeight) * yScale;
        const auto w = static_cast<float>(font->pixelHeight) * (static_cast<float>(colorMap->width) / static_cast<float>(colorMap->height)) * xScale;

        const auto yy = y - (h + yScale * static_cast<float>(font->pixelHeight)) * 0.5f;
        Game::RB_DrawStretchPicRotate(fontIcon.material, x, yy, w, h, s0, t0, s1, t1, sinAngle, cosAngle, color);

        return w;
    }

    float TextRenderer::DrawHudIcon(const char*& text, const float x, const float y, const float sinAngle, const float cosAngle, const Game::Font_s* font, const float xScale, const float yScale, const unsigned color)
    {
        float s0, s1, t0, t1;

        if(*text == '\x01')
        {
            s0 = 0.0;
            t0 = 0.0;
            s1 = 1.0;
            t1 = 1.0;
        }
        else
        {
            s0 = 1.0;
            t0 = 0.0;
            s1 = 0.0;
            t1 = 1.0;
        }
        text++;

        if (*text == 0)
            return 0;

        const auto v12 = font->pixelHeight * (*text - 16) + 16;
        const auto w = static_cast<float>((((v12 >> 24) & 0x1F) + v12) >> 5) * xScale;
        text++;

        if (*text == 0)
            return 0;

        const auto h = static_cast<float>((font->pixelHeight * (*text - 16) + 16) >> 5) * yScale;
        text++;

        if (*text == 0)
            return 0;

        const auto materialNameLen = static_cast<uint8_t>(*text);
        text++;

        for(auto i = 0u; i < materialNameLen; i++)
        {
            if (text[i] == 0)
                return 0;
        }

        const std::string materialName(text, materialNameLen);
        text += materialNameLen;

        auto* material = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, materialName.data()).material;
        if (material == nullptr || material->techniqueSet == nullptr || material->techniqueSet->name == nullptr || strcmp(material->techniqueSet->name, "2d") != 0)
            material = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, "default").material;

        const auto yy = y - (h + yScale * static_cast<float>(font->pixelHeight)) * 0.5f;

        Game::RB_DrawStretchPicRotate(material, x, yy, w, h, s0, t0, s1, t1, sinAngle, cosAngle, color);

        return w;
    }

    void TextRenderer::RotateXY(const float cosAngle, const float sinAngle, const float pivotX, const float pivotY, const float x, const float y, float* outX, float* outY)
    {
        *outX = (x - pivotX) * cosAngle + pivotX - (y - pivotY) * sinAngle;
        *outY = (y - pivotY) * cosAngle + pivotY + (x - pivotX) * sinAngle;
    }

    void TextRenderer::DrawText2D(const char* text, float x, float y, Game::Font_s* font, float xScale, float yScale, float sinAngle, float cosAngle, Game::GfxColor color, int maxLength, int renderFlags, int cursorPos, char cursorLetter, float padding, Game::GfxColor glowForcedColor, int fxBirthTime, int fxLetterTime, int fxDecayStartTime, int fxDecayDuration, Game::Material* fxMaterial, Game::Material* fxMaterialGlow)
    {
        UpdateColorTable();

        Game::GfxColor dropShadowColor{0};
        dropShadowColor.array[3] = color.array[3];

        int randSeed = 1;
        bool drawRandomCharAtEnd = false;
        const auto forceMonospace = renderFlags & Game::TEXT_RENDERFLAG_FORCEMONOSPACE;
        const auto monospaceWidth = GetMonospaceWidth(font, renderFlags);
        auto* material = font->material;
        Game::Material* glowMaterial = nullptr;

        bool decaying;
        int decayTimeElapsed;
        if(renderFlags & Game::TEXT_RENDERFLAG_FX_DECODE)
        {
            if (!Game::SetupPulseFXVars(text, maxLength, fxBirthTime, fxLetterTime, fxDecayStartTime, fxDecayDuration, &drawRandomCharAtEnd, &randSeed, &maxLength, &decaying, &decayTimeElapsed))
                return;
        }
        else
        {
            drawRandomCharAtEnd = false;
            randSeed = 1;
            decaying = false;
            decayTimeElapsed = 0;
        }

        Game::FontPassType passes[Game::FONTPASS_COUNT];
        unsigned passCount = 0;

        if(renderFlags & Game::TEXT_RENDERFLAG_OUTLINE)
        {
            if(renderFlags & Game::TEXT_RENDERFLAG_GLOW)
            {
                glowMaterial = font->glowMaterial;
                passes[passCount++] = Game::FONTPASS_GLOW;
            }

            passes[passCount++] = Game::FONTPASS_OUTLINE;
            passes[passCount++] = Game::FONTPASS_NORMAL;
        }
        else
        {
            passes[passCount++] = Game::FONTPASS_NORMAL;

            if (renderFlags & Game::TEXT_RENDERFLAG_GLOW)
            {
                glowMaterial = font->glowMaterial;
                passes[passCount++] = Game::FONTPASS_GLOW;
            }
        }

        const auto startX = x - xScale * 0.5f;
        const auto startY = y - 0.5f * yScale;

        for(auto passIndex = 0u; passIndex < passCount; passIndex++)
        {
            float xRot, yRot;
            const char* curText = text;
            auto maxLengthRemaining = maxLength;
            auto currentColor = color;
            auto subtitleAllowGlow = false;
            auto extraFxChar = 0;
            auto drawExtraFxChar = false;
            auto passRandSeed = randSeed;
            auto count = 0;
            auto xa = startX;
            auto xy = startY;

            while(*curText && maxLengthRemaining)
            {
                auto letter = Game::SEH_ReadCharFromString(&curText, nullptr);

                if(letter == '^' && *curText >= COLOR_FIRST_CHAR && *curText <= COLOR_LAST_CHAR)
                {
                    const auto colorIndex = ColorIndexForChar(*curText);
                    subtitleAllowGlow = false;
                    if (colorIndex == TEXT_COLOR_DEFAULT)
                    {
                        currentColor = color;
                    }
                    else if (renderFlags & Game::TEXT_RENDERFLAG_SUBTITLETEXT && colorIndex == TEXT_COLOR_GREEN)
                    {
                        constexpr Game::GfxColor altColor{ MY_ALTCOLOR_TWO };
                        subtitleAllowGlow = true;
                        // Swap r and b for whatever reason
                        currentColor.packed = ColorRgba(altColor.array[2], altColor.array[1], altColor.array[0], Game::ModulateByteColors(altColor.array[3], color.array[3]));
                    }
                    else
                    {
                        const Game::GfxColor colorTableColor{ (*currentColorTable)[colorIndex] };
                        // Swap r and b for whatever reason
                        currentColor.packed = ColorRgba(colorTableColor.array[2], colorTableColor.array[1], colorTableColor.array[0], color.array[3]);
                    }

                    curText++;
                    count += 2;
                    continue;
                }

                auto finalColor = currentColor;

                if(letter == '^' && (*curText == '\x01' || *curText == '\x02'))
                {
                    RotateXY(cosAngle, sinAngle, startX, startY, xa, xy, &xRot, &yRot);
                    xa += DrawHudIcon(curText, xRot, yRot, sinAngle, cosAngle, font, xScale, yScale, finalColor.packed);

                    if (renderFlags & Game::TEXT_RENDERFLAG_PADDING)
                        xa += xScale * padding;
                    ++count;
                    maxLengthRemaining--;
                    continue;
                }

                if(letter == ':')
                {
                    FontIconInfo fontIconInfo{};
                    if(IsFontIcon(curText, fontIconInfo))
                    {
                        RotateXY(cosAngle, sinAngle, startX, startY, xa, xy, &xRot, &yRot);
                        xa += DrawFontIcon(fontIconInfo, xRot, yRot, sinAngle, cosAngle, font, xScale, yScale, ColorRgba(255, 255, 255, finalColor.array[3]));

                        if (renderFlags & Game::TEXT_RENDERFLAG_PADDING)
                            xa += xScale * padding;
                        ++count;
                        maxLengthRemaining--;
                        continue;
                    }
                }

                if(drawRandomCharAtEnd && maxLengthRemaining == 1)
                {
                    letter = R_FontGetRandomLetter(Game::RandWithSeed(&passRandSeed));

                    if(Game::RandWithSeed(&passRandSeed) % 2)
                    {
                        drawExtraFxChar = true;
                        letter = 'O';
                    }
                }

                auto skipDrawing = false;
                if(decaying)
                {
                    char decayAlpha;
                    Game::GetDecayingLetterInfo(letter, &passRandSeed, decayTimeElapsed, fxBirthTime, fxDecayDuration, currentColor.array[3], &skipDrawing, &decayAlpha, &letter, &drawExtraFxChar);
                    finalColor.array[3] = decayAlpha;
                }

                if(drawExtraFxChar)
                {
                    auto tempSeed = passRandSeed;
                    extraFxChar = Game::RandWithSeed(&tempSeed);
                }

                auto glyph = Game::R_GetCharacterGlyph(font, letter);
                auto xAdj = static_cast<float>(glyph->x0) * xScale;
                auto yAdj = static_cast<float>(glyph->y0) * yScale;

                if(!skipDrawing)
                {
                    if (passes[passIndex] == Game::FONTPASS_NORMAL)
                    {
                        if (renderFlags & Game::TEXT_RENDERFLAG_DROPSHADOW)
                        {
                            auto ofs = 1.0f;
                            if (renderFlags & Game::TEXT_RENDERFLAG_DROPSHADOW_EXTRA)
                                ofs += 1.0f;

                            xRot = xa + xAdj + ofs;
                            yRot = xy + yAdj + ofs;
                            RotateXY(cosAngle, sinAngle, startX, startY, xRot, yRot, &xRot, &yRot);
                            if (drawExtraFxChar)
                                DrawTextFxExtraCharacter(fxMaterial, extraFxChar, xRot, yRot, static_cast<float>(glyph->pixelWidth) * xScale, static_cast<float>(glyph->pixelHeight) * yScale, sinAngle, cosAngle, dropShadowColor.packed);
                            else
                                Game::RB_DrawChar(material, xRot, yRot, static_cast<float>(glyph->pixelWidth) * xScale, static_cast<float>(glyph->pixelHeight) * yScale, sinAngle, cosAngle, glyph, dropShadowColor.packed);
                        }

                        RotateXY(cosAngle, sinAngle, startX, startY, xa + xAdj, xy + yAdj, &xRot, &yRot);
                        if (drawExtraFxChar)
                            DrawTextFxExtraCharacter(fxMaterial, extraFxChar, xRot, yRot, static_cast<float>(glyph->pixelWidth) * xScale, static_cast<float>(glyph->pixelHeight) * yScale, sinAngle, cosAngle, finalColor.packed);
                        else
                            Game::RB_DrawChar(material, xRot, yRot, static_cast<float>(glyph->pixelWidth) * xScale, static_cast<float>(glyph->pixelHeight) * yScale, sinAngle, cosAngle, glyph, finalColor.packed);

                        if (renderFlags & Game::TEXT_RENDERFLAG_CURSOR && count == cursorPos)
                        {
                            RotateXY(cosAngle, sinAngle, startX, startY, xa, xy, &xRot, &yRot);
                            Game::RB_DrawCursor(material, cursorLetter, xRot, yRot, sinAngle, cosAngle, font, xScale, yScale, color.packed);
                        }
                    }
                    else if(passes[passIndex] == Game::FONTPASS_OUTLINE)
                    {
                        auto outlineSize = 1.0f;
                        if (renderFlags & Game::TEXT_RENDERFLAG_OUTLINE_EXTRA)
                            outlineSize = 1.3f;

                        for (const auto offset : MY_OFFSETS)
                        {
                            RotateXY(cosAngle, sinAngle, startX, startY, xa + xAdj + outlineSize * offset[0], xy + yAdj + outlineSize * offset[1], &xRot, &yRot);
                            if (drawExtraFxChar)
                                DrawTextFxExtraCharacter(fxMaterial, extraFxChar, xRot, yRot, static_cast<float>(glyph->pixelWidth) * xScale, static_cast<float>(glyph->pixelHeight) * yScale, sinAngle, cosAngle, dropShadowColor.packed);
                            else
                                Game::RB_DrawChar(material, xRot, yRot, static_cast<float>(glyph->pixelWidth) * xScale, static_cast<float>(glyph->pixelHeight) * yScale, sinAngle, cosAngle, glyph, dropShadowColor.packed);
                        }
                    }
                    else if(passes[passIndex] == Game::FONTPASS_GLOW && ((renderFlags & Game::TEXT_RENDERFLAG_SUBTITLETEXT) == 0 || subtitleAllowGlow))
                    {
                        GlowColor(&finalColor, finalColor, glowForcedColor, renderFlags);

                        for (const auto offset : MY_OFFSETS)
                        {
                            RotateXY(cosAngle, sinAngle, startX, startY, xa + xAdj + 2.0f * offset[0] * xScale, xy + yAdj + 2.0f * offset[1] * yScale, &xRot, &yRot);
                            if (drawExtraFxChar)
                                DrawTextFxExtraCharacter(fxMaterialGlow, extraFxChar, xRot, yRot, static_cast<float>(glyph->pixelWidth) * xScale, static_cast<float>(glyph->pixelHeight) * yScale, sinAngle, cosAngle, finalColor.packed);
                            else
                                Game::RB_DrawChar(glowMaterial, xRot, yRot, static_cast<float>(glyph->pixelWidth) * xScale, static_cast<float>(glyph->pixelHeight) * yScale, sinAngle, cosAngle, glyph, finalColor.packed);
                        }
                    }
                }

                if(forceMonospace)
                    xa += monospaceWidth * xScale;
                else
                    xa += static_cast<float>(glyph->dx) * xScale;

                if (renderFlags & Game::TEXT_RENDERFLAG_PADDING)
                    xa += xScale * padding;

                count++;
                maxLengthRemaining--;
            }

            if(renderFlags & Game::TEXT_RENDERFLAG_CURSOR && count == cursorPos)
            {
                RotateXY(cosAngle, sinAngle, startX, startY, xa, xy, &xRot, &yRot);
                Game::RB_DrawCursor(material, cursorLetter, xRot, yRot, sinAngle, cosAngle, font, xScale, yScale, color.packed);
            }
        }
    }

    void TextRenderer::UpdateColorTable()
    {
        if (cg_newColors.get<bool>())
            currentColorTable = &colorTableNew;
        else
            currentColorTable = &colorTableDefault;

        (*currentColorTable)[TEXT_COLOR_AXIS] = *reinterpret_cast<unsigned*>(0x66E5F70);
        (*currentColorTable)[TEXT_COLOR_ALLIES] = *reinterpret_cast<unsigned*>(0x66E5F74);
        (*currentColorTable)[TEXT_COLOR_RAINBOW] = HsvToRgb({ static_cast<uint8_t>((Game::Sys_Milliseconds() / 200) % 256), 255,255 });
        (*currentColorTable)[TEXT_COLOR_SERVER] = sv_customTextColor->current.unsignedInt;
    }

    TextRenderer::TextRenderer()
    {
        currentColorTable = &colorTableDefault;
        
        cg_newColors = Dvar::Register<bool>("cg_newColors", true, Game::dvar_flag::DVAR_FLAG_SAVED, "Use Warfare 2 color code style.");
        sv_customTextColor = Game::Dvar_RegisterColor("sv_customTextColor", 1, 0.7f, 0, 1, Game::dvar_flag::DVAR_FLAG_REPLICATED, "Color for the extended color code.");

        Utils::Hook(0x535410, DrawText2D, HOOK_JUMP).install()->quick();
    }
}