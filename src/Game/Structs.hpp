#define PROTOCOL 0x92

// This allows us to compile our structures in IDA, for easier reversing :3
#ifdef __cplusplus
namespace Game
{
#endif

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

		ASSET_TYPE_COUNT,
		ASSET_TYPE_INVALID = -1,
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
		dvar_value_t	_default; //48:64
		dvar_maxmin_t min; //65:67
		dvar_maxmin_t max; //68:72 woooo
	} dvar_t;

	typedef struct cmd_function_s
	{
		char pad[24];
	} cmd_function_t;

	typedef struct
	{
		char type;
		char pad[3];
		const char* folder;
		const char* file;
	} StreamFile;

	typedef struct
	{
		char pad[20];
		StreamFile* stream;
		char pad2[76];
	} snd_alias_t;

	typedef struct
	{
		const char* name;
		snd_alias_t* aliases;
		int numAliases;
	} snd_alias_list_t;

	typedef struct
	{
		const char *name;
		int allocFlags;
		int freeFlags;
	} XZoneInfo;



	typedef float vec_t;
	typedef vec_t vec4_t[4];
	struct expression_s;
	struct statement_s;
	struct menuDef_t;
	enum operationEnum;

	struct Material
	{
		const char *name;
	};

	struct keyname_t
	{
		const char *name;
		int keynum;
	};

	struct ItemFloatExpressionEntry
	{
		int target;
		const char *s1;
		const char *s2;
	};

#define ITEM_TYPE_TEXT				0		// simple text
#define ITEM_TYPE_BUTTON			1		// button, basically text with a border
#define ITEM_TYPE_RADIOBUTTON		2		// toggle button, may be grouped
#define ITEM_TYPE_CHECKBOX			3		// check box
#define ITEM_TYPE_EDITFIELD 		4		// editable text, associated with a dvar
#define ITEM_TYPE_COMBO 			5		// drop down list
#define ITEM_TYPE_LISTBOX			6		// scrollable list
#define ITEM_TYPE_MODEL 			7		// model
#define ITEM_TYPE_OWNERDRAW 		8		// owner draw, name specs what it is
#define ITEM_TYPE_NUMERICFIELD		9		// editable text, associated with a dvar
#define ITEM_TYPE_SLIDER			10		// mouse speed, volume, etc.
#define ITEM_TYPE_YESNO 			11		// yes no dvar setting
#define ITEM_TYPE_MULTI 			12		// multiple list setting, enumerated
#define ITEM_TYPE_DVARENUM 			13		// multiple list setting, enumerated from a dvar
#define ITEM_TYPE_BIND				14		// bind
#define ITEM_TYPE_MENUMODEL 		15		// special menu model
#define ITEM_TYPE_VALIDFILEFIELD	16		// text must be valid for use in a dos filename
#define ITEM_TYPE_DECIMALFIELD		17		// editable text, associated with a dvar, which allows decimal input
#define ITEM_TYPE_UPREDITFIELD		18		// editable text, associated with a dvar
#define ITEM_TYPE_GAME_MESSAGE_WINDOW 19	// game message window
#define ITEM_TYPE_NEWSTICKER		20		// horizontal scrollbox
#define ITEM_TYPE_TEXTSCROLL		21		// vertical scrollbox
#define ITEM_TYPE_EMAILFIELD		22
#define ITEM_TYPE_PASSWORDFIELD		23

	struct MenuEventHandlerSet;
	struct Statement_s;

	struct UIFunctionList
	{
		int totalFunctions;
		Statement_s **functions;
	};

	struct StaticDvar
	{
		/*dvar_t*/
		void *dvar;
		char *dvarName;
	};

	struct StaticDvarList
	{
		int numStaticDvars;
		StaticDvar **staticDvars;
	};

	struct StringList
	{
		int totalStrings;
		const char **strings;
	};

	struct ExpressionSupportingData
	{
		UIFunctionList uifunctions;
		StaticDvarList staticDvarList;
		StringList uiStrings;
	};

	enum expDataType : int
	{
		VAL_INT = 0x0,
		VAL_FLOAT = 0x1,
		VAL_STRING = 0x2,
		VAL_FUNCTION = 0x3,
	};

	struct ExpressionString
	{
		const char *string;
	};

	union operandInternalDataUnion
	{
		int intVal;
		float floatVal;
		ExpressionString stringVal;
		Statement_s *function;
	};

	struct Operand
	{
		expDataType dataType;
		operandInternalDataUnion internals;
	};

	union entryInternalData
	{
		//operationEnum op;
		Operand operand;
	};

	/* expressionEntry->type */
