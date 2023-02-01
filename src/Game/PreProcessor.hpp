#pragma once

namespace Game
{
	typedef void(*PC_FreeDefine_t)(define_s* define);
	extern PC_FreeDefine_t PC_FreeDefine;

	typedef define_s*(*PC_FindHashedDefine_t)(define_s** definehash, const char* name);
	extern PC_FindHashedDefine_t PC_FindHashedDefine;

	typedef int(*PC_ReadToken_t)(source_s*, token_s*);
	extern PC_ReadToken_t PC_ReadToken;

	typedef int(*PC_ReadTokenHandle_t)(int handle, pc_token_s* pc_token);
	extern PC_ReadTokenHandle_t PC_ReadTokenHandle;

	typedef int(*PC_ReadSourceToken_t)(source_s* source, token_s* token);
	extern PC_ReadSourceToken_t PC_ReadSourceToken;

	typedef int(*PC_UnreadSourceToken_t)(source_s* source, token_s* token);
	extern PC_UnreadSourceToken_t PC_UnreadSourceToken;

	typedef void(*PC_SourceError_t)(int, const char*, ...);
	extern PC_SourceError_t PC_SourceError;

	typedef void(*SourceError_t)(source_s* source, const char* str, ...);
	extern SourceError_t SourceError;

	typedef int(*PC_Directive_if_def_t)(source_s* source, int type);
	extern PC_Directive_if_def_t PC_Directive_if_def;

	typedef int(*PC_Directive_if_t)(source_s* source);
	extern PC_Directive_if_t PC_Directive_if;

	typedef int(*PC_Directive_ifdef_t)(source_s* source);
	extern PC_Directive_ifdef_t PC_Directive_ifdef;

	typedef int(*PC_Directive_ifndef_t)(source_s* source);
	extern PC_Directive_ifndef_t PC_Directive_ifndef;

	typedef int(*PC_Directive_elif_t)(source_s* source);
	extern PC_Directive_elif_t PC_Directive_elif;

	typedef int(*PC_Directive_else_t)(source_s* source);
	extern PC_Directive_else_t PC_Directive_else;

	typedef int(*PC_Directive_endif_t)(source_s* source);
	extern PC_Directive_endif_t PC_Directive_endif;

	typedef int(*PC_Directive_include_t)(source_s* source);
	extern PC_Directive_include_t PC_Directive_include;

	typedef int(*PC_Directive_define_t)(source_s* source);
	extern PC_Directive_define_t PC_Directive_define;

	typedef int(*PC_Directive_undef_t)(source_s* source);
	extern PC_Directive_undef_t PC_Directive_undef;

	typedef int(*PC_Directive_line_t)(source_s* source);
	extern PC_Directive_line_t PC_Directive_line;

	typedef int(*PC_Directive_error_t)(source_s* source);
	extern PC_Directive_error_t PC_Directive_error;

	typedef int(*PC_Directive_pragma_t)(source_s* source);
	extern PC_Directive_pragma_t PC_Directive_pragma;

	typedef int(*PC_Directive_eval_t)(source_s* source);
	extern PC_Directive_eval_t PC_Directive_eval;

	typedef int(*PC_Directive_evalfloat_t)(source_s* source);
	extern PC_Directive_evalfloat_t PC_Directive_evalfloat;

	extern int PC_ReadLine(source_s* source, token_s* token, bool expandDefines);
	extern void PC_PushIndent(source_s* source, int type, int skip);
	extern void PC_PopIndent(source_s* source, int* type, int* skip);
}
