#include <STDInclude.hpp>

namespace Game
{
	Dvar_RegisterBool_t Dvar_RegisterBool = Dvar_RegisterBool_t(0x4CE1A0);
	Dvar_RegisterFloat_t Dvar_RegisterFloat = Dvar_RegisterFloat_t(0x648440);
	Dvar_RegisterVec2_t Dvar_RegisterVec2 = Dvar_RegisterVec2_t(0x4F6070);
	Dvar_RegisterVec3_t Dvar_RegisterVec3 = Dvar_RegisterVec3_t(0x4EF8E0);
	Dvar_RegisterVec4_t Dvar_RegisterVec4 = Dvar_RegisterVec4_t(0x471500);
	Dvar_RegisterInt_t Dvar_RegisterInt = Dvar_RegisterInt_t(0x479830);
	Dvar_RegisterEnum_t Dvar_RegisterEnum = Dvar_RegisterEnum_t(0x412E40);
	Dvar_RegisterString_t Dvar_RegisterString = Dvar_RegisterString_t(0x4FC7E0);
	Dvar_RegisterColor_t Dvar_RegisterColor = Dvar_RegisterColor_t(0x4F28E0);
	Dvar_RegisterVec3Color_t Dvar_RegisterVec3Color = Dvar_RegisterVec3Color_t(0x4918B0);

	Dvar_GetUnpackedColorByName_t Dvar_GetUnpackedColorByName = Dvar_GetUnpackedColorByName_t(0x406530);
	Dvar_GetString_t Dvar_GetString = Dvar_GetString_t(0x4EC6B0);
	Dvar_GetVariantString_t Dvar_GetVariantString = Dvar_GetVariantString_t(0x4C47E0);
	Dvar_FindVar_t Dvar_FindVar = Dvar_FindVar_t(0x4D5390);
	Dvar_InfoString_Big_t Dvar_InfoString_Big = Dvar_InfoString_Big_t(0x4D98A0);
	Dvar_SetCommand_t Dvar_SetCommand = Dvar_SetCommand_t(0x4EE430);
	Dvar_DisplayableValue_t Dvar_DisplayableValue = Dvar_DisplayableValue_t(0x4B5530);
	Dvar_Reset_t Dvar_Reset = Dvar_Reset_t(0x4FEFD0);

	Dvar_SetFromStringByName_t Dvar_SetFromStringByName = Dvar_SetFromStringByName_t(0x4F52E0);
	Dvar_SetFromStringByNameFromSource_t Dvar_SetFromStringByNameFromSource = Dvar_SetFromStringByNameFromSource_t(0x4FC770);
	Dvar_SetStringByName_t Dvar_SetStringByName = Dvar_SetStringByName_t(0x44F060);
	Dvar_SetString_t Dvar_SetString = Dvar_SetString_t(0x4A9580);
	Dvar_SetBool_t Dvar_SetBool = Dvar_SetBool_t(0x4A9510);
	Dvar_SetBoolByName_t Dvar_SetBoolByName = Dvar_SetBoolByName_t(0x45C4D0);
	Dvar_SetFloat_t Dvar_SetFloat = Dvar_SetFloat_t(0x40BB20);
	Dvar_SetFloatByName_t Dvar_SetFloatByName = Dvar_SetFloatByName_t(0x466320);
	Dvar_SetInt_t Dvar_SetInt = Dvar_SetInt_t(0x421DA0);

	const dvar_t** com_developer = reinterpret_cast<const dvar_t**>(0x1AD78E8);
	const dvar_t** com_developer_script = reinterpret_cast<const dvar_t**>(0x1AD8F10);
	const dvar_t** com_timescale = reinterpret_cast<const dvar_t**>(0x1AD7920);
	const dvar_t** com_maxFrameTime = reinterpret_cast<const dvar_t**>(0x1AD78F4);
	const dvar_t** com_sv_running = reinterpret_cast<const dvar_t**>(0x1AD7934);
	const dvar_t** com_masterServerName = reinterpret_cast<const dvar_t**>(0x1AD8F48);
	const dvar_t** com_masterPort = reinterpret_cast<const dvar_t**>(0x1AD8F30);

	const dvar_t** dev_timescale = reinterpret_cast<const dvar_t**>(0x1AD8F20);

	const dvar_t** dvar_cheats = reinterpret_cast<const dvar_t**>(0x63F3348);

	const dvar_t** fs_cdpath = reinterpret_cast<const dvar_t**>(0x63D0BB0);
	const dvar_t** fs_basepath = reinterpret_cast<const dvar_t**>(0x63D0CD4);
	const dvar_t** fs_gameDirVar = reinterpret_cast<const dvar_t**>(0x63D0CC0);
	const dvar_t** fs_homepath = reinterpret_cast<const dvar_t**>(0x63D4FD8);