#define OPERATOR	0
#define OPERAND		1

	struct expressionEntry	// 0xC
	{
		int type;
		entryInternalData data;
	};

	struct Statement_s	// 0x18
	{
		int numEntries;
		expressionEntry *entries;
		ExpressionSupportingData *supportingData;
		char unknown[0xC];	// ?
	};

	struct SetLocalVarData
	{
		const char *localVarName;
		Statement_s *expression;
	};

	struct ConditionalScript
	{
		MenuEventHandlerSet *eventHandlerSet;
		Statement_s *eventExpression;  // loads this first
	};

	union EventData
	{
		const char *unconditionalScript;
		ConditionalScript *conditionalScript;
		MenuEventHandlerSet *elseScript;
		SetLocalVarData *setLocalVarData;
	};

	enum EventType
	{
		EVENT_UNCONDITIONAL = 0x0,
		EVENT_IF = 0x1,
		EVENT_ELSE = 0x2,
		EVENT_SET_LOCAL_VAR_BOOL = 0x3,
		EVENT_SET_LOCAL_VAR_INT = 0x4,
		EVENT_SET_LOCAL_VAR_FLOAT = 0x5,
		EVENT_SET_LOCAL_VAR_STRING = 0x6,
		EVENT_COUNT = 0x7,
	};

	struct MenuEventHandler
	{
		EventData eventData;
		EventType eventType;
	};

	struct MenuEventHandlerSet
	{
		int eventHandlerCount;
		MenuEventHandler **eventHandlers;
	};

	struct ItemKeyHandler
	{
		int key;
		MenuEventHandlerSet *action;
		ItemKeyHandler *next;
	};

#pragma pack(push, 4)
	struct rectDef_s
	{
		float x;
		float y;
		float w;
		float h;
		char horzAlign;
		char vertAlign;
	};
#pragma pack(pop)

	/* windowDef_t->dynamicFlags */
	// 0x1
#define WINDOWDYNAMIC_HASFOCUS		0x00000002
#define WINDOWDYNAMIC_VISIBLE		0x00000004
#define WINDOWDYNAMIC_FADEOUT		0x00000010
#define WINDOWDYNAMIC_FADEIN		0x00000020
	// 0x40
	// 0x80
#define WINDOWDYNAMIC_CLOSED		0x00000800
	// 0x2000
#define WINDOWDYNAMIC_BACKCOLOR		0x00008000
#define WINDOWDYNAMIC_FORECOLOR		0x00010000

	/* windowDef_t->staticFlags */
