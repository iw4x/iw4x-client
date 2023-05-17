#include <STDInclude.hpp>
#include "Gamepad.hpp"
#include "RawMouse.hpp"

namespace Components
{
	Game::ButtonToCodeMap_t Gamepad::buttonList[]
	{
		{Game::GPAD_X, Game::K_BUTTON_X},
		{Game::GPAD_A, Game::K_BUTTON_A},
		{Game::GPAD_B, Game::K_BUTTON_B},
		{Game::GPAD_Y, Game::K_BUTTON_Y},
		{Game::GPAD_L_TRIG, Game::K_BUTTON_LTRIG},
		{Game::GPAD_R_TRIG, Game::K_BUTTON_RTRIG},
		{Game::GPAD_L_SHLDR, Game::K_BUTTON_LSHLDR},
		{Game::GPAD_R_SHLDR, Game::K_BUTTON_RSHLDR},
		{Game::GPAD_START, Game::K_BUTTON_START},
		{Game::GPAD_BACK, Game::K_BUTTON_BACK},
		{Game::GPAD_L3, Game::K_BUTTON_LSTICK},
		{Game::GPAD_R3, Game::K_BUTTON_RSTICK},
		{Game::GPAD_UP, Game::K_DPAD_UP},
		{Game::GPAD_DOWN, Game::K_DPAD_DOWN},
		{Game::GPAD_LEFT, Game::K_DPAD_LEFT},
		{Game::GPAD_RIGHT, Game::K_DPAD_RIGHT},
	};

	Game::StickToCodeMap_t Gamepad::analogStickList[4]
	{
		{Game::GPAD_LX, Game::K_APAD_RIGHT, Game::K_APAD_LEFT},
		{Game::GPAD_LY, Game::K_APAD_UP, Game::K_APAD_DOWN},
		{Game::GPAD_RX, Game::K_APAD_RIGHT, Game::K_APAD_LEFT},
		{Game::GPAD_RY, Game::K_APAD_UP, Game::K_APAD_DOWN},
	};

	Game::GamePadStick Gamepad::stickForAxis[Game::GPAD_PHYSAXIS_COUNT]
	{
		Game::GPAD_RX,
		Game::GPAD_RY,
		Game::GPAD_LX,
		Game::GPAD_LY,
		Game::GPAD_INVALID,
		Game::GPAD_INVALID
	};

	Game::GamepadPhysicalAxis Gamepad::axisSameStick[Game::GPAD_PHYSAXIS_COUNT]
	{
		Game::GPAD_PHYSAXIS_RSTICK_Y,
		Game::GPAD_PHYSAXIS_RSTICK_X,
		Game::GPAD_PHYSAXIS_LSTICK_Y,
		Game::GPAD_PHYSAXIS_LSTICK_X,
		Game::GPAD_PHYSAXIS_NONE,
		Game::GPAD_PHYSAXIS_NONE
	};

	const char* Gamepad::physicalAxisNames[Game::GPAD_PHYSAXIS_COUNT]
	{
		"A_RSTICK_X",
		"A_RSTICK_Y",
		"A_LSTICK_X",
		"A_LSTICK_Y",
		"A_RTRIGGER",
		"A_LTRIGGER"
	};

	const char* Gamepad::virtualAxisNames[Game::GPAD_VIRTAXIS_COUNT]
	{
		"VA_SIDE",
		"VA_FORWARD",
		"VA_UP",
		"VA_YAW",
		"VA_PITCH",
		"VA_ATTACK"
	};

	const char* Gamepad::gamePadMappingTypeNames[Game::GPAD_MAP_COUNT]
	{
		"MAP_LINEAR",
		"MAP_SQUARED"
	};

	Game::keyNum_t Gamepad::menuScrollButtonList[]
	{
		Game::K_APAD_UP,
		Game::K_APAD_DOWN,
		Game::K_APAD_LEFT,
		Game::K_APAD_RIGHT,
		Game::K_DPAD_UP,
		Game::K_DPAD_DOWN,
		Game::K_DPAD_LEFT,
		Game::K_DPAD_RIGHT
	};

	Game::keyname_t Gamepad::extendedKeyNames[]
	{
		{"BUTTON_A", Game::K_BUTTON_A},
		{"BUTTON_B", Game::K_BUTTON_B},
		{"BUTTON_X", Game::K_BUTTON_X},
		{"BUTTON_Y", Game::K_BUTTON_Y},
		{"BUTTON_LSHLDR", Game::K_BUTTON_LSHLDR},
		{"BUTTON_RSHLDR", Game::K_BUTTON_RSHLDR},
		{"BUTTON_START", Game::K_BUTTON_START},
		{"BUTTON_BACK", Game::K_BUTTON_BACK},
		{"BUTTON_LSTICK", Game::K_BUTTON_LSTICK},
		{"BUTTON_RSTICK", Game::K_BUTTON_RSTICK},
		{"BUTTON_LTRIG", Game::K_BUTTON_LTRIG},
		{"BUTTON_RTRIG", Game::K_BUTTON_RTRIG},
		{"DPAD_UP", Game::K_DPAD_UP},
		{"DPAD_DOWN", Game::K_DPAD_DOWN},
		{"DPAD_LEFT", Game::K_DPAD_LEFT},
		{"DPAD_RIGHT", Game::K_DPAD_RIGHT},
	};

	Game::keyname_t Gamepad::extendedLocalizedKeyNamesXenon[]
	{
		// Material text icons pattern: 0x01 width height material_name_len
		{"^\x01\x32\x32\x08""button_a", Game::K_BUTTON_A},
		{"^\x01\x32\x32\x08""button_b", Game::K_BUTTON_B},
		{"^\x01\x32\x32\x08""button_x", Game::K_BUTTON_X},
		{"^\x01\x32\x32\x08""button_y", Game::K_BUTTON_Y},
		{"^\x01\x32\x32\x0D""button_lshldr", Game::K_BUTTON_LSHLDR},
		{"^\x01\x32\x32\x0D""button_rshldr", Game::K_BUTTON_RSHLDR},
		{"^\x01\x32\x32\x0C""button_start", Game::K_BUTTON_START},
		{"^\x01\x32\x32\x0B""button_back", Game::K_BUTTON_BACK},
		{"^\x01\x48\x32\x0D""button_lstick", Game::K_BUTTON_LSTICK},
		{"^\x01\x48\x32\x0D""button_rstick", Game::K_BUTTON_RSTICK},
		{"^\x01\x32\x32\x0C""button_ltrig", Game::K_BUTTON_LTRIG},
		{"^\x01\x32\x32\x0C""button_rtrig", Game::K_BUTTON_RTRIG},
		{"^\x01\x32\x32\x07""dpad_up", Game::K_DPAD_UP},
		{"^\x01\x32\x32\x09""dpad_down", Game::K_DPAD_DOWN},
		{"^\x01\x32\x32\x09""dpad_left", Game::K_DPAD_LEFT},
		{"^\x01\x32\x32\x0A""dpad_right", Game::K_DPAD_RIGHT},
	};

	Game::keyname_t Gamepad::extendedLocalizedKeyNamesPs3[]
	{
		// Material text icons pattern: 0x01 width height material_name_len
		{"^\x01\x32\x32\x10""button_ps3_cross", Game::K_BUTTON_A},
		{"^\x01\x32\x32\x11""button_ps3_circle", Game::K_BUTTON_B},
		{"^\x01\x32\x32\x11""button_ps3_square", Game::K_BUTTON_X},
		{"^\x01\x32\x32\x13""button_ps3_triangle", Game::K_BUTTON_Y},
		{"^\x01\x32\x32\x0D""button_ps3_l1", Game::K_BUTTON_LSHLDR},
		{"^\x01\x32\x32\x0D""button_ps3_r1", Game::K_BUTTON_RSHLDR},
		{"^\x01\x32\x32\x10""button_ps3_start", Game::K_BUTTON_START},
		{"^\x01\x32\x32\x0F""button_ps3_back", Game::K_BUTTON_BACK},
		{"^\x01\x48\x32\x0D""button_ps3_l3", Game::K_BUTTON_LSTICK},
		{"^\x01\x48\x32\x0D""button_ps3_r3", Game::K_BUTTON_RSTICK},
		{"^\x01\x32\x32\x0D""button_ps3_l2", Game::K_BUTTON_LTRIG},
		{"^\x01\x32\x32\x0D""button_ps3_r2", Game::K_BUTTON_RTRIG},
		{"^\x01\x32\x32\x0B""dpad_ps3_up", Game::K_DPAD_UP},
		{"^\x01\x32\x32\x0D""dpad_ps3_down", Game::K_DPAD_DOWN},
		{"^\x01\x32\x32\x0D""dpad_ps3_left", Game::K_DPAD_LEFT},
		{"^\x01\x32\x32\x0E""dpad_ps3_right", Game::K_DPAD_RIGHT},
	};
	Game::keyname_t Gamepad::combinedKeyNames[Game::KEY_NAME_COUNT + std::extent_v<decltype(extendedKeyNames)> + 1];
	Game::keyname_t Gamepad::combinedLocalizedKeyNamesXenon[Game::KEY_NAME_COUNT + std::extent_v<decltype(extendedLocalizedKeyNamesXenon)> + 1];
	Game::keyname_t Gamepad::combinedLocalizedKeyNamesPs3[Game::KEY_NAME_COUNT + std::extent_v<decltype(extendedLocalizedKeyNamesPs3)> + 1];

	Gamepad::ControllerMenuKeyMapping Gamepad::controllerMenuKeyMappings[]
	{
		{Game::K_BUTTON_A, Game::K_ENTER},
		{Game::K_BUTTON_START, Game::K_ENTER},
		{Game::K_BUTTON_B, Game::K_ESCAPE},
		{Game::K_BUTTON_BACK, Game::K_ESCAPE},
		{Game::K_DPAD_UP, Game::K_UPARROW},
		{Game::K_APAD_UP, Game::K_UPARROW},
		{Game::K_DPAD_DOWN, Game::K_DOWNARROW},
		{Game::K_APAD_DOWN, Game::K_DOWNARROW},
		{Game::K_DPAD_LEFT, Game::K_LEFTARROW},
		{Game::K_APAD_LEFT, Game::K_LEFTARROW},
		{Game::K_DPAD_RIGHT, Game::K_RIGHTARROW},
		{Game::K_APAD_RIGHT, Game::K_RIGHTARROW},
	};

	Gamepad::GamePad Gamepad::gamePads[Game::MAX_GPAD_COUNT]{};
	Gamepad::GamePadGlobals Gamepad::gamePadGlobals[Game::MAX_GPAD_COUNT]{{}};
	int Gamepad::gamePadBindingsModifiedFlags = 0;

