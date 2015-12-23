namespace Game
{
	typedef enum
	{
		ASSET_TYPE_PHYSPRESET = 0,
		ASSET_TYPE_PHYS_COLLMAP = 1,
		ASSET_TYPE_XANIM = 2,
		ASSET_TYPE_XMODELSURFS = 3,
		ASSET_TYPE_XMODEL = 4,
		ASSET_TYPE_MATERIAL = 5,
		ASSET_TYPE_PIXELSHADER = 6,
		ASSET_TYPE_VERTEXSHADER = 7,
		ASSET_TYPE_VERTEXDECL = 8,
		ASSET_TYPE_TECHSET = 9,
		ASSET_TYPE_IMAGE = 10,
		ASSET_TYPE_SOUND = 11,
		ASSET_TYPE_SNDCURVE = 12,
		ASSET_TYPE_LOADED_SOUND = 13,
		ASSET_TYPE_COL_MAP_SP = 14,
		ASSET_TYPE_COL_MAP_MP = 15,
		ASSET_TYPE_COM_MAP = 16,
		ASSET_TYPE_GAME_MAP_SP = 17,
		ASSET_TYPE_GAME_MAP_MP = 18,
		ASSET_TYPE_MAP_ENTS = 19,
		ASSET_TYPE_FX_MAP = 20,
		ASSET_TYPE_GFX_MAP = 21,
		ASSET_TYPE_LIGHTDEF = 22,
		ASSET_TYPE_UI_MAP = 23,
		ASSET_TYPE_FONT = 24,
		ASSET_TYPE_MENUFILE = 25,
		ASSET_TYPE_MENU = 26,
		ASSET_TYPE_LOCALIZE = 27,
		ASSET_TYPE_WEAPON = 28,
		ASSET_TYPE_SNDDRIVERGLOBALS = 29,
		ASSET_TYPE_FX = 30,
		ASSET_TYPE_IMPACTFX = 31,
		ASSET_TYPE_AITYPE = 32,
		ASSET_TYPE_MPTYPE = 33,
		ASSET_TYPE_CHARACTER = 34,
		ASSET_TYPE_XMODELALIAS = 35,
		ASSET_TYPE_RAWFILE = 36,
		ASSET_TYPE_STRINGTABLE = 37,
		ASSET_TYPE_LEADERBOARDDEF = 38,
		ASSET_TYPE_STRUCTUREDDATADEF = 39,
		ASSET_TYPE_TRACER = 40,
		ASSET_TYPE_VEHICLE = 41,
		ASSET_TYPE_ADDON_MAP_ENTS = 42,
		ASSET_TYPE_MAX = 43
	} XAssetType;

	typedef enum
	{
		DVAR_FLAG_NONE = 0x0,			//no flags
		DVAR_FLAG_SAVED = 0x1,			//saves in config_mp.cfg for clients
		DVAR_FLAG_LATCHED = 0x2,			//no changing apart from initial value (although it might apply on a map reload, I think)
		DVAR_FLAG_CHEAT = 0x4,			//cheat
		DVAR_FLAG_REPLICATED = 0x8,			//on change, this is sent to all clients (if you are host)
		DVAR_FLAG_UNKNOWN10 = 0x10,			//unknown
		DVAR_FLAG_UNKNOWN20 = 0x20,			//unknown
		DVAR_FLAG_UNKNOWN40 = 0x40,			//unknown
		DVAR_FLAG_UNKNOWN80 = 0x80,			//unknown
		DVAR_FLAG_USERCREATED = 0x100,		//a 'set' type command created it
		DVAR_FLAG_USERINFO = 0x200,		//userinfo?
		DVAR_FLAG_SERVERINFO = 0x400,		//in the getstatus oob
		DVAR_FLAG_WRITEPROTECTED = 0x800,		//write protected
		DVAR_FLAG_UNKNOWN1000 = 0x1000,		//unknown
		DVAR_FLAG_READONLY = 0x2000,		//read only (same as 0x800?)
		DVAR_FLAG_UNKNOWN4000 = 0x4000,		//unknown
		DVAR_FLAG_UNKNOWN8000 = 0x8000,		//unknown
		DVAR_FLAG_UNKNOWN10000 = 0x10000,		//unknown
		DVAR_FLAG_DEDISAVED = 0x1000000,		//unknown
		DVAR_FLAG_NONEXISTENT = 0xFFFFFFFF	//no such dvar
	} dvar_flag;

	typedef enum
	{
		DVAR_TYPE_BOOL = 0,
		DVAR_TYPE_FLOAT = 1,
		DVAR_TYPE_FLOAT_2 = 2,
		DVAR_TYPE_FLOAT_3 = 3,
		DVAR_TYPE_FLOAT_4 = 4,
		DVAR_TYPE_INT = 5,
		DVAR_TYPE_ENUM = 6,
		DVAR_TYPE_STRING = 7,
		DVAR_TYPE_COLOR = 8,
		//DVAR_TYPE_INT64	= 9 only in Tx
	} dvar_type;
	// 67/72 bytes figured out
	union dvar_value_t {
		char*	string;
		int		integer;
		float	value;
		bool	boolean;
		float	vec2[2];
		float	vec3[3];
		float	vec4[4];
		BYTE	color[4]; //to get float: multiply by 0.003921568859368563 - BaberZz
		//__int64 integer64; only in Tx
	};
	union dvar_maxmin_t {
		int i;
		float f;
	};
	typedef struct dvar_t
	{
		//startbyte:endbyte
		const char*		name; //0:3
		const char*		description; //4:7
		unsigned int	flags; //8:11
		char			type; //12:12
		char			pad2[3]; //13:15
		dvar_value_t	current; //16:31
		dvar_value_t	latched; //32:47
		dvar_value_t	default; //48:64
		dvar_maxmin_t min; //65:67
		dvar_maxmin_t max; //68:72 woooo
	} dvar_t;

	typedef struct cmd_function_s
	{
		char pad[24];
	} cmd_function_t;
}