#define WINDOWSTATIC_DECORATION				0x00100000
#define WINDOWSTATIC_HORIZONTALSCROLL			0x00200000
#define WINDOWSTATIC_SCREENSPACE				0x00400000
#define WINDOWSTATIC_AUTOWRAPPED				0x00800000
#define WINDOWSTATIC_POPUP						0x01000000
#define WINDOWSTATIC_OUTOFBOUNDSCLICK			0x02000000
#define WINDOWSTATIC_LEGACYSPLITSCREENSCALE	0x04000000
#define WINDOWSTATIC_HIDDENDURINGFLASH			0x10000000
#define WINDOWSTATIC_HIDDENDURINGSCOPE			0x20000000
#define WINDOWSTATIC_HIDDENDURINGUI			0x40000000
#define WINDOWSTATIC_TEXTONLYFOCUS				0x80000000

	struct windowDef_t // 0xA4
	{
		const char *name;	// 0x00
		rectDef_s rect;
		rectDef_s rectClient;
		char *group;		// 0x2C
		int style;			// 0x30
		int border;			// 0x34
		int ownerDraw;		// 0x38
		int ownerDrawFlags;	// 0x3C
		float borderSize;	// 0x40
		int staticFlags;	// 0x44
		int dynamicFlags;	// 0x48
		int nextTime;		// 0x4C
		float foreColor[4];	// 0x50
		float backColor[4];	// 0x60
		float borderColor[4];// 0x70
		float outlineColor[4];// 0x80
		float disableColor[4];// 0x90
		Material *background;	// 0xA0
	};

	enum ItemFloatExpressionTarget
	{
		ITEM_FLOATEXP_TGT_RECT_X = 0x0,
		ITEM_FLOATEXP_TGT_RECT_Y = 0x1,
		ITEM_FLOATEXP_TGT_RECT_W = 0x2,
		ITEM_FLOATEXP_TGT_RECT_H = 0x3,
		ITEM_FLOATEXP_TGT_FORECOLOR_R = 0x4,
		ITEM_FLOATEXP_TGT_FORECOLOR_G = 0x5,
		ITEM_FLOATEXP_TGT_FORECOLOR_B = 0x6,
		ITEM_FLOATEXP_TGT_FORECOLOR_RGB = 0x7,
		ITEM_FLOATEXP_TGT_FORECOLOR_A = 0x8,
		ITEM_FLOATEXP_TGT_GLOWCOLOR_R = 0x9,
		ITEM_FLOATEXP_TGT_GLOWCOLOR_G = 0xA,
		ITEM_FLOATEXP_TGT_GLOWCOLOR_B = 0xB,
		ITEM_FLOATEXP_TGT_GLOWCOLOR_RGB = 0xC,
		ITEM_FLOATEXP_TGT_GLOWCOLOR_A = 0xD,
		ITEM_FLOATEXP_TGT_BACKCOLOR_R = 0xE,
		ITEM_FLOATEXP_TGT_BACKCOLOR_G = 0xF,
		ITEM_FLOATEXP_TGT_BACKCOLOR_B = 0x10,
		ITEM_FLOATEXP_TGT_BACKCOLOR_RGB = 0x11,
		ITEM_FLOATEXP_TGT_BACKCOLOR_A = 0x12,
		ITEM_FLOATEXP_TGT__COUNT = 0x13,
	};

	struct ItemFloatExpression
	{
		ItemFloatExpressionTarget target;
		Statement_s *expression;
	};

	struct editFieldDef_s
	{
		float minVal;
		float maxVal;
		float defVal;
		float range;
		int maxChars;
		int maxCharsGotoNext;
		int maxPaintChars;
		int paintOffset;
	};

	struct multiDef_s	// 0x188
	{
		const char *dvarList[32];
		const char *dvarStr[32];
		float dvarValue[32];
		int count;
		int strDef;
	};

	struct columnInfo_s
	{
		int xpos;
		int width;
		int maxChars;
		int alignment;
	};

	struct listBoxDef_s	// 0x144
	{
		// somethings not right here
		int startPos[2];
		int endPos[2];
		float elementWidth;
		float elementHeight;
		int elementStyle;
		int numColumns;
		columnInfo_s columnInfo[16];
		MenuEventHandlerSet *doubleClick;	// 0xC8
		int notselectable;
		int noscrollbars;
		int usepaging;
		float selectBorder[4];
		Material *selectIcon;
	};

	struct newsTickerDef_s
	{
		int feedId;
		int speed;
		int spacing;
	};

	struct textScrollDef_s
	{
		int startTime;
	};

	union itemDefData_t
	{
		listBoxDef_s *listBox;
		editFieldDef_s *editField;
		newsTickerDef_s *ticker;
		multiDef_s *multiDef;
		const char *enumDvarName;
		textScrollDef_s *scroll;
		void *data;
	};

	struct itemDef_t
	{
		windowDef_t window;
		rectDef_s textRect;
		int type;
		int dataType;
		int alignment;
		int fontEnum;
		int textAlignMode;
		float textAlignX;
		float textAlignY;
		float textScale;
		int textStyle;
		int gameMsgWindowIndex;
		int gameMsgWindowMode;
		const char *text;
		int textSaveGameInfo;
		int parent;
		MenuEventHandlerSet *mouseEnterText;
		MenuEventHandlerSet *mouseExitText;
		MenuEventHandlerSet *mouseEnter;
		MenuEventHandlerSet *mouseExit;
		MenuEventHandlerSet *action;
		MenuEventHandlerSet *accept;
		MenuEventHandlerSet *onFocus;
		MenuEventHandlerSet *leaveFocus;
		const char *dvar;
		const char *dvarTest;
		ItemKeyHandler *onKey;
		const char *enableDvar;
		const char *localVar;
		int dvarFlags;
		const char *focusSound;
		float special;
		int cursorPos;
		itemDefData_t typeData;
		int imageTrack;
		int floatExpressionCount;
		ItemFloatExpression *floatExpressions;
		Statement_s *visibleExp;
		Statement_s *disabledExp;
		Statement_s *textExp;
		Statement_s *materialExp;
		float glowColor[4];
		bool decayActive;
		int fxBirthTime;
		int fxLetterTime;
		int fxDecayStartTime;
		int fxDecayDuration;
		int lastSoundPlayedTime;
	};

	struct menuTransition	// 0x18
	{
		int transitionType;
		int startTime;
		float startVal;
		float endVal;
		float time;
		int endTriggerType;
	};

	struct menuDef_t
	{
		windowDef_t window;
		int font;
		int fullscreen;
		int itemCount;
		int fontIndex;
		int cursorItems;
		int fadeCycle;
		float fadeClamp;
		float fadeAmount;
		float fadeInAmount;
		float blurRadius;
		MenuEventHandlerSet *onOpen;
		MenuEventHandlerSet *onRequestClose;
		MenuEventHandlerSet *onClose;
		MenuEventHandlerSet *onEsc;
		ItemKeyHandler *onKey;
		Statement_s *visibleExp;
		const char *allowedBinding;
		const char *soundLoop;
		int imageTrack;
		float focusColor[4];
		Statement_s *rectXExp;
		Statement_s *rectYExp;
		Statement_s *rectHExp;
		Statement_s *rectWExp;
		Statement_s *openSoundExp;
		Statement_s *closeSoundExp;
		itemDef_t **items;
		char unknown[112];
		ExpressionSupportingData *expressionData;
	};

	struct MenuList
	{
		char *name;
		int menuCount;
		menuDef_t **menus;
	};

	enum FsListBehavior_e
	{
		FS_LIST_PURE_ONLY = 0x0,
		FS_LIST_ALL = 0x1,
	};

	typedef enum 
	{
		NA_BOT,
		NA_BAD,					// an address lookup failed
		NA_LOOPBACK,
		NA_BROADCAST,
		NA_IP,
		NA_IP6, // custom type
	} netadrtype_t;

	typedef enum 
	{
		NS_CLIENT,
		NS_SERVER
	} netsrc_t;

	typedef union
	{
		BYTE bytes[4];
		DWORD full;
	} netIP_t;

	typedef struct 
	{
		netadrtype_t type;
		netIP_t ip;
		unsigned short	port;
		BYTE	ipx[10];
	} netadr_t;

	typedef struct
	{
		int overflowed;
		int readOnly;
		char *data;
		char *splitData;
		int maxsize;
		int cursize;
		int splitSize;
		int readcount;
		int bit;
		int lastEntityRef;
	} msg_t;

	typedef struct gclient_s 
	{
		unsigned char pad[12764];
		unsigned int team;
		char pad2[1164];
	} gclient_t;

	typedef struct gentity_s 
	{
		unsigned char pad[312]; // 0
		float origin[3]; // 312
		float angles[3]; // 324
		char pad2[8];
		gclient_t* client; // 344
		unsigned char pad3[28];
		short classname;
		short pad4;
		unsigned char pad5[248];
	} gentity_t;