	Dvar::Var Gamepad::gpad_enabled;
	Dvar::Var Gamepad::gpad_debug;
	Dvar::Var Gamepad::gpad_present;
	Dvar::Var Gamepad::gpad_in_use;
	Dvar::Var Gamepad::gpad_style;
	Dvar::Var Gamepad::gpad_sticksConfig;
	Dvar::Var Gamepad::gpad_buttonConfig;
	Dvar::Var Gamepad::gpad_menu_scroll_delay_first;
	Dvar::Var Gamepad::gpad_menu_scroll_delay_rest;
	Dvar::Var Gamepad::gpad_rumble;
	Dvar::Var Gamepad::gpad_stick_pressed_hysteresis;
	Dvar::Var Gamepad::gpad_stick_pressed;
	Dvar::Var Gamepad::gpad_stick_deadzone_max;
	Dvar::Var Gamepad::gpad_stick_deadzone_min;
	Dvar::Var Gamepad::gpad_button_deadzone;
	Dvar::Var Gamepad::gpad_button_rstick_deflect_max;
	Dvar::Var Gamepad::gpad_button_lstick_deflect_max;
	Dvar::Var Gamepad::gpad_use_hold_time;
	Dvar::Var Gamepad::gpad_lockon_enabled;
	Dvar::Var Gamepad::gpad_slowdown_enabled;
	Dvar::Var Gamepad::input_viewSensitivity;
	Dvar::Var Gamepad::input_invertPitch;
	Dvar::Var Gamepad::sv_allowAimAssist;
	Dvar::Var Gamepad::aim_turnrate_pitch;
	Dvar::Var Gamepad::aim_turnrate_pitch_ads;
	Dvar::Var Gamepad::aim_turnrate_yaw;
	Dvar::Var Gamepad::aim_turnrate_yaw_ads;
	Dvar::Var Gamepad::aim_accel_turnrate_enabled;
	Dvar::Var Gamepad::aim_accel_turnrate_lerp;
	Dvar::Var Gamepad::aim_input_graph_enabled;
	Dvar::Var Gamepad::aim_input_graph_index;
	Dvar::Var Gamepad::aim_scale_view_axis;
	Dvar::Var Gamepad::cl_bypassMouseInput;
	Dvar::Var Gamepad::cg_mapLocationSelectionCursorSpeed;
	Dvar::Var Gamepad::aim_aimAssistRangeScale;
	Dvar::Var Gamepad::aim_slowdown_enabled;
	Dvar::Var Gamepad::aim_slowdown_debug;
	Dvar::Var Gamepad::aim_slowdown_pitch_scale;
	Dvar::Var Gamepad::aim_slowdown_pitch_scale_ads;
	Dvar::Var Gamepad::aim_slowdown_yaw_scale;
	Dvar::Var Gamepad::aim_slowdown_yaw_scale_ads;
	Dvar::Var Gamepad::aim_lockon_enabled;
	Dvar::Var Gamepad::aim_lockon_deflection;
	Dvar::Var Gamepad::aim_lockon_pitch_strength;
	Dvar::Var Gamepad::aim_lockon_strength;

	Gamepad::GamePadGlobals::GamePadGlobals()
		: axes{},
		  nextScrollTime(0)
	{
		for (auto& virtualAxis : axes.virtualAxes)
		{
			virtualAxis.physicalAxis = Game::GPAD_PHYSAXIS_NONE;
			virtualAxis.mapType = Game::GPAD_MAP_NONE;
		}
	}

	__declspec(naked) void Gamepad::MSG_WriteDeltaUsercmdKeyStub()
	{
		__asm
		{
			// fix stack pointer
			add esp, 0Ch

			// put both forward move and rightmove values in the movement button
			mov dl, byte ptr[edi + 1Ah] // to_forwardMove
			mov dh, byte ptr[edi + 1Bh] // to_rightMove

			mov [esp + 30h], dx // to_buttons

			mov dl, byte ptr [ebp + 1Ah] // from_forwardMove
			mov dh, byte ptr [ebp + 1Bh] // from_rightMove

			mov [esp + 2Ch], dx // from_buttons

			// return back
			push 0x60E40E
			retn
		}
	}

	void Gamepad::ApplyMovement(Game::msg_t* msg, int key, Game::usercmd_s* from, Game::usercmd_s* to)
	{
		char forward;
		char right;

		if (Game::MSG_ReadBit(msg))
		{
			const auto movementBits = static_cast<short>(key ^ Game::MSG_ReadBits(msg, 16));

			forward = static_cast<char>(movementBits);
			right = static_cast<char>(movementBits >> 8);
		}
		else
		{
			forward = from->forwardmove;
			right = from->rightmove;
		}

		to->forwardmove = forward;
		to->rightmove = right;
	}

	__declspec(naked) void Gamepad::MSG_ReadDeltaUsercmdKeyStub()
	{
		__asm
		{
			push ebx // to
			push ebp // from
			push edi // key
			push esi // msg
			call ApplyMovement
			add esp, 10h

			// return back
			push 0x4921BF
			ret
		}
	}

	__declspec(naked) void Gamepad::MSG_ReadDeltaUsercmdKeyStub2()
	{
		__asm
		{
			push ebx // to
			push ebp // from
			push edi // key
			push esi // msg
			call ApplyMovement
			add esp, 10h

			// return back
			push 3
			push esi
			push 0x492085
			ret
		}
	}

	bool Gamepad::GPad_Check(const int localClientNum, const int portIndex)
	{
		AssertIn(localClientNum, Game::MAX_GPAD_COUNT);

		auto& gamePad = gamePads[localClientNum];

		if (XInputGetCapabilities(portIndex, XINPUT_FLAG_GAMEPAD, &gamePad.caps) == ERROR_SUCCESS)
		{
			gamePad.enabled = true;
			gamePad.portIndex = portIndex;
			return true;
		}

		gamePad.enabled = false;
		return false;
	}

	void Gamepad::GPad_RefreshAll()
	{
		auto currentGamePadNum = 0;
		for (auto currentPort = 0; currentPort < XUSER_MAX_COUNT && currentGamePadNum < Game::MAX_GPAD_COUNT; currentPort++)
		{
			if (GPad_Check(currentGamePadNum, currentPort))
			{
				++currentGamePadNum;
			}
		}
	}

	float Gamepad::LinearTrack(const float target, const float current, const float rate, const float deltaTime)
	{
		const auto err = target - current;
		float step;
		if (err <= 0.0f)
		{
			step = -rate * deltaTime;
		}
		else
		{
			step = rate * deltaTime;
		}

		if (std::fabs(err) <= 0.001f)
		{
			return target;
		}

		if (std::fabs(step) <= std::fabs(err))
		{
			return current + step;
		}

		return target;
	}

	bool Gamepad::AimAssist_DoBoundsIntersectCenterBox(const float* clipMins, const float* clipMaxs, const float clipHalfWidth, const float clipHalfHeight)
	{
		return clipHalfWidth >= clipMins[0] && clipMaxs[0] >= -clipHalfWidth && clipHalfHeight >= clipMins[1] && clipMaxs[1] >= -clipHalfHeight;
	}

	bool Gamepad::AimAssist_IsPlayerUsingOffhand(Game::AimAssistPlayerState* ps)
	{
		// Check offhand flag
		if ((ps->weapFlags & Game::PWF_USING_OFFHAND) == 0)
		{
			return false;
		}

		// If offhand weapon has no id we are not using one
		if (!ps->weapIndex)
		{
			return false;
		}

		const auto* weaponDef = Game::BG_GetWeaponDef(static_cast<std::uint32_t>(ps->weapIndex));

		return weaponDef->offhandClass != Game::OFFHAND_CLASS_NONE;
	}

	const Game::AimScreenTarget* Gamepad::AimAssist_GetBestTarget(const Game::AimAssistGlobals* aaGlob, const float range, const float regionWidth, const float regionHeight)
	{
		const auto rangeSqr = range * range;
		for (auto targetIndex = 0; targetIndex < aaGlob->screenTargetCount; targetIndex++)
		{
			const auto* currentTarget = &aaGlob->screenTargets[targetIndex];
			if (currentTarget->distSqr <= rangeSqr && AimAssist_DoBoundsIntersectCenterBox(currentTarget->clipMins, currentTarget->clipMaxs, regionWidth, regionHeight))
			{
				return currentTarget;
			}
		}

		return nullptr;
	}

	const Game::AimScreenTarget* Gamepad::AimAssist_GetTargetFromEntity(const Game::AimAssistGlobals* aaGlob, const int entIndex)
	{
		if (entIndex == Game::AIM_TARGET_INVALID)
		{
			return nullptr;
		}

		for (auto targetIndex = 0; targetIndex < aaGlob->screenTargetCount; targetIndex++)
		{
			const auto* currentTarget = &aaGlob->screenTargets[targetIndex];
			if (currentTarget->entIndex == entIndex)
			{
				return currentTarget;
			}
		}

		return nullptr;
	}

	const Game::AimScreenTarget* Gamepad::AimAssist_GetPrevOrBestTarget(const Game::AimAssistGlobals* aaGlob, const float range, const float regionWidth, const float regionHeight,
																		const int prevTargetEnt)
	{
		const auto screenTarget = AimAssist_GetTargetFromEntity(aaGlob, prevTargetEnt);

		if (screenTarget && (range * range) > screenTarget->distSqr && AimAssist_DoBoundsIntersectCenterBox(screenTarget->clipMins, screenTarget->clipMaxs, regionWidth, regionHeight))
		{
			return screenTarget;
		}

		return AimAssist_GetBestTarget(aaGlob, range, regionWidth, regionHeight);
	}

	bool Gamepad::AimAssist_IsLockonActive(const int localClientNum)
	{
		AssertIn(localClientNum, Game::MAX_GPAD_COUNT);

		if (!aim_lockon_enabled.get<bool>() || !gpad_lockon_enabled.get<bool>())
		{
			return false;
		}

		auto& aaGlob = Game::aaGlobArray[localClientNum];
		if (AimAssist_IsPlayerUsingOffhand(&aaGlob.ps))
		{
			return false;
		}

		if (aaGlob.autoAimActive || aaGlob.autoMeleeState == Game::AIM_MELEE_STATE_UPDATING)
		{
			return false;
		}

		return true;
	}

	void Gamepad::AimAssist_ApplyLockOn(const Game::AimInput* input, Game::AimOutput* output)
	{
		assert(input);
		assert(output);
		AssertIn(input->localClientNum, Game::STATIC_MAX_LOCAL_CLIENTS);

		auto& aaGlob = Game::aaGlobArray[input->localClientNum];

		const auto prevTargetEnt = aaGlob.lockOnTargetEnt;
		aaGlob.lockOnTargetEnt = Game::AIM_TARGET_INVALID;

		if (!AimAssist_IsLockonActive(input->localClientNum))
		{
			return;
		}

		const auto* weaponDef = Game::BG_GetWeaponDef(static_cast<std::uint32_t>(aaGlob.ps.weapIndex));
		if (weaponDef->requireLockonToFire)
		{
			return;
		}

		const auto deflection = aim_lockon_deflection.get<float>();
		if (deflection > std::fabs(input->pitchAxis) && deflection > std::fabs(input->yawAxis) && deflection > std::fabs(input->rightAxis))
		{
			return;
		}

		if (!aaGlob.ps.weapIndex)
		{
			return;
		}

		const auto aimAssistRange = AimAssist_Lerp(weaponDef->aimAssistRange, weaponDef->aimAssistRangeAds, aaGlob.adsLerp) * aim_aimAssistRangeScale.get<float>();
		const auto screenTarget = AimAssist_GetPrevOrBestTarget(&aaGlob, aimAssistRange, aaGlob.tweakables.lockOnRegionWidth, aaGlob.tweakables.lockOnRegionHeight, prevTargetEnt);

		if (screenTarget && screenTarget->distSqr > 0.0f)
		{
			aaGlob.lockOnTargetEnt = screenTarget->entIndex;
			const auto arcLength = std::sqrt(screenTarget->distSqr) * static_cast<float>(M_PI);

			const auto pitchTurnRate =
				(screenTarget->velocity[0] * aaGlob.viewAxis[2][0] + screenTarget->velocity[1] * aaGlob.viewAxis[2][1] + screenTarget->velocity[2] * aaGlob.viewAxis[2][2]
					- (aaGlob.ps.velocity[0] * aaGlob.viewAxis[2][0] + aaGlob.ps.velocity[1] * aaGlob.viewAxis[2][1] + aaGlob.ps.velocity[2] * aaGlob.viewAxis[2][2]))
				/ arcLength * 180.0f * aim_lockon_pitch_strength.get<float>();

			const auto yawTurnRate = 
				(screenTarget->velocity[0] * aaGlob.viewAxis[1][0] + screenTarget->velocity[1] * aaGlob.viewAxis[1][1] + screenTarget->velocity[2] * aaGlob.viewAxis[1][2]
				- (aaGlob.ps.velocity[0] * aaGlob.viewAxis[1][0] + aaGlob.ps.velocity[1] * aaGlob.viewAxis[1][1] + aaGlob.ps.velocity[2] * aaGlob.viewAxis[1][2]))
			/ arcLength * 180.0f * aim_lockon_strength.get<float>();

			output->pitch -= pitchTurnRate * input->deltaTime;
			output->yaw += yawTurnRate * input->deltaTime;
		}
	}

