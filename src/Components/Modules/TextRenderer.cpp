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

    bool TextRenderer::IsFontIcon(const char*& text, std::string& fontIconName)
    {
        const auto* curPos = text;

        while(*curPos != ' ' && *curPos != ':' && *curPos != 0)
            curPos++;

        if (*curPos != ':')
            return false;

        fontIconName = std::string(text, static_cast<size_t>(curPos - text));
        text = curPos + 1;
        return true;
    }

    Game::GfxImage* TextRenderer::GetFontIconColorMap(Game::Material* fontIconMaterial)
    {
        for(auto i = 0u; i < fontIconMaterial->textureCount; i++)
        {
            if (fontIconMaterial->textureTable[i].nameHash == COLOR_MAP_HASH)
                return fontIconMaterial->textureTable[i].u.image;
        }

        return nullptr;
    }

    float TextRenderer::DrawFontIcon(const std::string& fontIconName, float x, float y, float sinAngle, float cosAngle, const Game::Font_s* font, float xScale, const float yScale, unsigned color)
    {
        auto* material = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, fontIconName.data()).material;
        if (material == nullptr || material->techniqueSet == nullptr || material->techniqueSet->name == nullptr || strcmp(material->techniqueSet->name, "2d") != 0)
            material = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, "default").material;

        const auto* colorMap = GetFontIconColorMap(material);
        if (colorMap == nullptr)
            return 0;

        const auto h = static_cast<float>(font->pixelHeight) * yScale;
        const auto w = static_cast<float>(font->pixelHeight) * (static_cast<float>(colorMap->width) / static_cast<float>(colorMap->height)) * xScale;

        const auto yy = y - (h + yScale * static_cast<float>(font->pixelHeight)) * 0.5f;
        Game::RB_DrawStretchPicRotate(material, x, yy, w, h, 0.0, 0.0, 1.0, 1.0, sinAngle, cosAngle, color);

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
            const char* curText = text;
            auto maxLengthRemaining = maxLength;
            auto currentColor = color.packed;
            auto subtitleAllowGlow = false;
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
                        currentColor = color.packed;
                    }
                    else if (renderFlags & Game::TEXT_RENDERFLAG_SUBTITLETEXT && colorIndex == TEXT_COLOR_GREEN)
                    {
                        constexpr Game::GfxColor altColor{ MY_ALTCOLOR_TWO };
                        subtitleAllowGlow = true;
                        // Swap r and b for whatever reason
                        currentColor = ColorRgba(altColor.array[2], altColor.array[1], altColor.array[0], Game::ModulateByteColors(altColor.array[3], color.array[3]));
                    }
                    else
                    {
                        const Game::GfxColor colorTableColor{ (*currentColorTable)[colorIndex] };
                        // Swap r and b for whatever reason
                        currentColor = ColorRgba(colorTableColor.array[2], colorTableColor.array[1], colorTableColor.array[0], color.array[3]);
                    }

                    curText++;
                    count += 2;
                    continue;
                }

                if(letter == '^' && (*curText == '\x01' || *curText == '\x02'))
                {
                    float xRot, yRot;
                    RotateXY(cosAngle, sinAngle, startX, startY, xa, xy, &xRot, &yRot);
                    xa += DrawHudIcon(curText, xRot, yRot, sinAngle, cosAngle, font, xScale, yScale, currentColor);

                    if (renderFlags & Game::TEXT_RENDERFLAG_PADDING)
                        xa += xScale * padding;
                    ++count;
                    maxLengthRemaining--;
                    continue;
                }

                if(letter == ':')
                {
                    std::string fontIconName;
                    if(IsFontIcon(curText, fontIconName))
                    {
                        float xRot, yRot;
                        RotateXY(cosAngle, sinAngle, startX, startY, xa, xy, &xRot, &yRot);
                        xa += DrawFontIcon(fontIconName, xRot, yRot, sinAngle, cosAngle, font, xScale, yScale, currentColor);

                        if (renderFlags & Game::TEXT_RENDERFLAG_PADDING)
                            xa += xScale * padding;
                        ++count;
                        maxLengthRemaining--;
                        continue;
                    }
                }

                if(drawRandomCharAtEnd && maxLengthRemaining == 1)
                {
                    
                }

                if(passes[passIndex] == Game::FONTPASS_NORMAL)
                {
                    if (renderFlags & Game::TEXT_RENDERFLAG_CURSOR && count == cursorPos)
                    {
                        float xRot, yRot;
                        RotateXY(cosAngle, sinAngle, startX, startY, xa, xy, &xRot, &yRot);
                        Game::RB_DrawCursor(material, cursorLetter, xRot, yRot, sinAngle, cosAngle, font, xScale, yScale, color.packed);
                    }

                    float xRot, yRot;
                    auto glyph = Game::R_GetCharacterGlyph(font, letter);
                    auto xAdj = glyph->x0 * xScale;
                    auto yAdj = glyph->y0 * yScale;
                    RotateXY(cosAngle, sinAngle, startX, startY, xa + xAdj, xy + yAdj, &xRot, &yRot);
                    Game::RB_DrawChar(material, xRot, yRot, static_cast<float>(glyph->pixelWidth) * xScale /** 1.75f*/, static_cast<float>(glyph->pixelHeight) * yScale /** 1.125f*/, sinAngle, cosAngle, glyph, currentColor);

                    xa += static_cast<float>(glyph->dx) * xScale;
                }
                
                count++;
                maxLengthRemaining--;
            }

            if(renderFlags & Game::TEXT_RENDERFLAG_CURSOR && count == cursorPos)
            {
                float xRot, yRot;
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