#pragma pack(push, 1)
	typedef struct client_s
	{
		// 0
		int state;
		// 4
		char pad[36];
		// 40
		netadr_t adr;
		// 60
		char pad1[1568];
		// 1628
		char connectInfoString[1024];
		// 2652
		char pad2[133192];
		// 135844
		char name[16];
		// 135860
		char pad3[12];
		// 135872
		int snapNum;
		// 135876
		int pad4;
		// 135880
		short ping;
		// 135882
		//char pad5[142390];
		char pad5[133158];
		// 269040
		int isBot;
		// 269044
		char pad6[9228];
		// 278272
		__int64 steamid;
		// 278280
		char pad7[403592];
	} client_t;
#pragma pack(pop)

	// Q3TA precompiler code

	//undef if binary numbers of the form 0b... or 0B... are not allowed
#define BINARYNUMBERS
	//undef if not using the token.intvalue and token.floatvalue
#define NUMBERVALUE
	//use dollar sign also as punctuation
#define DOLLAR

	//maximum token length
#define MAX_TOKEN					1024

	//punctuation
	typedef struct punctuation_s
	{
		char *p;						//punctuation character(s)
		int n;							//punctuation indication
		struct punctuation_s *next;		//next punctuation
	} punctuation_t;

	//token
	typedef struct token_s
	{
		char string[MAX_TOKEN];			//available token
		int type;						//last read token type
		int subtype;					//last read token sub type
#ifdef NUMBERVALUE
		unsigned long int intvalue;	//integer value
		long double floatvalue;			//floating point value
#endif //NUMBERVALUE
		char *whitespace_p;				//start of white space before token
		char *endwhitespace_p;			//start of white space before token
		int line;						//line the token was on
		int linescrossed;				//lines crossed in white space
		struct token_s *next;			//next token in chain
	} token_t;

	//script file
	typedef struct script_s
	{
		char filename[64];				//file name of the script
		char *buffer;					//buffer containing the script
		char *script_p;					//current pointer in the script
		char *end_p;					//pointer to the end of the script
		char *lastscript_p;				//script pointer before reading token
		char *whitespace_p;				//begin of the white space
		char *endwhitespace_p;			//end of the white space
		int length;						//length of the script in bytes
		int line;						//current line in script
		int lastline;					//line before reading token
		int tokenavailable;				//set by UnreadLastToken
		int flags;						//several script flags
		punctuation_t *punctuations;	//the punctuations used in the script
		punctuation_t **punctuationtable;
		token_t token;					//available token
		struct script_s *next;			//next script in a chain
	} script_t;

	//macro definitions
	typedef struct define_s
	{
		char *name;							//define name
		int flags;							//define flags
		int builtin;						// > 0 if builtin define
		int numparms;						//number of define parameters
		token_t *parms;						//define parameters
		token_t *tokens;					//macro tokens (possibly containing parm tokens)
		struct define_s *next;				//next defined macro in a list
		struct define_s *hashnext;			//next define in the hash chain
	} define_t;

	//indents
	//used for conditional compilation directives:
	//#if, #else, #elif, #ifdef, #ifndef
	typedef struct indent_s
	{
		int type;								//indent type
		int skip;								//true if skipping current indent
		script_t *script;						//script the indent was in
		struct indent_s *next;					//next indent on the indent stack
	} indent_t;

	//source file
	typedef struct source_s
	{
		char filename[64];					//file name of the script
		char includepath[64];					//path to include files
		punctuation_t *punctuations;			//punctuations to use
		script_t *scriptstack;					//stack with scripts of the source
		token_t *tokens;						//tokens to read first
		define_t *defines;						//list with macro definitions
		define_t **definehash;					//hash chain with defines
		indent_t *indentstack;					//stack with indents
		int skip;								// > 0 if skipping conditional code
		token_t token;							//last read token
	} source_t;