	void Gamepad::AimAssist_CalcAdjustedAxis(const Game::AimInput* input, float* pitchAxis, float* yawAxis)
	{
		assert(input);
		assert(pitchAxis);
		assert(yawAxis);

		const auto graphIndex = aim_input_graph_index.get<int>();
		if (aim_input_graph_enabled.get<bool>() && graphIndex >= 0 && static_cast<unsigned>(graphIndex) < Game::AIM_ASSIST_GRAPH_COUNT)
		{
			const auto deflection = std::sqrt(input->pitchAxis * input->pitchAxis + input->yawAxis * input->yawAxis);

			float fraction;
			if (deflection - 1.0f < 0.0f)
				fraction = deflection;
			else
				fraction = 1.0f;

			if (0.0f - deflection >= 0.0f)
				fraction = 0.0f;

			const auto graphScale = Game::GraphFloat_GetValue(&Game::aaInputGraph[graphIndex], fraction);
			*pitchAxis = input->pitchAxis * graphScale;
			*yawAxis = input->yawAxis * graphScale;
		}
		else
		{
			*pitchAxis = input->pitchAxis;
			*yawAxis = input->yawAxis;
		}

		if (aim_scale_view_axis.get<bool>())
		{
			const auto absPitchAxis = std::fabs(*pitchAxis);
			const auto absYawAxis = std::fabs(*yawAxis);

			if (absPitchAxis <= absYawAxis)
			{
				*pitchAxis = (1.0f - (absYawAxis - absPitchAxis)) * *pitchAxis;
			}
			else
			{
				*yawAxis = (1.0f - (absPitchAxis - absYawAxis)) * *yawAxis;
			}
		}
	}

	bool Gamepad::AimAssist_IsSlowdownActive(const Game::AimAssistPlayerState* ps)
	{
		if (!aim_slowdown_enabled.get<bool>() || !gpad_slowdown_enabled.get<bool>())
		{
			return false;
		}

		if (!ps->weapIndex)
		{
			return false;
		}

		const auto* weaponDef = Game::BG_GetWeaponDef(static_cast<std::uint32_t>(ps->weapIndex));
		if (weaponDef->requireLockonToFire)
		{
			return false;
		}

		if (ps->linkFlags & Game::PLF_WEAPONVIEW_ONLY)
		{
			return false;
		}

		if (ps->weaponState >= Game::WEAPON_STUNNED_START && ps->weaponState <= Game::WEAPON_STUNNED_END)
		{
			return false;
		}

		if (ps->eFlags & (Game::EF_VEHICLE_ACTIVE | Game::EF_TURRET_ACTIVE_DUCK | Game::EF_TURRET_ACTIVE_PRONE))
		{
			return false;
		}

		if (!ps->hasAmmo)
		{
			return false;
		}

		return true;
	}

	void Gamepad::AimAssist_CalcSlowdown(const Game::AimInput* input, float* pitchScale, float* yawScale)
	{
		assert(input);
		assert(pitchScale);
		assert(yawScale);
		AssertIn(input->localClientNum, Game::STATIC_MAX_LOCAL_CLIENTS);

		auto& aaGlob = Game::aaGlobArray[input->localClientNum];

		*pitchScale = 1.0f;
		*yawScale = 1.0f;
		
		if (!AimAssist_IsSlowdownActive(&aaGlob.ps))
		{
			return;
		}

		const auto* weaponDef = Game::BG_GetWeaponDef(static_cast<std::uint32_t>(aaGlob.ps.weapIndex));
		const auto aimAssistRange = AimAssist_Lerp(weaponDef->aimAssistRange, weaponDef->aimAssistRangeAds, aaGlob.adsLerp) * aim_aimAssistRangeScale.get<float>();
		const auto screenTarget = AimAssist_GetBestTarget(&aaGlob, aimAssistRange, aaGlob.tweakables.slowdownRegionWidth, aaGlob.tweakables.slowdownRegionHeight);

		if (screenTarget)
		{
			*pitchScale = AimAssist_Lerp(aim_slowdown_pitch_scale.get<float>(), aim_slowdown_pitch_scale_ads.get<float>(), aaGlob.adsLerp);
			*yawScale = AimAssist_Lerp(aim_slowdown_yaw_scale.get<float>(), aim_slowdown_yaw_scale_ads.get<float>(), aaGlob.adsLerp);
		}

		if (AimAssist_IsPlayerUsingOffhand(&aaGlob.ps))
		{
			*pitchScale = 1.0f;
		}
	}

	float Gamepad::AimAssist_Lerp(const float from, const float to, const float fraction)
	{
		return (to - from) * fraction + from;
	}

	void Gamepad::AimAssist_ApplyTurnRates(const Game::AimInput* input, Game::AimOutput* output)
	{
		assert(input);
		assert(output);
		AssertIn(input->localClientNum, Game::STATIC_MAX_LOCAL_CLIENTS);

		auto& aaGlob = Game::aaGlobArray[input->localClientNum];

		auto slowdownPitchScale = 0.0f;
		auto slowdownYawScale = 0.0f;
		float adjustedPitchAxis;
		float adjustedYawAxis;

		if (aaGlob.autoMeleeState == Game::AIM_MELEE_STATE_UPDATING)
		{
			adjustedPitchAxis = 0.0f;
			adjustedYawAxis = 0.0f;
			slowdownPitchScale = 1.0f;
			slowdownYawScale = 1.0f;
		}
		else
		{
			AimAssist_CalcAdjustedAxis(input, &adjustedPitchAxis, &adjustedYawAxis);
			AimAssist_CalcSlowdown(input, &slowdownPitchScale, &slowdownYawScale);
		}

		const auto sensitivity = input_viewSensitivity.get<float>();
		auto pitchTurnRate = AimAssist_Lerp(aim_turnrate_pitch.get<float>(), aim_turnrate_pitch_ads.get<float>(), aaGlob.adsLerp);
		pitchTurnRate = slowdownPitchScale * aaGlob.fovTurnRateScale * sensitivity * pitchTurnRate;
		auto yawTurnRate = AimAssist_Lerp(aim_turnrate_yaw.get<float>(), aim_turnrate_yaw_ads.get<float>(), aaGlob.adsLerp);
		yawTurnRate = slowdownYawScale * aaGlob.fovTurnRateScale * sensitivity * yawTurnRate;

		if (input->pitchMax > 0 && input->pitchMax < pitchTurnRate)
			pitchTurnRate = input->pitchMax;
		if (input->yawMax > 0 && input->yawMax < yawTurnRate)
			yawTurnRate = input->yawMax;

		const auto pitchSign = adjustedPitchAxis >= 0.0f ? 1.0f : -1.0f;
		const auto yawSign = adjustedYawAxis >= 0.0f ? 1.0f : -1.0f;

		const auto pitchDelta = std::fabs(adjustedPitchAxis) * pitchTurnRate;
		const auto yawDelta = std::fabs(adjustedYawAxis) * yawTurnRate;

		if (!aim_accel_turnrate_enabled.get<bool>())
		{
			aaGlob.pitchDelta = pitchDelta;
			aaGlob.yawDelta = yawDelta;
		}
		else
		{
			const auto accel = aim_accel_turnrate_lerp.get<float>() * sensitivity;
			if (pitchDelta <= aaGlob.pitchDelta)
				aaGlob.pitchDelta = pitchDelta;
			else
				aaGlob.pitchDelta = LinearTrack(pitchDelta, aaGlob.pitchDelta, accel, input->deltaTime);

			if (yawDelta <= aaGlob.yawDelta)
				aaGlob.yawDelta = yawDelta;
			else
				aaGlob.yawDelta = LinearTrack(yawDelta, aaGlob.yawDelta, accel, input->deltaTime);
		}

		output->pitch += aaGlob.pitchDelta * input->deltaTime * pitchSign;
		output->yaw += aaGlob.yawDelta * input->deltaTime * yawSign;
	}

	void Gamepad::AimAssist_UpdateGamePadInput(const Game::AimInput* input, Game::AimOutput* output)
	{
		assert(input);
		assert(output);
		AssertIn(input->localClientNum, Game::STATIC_MAX_LOCAL_CLIENTS);

		auto& aaGlob = Game::aaGlobArray[input->localClientNum];

		output->pitch = input->pitch;
		output->yaw = input->yaw;

		if (aaGlob.initialized)
		{
			Game::AimAssist_UpdateTweakables(input->localClientNum);
			Game::AimAssist_UpdateAdsLerp(input);
			AimAssist_ApplyTurnRates(input, output);
			Game::AimAssist_ApplyAutoMelee(input, output);
			AimAssist_ApplyLockOn(input, output);

			aaGlob.prevButtons = input->buttons;
		}
	}

	void Gamepad::CL_RemoteControlMove_GamePad(const int localClientNum, Game::usercmd_s* cmd)
	{
		// Buttons are already handled by keyboard input handler

		const auto up = CL_GamepadAxisValue(localClientNum, Game::GPAD_VIRTAXIS_FORWARD);
		const auto right = CL_GamepadAxisValue(localClientNum, Game::GPAD_VIRTAXIS_SIDE);
		const auto yaw = CL_GamepadAxisValue(localClientNum, Game::GPAD_VIRTAXIS_YAW);
		const auto pitch = CL_GamepadAxisValue(localClientNum, Game::GPAD_VIRTAXIS_PITCH);
		const auto sensitivity = input_viewSensitivity.get<float>();

		constexpr auto scale = static_cast<float>(std::numeric_limits<char>::max());
		cmd->remoteControlAngles[0] = ClampChar(cmd->remoteControlAngles[0] + static_cast<int>(std::floor(-up * scale * sensitivity))
			+ static_cast<int>(std::floor(-pitch * scale * sensitivity)));
		cmd->remoteControlAngles[1] = ClampChar(cmd->remoteControlAngles[1] + static_cast<int>(std::floor(-right * scale * sensitivity))
			+ static_cast<int>(std::floor(-yaw * scale * sensitivity)));
	}

	constexpr auto CL_RemoteControlMove = 0x5A6BA0;
	__declspec(naked) void Gamepad::CL_RemoteControlMove_Stub()
	{
		__asm
		{
			// Prepare args for our function call
			push edi // usercmd
			push eax // localClientNum

			call CL_RemoteControlMove

			// Call our function, the args were already prepared earlier
			call CL_RemoteControlMove_GamePad
			add esp, 0x8

			ret
		}
	}

