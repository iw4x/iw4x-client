#include <STDInclude.hpp>

#include "Script.hpp"
#include "HudElem.hpp"

namespace Components::GSC
{
	constexpr auto FONTSTYLE_SHIFT = 16;
	constexpr auto FONTSTYLE_MASK = 0xFF;

	const std::unordered_map<std::string, int>  HudElem::HEFontStyles  =
	{
		{ "None", 0 },
		{ "DropShadow", 3 },
		{ "DropShadowBig", 6 },
		{ "Monospace", 0x80 },
		{ "DropShadowMonospace", 0x84 },
		{ "Outline", 7 },
		{ "OutlineBig", 8 }
	};

	void HudElem::HudElemColorToVec4(IN const Game::hudelem_color_t &inHudColor, OUT Game::vec4_t &outFloatColor)
	{
		static const DWORD HudElemColorToVec4Addr = 0x588240;
		_asm
		{
			pushad;

			mov eax, inHudColor;

			mov esi, outFloatColor;

			call HudElemColorToVec4Addr;

			popad;
		}

	}

	void HudElem::GetHudElemInfo(int clientIndex, Game::hudelem_s *elem, Game::cg_hudelem_t* cg_hud_elem, uint8_t *elementText)
	{
		static const DWORD GetHudElemInfoAddr = 0x587ED0;

		_asm
		{
			pushad;

			mov eax, elem;
			push elementText;
			push cg_hud_elem;
			push clientIndex;

			call GetHudElemInfoAddr;

			add esp, 12;

			popad;
		}
	}

	void HudElem::DrawWorldSpaceTextHudElem(int localClientIndex, Game::hudelem_s* elem)
	{
		Game::vec2_t screenPosition{};
		Game::vec3_t hudElemWorldPosition = {
			elem->x,
			elem->y,
			elem->z
		};

		const auto screen = Game::ScrPlace_GetActivePlacement(localClientIndex);

		Game::CG_WorldPosToScreenPos(localClientIndex, screen, &hudElemWorldPosition, OUT screenPosition);
		static Game::cg_hudelem_t elemInfo{}; // This one better not disappear - it's gonna get picked up by another thread to be drawn
		uint8_t txtBuffer[256]{};

		// We copy it because we want to modify some stuff for world space rendering
		Game::hudelem_s copy = *elem;

		// Extract stle from font
		const auto style = copy.fromAlignScreen;
		copy.fromAlignScreen = 0;

		HudElemColorToVec4(elem->color, elemInfo.color);
		GetHudElemInfo(localClientIndex, &copy, &elemInfo, txtBuffer);

		if (elemInfo.hudElemText[0])
		{
			const auto length = strnlen_s(elemInfo.hudElemText, ARRAYSIZE(elemInfo.hudElemText));

			Game::CL_DrawTextPhysicalWithEffects(
				elemInfo.hudElemText,
				length,
				elemInfo.font,
				screenPosition[0],
				screenPosition[1],
				elem->fontScale,
				elem->fontScale,
				elemInfo.color,
				style,
				reinterpret_cast<const float*>(0x7DBA08), // g_glowColor
				Game::sharedUiInfo->assets.textDecodeCharacters,
				Game::sharedUiInfo->assets.textDecodeCharactersGlow,
				elem->fxBirthTime,
				elem->fxLetterTime,
				elem->fxDecayStartTime,
				elem->fxDecayDuration
			);
		}
	}

	void HudElem::AddDrawSurfForHudElem3DText(int localClientIndex, Game::hudelem_s* elem)
	{
		if (elem && elem->type == Game::HE_TYPE_TEXT)
		{
			if (Is3D(elem))
			{
				// That means it's world space!
				DrawWorldSpaceTextHudElem(localClientIndex, elem);
			}
		}
	}

	__declspec(naked) void HudElem::CG_AddDrawSurfsFor3dHudElems_Stub()
	{
		_asm
		{
			pushad
			push edi;
			push ebp;

			call AddDrawSurfForHudElem3DText;

			pop ebp;
			add esp, 4;

			popad;

			// Original code
			cmp     dword ptr [edi], 0Fh;
			jne     skip;
			push 0x446F39;
			retn;


			skip:
				push 0x446F42;
				retn;

		}
	}