#define MAX_TOKENLENGTH		1024

	typedef struct pc_token_s
	{
		int type;
		int subtype;
		int intvalue;
		float floatvalue;
		char string[MAX_TOKENLENGTH];
	} pc_token_t;

	//token types
#define TT_STRING						1			// string
#define TT_LITERAL					2			// literal
#define TT_NUMBER						3			// number
#define TT_NAME						4			// name
#define TT_PUNCTUATION				5			// punctuation

	//typedef int menuDef_t;
	//typedef int itemDef_t;

#define KEYWORDHASH_SIZE	512

	typedef struct keywordHash_s
	{
		char *keyword;
		bool(*func)(menuDef_t *item, int handle);
		//struct keywordHash_s *next;
	} keywordHash_t;

	enum UILocalVarType
	{
		UILOCALVAR_INT = 0x0,
		UILOCALVAR_FLOAT = 0x1,
		UILOCALVAR_STRING = 0x2,
	};

	struct UILocalVar
	{
		UILocalVarType type;
		const char *name;
		union
		{
			int integer;
			float value;
			const char *string;
		};
	};

	struct UILocalVarContext
	{
		UILocalVar table[256];
	};

	struct UiContext
	{
// 		int localClientNum;
// 		float bias;
// 		int realTime;
// 		int frameTime;
// 		int cursorx;
// 		int cursory;
// 		int debug;
// 		int screenWidth;
// 		int screenHeight;
// 		float screenAspect;
// 		float FPS;
// 		float blurRadiusOut;
		char pad[56];
		menuDef_t *menus[512];
		char pad2[512];
		int menuCount;
		// Unsure if below is correct
		menuDef_t *menuStack[16];
		int openMenuCount;
		UILocalVarContext localVars;
	};

	struct LocalizedEntry
	{
		const char* value;
		const char* name;
	};

	struct MapEnts
	{
		const char* name;
		const char* entitystring;
	};

	struct StringTableCell
	{
		const char *string;
		int hash;
	};

	struct StringTable
	{
		const char *name;
		int columnCount;
		int rowCount;
		StringTableCell *values;
	};

	struct RawFile
	{
		const char* name;
		int sizeCompressed;
		int sizeUnCompressed;
		char * compressedData;
	};

	typedef struct fontEntry_s
	{
		unsigned short character;
		unsigned char padLeft;
		unsigned char padTop;
		unsigned char padRight;
		unsigned char width;
		unsigned char height;
		unsigned char const0;
		float uvLeft;
		float uvTop;
		float uvRight;
		float uvBottom;
	} fontEntry_t;

	typedef struct Font_s
	{
		char* name;
		int size;
		int entries;
		Material* image;
		Material* glowImage;
		fontEntry_t* characters;
	} Font;

	typedef enum
	{
		STRUCTURED_DATA_INT = 0,
		STRUCTURED_DATA_BYTE = 1,
		STRUCTURED_DATA_BOOL = 2,
		STRUCTURED_DATA_STRING = 3,
		STRUCTURED_DATA_ENUM = 4,
		STRUCTURED_DATA_STRUCT = 5,
		STRUCTURED_DATA_INDEXEDARR = 6,
		STRUCTURED_DATA_ENUMARR = 7,
		STRUCTURED_DATA_FLOAT = 8,
		STRUCTURED_DATA_SHORT = 9
	} structuredDataType_t;

	typedef struct
	{
		structuredDataType_t type;
		union
		{
			int index;
		};
		int offset;
	} structuredDataItem_t;

	typedef struct
	{
		const char* name;
		structuredDataItem_t item;
	} structuredDataChild_t;

	typedef struct
	{
		int numChildren;
		structuredDataChild_t* children;
		int unknown1;
		int unknown2;
	} structuredDataStruct_t;

	typedef struct
	{
		int enumIndex;
		structuredDataItem_t item;
	} structuredDataEnumArray_t;

	typedef struct
	{
		const char* key;
		int index;
	} structuredDataEnumIndex_t;

	typedef struct
	{
		int numIndices;
		int unknown;
		structuredDataEnumIndex_t* indices;
	} structuredDataEnum_t;

	typedef struct
	{
		int numItems;
		structuredDataItem_t item;
	} structuredDataIndexedArray_t;

	typedef struct
	{
		int version;
		unsigned int hash;
		int numEnums;
		structuredDataEnum_t* enums;
		int numStructs;
		structuredDataStruct_t* structs;
		int numIndexedArrays;
		structuredDataIndexedArray_t* indexedArrays;
		int numEnumArrays;
		structuredDataEnumArray_t* enumArrays;
		structuredDataItem_t rootItem;
	} structuredData_t;

	typedef struct
	{
		const char* name;
		int unknown;
		structuredData_t* data;
	} structuredDataDef_t;

	typedef struct
	{
		structuredData_t* data;
		structuredDataItem_t* item;
		int offset;
		int error;
	} structuredDataFindState_t;

	union XAssetHeader
	{
		void *data;
		MenuList *menuList;
		menuDef_t *menu;
		Material *material;
		snd_alias_list_t *aliasList;
		LocalizedEntry *localize;
		StringTable *stringTable;
		MapEnts* mapEnts;
		RawFile* rawfile;
		Font* font;
		structuredDataDef_t* structuredData;
	};

	struct XAsset
	{
		XAssetType type;
		XAssetHeader header;
	};

	struct XBlock
	{
		char *data;
		unsigned int size;
	};

	struct XAssetEntry
	{
		XAsset asset;
		char zoneIndex;
		bool inuse;
		unsigned __int16 nextHash;
		unsigned __int16 nextOverride;
		unsigned __int16 usageFrame;
	};

	enum XFileLanguage : unsigned char
	{
		XLANG_NONE = 0x00,
		XLANG_ENGLISH = 0x01,
		XLANG_FRENCH = 0x02,
		XLANG_GERMAN = 0x03,
		XLANG_ITALIAN = 0x04,
		XLANG_SPANISH = 0x05,
		XLANG_BRITISH = 0x06,
		XLANG_RUSSIAN = 0x07,
		XLANG_POLISH = 0x08,
		XLANG_KOREAN = 0x09,
		XLANG_TAIWANESE = 0x0A,
		XLANG_JAPANESE = 0x0B,
		XLANG_CHINESE = 0x0C,
		XLANG_THAI = 0x0D,
		XLANG_LEET = 0x0E, // Wat?
		XLANG_CZECH = 0x0F,
	};