	bool Gamepad::CG_HandleLocationSelectionInput_GamePad(const int localClientNum, Game::usercmd_s* /*cmd*/)
	{
		// Buttons are already handled by keyboard input handler

		const auto frameTime = static_cast<float>(Game::cgArray[0].frametime) * 0.001f;
		const auto mapAspectRatio = Game::cgArray[0].compassMapWorldSize[0] / Game::cgArray[0].compassMapWorldSize[1];
		const auto selectionRequiresAngle = (Game::cgArray[0].predictedPlayerState.locationSelectionInfo & 0x80) != 0;

		auto up = CL_GamepadAxisValue(localClientNum, Game::GPAD_VIRTAXIS_FORWARD);
		auto right = CL_GamepadAxisValue(localClientNum, Game::GPAD_VIRTAXIS_SIDE);
		auto magnitude = up * up + right * right;

		if (magnitude > 1.0f)
		{
			magnitude = std::sqrt(magnitude);
			up /= magnitude;
			right /= magnitude;
		}

		Game::cgArray[0].selectedLocation[0] += right * cg_mapLocationSelectionCursorSpeed.get<float>() * frameTime;
		Game::cgArray[0].selectedLocation[1] -= up * mapAspectRatio * cg_mapLocationSelectionCursorSpeed.get<float>() * frameTime;

		if (selectionRequiresAngle)
		{
			const auto yawUp = CL_GamepadAxisValue(localClientNum, Game::GPAD_VIRTAXIS_PITCH);
			const auto yawRight = CL_GamepadAxisValue(localClientNum, Game::GPAD_VIRTAXIS_YAW);

			if (std::fabs(yawUp) > 0.0f || std::fabs(yawRight) > 0.0f)
			{
				Game::vec2_t vec
				{
					yawUp,
					-yawRight
				};

				Game::cgArray[0].selectedLocationAngle = Game::AngleNormalize360(Game::vectoryaw(&vec));
				Game::cgArray[0].selectedAngleLocation[0] = Game::cgArray[0].selectedLocation[0];
				Game::cgArray[0].selectedAngleLocation[1] = Game::cgArray[0].selectedLocation[1];
			}
		}
		else
		{
			Game::cgArray[0].selectedAngleLocation[0] = Game::cgArray[0].selectedLocation[0];
			Game::cgArray[0].selectedAngleLocation[1] = Game::cgArray[0].selectedLocation[1];
		}

		return true;
	}

	constexpr auto CG_HandleLocationSelectionInput = 0x5A67A0;
	__declspec(naked) void Gamepad::CG_HandleLocationSelectionInput_Stub()
	{
		__asm
		{
			// Prepare args for our function call
			push esi // usercmd
			push eax // localClientNum

			call CG_HandleLocationSelectionInput

			test al,al
			jz exit_handling

			// Call our function, the args were already prepared earlier
			call CG_HandleLocationSelectionInput_GamePad

		exit_handling:
			add esp, 0x8
			ret
		}
	}

	bool Gamepad::CG_ShouldUpdateViewAngles(const int localClientNum)
	{
		return !Game::Key_IsCatcherActive(localClientNum, Game::KEYCATCH_MASK_ANY) || Game::UI_GetActiveMenu(localClientNum) == Game::UIMENU_SCOREBOARD;
	}

	float Gamepad::CL_GamepadAxisValue(const int localClientNum, const Game::GamepadVirtualAxis virtualAxis)
	{
		assert(virtualAxis > Game::GPAD_VIRTAXIS_NONE && virtualAxis < Game::GPAD_VIRTAXIS_COUNT);

		const auto& gamePadGlobal = gamePadGlobals[localClientNum];
		const auto& [physicalAxis, mapType] = gamePadGlobal.axes.virtualAxes[virtualAxis];

		if (physicalAxis <= Game::GPAD_PHYSAXIS_NONE || physicalAxis >= Game::GPAD_PHYSAXIS_COUNT)
		{
			return 0.0f;
		}

		auto axisDeflection = gamePadGlobal.axes.axesValues[physicalAxis];

		if (mapType == Game::GPAD_MAP_SQUARED)
		{
			const auto otherAxisSameStick = axisSameStick[physicalAxis];

			float otherAxisDeflection;
			if (otherAxisSameStick <= Game::GPAD_PHYSAXIS_NONE || otherAxisSameStick >= Game::GPAD_PHYSAXIS_COUNT)
			{
				otherAxisDeflection = 0.0f;
			}
			else
			{
				otherAxisDeflection = gamePadGlobal.axes.axesValues[otherAxisSameStick];
			}

			axisDeflection = std::sqrt(axisDeflection * axisDeflection + otherAxisDeflection * otherAxisDeflection) * axisDeflection;
		}

		return axisDeflection;
	}

	char Gamepad::ClampChar(const int value)
	{
		return static_cast<char>(std::clamp<int>(value, std::numeric_limits<char>::min(), std::numeric_limits<char>::max()));
	}

	void Gamepad::CL_GamepadMove(const int localClientNum, const float frameTimeBase, Game::usercmd_s* cmd)
	{
		AssertIn(localClientNum, Game::MAX_GPAD_COUNT);

		auto& gamePad = gamePads[localClientNum];
		auto& clientActive = Game::clients[localClientNum];

		auto pitch = CL_GamepadAxisValue(localClientNum, Game::GPAD_VIRTAXIS_PITCH);
		if (!input_invertPitch.get<bool>())
			pitch *= -1;

		auto yaw = -CL_GamepadAxisValue(localClientNum, Game::GPAD_VIRTAXIS_YAW);
		auto forward = CL_GamepadAxisValue(localClientNum, Game::GPAD_VIRTAXIS_FORWARD);
		auto side = CL_GamepadAxisValue(localClientNum, Game::GPAD_VIRTAXIS_SIDE);

		// The game implements an attack axis at this location. This axis is unused however so for this patch it was not implemented.
		//auto attack = CL_GamepadAxisValue(localClientNum, Game::GPAD_VIRTAXIS_ATTACK);

		auto moveScale = static_cast<float>(std::numeric_limits<char>::max());

		if (std::fabs(side) > 0.0f || std::fabs(forward) > 0.0f)
		{
			const auto length = std::fabs(side) <= std::fabs(forward)
									? side / forward
									: forward / side;
			moveScale = std::sqrt((length * length) + 1.0f) * moveScale;
		}

		const auto forwardMove = static_cast<int>(std::floor(forward * moveScale));
		const auto rightMove = static_cast<int>(std::floor(side * moveScale));

		cmd->rightmove = ClampChar(cmd->rightmove + rightMove);
		cmd->forwardmove = ClampChar(cmd->forwardmove + forwardMove);

		// Swap attack and throw buttons when using controller and akimbo to match "left trigger"="left weapon" and "right trigger"="right weapon"
		if (gamePad.inUse && clientActive.snap.ps.weapCommon.lastWeaponHand == Game::WEAPON_HAND_LEFT)
		{
			auto oldButtons = cmd->buttons;
			if (oldButtons & Game::CMD_BUTTON_ATTACK)
			{
				cmd->buttons |= Game::CMD_BUTTON_THROW;
			}
			else
			{
				cmd->buttons &= ~Game::CMD_BUTTON_THROW;
			}

			if (oldButtons & Game::CMD_BUTTON_THROW)
			{
				cmd->buttons |= Game::CMD_BUTTON_ATTACK;
			}
			else
			{
				cmd->buttons &= ~Game::CMD_BUTTON_ATTACK;
			}
		}

		// Check for frozen controls. Flag name should start with PMF_
		if (CG_ShouldUpdateViewAngles(localClientNum) && (clientActive.snap.ps.pm_flags & Game::PMF_FROZEN) == 0)
		{
			Game::AimInput aimInput{};
			Game::AimOutput aimOutput{};
			aimInput.deltaTime = frameTimeBase;
			aimInput.buttons = cmd->buttons;
			aimInput.localClientNum = localClientNum;
			aimInput.deltaTimeScaled = static_cast<float>(Game::cls->frametime) * 0.001f;
			aimInput.pitch = clientActive.clViewangles[0];
			aimInput.pitchAxis = pitch;
			aimInput.pitchMax = clientActive.cgameMaxPitchSpeed;
			aimInput.yaw = clientActive.clViewangles[1];
			aimInput.yawAxis = yaw;
			aimInput.yawMax = clientActive.cgameMaxYawSpeed;
			aimInput.forwardAxis = forward;
			aimInput.rightAxis = side;
			AimAssist_UpdateGamePadInput(&aimInput, &aimOutput);

			cmd->meleeChargeDist = aimOutput.meleeChargeDist;
			cmd->meleeChargeYaw = aimOutput.meleeChargeYaw;

			clientActive.clViewangles[0] = aimOutput.pitch;
			clientActive.clViewangles[1] = aimOutput.yaw;
		}
	}

	void Gamepad::CL_MouseMove(const int localClientNum, Game::usercmd_s* cmd, const float frametime_base)
	{
		auto& gamePad = gamePads[localClientNum];
		if (!gamePad.inUse)
		{
			Game::CL_MouseMove(localClientNum, cmd, frametime_base);
		}
		else if (gpad_enabled.get<bool>() && gamePad.enabled)
		{
			CL_GamepadMove(localClientNum, frametime_base, cmd);
		}
	}

	__declspec(naked) void Gamepad::CL_MouseMove_Stub()
	{
		__asm
		{
			pushad

			push [esp + 0x20 + 0x4] // frametime_base
			push ebx // cmd
			push eax // localClientNum
			call CL_MouseMove
			add esp, 0xC

			popad

			ret
		}
	}

	bool Gamepad::Gamepad_ShouldUse(const Game::gentity_s* playerEnt, const unsigned useTime)
	{
		// Only apply hold time to +usereload keybind
		return !(playerEnt->client->buttons & Game::CMD_BUTTON_USE_RELOAD) || useTime >= static_cast<unsigned>(gpad_use_hold_time.get<int>());
	}

	__declspec(naked) void Gamepad::Player_UseEntity_Stub()
	{
		__asm
		{
			// Execute overwritten instructions
			cmp eax, [ecx + 0x10]
			jl skipUse

			// Call our custom check
			push eax
			pushad
			push eax
			push edi
			call Gamepad_ShouldUse
			add esp, 8h
			mov [esp + 0x20], eax
			popad
			pop eax

			// Skip use if custom check returns false
			test al, al
			jz skipUse

			// perform use
			push 0x5FE39B
			ret

		skipUse:
			push 0x5FE3AF
			ret
		}
	}

	bool Gamepad::Key_IsValidGamePadChar(const int key)
	{
		return key >= Game::K_FIRSTGAMEPADBUTTON_RANGE_1 && key <= Game::K_LASTGAMEPADBUTTON_RANGE_1
			|| key >= Game::K_FIRSTGAMEPADBUTTON_RANGE_2 && key <= Game::K_LASTGAMEPADBUTTON_RANGE_2
			|| key >= Game::K_FIRSTGAMEPADBUTTON_RANGE_3 && key <= Game::K_LASTGAMEPADBUTTON_RANGE_3;
	}

	void Gamepad::CL_GamepadResetMenuScrollTime(const int localClientNum, const int key, const bool down, const unsigned time)
	{
		AssertIn(localClientNum, Game::MAX_GPAD_COUNT);

		auto& gamePadGlobal = gamePadGlobals[localClientNum];

		if (!down)
		{
			return;
		}

		const auto scrollDelayFirst = gpad_menu_scroll_delay_first.get<int>();
		for (const auto scrollButton : menuScrollButtonList)
		{
			if (key == scrollButton)
			{
				gamePadGlobal.nextScrollTime = scrollDelayFirst + time;
				return;
			}
		}
	}