	const dvar_t** sv_privatePassword = reinterpret_cast<const dvar_t**>(0x62C7C14);
	const dvar_t** sv_hostname = reinterpret_cast<const dvar_t**>(0x2098D98);
	const dvar_t** sv_gametype = reinterpret_cast<const dvar_t**>(0x2098DD4);
	const dvar_t** sv_mapname = reinterpret_cast<const dvar_t**>(0x2098DDC);
	const dvar_t** sv_mapRotation = reinterpret_cast<const dvar_t**>(0x62C7C44);
	const dvar_t** sv_mapRotationCurrent = reinterpret_cast<const dvar_t**>(0x2098DF0);
	const dvar_t** sv_maxclients = reinterpret_cast<const dvar_t**>(0x2098D90);
	const dvar_t** sv_cheats = reinterpret_cast<const dvar_t**>(0x2098DE0);
	const dvar_t** sv_voiceQuality = reinterpret_cast<const dvar_t**>(0x2098DB0);

	const dvar_t** nextmap = reinterpret_cast<const dvar_t**>(0x1AD7924);

	const dvar_t** cl_showSend = reinterpret_cast<const dvar_t**>(0xA1E870);
	const dvar_t** cl_voice = reinterpret_cast<const dvar_t**>(0xB2BB44);
	const dvar_t** cl_ingame = reinterpret_cast<const dvar_t**>(0xB2BB80);
	const dvar_t** cl_shownet = reinterpret_cast<const dvar_t**>(0x1059008);

	const dvar_t** g_cheats = reinterpret_cast<const dvar_t**>(0x1A45D54);
	const dvar_t** g_deadChat = reinterpret_cast<const dvar_t**>(0x19BD5DC);
	const dvar_t** g_allowVote = reinterpret_cast<const dvar_t**>(0x19BD644);
	const dvar_t** g_oldVoting = reinterpret_cast<const dvar_t**>(0x1A45DEC);
	const dvar_t** g_gametype = reinterpret_cast<const dvar_t**>(0x1A45DC8);
	const dvar_t** g_password = reinterpret_cast<const dvar_t**>(0x18835C0);
	const dvar_t** g_log = reinterpret_cast<const dvar_t**>(0x1A45D9C);

	const dvar_t** cg_chatHeight = reinterpret_cast<const dvar_t**>(0x7ED398);
	const dvar_t** cg_chatTime = reinterpret_cast<const dvar_t**>(0x9F5DE8);
	const dvar_t** cg_scoreboardHeight = reinterpret_cast<const dvar_t**>(0x9FD070);
	const dvar_t** cg_scoreboardWidth = reinterpret_cast<const dvar_t**>(0x9FD0AC);

	const dvar_t** version = reinterpret_cast<const dvar_t**>(0x1AD7930);

	const dvar_t** viewposNow = reinterpret_cast<const dvar_t**>(0x9FD30C);

	const dvar_t** ui_currentMap = reinterpret_cast<const dvar_t**>(0x62E2834);
	const dvar_t** ui_gametype = reinterpret_cast<const dvar_t**>(0x62E2828);
	const dvar_t** ui_mapname = reinterpret_cast<const dvar_t**>(0x62E279C);
	const dvar_t** ui_joinGametype = reinterpret_cast<const dvar_t**>(0x62E2840);
	const dvar_t** ui_netGameType = reinterpret_cast<const dvar_t**>(0x62E2838);
	const dvar_t** ui_netSource = reinterpret_cast<const dvar_t**>(0x62E27E8);

	const dvar_t** loc_warnings = reinterpret_cast<const dvar_t**>(0x62C8700);
	const dvar_t** loc_warningsAsErrors = reinterpret_cast<const dvar_t**>(0x62C86FC);

	const dvar_t** party_minplayers = reinterpret_cast<const dvar_t**>(0x1081BFC);
	const dvar_t** party_maxplayers = reinterpret_cast<const dvar_t**>(0x1080998);

	const dvar_t** ip = reinterpret_cast<const dvar_t**>(0x64A1DF8);
	const dvar_t** port = reinterpret_cast<const dvar_t**>(0x64A3004);

	__declspec(naked) void Dvar_SetVariant(dvar_t* /*dvar*/, DvarValue /*value*/, DvarSetSource /*source*/)
	{
		static DWORD Dvar_SetVariant_t = 0x647400;

		__asm
		{
			pushad

			mov eax, [esp + 0x4 + 0x20] // dvar
			push [esp + 0x18 + 0x20] // source
			push [esp + 0x18 + 0x20] // value
			push [esp + 0x18 + 0x20] // value
			push [esp + 0x18 + 0x20] // value
			push [esp + 0x18 + 0x20] // value

			call Dvar_SetVariant_t
			add esp, 0x14

			popad

			retn
		}
	}

	void Dvar_SetFromStringFromSource(const dvar_t* dvar, const char* string, DvarSetSource source)
	{
		static DWORD Dvar_SetFromStringFromSource_t = 0x648580;

		__asm
		{
			pushad
			mov esi, dvar
			mov eax, string
			push source
			call Dvar_SetFromStringFromSource_t
			add esp, 0x4
			popad
		}
	}
}