	void HudElem::HECmd_SetTextExtended(Game::scr_entref_t entref)
	{
		const auto HECmd_SetText = Utils::Hook::Call<void(Game::scr_entref_t)>(0x5E1160);

		HECmd_SetText(entref);

		if (entref.classnum != 1)
		{
			return;
		}

		Game::game_hudelem_s* gameHudElem = &Game::g_hudelems[entref.entnum];

		if (gameHudElem)
		{
			Game::hudelem_s* hudElem = &gameHudElem->elem;

			Game::vec3_t color = {1.F, 1.F, 1.F};
			float alpha = 1.f;
			float scale = 1.f;

			Game::HE_Font font = Game::HE_Font::HE_FONT_DEFAULT;
			int style = 0;

			if (Game::Scr_GetNumParam() > 1)
			{
				Game::Scr_GetVector(1, color);
			}

			if (Game::Scr_GetNumParam() > 2)
			{
				alpha = Game::Scr_GetFloat(2);
			}

			if (Game::Scr_GetNumParam() > 3)
			{
				scale = Game::Scr_GetFloat(3);
			}

			if (Game::Scr_GetNumParam() > 4)
			{
				font = static_cast<Game::HE_Font>(std::clamp(Game::Scr_GetInt(4), 0, Game::HE_Font::HE_FONT_COUNT-1));
			}

			if (Game::Scr_GetNumParam() > 5)
			{
				const std::string str = Game::Scr_GetString(5);

				if (HudElem::HEFontStyles.contains(str))
				{
					style = HudElem::HEFontStyles.at(str);
				}
			}

			hudElem->color.__s0.r = static_cast<uint8_t>(color[0] * std::numeric_limits<uint8_t>().max());
			hudElem->color.__s0.g = static_cast<uint8_t>(color[1] * std::numeric_limits<uint8_t>().max());
			hudElem->color.__s0.b = static_cast<uint8_t>(color[2] * std::numeric_limits<uint8_t>().max());
			hudElem->color.__s0.a = static_cast<uint8_t>(alpha * std::numeric_limits<uint8_t>().max());

			hudElem->fontScale = scale;
			
			if (Is3D(hudElem))
			{
				hudElem->font = font;
				hudElem->fromAlignScreen = style; // We abuse "FromAlignScreen" for storage here, because it's unused anyway
			}
			else
			{
				hudElem->font = font;
			}
		}
	}

	bool HudElem::Is3D(const Game::hudelem_s* hudElem)
	{
		return hudElem->z != 0.f;
	}

	Game::game_hudelem_s* HudElem::HECmd_GetHudElem(Game::scr_entref_t entref)
	{
		if (entref.classnum != 1)
		{
			Game::Scr_ObjectError("not a hud element");
			return nullptr;
		}

		assert(entref.entnum < 1024);
		return &Game::g_hudelems[entref.entnum];
	}

	void HudElem::DrawSingleHudElem2d(int localClientNum, Game::hudelem_s *elem)
	{
		if (elem && elem->type == Game::HE_TYPE_TEXT && Is3D(elem))
		{
			return; // Handled by a 3D Draw call earlier in this class
		}

		Utils::Hook::Call<void(int, Game::hudelem_s*)>(0x588D80)(localClientNum, elem);
	}


	void HudElem::AddScriptMethods()
	{
		// gsc: self setTextExtended(text:string, color:vec3, alpha:float, scale:float, font:integer, style:string); 
		Script::AddMethod("setTextExtended", HECmd_SetTextExtended);
		

		// gsc: self clearHudText()
		Script::AddMethod("clearHudText", [](Game::scr_entref_t entref) -> void
		{
			auto* hud = HECmd_GetHudElem(entref);

			// Frees config string up
			if ((hud->elem).text)
			{
				// This calls SL_RemoveRefToString and synchronizes the new string (here it will synchronize "emptyString")
				Game::SV_SetConfigstring((hud->elem).text + Game::CS_LOCALIZED_STRINGS, nullptr);
			}
		});
	}

	HudElem::HudElem()
	{
		Utils::Hook(0x446F34, CG_AddDrawSurfsFor3dHudElems_Stub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x4CB995, DrawSingleHudElem2d, HOOK_CALL).install()->quick();

		AddScriptMethods();
	}
}