	void Gamepad::CL_GamepadGenerateAPad(const int localClientNum, const Game::GamepadPhysicalAxis physicalAxis, unsigned time)
	{
		AssertIn(localClientNum, Game::MAX_GPAD_COUNT);
		assert(physicalAxis >= 0 && physicalAxis < Game::GPAD_PHYSAXIS_COUNT);

		auto& gamePad = gamePads[localClientNum];

		const auto stick = stickForAxis[physicalAxis];
		const auto stickIndex = stick & Game::GPAD_VALUE_MASK;
		if (stick != Game::GPAD_INVALID)
		{
			assert(stickIndex < 4);
			const auto& mapping = analogStickList[stickIndex];

			if (gamePad.stickDown[stickIndex][Game::GPAD_STICK_POS])
			{
				const Game::GamePadButtonEvent event = gamePad.stickDownLast[stickIndex][Game::GPAD_STICK_POS] ? Game::GPAD_BUTTON_UPDATE : Game::GPAD_BUTTON_PRESSED;
				CL_GamepadButtonEvent(localClientNum, mapping.posCode, event, time);
			}
			else if (gamePad.stickDown[stickIndex][Game::GPAD_STICK_NEG])
			{
				const Game::GamePadButtonEvent event = gamePad.stickDownLast[stickIndex][Game::GPAD_STICK_NEG] ? Game::GPAD_BUTTON_UPDATE : Game::GPAD_BUTTON_PRESSED;
				CL_GamepadButtonEvent(localClientNum, mapping.negCode, event, time);
			}
			else if (gamePad.stickDownLast[stickIndex][Game::GPAD_STICK_POS])
			{
				CL_GamepadButtonEvent(localClientNum, mapping.posCode, Game::GPAD_BUTTON_RELEASED, time);
			}
			else if (gamePad.stickDownLast[stickIndex][Game::GPAD_STICK_NEG])
			{
				CL_GamepadButtonEvent(localClientNum, mapping.negCode, Game::GPAD_BUTTON_RELEASED, time);
			}
		}
	}

	void Gamepad::CL_GamepadEvent(const int localClientNum, const Game::GamepadPhysicalAxis physicalAxis, const float value, const unsigned time)
	{
		AssertIn(localClientNum, Game::MAX_GPAD_COUNT);
		assert(physicalAxis >= 0 && physicalAxis < Game::GPAD_PHYSAXIS_COUNT);

		auto& gamePad = gamePads[localClientNum];
		auto& gamePadGlobal = gamePadGlobals[localClientNum];

		gamePadGlobal.axes.axesValues[physicalAxis] = value;
		CL_GamepadGenerateAPad(localClientNum, physicalAxis, time);

		if (std::fabs(value) > 0.0f)
		{
			gamePad.inUse = true;
			gpad_in_use.setRaw(true);
		}
	}

	void Gamepad::UI_GamepadKeyEvent(const int localClientNum, const int key, const bool down)
	{
		// If we are currently capturing a key for menu bind inputs then do not map keys and pass to game
		if (*Game::g_waitingForKey)
		{
			Game::UI_KeyEvent(localClientNum, key, down);
			return;
		}

		for (const auto& mapping : controllerMenuKeyMappings)
		{
			if (mapping.controllerKey == key)
			{
				Game::UI_KeyEvent(localClientNum, mapping.pcKey, down);
				return;
			}
		}

		// No point in sending unmapped controller keystrokes to the key event handler since it doesn't know how to use it anyway
		// Game::UI_KeyEvent(localClientNum, key, down);
	}

	bool Gamepad::Scoreboard_HandleInput(int localClientNum, int key)
	{
		AssertIn(localClientNum, Game::STATIC_MAX_LOCAL_CLIENTS);

		auto& keyState = Game::playerKeys[localClientNum];

		if (keyState.keys[key].binding && std::strcmp(keyState.keys[key].binding, "togglescores") == 0)
		{
			Game::Cbuf_AddText(localClientNum, "togglescores\n");
			return true;
		}

		switch (key)
		{
		case Game::K_DPAD_UP:
			Game::CG_ScrollScoreboardUp(Game::cgArray);
			return true;

		case Game::K_DPAD_DOWN:
			Game::CG_ScrollScoreboardDown(Game::cgArray);
			return true;

		default:
			return false;
		}
	}

	bool Gamepad::CL_CheckForIgnoreDueToRepeat(const int localClientNum, const int key, const int repeatCount, const unsigned time)
	{
		AssertIn(localClientNum, Game::STATIC_MAX_LOCAL_CLIENTS);

		auto& gamePadGlobal = gamePadGlobals[localClientNum];

		if (Game::Key_IsCatcherActive(localClientNum, Game::KEYCATCH_UI))
		{
			const int scrollDelayFirst = gpad_menu_scroll_delay_first.get<int>();
			const int scrollDelayRest = gpad_menu_scroll_delay_rest.get<int>();

			for (const auto menuScrollButton : menuScrollButtonList)
			{
				if (key == menuScrollButton)
				{
					if (repeatCount == 1)
					{
						gamePadGlobal.nextScrollTime = time + scrollDelayFirst;
						return false;
					}

					if (time > gamePadGlobal.nextScrollTime)
					{
						gamePadGlobal.nextScrollTime = time + scrollDelayRest;
						return false;
					}
					break;
				}
			}
		}

		return repeatCount > 1;
	}

	void Gamepad::CL_GamepadButtonEvent(const int localClientNum, const int key, const Game::GamePadButtonEvent buttonEvent, const unsigned time)
	{
		AssertIn(localClientNum, Game::MAX_GPAD_COUNT);

		const auto pressed = buttonEvent == Game::GPAD_BUTTON_PRESSED;
		const auto pressedOrUpdated = pressed || buttonEvent == Game::GPAD_BUTTON_UPDATE;

		auto& keyState = Game::playerKeys[localClientNum];
		keyState.keys[key].down = pressedOrUpdated;

		if (pressedOrUpdated)
		{
			if (++keyState.keys[key].repeats == 1)
			{
				keyState.anyKeyDown++;
			}
		}
		else if (buttonEvent == Game::GPAD_BUTTON_RELEASED && keyState.keys[key].repeats > 0)
		{
			keyState.keys[key].repeats = 0;
			if (--keyState.anyKeyDown < 0)
			{
				keyState.anyKeyDown = 0;
			}
		}

		if (pressedOrUpdated && CL_CheckForIgnoreDueToRepeat(localClientNum, key, keyState.keys[key].repeats, time))
			return;

		if (Game::Key_IsCatcherActive(localClientNum, Game::KEYCATCH_LOCATION_SELECTION) && pressedOrUpdated)
		{
			if (key == Game::K_BUTTON_B || keyState.keys[key].binding && std::strcmp(keyState.keys[key].binding, "+actionslot 4") == 0)
			{
				keyState.locSelInputState = Game::LOC_SEL_INPUT_CANCEL;
			}
			else if (key == Game::K_BUTTON_A || keyState.keys[key].binding && std::strcmp(keyState.keys[key].binding, "+attack") == 0)
			{
				keyState.locSelInputState = Game::LOC_SEL_INPUT_CONFIRM;
			}
			return;
		}

		const auto activeMenu = Game::UI_GetActiveMenu(localClientNum);
		if (activeMenu == Game::UIMENU_SCOREBOARD)
		{
			if (buttonEvent == Game::GPAD_BUTTON_PRESSED && Scoreboard_HandleInput(localClientNum, key))
			{
				return;
			}
		}

		keyState.locSelInputState = Game::LOC_SEL_INPUT_NONE;

		const auto* keyBinding = keyState.keys[key].binding;

		char cmd[1024];
		if (pressedOrUpdated)
		{
			if (Game::Key_IsCatcherActive(localClientNum, Game::KEYCATCH_UI))
			{
				UI_GamepadKeyEvent(localClientNum, key, pressedOrUpdated);
				return;
			}

			if (keyBinding)
			{
				if (keyBinding[0] == '+')
				{
					sprintf_s(cmd, "%s %i %i\n", keyBinding, key, time);
					Game::Cbuf_AddText(localClientNum, cmd);
				}
				else
				{
					Game::Cbuf_InsertText(localClientNum, keyBinding);
				}
			}
		}
		else
		{
			if (keyBinding && keyBinding[0] == '+')
			{
				sprintf_s(cmd, "-%s %i %i\n", &keyBinding[1], key, time);
				Game::Cbuf_AddText(localClientNum, cmd);
			}

			if (Game::Key_IsCatcherActive(localClientNum, Game::KEYCATCH_UI))
			{
				UI_GamepadKeyEvent(localClientNum, key, pressedOrUpdated);
			}
		}
	}

	void Gamepad::CL_GamepadButtonEventForPort(const int localClientNum, const int key, const Game::GamePadButtonEvent buttonEvent, const unsigned time)
	{
		AssertIn(localClientNum, Game::MAX_GPAD_COUNT);

		auto& gamePad = gamePads[localClientNum];
		gamePad.inUse = true;
		gpad_in_use.setRaw(true);

		if (Game::Key_IsCatcherActive(localClientNum, Game::KEYCATCH_UI))
		{
			CL_GamepadResetMenuScrollTime(localClientNum, key, buttonEvent == Game::GPAD_BUTTON_PRESSED, time);
		}


		CL_GamepadButtonEvent(localClientNum, key, buttonEvent, time);
	}

	void Gamepad::GPad_ConvertStickToFloat(const short x, const short y, float& outX, float& outY)
	{
		if (x == 0 && y == 0)
		{
			outX = 0.0f;
			outY = 0.0f;
			return;
		}

		Game::vec2_t stickVec;
		stickVec[0] = static_cast<float>(x) / static_cast<float>(std::numeric_limits<short>::max());
		stickVec[1] = static_cast<float>(y) / static_cast<float>(std::numeric_limits<short>::max());

		const auto deadZoneTotal = gpad_stick_deadzone_min.get<float>() + gpad_stick_deadzone_max.get<float>();
		auto len = Game::Vec2Normalize(stickVec);

		if (gpad_stick_deadzone_min.get<float>() <= len)
		{
			if (1.0f - gpad_stick_deadzone_max.get<float>() >= len)
			{
				len = (len - gpad_stick_deadzone_min.get<float>()) / (1.0f - deadZoneTotal);
			}
			else
			{
				len = 1.0f;
			}
		}
		else
		{
			len = 0.0f;
		}

		outX = stickVec[0] * len;
		outY = stickVec[1] * len;
	}

	float Gamepad::GPad_GetStick(const int localClientNum, const Game::GamePadStick stick)
	{
		AssertIn(localClientNum, Game::MAX_GPAD_COUNT);
		assert(stick & Game::GPAD_STICK_MASK);

		auto& gamePad = gamePads[localClientNum];
		return gamePad.sticks[stick];
	}

	float Gamepad::GPad_GetButton(const int localClientNum, Game::GamePadButton button)
	{
		AssertIn(localClientNum, Game::MAX_GPAD_COUNT);
		auto& gamePad = gamePads[localClientNum];

		float value = 0.0f;

		if (button & Game::GPAD_DIGITAL_MASK)
		{
			const auto buttonValue = button & Game::GPAD_VALUE_MASK;
			value = buttonValue & gamePad.digitals ? 1.0f : 0.0f;
		}
		else if (button & Game::GPAD_ANALOG_MASK)
		{
			const auto analogIndex = button & Game::GPAD_VALUE_MASK;
			if (analogIndex < std::extent_v<decltype(gamePad.analogs)>)
			{
				value = gamePad.analogs[analogIndex];
			}
		}

		return value;
	}