#pragma pack(push, 1)
	struct  XFileHeader
	{
		unsigned __int64 magic;
		unsigned int version;
		XFileLanguage language;
		DWORD highDateTime;
		DWORD lowDateTime;
	};
#pragma pack(pop)

	enum XFILE_BLOCK_TYPES
	{
		XFILE_BLOCK_TEMP = 0x0,
		XFILE_BLOCK_PHYSICAL = 0x1,
		XFILE_BLOCK_RUNTIME = 0x2,
		XFILE_BLOCK_VIRTUAL = 0x3,
		XFILE_BLOCK_LARGE = 0x4,

		// Those are probably incorrect
		XFILE_BLOCK_CALLBACK,
		XFILE_BLOCK_VERTEX,
		XFILE_BLOCK_INDEX,

		MAX_XFILE_COUNT,

		XFILE_BLOCK_INVALID = -1
	};

	struct XFile
	{
		unsigned int size;
		unsigned int externalSize;
		unsigned int blockSize[MAX_XFILE_COUNT];
	};

	struct ScriptStringList
	{
		int count;
		const char **strings;
	};

	struct XAssetList
	{
		ScriptStringList stringList;
		int assetCount;
		XAsset *assets;
	};

	struct ZoneHeader
	{
		XFile xFile;
		XAssetList assetList;
	};

	struct XNKID
	{
		char ab[8];
	};

	struct XNADDR
	{
		in_addr ina;
		in_addr inaOnline;
		unsigned __int16 wPortOnline;
		char abEnet[6];
		char abOnline[20];
	};

	struct XNKEY
	{
		char ab[16];
	};

	struct _XSESSION_INFO
	{
		XNKID sessionID;
		XNADDR hostAddress;
		XNKEY keyExchangeKey;
	};

	struct mapArena_t
	{
		char uiName[32];
		char mapName[16];
		char pad[2768];
	};

	struct gameTypeName_t
	{
		char gameType[12];
		char uiName[32];
	};

	typedef struct party_s
	{
		BYTE pad1[544];
		int privateSlots;
		int publicSlots;
	} party_t;

	typedef struct PartyData_s
	{
		DWORD unk;
	} PartyData_t;
	
#ifdef __cplusplus
}
#endif