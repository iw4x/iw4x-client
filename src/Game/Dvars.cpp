#include <STDInclude.hpp>

namespace Game
{
	const dvar_t** com_developer = reinterpret_cast<const dvar_t**>(0x1AD78E8);
	const dvar_t** com_developer_script = reinterpret_cast<const dvar_t**>(0x1AD8F10);
	const dvar_t** com_timescale = reinterpret_cast<const dvar_t**>(0x1AD7920);
	const dvar_t** com_sv_running = reinterpret_cast<const dvar_t**>(0x1AD7934);

	const dvar_t** dev_timescale = reinterpret_cast<const dvar_t**>(0x1AD8F20);

	const dvar_t** dvar_cheats = reinterpret_cast<const dvar_t**>(0x63F3348);

	const dvar_t** fs_gameDirVar = reinterpret_cast<const dvar_t**>(0x63D0CC0);

	const dvar_t** sv_hostname = reinterpret_cast<const dvar_t**>(0x2098D98);
	const dvar_t** sv_gametype = reinterpret_cast<const dvar_t**>(0x2098DD4);
	const dvar_t** sv_mapname = reinterpret_cast<const dvar_t**>(0x2098DDC);
	const dvar_t** sv_mapRotation = reinterpret_cast<const dvar_t**>(0x62C7C44);
	const dvar_t** sv_mapRotationCurrent = reinterpret_cast<const dvar_t**>(0x2098DF0);
	const dvar_t** sv_maxclients = reinterpret_cast<const dvar_t**>(0x2098D90);
	const dvar_t** sv_cheats = reinterpret_cast<const dvar_t**>(0x2098DE0);
	const dvar_t** sv_voiceQuality = reinterpret_cast<const dvar_t**>(0x2098DB0);

	const dvar_t** cl_showSend = reinterpret_cast<const dvar_t**>(0xA1E870);
	const dvar_t** cl_voice = reinterpret_cast<const dvar_t**>(0xB2BB44);

	const dvar_t** g_cheats = reinterpret_cast<const dvar_t**>(0x1A45D54);
	const dvar_t** g_deadChat = reinterpret_cast<const dvar_t**>(0x19BD5DC);
	const dvar_t** g_allowVote = reinterpret_cast<const dvar_t**>(0x19BD644);
	const dvar_t** g_oldVoting = reinterpret_cast<const dvar_t**>(0x1A45DEC);
	const dvar_t** g_gametype = reinterpret_cast<const dvar_t**>(0x1A45DC8);

	const dvar_t** version = reinterpret_cast<const dvar_t**>(0x1AD7930);
}