	bool Gamepad::GPad_IsButtonPressed(const int localClientNum, Game::GamePadButton button)
	{
		AssertIn(localClientNum, Game::MAX_GPAD_COUNT);
		assert(button & (Game::GPAD_DIGITAL_MASK | Game::GPAD_ANALOG_MASK));

		auto& gamePad = gamePads[localClientNum];

		bool down = false;
		bool lastDown = false;

		if (button & Game::GPAD_DIGITAL_MASK)
		{
			const auto buttonValue = button & Game::GPAD_VALUE_MASK;
			down = (buttonValue & gamePad.digitals) != 0;
			lastDown = (buttonValue & gamePad.lastDigitals) != 0;
		}
		else if (button & Game::GPAD_ANALOG_MASK)
		{
			const auto analogIndex = button & Game::GPAD_VALUE_MASK;
			assert(analogIndex < std::extent_v<decltype(gamePad.analogs)>);

			if (analogIndex < std::extent_v<decltype(gamePad.analogs)>)
			{
				down = gamePad.analogs[analogIndex] > 0.0f;
				lastDown = gamePad.lastAnalogs[analogIndex] > 0.0f;
			}
		}

		return down && !lastDown;
	}

	bool Gamepad::GPad_ButtonRequiresUpdates(const int localClientNum, Game::GamePadButton button)
	{
		return (button & Game::GPAD_ANALOG_MASK || button & Game::GPAD_DPAD_MASK) && GPad_GetButton(localClientNum, button) > 0.0f;
	}

	bool Gamepad::GPad_IsButtonReleased(int localClientNum, Game::GamePadButton button)
	{
		AssertIn(localClientNum, Game::MAX_GPAD_COUNT);

		auto& gamePad = gamePads[localClientNum];

		bool down = false;
		bool lastDown = false;

		if (button & Game::GPAD_DIGITAL_MASK)
		{
			const auto buttonValue = button & Game::GPAD_VALUE_MASK;

			down = (gamePad.digitals & buttonValue) != 0;
			lastDown = (gamePad.lastDigitals & buttonValue) != 0;
		}
		else if (button & Game::GPAD_ANALOG_MASK)
		{
			const auto analogIndex = button & Game::GPAD_VALUE_MASK;
			assert(analogIndex < std::extent_v<decltype(gamePad.analogs)>);

			if (analogIndex < std::extent_v<decltype(gamePad.analogs)>)
			{
				down = gamePad.analogs[analogIndex] > 0.0f;
				lastDown = gamePad.lastAnalogs[analogIndex] > 0.0f;
			}
		}

		return !down && lastDown;
	}

	void Gamepad::GPad_UpdateSticksDown(const int localClientNum)
	{
		AssertIn(localClientNum, Game::MAX_GPAD_COUNT);

		for (auto stickIndex = 0u; stickIndex < std::extent_v<decltype(GamePad::sticks)>; stickIndex++)
		{
			for (auto dir = 0; dir < Game::GPAD_STICK_DIR_COUNT; dir++)
			{
				auto& gamePad = gamePads[localClientNum];
				gamePad.stickDownLast[stickIndex][dir] = gamePad.stickDown[stickIndex][dir];

				auto threshold = gpad_stick_pressed.get<float>();

				if (gamePad.stickDownLast[stickIndex][dir])
				{
					threshold -= gpad_stick_pressed_hysteresis.get<float>();
				}
				else
				{
					threshold += gpad_stick_pressed_hysteresis.get<float>();
				}

				if (dir == Game::GPAD_STICK_POS)
				{
					gamePad.stickDown[stickIndex][dir] = gamePad.sticks[stickIndex] > threshold;
				}
				else
				{
					assert(dir == Game::GPAD_STICK_NEG);
					gamePad.stickDown[stickIndex][dir] = gamePad.sticks[stickIndex] < -threshold;
				}
			}
		}
	}

	void Gamepad::GPad_UpdateSticks(const int localClientNum, const XINPUT_GAMEPAD& state)
	{
		AssertIn(localClientNum, Game::MAX_GPAD_COUNT);

		auto& gamePad = gamePads[localClientNum];

		Game::vec2_t lVec, rVec;
		GPad_ConvertStickToFloat(state.sThumbLX, state.sThumbLY, lVec[0], lVec[1]);
		GPad_ConvertStickToFloat(state.sThumbRX, state.sThumbRY, rVec[0], rVec[1]);

		gamePad.lastSticks[0] = gamePad.sticks[0];
		gamePad.sticks[0] = lVec[0];
		gamePad.lastSticks[1] = gamePad.sticks[1];
		gamePad.sticks[1] = lVec[1];
		gamePad.lastSticks[2] = gamePad.sticks[2];
		gamePad.sticks[2] = rVec[0];
		gamePad.lastSticks[3] = gamePad.sticks[3];
		gamePad.sticks[3] = rVec[1];

		GPad_UpdateSticksDown(localClientNum);

		if (gpad_debug.get<bool>())
		{
			Logger::Debug("Left: X: {:f} Y: {:f}", lVec[0], lVec[1]);
			Logger::Debug("Right: X: {:f} Y: {:f}", rVec[0], rVec[1]);
			Logger::Debug("Down: {}:{} {}:{} {}:{} {}:{}", gamePad.stickDown[0][Game::GPAD_STICK_POS], gamePad.stickDown[0][Game::GPAD_STICK_NEG],
						gamePad.stickDown[1][Game::GPAD_STICK_POS], gamePad.stickDown[1][Game::GPAD_STICK_NEG],
						gamePad.stickDown[2][Game::GPAD_STICK_POS], gamePad.stickDown[2][Game::GPAD_STICK_NEG],
						gamePad.stickDown[3][Game::GPAD_STICK_POS], gamePad.stickDown[3][Game::GPAD_STICK_NEG]);
		}
	}

	void Gamepad::GPad_UpdateDigitals(const int localClientNum, const XINPUT_GAMEPAD& state)
	{
		AssertIn(localClientNum, Game::MAX_GPAD_COUNT);

		auto& gamePad = gamePads[localClientNum];

		gamePad.lastDigitals = gamePad.digitals;
		gamePad.digitals = state.wButtons;

		const auto leftDeflect = gpad_button_lstick_deflect_max.get<float>();
		if (std::fabs(gamePad.sticks[0]) > leftDeflect || std::fabs(gamePad.sticks[1]) > leftDeflect)
		{
			gamePad.digitals &= ~static_cast<short>(XINPUT_GAMEPAD_LEFT_THUMB);
		}

		const auto rightDeflect = gpad_button_rstick_deflect_max.get<float>();
		if (std::fabs(gamePad.sticks[2]) > leftDeflect || std::fabs(gamePad.sticks[3]) > rightDeflect)
		{
			gamePad.digitals &= ~static_cast<short>(XINPUT_GAMEPAD_RIGHT_THUMB);
		}

		if (gpad_debug.get<bool>())
		{
			Logger::Debug("Buttons: {:#x}", gamePad.digitals);
		}
	}

	void Gamepad::GPad_UpdateAnalogs(const int localClientNum, const XINPUT_GAMEPAD& state)
	{
		AssertIn(localClientNum, Game::MAX_GPAD_COUNT);

		auto& gamePad = gamePads[localClientNum];

		const auto buttonDeadZone = gpad_button_deadzone.get<float>();

		gamePad.lastAnalogs[0] = gamePad.analogs[0];
		gamePad.analogs[0] = static_cast<float>(state.bLeftTrigger) / static_cast<float>(std::numeric_limits<unsigned char>::max());
		if (gamePad.analogs[0] < buttonDeadZone)
		{
			gamePad.analogs[0] = 0.0f;
		}

		gamePad.lastAnalogs[1] = gamePad.analogs[1];
		gamePad.analogs[1] = static_cast<float>(state.bRightTrigger) / static_cast<float>(std::numeric_limits<unsigned char>::max());
		if (gamePad.analogs[1] < buttonDeadZone)
		{
			gamePad.analogs[1] = 0.0f;
		}

		if (gpad_debug.get<bool>())
		{
			Logger::Debug("Triggers: {:f} {:f}", gamePad.analogs[0], gamePad.analogs[1]);
		}
	}

	void Gamepad::GPad_UpdateAll()
	{
		GPad_RefreshAll();

		for (auto localClientNum = 0; localClientNum < Game::MAX_GPAD_COUNT; ++localClientNum)
		{
			const auto& gamePad = gamePads[localClientNum];
			if (!gamePad.enabled)
			{
				continue;
			}

			XINPUT_STATE inputState;
			if (XInputGetState(gamePad.portIndex, &inputState) != ERROR_SUCCESS)
			{
				continue;
			}

			GPad_UpdateSticks(localClientNum, inputState.Gamepad);
			GPad_UpdateDigitals(localClientNum, inputState.Gamepad);
			GPad_UpdateAnalogs(localClientNum, inputState.Gamepad);
		}
	}

	void Gamepad::IN_GamePadsMove()
	{
		if (!gpad_enabled.get<bool>())
			return;

		GPad_UpdateAll();
		const auto time = Game::Sys_Milliseconds();

		bool gpadPresent = false;
		for (auto localClientNum = 0; localClientNum < Game::MAX_GPAD_COUNT; ++localClientNum)
		{
			const auto& gamePad = gamePads[localClientNum];
			if (!gamePad.enabled)
			{
				continue;
			}

			gpadPresent = true;
			const auto lx = GPad_GetStick(localClientNum, Game::GPAD_LX);
			const auto ly = GPad_GetStick(localClientNum, Game::GPAD_LY);
			const auto rx = GPad_GetStick(localClientNum, Game::GPAD_RX);
			const auto ry = GPad_GetStick(localClientNum, Game::GPAD_RY);
			const auto leftTrig = GPad_GetButton(localClientNum, Game::GPAD_L_TRIG);
			const auto rightTrig = GPad_GetButton(localClientNum, Game::GPAD_R_TRIG);

			CL_GamepadEvent(localClientNum, Game::GPAD_PHYSAXIS_LSTICK_X, lx, time);
			CL_GamepadEvent(localClientNum, Game::GPAD_PHYSAXIS_LSTICK_Y, ly, time);
			CL_GamepadEvent(localClientNum, Game::GPAD_PHYSAXIS_RSTICK_X, rx, time);
			CL_GamepadEvent(localClientNum, Game::GPAD_PHYSAXIS_RSTICK_Y, ry, time);
			CL_GamepadEvent(localClientNum, Game::GPAD_PHYSAXIS_LTRIGGER, leftTrig, time);
			CL_GamepadEvent(localClientNum, Game::GPAD_PHYSAXIS_RTRIGGER, rightTrig, time);

			for (const auto& buttonMapping : buttonList)
			{
				if (GPad_IsButtonPressed(localClientNum, buttonMapping.padButton))
				{
					CL_GamepadButtonEventForPort(localClientNum, buttonMapping.code, Game::GPAD_BUTTON_PRESSED, time);
				}
				else if (GPad_ButtonRequiresUpdates(localClientNum, buttonMapping.padButton))
				{
					CL_GamepadButtonEventForPort(localClientNum, buttonMapping.code, Game::GPAD_BUTTON_UPDATE, time);
				}
				else if (GPad_IsButtonReleased(localClientNum, buttonMapping.padButton))
				{
					CL_GamepadButtonEventForPort(localClientNum, buttonMapping.code, 	Game::GPAD_BUTTON_RELEASED, time);
				}
			}
		}

		gpad_present.setRaw(gpadPresent);
	}

	void Gamepad::IN_Frame_Hk()
	{
		RawMouse::IN_MouseMove();

		IN_GamePadsMove();
	}

	void Gamepad::Gamepad_WriteBindings(const int localClientNum, const int handle)
	{
		AssertIn(localClientNum, Game::MAX_GPAD_COUNT);

		auto& gamePadGlobal = gamePadGlobals[localClientNum];

		Game::FS_Printf(handle, "unbindallaxis\n");

		for (auto virtualAxisIndex = 0u; virtualAxisIndex < Game::GPAD_VIRTAXIS_COUNT; ++virtualAxisIndex)
		{
			const auto& axisMapping = gamePadGlobal.axes.virtualAxes[virtualAxisIndex];
			if (axisMapping.physicalAxis <= Game::GPAD_PHYSAXIS_NONE || axisMapping.physicalAxis >= Game::GPAD_PHYSAXIS_COUNT ||
				axisMapping.mapType <= Game::GPAD_MAP_NONE || axisMapping.mapType >= Game::GPAD_MAP_COUNT)
			{
				continue;
			}

			const auto* physicalAxisName = physicalAxisNames[axisMapping.physicalAxis];
			const auto* virtualAxisName = virtualAxisNames[virtualAxisIndex];
			const auto* mappingName = gamePadMappingTypeNames[axisMapping.mapType];

			Game::FS_Printf(handle, "bindaxis %s %s %s\n", physicalAxisName, virtualAxisName, mappingName);
		}
	}

	void Gamepad::Key_WriteBindings_Hk(const int localClientNum, const int handle)
	{
		// Call original function
		Utils::Hook::Call<void(int, int)>(0x4A5A20)(localClientNum, handle);

		Gamepad_WriteBindings(0, handle);
	}

	void __declspec(naked) Gamepad::Com_WriteConfiguration_Modified_Stub()
	{
		__asm
		{
			mov eax, [ecx + 0x18]
			or eax, gamePadBindingsModifiedFlags // Also check for gamePadBindingsModifiedFlags
			test al, 1
			jz endMethod
			mov gamePadBindingsModifiedFlags, 0 // Reset gamePadBindingsModifiedFlags
			mov eax, [ecx + 0x18] // Restore eax to dvar_modified_flags

			push 0x60B26E
			retn

		endMethod:
			push 0x60B298
			retn
		}
	}

	void Gamepad::Gamepad_BindAxis(const int localClientNum, const Game::GamepadPhysicalAxis realIndex, const Game::GamepadVirtualAxis axisIndex, const Game::GamepadMapping mapType)
	{
		AssertIn(localClientNum, Game::MAX_GPAD_COUNT);
		assert(realIndex > Game::GPAD_PHYSAXIS_NONE && realIndex < Game::GPAD_PHYSAXIS_COUNT);
		assert(axisIndex > Game::GPAD_VIRTAXIS_NONE && axisIndex < Game::GPAD_VIRTAXIS_COUNT);
		assert(mapType > Game::GPAD_MAP_NONE && mapType < Game::GPAD_MAP_COUNT);

		auto& gamePadGlobal = gamePadGlobals[localClientNum];
		gamePadGlobal.axes.virtualAxes[axisIndex].physicalAxis = realIndex;
		gamePadGlobal.axes.virtualAxes[axisIndex].mapType = mapType;

		gamePadBindingsModifiedFlags |= 1;
	}

	Game::GamepadPhysicalAxis Gamepad::StringToPhysicalAxis(const char* str)
	{
		for (std::size_t i = 0; i < std::extent_v<decltype(physicalAxisNames)>; i++)
		{
			if (std::strcmp(str, physicalAxisNames[i]) == 0)
			{
				return static_cast<Game::GamepadPhysicalAxis>(i);
			}
		}

		return Game::GPAD_PHYSAXIS_NONE;
	}

	Game::GamepadVirtualAxis Gamepad::StringToVirtualAxis(const char* str)
	{
		for (std::size_t i = 0; i < std::extent_v<decltype(virtualAxisNames)>; ++i)
		{
			if (std::strcmp(str, virtualAxisNames[i]) == 0)
			{
				return static_cast<Game::GamepadVirtualAxis>(i);
			}
		}

		return Game::GPAD_VIRTAXIS_NONE;
	}

	Game::GamepadMapping Gamepad::StringToGamePadMapping(const char* str)
	{
		for (std::size_t i = 0; i < std::extent_v<decltype(gamePadMappingTypeNames)>; ++i)
		{
			if (std::strcmp(str, gamePadMappingTypeNames[i]) == 0)
			{
				return static_cast<Game::GamepadMapping>(i);
			}
		}

		return Game::GPAD_MAP_NONE;
	}

	void Gamepad::Axis_Bind_f(const Command::Params* params)
	{
		if (params->size() < 4)
		{
			Logger::Print("bindaxis <real axis> <virtual axis> <input type>\n");
			return;
		}

		const auto* physicalAxisText = params->get(1);
		const auto* virtualAxisText = params->get(2);
		const auto* mappingText = params->get(3);

		const Game::GamepadPhysicalAxis physicalAxis = StringToPhysicalAxis(physicalAxisText);
		if (physicalAxis == Game::GPAD_PHYSAXIS_NONE)
		{
			Logger::Print("\"{}\" isn't a valid physical axis\n", physicalAxisText);
			return;
		}

		const Game::GamepadVirtualAxis virtualAxis = StringToVirtualAxis(virtualAxisText);
		if (virtualAxis == Game::GPAD_VIRTAXIS_NONE)
		{
			Logger::Print("\"{}\" isn't a valid virtual axis\n", virtualAxisText);
			return;
		}

		const Game::GamepadMapping mapping = StringToGamePadMapping(mappingText);
		if (mapping == Game::GPAD_MAP_NONE)
		{
			Logger::Print("\"{}\" isn't a valid input type\n", mappingText);
			return;
		}

		Gamepad_BindAxis(0, physicalAxis, virtualAxis, mapping);
	}

	void Gamepad::Axis_Unbindall_f()
	{
		auto& gamePadGlobal = gamePadGlobals[0];

		for (auto& virtualAxis : gamePadGlobal.axes.virtualAxes)
		{
			virtualAxis.physicalAxis = Game::GPAD_PHYSAXIS_NONE;
			virtualAxis.mapType = Game::GPAD_MAP_NONE;
		}
	}

	void Gamepad::Bind_GP_SticksConfigs_f()
	{
		const auto* stickConfigName = gpad_sticksConfig.get<const char*>();
		Game::Cbuf_AddText(0, Utils::String::VA("exec %s\n", stickConfigName));
	}

	void Gamepad::Bind_GP_ButtonsConfigs_f()
	{
		const auto* buttonConfigName = gpad_buttonConfig.get<const char*>();
		Game::Cbuf_AddText(0, Utils::String::VA("exec %s\n", buttonConfigName));
	}

	void Gamepad::Scores_Toggle_f()
	{
		if (Game::cgArray[0].nextSnap)
		{
			if (Game::UI_GetActiveMenu(0) != Game::UIMENU_SCOREBOARD)
			{
				Game::CG_ScoresDown_f();
			}
			else
			{
				Game::CG_ScoresUp_f();
			}
		}
	}

	void Gamepad::InitDvars()
	{
		gpad_enabled = Dvar::Register<bool>("gpad_enabled", false, Game::DVAR_ARCHIVE, "Game pad enabled");
		gpad_debug = Dvar::Register<bool>("gpad_debug", false, Game::DVAR_NONE, "Game pad debugging");
		gpad_present = Dvar::Register<bool>("gpad_present", false, Game::DVAR_ROM, "Game pad present");
		gpad_in_use = Dvar::Register<bool>("gpad_in_use", false, Game::DVAR_ROM, "A game pad is in use");
		gpad_style = Dvar::Register<bool>("gpad_style", false, Game::DVAR_ARCHIVE, "Switch between Xbox and PS HUD");
		gpad_sticksConfig = Dvar::Register<const char*>("gpad_sticksConfig", "", Game::DVAR_ARCHIVE, "Game pad stick configuration");
		gpad_buttonConfig = Dvar::Register<const char*>("gpad_buttonConfig", "", Game::DVAR_ARCHIVE, "Game pad button configuration");
		gpad_menu_scroll_delay_first = Dvar::Register<int>("gpad_menu_scroll_delay_first", 420, 0, 1000, Game::DVAR_ARCHIVE, "Menu scroll key-repeat delay, for the first repeat, in milliseconds");
		gpad_menu_scroll_delay_rest = Dvar::Register<int>("gpad_menu_scroll_delay_rest", 210, 0, 1000, Game::DVAR_ARCHIVE,
														  "Menu scroll key-repeat delay, for repeats after the first, in milliseconds");
		gpad_rumble = Dvar::Register<bool>("gpad_rumble", true, Game::DVAR_ARCHIVE, "Enable game pad rumble");
		gpad_stick_pressed_hysteresis = Dvar::Register<float>("gpad_stick_pressed_hysteresis", 0.1f, 0.0f, 1.0f, Game::DVAR_ARCHIVE,
															  "Game pad stick pressed no-change-zone around gpad_stick_pressed to prevent bouncing");
		gpad_stick_pressed = Dvar::Register<float>("gpad_stick_pressed", 0.4f, 0.0, 1.0, Game::DVAR_ARCHIVE, "Game pad stick pressed threshhold");
		gpad_stick_deadzone_max = Dvar::Register<float>("gpad_stick_deadzone_max", 0.01f, 0.0f, 1.0f, Game::DVAR_ARCHIVE, "Game pad maximum stick deadzone");
		gpad_stick_deadzone_min = Dvar::Register<float>("gpad_stick_deadzone_min", 0.2f, 0.0f, 1.0f, Game::DVAR_ARCHIVE, "Game pad minimum stick deadzone");
		gpad_button_deadzone = Dvar::Register<float>("gpad_button_deadzone", 0.13f, 0.0f, 1.0f, Game::DVAR_ARCHIVE, "Game pad button deadzone threshhold");
		gpad_button_lstick_deflect_max = Dvar::Register<float>("gpad_button_lstick_deflect_max", 1.0f, 0.0f, 1.0f, Game::DVAR_ARCHIVE, "Game pad maximum pad stick pressed value");
		gpad_button_rstick_deflect_max = Dvar::Register<float>("gpad_button_rstick_deflect_max", 1.0f, 0.0f, 1.0f, Game::DVAR_ARCHIVE, "Game pad maximum pad stick pressed value");
		gpad_use_hold_time = Dvar::Register<int>("gpad_use_hold_time", 250, 0, std::numeric_limits<int>::max(), Game::DVAR_ARCHIVE, "Time to hold the 'use' button on gamepads to activate use");
		gpad_lockon_enabled = Dvar::Register<bool>("gpad_lockon_enabled", true, Game::DVAR_ARCHIVE, "Game pad lockon aim assist enabled");
		gpad_slowdown_enabled = Dvar::Register<bool>("gpad_slowdown_enabled", true, Game::DVAR_ARCHIVE, "Game pad slowdown aim assist enabled");

		input_viewSensitivity = Dvar::Register<float>("input_viewSensitivity", 1.0f, 0.0001f, 5.0f, Game::DVAR_ARCHIVE, "View Sensitivity");
		input_invertPitch = Dvar::Register<bool>("input_invertPitch", false, Game::DVAR_ARCHIVE, "Invert gamepad pitch");
		sv_allowAimAssist = Dvar::Register<bool>("sv_allowAimAssist", true, Game::DVAR_NONE, "Controls whether aim assist features on clients are enabled");
		aim_turnrate_pitch = Dvar::Var("aim_turnrate_pitch");
		aim_turnrate_pitch_ads = Dvar::Var("aim_turnrate_pitch_ads");
		aim_turnrate_yaw = Dvar::Var("aim_turnrate_yaw");
		aim_turnrate_yaw_ads = Dvar::Var("aim_turnrate_yaw_ads");
		aim_accel_turnrate_enabled = Dvar::Var("aim_accel_turnrate_enabled");
		aim_accel_turnrate_lerp = Dvar::Var("aim_accel_turnrate_lerp");
		aim_input_graph_enabled = Dvar::Var("aim_input_graph_enabled");
		aim_input_graph_index = Dvar::Var("aim_input_graph_index");
		aim_scale_view_axis = Dvar::Var("aim_scale_view_axis");
		cl_bypassMouseInput = Dvar::Var("cl_bypassMouseInput");
		cg_mapLocationSelectionCursorSpeed = Dvar::Var("cg_mapLocationSelectionCursorSpeed");
		aim_aimAssistRangeScale = Dvar::Var("aim_aimAssistRangeScale");
		aim_slowdown_enabled = Dvar::Var("aim_slowdown_enabled");
		aim_slowdown_debug = Dvar::Var("aim_slowdown_debug");
		aim_slowdown_pitch_scale = Dvar::Var("aim_slowdown_pitch_scale");
		aim_slowdown_pitch_scale_ads = Dvar::Var("aim_slowdown_pitch_scale_ads");
		aim_slowdown_yaw_scale = Dvar::Var("aim_slowdown_yaw_scale");
		aim_slowdown_yaw_scale_ads = Dvar::Var("aim_slowdown_yaw_scale_ads");
		aim_lockon_enabled = Dvar::Var("aim_lockon_enabled");
		aim_lockon_deflection = Dvar::Var("aim_lockon_deflection");
		aim_lockon_pitch_strength = Dvar::Var("aim_lockon_pitch_strength");
		aim_lockon_strength = Dvar::Var("aim_lockon_strength");
	}

	void Gamepad::CG_RegisterDvars_Hk()
	{
		// Call original function
		Utils::Hook::Call<void()>(0x4F8DC0)();

		InitDvars();
	}

	const char* Gamepad::GetGamePadCommand(const char* command)
	{
		if (std::strcmp(command, "+activate") == 0 || std::strcmp(command, "+reload") == 0)
		{
			return "+usereload";
		}

		if (std::strcmp(command, "+melee_breath") == 0)
		{
			return "+holdbreath";
		}

		return command;
	}

	int Gamepad::Key_GetCommandAssignmentInternal([[maybe_unused]] int localClientNum, const char* cmd, int (*keys)[2])
	{
		auto keyCount = 0;

		(*keys)[0] = -1;
		(*keys)[1] = -1;

		if (gamePads[0].inUse)
		{
			const auto gamePadCmd = GetGamePadCommand(cmd);
			for (auto keyNum = 0; keyNum < Game::K_LAST_KEY; keyNum++)
			{
				if (!Key_IsValidGamePadChar(keyNum))
				{
					continue;
				}

				if (Game::playerKeys[0].keys[keyNum].binding && std::strcmp(Game::playerKeys[0].keys[keyNum].binding, gamePadCmd) == 0)
				{
					(*keys)[keyCount++] = keyNum;

					if (keyCount >= 2)
					{
						return keyCount;
					}
				}
			}
		}
		else
		{
			for (auto keyNum = 0; keyNum < Game::K_LAST_KEY; keyNum++)
			{
				if (Key_IsValidGamePadChar(keyNum))
				{
					continue;
				}

				if (Game::playerKeys[0].keys[keyNum].binding && std::strcmp(Game::playerKeys[0].keys[keyNum].binding, cmd) == 0)
				{
					(*keys)[keyCount++] = keyNum;

					if (keyCount >= 2)
					{
						return keyCount;
					}
				}
			}
		}

		return keyCount;
	}

	void __declspec(naked) Gamepad::Key_GetCommandAssignmentInternal_Stub()
	{
		__asm
		{
			push eax
			pushad
			
			push [esp + 0x20 + 0x4 + 0x8] // keyNums
			push [esp + 0x20 + 0x4 + 0x8] // command
			push eax // localClientNum
			call Key_GetCommandAssignmentInternal
			add esp, 0xC

			mov[esp + 0x20], eax

			popad
			pop eax
			ret
		}
	}

	void Gamepad::Key_SetBinding_Hk(const int localClientNum, const int keyNum, const char* binding)
	{
		if (Key_IsValidGamePadChar(keyNum))
		{
			gpad_buttonConfig.set("custom");
		}

		Game::Key_SetBinding(localClientNum, keyNum, binding);
	}

	void Gamepad::CL_KeyEvent_Hk(const int localClientNum, const int key, const int down, const unsigned time)
	{
		// A keyboard key has been pressed. Mark controller as unused.
		gamePads[0].inUse = false;
		gpad_in_use.setRaw(false);

		// Call original function
		Utils::Hook::Call<void(int, int, int, unsigned)>(0x4F6480)(localClientNum, key, down, time);
	}

	bool Gamepad::IsGamePadInUse()
	{
		return gamePads[0].inUse;
	}

	void Gamepad::OnMouseMove([[maybe_unused]] const int x, [[maybe_unused]] const int y, const int dx, const int dy)
	{
		if (dx != 0 || dy != 0)
		{
			gamePads[0].inUse = false;
			gpad_in_use.setRaw(false);
		}
	}

	int Gamepad::CL_MouseEvent_Hk(const int x, const int y, const int dx, const int dy)
	{
		OnMouseMove(x, y, dx, dy);

		// Call original function
		return Utils::Hook::Call<int(int, int, int, int)>(0x4D7C50)(x, y, dx, dy);
	}

	bool Gamepad::UI_RefreshViewport_Hk()
	{
		return cl_bypassMouseInput.get<bool>() || IsGamePadInUse();
	}

	Game::keyname_t* Gamepad::GetLocalizedKeyNameMap()
	{
		if (gpad_style.get<bool>())
		{
			return combinedLocalizedKeyNamesPs3;
		}

		return combinedLocalizedKeyNamesXenon;
	}

	void __declspec(naked) Gamepad::GetLocalizedKeyName_Stub()
	{
		__asm
		{
			push eax
			pushad

			call GetLocalizedKeyNameMap
			mov [esp + 0x20], eax

			popad
			pop eax

			// Re-execute last instruction from game to set flags again for upcoming jump
			test edi, edi
			ret
		}
	}

	void Gamepad::CreateKeyNameMap()
	{
		std::memcpy(combinedKeyNames, Game::keyNames, sizeof(Game::keyname_t) * Game::KEY_NAME_COUNT);
		std::memcpy(&combinedKeyNames[Game::KEY_NAME_COUNT], extendedKeyNames, sizeof(Game::keyname_t) * std::extent_v<decltype(extendedKeyNames)>);
		combinedKeyNames[std::extent_v<decltype(combinedKeyNames)> - 1] = {nullptr, 0};

		std::memcpy(combinedLocalizedKeyNamesXenon, Game::localizedKeyNames, sizeof(Game::keyname_t) * Game::LOCALIZED_KEY_NAME_COUNT);
		std::memcpy(&combinedLocalizedKeyNamesXenon[Game::LOCALIZED_KEY_NAME_COUNT], extendedLocalizedKeyNamesXenon, sizeof(Game::keyname_t) * std::extent_v<decltype(extendedLocalizedKeyNamesXenon)>);

		combinedLocalizedKeyNamesXenon[std::extent_v<decltype(combinedLocalizedKeyNamesXenon)> - 1] = {nullptr, 0};

		std::memcpy(combinedLocalizedKeyNamesPs3, Game::localizedKeyNames, sizeof(Game::keyname_t) * Game::LOCALIZED_KEY_NAME_COUNT);
		std::memcpy(&combinedLocalizedKeyNamesPs3[Game::LOCALIZED_KEY_NAME_COUNT], extendedLocalizedKeyNamesPs3, sizeof(Game::keyname_t) * std::extent_v<decltype(extendedLocalizedKeyNamesPs3)>);

		combinedLocalizedKeyNamesPs3[std::extent_v<decltype(combinedLocalizedKeyNamesPs3)> - 1] = {nullptr, 0};

		Utils::Hook::Set<Game::keyname_t*>(0x4A780A, combinedKeyNames);
		Utils::Hook::Set<Game::keyname_t*>(0x4A7810, combinedKeyNames);
		Utils::Hook::Set<Game::keyname_t*>(0x435C9F, combinedKeyNames);
		Utils::Hook(0x435C97, GetLocalizedKeyName_Stub, HOOK_CALL).install()->quick();
	}

	Gamepad::Gamepad()
	{
		if (ZoneBuilder::IsEnabled())
		{
			return;
		}

		// Initialize gamepad environment
		Utils::Hook(0x4059FE, CG_RegisterDvars_Hk, HOOK_CALL).install()->quick();

		// package the forward and right move components in the move buttons
		Utils::Hook(0x60E38D, MSG_WriteDeltaUsercmdKeyStub, HOOK_JUMP).install()->quick();

		// send two bytes for sending movement data
		Utils::Hook::Set<BYTE>(0x60E501, 16);
		Utils::Hook::Set<BYTE>(0x60E5CD, 16);

		// make sure to parse the movement data properly and apply it
		Utils::Hook(0x492127, MSG_ReadDeltaUsercmdKeyStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x492009, MSG_ReadDeltaUsercmdKeyStub2, HOOK_JUMP).install()->quick();

		// Also rewrite configuration when gamepad config is dirty
		Utils::Hook(0x60B264, Com_WriteConfiguration_Modified_Stub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x60B223, Key_WriteBindings_Hk, HOOK_CALL).install()->quick();

		// Add hold time to gamepad usereload on hold prompts
		Utils::Hook(0x5FE396, Player_UseEntity_Stub, HOOK_JUMP).install()->quick();

		CreateKeyNameMap();

		Command::Add("bindaxis", Axis_Bind_f);
		Command::Add("unbindallaxis", Axis_Unbindall_f);
		Command::Add("bindgpsticksconfigs", Bind_GP_SticksConfigs_f);
		Command::Add("bindgpbuttonsconfigs", Bind_GP_ButtonsConfigs_f);
		Command::Add("togglescores", Scores_Toggle_f);

		if (Dedicated::IsEnabled())
		{
			return;
		}

		// Gamepad on frame hook
		Utils::Hook(0x475E9E, IN_Frame_Hk, HOOK_CALL).install()->quick();

		// Mark controller as unused when keyboard key is pressed
		Utils::Hook(0x43D179, CL_KeyEvent_Hk, HOOK_CALL).install()->quick();

		// Mark controller as unused when mouse is moved
		Utils::Hook(0x64C507, CL_MouseEvent_Hk, HOOK_CALL).install()->quick();

		// Hide cursor when controller is active
		Utils::Hook(0x48E527, UI_RefreshViewport_Hk, HOOK_CALL).install()->quick();

		// Only return gamepad keys when gamepad enabled and only non gamepad keys when not
		Utils::Hook(0x5A7890, Key_GetCommandAssignmentInternal_Stub, HOOK_JUMP).install()->quick();

		// Whenever a key binding for a gamepad key is replaced update the button config
		Utils::Hook(0x47D473, Key_SetBinding_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x47D485, Key_SetBinding_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x47D49D, Key_SetBinding_Hk, HOOK_CALL).install()->quick();

		// Add gamepad inputs to remote control (eg predator) handling
		Utils::Hook(0x5A6D4E, CL_RemoteControlMove_Stub, HOOK_CALL).install()->quick();

		// Add gamepad inputs to location selection (eg airstrike location) handling
		Utils::Hook(0x5A6D72, CG_HandleLocationSelectionInput_Stub, HOOK_CALL).install()->quick();

		// Add gamepad inputs to user commands if it is enabled
		Utils::Hook(0x5A6DAE, CL_MouseMove_Stub, HOOK_CALL).install()->quick();
	}
}
