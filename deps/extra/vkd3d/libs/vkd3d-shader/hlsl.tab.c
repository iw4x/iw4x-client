/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Substitute the type names.  */
#define YYSTYPE         HLSL_YYSTYPE
#define YYLTYPE         HLSL_YYLTYPE
/* Substitute the variable and function names.  */
#define yyparse         hlsl_yyparse
#define yylex           hlsl_yylex
#define yyerror         hlsl_yyerror
#define yydebug         hlsl_yydebug
#define yynerrs         hlsl_yynerrs


# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "hlsl.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_KW_BLENDSTATE = 3,              /* KW_BLENDSTATE  */
  YYSYMBOL_KW_BREAK = 4,                   /* KW_BREAK  */
  YYSYMBOL_KW_BUFFER = 5,                  /* KW_BUFFER  */
  YYSYMBOL_KW_BYTEADDRESSBUFFER = 6,       /* KW_BYTEADDRESSBUFFER  */
  YYSYMBOL_KW_CASE = 7,                    /* KW_CASE  */
  YYSYMBOL_KW_CONSTANTBUFFER = 8,          /* KW_CONSTANTBUFFER  */
  YYSYMBOL_KW_CBUFFER = 9,                 /* KW_CBUFFER  */
  YYSYMBOL_KW_CENTROID = 10,               /* KW_CENTROID  */
  YYSYMBOL_KW_COLUMN_MAJOR = 11,           /* KW_COLUMN_MAJOR  */
  YYSYMBOL_KW_COMPILE = 12,                /* KW_COMPILE  */
  YYSYMBOL_KW_COMPILESHADER = 13,          /* KW_COMPILESHADER  */
  YYSYMBOL_KW_COMPUTESHADER = 14,          /* KW_COMPUTESHADER  */
  YYSYMBOL_KW_CONST = 15,                  /* KW_CONST  */
  YYSYMBOL_KW_CONTINUE = 16,               /* KW_CONTINUE  */
  YYSYMBOL_KW_DEFAULT = 17,                /* KW_DEFAULT  */
  YYSYMBOL_KW_DEPTHSTENCILSTATE = 18,      /* KW_DEPTHSTENCILSTATE  */
  YYSYMBOL_KW_DEPTHSTENCILVIEW = 19,       /* KW_DEPTHSTENCILVIEW  */
  YYSYMBOL_KW_DISCARD = 20,                /* KW_DISCARD  */
  YYSYMBOL_KW_DO = 21,                     /* KW_DO  */
  YYSYMBOL_KW_DOMAINSHADER = 22,           /* KW_DOMAINSHADER  */
  YYSYMBOL_KW_ELSE = 23,                   /* KW_ELSE  */
  YYSYMBOL_KW_EXPORT = 24,                 /* KW_EXPORT  */
  YYSYMBOL_KW_EXTERN = 25,                 /* KW_EXTERN  */
  YYSYMBOL_KW_FALSE = 26,                  /* KW_FALSE  */
  YYSYMBOL_KW_FOR = 27,                    /* KW_FOR  */
  YYSYMBOL_KW_FXGROUP = 28,                /* KW_FXGROUP  */
  YYSYMBOL_KW_GEOMETRYSHADER = 29,         /* KW_GEOMETRYSHADER  */
  YYSYMBOL_KW_GROUPSHARED = 30,            /* KW_GROUPSHARED  */
  YYSYMBOL_KW_HULLSHADER = 31,             /* KW_HULLSHADER  */
  YYSYMBOL_KW_IF = 32,                     /* KW_IF  */
  YYSYMBOL_KW_IN = 33,                     /* KW_IN  */
  YYSYMBOL_KW_INLINE = 34,                 /* KW_INLINE  */
  YYSYMBOL_KW_INOUT = 35,                  /* KW_INOUT  */
  YYSYMBOL_KW_INPUTPATCH = 36,             /* KW_INPUTPATCH  */
  YYSYMBOL_KW_LINE = 37,                   /* KW_LINE  */
  YYSYMBOL_KW_LINEADJ = 38,                /* KW_LINEADJ  */
  YYSYMBOL_KW_LINEAR = 39,                 /* KW_LINEAR  */
  YYSYMBOL_KW_LINESTREAM = 40,             /* KW_LINESTREAM  */
  YYSYMBOL_KW_MATRIX = 41,                 /* KW_MATRIX  */
  YYSYMBOL_KW_NAMESPACE = 42,              /* KW_NAMESPACE  */
  YYSYMBOL_KW_NOINTERPOLATION = 43,        /* KW_NOINTERPOLATION  */
  YYSYMBOL_KW_NOPERSPECTIVE = 44,          /* KW_NOPERSPECTIVE  */
  YYSYMBOL_KW_NULL = 45,                   /* KW_NULL  */
  YYSYMBOL_KW_OUT = 46,                    /* KW_OUT  */
  YYSYMBOL_KW_OUTPUTPATCH = 47,            /* KW_OUTPUTPATCH  */
  YYSYMBOL_KW_PACKOFFSET = 48,             /* KW_PACKOFFSET  */
  YYSYMBOL_KW_PASS = 49,                   /* KW_PASS  */
  YYSYMBOL_KW_PIXELSHADER = 50,            /* KW_PIXELSHADER  */
  YYSYMBOL_KW_POINT = 51,                  /* KW_POINT  */
  YYSYMBOL_KW_POINTSTREAM = 52,            /* KW_POINTSTREAM  */
  YYSYMBOL_KW_RASTERIZERORDEREDBUFFER = 53, /* KW_RASTERIZERORDEREDBUFFER  */
  YYSYMBOL_KW_RASTERIZERORDEREDSTRUCTUREDBUFFER = 54, /* KW_RASTERIZERORDEREDSTRUCTUREDBUFFER  */
  YYSYMBOL_KW_RASTERIZERORDEREDTEXTURE1D = 55, /* KW_RASTERIZERORDEREDTEXTURE1D  */
  YYSYMBOL_KW_RASTERIZERORDEREDTEXTURE1DARRAY = 56, /* KW_RASTERIZERORDEREDTEXTURE1DARRAY  */
  YYSYMBOL_KW_RASTERIZERORDEREDTEXTURE2D = 57, /* KW_RASTERIZERORDEREDTEXTURE2D  */
  YYSYMBOL_KW_RASTERIZERORDEREDTEXTURE2DARRAY = 58, /* KW_RASTERIZERORDEREDTEXTURE2DARRAY  */
  YYSYMBOL_KW_RASTERIZERORDEREDTEXTURE3D = 59, /* KW_RASTERIZERORDEREDTEXTURE3D  */
  YYSYMBOL_KW_RASTERIZERSTATE = 60,        /* KW_RASTERIZERSTATE  */
  YYSYMBOL_KW_RENDERTARGETVIEW = 61,       /* KW_RENDERTARGETVIEW  */
  YYSYMBOL_KW_RETURN = 62,                 /* KW_RETURN  */
  YYSYMBOL_KW_REGISTER = 63,               /* KW_REGISTER  */
  YYSYMBOL_KW_ROW_MAJOR = 64,              /* KW_ROW_MAJOR  */
  YYSYMBOL_KW_RWBUFFER = 65,               /* KW_RWBUFFER  */
  YYSYMBOL_KW_RWBYTEADDRESSBUFFER = 66,    /* KW_RWBYTEADDRESSBUFFER  */
  YYSYMBOL_KW_RWSTRUCTUREDBUFFER = 67,     /* KW_RWSTRUCTUREDBUFFER  */
  YYSYMBOL_KW_RWTEXTURE1D = 68,            /* KW_RWTEXTURE1D  */
  YYSYMBOL_KW_RWTEXTURE1DARRAY = 69,       /* KW_RWTEXTURE1DARRAY  */
  YYSYMBOL_KW_RWTEXTURE2D = 70,            /* KW_RWTEXTURE2D  */
  YYSYMBOL_KW_RWTEXTURE2DARRAY = 71,       /* KW_RWTEXTURE2DARRAY  */
  YYSYMBOL_KW_RWTEXTURE3D = 72,            /* KW_RWTEXTURE3D  */
  YYSYMBOL_KW_SAMPLER = 73,                /* KW_SAMPLER  */
  YYSYMBOL_KW_SAMPLER1D = 74,              /* KW_SAMPLER1D  */
  YYSYMBOL_KW_SAMPLER2D = 75,              /* KW_SAMPLER2D  */
  YYSYMBOL_KW_SAMPLER3D = 76,              /* KW_SAMPLER3D  */
  YYSYMBOL_KW_SAMPLERCUBE = 77,            /* KW_SAMPLERCUBE  */
  YYSYMBOL_KW_SAMPLER_STATE = 78,          /* KW_SAMPLER_STATE  */
  YYSYMBOL_KW_SAMPLERCOMPARISONSTATE = 79, /* KW_SAMPLERCOMPARISONSTATE  */
  YYSYMBOL_KW_SHARED = 80,                 /* KW_SHARED  */
  YYSYMBOL_KW_SNORM = 81,                  /* KW_SNORM  */
  YYSYMBOL_KW_STATEBLOCK = 82,             /* KW_STATEBLOCK  */
  YYSYMBOL_KW_STATEBLOCK_STATE = 83,       /* KW_STATEBLOCK_STATE  */
  YYSYMBOL_KW_STATIC = 84,                 /* KW_STATIC  */
  YYSYMBOL_KW_STRING = 85,                 /* KW_STRING  */
  YYSYMBOL_KW_STRUCT = 86,                 /* KW_STRUCT  */
  YYSYMBOL_KW_STRUCTUREDBUFFER = 87,       /* KW_STRUCTUREDBUFFER  */
  YYSYMBOL_KW_SWITCH = 88,                 /* KW_SWITCH  */
  YYSYMBOL_KW_TBUFFER = 89,                /* KW_TBUFFER  */
  YYSYMBOL_KW_TECHNIQUE = 90,              /* KW_TECHNIQUE  */
  YYSYMBOL_KW_TECHNIQUE10 = 91,            /* KW_TECHNIQUE10  */
  YYSYMBOL_KW_TECHNIQUE11 = 92,            /* KW_TECHNIQUE11  */
  YYSYMBOL_KW_TEXTURE = 93,                /* KW_TEXTURE  */
  YYSYMBOL_KW_TEXTURE1D = 94,              /* KW_TEXTURE1D  */
  YYSYMBOL_KW_TEXTURE1DARRAY = 95,         /* KW_TEXTURE1DARRAY  */
  YYSYMBOL_KW_TEXTURE2D = 96,              /* KW_TEXTURE2D  */
  YYSYMBOL_KW_TEXTURE2DARRAY = 97,         /* KW_TEXTURE2DARRAY  */
  YYSYMBOL_KW_TEXTURE2DMS = 98,            /* KW_TEXTURE2DMS  */
  YYSYMBOL_KW_TEXTURE2DMSARRAY = 99,       /* KW_TEXTURE2DMSARRAY  */
  YYSYMBOL_KW_TEXTURE3D = 100,             /* KW_TEXTURE3D  */
  YYSYMBOL_KW_TEXTURECUBE = 101,           /* KW_TEXTURECUBE  */
  YYSYMBOL_KW_TEXTURECUBEARRAY = 102,      /* KW_TEXTURECUBEARRAY  */
  YYSYMBOL_KW_TRIANGLE = 103,              /* KW_TRIANGLE  */
  YYSYMBOL_KW_TRIANGLEADJ = 104,           /* KW_TRIANGLEADJ  */
  YYSYMBOL_KW_TRIANGLESTREAM = 105,        /* KW_TRIANGLESTREAM  */
  YYSYMBOL_KW_TRUE = 106,                  /* KW_TRUE  */
  YYSYMBOL_KW_TYPEDEF = 107,               /* KW_TYPEDEF  */
  YYSYMBOL_KW_UNSIGNED = 108,              /* KW_UNSIGNED  */
  YYSYMBOL_KW_UNIFORM = 109,               /* KW_UNIFORM  */
  YYSYMBOL_KW_UNORM = 110,                 /* KW_UNORM  */
  YYSYMBOL_KW_VECTOR = 111,                /* KW_VECTOR  */
  YYSYMBOL_KW_VERTEXSHADER = 112,          /* KW_VERTEXSHADER  */
  YYSYMBOL_KW_VOID = 113,                  /* KW_VOID  */
  YYSYMBOL_KW_VOLATILE = 114,              /* KW_VOLATILE  */
  YYSYMBOL_KW_WHILE = 115,                 /* KW_WHILE  */
  YYSYMBOL_OP_INC = 116,                   /* OP_INC  */
  YYSYMBOL_OP_DEC = 117,                   /* OP_DEC  */
  YYSYMBOL_OP_AND = 118,                   /* OP_AND  */
  YYSYMBOL_OP_OR = 119,                    /* OP_OR  */
  YYSYMBOL_OP_EQ = 120,                    /* OP_EQ  */
  YYSYMBOL_OP_LEFTSHIFT = 121,             /* OP_LEFTSHIFT  */
  YYSYMBOL_OP_LEFTSHIFTASSIGN = 122,       /* OP_LEFTSHIFTASSIGN  */
  YYSYMBOL_OP_RIGHTSHIFT = 123,            /* OP_RIGHTSHIFT  */
  YYSYMBOL_OP_RIGHTSHIFTASSIGN = 124,      /* OP_RIGHTSHIFTASSIGN  */
  YYSYMBOL_OP_LE = 125,                    /* OP_LE  */
  YYSYMBOL_OP_GE = 126,                    /* OP_GE  */
  YYSYMBOL_OP_NE = 127,                    /* OP_NE  */
  YYSYMBOL_OP_ADDASSIGN = 128,             /* OP_ADDASSIGN  */
  YYSYMBOL_OP_SUBASSIGN = 129,             /* OP_SUBASSIGN  */
  YYSYMBOL_OP_MULASSIGN = 130,             /* OP_MULASSIGN  */
  YYSYMBOL_OP_DIVASSIGN = 131,             /* OP_DIVASSIGN  */
  YYSYMBOL_OP_MODASSIGN = 132,             /* OP_MODASSIGN  */
  YYSYMBOL_OP_ANDASSIGN = 133,             /* OP_ANDASSIGN  */
  YYSYMBOL_OP_ORASSIGN = 134,              /* OP_ORASSIGN  */
  YYSYMBOL_OP_XORASSIGN = 135,             /* OP_XORASSIGN  */
  YYSYMBOL_C_FLOAT = 136,                  /* C_FLOAT  */
  YYSYMBOL_C_INTEGER = 137,                /* C_INTEGER  */
  YYSYMBOL_C_UNSIGNED = 138,               /* C_UNSIGNED  */
  YYSYMBOL_PRE_LINE = 139,                 /* PRE_LINE  */
  YYSYMBOL_VAR_IDENTIFIER = 140,           /* VAR_IDENTIFIER  */
  YYSYMBOL_NEW_IDENTIFIER = 141,           /* NEW_IDENTIFIER  */
  YYSYMBOL_NAMESPACE_IDENTIFIER = 142,     /* NAMESPACE_IDENTIFIER  */
  YYSYMBOL_STRING = 143,                   /* STRING  */
  YYSYMBOL_TYPE_IDENTIFIER = 144,          /* TYPE_IDENTIFIER  */
  YYSYMBOL_145_ = 145,                     /* ';'  */
  YYSYMBOL_146_ = 146,                     /* '{'  */
  YYSYMBOL_147_ = 147,                     /* '}'  */
  YYSYMBOL_148_ = 148,                     /* '<'  */
  YYSYMBOL_149_ = 149,                     /* '>'  */
  YYSYMBOL_150_ = 150,                     /* ':'  */
  YYSYMBOL_151_ = 151,                     /* '['  */
  YYSYMBOL_152_ = 152,                     /* ']'  */
  YYSYMBOL_153_ = 153,                     /* '('  */
  YYSYMBOL_154_ = 154,                     /* ')'  */
  YYSYMBOL_155_ = 155,                     /* ','  */
  YYSYMBOL_156_ = 156,                     /* '.'  */
  YYSYMBOL_157_ = 157,                     /* '='  */
  YYSYMBOL_158_ = 158,                     /* '+'  */
  YYSYMBOL_159_ = 159,                     /* '-'  */
  YYSYMBOL_160_ = 160,                     /* '~'  */
  YYSYMBOL_161_ = 161,                     /* '!'  */
  YYSYMBOL_162_ = 162,                     /* '*'  */
  YYSYMBOL_163_ = 163,                     /* '/'  */
  YYSYMBOL_164_ = 164,                     /* '%'  */
  YYSYMBOL_165_ = 165,                     /* '&'  */
  YYSYMBOL_166_ = 166,                     /* '^'  */
  YYSYMBOL_167_ = 167,                     /* '|'  */
  YYSYMBOL_168_ = 168,                     /* '?'  */
  YYSYMBOL_YYACCEPT = 169,                 /* $accept  */
  YYSYMBOL_hlsl_prog = 170,                /* hlsl_prog  */
  YYSYMBOL_namespace_declaration = 171,    /* namespace_declaration  */
  YYSYMBOL_namespace_prefix = 172,         /* namespace_prefix  */
  YYSYMBOL_name_opt = 173,                 /* name_opt  */
  YYSYMBOL_pass = 174,                     /* pass  */
  YYSYMBOL_annotations_list = 175,         /* annotations_list  */
  YYSYMBOL_annotations_opt = 176,          /* annotations_opt  */
  YYSYMBOL_pass_list = 177,                /* pass_list  */
  YYSYMBOL_passes = 178,                   /* passes  */
  YYSYMBOL_technique9 = 179,               /* technique9  */
  YYSYMBOL_technique10 = 180,              /* technique10  */
  YYSYMBOL_technique11 = 181,              /* technique11  */
  YYSYMBOL_global_technique = 182,         /* global_technique  */
  YYSYMBOL_group_technique = 183,          /* group_technique  */
  YYSYMBOL_group_techniques = 184,         /* group_techniques  */
  YYSYMBOL_effect_group = 185,             /* effect_group  */
  YYSYMBOL_buffer_declaration = 186,       /* buffer_declaration  */
  YYSYMBOL_buffer_body = 187,              /* buffer_body  */
  YYSYMBOL_buffer_type = 188,              /* buffer_type  */
  YYSYMBOL_declaration_statement_list = 189, /* declaration_statement_list  */
  YYSYMBOL_preproc_directive = 190,        /* preproc_directive  */
  YYSYMBOL_struct_declaration_without_vars = 191, /* struct_declaration_without_vars  */
  YYSYMBOL_struct_spec = 192,              /* struct_spec  */
  YYSYMBOL_named_struct_spec = 193,        /* named_struct_spec  */
  YYSYMBOL_unnamed_struct_spec = 194,      /* unnamed_struct_spec  */
  YYSYMBOL_any_identifier = 195,           /* any_identifier  */
  YYSYMBOL_unqualified_identifier = 196,   /* unqualified_identifier  */
  YYSYMBOL_base_optional = 197,            /* base_optional  */
  YYSYMBOL_fields_list = 198,              /* fields_list  */
  YYSYMBOL_field_type = 199,               /* field_type  */
  YYSYMBOL_field = 200,                    /* field  */
  YYSYMBOL_attribute = 201,                /* attribute  */
  YYSYMBOL_attribute_list = 202,           /* attribute_list  */
  YYSYMBOL_attribute_list_optional = 203,  /* attribute_list_optional  */
  YYSYMBOL_func_declaration = 204,         /* func_declaration  */
  YYSYMBOL_func_prototype_no_attrs = 205,  /* func_prototype_no_attrs  */
  YYSYMBOL_func_prototype = 206,           /* func_prototype  */
  YYSYMBOL_compound_statement = 207,       /* compound_statement  */
  YYSYMBOL_scope_start = 208,              /* scope_start  */
  YYSYMBOL_loop_scope_start = 209,         /* loop_scope_start  */
  YYSYMBOL_switch_scope_start = 210,       /* switch_scope_start  */
  YYSYMBOL_annotations_scope_start = 211,  /* annotations_scope_start  */
  YYSYMBOL_var_identifier = 212,           /* var_identifier  */
  YYSYMBOL_unqualified_var_identifier = 213, /* unqualified_var_identifier  */
  YYSYMBOL_var_modifier_identifier = 214,  /* var_modifier_identifier  */
  YYSYMBOL_colon_attributes = 215,         /* colon_attributes  */
  YYSYMBOL_semantic = 216,                 /* semantic  */
  YYSYMBOL_register_reservation = 217,     /* register_reservation  */
  YYSYMBOL_packoffset_reservation = 218,   /* packoffset_reservation  */
  YYSYMBOL_parameters = 219,               /* parameters  */
  YYSYMBOL_param_list = 220,               /* param_list  */
  YYSYMBOL_parameter = 221,                /* parameter  */
  YYSYMBOL_parameter_decl = 222,           /* parameter_decl  */
  YYSYMBOL_texture_type = 223,             /* texture_type  */
  YYSYMBOL_texture_ms_type = 224,          /* texture_ms_type  */
  YYSYMBOL_uav_type = 225,                 /* uav_type  */
  YYSYMBOL_rov_type = 226,                 /* rov_type  */
  YYSYMBOL_so_type = 227,                  /* so_type  */
  YYSYMBOL_resource_format = 228,          /* resource_format  */
  YYSYMBOL_patch_type = 229,               /* patch_type  */
  YYSYMBOL_type_no_void = 230,             /* type_no_void  */
  YYSYMBOL_type = 231,                     /* type  */
  YYSYMBOL_declaration_statement = 232,    /* declaration_statement  */
  YYSYMBOL_typedef_type = 233,             /* typedef_type  */
  YYSYMBOL_typedef = 234,                  /* typedef  */
  YYSYMBOL_type_specs = 235,               /* type_specs  */
  YYSYMBOL_type_spec = 236,                /* type_spec  */
  YYSYMBOL_declaration = 237,              /* declaration  */
  YYSYMBOL_variables_def = 238,            /* variables_def  */
  YYSYMBOL_variables_def_typed = 239,      /* variables_def_typed  */
  YYSYMBOL_variable_decl = 240,            /* variable_decl  */
  YYSYMBOL_state_block_start = 241,        /* state_block_start  */
  YYSYMBOL_stateblock_lhs_identifier = 242, /* stateblock_lhs_identifier  */
  YYSYMBOL_state_block_index_opt = 243,    /* state_block_index_opt  */
  YYSYMBOL_state_block = 244,              /* state_block  */
  YYSYMBOL_state_block_list = 245,         /* state_block_list  */
  YYSYMBOL_variable_def = 246,             /* variable_def  */
  YYSYMBOL_variable_def_typed = 247,       /* variable_def_typed  */
  YYSYMBOL_array = 248,                    /* array  */
  YYSYMBOL_arrays = 249,                   /* arrays  */
  YYSYMBOL_var_modifiers = 250,            /* var_modifiers  */
  YYSYMBOL_complex_initializer = 251,      /* complex_initializer  */
  YYSYMBOL_complex_initializer_list = 252, /* complex_initializer_list  */
  YYSYMBOL_initializer_expr = 253,         /* initializer_expr  */
  YYSYMBOL_initializer_expr_list = 254,    /* initializer_expr_list  */
  YYSYMBOL_boolean = 255,                  /* boolean  */
  YYSYMBOL_statement_list = 256,           /* statement_list  */
  YYSYMBOL_statement = 257,                /* statement  */
  YYSYMBOL_jump_statement = 258,           /* jump_statement  */
  YYSYMBOL_selection_statement = 259,      /* selection_statement  */
  YYSYMBOL_if_body = 260,                  /* if_body  */
  YYSYMBOL_loop_statement = 261,           /* loop_statement  */
  YYSYMBOL_switch_statement = 262,         /* switch_statement  */
  YYSYMBOL_switch_case = 263,              /* switch_case  */
  YYSYMBOL_switch_cases = 264,             /* switch_cases  */
  YYSYMBOL_expr_optional = 265,            /* expr_optional  */
  YYSYMBOL_expr_statement = 266,           /* expr_statement  */
  YYSYMBOL_func_arguments = 267,           /* func_arguments  */
  YYSYMBOL_primary_expr = 268,             /* primary_expr  */
  YYSYMBOL_postfix_expr = 269,             /* postfix_expr  */
  YYSYMBOL_unary_expr = 270,               /* unary_expr  */
  YYSYMBOL_mul_expr = 271,                 /* mul_expr  */
  YYSYMBOL_add_expr = 272,                 /* add_expr  */
  YYSYMBOL_shift_expr = 273,               /* shift_expr  */
  YYSYMBOL_relational_expr = 274,          /* relational_expr  */
  YYSYMBOL_equality_expr = 275,            /* equality_expr  */
  YYSYMBOL_bitand_expr = 276,              /* bitand_expr  */
  YYSYMBOL_bitxor_expr = 277,              /* bitxor_expr  */
  YYSYMBOL_bitor_expr = 278,               /* bitor_expr  */
  YYSYMBOL_logicand_expr = 279,            /* logicand_expr  */
  YYSYMBOL_logicor_expr = 280,             /* logicor_expr  */
  YYSYMBOL_conditional_expr = 281,         /* conditional_expr  */
  YYSYMBOL_assignment_expr = 282,          /* assignment_expr  */
  YYSYMBOL_assign_op = 283,                /* assign_op  */
  YYSYMBOL_expr = 284                      /* expr  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;



/* Unqualified %code blocks.  */


#define YYLLOC_DEFAULT(cur, rhs, n) (cur) = YYRHSLOC(rhs, !!n)

static void yyerror(YYLTYPE *loc, void *scanner, struct hlsl_ctx *ctx, const char *s)
{
    hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX, "%s", s);
}

static struct hlsl_ir_node *node_from_block(struct hlsl_block *block)
{
    return block->value;
}

static struct hlsl_block *make_empty_block(struct hlsl_ctx *ctx)
{
    struct hlsl_block *block;

    if ((block = hlsl_alloc(ctx, sizeof(*block))))
        hlsl_block_init(block);
    return block;
}

static struct list *make_empty_list(struct hlsl_ctx *ctx)
{
    struct list *list;

    if ((list = hlsl_alloc(ctx, sizeof(*list))))
        list_init(list);
    return list;
}

static void destroy_block(struct hlsl_block *block)
{
    if (!block)
        return;

    hlsl_block_cleanup(block);
    vkd3d_free(block);
}

static void destroy_switch_cases(struct list *cases)
{
    hlsl_cleanup_ir_switch_cases(cases);
    vkd3d_free(cases);
}

static bool hlsl_types_are_componentwise_compatible(struct hlsl_ctx *ctx, struct hlsl_type *src,
        struct hlsl_type *dst)
{
    unsigned int k, count = hlsl_type_component_count(dst);

    if (count > hlsl_type_component_count(src))
        return false;

    for (k = 0; k < count; ++k)
    {
        struct hlsl_type *src_comp_type, *dst_comp_type;

        src_comp_type = hlsl_type_get_component_type(ctx, src, k);
        dst_comp_type = hlsl_type_get_component_type(ctx, dst, k);

        if ((src_comp_type->class != HLSL_CLASS_SCALAR || dst_comp_type->class != HLSL_CLASS_SCALAR)
                && !hlsl_types_are_equal(src_comp_type, dst_comp_type))
            return false;
    }
    return true;
}

static bool hlsl_types_are_componentwise_equal(struct hlsl_ctx *ctx, struct hlsl_type *src,
        struct hlsl_type *dst)
{
    unsigned int k, count = hlsl_type_component_count(src);

    if (count != hlsl_type_component_count(dst))
        return false;

    for (k = 0; k < count; ++k)
    {
        struct hlsl_type *src_comp_type, *dst_comp_type;

        src_comp_type = hlsl_type_get_component_type(ctx, src, k);
        dst_comp_type = hlsl_type_get_component_type(ctx, dst, k);

        if (!hlsl_types_are_equal(src_comp_type, dst_comp_type))
            return false;
    }
    return true;
}

static bool type_contains_only_numerics(const struct hlsl_type *type)
{
    unsigned int i;

    if (type->class == HLSL_CLASS_ARRAY)
        return type_contains_only_numerics(type->e.array.type);
    if (type->class == HLSL_CLASS_STRUCT)
    {
        for (i = 0; i < type->e.record.field_count; ++i)
        {
            if (!type_contains_only_numerics(type->e.record.fields[i].type))
                return false;
        }
        return true;
    }
    return hlsl_is_numeric_type(type);
}

static bool explicit_compatible_data_types(struct hlsl_ctx *ctx, struct hlsl_type *src, struct hlsl_type *dst)
{
    if (hlsl_is_numeric_type(src) && src->e.numeric.dimx == 1 && src->e.numeric.dimy == 1
            && type_contains_only_numerics(dst))
        return true;

    if (src->class == HLSL_CLASS_MATRIX && dst->class == HLSL_CLASS_MATRIX
            && src->e.numeric.dimx >= dst->e.numeric.dimx && src->e.numeric.dimy >= dst->e.numeric.dimy)
        return true;

    if ((src->class == HLSL_CLASS_MATRIX && src->e.numeric.dimx > 1 && src->e.numeric.dimy > 1)
            && hlsl_type_component_count(src) != hlsl_type_component_count(dst))
        return false;

    if ((dst->class == HLSL_CLASS_MATRIX && dst->e.numeric.dimy > 1)
            && hlsl_type_component_count(src) != hlsl_type_component_count(dst))
        return false;

    return hlsl_types_are_componentwise_compatible(ctx, src, dst);
}

static bool implicit_compatible_data_types(struct hlsl_ctx *ctx, struct hlsl_type *src, struct hlsl_type *dst)
{
    if (hlsl_is_numeric_type(src) != hlsl_is_numeric_type(dst))
        return false;

    if (hlsl_is_numeric_type(src))
    {
        /* Scalar vars can be converted to any other numeric data type */
        if (src->e.numeric.dimx == 1 && src->e.numeric.dimy == 1)
            return true;
        /* The other way around is true too */
        if (dst->e.numeric.dimx == 1 && dst->e.numeric.dimy == 1)
            return true;

        if (src->class == HLSL_CLASS_MATRIX || dst->class == HLSL_CLASS_MATRIX)
        {
            if (src->class == HLSL_CLASS_MATRIX && dst->class == HLSL_CLASS_MATRIX)
                return src->e.numeric.dimx >= dst->e.numeric.dimx && src->e.numeric.dimy >= dst->e.numeric.dimy;

            /* Matrix-vector conversion is apparently allowed if they have
            * the same components count, or if the matrix is 1xN or Nx1
            * and we are reducing the component count */
            if (src->class == HLSL_CLASS_VECTOR || dst->class == HLSL_CLASS_VECTOR)
            {
                if (hlsl_type_component_count(src) == hlsl_type_component_count(dst))
                    return true;

                if ((src->class == HLSL_CLASS_VECTOR || src->e.numeric.dimx == 1 || src->e.numeric.dimy == 1)
                        && (dst->class == HLSL_CLASS_VECTOR || dst->e.numeric.dimx == 1 || dst->e.numeric.dimy == 1))
                    return hlsl_type_component_count(src) >= hlsl_type_component_count(dst);
            }

            return false;
        }
        else
        {
            return src->e.numeric.dimx >= dst->e.numeric.dimx;
        }
    }

    if (src->class == HLSL_CLASS_NULL)
    {
        switch (dst->class)
        {
            case HLSL_CLASS_DEPTH_STENCIL_STATE:
            case HLSL_CLASS_DEPTH_STENCIL_VIEW:
            case HLSL_CLASS_PIXEL_SHADER:
            case HLSL_CLASS_RASTERIZER_STATE:
            case HLSL_CLASS_RENDER_TARGET_VIEW:
            case HLSL_CLASS_SAMPLER:
            case HLSL_CLASS_STRING:
            case HLSL_CLASS_TEXTURE:
            case HLSL_CLASS_UAV:
            case HLSL_CLASS_VERTEX_SHADER:
                return true;
            default:
                break;
        }
    }

    return hlsl_types_are_componentwise_equal(ctx, src, dst);
}

static void check_condition_type(struct hlsl_ctx *ctx, const struct hlsl_ir_node *cond)
{
    const struct hlsl_type *type = cond->data_type;

    if (type->class == HLSL_CLASS_ERROR)
        return;

    if (type->class > HLSL_CLASS_LAST_NUMERIC || type->e.numeric.dimx > 1 || type->e.numeric.dimy > 1)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, type)))
            hlsl_error(ctx, &cond->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Condition type '%s' is not a scalar numeric type.", string->buffer);
        hlsl_release_string_buffer(ctx, string);
    }
}

static struct hlsl_ir_node *add_cast(struct hlsl_ctx *ctx, struct hlsl_block *block,
        struct hlsl_ir_node *node, struct hlsl_type *dst_type, const struct vkd3d_shader_location *loc)
{
    struct hlsl_type *src_type = node->data_type;

    if (hlsl_types_are_equal(src_type, dst_type))
        return node;

    if (src_type->class == HLSL_CLASS_NULL)
        return node;

    return hlsl_block_add_cast(ctx, block, node, dst_type, loc);
}

static struct hlsl_ir_node *add_implicit_conversion(struct hlsl_ctx *ctx, struct hlsl_block *block,
        struct hlsl_ir_node *node, struct hlsl_type *dst_type, const struct vkd3d_shader_location *loc)
{
    struct hlsl_type *src_type = node->data_type;

    if (hlsl_types_are_equal(src_type, dst_type))
        return node;

    if (node->type == HLSL_IR_SAMPLER_STATE && dst_type->class == HLSL_CLASS_SAMPLER)
        return node;

    if (implicit_compatible_data_types(ctx, src_type, dst_type))
    {
        if (hlsl_is_numeric_type(dst_type) && hlsl_is_numeric_type(src_type)
                && dst_type->e.numeric.dimx * dst_type->e.numeric.dimy < src_type->e.numeric.dimx * src_type->e.numeric.dimy
                && ctx->compile_info.warn_implicit_truncation)
            hlsl_warning(ctx, loc, VKD3D_SHADER_WARNING_HLSL_IMPLICIT_TRUNCATION, "Implicit truncation of %s type.",
                    src_type->class == HLSL_CLASS_VECTOR ? "vector" : "matrix");
    }
    else
    {
        struct vkd3d_string_buffer *src_string, *dst_string;

        src_string = hlsl_type_to_string(ctx, src_type);
        dst_string = hlsl_type_to_string(ctx, dst_type);
        if (src_string && dst_string)
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Can't implicitly convert from %s to %s.", src_string->buffer, dst_string->buffer);
        hlsl_release_string_buffer(ctx, src_string);
        hlsl_release_string_buffer(ctx, dst_string);
    }

    return add_cast(ctx, block, node, dst_type, loc);
}

static void add_explicit_conversion(struct hlsl_ctx *ctx, struct hlsl_block *block,
        struct hlsl_type *dst_type, const struct parse_array_sizes *arrays, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *instr = node_from_block(block);
    struct hlsl_type *src_type = instr->data_type;
    unsigned int i;

    for (i = 0; i < arrays->count; ++i)
    {
        if (arrays->sizes[i] == HLSL_ARRAY_ELEMENTS_COUNT_IMPLICIT)
        {
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE, "Implicit size arrays not allowed in casts.");
            dst_type = ctx->builtin_types.error;
            break;
        }
        dst_type = hlsl_new_array_type(ctx, dst_type, arrays->sizes[i], HLSL_ARRAY_GENERIC);
    }

    if (instr->data_type->class == HLSL_CLASS_ERROR)
        return;

    if (!explicit_compatible_data_types(ctx, src_type, dst_type))
    {
        struct vkd3d_string_buffer *src_string, *dst_string;

        src_string = hlsl_type_to_string(ctx, src_type);
        dst_string = hlsl_type_to_string(ctx, dst_type);
        if (src_string && dst_string)
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE, "Can't cast from %s to %s.",
                    src_string->buffer, dst_string->buffer);
        hlsl_release_string_buffer(ctx, src_string);
        hlsl_release_string_buffer(ctx, dst_string);
    }

    add_cast(ctx, block, instr, dst_type, loc);
}

static uint32_t add_modifiers(struct hlsl_ctx *ctx, uint32_t modifiers, uint32_t mod,
        const struct vkd3d_shader_location *loc)
{
    if (modifiers & mod)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_modifiers_to_string(ctx, mod)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_MODIFIER,
                    "Modifier '%s' was already specified.", string->buffer);
        hlsl_release_string_buffer(ctx, string);
        return modifiers;
    }
    return modifiers | mod;
}

static void append_conditional_break(struct hlsl_ctx *ctx, struct hlsl_block *cond_block)
{
    struct hlsl_ir_node *condition, *cast, *not;
    struct hlsl_block then_block;
    struct hlsl_type *bool_type;

    /* E.g. "for (i = 0; ; ++i)". */
    if (list_empty(&cond_block->instrs))
        return;

    condition = node_from_block(cond_block);

    check_condition_type(ctx, condition);

    bool_type = hlsl_get_scalar_type(ctx, HLSL_TYPE_BOOL);
    /* We already checked for a 1-component numeric type, so
     * add_implicit_conversion() is equivalent to add_cast() here. */
    cast = add_cast(ctx, cond_block, condition, bool_type, &condition->loc);
    not = hlsl_block_add_unary_expr(ctx, cond_block, HLSL_OP1_LOGIC_NOT, cast, &condition->loc);

    hlsl_block_init(&then_block);
    hlsl_block_add_jump(ctx, &then_block, HLSL_IR_JUMP_BREAK, NULL, &condition->loc);
    hlsl_block_add_if(ctx, cond_block, not, &then_block, NULL, HLSL_IF_FLATTEN_DEFAULT, true, &condition->loc);
}

static void check_attribute_list_for_duplicates(struct hlsl_ctx *ctx, const struct parse_attribute_list *attrs)
{
    unsigned int i, j;

    for (i = 0; i < attrs->count; ++i)
    {
        for (j = i + 1; j < attrs->count; ++j)
        {
            if (!strcmp(attrs->attrs[i]->name, attrs->attrs[j]->name))
                hlsl_error(ctx, &attrs->attrs[j]->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX,
                        "Found duplicate attribute \"%s\".", attrs->attrs[j]->name);
        }
    }
}

static void resolve_loop_continue(struct hlsl_ctx *ctx, struct hlsl_block *block,
        enum hlsl_loop_type type, struct hlsl_block *cond)
{
    struct hlsl_ir_node *instr, *next;

    LIST_FOR_EACH_ENTRY_SAFE(instr, next, &block->instrs, struct hlsl_ir_node, entry)
    {
        if (instr->type == HLSL_IR_IF)
        {
            struct hlsl_ir_if *iff = hlsl_ir_if(instr);

            resolve_loop_continue(ctx, &iff->then_block, type, cond);
            resolve_loop_continue(ctx, &iff->else_block, type, cond);
        }
        else if (instr->type == HLSL_IR_JUMP)
        {
            struct hlsl_ir_jump *jump = hlsl_ir_jump(instr);
            struct hlsl_block cond_block;

            if (jump->type != HLSL_IR_JUMP_UNRESOLVED_CONTINUE)
                continue;

            if (type == HLSL_LOOP_DO_WHILE)
            {
                if (!hlsl_clone_block(ctx, &cond_block, cond))
                    return;
                append_conditional_break(ctx, &cond_block);
                list_move_before(&instr->entry, &cond_block.instrs);
            }
        }
    }
}

static void check_loop_attributes(struct hlsl_ctx *ctx, const struct parse_attribute_list *attributes,
        const struct vkd3d_shader_location *loc)
{
    bool has_unroll = false, has_loop = false, has_fastopt = false;
    unsigned int i;

    for (i = 0; i < attributes->count; ++i)
    {
        const char *name = attributes->attrs[i]->name;

        has_loop |= !strcmp(name, "loop");
        has_unroll |= !strcmp(name, "unroll");
        has_fastopt |= !strcmp(name, "fastopt");
    }

    if (has_unroll && has_loop)
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX, "Unroll attribute can't be used with 'loop' attribute.");

    if (has_unroll && has_fastopt)
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX, "Unroll attribute can't be used with 'fastopt' attribute.");
}

static bool is_compile_time_const(const struct hlsl_block *block, bool default_values)
{
    struct hlsl_ir_node *node;
    struct hlsl_ir_var *var;

    LIST_FOR_EACH_ENTRY(node, &block->instrs, struct hlsl_ir_node, entry)
    {
        switch (node->type)
        {
            case HLSL_IR_COMPILE:
            case HLSL_IR_CONSTANT:
            case HLSL_IR_EXPR:
            case HLSL_IR_SAMPLER_STATE:
            case HLSL_IR_STRING_CONSTANT:
            case HLSL_IR_SWIZZLE:
            case HLSL_IR_INDEX:
                break;
            case HLSL_IR_LOAD:
                var = hlsl_ir_load(node)->src.var;
                if (var->is_synthetic || var->is_compile_time_const || (var->default_values && default_values))
                    break;
                return false;
            case HLSL_IR_STORE:
                var = hlsl_ir_store(node)->lhs.var;
                if (var->is_synthetic)
                    break;
                return false;
            case HLSL_IR_CALL:
            case HLSL_IR_IF:
            case HLSL_IR_INTERLOCKED:
            case HLSL_IR_LOOP:
            case HLSL_IR_JUMP:
            case HLSL_IR_RESOURCE_LOAD:
            case HLSL_IR_RESOURCE_STORE:
            case HLSL_IR_SWITCH:
            case HLSL_IR_STATEBLOCK_CONSTANT:
            case HLSL_IR_SYNC:
                return false;
        }
    }

    return true;
}

static struct hlsl_default_value evaluate_static_expression(struct hlsl_ctx *ctx,
        struct hlsl_block *block, struct hlsl_type *dst_type, bool default_values,
        const struct vkd3d_shader_location *loc)
{
    struct hlsl_default_value ret = {0};
    struct hlsl_ir_node *node;
    struct hlsl_block expr;
    struct hlsl_src src;

    if (node_from_block(block)->data_type->class == HLSL_CLASS_ERROR)
        return ret;

    if (!is_compile_time_const(block, default_values))
        hlsl_error(ctx, &node_from_block(block)->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX,
                "Expected literal expression.");

    if (!hlsl_clone_block(ctx, &expr, &ctx->static_initializers))
        return ret;
    hlsl_block_add_block(&expr, block);

    node = add_implicit_conversion(ctx, &expr, node_from_block(&expr), dst_type, loc);

    /* Wrap the node into a src to allow the reference to survive the multiple const passes. */
    hlsl_src_from_node(&src, node);
    hlsl_lower_index_loads(ctx, &expr);
    hlsl_run_const_passes(ctx, &expr);
    node = src.node;
    hlsl_src_remove(&src);

    if (node->type == HLSL_IR_CONSTANT)
    {
        struct hlsl_ir_constant *constant = hlsl_ir_constant(node);

        ret.number = constant->value.u[0];
    }
    else if (node->type == HLSL_IR_STRING_CONSTANT)
    {
        struct hlsl_ir_string_constant *string = hlsl_ir_string_constant(node);

        if (!(ret.string = vkd3d_strdup(string->string)))
            return ret;
    }
    else if (node->type == HLSL_IR_COMPILE)
    {
        list_remove(&node->entry);
        ret.shader = hlsl_ir_compile(node);
    }
    else
    {
        hlsl_error(ctx, &node->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX,
                "Failed to evaluate constant expression.");
    }

    hlsl_block_cleanup(&expr);

    return ret;
}

static unsigned int evaluate_static_expression_as_uint(struct hlsl_ctx *ctx, struct hlsl_block *block,
        const struct vkd3d_shader_location *loc)
{
    struct hlsl_default_value res;

    res = evaluate_static_expression(ctx, block, hlsl_get_scalar_type(ctx, HLSL_TYPE_UINT), false, loc);
    VKD3D_ASSERT(!res.string);
    return res.number.u;
}

static struct hlsl_block *create_loop(struct hlsl_ctx *ctx, enum hlsl_loop_type type,
        const struct parse_attribute_list *attributes, struct hlsl_block *init, struct hlsl_block *cond,
        struct hlsl_block *iter, struct hlsl_block *body, const struct vkd3d_shader_location *loc)
{
    enum hlsl_loop_unroll_type unroll_type = HLSL_LOOP_UNROLL;
    struct hlsl_ir_node *unroll_limit = NULL;
    unsigned int i;

    check_attribute_list_for_duplicates(ctx, attributes);
    check_loop_attributes(ctx, attributes, loc);

    for (i = 0; i < attributes->count; ++i)
    {
        const struct hlsl_attribute *attr = attributes->attrs[i];
        if (!strcmp(attr->name, "unroll"))
        {
            if (attr->args_count > 1)
            {
                hlsl_warning(ctx, &attr->loc, VKD3D_SHADER_WARNING_HLSL_IGNORED_ATTRIBUTE,
                        "Ignoring 'unroll' attribute with more than 1 argument.");
                continue;
            }

            if (attr->args_count == 1)
            {
                struct hlsl_block expr;

                if (hlsl_block_has_side_effects(&attr->instrs))
                {
                    hlsl_error(ctx, &attr->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX,
                            "Unroll limit expressions cannot have side effects.");
                    continue;
                }

                if (!init && !(init = make_empty_block(ctx)))
                    return NULL;

                hlsl_block_init(&expr);
                if (!hlsl_clone_block(ctx, &expr, &attr->instrs))
                    return NULL;

                list_move_head(&init->instrs, &expr.instrs);
                unroll_limit = add_implicit_conversion(ctx, init, node_from_block(&expr),
                        hlsl_get_scalar_type(ctx, HLSL_TYPE_UINT), loc);
            }

            unroll_type = HLSL_LOOP_FORCE_UNROLL;
        }
        else if (!strcmp(attr->name, "loop"))
        {
            unroll_type = HLSL_LOOP_FORCE_LOOP;
        }
        else if (!strcmp(attr->name, "fastopt")
                || !strcmp(attr->name, "allow_uav_condition"))
        {
            hlsl_fixme(ctx, loc, "Unhandled attribute '%s'.", attr->name);
        }
        else
        {
            hlsl_warning(ctx, loc, VKD3D_SHADER_WARNING_HLSL_UNKNOWN_ATTRIBUTE, "Unrecognized attribute '%s'.", attr->name);
        }
    }

    resolve_loop_continue(ctx, body, type, cond);

    if (!init && !(init = make_empty_block(ctx)))
        goto oom;

    append_conditional_break(ctx, cond);

    if (type == HLSL_LOOP_DO_WHILE)
        list_move_tail(&body->instrs, &cond->instrs);
    else
        list_move_head(&body->instrs, &cond->instrs);

    hlsl_block_add_loop(ctx, init, iter, body, unroll_type, unroll_limit, loc);

    destroy_block(cond);
    destroy_block(body);
    destroy_block(iter);
    return init;

oom:
    destroy_block(init);
    destroy_block(cond);
    destroy_block(iter);
    destroy_block(body);
    return NULL;
}

static unsigned int initializer_size(const struct parse_initializer *initializer)
{
    unsigned int count = 0, i;

    for (i = 0; i < initializer->args_count; ++i)
    {
        count += hlsl_type_component_count(initializer->args[i]->data_type);
    }
    return count;
}

static void cleanup_parse_attribute_list(struct parse_attribute_list *attr_list)
{
    unsigned int i = 0;

    VKD3D_ASSERT(attr_list);
    for (i = 0; i < attr_list->count; ++i)
        hlsl_free_attribute((struct hlsl_attribute *) attr_list->attrs[i]);
    vkd3d_free(attr_list->attrs);
}

static void parse_colon_attributes_cleanup(struct parse_colon_attributes *a)
{
    hlsl_cleanup_semantic(&a->semantic);
}

static void parse_function_cleanup(struct parse_function *f)
{
    hlsl_func_parameters_cleanup(&f->parameters);
    hlsl_cleanup_semantic(&f->return_semantic);
}

static void cleanup_parse_initializer(struct parse_initializer *initializer)
{
    destroy_block(initializer->instrs);
    vkd3d_free(initializer->args);
}

static struct hlsl_ir_node *get_swizzle(struct hlsl_ctx *ctx, struct hlsl_ir_node *value, const char *swizzle,
        struct vkd3d_shader_location *loc)
{
    unsigned int len = strlen(swizzle), component = 0;
    unsigned int i, set, swiz = 0;
    bool valid;

    if (value->data_type->class == HLSL_CLASS_MATRIX)
    {
        /* Matrix swizzle */
        struct hlsl_matrix_swizzle s;
        bool m_swizzle;
        unsigned int inc, x, y;

        if (len < 3 || swizzle[0] != '_')
            return NULL;
        m_swizzle = swizzle[1] == 'm';
        inc = m_swizzle ? 4 : 3;

        if (len % inc || len > inc * 4)
            return NULL;

        for (i = 0; i < len; i += inc)
        {
            if (swizzle[i] != '_')
                return NULL;
            if (m_swizzle)
            {
                if (swizzle[i + 1] != 'm')
                    return NULL;
                y = swizzle[i + 2] - '0';
                x = swizzle[i + 3] - '0';
            }
            else
            {
                y = swizzle[i + 1] - '1';
                x = swizzle[i + 2] - '1';
            }

            if (x >= value->data_type->e.numeric.dimx || y >= value->data_type->e.numeric.dimy)
                return NULL;
            s.components[component].x = x;
            s.components[component].y = y;
            component++;
        }
        return hlsl_new_matrix_swizzle(ctx, s, component, value, loc);
    }

    /* Vector swizzle */
    if (len > 4)
        return NULL;

    for (set = 0; set < 2; ++set)
    {
        valid = true;
        component = 0;
        for (i = 0; i < len; ++i)
        {
            char c[2][4] = {{'x', 'y', 'z', 'w'}, {'r', 'g', 'b', 'a'}};
            unsigned int s = 0;

            for (s = 0; s < 4; ++s)
            {
                if (swizzle[i] == c[set][s])
                    break;
            }
            if (s == 4)
            {
                valid = false;
                break;
            }

            if (s >= value->data_type->e.numeric.dimx)
                return NULL;
            hlsl_swizzle_set_component(&swiz, component++, s);
        }
        if (valid)
            return hlsl_new_swizzle(ctx, swiz, component, value, loc);
    }

    return NULL;
}

static bool add_return(struct hlsl_ctx *ctx, struct hlsl_block *block,
        struct hlsl_ir_node *return_value, const struct vkd3d_shader_location *loc)
{
    struct hlsl_type *return_type = ctx->cur_function->return_type;

    if (ctx->cur_function->return_var)
    {
        if (return_value)
        {
            if (return_value->data_type->class == HLSL_CLASS_ERROR)
                return true;

            return_value = add_implicit_conversion(ctx, block, return_value, return_type, loc);
            hlsl_block_add_simple_store(ctx, block, ctx->cur_function->return_var, return_value);
        }
        else
        {
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_RETURN, "Non-void functions must return a value.");
            return false;
        }
    }
    else
    {
        if (return_value)
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_RETURN, "Void functions cannot return a value.");
    }

    hlsl_block_add_jump(ctx, block, HLSL_IR_JUMP_RETURN, NULL, loc);
    return true;
}

struct hlsl_ir_node *hlsl_add_load_component(struct hlsl_ctx *ctx, struct hlsl_block *block,
        struct hlsl_ir_node *var_instr, unsigned int comp, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_var *var;
    struct hlsl_deref src;

    if (!(var = hlsl_new_synthetic_var(ctx, "deref", var_instr->data_type, &var_instr->loc)))
    {
        block->value = ctx->error_instr;
        return ctx->error_instr;
    }

    hlsl_block_add_simple_store(ctx, block, var, var_instr);

    hlsl_deref_init_simple(&src, var);
    return hlsl_block_add_load_component(ctx, block, &src, comp, loc);
}

static void add_record_access(struct hlsl_ctx *ctx, struct hlsl_block *block, struct hlsl_ir_node *record,
        unsigned int idx, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *c;

    VKD3D_ASSERT(idx < record->data_type->e.record.field_count);

    c = hlsl_block_add_uint_constant(ctx, block, idx, loc);
    hlsl_block_add_index(ctx, block, record, c, loc);
}

static struct hlsl_ir_node *add_binary_arithmetic_expr(struct hlsl_ctx *ctx, struct hlsl_block *block,
        enum hlsl_ir_expr_op op, struct hlsl_ir_node *arg1, struct hlsl_ir_node *arg2,
        const struct vkd3d_shader_location *loc);

static bool add_array_access(struct hlsl_ctx *ctx, struct hlsl_block *block, struct hlsl_ir_node *array,
        struct hlsl_ir_node *index, const struct vkd3d_shader_location *loc)
{
    const struct hlsl_type *expr_type = array->data_type, *index_type = index->data_type;

    if (array->data_type->class == HLSL_CLASS_ERROR || index->data_type->class == HLSL_CLASS_ERROR)
    {
        block->value = ctx->error_instr;
        return true;
    }

    if (expr_type->class == HLSL_CLASS_MIPS && expr_type->e.mips.mips_type == HLSL_MIPS_SLICE)
    {
        unsigned int dim_count = hlsl_sampler_dim_count(expr_type->e.mips.type->sampler_dim);
        struct hlsl_resource_load_params params = {.type = HLSL_RESOURCE_LOAD};
        struct hlsl_ir_node *mips_property, *mip, *coords;
        struct vkd3d_string_buffer *string;

        if (dim_count == 1 && index_type->class != HLSL_CLASS_SCALAR)
        {
            if ((string = hlsl_type_to_string(ctx, expr_type)))
                hlsl_error(ctx, &index->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                        "Texture mip index of type '%s' must be a scalar.", string->buffer);
            hlsl_release_string_buffer(ctx, string);
            return false;
        }
        if (dim_count > 1 && (index_type->class != HLSL_CLASS_VECTOR || index_type->e.numeric.dimx != dim_count))
        {
            if ((string = hlsl_type_to_string(ctx, expr_type)))
                hlsl_error(ctx, &index->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                        "Texture mip index of type '%s' must be a vec%u.", string->buffer, dim_count);
            hlsl_release_string_buffer(ctx, string);
            return false;
        }

        mips_property = array->type == HLSL_IR_INDEX ? hlsl_ir_index(array)->val.node : NULL;
        if (!mips_property || mips_property->type != HLSL_IR_EXPR || hlsl_ir_expr(mips_property)->op != HLSL_OP1_MIPS)
        {
            hlsl_error(ctx, &array->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX,
                    ".mips can only be used in a two-element indexing expression .mips[mip][pos].");
            return false;
        }

        mip = hlsl_ir_index(array)->idx.node;

        coords = add_implicit_conversion(ctx, block, index,
                hlsl_get_vector_type(ctx, HLSL_TYPE_UINT, dim_count), &index->loc);
        coords = hlsl_block_add_mipmap_level(ctx, block, coords, mip, loc);

        params.resource = hlsl_ir_expr(mips_property)->operands[0].node;
        params.coords = coords;
        params.format = expr_type->e.mips.type->e.resource.format;
        hlsl_block_add_resource_load(ctx, block, &params, loc);

        /* Remove the consumed mips slice, so that validate_mips_access() won't flag it as invalid. */
        if (list_empty(&array->uses))
        {
            list_remove(&array->entry);
            hlsl_free_instr(array);
        }
        return true;
    }

    if ((expr_type->class == HLSL_CLASS_TEXTURE || expr_type->class == HLSL_CLASS_UAV)
            && expr_type->sampler_dim != HLSL_SAMPLER_DIM_GENERIC)
    {
        unsigned int dim_count = hlsl_sampler_dim_count(expr_type->sampler_dim);

        if (index_type->class > HLSL_CLASS_VECTOR || index_type->e.numeric.dimx != dim_count)
        {
            struct vkd3d_string_buffer *string;

            if ((string = hlsl_type_to_string(ctx, expr_type)))
                hlsl_error(ctx, &index->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                        "Array index of type '%s' must be of type 'uint%u'.", string->buffer, dim_count);
            hlsl_release_string_buffer(ctx, string);
            return false;
        }

        index = add_implicit_conversion(ctx, block, index,
                hlsl_get_vector_type(ctx, HLSL_TYPE_UINT, dim_count), &index->loc);
        hlsl_block_add_index(ctx, block, array, index, loc);
        return true;
    }

    if (index_type->class != HLSL_CLASS_SCALAR)
    {
        hlsl_error(ctx, &index->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE, "Array index is not scalar.");
        return false;
    }

    index = hlsl_block_add_cast(ctx, block, index, hlsl_get_scalar_type(ctx, HLSL_TYPE_UINT), &index->loc);

    if (expr_type->class != HLSL_CLASS_ARRAY && expr_type->class != HLSL_CLASS_VECTOR
            && expr_type->class != HLSL_CLASS_MATRIX && expr_type->class != HLSL_CLASS_MIPS)
    {
        if (expr_type->class == HLSL_CLASS_SCALAR)
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_INDEX, "Scalar expressions cannot be array-indexed.");
        else
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_INDEX, "Expression cannot be array-indexed.");
        return false;
    }

    hlsl_block_add_index(ctx, block, array, index, loc);
    return true;
}

static const struct hlsl_struct_field *get_struct_field(const struct hlsl_struct_field *fields,
        size_t count, const char *name)
{
    size_t i;

    for (i = 0; i < count; ++i)
    {
        if (!strcmp(fields[i].name, name))
            return &fields[i];
    }
    return NULL;
}

static struct hlsl_type *apply_type_modifiers(struct hlsl_ctx *ctx, struct hlsl_type *type,
        uint32_t *modifiers, bool force_majority, const struct vkd3d_shader_location *loc)
{
    unsigned int default_majority = 0;
    struct hlsl_type *new_type;

    if (!(*modifiers & HLSL_MODIFIERS_MAJORITY_MASK)
            && !(type->modifiers & HLSL_MODIFIERS_MAJORITY_MASK)
            && type->class == HLSL_CLASS_MATRIX)
    {
        if (!(default_majority = ctx->matrix_majority) && force_majority)
            default_majority = HLSL_MODIFIER_COLUMN_MAJOR;
    }
    else if (type->class != HLSL_CLASS_MATRIX && (*modifiers & HLSL_MODIFIERS_MAJORITY_MASK))
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_MODIFIER,
                "'row_major' and 'column_major' modifiers are only allowed for matrices.");
    }

    if (!default_majority && !(*modifiers & HLSL_TYPE_MODIFIERS_MASK))
        return type;

    if (!(new_type = hlsl_type_clone(ctx, type, default_majority, *modifiers & HLSL_TYPE_MODIFIERS_MASK)))
        return NULL;

    *modifiers &= ~HLSL_TYPE_MODIFIERS_MASK;

    if ((new_type->modifiers & HLSL_MODIFIER_ROW_MAJOR) && (new_type->modifiers & HLSL_MODIFIER_COLUMN_MAJOR))
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_MODIFIER,
                "'row_major' and 'column_major' modifiers are mutually exclusive.");

    return new_type;
}

static void free_parse_variable_def(struct parse_variable_def *v)
{
    cleanup_parse_initializer(&v->initializer);
    vkd3d_free(v->arrays.sizes);
    vkd3d_free(v->name);
    hlsl_cleanup_semantic(&v->semantic);
    if (v->state_block_count)
    {
        for (unsigned int i = 0; i < v->state_block_count; ++i)
            hlsl_free_state_block(v->state_blocks[i]);
        vkd3d_free(v->state_blocks);
    }
    vkd3d_free(v);
}

static void destroy_parse_variable_defs(struct list *defs)
{
    struct parse_variable_def *v, *v_next;

    LIST_FOR_EACH_ENTRY_SAFE(v, v_next, defs, struct parse_variable_def, entry)
        free_parse_variable_def(v);
    vkd3d_free(defs);
}

static bool gen_struct_fields(struct hlsl_ctx *ctx, struct parse_fields *fields,
        struct hlsl_type *type, uint32_t modifiers, struct list *defs)
{
    struct parse_variable_def *v, *v_next;
    size_t i = 0;

    if (type->class == HLSL_CLASS_MATRIX)
        VKD3D_ASSERT(type->modifiers & HLSL_MODIFIERS_MAJORITY_MASK);

    memset(fields, 0, sizeof(*fields));
    fields->count = list_count(defs);
    if (!hlsl_array_reserve(ctx, (void **)&fields->fields, &fields->capacity, fields->count, sizeof(*fields->fields)))
        return false;

    LIST_FOR_EACH_ENTRY_SAFE(v, v_next, defs, struct parse_variable_def, entry)
    {
        struct hlsl_struct_field *field = &fields->fields[i++];
        bool unbounded_res_array = false;
        unsigned int k;

        field->type = type;

        if (hlsl_version_ge(ctx, 5, 1) && hlsl_type_is_resource(type))
        {
            for (k = 0; k < v->arrays.count; ++k)
                unbounded_res_array |= (v->arrays.sizes[k] == HLSL_ARRAY_ELEMENTS_COUNT_IMPLICIT);
        }

        if (unbounded_res_array)
        {
            if (v->arrays.count == 1)
            {
                hlsl_fixme(ctx, &v->loc, "Unbounded resource arrays as struct fields.");
                free_parse_variable_def(v);
                vkd3d_free(field);
                continue;
            }
            else
            {
                hlsl_error(ctx, &v->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                        "Unbounded resource arrays cannot be multi-dimensional.");
            }
        }
        else
        {
            for (k = 0; k < v->arrays.count; ++k)
            {
                if (v->arrays.sizes[k] == HLSL_ARRAY_ELEMENTS_COUNT_IMPLICIT)
                {
                    hlsl_error(ctx, &v->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                            "Implicit size arrays not allowed in struct fields.");
                    field->type = ctx->builtin_types.error;
                    break;
                }

                field->type = hlsl_new_array_type(ctx, field->type, v->arrays.sizes[k], HLSL_ARRAY_GENERIC);
            }
        }

        if (hlsl_version_ge(ctx, 5, 1) && field->type->class == HLSL_CLASS_ARRAY && hlsl_type_is_resource(field->type))
            hlsl_fixme(ctx, &v->loc, "Shader model 5.1+ resource array.");

        vkd3d_free(v->arrays.sizes);
        field->loc = v->loc;
        field->name = v->name;
        field->semantic = v->semantic;
        field->storage_modifiers = modifiers | v->semantic.modifiers;
        if (v->initializer.args_count)
        {
            hlsl_error(ctx, &v->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX, "Illegal initializer on a struct field.");
            cleanup_parse_initializer(&v->initializer);
        }
        if (v->reg_reservation.offset_type)
            hlsl_error(ctx, &v->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                "packoffset() is not allowed inside struct definitions.");
        vkd3d_free(v);
    }
    vkd3d_free(defs);
    return true;
}

static void add_record_access_recurse(struct hlsl_ctx *ctx, struct hlsl_block *block,
        const char *name, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *record = node_from_block(block);
    const struct hlsl_type *type = record->data_type;
    const struct hlsl_struct_field *field, *base;

    if (type->class == HLSL_CLASS_ERROR)
        return;

    if ((field = get_struct_field(type->e.record.fields, type->e.record.field_count, name)))
    {
        unsigned int field_idx = field - type->e.record.fields;

        add_record_access(ctx, block, record, field_idx, loc);
    }
    else if ((base = get_struct_field(type->e.record.fields, type->e.record.field_count, "$super")))
    {
        unsigned int base_idx = base - type->e.record.fields;

        add_record_access(ctx, block, record, base_idx, loc);
        add_record_access_recurse(ctx, block, name, loc);
    }
    else
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_NOT_DEFINED, "Field \"%s\" is not defined.", name);
        block->value = ctx->error_instr;
    }
}

static bool add_typedef(struct hlsl_ctx *ctx, struct hlsl_type *const orig_type, struct list *list)
{
    struct parse_variable_def *v, *v_next;
    struct hlsl_scope *namespace;
    struct hlsl_type *type;
    unsigned int i;
    bool ret;

    LIST_FOR_EACH_ENTRY_SAFE(v, v_next, list, struct parse_variable_def, entry)
    {
        if (!v->arrays.count)
        {
            if (!(type = hlsl_type_clone(ctx, orig_type, 0, 0)))
            {
                free_parse_variable_def(v);
                continue;
            }
        }
        else
        {
            uint32_t var_modifiers = 0;

            if (!(type = apply_type_modifiers(ctx, orig_type, &var_modifiers, true, &v->loc)))
            {
                free_parse_variable_def(v);
                continue;
            }
        }

        ret = true;
        for (i = 0; i < v->arrays.count; ++i)
        {
            if (v->arrays.sizes[i] == HLSL_ARRAY_ELEMENTS_COUNT_IMPLICIT)
            {
                hlsl_error(ctx, &v->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                        "Implicit size arrays not allowed in typedefs.");
                if (!(type = hlsl_type_clone(ctx, ctx->builtin_types.error, 0, 0)))
                {
                    free_parse_variable_def(v);
                    ret = false;
                }
                break;
            }

            if (!(type = hlsl_new_array_type(ctx, type, v->arrays.sizes[i], HLSL_ARRAY_GENERIC)))
            {
                free_parse_variable_def(v);
                ret = false;
                break;
            }
        }
        if (!ret)
            continue;
        vkd3d_free(v->arrays.sizes);

        vkd3d_free((void *)type->name);
        type->name = v->name;
        type->is_typedef = true;

        if ((namespace = hlsl_get_namespace(ctx->cur_scope, type->name)))
        {
            hlsl_error(ctx, &v->loc, VKD3D_SHADER_ERROR_HLSL_REDEFINED,
                    "Type '%s' is already declared as a namespace.", v->name);
            hlsl_note(ctx, &namespace->namespace_loc, VKD3D_SHADER_LOG_ERROR,
                    "'%s' was previously declared here.", v->name);
        }
        else if (!hlsl_scope_add_type(ctx->cur_scope, type))
        {
            hlsl_error(ctx, &v->loc, VKD3D_SHADER_ERROR_HLSL_REDEFINED,
                    "Type '%s' is already defined.", v->name);
        }
        cleanup_parse_initializer(&v->initializer);
        vkd3d_free(v);
    }
    vkd3d_free(list);
    return true;
}

static void check_invalid_stream_output_object(struct hlsl_ctx *ctx, const struct hlsl_type *type,
        const char *name, const struct vkd3d_shader_location* loc)
{
    if (hlsl_type_component_count(type) != 1)
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                "Stream output object '%s' is not single-element.", name);
}

static void initialize_var_components(struct hlsl_ctx *ctx, struct hlsl_block *instrs,
        struct hlsl_ir_var *dst, unsigned int *store_index, struct hlsl_ir_node *src,
        bool is_default_values_initializer);

static bool add_func_parameter(struct hlsl_ctx *ctx, struct hlsl_func_parameters *parameters,
        struct parse_parameter *param, const struct vkd3d_shader_location *loc)
{
    uint32_t modifiers = param->modifiers | param->semantic.modifiers;
    struct hlsl_ir_var *var;

    if (param->type->class == HLSL_CLASS_MATRIX)
        VKD3D_ASSERT(param->type->modifiers & HLSL_MODIFIERS_MAJORITY_MASK);

    if ((modifiers & HLSL_STORAGE_OUT) && (modifiers & HLSL_STORAGE_UNIFORM))
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_MODIFIER,
                "Parameter '%s' is declared as both \"out\" and \"uniform\".", param->name);

    if ((modifiers & HLSL_STORAGE_OUT) && !(modifiers & HLSL_STORAGE_IN)
            && (param->type->modifiers & HLSL_MODIFIER_CONST))
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_MODIFIER,
                "Parameter '%s' is declared as both \"out\" and \"const\".", param->name);

    if (param->reg_reservation.offset_type)
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                "packoffset() is not allowed on function parameters.");

    if (parameters->count && parameters->vars[parameters->count - 1]->default_values
                && !param->initializer.args_count)
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_MISSING_INITIALIZER,
                "Missing default value for parameter '%s'.", param->name);

    if (param->initializer.args_count && (modifiers & HLSL_STORAGE_OUT))
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_MODIFIER,
                "Output parameter '%s' has a default value.", param->name);

    if (hlsl_get_stream_output_type(param->type))
        check_invalid_stream_output_object(ctx, param->type, param->name, loc);

    if (!(var = hlsl_new_var(ctx, param->name, param->type, loc, &param->semantic, modifiers,
            &param->reg_reservation)))
        return false;
    var->is_param = 1;

    if (param->initializer.args_count)
    {
        unsigned int component_count = hlsl_type_component_count(param->type);
        unsigned int store_index = 0;
        unsigned int size, i;

        if (!(var->default_values = hlsl_calloc(ctx, component_count, sizeof(*var->default_values))))
            return false;

        if (!param->initializer.braces)
        {
            add_implicit_conversion(ctx, param->initializer.instrs, param->initializer.args[0], param->type, loc);
            param->initializer.args[0] = node_from_block(param->initializer.instrs);
        }

        size = initializer_size(&param->initializer);
        if (component_count != size)
        {
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                    "Expected %u components in initializer, but got %u.", component_count, size);
        }

        for (i = 0; i < param->initializer.args_count; ++i)
        {
            initialize_var_components(ctx, param->initializer.instrs, var,
                    &store_index, param->initializer.args[i], true);
        }

        cleanup_parse_initializer(&param->initializer);
    }

    hlsl_add_var(ctx, var);

    if (!hlsl_array_reserve(ctx, (void **)&parameters->vars, &parameters->capacity,
            parameters->count + 1, sizeof(*parameters->vars)))
        return false;
    parameters->vars[parameters->count++] = var;
    return true;
}

static void add_pass(struct hlsl_ctx *ctx, const char *name, struct hlsl_scope *annotations,
        struct hlsl_state_block *state_block, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_var *var;
    struct hlsl_type *type;

    type = hlsl_get_type(ctx->globals, "pass", false, false);
    if (!(var = hlsl_new_var(ctx, name, type, loc, NULL, 0, NULL)))
        return;
    var->annotations = annotations;

    var->state_blocks = hlsl_alloc(ctx, sizeof(*var->state_blocks));
    var->state_blocks[0] = state_block;
    var->state_block_count = 1;
    var->state_block_capacity = 1;

    hlsl_add_var(ctx, var);
}

static void add_technique(struct hlsl_ctx *ctx, const char *name, struct hlsl_scope *scope,
        struct hlsl_scope *annotations, const char *typename, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_var *var;
    struct hlsl_type *type;

    if (ctx->cur_scope->namespace)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX,
                "Techniques are not allowed inside namespaces.");
        return;
    }

    type = hlsl_get_type(ctx->globals, typename, false, false);
    if (!(var = hlsl_new_var(ctx, name, type, loc, NULL, 0, NULL)))
        return;
    var->scope = scope;
    var->annotations = annotations;

    hlsl_add_var(ctx, var);
}

static void add_effect_group(struct hlsl_ctx *ctx, const char *name, struct hlsl_scope *scope,
        struct hlsl_scope *annotations, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_var *var;
    struct hlsl_type *type;

    if (ctx->cur_scope->namespace)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX,
                "Effect groups are not allowed inside namespaces.");
        return;
    }

    type = hlsl_get_type(ctx->globals, "fxgroup", false, false);
    if (!(var = hlsl_new_var(ctx, name, type, loc, NULL, 0, NULL)))
        return;
    var->scope = scope;
    var->annotations = annotations;

    hlsl_add_var(ctx, var);
}

static bool parse_reservation_index(struct hlsl_ctx *ctx, const char *string, unsigned int bracket_offset,
        struct hlsl_reg_reservation *reservation)
{
    char *endptr;

    reservation->reg_type = ascii_tolower(string[0]);

    /* Prior to SM5.1, fxc simply ignored bracket offsets for 'b' types. */
    if (reservation->reg_type == 'b' && hlsl_version_lt(ctx, 5, 1))
    {
        bracket_offset = 0;
    }

    if (string[1] == '\0')
    {
        reservation->reg_index = bracket_offset;
        return true;
    }

    reservation->reg_index = strtoul(string + 1, &endptr, 10) + bracket_offset;

    if (*endptr)
    {
        /* fxc for SM >= 4 treats all parse failures for 'b' types as successes,
         * setting index to -1. It will later fail while validating slot limits. */
        if (reservation->reg_type == 'b' && hlsl_version_ge(ctx, 4, 0))
        {
            reservation->reg_index = -1;
            return true;
        }

        /* All other types tolerate leftover characters. */
        if (endptr == string + 1)
            return false;
    }

    return true;
}

static bool parse_reservation_space(const char *string, uint32_t *space)
{
    return !ascii_strncasecmp(string, "space", 5) && sscanf(string + 5, "%u", space);
}

static struct hlsl_reg_reservation parse_packoffset(struct hlsl_ctx *ctx, const char *reg_string,
        const char *swizzle, const struct vkd3d_shader_location *loc)
{
    struct hlsl_reg_reservation reservation = {0};
    char *endptr;

    if (hlsl_version_lt(ctx, 4, 0))
        return reservation;

    reservation.offset_index = strtoul(reg_string + 1, &endptr, 10);
    if (*endptr)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                "Invalid packoffset() syntax.");
        return reservation;
    }

    reservation.offset_type = ascii_tolower(reg_string[0]);
    if (reservation.offset_type != 'c')
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                "Only 'c' registers are allowed in packoffset().");
        return reservation;
    }

    reservation.offset_index *= 4;

    if (swizzle)
    {
        if (strlen(swizzle) != 1)
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                    "Invalid packoffset() component \"%s\".", swizzle);

        if (swizzle[0] == 'x' || swizzle[0] == 'r')
            reservation.offset_index += 0;
        else if (swizzle[0] == 'y' || swizzle[0] == 'g')
            reservation.offset_index += 1;
        else if (swizzle[0] == 'z' || swizzle[0] == 'b')
            reservation.offset_index += 2;
        else if (swizzle[0] == 'w' || swizzle[0] == 'a')
            reservation.offset_index += 3;
        else
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                    "Invalid packoffset() component \"%s\".", swizzle);
    }

    return reservation;
}

static struct hlsl_block *make_block(struct hlsl_ctx *ctx, struct hlsl_ir_node *instr)
{
    struct hlsl_block *block;

    if (!(block = make_empty_block(ctx)))
    {
        hlsl_free_instr(instr);
        return NULL;
    }
    hlsl_block_add_instr(block, instr);
    return block;
}

static bool expr_compatible_data_types(struct hlsl_type *t1, struct hlsl_type *t2)
{
    /* Scalar vars can be converted to pretty much everything */
    if ((t1->e.numeric.dimx == 1 && t1->e.numeric.dimy == 1) || (t2->e.numeric.dimx == 1 && t2->e.numeric.dimy == 1))
        return true;

    if (t1->class == HLSL_CLASS_VECTOR && t2->class == HLSL_CLASS_VECTOR)
        return true;

    if (t1->class == HLSL_CLASS_MATRIX || t2->class == HLSL_CLASS_MATRIX)
    {
        /* Matrix-vector conversion is apparently allowed if either they have the same components
           count or the matrix is nx1 or 1xn */
        if (t1->class == HLSL_CLASS_VECTOR || t2->class == HLSL_CLASS_VECTOR)
        {
            if (hlsl_type_component_count(t1) == hlsl_type_component_count(t2))
                return true;

            return (t1->class == HLSL_CLASS_MATRIX && (t1->e.numeric.dimx == 1 || t1->e.numeric.dimy == 1))
                    || (t2->class == HLSL_CLASS_MATRIX && (t2->e.numeric.dimx == 1 || t2->e.numeric.dimy == 1));
        }

        /* Both matrices */
        if ((t1->e.numeric.dimx >= t2->e.numeric.dimx && t1->e.numeric.dimy >= t2->e.numeric.dimy)
                || (t1->e.numeric.dimx <= t2->e.numeric.dimx && t1->e.numeric.dimy <= t2->e.numeric.dimy))
            return true;
    }

    return false;
}

static enum hlsl_base_type expr_common_base_type(enum hlsl_base_type t1, enum hlsl_base_type t2)
{
    if (t1 == t2)
        return t1 == HLSL_TYPE_BOOL ? HLSL_TYPE_INT : t1;
    if (t1 == HLSL_TYPE_DOUBLE || t2 == HLSL_TYPE_DOUBLE)
        return HLSL_TYPE_DOUBLE;
    if (t1 == HLSL_TYPE_FLOAT || t2 == HLSL_TYPE_FLOAT
            || t1 == HLSL_TYPE_HALF || t2 == HLSL_TYPE_HALF)
        return HLSL_TYPE_FLOAT;
    if (t1 == HLSL_TYPE_UINT || t2 == HLSL_TYPE_UINT)
        return HLSL_TYPE_UINT;
    if (t1 == HLSL_TYPE_INT || t2 == HLSL_TYPE_INT)
        return HLSL_TYPE_INT;
    if (t1 == HLSL_TYPE_MIN16UINT || t2 == HLSL_TYPE_MIN16UINT)
        return HLSL_TYPE_MIN16UINT;
    vkd3d_unreachable();
}

static bool expr_common_shape(struct hlsl_ctx *ctx, struct hlsl_type *t1, struct hlsl_type *t2,
        const struct vkd3d_shader_location *loc, enum hlsl_type_class *type, unsigned int *dimx, unsigned int *dimy)
{
    if (!hlsl_is_numeric_type(t1))
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, t1)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Expression of type \"%s\" cannot be used in a numeric expression.", string->buffer);
        hlsl_release_string_buffer(ctx, string);
        return false;
    }

    if (!hlsl_is_numeric_type(t2))
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, t2)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Expression of type \"%s\" cannot be used in a numeric expression.", string->buffer);
        hlsl_release_string_buffer(ctx, string);
        return false;
    }

    if (!expr_compatible_data_types(t1, t2))
    {
        struct vkd3d_string_buffer *t1_string = hlsl_type_to_string(ctx, t1);
        struct vkd3d_string_buffer *t2_string = hlsl_type_to_string(ctx, t2);

        if (t1_string && t2_string)
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Expression data types \"%s\" and \"%s\" are incompatible.",
                    t1_string->buffer, t2_string->buffer);
        hlsl_release_string_buffer(ctx, t1_string);
        hlsl_release_string_buffer(ctx, t2_string);
        return false;
    }

    if (t1->e.numeric.dimx == 1 && t1->e.numeric.dimy == 1)
    {
        *type = t2->class;
        *dimx = t2->e.numeric.dimx;
        *dimy = t2->e.numeric.dimy;
    }
    else if (t2->e.numeric.dimx == 1 && t2->e.numeric.dimy == 1)
    {
        *type = t1->class;
        *dimx = t1->e.numeric.dimx;
        *dimy = t1->e.numeric.dimy;
    }
    else if (t1->class == HLSL_CLASS_MATRIX && t2->class == HLSL_CLASS_MATRIX)
    {
        *type = HLSL_CLASS_MATRIX;
        *dimx = min(t1->e.numeric.dimx, t2->e.numeric.dimx);
        *dimy = min(t1->e.numeric.dimy, t2->e.numeric.dimy);
    }
    else
    {
        if (t1->e.numeric.dimx * t1->e.numeric.dimy <= t2->e.numeric.dimx * t2->e.numeric.dimy)
        {
            *type = t1->class;
            *dimx = t1->e.numeric.dimx;
            *dimy = t1->e.numeric.dimy;
        }
        else
        {
            *type = t2->class;
            *dimx = t2->e.numeric.dimx;
            *dimy = t2->e.numeric.dimy;
        }
    }

    return true;
}

static struct hlsl_ir_node *add_expr(struct hlsl_ctx *ctx, struct hlsl_block *block,
        enum hlsl_ir_expr_op op, struct hlsl_ir_node *operands[HLSL_MAX_OPERANDS],
        struct hlsl_type *type, const struct vkd3d_shader_location *loc)
{
    unsigned int i;

    if (type->class == HLSL_CLASS_MATRIX)
    {
        struct hlsl_type *scalar_type;
        struct hlsl_deref var_deref;
        struct hlsl_ir_var *var;

        scalar_type = hlsl_get_scalar_type(ctx, type->e.numeric.type);

        if (!(var = hlsl_new_synthetic_var(ctx, "split_op", type, loc)))
            return NULL;
        hlsl_deref_init_simple(&var_deref, var);

        for (i = 0; i < type->e.numeric.dimy * type->e.numeric.dimx; ++i)
        {
            struct hlsl_ir_node *value, *cell_operands[HLSL_MAX_OPERANDS] = { NULL };
            unsigned int j;

            for (j = 0; j < HLSL_MAX_OPERANDS; j++)
            {
                if (operands[j])
                    cell_operands[j] = hlsl_add_load_component(ctx, block, operands[j], i, loc);
            }

            if (!(value = add_expr(ctx, block, op, cell_operands, scalar_type, loc)))
                return NULL;

            hlsl_block_add_store_component(ctx, block, &var_deref, i, value);
        }

        return hlsl_block_add_simple_load(ctx, block, var, loc);
    }

    return hlsl_block_add_expr(ctx, block, op, operands, type, loc);
}

static void check_integer_type(struct hlsl_ctx *ctx, const struct hlsl_ir_node *instr)
{
    const struct hlsl_type *type = instr->data_type;
    struct vkd3d_string_buffer *string;

    if (hlsl_type_is_integer(type))
        return;

    if ((string = hlsl_type_to_string(ctx, type)))
        hlsl_error(ctx, &instr->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                "Expression type '%s' is not integer.", string->buffer);
    hlsl_release_string_buffer(ctx, string);
}

static struct hlsl_ir_node *add_unary_arithmetic_expr(struct hlsl_ctx *ctx, struct hlsl_block *block,
        enum hlsl_ir_expr_op op, struct hlsl_ir_node *arg, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *args[HLSL_MAX_OPERANDS] = {arg};

    if (arg->data_type->class == HLSL_CLASS_ERROR)
        return arg;

    if (!hlsl_is_numeric_type(arg->data_type))
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                "Unary arithmetic expression operand must be numeric.");
        block->value = ctx->error_instr;
        return block->value;
    }

    return add_expr(ctx, block, op, args, arg->data_type, loc);
}

static struct hlsl_ir_node *add_unary_bitwise_expr(struct hlsl_ctx *ctx, struct hlsl_block *block,
        enum hlsl_ir_expr_op op, struct hlsl_ir_node *arg, const struct vkd3d_shader_location *loc)
{
    if (arg->data_type->class == HLSL_CLASS_ERROR)
        return arg;

    check_integer_type(ctx, arg);

    return add_unary_arithmetic_expr(ctx, block, op, arg, loc);
}

static struct hlsl_ir_node *add_unary_logical_expr(struct hlsl_ctx *ctx, struct hlsl_block *block,
        enum hlsl_ir_expr_op op, struct hlsl_ir_node *arg, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *args[HLSL_MAX_OPERANDS] = {0};
    struct hlsl_type *bool_type;

    if (arg->data_type->class == HLSL_CLASS_ERROR)
        return arg;

    if (!hlsl_is_numeric_type(arg->data_type))
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                "Unary logical expression operand must be numeric.");
        block->value = ctx->error_instr;
        return block->value;
    }

    bool_type = hlsl_change_base_type(ctx, arg->data_type, HLSL_TYPE_BOOL);
    args[0] = add_implicit_conversion(ctx, block, arg, bool_type, loc);
    return add_expr(ctx, block, op, args, bool_type, loc);
}

static struct hlsl_type *get_common_numeric_type(struct hlsl_ctx *ctx, const struct hlsl_ir_node *arg1,
        const struct hlsl_ir_node *arg2, const struct vkd3d_shader_location *loc)
{
    enum hlsl_type_class type;
    enum hlsl_base_type base;
    unsigned int dimx, dimy;

    if (!expr_common_shape(ctx, arg1->data_type, arg2->data_type, loc, &type, &dimx, &dimy))
        return NULL;
    base = expr_common_base_type(arg1->data_type->e.numeric.type, arg2->data_type->e.numeric.type);
    return hlsl_get_numeric_type(ctx, type, base, dimx, dimy);
}

static struct hlsl_ir_node *add_binary_arithmetic_expr(struct hlsl_ctx *ctx, struct hlsl_block *block,
        enum hlsl_ir_expr_op op, struct hlsl_ir_node *arg1, struct hlsl_ir_node *arg2,
        const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *args[HLSL_MAX_OPERANDS] = {0};
    struct hlsl_type *common_type;

    if (!(common_type = get_common_numeric_type(ctx, arg1, arg2, loc)))
    {
        block->value = ctx->error_instr;
        return block->value;
    }

    args[0] = add_implicit_conversion(ctx, block, arg1, common_type, loc);
    args[1] = add_implicit_conversion(ctx, block, arg2, common_type, loc);
    return add_expr(ctx, block, op, args, common_type, loc);
}

static struct hlsl_ir_node *add_binary_bitwise_expr(struct hlsl_ctx *ctx, struct hlsl_block *block,
        enum hlsl_ir_expr_op op, struct hlsl_ir_node *arg1, struct hlsl_ir_node *arg2,
        const struct vkd3d_shader_location *loc)
{
    check_integer_type(ctx, arg1);
    check_integer_type(ctx, arg2);

    return add_binary_arithmetic_expr(ctx, block, op, arg1, arg2, loc);
}

static struct hlsl_ir_node *add_binary_comparison_expr(struct hlsl_ctx *ctx, struct hlsl_block *block,
        enum hlsl_ir_expr_op op, struct hlsl_ir_node *arg1, struct hlsl_ir_node *arg2,
        const struct vkd3d_shader_location *loc)
{
    struct hlsl_type *common_type, *return_type;
    enum hlsl_type_class type;
    enum hlsl_base_type base;
    unsigned int dimx, dimy;
    struct hlsl_ir_node *args[HLSL_MAX_OPERANDS] = {0};

    if (!expr_common_shape(ctx, arg1->data_type, arg2->data_type, loc, &type, &dimx, &dimy))
        return NULL;

    base = expr_common_base_type(arg1->data_type->e.numeric.type, arg2->data_type->e.numeric.type);
    common_type = hlsl_get_numeric_type(ctx, type, base, dimx, dimy);
    return_type = hlsl_get_numeric_type(ctx, type, HLSL_TYPE_BOOL, dimx, dimy);

    args[0] = add_implicit_conversion(ctx, block, arg1, common_type, loc);
    args[1] = add_implicit_conversion(ctx, block, arg2, common_type, loc);
    return add_expr(ctx, block, op, args, return_type, loc);
}

static struct hlsl_ir_node *add_binary_logical_expr(struct hlsl_ctx *ctx, struct hlsl_block *block,
        enum hlsl_ir_expr_op op, struct hlsl_ir_node *arg1, struct hlsl_ir_node *arg2,
        const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *args[HLSL_MAX_OPERANDS] = {0};
    struct hlsl_type *common_type;
    enum hlsl_type_class type;
    unsigned int dimx, dimy;

    if (!expr_common_shape(ctx, arg1->data_type, arg2->data_type, loc, &type, &dimx, &dimy))
        return NULL;

    common_type = hlsl_get_numeric_type(ctx, type, HLSL_TYPE_BOOL, dimx, dimy);

    args[0] = add_implicit_conversion(ctx, block, arg1, common_type, loc);
    args[1] = add_implicit_conversion(ctx, block, arg2, common_type, loc);
    return add_expr(ctx, block, op, args, common_type, loc);
}

static struct hlsl_ir_node *add_binary_shift_expr(struct hlsl_ctx *ctx, struct hlsl_block *block,
        enum hlsl_ir_expr_op op, struct hlsl_ir_node *arg1, struct hlsl_ir_node *arg2,
        const struct vkd3d_shader_location *loc)
{
    enum hlsl_base_type base = arg1->data_type->e.numeric.type;
    struct hlsl_ir_node *args[HLSL_MAX_OPERANDS] = {0};
    struct hlsl_type *return_type, *integer_type;
    enum hlsl_type_class type;
    unsigned int dimx, dimy;

    check_integer_type(ctx, arg1);
    check_integer_type(ctx, arg2);

    if (base == HLSL_TYPE_BOOL)
        base = HLSL_TYPE_INT;

    if (!expr_common_shape(ctx, arg1->data_type, arg2->data_type, loc, &type, &dimx, &dimy))
        return NULL;

    return_type = hlsl_get_numeric_type(ctx, type, base, dimx, dimy);
    integer_type = hlsl_get_numeric_type(ctx, type, HLSL_TYPE_INT, dimx, dimy);

    args[0] = add_implicit_conversion(ctx, block, arg1, return_type, loc);
    args[1] = add_implicit_conversion(ctx, block, arg2, integer_type, loc);
    return add_expr(ctx, block, op, args, return_type, loc);
}

static struct hlsl_ir_node *add_binary_dot_expr(struct hlsl_ctx *ctx, struct hlsl_block *instrs,
        struct hlsl_ir_node *arg1, struct hlsl_ir_node *arg2, const struct vkd3d_shader_location *loc)
{
    enum hlsl_base_type base = expr_common_base_type(arg1->data_type->e.numeric.type, arg2->data_type->e.numeric.type);
    struct hlsl_ir_node *args[HLSL_MAX_OPERANDS] = {0};
    struct hlsl_type *common_type, *ret_type;
    enum hlsl_ir_expr_op op;
    unsigned dim;

    if (arg1->data_type->class == HLSL_CLASS_MATRIX)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, arg1->data_type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE, "Invalid type %s.", string->buffer);
        hlsl_release_string_buffer(ctx, string);
        return NULL;
    }

    if (arg2->data_type->class == HLSL_CLASS_MATRIX)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, arg2->data_type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE, "Invalid type %s.", string->buffer);
        hlsl_release_string_buffer(ctx, string);
        return NULL;
    }

    if (arg1->data_type->class == HLSL_CLASS_SCALAR)
        dim = arg2->data_type->e.numeric.dimx;
    else if (arg2->data_type->class == HLSL_CLASS_SCALAR)
        dim = arg1->data_type->e.numeric.dimx;
    else
        dim = min(arg1->data_type->e.numeric.dimx, arg2->data_type->e.numeric.dimx);

    if (dim == 1)
        op = HLSL_OP2_MUL;
    else
        op = HLSL_OP2_DOT;

    common_type = hlsl_get_vector_type(ctx, base, dim);
    ret_type = hlsl_get_scalar_type(ctx, base);

    args[0] = add_implicit_conversion(ctx, instrs, arg1, common_type, loc);
    args[1] = add_implicit_conversion(ctx, instrs, arg2, common_type, loc);
    return add_expr(ctx, instrs, op, args, ret_type, loc);
}

static struct hlsl_ir_node *add_binary_expr(struct hlsl_ctx *ctx, struct hlsl_block *block, enum hlsl_ir_expr_op op,
        struct hlsl_ir_node *lhs, struct hlsl_ir_node *rhs, const struct vkd3d_shader_location *loc)
{
    switch (op)
    {
        case HLSL_OP2_ADD:
        case HLSL_OP2_DIV:
        case HLSL_OP2_MOD:
        case HLSL_OP2_MUL:
            return add_binary_arithmetic_expr(ctx, block, op, lhs, rhs, loc);

        case HLSL_OP2_BIT_AND:
        case HLSL_OP2_BIT_OR:
        case HLSL_OP2_BIT_XOR:
            return add_binary_bitwise_expr(ctx, block, op, lhs, rhs, loc);

        case HLSL_OP2_LESS:
        case HLSL_OP2_GEQUAL:
        case HLSL_OP2_EQUAL:
        case HLSL_OP2_NEQUAL:
            return add_binary_comparison_expr(ctx, block, op, lhs, rhs, loc);

        case HLSL_OP2_LOGIC_AND:
        case HLSL_OP2_LOGIC_OR:
            return add_binary_logical_expr(ctx, block, op, lhs, rhs, loc);

        case HLSL_OP2_LSHIFT:
        case HLSL_OP2_RSHIFT:
            return add_binary_shift_expr(ctx, block, op, lhs, rhs, loc);

        default:
            vkd3d_unreachable();
    }
}

static struct hlsl_block *add_binary_expr_merge(struct hlsl_ctx *ctx, struct hlsl_block *block1,
        struct hlsl_block *block2, enum hlsl_ir_expr_op op, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg1 = node_from_block(block1), *arg2 = node_from_block(block2);

    hlsl_block_add_block(block1, block2);
    destroy_block(block2);

    if (arg1->data_type->class == HLSL_CLASS_ERROR || arg2->data_type->class == HLSL_CLASS_ERROR)
    {
        block1->value = ctx->error_instr;
        return block1;
    }

    if (add_binary_expr(ctx, block1, op, arg1, arg2, loc) == NULL)
        return NULL;

    return block1;
}

static enum hlsl_ir_expr_op op_from_assignment(enum parse_assign_op op)
{
    static const enum hlsl_ir_expr_op ops[] =
    {
        0,
        HLSL_OP2_ADD,
        0,
        HLSL_OP2_MUL,
        HLSL_OP2_DIV,
        HLSL_OP2_MOD,
        HLSL_OP2_LSHIFT,
        HLSL_OP2_RSHIFT,
        HLSL_OP2_BIT_AND,
        HLSL_OP2_BIT_OR,
        HLSL_OP2_BIT_XOR,
    };

    return ops[op];
}

static bool invert_swizzle(uint32_t *swizzle, unsigned int *writemask, unsigned int *ret_width)
{
    unsigned int i, j, bit = 0, inverted = 0, width, new_writemask = 0, new_swizzle = 0;

    /* Apply the writemask to the swizzle to get a new writemask and swizzle. */
    for (i = 0; i < 4; ++i)
    {
        if (*writemask & (1 << i))
        {
            unsigned int s = hlsl_swizzle_get_component(*swizzle, i);
            hlsl_swizzle_set_component(&new_swizzle, bit++, s);
            if (new_writemask & (1 << s))
                return false;
            new_writemask |= 1 << s;
        }
    }
    width = bit;

    /* Invert the swizzle. */
    bit = 0;
    for (i = 0; i < 4; ++i)
    {
        for (j = 0; j < width; ++j)
        {
            unsigned int s = hlsl_swizzle_get_component(new_swizzle, j);
            if (s == i)
                hlsl_swizzle_set_component(&inverted, bit++, j);
        }
    }

    *swizzle = inverted;
    *writemask = new_writemask;
    *ret_width = width;
    return true;
}

static bool invert_swizzle_matrix(const struct hlsl_matrix_swizzle *swizzle,
        uint32_t *ret_inverted, unsigned int *writemask, unsigned int *ret_width)
{
    unsigned int i, j, bit = 0, inverted = 0, width, new_writemask = 0;
    struct hlsl_matrix_swizzle new_swizzle = {0};

    /* First, we filter the swizzle to remove components that aren't enabled by writemask. */
    for (i = 0; i < 4; ++i)
    {
        if (*writemask & (1 << i))
        {
            unsigned int x = swizzle->components[i].x;
            unsigned int y = swizzle->components[i].y;
            unsigned int idx = x + y * 4;

            new_swizzle.components[bit++] = swizzle->components[i];
            if (new_writemask & (1 << idx))
                return false;
            new_writemask |= 1 << idx;
        }
    }
    width = bit;

    /* Then we invert the swizzle. The resulting swizzle uses a uint32_t
     * vector format, because it's for the incoming vector. */
    bit = 0;
    for (i = 0; i < 16; ++i)
    {
        for (j = 0; j < width; ++j)
        {
            unsigned int x = new_swizzle.components[j].x;
            unsigned int y = new_swizzle.components[j].y;
            unsigned int idx = x + y * 4;
            if (idx == i)
                hlsl_swizzle_set_component(&inverted, bit++, j);
        }
    }

    *ret_inverted = inverted;
    *writemask = new_writemask;
    *ret_width = width;
    return true;
}

static struct hlsl_ir_node *resolve_assignment_lhs(struct hlsl_ctx *ctx, struct hlsl_block *block,
        bool is_function_out_arg, struct hlsl_ir_node *lhs, struct hlsl_type **lhs_type,
        struct hlsl_ir_node **rhs, unsigned int *writemask, bool *matrix_writemask)
{
    unsigned int width = 0;
    bool first_cast = true;

    if (hlsl_is_numeric_type(lhs->data_type))
    {
        unsigned int size = hlsl_type_component_count(lhs->data_type);

        *writemask = (1 << size) - 1;
        width = size;
    }
    else
    {
        *writemask = 0;
    }

    *lhs_type = lhs->data_type;
    *matrix_writemask = false;

    while (lhs->type != HLSL_IR_LOAD && lhs->type != HLSL_IR_INDEX)
    {
        if (lhs->type == HLSL_IR_EXPR && hlsl_ir_expr(lhs)->op == HLSL_OP1_CAST)
        {
            struct hlsl_ir_node *cast = lhs;
            lhs = hlsl_ir_expr(cast)->operands[0].node;

            if (hlsl_type_component_count(lhs->data_type) != hlsl_type_component_count(cast->data_type))
            {
                hlsl_fixme(ctx, &cast->loc, "Size change on the LHS.");
                return NULL;
            }
            if (hlsl_version_ge(ctx, 4, 0) && (!is_function_out_arg || !first_cast))
            {
                hlsl_error(ctx, &cast->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_LVALUE,
                        "Base type casts are not allowed on the LHS for profiles >= 4.");
                return NULL;
            }

            *lhs_type = lhs->data_type;
            if ((*lhs_type)->class == HLSL_CLASS_VECTOR
                    || ((*lhs_type)->class == HLSL_CLASS_MATRIX && *matrix_writemask))
                *lhs_type = hlsl_get_vector_type(ctx, lhs->data_type->e.numeric.type, width);

            first_cast = false;
        }
        else if (lhs->type == HLSL_IR_SWIZZLE)
        {
            struct hlsl_ir_swizzle *swizzle = hlsl_ir_swizzle(lhs);
            uint32_t s;

            VKD3D_ASSERT(!*matrix_writemask);

            if (swizzle->val.node->data_type->class == HLSL_CLASS_MATRIX)
            {
                struct hlsl_matrix_swizzle ms = swizzle->u.matrix;

                if (swizzle->val.node->type != HLSL_IR_LOAD && swizzle->val.node->type != HLSL_IR_INDEX)
                {
                    hlsl_fixme(ctx, &lhs->loc, "Unhandled source of matrix swizzle.");
                    return NULL;
                }
                if (!invert_swizzle_matrix(&ms, &s, writemask, &width))
                {
                    hlsl_error(ctx, &lhs->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_WRITEMASK,
                            "Invalid writemask for matrix.");
                    return NULL;
                }
                *matrix_writemask = true;
            }
            else
            {
                s = swizzle->u.vector;
                if (!invert_swizzle(&s, writemask, &width))
                {
                    hlsl_error(ctx, &lhs->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_WRITEMASK, "Invalid writemask.");
                    return NULL;
                }
            }

            *rhs = hlsl_block_add_swizzle(ctx, block, s, width, *rhs, &swizzle->node.loc);
            lhs = swizzle->val.node;
            *lhs_type = hlsl_get_vector_type(ctx, (*lhs_type)->e.numeric.type, width);
        }
        else
        {
            hlsl_error(ctx, &lhs->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_LVALUE, "Invalid lvalue.");
            return NULL;
        }
    }

    return lhs;
}

static bool add_assignment(struct hlsl_ctx *ctx, struct hlsl_block *block, struct hlsl_ir_node *lhs,
        enum parse_assign_op assign_op, struct hlsl_ir_node *rhs, bool is_function_out_arg)
{
    struct hlsl_type *lhs_type = lhs->data_type;
    bool matrix_writemask = false;
    unsigned int writemask = 0;

    if (lhs->data_type->class == HLSL_CLASS_ERROR || rhs->data_type->class == HLSL_CLASS_ERROR)
    {
        block->value = ctx->error_instr;
        return true;
    }

    if (assign_op == ASSIGN_OP_SUB)
    {
        if (!(rhs = add_unary_arithmetic_expr(ctx, block, HLSL_OP1_NEG, rhs, &rhs->loc)))
            return false;
        assign_op = ASSIGN_OP_ADD;
    }
    if (assign_op != ASSIGN_OP_ASSIGN)
    {
        enum hlsl_ir_expr_op op = op_from_assignment(assign_op);

        VKD3D_ASSERT(op);
        if (!(rhs = add_binary_expr(ctx, block, op, lhs, rhs, &rhs->loc)))
            return false;
    }

    rhs = add_implicit_conversion(ctx, block, rhs, lhs_type, &rhs->loc);

    if (!(lhs = resolve_assignment_lhs(ctx, block, is_function_out_arg, lhs,
            &lhs_type, &rhs, &writemask, &matrix_writemask)))
        return false;

    /* lhs casts could have resulted in a discrepancy between the
     * rhs->data_type and the type of the variable that will be ulimately
     * stored to. This is corrected. */
    rhs = add_cast(ctx, block, rhs, lhs_type, &rhs->loc);

    if (matrix_writemask)
    {
        struct hlsl_deref deref;
        unsigned int i, j, k = 0;

        hlsl_deref_init_from_index_chain(&deref, lhs, ctx);

        for (i = 0; i < lhs->data_type->e.numeric.dimy; ++i)
        {
            for (j = 0; j < lhs->data_type->e.numeric.dimx; ++j)
            {
                struct hlsl_ir_node *load;
                const unsigned int idx = i * 4 + j;
                const unsigned int component = i * lhs->data_type->e.numeric.dimx + j;

                if (!(writemask & (1 << idx)))
                    continue;

                load = hlsl_add_load_component(ctx, block, rhs, k++, &rhs->loc);
                hlsl_block_add_store_component(ctx, block, &deref, component, load);
            }
        }

        hlsl_deref_cleanup(&deref);
    }
    else if (lhs->type == HLSL_IR_INDEX && hlsl_index_is_noncontiguous(hlsl_ir_index(lhs)))
    {
        struct hlsl_ir_index *row = hlsl_ir_index(lhs);
        struct hlsl_ir_node *mat = row->val.node;
        unsigned int i, k = 0;

        VKD3D_ASSERT(!matrix_writemask);

        for (i = 0; i < mat->data_type->e.numeric.dimx; ++i)
        {
            struct hlsl_ir_node *cell, *load, *c;
            struct hlsl_deref deref;

            if (!(writemask & (1 << i)))
                continue;

            c = hlsl_block_add_uint_constant(ctx, block, i, &lhs->loc);

            cell = hlsl_block_add_index(ctx, block, &row->node, c, &lhs->loc);
            load = hlsl_add_load_component(ctx, block, rhs, k++, &rhs->loc);

            if (!hlsl_deref_init_from_index_chain(&deref, cell, ctx))
                return false;

            hlsl_block_add_store_index(ctx, block, &deref, NULL, load, 0, &rhs->loc);
            hlsl_deref_cleanup(&deref);
        }
    }
    else
    {
        struct hlsl_deref deref;

        if (!hlsl_deref_init_from_index_chain(&deref, lhs, ctx))
            return false;

        hlsl_block_add_store_index(ctx, block, &deref, NULL, rhs, writemask, &rhs->loc);
        hlsl_deref_cleanup(&deref);
    }

    block->value = rhs;
    return true;
}

static bool add_increment(struct hlsl_ctx *ctx, struct hlsl_block *block, bool decrement, bool post,
        const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *lhs = node_from_block(block);
    struct hlsl_ir_node *one;

    if (lhs->data_type->class == HLSL_CLASS_ERROR)
        return true;

    if (lhs->data_type->modifiers & HLSL_MODIFIER_CONST)
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_MODIFIES_CONST,
                "Argument to %s%screment operator is const.", post ? "post" : "pre", decrement ? "de" : "in");

    one = hlsl_block_add_int_constant(ctx, block, 1, loc);

    if (!add_assignment(ctx, block, lhs, decrement ? ASSIGN_OP_SUB : ASSIGN_OP_ADD, one, false))
        return false;

    if (post)
    {
        struct hlsl_ir_node *copy;

        if (!(copy = hlsl_new_copy(ctx, lhs)))
            return false;
        hlsl_block_add_instr(block, copy);

        /* Post increment/decrement expressions are considered const. */
        if (!(copy->data_type = hlsl_type_clone(ctx, copy->data_type, 0, HLSL_MODIFIER_CONST)))
            return false;
    }

    return true;
}

static void initialize_var_components(struct hlsl_ctx *ctx, struct hlsl_block *instrs,
        struct hlsl_ir_var *dst, unsigned int *store_index, struct hlsl_ir_node *src,
        bool is_default_values_initializer)
{
    unsigned int src_comp_count = hlsl_type_component_count(src->data_type);
    struct hlsl_deref dst_deref;
    unsigned int k;

    hlsl_deref_init_simple(&dst_deref, dst);

    for (k = 0; k < src_comp_count; ++k)
    {
        struct hlsl_ir_node *conv, *load;
        struct hlsl_type *dst_comp_type;
        struct hlsl_block block;

        load = hlsl_add_load_component(ctx, instrs, src, k, &src->loc);

        dst_comp_type = hlsl_type_get_component_type(ctx, dst->data_type, *store_index);

        if (is_default_values_initializer)
        {
            struct hlsl_default_value default_value = {0};

            if ((src->type == HLSL_IR_SAMPLER_STATE || src->type == HLSL_IR_COMPILE)
                    && hlsl_is_numeric_type(dst_comp_type) && dst->default_values)
            {
                /* Default values are discarded if they contain an object
                 * literal expression for a numeric component. */
                hlsl_warning(ctx, &src->loc, VKD3D_SHADER_WARNING_HLSL_IGNORED_DEFAULT_VALUE,
                        "Component %u in variable '%s' initializer is an object literal. Default values discarded.",
                        k, dst->name);
                hlsl_free_default_values(dst);
            }
            else if (src->type != HLSL_IR_SAMPLER_STATE)
            {
                if (!hlsl_clone_block(ctx, &block, instrs))
                    return;
                default_value = evaluate_static_expression(ctx, &block, dst_comp_type, true, &src->loc);

                if (dst->default_values)
                    dst->default_values[*store_index] = default_value;
                else
                    hlsl_free_default_value(&default_value);

                hlsl_block_cleanup(&block);
            }
        }
        else
        {
            if (src->type == HLSL_IR_SAMPLER_STATE)
            {
                /* Sampler states end up in the variable's state_blocks instead of
                 * being used to initialize its value. */
                struct hlsl_ir_sampler_state *sampler_state = hlsl_ir_sampler_state(src);

                if (dst_comp_type->class != HLSL_CLASS_SAMPLER)
                {
                    struct vkd3d_string_buffer *dst_string;

                    dst_string = hlsl_type_to_string(ctx, dst_comp_type);
                    if (dst_string)
                        hlsl_error(ctx, &src->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                                "Cannot assign sampler_state to %s.", dst_string->buffer);
                    hlsl_release_string_buffer(ctx, dst_string);
                    return;
                }

                if (!hlsl_array_reserve(ctx, (void **)&dst->state_blocks, &dst->state_block_capacity,
                        dst->state_block_count + 1, sizeof(*dst->state_blocks)))
                    return;

                dst->state_blocks[dst->state_block_count] = sampler_state->state_block;
                sampler_state->state_block = NULL;
                ++dst->state_block_count;
            }
            else
            {
                conv = add_implicit_conversion(ctx, instrs, load, dst_comp_type, &src->loc);
                hlsl_block_add_store_component(ctx, instrs, &dst_deref, *store_index, conv);
            }
        }

        ++*store_index;
    }
}

static void initialize_var(struct hlsl_ctx *ctx, struct hlsl_ir_var *dst,
        const struct parse_initializer *initializer, bool is_default_values_initializer)
{
    unsigned int store_index = 0;

    /* If any of the elements has an error type, then initializer_size() is not
     * meaningful. */
    for (unsigned int i = 0; i < initializer->args_count; ++i)
    {
        if (initializer->args[i]->data_type->class == HLSL_CLASS_ERROR)
            return;
    }

    if (initializer_size(initializer) != hlsl_type_component_count(dst->data_type))
    {
        hlsl_error(ctx, &initializer->loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                "Expected %u components in initializer, but got %u.",
                hlsl_type_component_count(dst->data_type), initializer_size(initializer));
        return;
    }

    for (unsigned int i = 0; i < initializer->args_count; ++i)
        initialize_var_components(ctx, initializer->instrs, dst, &store_index,
                initializer->args[i], is_default_values_initializer);
}

static bool type_has_object_components(const struct hlsl_type *type)
{
    if (type->class == HLSL_CLASS_ARRAY)
        return type_has_object_components(type->e.array.type);

    if (type->class == HLSL_CLASS_STRUCT)
    {
        for (unsigned int i = 0; i < type->e.record.field_count; ++i)
        {
            if (type_has_object_components(type->e.record.fields[i].type))
                return true;
        }

        return false;
    }

    return !hlsl_is_numeric_type(type);
}

static bool type_has_numeric_components(struct hlsl_type *type)
{
    if (hlsl_is_numeric_type(type))
        return true;
    if (type->class == HLSL_CLASS_ARRAY)
        return type_has_numeric_components(type->e.array.type);

    if (type->class == HLSL_CLASS_STRUCT)
    {
        unsigned int i;

        for (i = 0; i < type->e.record.field_count; ++i)
        {
            if (type_has_numeric_components(type->e.record.fields[i].type))
                return true;
        }
    }
    return false;
}

static void check_invalid_non_parameter_modifiers(struct hlsl_ctx *ctx, unsigned int modifiers,
        const struct vkd3d_shader_location *loc)
{
    modifiers &= (HLSL_STORAGE_IN | HLSL_STORAGE_OUT | HLSL_PRIMITIVE_MODIFIERS_MASK);
    if (modifiers)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_modifiers_to_string(ctx, modifiers)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_MODIFIER,
                    "Modifiers '%s' are not allowed on non-parameter variables.", string->buffer);
        hlsl_release_string_buffer(ctx, string);
    }
}

static void check_invalid_object_fields(struct hlsl_ctx *ctx, const struct hlsl_ir_var *var)
{
    const struct hlsl_type *type = var->data_type;

    while (type->class == HLSL_CLASS_ARRAY)
        type = type->e.array.type;

    if (type->class == HLSL_CLASS_STRUCT && type_has_object_components(type))
        hlsl_error(ctx, &var->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                "Target profile doesn't support objects as struct members in uniform variables.");
}

static void validate_groupshared_var(struct hlsl_ctx *ctx, const struct hlsl_ir_var *var)
{
    if (type_has_object_components(var->data_type))
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, var->data_type)))
        {
            hlsl_error(ctx, &var->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Groupshared type %s is not numeric.", string->buffer);
            hlsl_release_string_buffer(ctx, string);
        }
    }
}

static void declare_var(struct hlsl_ctx *ctx, struct parse_variable_def *v)
{
    bool nonlocal = ctx->cur_scope == ctx->globals || ctx->cur_scope->namespace;
    uint32_t modifiers = v->modifiers | v->semantic.modifiers;
    struct hlsl_type *basic_type = v->basic_type;
    struct hlsl_ir_function_decl *func;
    struct hlsl_semantic new_semantic;
    bool unbounded_res_array = false;
    bool constant_buffer = false;
    struct hlsl_scope *scope;
    struct hlsl_ir_var *var;
    struct hlsl_type *type;
    bool stream_output;
    char *var_name;
    unsigned int i;

    VKD3D_ASSERT(basic_type);

    if (basic_type->class == HLSL_CLASS_MATRIX)
        VKD3D_ASSERT(basic_type->modifiers & HLSL_MODIFIERS_MAJORITY_MASK);

    type = basic_type;

    if (basic_type->class <= HLSL_CLASS_LAST_NUMERIC && basic_type->e.numeric.type == HLSL_TYPE_HALF
            && !v->arrays.count && nonlocal && !ctx->compile_info.allow_half_globals
            && ctx->profile->type != VKD3D_SHADER_TYPE_EFFECT)
    {
        hlsl_error(ctx, &v->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                "The half type is not allowed for global variables.");
    }

    if (hlsl_version_ge(ctx, 5, 1) && hlsl_type_is_resource(type))
    {
        for (i = 0; i < v->arrays.count; ++i)
            unbounded_res_array |= (v->arrays.sizes[i] == HLSL_ARRAY_ELEMENTS_COUNT_IMPLICIT);
    }

    if (type->class == HLSL_CLASS_CONSTANT_BUFFER)
    {
        type = type->e.resource.format;
        constant_buffer = true;
    }

    if (unbounded_res_array)
    {
        if (v->arrays.count == 1)
        {
            hlsl_fixme(ctx, &v->loc, "Unbounded resource arrays.");
            return;
        }
        else
        {
            hlsl_error(ctx, &v->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Unbounded resource arrays cannot be multi-dimensional.");
        }
    }
    else
    {
        for (i = 0; i < v->arrays.count; ++i)
        {
            if (v->arrays.sizes[i] == HLSL_ARRAY_ELEMENTS_COUNT_IMPLICIT)
            {
                unsigned int size = initializer_size(&v->initializer);
                unsigned int elem_components = hlsl_type_component_count(type);

                if (i < v->arrays.count - 1)
                {
                    hlsl_error(ctx, &v->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                            "Only innermost array size can be implicit.");
                    type = ctx->builtin_types.error;
                    break;
                }
                else if (elem_components == 0)
                {
                    hlsl_error(ctx, &v->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                            "Cannot declare an implicit size array of a size 0 type.");
                    type = ctx->builtin_types.error;
                    break;
                }
                else if (size == 0)
                {
                    hlsl_error(ctx, &v->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                            "Implicit size arrays need to be initialized.");
                    type = ctx->builtin_types.error;
                    break;
                }
                else if (size % elem_components != 0)
                {
                    hlsl_error(ctx, &v->loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                            "Cannot initialize implicit size array with %u components, expected a multiple of %u.",
                            size, elem_components);
                    type = ctx->builtin_types.error;
                    break;
                }
                else
                {
                    v->arrays.sizes[i] = size / elem_components;
                }
            }
            type = hlsl_new_array_type(ctx, type, v->arrays.sizes[i], HLSL_ARRAY_GENERIC);
        }
    }

    if (hlsl_version_ge(ctx, 5, 1) && type->class == HLSL_CLASS_ARRAY && hlsl_type_is_resource(type))
    {
        /* SM 5.1/6.x descriptor arrays act differently from previous versions.
         * Not only are they treated as a single object in reflection, but they
         * act as a single component for the purposes of assignment and
         * initialization. */
        hlsl_fixme(ctx, &v->loc, "Shader model 5.1+ resource array.");
    }

    stream_output = !!hlsl_get_stream_output_type(type);
    if (stream_output)
        check_invalid_stream_output_object(ctx, type, v->name, &v->loc);

    if (!(var_name = vkd3d_strdup(v->name)))
        return;

    if (!hlsl_clone_semantic(ctx, &new_semantic, &v->semantic))
    {
        vkd3d_free(var_name);
        return;
    }

    if (modifiers & HLSL_STORAGE_GROUPSHARED)
        modifiers &= ~HLSL_STORAGE_STATIC;

    if (ctx->cur_scope == ctx->globals
            && !(modifiers & (HLSL_STORAGE_STATIC | HLSL_STORAGE_GROUPSHARED)))
        modifiers |= HLSL_STORAGE_UNIFORM;

    if (ctx->cur_scope == ctx->globals && (modifiers & HLSL_STORAGE_UNIFORM)
            && ctx->compile_info.const_global_uniforms)
        type = hlsl_type_clone(ctx, type, 0, HLSL_MODIFIER_CONST);

    if (!(var = hlsl_new_var(ctx, var_name, type, &v->loc, &new_semantic, modifiers, &v->reg_reservation)))
    {
        hlsl_cleanup_semantic(&new_semantic);
        vkd3d_free(var_name);
        return;
    }

    var->annotations = v->annotations;

    if (constant_buffer && ctx->cur_scope == ctx->globals)
    {
        if (!(var_name = vkd3d_strdup(v->name)))
            return;
        var->buffer = hlsl_new_buffer(ctx, HLSL_BUFFER_CONSTANT, var_name, modifiers, &v->reg_reservation, NULL, &v->loc);
    }
    else
    {
        var->buffer = ctx->cur_buffer;
    }

    if (var->buffer == ctx->globals_buffer)
    {
        if (var->reg_reservation.offset_type)
            hlsl_error(ctx, &var->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                    "packoffset() is only allowed inside constant buffer declarations.");
    }

    if (ctx->cur_scope->namespace)
    {
        if (!(modifiers & HLSL_STORAGE_STATIC))
            hlsl_fixme(ctx, &v->loc, "Non-static variables inside namespaces.");
        if (constant_buffer || !type_contains_only_numerics(type))
            hlsl_fixme(ctx, &v->loc, "Non-numeric variables inside namespaces.");
    }

    if (!(modifiers & (HLSL_STORAGE_UNIFORM | HLSL_STORAGE_STATIC | HLSL_STORAGE_GROUPSHARED))
            && (type->modifiers & HLSL_MODIFIER_CONST) && !v->initializer.args_count)
        hlsl_error(ctx, &var->loc, VKD3D_SHADER_ERROR_HLSL_MISSING_INITIALIZER,
                "Const variable \"%s\" is missing an initializer.", var->name);

    if (nonlocal)
    {
        if ((modifiers & HLSL_STORAGE_UNIFORM) && (modifiers & HLSL_STORAGE_STATIC))
            hlsl_error(ctx, &var->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_MODIFIER,
                    "Variable '%s' is declared as both \"uniform\" and \"static\".", var->name);

        if ((modifiers & HLSL_STORAGE_UNIFORM) && (modifiers & HLSL_STORAGE_GROUPSHARED))
            hlsl_error(ctx, &var->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_MODIFIER,
                    "Variable '%s' is declared as both \"uniform\" and \"groupshared\".", var->name);

        /* d3dcompiler/fxc always validates global groupshared variables,
         * regardless of whether the groupshared modifier is ignored. */
        if (modifiers & HLSL_STORAGE_GROUPSHARED)
            validate_groupshared_var(ctx, var);

        if (stream_output)
            hlsl_error(ctx, &var->loc, VKD3D_SHADER_ERROR_HLSL_MISPLACED_STREAM_OUTPUT,
                    "Stream output object '%s' is not allowed in the global scope.", var->name);

        if ((ctx->profile->major_version < 5 || ctx->profile->type == VKD3D_SHADER_TYPE_EFFECT)
                && (var->storage_modifiers & HLSL_STORAGE_UNIFORM))
        {
            check_invalid_object_fields(ctx, var);
        }

        if ((func = hlsl_get_first_func_decl(ctx, var->name)))
        {
            hlsl_error(ctx, &var->loc, VKD3D_SHADER_ERROR_HLSL_REDEFINED,
                    "'%s' is already defined as a function.", var->name);
            hlsl_note(ctx, &func->loc, VKD3D_SHADER_LOG_ERROR,
                    "'%s' was previously defined here.", var->name);
        }

        if ((scope = hlsl_get_namespace(ctx->cur_scope, var->name)))
        {
            hlsl_error(ctx, &var->loc, VKD3D_SHADER_ERROR_HLSL_REDEFINED,
                    "'%s' is already defined as a namespace.", var->name);
            hlsl_note(ctx, &scope->namespace_loc, VKD3D_SHADER_LOG_ERROR,
                    "'%s' was previously defined here.", var->name);
        }
    }
    else
    {
        static const unsigned int invalid = HLSL_STORAGE_EXTERN | HLSL_STORAGE_SHARED
                | HLSL_STORAGE_GROUPSHARED | HLSL_STORAGE_UNIFORM;

        if (modifiers & invalid)
        {
            struct vkd3d_string_buffer *string;

            if ((string = hlsl_modifiers_to_string(ctx, modifiers & invalid)))
                hlsl_error(ctx, &var->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_MODIFIER,
                        "Modifiers '%s' are not allowed on local variables.", string->buffer);
            hlsl_release_string_buffer(ctx, string);
        }
        if (var->semantic.name)
            hlsl_error(ctx, &var->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_SEMANTIC,
                    "Semantics are not allowed on local variables.");

        if ((type->modifiers & HLSL_MODIFIER_CONST) && !v->initializer.args_count && !(modifiers & HLSL_STORAGE_STATIC))
        {
            hlsl_error(ctx, &v->loc, VKD3D_SHADER_ERROR_HLSL_MISSING_INITIALIZER,
                "Const variable \"%s\" is missing an initializer.", var->name);
        }

        if (var->annotations)
        {
            hlsl_error(ctx, &v->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX,
                    "Annotations are only allowed for objects in the global scope.");
        }
    }

    if ((var->storage_modifiers & HLSL_STORAGE_STATIC) && type_has_numeric_components(var->data_type)
            && type_has_object_components(var->data_type))
    {
        hlsl_error(ctx, &var->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                "Static variables cannot have both numeric and resource components.");
    }

    hlsl_add_var(ctx, var);
}

static struct hlsl_block *initialize_vars(struct hlsl_ctx *ctx, struct list *var_list)
{
    struct parse_variable_def *v, *v_next;
    struct hlsl_block *initializers;
    unsigned int component_count;
    struct hlsl_ir_var *var;
    struct hlsl_type *type;

    if (!(initializers = make_empty_block(ctx)))
    {
        destroy_parse_variable_defs(var_list);
        return NULL;
    }

    LIST_FOR_EACH_ENTRY_SAFE(v, v_next, var_list, struct parse_variable_def, entry)
    {
        /* If this fails, the variable failed to be declared. */
        if (!(var = hlsl_get_var(ctx->cur_scope, v->name)))
        {
            free_parse_variable_def(v);
            continue;
        }

        type = var->data_type;
        component_count = hlsl_type_component_count(type);

        var->state_blocks = v->state_blocks;
        var->state_block_count = v->state_block_count;
        var->state_block_capacity = v->state_block_capacity;
        v->state_block_count = 0;
        v->state_block_capacity = 0;
        v->state_blocks = NULL;

        if (var->state_blocks && component_count != var->state_block_count)
        {
            hlsl_error(ctx, &v->loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                    "Expected %u state blocks, but got %u.", component_count, var->state_block_count);
            free_parse_variable_def(v);
            continue;
        }

        if (v->initializer.args_count)
        {
            bool is_default_values_initializer, static_initialization;

            is_default_values_initializer = (ctx->cur_buffer != ctx->globals_buffer)
                    || (var->storage_modifiers & HLSL_STORAGE_UNIFORM)
                    || ctx->cur_scope->annotations;
            if (hlsl_get_multiarray_element_type(type)->class == HLSL_CLASS_SAMPLER)
                is_default_values_initializer = false;

            static_initialization = var->storage_modifiers & HLSL_STORAGE_STATIC
                    || (var->data_type->modifiers & HLSL_MODIFIER_CONST
                    && is_compile_time_const(v->initializer.instrs, false));

            if (is_default_values_initializer)
            {
                /* Default values might have been allocated already for another variable of the same name,
                   in the same scope. */
                if (var->default_values)
                {
                    free_parse_variable_def(v);
                    continue;
                }

                if (!(var->default_values = hlsl_calloc(ctx, component_count, sizeof(*var->default_values))))
                {
                    free_parse_variable_def(v);
                    continue;
                }
            }

            if (!v->initializer.braces)
                v->initializer.args[0] = add_implicit_conversion(ctx,
                        v->initializer.instrs, v->initializer.args[0], type, &v->loc);

            if (var->data_type->class != HLSL_CLASS_ERROR)
                initialize_var(ctx, var, &v->initializer, is_default_values_initializer);

            if (is_default_values_initializer)
            {
                hlsl_dump_var_default_values(var);
            }
            else if (static_initialization)
            {
                hlsl_block_add_block(&ctx->static_initializers, v->initializer.instrs);
                var->is_compile_time_const = true;
            }
            else
            {
                hlsl_block_add_block(initializers, v->initializer.instrs);
            }

            if (var->state_blocks)
                TRACE("Variable %s has %u state blocks.\n", var->name, var->state_block_count);
        }
        else if (var->storage_modifiers & HLSL_STORAGE_STATIC)
        {
            struct hlsl_ir_node *cast, *zero;

            /* Initialize statics to zero by default. */

            if (type_has_object_components(var->data_type))
            {
                free_parse_variable_def(v);
                continue;
            }

            zero = hlsl_block_add_uint_constant(ctx, &ctx->static_initializers, 0, &var->loc);
            cast = add_cast(ctx, &ctx->static_initializers, zero, var->data_type, &var->loc);
            hlsl_block_add_simple_store(ctx, &ctx->static_initializers, var, cast);
            var->is_compile_time_const = true;
        }
        free_parse_variable_def(v);
    }

    vkd3d_free(var_list);
    return initializers;
}

static bool func_is_compatible_match(struct hlsl_ctx *ctx, const struct hlsl_ir_function_decl *decl,
        bool is_compile, const struct parse_initializer *args)
{
    unsigned int i, k;

    k = 0;
    for (i = 0; i < decl->parameters.count; ++i)
    {
        if (is_compile && !(decl->parameters.vars[i]->storage_modifiers & HLSL_STORAGE_UNIFORM))
            continue;

        if (k >= args->args_count)
        {
            if (!decl->parameters.vars[i]->default_values)
                return false;
            return true;
        }

        if (!implicit_compatible_data_types(ctx, args->args[k]->data_type, decl->parameters.vars[i]->data_type))
            return false;

        ++k;
    }
    if (k < args->args_count)
        return false;
    return true;
}

static enum hlsl_base_type hlsl_base_type_class(enum hlsl_base_type t)
{
    switch (t)
    {
        case HLSL_TYPE_HALF:
        case HLSL_TYPE_FLOAT:
        case HLSL_TYPE_DOUBLE:
            return HLSL_TYPE_FLOAT;

        case HLSL_TYPE_INT:
        case HLSL_TYPE_MIN16UINT:
        case HLSL_TYPE_UINT:
            return HLSL_TYPE_INT;

        case HLSL_TYPE_BOOL:
            return HLSL_TYPE_BOOL;
    }

    return 0;
}

static unsigned int hlsl_base_type_width(enum hlsl_base_type t)
{
    switch (t)
    {
        case HLSL_TYPE_HALF:
        case HLSL_TYPE_MIN16UINT:
            return 16;

        case HLSL_TYPE_FLOAT:
        case HLSL_TYPE_INT:
        case HLSL_TYPE_UINT:
        case HLSL_TYPE_BOOL:
            return 32;

        case HLSL_TYPE_DOUBLE:
            return 64;
    }

    return 0;
}

static uint32_t get_argument_conversion_mask(const struct hlsl_ir_var *parameter, const struct hlsl_ir_node *arg)
{
    enum
    {
        COMPONENT_COUNT_WIDENING      = 1u << 0,
        COMPONENT_TYPE_NARROWING      = 1u << 1,
        COMPONENT_TYPE_MISMATCH       = 1u << 2,
        COMPONENT_TYPE_CLASS_MISMATCH = 1u << 3,
        COMPONENT_COUNT_NARROWING     = 1u << 4,
    };
    struct
    {
        enum hlsl_base_type type;
        enum hlsl_base_type class;
        unsigned int count, width;
    } p, a;
    uint32_t mask = 0;

    /* TODO: Non-numeric types. */
    if (!hlsl_is_numeric_type(arg->data_type))
        return 0;

    p.type = parameter->data_type->e.numeric.type;
    p.class = hlsl_base_type_class(p.type);
    p.count = hlsl_type_component_count(parameter->data_type);
    p.width = hlsl_base_type_width(p.type);

    a.type = arg->data_type->e.numeric.type;
    a.class = hlsl_base_type_class(a.type);
    a.count = hlsl_type_component_count(arg->data_type);
    a.width = hlsl_base_type_width(a.type);

    /* Component count narrowing. E.g., passing a float4 argument to a float2
     * or int2 parameter. */
    if (a.count > p.count)
        mask |= COMPONENT_COUNT_NARROWING;
    /* Different component type classes. E.g., passing an int argument to a
     * float parameter. */
    if (a.class != p.class)
        mask |= COMPONENT_TYPE_CLASS_MISMATCH;
    /* Different component types. E.g., passing an int argument to an uint
     * parameter. */
    if (a.type != p.type)
        mask |= COMPONENT_TYPE_MISMATCH;
    /* Component type narrowing. E.g., passing a float argument to a half
     * parameter. */
    if (a.width > p.width)
        mask |= COMPONENT_TYPE_NARROWING;
    /* Component count widening. E.g., passing an int2 argument to an int4
     * parameter. */
    if (a.count < p.count)
        mask |= COMPONENT_COUNT_WIDENING;

    return mask;
}

static int function_compare(const struct hlsl_ir_function_decl *candidate,
        const struct hlsl_ir_function_decl *ref, const struct parse_initializer *args)
{
    uint32_t candidate_mask = 0, ref_mask = 0, c, r;
    bool any_worse = false, any_better = false;
    unsigned int i;
    int ret;

    for (i = 0; i < args->args_count; ++i)
    {
        candidate_mask |= (c = get_argument_conversion_mask(candidate->parameters.vars[i], args->args[i]));
        ref_mask |= (r = get_argument_conversion_mask(ref->parameters.vars[i], args->args[i]));

        if (c > r)
            any_worse = true;
        else if (c < r)
            any_better = true;
    }

    /* We consider a candidate better if at least one parameter is a better
     * match, and none are a worse match. */
    if ((ret = any_better - any_worse))
        return ret;
    /* Otherwise, consider the kind of conversions across all parameters. */
    return vkd3d_u32_compare(ref_mask, candidate_mask);
}

static struct hlsl_ir_function_decl *find_function_call(struct hlsl_ctx *ctx,
        const char *name, const struct parse_initializer *args, bool is_compile,
        const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_function_decl *decl;
    struct vkd3d_string_buffer *s;
    struct hlsl_ir_function *func;
    struct rb_entry *entry;
    int compare;
    size_t i;
    struct
    {
        struct hlsl_ir_function_decl **candidates;
        size_t count, capacity;
    } candidates = {0};

    if (!(entry = rb_get(&ctx->functions, name)))
        return NULL;
    func = RB_ENTRY_VALUE(entry, struct hlsl_ir_function, entry);

    LIST_FOR_EACH_ENTRY(decl, &func->overloads, struct hlsl_ir_function_decl, entry)
    {
        if (!func_is_compatible_match(ctx, decl, is_compile, args))
            continue;

        if (candidates.count)
        {
            compare = function_compare(decl, candidates.candidates[0], args);

            /* The candidate is worse; skip it. */
            if (compare < 0)
                continue;

            /* The candidate is better; replace the current candidates. */
            if (compare > 0)
            {
                candidates.candidates[0] = decl;
                candidates.count = 1;
                continue;
            }
        }

        if (!(hlsl_array_reserve(ctx, (void **)&candidates.candidates,
                &candidates.capacity, candidates.count + 1, sizeof(decl))))
        {
            vkd3d_free(candidates.candidates);
            return NULL;
        }
        candidates.candidates[candidates.count++] = decl;
    }

    if (!candidates.count)
        return NULL;

    if (candidates.count > 1)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_AMBIGUOUS_CALL, "Ambiguous function call.");
        if ((s = hlsl_get_string_buffer(ctx)))
        {
            hlsl_note(ctx, loc, VKD3D_SHADER_LOG_ERROR, "Candidates are:");
            for (i = 0; i < candidates.count; ++i)
            {
                hlsl_dump_ir_function_decl(ctx, s, candidates.candidates[i]);
                hlsl_note(ctx, loc, VKD3D_SHADER_LOG_ERROR, "    %s;", s->buffer);
                vkd3d_string_buffer_clear(s);
            }
            hlsl_release_string_buffer(ctx, s);
        }
    }

    decl = candidates.candidates[0];
    vkd3d_free(candidates.candidates);

    return decl;
}

static void add_void_expr(struct hlsl_ctx *ctx, struct hlsl_block *block,
        const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *operands[HLSL_MAX_OPERANDS] = {0};

    hlsl_block_add_expr(ctx, block, HLSL_OP0_VOID, operands, ctx->builtin_types.Void, loc);
}

static bool parse_function_call_arguments(struct hlsl_ctx *ctx, const struct hlsl_ir_function_decl *func,
        const struct parse_initializer *args, bool is_compile, const struct vkd3d_shader_location *loc)
{
    unsigned int i, j, k;

    VKD3D_ASSERT(args->args_count <= func->parameters.count);

    k = 0;
    for (i = 0; i < func->parameters.count; ++i)
    {
        struct hlsl_ir_var *param = func->parameters.vars[i];
        struct hlsl_ir_node *arg;

        if (is_compile && !(param->storage_modifiers & HLSL_STORAGE_UNIFORM))
            continue;

        if (k >= args->args_count)
            break;
        arg = args->args[k];

        if (param->storage_modifiers & HLSL_STORAGE_IN)
        {
            arg = add_cast(ctx, args->instrs, arg, param->data_type, &arg->loc);
            hlsl_block_add_simple_store(ctx, args->instrs, param, arg);
        }

        ++k;
    }

    /* Add default values for the remaining parameters. */
    for (; i < func->parameters.count; ++i)
    {
        struct hlsl_ir_var *param = func->parameters.vars[i];
        unsigned int comp_count = hlsl_type_component_count(param->data_type);
        struct hlsl_deref param_deref;

        VKD3D_ASSERT(param->default_values);

        if (is_compile && !(param->storage_modifiers & HLSL_STORAGE_UNIFORM))
            continue;

        hlsl_deref_init_simple(&param_deref, param);

        for (j = 0; j < comp_count; ++j)
        {
            struct hlsl_type *type = hlsl_type_get_component_type(ctx, param->data_type, j);
            struct hlsl_constant_value value;
            struct hlsl_ir_node *comp;

            if (!param->default_values[j].string)
            {
                value.u[0] = param->default_values[j].number;
                comp = hlsl_block_add_constant(ctx, args->instrs, type, &value, loc);
                hlsl_block_add_store_component(ctx, args->instrs, &param_deref, j, comp);
            }
        }
    }

    return true;
}

static struct hlsl_ir_node *add_user_call(struct hlsl_ctx *ctx, struct hlsl_ir_function_decl *func,
        const struct parse_initializer *args, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *call;
    unsigned int i;

    if (!parse_function_call_arguments(ctx, func, args, false, loc))
        return NULL;

    if (!(call = hlsl_new_call(ctx, func, loc)))
        return NULL;

    hlsl_block_add_instr(args->instrs, call);

    for (i = 0; i < args->args_count; ++i)
    {
        struct hlsl_ir_var *param = func->parameters.vars[i];
        struct hlsl_ir_node *arg = args->args[i];

        if (param->storage_modifiers & HLSL_STORAGE_OUT)
        {
            struct hlsl_ir_node *load;

            if (arg->data_type->modifiers & HLSL_MODIFIER_CONST)
                hlsl_error(ctx, &arg->loc, VKD3D_SHADER_ERROR_HLSL_MODIFIES_CONST,
                        "Output argument to \"%s\" is const.", func->func->name);

            load = hlsl_block_add_simple_load(ctx, args->instrs, param, &arg->loc);
            if (!add_assignment(ctx, args->instrs, arg, ASSIGN_OP_ASSIGN, load, true))
                return NULL;
        }
    }

    if (func->return_var)
        hlsl_block_add_simple_load(ctx, args->instrs, func->return_var, loc);
    else
        add_void_expr(ctx, args->instrs, loc);

    return call;
}

static struct hlsl_ir_node *intrinsic_float_convert_arg(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, struct hlsl_ir_node *arg, const struct vkd3d_shader_location *loc)
{
    struct hlsl_type *type = arg->data_type;

    if (!hlsl_type_is_integer(type))
        return arg;

    type = hlsl_change_base_type(ctx, type, HLSL_TYPE_FLOAT);
    return add_implicit_conversion(ctx, params->instrs, arg, type, loc);
}

static void convert_args(struct hlsl_ctx *ctx, const struct parse_initializer *params,
        struct hlsl_type *type, const struct vkd3d_shader_location *loc)
{
    unsigned int i;

    for (i = 0; i < params->args_count; ++i)
        params->args[i] = add_implicit_conversion(ctx, params->instrs, params->args[i], type, loc);
}

static struct hlsl_type *elementwise_intrinsic_get_common_type(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    enum hlsl_base_type base = params->args[0]->data_type->e.numeric.type;
    bool vectors = false, matrices = false;
    unsigned int dimx = 4, dimy = 4;
    struct hlsl_type *common_type;
    unsigned int i;

    for (i = 0; i < params->args_count; ++i)
    {
        struct hlsl_type *arg_type = params->args[i]->data_type;

        base = expr_common_base_type(base, arg_type->e.numeric.type);

        if (arg_type->class == HLSL_CLASS_VECTOR)
        {
            vectors = true;
            dimx = min(dimx, arg_type->e.numeric.dimx);
        }
        else if (arg_type->class == HLSL_CLASS_MATRIX)
        {
            matrices = true;
            dimx = min(dimx, arg_type->e.numeric.dimx);
            dimy = min(dimy, arg_type->e.numeric.dimy);
        }
    }

    if (matrices && vectors)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                "Cannot use both matrices and vectors in an elementwise intrinsic.");
        return NULL;
    }
    else if (matrices)
    {
        common_type = hlsl_get_matrix_type(ctx, base, dimx, dimy);
    }
    else if (vectors)
    {
        common_type = hlsl_get_vector_type(ctx, base, dimx);
    }
    else
    {
        common_type = hlsl_get_scalar_type(ctx, base);
    }

    return common_type;
}

static bool elementwise_intrinsic_convert_args(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_type *common_type;

    if (!(common_type = elementwise_intrinsic_get_common_type(ctx, params, loc)))
        return false;

    convert_args(ctx, params, common_type, loc);
    return true;
}

static bool elementwise_intrinsic_float_convert_args(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_type *type;

    if (!(type = elementwise_intrinsic_get_common_type(ctx, params, loc)))
        return false;
    if (hlsl_type_is_integer(type))
        type = hlsl_change_base_type(ctx, type, HLSL_TYPE_FLOAT);

    convert_args(ctx, params, type, loc);
    return true;
}

static bool elementwise_intrinsic_int_convert_args(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_type *type;

    if (!(type = elementwise_intrinsic_get_common_type(ctx, params, loc)))
        return false;

    type = hlsl_change_base_type(ctx, type, HLSL_TYPE_INT);

    convert_args(ctx, params, type, loc);
    return true;
}

static bool elementwise_intrinsic_uint_convert_args(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_type *type;

    if (!(type = elementwise_intrinsic_get_common_type(ctx, params, loc)))
        return false;

    type = hlsl_change_base_type(ctx, type, HLSL_TYPE_UINT);

    convert_args(ctx, params, type, loc);
    return true;
}

static bool intrinsic_abs(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    const struct hlsl_type *type = params->args[0]->data_type;

    if (!hlsl_type_is_floating_point(type) && !hlsl_type_is_signed_integer(type))
        return true;

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_ABS, params->args[0], loc);
}

static bool write_acos_or_asin(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc, bool asin_mode)
{
    struct hlsl_ir_function_decl *func;
    struct hlsl_ir_node *arg;
    struct hlsl_type *type;
    char *body;

    static const char template[] =
            "%s %s(%s x)\n"
            "{\n"
            "    %s abs_arg = abs(x);\n"
            "    %s poly_approx = (((-0.018729\n"
            "        * abs_arg + 0.074261)\n"
            "        * abs_arg - 0.212114)\n"
            "        * abs_arg + 1.570729);\n"
            "    %s correction = sqrt(1.0 - abs_arg);\n"
            "    %s zero_flip = (x < 0.0) * (-2.0 * correction * poly_approx + 3.141593);\n"
            "    %s result = poly_approx * correction + zero_flip;\n"
            "    return %s;\n"
            "}";
    static const char fn_name_acos[] = "acos";
    static const char fn_name_asin[] = "asin";
    static const char return_stmt_acos[] = "result";
    static const char return_stmt_asin[] = "-result + 1.570796";

    const char *fn_name = asin_mode ? fn_name_asin : fn_name_acos;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);
    type = arg->data_type;

    if (!(body = hlsl_sprintf_alloc(ctx, template,
            type->name, fn_name, type->name,
            type->name, type->name, type->name, type->name, type->name,
            (asin_mode ? return_stmt_asin : return_stmt_acos))))
        return false;
    func = hlsl_compile_internal_function(ctx, fn_name, body);
    vkd3d_free(body);
    if (!func)
        return false;

    return !!add_user_call(ctx, func, params, loc);
}

static bool intrinsic_acos(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return write_acos_or_asin(ctx, params, loc, false);
}

static void add_combine_components(struct hlsl_ctx *ctx, const struct parse_initializer *params,
        struct hlsl_ir_node *arg, enum hlsl_ir_expr_op op, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *res, *load;
    unsigned int i, count;

    count = hlsl_type_component_count(arg->data_type);

    res = hlsl_add_load_component(ctx, params->instrs, arg, 0, loc);

    for (i = 1; i < count; ++i)
    {
        load = hlsl_add_load_component(ctx, params->instrs, arg, i, loc);
        res = hlsl_block_add_binary_expr(ctx, params->instrs, op, res, load);
    }
}

static bool intrinsic_all(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg = params->args[0], *cast;
    struct hlsl_type *bool_type;

    bool_type = hlsl_change_base_type(ctx, arg->data_type, HLSL_TYPE_BOOL);
    cast = add_cast(ctx, params->instrs, arg, bool_type, loc);
    add_combine_components(ctx, params, cast, HLSL_OP2_LOGIC_AND, loc);
    return true;
}

static bool intrinsic_any(struct hlsl_ctx *ctx, const struct parse_initializer *params,
        const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg = params->args[0], *cast;
    struct hlsl_type *bool_type;

    bool_type = hlsl_change_base_type(ctx, arg->data_type, HLSL_TYPE_BOOL);
    cast = add_cast(ctx, params->instrs, arg, bool_type, loc);
    add_combine_components(ctx, params, cast, HLSL_OP2_LOGIC_OR, loc);
    return true;
}

static bool intrinsic_asin(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return write_acos_or_asin(ctx, params, loc, true);
}

static bool write_atan_or_atan2(struct hlsl_ctx *ctx,
        const struct parse_initializer *params,
        const struct vkd3d_shader_location *loc, bool atan2_mode)
{
    struct hlsl_ir_function_decl *func;
    struct hlsl_type *type;
    struct vkd3d_string_buffer *buf;
    int ret;

    static const char atan2_name[] = "atan2";
    static const char atan_name[] = "atan";

    static const char atan2_header_template[] =
            "%s atan2(%s y, %s x)\n"
            "{\n"
            "    %s in_y, in_x;\n"
            "    in_y = y;\n"
            "    in_x = x;\n";
    static const char atan_header_template[] =
            "%s atan(%s y)\n"
            "{\n"
            "    %s in_y, in_x;\n"
            "    in_y = y;\n"
            "    in_x = 1.0;\n";

    static const char body_template[] =
            "    %s recip, input, x2, poly_approx, flipped;"
            "    recip = 1.0 / max(abs(in_y), abs(in_x));\n"
            "    input = recip * min(abs(in_y), abs(in_x));\n"
            "    x2 = input * input;\n"
            "    poly_approx = ((((0.020835\n"
            "        * x2 - 0.085133)\n"
            "        * x2 + 0.180141)\n"
            "        * x2 - 0.330299)\n"
            "        * x2 + 0.999866)\n"
            "        * input;\n"
            "    flipped = poly_approx * -2.0 + 1.570796;\n"
            "    poly_approx += abs(in_x) < abs(in_y) ? flipped : 0.0;\n"
            "    poly_approx += in_x < 0.0 ? -3.1415927 : 0.0;\n"
            "    return (min(in_x, in_y) < 0.0 && max(in_x, in_y) >= 0.0)\n"
            "        ? -poly_approx\n"
            "        : poly_approx;\n"
            "}";

    if (!elementwise_intrinsic_float_convert_args(ctx, params, loc))
        return false;
    type = params->args[0]->data_type;

    if (!(buf = hlsl_get_string_buffer(ctx)))
        return false;

    if (atan2_mode)
        ret = vkd3d_string_buffer_printf(buf, atan2_header_template,
                type->name, type->name, type->name, type->name);
    else
        ret = vkd3d_string_buffer_printf(buf, atan_header_template,
                type->name, type->name, type->name);
    if (ret < 0)
    {
        hlsl_release_string_buffer(ctx, buf);
        return false;
    }

    ret = vkd3d_string_buffer_printf(buf, body_template, type->name);
    if (ret < 0)
    {
        hlsl_release_string_buffer(ctx, buf);
        return false;
    }

    func = hlsl_compile_internal_function(ctx,
            atan2_mode ? atan2_name : atan_name, buf->buffer);
    hlsl_release_string_buffer(ctx, buf);
    if (!func)
        return false;

    return !!add_user_call(ctx, func, params, loc);
}

static bool intrinsic_atan(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return write_atan_or_atan2(ctx, params, loc, false);
}


static bool intrinsic_atan2(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return write_atan_or_atan2(ctx, params, loc, true);
}

static bool intrinsic_asfloat(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *operands[HLSL_MAX_OPERANDS] = {0};
    struct hlsl_type *data_type;

    data_type = params->args[0]->data_type;
    if (data_type->e.numeric.type == HLSL_TYPE_BOOL || data_type->e.numeric.type == HLSL_TYPE_DOUBLE)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, data_type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Wrong argument type of asfloat(): expected 'int', 'uint', 'float', or 'half', but got '%s'.",
                    string->buffer);
        hlsl_release_string_buffer(ctx, string);
    }
    data_type = hlsl_change_base_type(ctx, data_type, HLSL_TYPE_FLOAT);

    operands[0] = params->args[0];
    return add_expr(ctx, params->instrs, HLSL_OP1_REINTERPRET, operands, data_type, loc);
}

static bool intrinsic_asint(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *operands[HLSL_MAX_OPERANDS] = {0};
    struct hlsl_type *data_type;

    data_type = params->args[0]->data_type;
    if (data_type->e.numeric.type == HLSL_TYPE_BOOL || data_type->e.numeric.type == HLSL_TYPE_DOUBLE)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, data_type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Wrong argument type of asint(): expected 'int', 'uint', 'float', or 'half', but got '%s'.",
                    string->buffer);
        hlsl_release_string_buffer(ctx, string);
    }
    data_type = hlsl_change_base_type(ctx, data_type, HLSL_TYPE_INT);

    operands[0] = params->args[0];
    return add_expr(ctx, params->instrs, HLSL_OP1_REINTERPRET, operands, data_type, loc);
}

static bool intrinsic_asuint(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *operands[HLSL_MAX_OPERANDS] = {0};
    struct hlsl_type *data_type;

    if (params->args_count != 1 && params->args_count != 3)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                "Wrong number of arguments to function 'asuint': expected 1 or 3, but got %u.", params->args_count);
        return false;
    }

    if (params->args_count == 3)
    {
        hlsl_fixme(ctx, loc, "Double-to-integer conversion.");
        return false;
    }

    data_type = params->args[0]->data_type;
    if (data_type->e.numeric.type == HLSL_TYPE_BOOL || data_type->e.numeric.type == HLSL_TYPE_DOUBLE)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, data_type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Wrong type for argument 0 of asuint(): expected 'int', 'uint', 'float', or 'half', but got '%s'.",
                    string->buffer);
        hlsl_release_string_buffer(ctx, string);
    }
    data_type = hlsl_change_base_type(ctx, data_type, HLSL_TYPE_UINT);

    operands[0] = params->args[0];
    return add_expr(ctx, params->instrs, HLSL_OP1_REINTERPRET, operands, data_type, loc);
}

static bool intrinsic_ceil(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_CEIL, arg, loc);
}

static bool intrinsic_clamp(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *max;

    if (!elementwise_intrinsic_convert_args(ctx, params, loc))
        return false;

    if (!(max = add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MAX, params->args[0], params->args[1], loc)))
        return false;

    return !!add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MIN, max, params->args[2], loc);
}

static bool intrinsic_clip(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *condition;

    if (!elementwise_intrinsic_float_convert_args(ctx, params, loc))
        return false;

    condition = params->args[0];

    if (ctx->profile->major_version < 4 && hlsl_type_component_count(condition->data_type) > 4)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, condition->data_type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Argument type cannot exceed 4 components, got type \"%s\".", string->buffer);
        hlsl_release_string_buffer(ctx, string);
        return false;
    }

    hlsl_block_add_jump(ctx, params->instrs, HLSL_IR_JUMP_DISCARD_NEG, condition, loc);
    return true;
}

static bool intrinsic_cos(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_COS, arg, loc);
}

static bool write_cosh_or_sinh(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc, bool sinh_mode)
{
    struct hlsl_ir_function_decl *func;
    struct hlsl_ir_node *arg;
    const char *fn_name, *type_name;
    char *body;

    static const char template[] =
            "%s %s(%s x)\n"
            "{\n"
            "    return (exp(x) %s exp(-x)) / 2;\n"
            "}\n";
    static const char fn_name_sinh[] = "sinh";
    static const char fn_name_cosh[] = "cosh";

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    type_name = arg->data_type->name;
    fn_name = sinh_mode ? fn_name_sinh : fn_name_cosh;

    if (!(body = hlsl_sprintf_alloc(ctx, template,
            type_name, fn_name, type_name, sinh_mode ? "-" : "+")))
        return false;

    func = hlsl_compile_internal_function(ctx, fn_name, body);
    vkd3d_free(body);
    if (!func)
        return false;

    return !!add_user_call(ctx, func, params, loc);
}

static bool intrinsic_cosh(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return write_cosh_or_sinh(ctx, params, loc, false);
}

static bool intrinsic_countbits(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *operands[HLSL_MAX_OPERANDS] = {0};
    struct hlsl_type *type;

    if (!elementwise_intrinsic_uint_convert_args(ctx, params, loc))
        return false;
    type = hlsl_change_base_type(ctx, params->args[0]->data_type, HLSL_TYPE_UINT);

    operands[0] = params->args[0];
    return add_expr(ctx, params->instrs, HLSL_OP1_COUNTBITS, operands, type, loc);
}

static bool intrinsic_cross(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg1_swzl1, *arg1_swzl2, *arg2_swzl1, *arg2_swzl2;
    struct hlsl_ir_node *arg1 = params->args[0], *arg2 = params->args[1];
    struct hlsl_ir_node *arg1_cast, *arg2_cast, *mul1_neg, *mul1, *mul2;
    struct hlsl_type *cast_type;
    enum hlsl_base_type base;

    base = expr_common_base_type(arg1->data_type->e.numeric.type, arg2->data_type->e.numeric.type);
    if (hlsl_base_type_is_integer(base))
        base = HLSL_TYPE_FLOAT;

    cast_type = hlsl_get_vector_type(ctx, base, 3);

    arg1_cast = add_implicit_conversion(ctx, params->instrs, arg1, cast_type, loc);
    arg2_cast = add_implicit_conversion(ctx, params->instrs, arg2, cast_type, loc);
    arg1_swzl1 = hlsl_block_add_swizzle(ctx, params->instrs, HLSL_SWIZZLE(Z, X, Y, Z), 3, arg1_cast, loc);
    arg2_swzl1 = hlsl_block_add_swizzle(ctx, params->instrs, HLSL_SWIZZLE(Y, Z, X, Y), 3, arg2_cast, loc);

    if (!(mul1 = add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MUL, arg1_swzl1, arg2_swzl1, loc)))
        return false;

    mul1_neg = hlsl_block_add_unary_expr(ctx, params->instrs, HLSL_OP1_NEG, mul1, loc);

    arg1_swzl2 = hlsl_block_add_swizzle(ctx, params->instrs, HLSL_SWIZZLE(Y, Z, X, Y), 3, arg1_cast, loc);
    arg2_swzl2 = hlsl_block_add_swizzle(ctx, params->instrs, HLSL_SWIZZLE(Z, X, Y, Z), 3, arg2_cast, loc);

    if (!(mul2 = add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MUL, arg1_swzl2, arg2_swzl2, loc)))
        return false;

    return !!add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_ADD, mul2, mul1_neg, loc);
}

static bool intrinsic_ddx(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_DSX, arg, loc);
}

static bool intrinsic_ddx_coarse(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_DSX_COARSE, arg, loc);
}

static bool intrinsic_ddx_fine(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_DSX_FINE, arg, loc);
}

static bool intrinsic_ddy(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_DSY, arg, loc);
}

static bool intrinsic_ddy_coarse(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_DSY_COARSE, arg, loc);
}

static bool intrinsic_degrees(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg, *deg;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    /* 1 rad = 180/pi degree = 57.2957795 degree */
    deg = hlsl_block_add_float_constant(ctx, params->instrs, 57.2957795f, loc);
    return !!add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MUL, arg, deg, loc);
}

static bool intrinsic_ddy_fine(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_DSY_FINE, arg, loc);
}

static bool intrinsic_determinant(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    static const char determinant2x2[] =
            "%s determinant(%s2x2 m)\n"
            "{\n"
            "    return m._11 * m._22 - m._12 * m._21;\n"
            "}";
    static const char determinant3x3[] =
            "%s determinant(%s3x3 m)\n"
            "{\n"
            "    %s2x2 m1 = { m._22, m._23, m._32, m._33 };\n"
            "    %s2x2 m2 = { m._21, m._23, m._31, m._33 };\n"
            "    %s2x2 m3 = { m._21, m._22, m._31, m._32 };\n"
            "    %s3 v1 = { m._11, -m._12, m._13 };\n"
            "    %s3 v2 = { determinant(m1), determinant(m2), determinant(m3) };\n"
            "    return dot(v1, v2);\n"
            "}";
    static const char determinant4x4[] =
            "%s determinant(%s4x4 m)\n"
            "{\n"
            "    %s3x3 m1 = { m._22, m._23, m._24, m._32, m._33, m._34, m._42, m._43, m._44 };\n"
            "    %s3x3 m2 = { m._21, m._23, m._24, m._31, m._33, m._34, m._41, m._43, m._44 };\n"
            "    %s3x3 m3 = { m._21, m._22, m._24, m._31, m._32, m._34, m._41, m._42, m._44 };\n"
            "    %s3x3 m4 = { m._21, m._22, m._23, m._31, m._32, m._33, m._41, m._42, m._43 };\n"
            "    %s4 v1 = { m._11, -m._12, m._13, -m._14 };\n"
            "    %s4 v2 = { determinant(m1), determinant(m2), determinant(m3), determinant(m4) };\n"
            "    return dot(v1, v2);\n"
            "}";
    static const char *templates[] =
    {
        [2] = determinant2x2,
        [3] = determinant3x3,
        [4] = determinant4x4,
    };

    struct hlsl_ir_node *arg = params->args[0];
    const struct hlsl_type *type = arg->data_type;
    struct hlsl_ir_function_decl *func;
    const char *typename, *template;
    unsigned int dim;
    char *body;

    if (type->class != HLSL_CLASS_SCALAR && type->class != HLSL_CLASS_MATRIX)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE, "Invalid argument type.");
        return false;
    }

    arg = intrinsic_float_convert_arg(ctx, params, arg, loc);

    dim = min(type->e.numeric.dimx, type->e.numeric.dimy);
    if (dim == 1)
        return hlsl_add_load_component(ctx, params->instrs, arg, 0, loc);

    typename = hlsl_get_scalar_type(ctx, arg->data_type->e.numeric.type)->name;
    template = templates[dim];

    switch (dim)
    {
        case 2:
            body = hlsl_sprintf_alloc(ctx, template, typename, typename);
            break;
        case 3:
            body = hlsl_sprintf_alloc(ctx, template, typename, typename, typename,
                    typename, typename, typename, typename);
            break;
        case 4:
            body = hlsl_sprintf_alloc(ctx, template, typename, typename, typename,
                    typename, typename, typename, typename, typename);
            break;
        default:
            vkd3d_unreachable();
    }

    if (!body)
        return false;

    func = hlsl_compile_internal_function(ctx, "determinant", body);
    vkd3d_free(body);
    if (!func)
        return false;

    return !!add_user_call(ctx, func, params, loc);
}

static bool intrinsic_distance(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg1, *arg2, *neg, *add, *dot;

    arg1 = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);
    arg2 = intrinsic_float_convert_arg(ctx, params, params->args[1], loc);

    if (!(neg = add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_NEG, arg2, loc)))
        return false;

    if (!(add = add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_ADD, arg1, neg, loc)))
        return false;

    if (!(dot = add_binary_dot_expr(ctx, params->instrs, add, add, loc)))
        return false;

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_SQRT, dot, loc);
}

static bool intrinsic_dot(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return !!add_binary_dot_expr(ctx, params->instrs, params->args[0], params->args[1], loc);
}

static bool intrinsic_dst(struct hlsl_ctx *ctx, const struct parse_initializer *params,
        const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_function_decl *func;
    struct hlsl_type *type, *vec4_type;
    char *body;

    static const char template[] =
            "%s dst(%s i0, %s i1)\n"
            "{\n"
            /* Scalars and vector-4s are both valid inputs, so promote scalars
             * if necessary. */
            "    %s src0 = i0, src1 = i1;\n"
            "    return %s(1, src0.y * src1.y, src0.z, src1.w);\n"
            "}";

    if (!elementwise_intrinsic_convert_args(ctx, params, loc))
        return false;
    type = params->args[0]->data_type;
    if (!(type->class == HLSL_CLASS_SCALAR
            || (type->class == HLSL_CLASS_VECTOR && type->e.numeric.dimx == 4)))
    {
        struct vkd3d_string_buffer *string;
        if ((string = hlsl_type_to_string(ctx, type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Wrong dimension for dst(): expected scalar or 4-dimensional vector, but got %s.",
                    string->buffer);
        hlsl_release_string_buffer(ctx, string);
    }
    vec4_type = hlsl_get_vector_type(ctx, type->e.numeric.type, 4);

    if (!(body = hlsl_sprintf_alloc(ctx, template,
            vec4_type->name, type->name, type->name,
            vec4_type->name,
            vec4_type->name)))
        return false;
    func = hlsl_compile_internal_function(ctx, "dst", body);
    vkd3d_free(body);
    if (!func)
        return false;

    return !!add_user_call(ctx, func, params, loc);
}

static bool intrinsic_exp(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg, *mul, *coeff;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    /* 1/ln(2) */
    coeff = hlsl_block_add_float_constant(ctx, params->instrs, 1.442695f, loc);

    if (!(mul = add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MUL, coeff, arg, loc)))
        return false;

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_EXP2, mul, loc);
}

static bool intrinsic_exp2(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_EXP2, arg, loc);
}

static bool intrinsic_faceforward(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_function_decl *func;
    struct hlsl_type *type;
    char *body;

    static const char template[] =
            "%s faceforward(%s n, %s i, %s ng)\n"
            "{\n"
            "    return dot(i, ng) < 0 ? n : -n;\n"
            "}\n";

    if (!elementwise_intrinsic_float_convert_args(ctx, params, loc))
        return false;
    type = params->args[0]->data_type;

    if (!(body = hlsl_sprintf_alloc(ctx, template,
            type->name, type->name, type->name, type->name)))
        return false;
    func = hlsl_compile_internal_function(ctx, "faceforward", body);
    vkd3d_free(body);
    if (!func)
        return false;

    return !!add_user_call(ctx, func, params, loc);
}

static bool intrinsic_f16tof32(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *operands[HLSL_MAX_OPERANDS] = {0};
    struct hlsl_type *type;

    if (!elementwise_intrinsic_uint_convert_args(ctx, params, loc))
        return false;

    type = hlsl_change_base_type(ctx, params->args[0]->data_type, HLSL_TYPE_FLOAT);

    operands[0] = params->args[0];
    return add_expr(ctx, params->instrs, HLSL_OP1_F16TOF32, operands, type, loc);
}

static bool intrinsic_f32tof16(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *operands[HLSL_MAX_OPERANDS] = {0};
    struct hlsl_type *type;

    if (!elementwise_intrinsic_float_convert_args(ctx, params, loc))
        return false;

    type = hlsl_change_base_type(ctx, params->args[0]->data_type, HLSL_TYPE_UINT);

    operands[0] = params->args[0];
    return add_expr(ctx, params->instrs, HLSL_OP1_F32TOF16, operands, type, loc);
}

static bool intrinsic_firstbithigh(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *operands[HLSL_MAX_OPERANDS] = {0};
    struct hlsl_type *type = params->args[0]->data_type;
    struct hlsl_ir_node *c, *clz, *eq, *xor;

    if (hlsl_version_lt(ctx, 4, 0))
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INCOMPATIBLE_PROFILE,
                "The 'firstbithigh' intrinsic requires shader model 4.0 or higher.");

    if (hlsl_type_is_unsigned_integer(type))
    {
        if (!elementwise_intrinsic_uint_convert_args(ctx, params, loc))
            return false;
    }
    else
    {
        if (!elementwise_intrinsic_int_convert_args(ctx, params, loc))
            return false;
    }
    type = hlsl_change_base_type(ctx, type, HLSL_TYPE_UINT);

    operands[0] = params->args[0];
    if (hlsl_version_lt(ctx, 5, 0))
        return add_expr(ctx, params->instrs, HLSL_OP1_FIND_MSB, operands, type, loc);

    c = hlsl_block_add_uint_constant(ctx, params->instrs, 0x1f, loc);

    if (!(clz = add_expr(ctx, params->instrs, HLSL_OP1_CLZ, operands, type, loc)))
        return false;
    if (!(xor = add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_BIT_XOR, c, clz, loc)))
        return false;

    c = hlsl_block_add_uint_constant(ctx, params->instrs, ~0u, loc);

    if (!(eq = add_binary_comparison_expr(ctx, params->instrs, HLSL_OP2_EQUAL, clz, c, loc)))
        return false;

    operands[0] = eq;
    operands[1] = add_implicit_conversion(ctx, params->instrs, c, type, loc);
    operands[2] = xor;
    return add_expr(ctx, params->instrs, HLSL_OP3_TERNARY, operands, type, loc);
}

static bool intrinsic_firstbitlow(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *operands[HLSL_MAX_OPERANDS] = {0};
    struct hlsl_type *type;

    if (hlsl_version_lt(ctx, 4, 0))
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INCOMPATIBLE_PROFILE,
                "The 'firstbitlow' intrinsic requires shader model 4.0 or higher.");

    if (!elementwise_intrinsic_uint_convert_args(ctx, params, loc))
        return false;
    type = hlsl_change_base_type(ctx, params->args[0]->data_type, HLSL_TYPE_UINT);

    operands[0] = params->args[0];
    return add_expr(ctx, params->instrs, HLSL_OP1_CTZ, operands, type, loc);
}

static bool intrinsic_floor(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_FLOOR, arg, loc);
}

static bool intrinsic_fmod(struct hlsl_ctx *ctx, const struct parse_initializer *params,
        const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *x, *y, *div, *abs, *frac, *neg_frac, *ge, *select, *zero;
    struct hlsl_ir_node *operands[HLSL_MAX_OPERANDS] = { 0 };
    static const struct hlsl_constant_value zero_value;

    x = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);
    y = intrinsic_float_convert_arg(ctx, params, params->args[1], loc);

    if (!(div = add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_DIV, x, y, loc)))
        return false;

    zero = hlsl_block_add_constant(ctx, params->instrs, div->data_type, &zero_value, loc);

    if (!(abs = add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_ABS, div, loc)))
        return false;

    if (!(frac = add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_FRACT, abs, loc)))
        return false;

    if (!(neg_frac = add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_NEG, frac, loc)))
        return false;

    if (!(ge = add_binary_comparison_expr(ctx, params->instrs, HLSL_OP2_GEQUAL, div, zero, loc)))
        return false;

    operands[0] = ge;
    operands[1] = frac;
    operands[2] = neg_frac;
    if (!(select = add_expr(ctx, params->instrs, HLSL_OP3_TERNARY, operands, x->data_type, loc)))
        return false;

    return !!add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MUL, select, y, loc);
}

static bool intrinsic_frac(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_FRACT, arg, loc);
}

static bool intrinsic_frexp(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_type *type, *uint_dim_type, *int_dim_type, *bool_dim_type;
    struct hlsl_ir_function_decl *func;
    char *body;

    static const char template[] =
            "%s frexp(%s x, out %s exp)\n"
            "{\n"
            /* If x is zero, always return zero for exp and mantissa. */
            "    %s is_nonzero_mask = x != 0.0;\n"
            "    %s bits = asuint(x);\n"
            /* Subtract 126, not 127, to increase the exponent */
            "    %s exp_int = asint((bits & 0x7f800000u) >> 23) - 126;\n"
            /* Clear the given exponent and replace it with the bit pattern
             * for 2^-1 */
            "    %s mantissa = asfloat((bits & 0x007fffffu) | 0x3f000000);\n"
            "    exp = is_nonzero_mask * %s(exp_int);\n"
            "    return is_nonzero_mask * mantissa;\n"
            "}\n";

    if (!elementwise_intrinsic_float_convert_args(ctx, params, loc))
        return false;
    type = params->args[0]->data_type;

    if (type->e.numeric.type == HLSL_TYPE_DOUBLE)
    {
        hlsl_fixme(ctx, loc, "frexp() on doubles.");
        return false;
    }
    type = hlsl_change_base_type(ctx, type, HLSL_TYPE_FLOAT);
    uint_dim_type = hlsl_change_base_type(ctx, type, HLSL_TYPE_UINT);
    int_dim_type = hlsl_change_base_type(ctx, type, HLSL_TYPE_INT);
    bool_dim_type = hlsl_change_base_type(ctx, type, HLSL_TYPE_BOOL);

    if (!(body = hlsl_sprintf_alloc(ctx, template, type->name, type->name, type->name,
            bool_dim_type->name, uint_dim_type->name, int_dim_type->name, type->name, type->name)))
        return false;
    func = hlsl_compile_internal_function(ctx, "frexp", body);
    vkd3d_free(body);
    if (!func)
        return false;

    return !!add_user_call(ctx, func, params, loc);
}

static bool intrinsic_fwidth(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_function_decl *func;
    struct hlsl_type *type;
    char *body;

    static const char template[] =
            "%s fwidth(%s x)\n"
            "{\n"
            "    return abs(ddx(x)) + abs(ddy(x));\n"
            "}";

    if (!elementwise_intrinsic_float_convert_args(ctx, params, loc))
        return false;
    type = params->args[0]->data_type;

    if (!(body = hlsl_sprintf_alloc(ctx, template, type->name, type->name)))
        return false;
    func = hlsl_compile_internal_function(ctx, "fwidth", body);
    vkd3d_free(body);
    if (!func)
        return false;

    return !!add_user_call(ctx, func, params, loc);
}

static bool intrinsic_isfinite(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_type *bool_type = hlsl_change_base_type(ctx, params->args[0]->data_type, HLSL_TYPE_BOOL);
    struct hlsl_ir_node *args[HLSL_MAX_OPERANDS] = {0};

    args[0] = params->args[0];
    return !!add_expr(ctx, params->instrs, HLSL_OP1_ISFINITE, args, bool_type, loc);
}

static bool intrinsic_isinf(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_type *bool_type = hlsl_change_base_type(ctx, params->args[0]->data_type, HLSL_TYPE_BOOL);
    struct hlsl_ir_node *args[HLSL_MAX_OPERANDS] = {0};

    args[0] = params->args[0];
    return !!add_expr(ctx, params->instrs, HLSL_OP1_ISINF, args, bool_type, loc);
}

static bool intrinsic_ldexp(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg;

    if (!elementwise_intrinsic_float_convert_args(ctx, params, loc))
        return false;

    if (!(arg = add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_EXP2, params->args[1], loc)))
        return false;

    return !!add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MUL, params->args[0], arg, loc);
}

static bool intrinsic_length(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_type *type = params->args[0]->data_type;
    struct hlsl_ir_node *arg, *dot;

    if (type->class == HLSL_CLASS_MATRIX)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Invalid type %s.", string->buffer);
        hlsl_release_string_buffer(ctx, string);
    }

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    if (!(dot = add_binary_dot_expr(ctx, params->instrs, arg, arg, loc)))
        return false;

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_SQRT, dot, loc);
}

static bool intrinsic_lerp(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *neg, *add, *mul;

    if (!elementwise_intrinsic_float_convert_args(ctx, params, loc))
        return false;

    if (!(neg = add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_NEG, params->args[0], loc)))
        return false;

    if (!(add = add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_ADD, params->args[1], neg, loc)))
        return false;

    if (!(mul = add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MUL, params->args[2], add, loc)))
        return false;

    return !!add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_ADD, params->args[0], mul, loc);
}

static bool intrinsic_lit(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_function_decl *func;

    static const char body[] =
            "float4 lit(float n_l, float n_h, float m)\n"
            "{\n"
            "    float4 ret;\n"
            "    ret.xw = 1.0;\n"
            "    ret.y = max(n_l, 0);\n"
            "    ret.z = (n_l < 0 || n_h < 0) ? 0 : pow(n_h, m);\n"
            "    return ret;\n"
            "}";

    if (params->args[0]->data_type->class != HLSL_CLASS_SCALAR
            || params->args[1]->data_type->class != HLSL_CLASS_SCALAR
            || params->args[2]->data_type->class != HLSL_CLASS_SCALAR)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE, "Invalid argument type.");
        return false;
    }

    if (!(func = hlsl_compile_internal_function(ctx, "lit", body)))
        return false;

    return !!add_user_call(ctx, func, params, loc);
}

static bool intrinsic_log(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *log, *arg, *coeff;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    if (!(log = add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_LOG2, arg, loc)))
        return false;

    /* ln(2) */
    coeff = hlsl_block_add_float_constant(ctx, params->instrs, 0.69314718055f, loc);
    return !!add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MUL, log, coeff, loc);
}

static bool intrinsic_log10(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *log, *arg, *coeff;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    if (!(log = add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_LOG2, arg, loc)))
        return false;

    /* 1 / log2(10) */
    coeff = hlsl_block_add_float_constant(ctx, params->instrs, 0.301029996f, loc);
    return !!add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MUL, log, coeff, loc);
}

static bool intrinsic_log2(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_LOG2, arg, loc);
}

static bool intrinsic_mad(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *args[HLSL_MAX_OPERANDS] = {0};

    if (!elementwise_intrinsic_convert_args(ctx, params, loc))
        return false;

    args[0] = params->args[0];
    args[1] = params->args[1];
    args[2] = params->args[2];
    return add_expr(ctx, params->instrs, HLSL_OP3_MAD, args, args[0]->data_type, loc);
}

static bool intrinsic_max(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    if (!elementwise_intrinsic_convert_args(ctx, params, loc))
        return false;

    return !!add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MAX, params->args[0], params->args[1], loc);
}

static bool intrinsic_min(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    if (!elementwise_intrinsic_convert_args(ctx, params, loc))
        return false;

    return !!add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MIN, params->args[0], params->args[1], loc);
}

static bool intrinsic_modf(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_function_decl *func;
    struct hlsl_type *type;
    char *body;

    static const char template[] =
            "%s modf(%s x, out %s ip)\n"
            "{\n"
            "    ip = trunc(x);\n"
            "    return x - ip;\n"
            "}";

    if (!elementwise_intrinsic_float_convert_args(ctx, params, loc))
        return false;
    type = params->args[0]->data_type;

    if (!(body = hlsl_sprintf_alloc(ctx, template,
            type->name, type->name, type->name)))
        return false;
    func = hlsl_compile_internal_function(ctx, "modf", body);
    vkd3d_free(body);
    if (!func)
        return false;

    return !!add_user_call(ctx, func, params, loc);
}

static bool intrinsic_mul(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg1 = params->args[0], *arg2 = params->args[1], *cast1, *cast2;
    enum hlsl_base_type base = expr_common_base_type(arg1->data_type->e.numeric.type, arg2->data_type->e.numeric.type);
    struct hlsl_type *cast_type1 = arg1->data_type, *cast_type2 = arg2->data_type, *matrix_type, *ret_type;
    unsigned int i, j, k, vect_count = 0;
    struct hlsl_deref var_deref;
    struct hlsl_ir_node *load;
    struct hlsl_ir_var *var;

    if (arg1->data_type->class == HLSL_CLASS_SCALAR || arg2->data_type->class == HLSL_CLASS_SCALAR)
        return !!add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MUL, arg1, arg2, loc);

    if (arg1->data_type->class == HLSL_CLASS_VECTOR)
    {
        vect_count++;
        cast_type1 = hlsl_get_matrix_type(ctx, base, arg1->data_type->e.numeric.dimx, 1);
    }
    if (arg2->data_type->class == HLSL_CLASS_VECTOR)
    {
        vect_count++;
        cast_type2 = hlsl_get_matrix_type(ctx, base, 1, arg2->data_type->e.numeric.dimx);
    }

    matrix_type = hlsl_get_matrix_type(ctx, base, cast_type2->e.numeric.dimx, cast_type1->e.numeric.dimy);

    if (vect_count == 0)
    {
        ret_type = matrix_type;
    }
    else if (vect_count == 1)
    {
        VKD3D_ASSERT(matrix_type->e.numeric.dimx == 1 || matrix_type->e.numeric.dimy == 1);
        ret_type = hlsl_get_vector_type(ctx, base, matrix_type->e.numeric.dimx * matrix_type->e.numeric.dimy);
    }
    else
    {
        VKD3D_ASSERT(matrix_type->e.numeric.dimx == 1 && matrix_type->e.numeric.dimy == 1);
        ret_type = hlsl_get_scalar_type(ctx, base);
    }

    cast1 = add_implicit_conversion(ctx, params->instrs, arg1, cast_type1, loc);
    cast2 = add_implicit_conversion(ctx, params->instrs, arg2, cast_type2, loc);

    if (!(var = hlsl_new_synthetic_var(ctx, "mul", matrix_type, loc)))
        return false;
    hlsl_deref_init_simple(&var_deref, var);

    for (i = 0; i < matrix_type->e.numeric.dimx; ++i)
    {
        for (j = 0; j < matrix_type->e.numeric.dimy; ++j)
        {
            struct hlsl_ir_node *instr = NULL;

            for (k = 0; k < cast_type1->e.numeric.dimx && k < cast_type2->e.numeric.dimy; ++k)
            {
                struct hlsl_ir_node *value1, *value2, *mul;

                value1 = hlsl_add_load_component(ctx, params->instrs,
                        cast1, j * cast1->data_type->e.numeric.dimx + k, loc);
                value2 = hlsl_add_load_component(ctx, params->instrs,
                        cast2, k * cast2->data_type->e.numeric.dimx + i, loc);

                if (!(mul = add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MUL, value1, value2, loc)))
                    return false;

                if (instr)
                {
                    if (!(instr = add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_ADD, instr, mul, loc)))
                        return false;
                }
                else
                {
                    instr = mul;
                }
            }

            hlsl_block_add_store_component(ctx, params->instrs, &var_deref,
                    j * matrix_type->e.numeric.dimx + i, instr);
        }
    }

    load = hlsl_block_add_simple_load(ctx, params->instrs, var, loc);
    add_implicit_conversion(ctx, params->instrs, load, ret_type, loc);
    return true;
}

static bool intrinsic_noise(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_type *type = params->args[0]->data_type, *ret_type;
    struct hlsl_ir_node *args[HLSL_MAX_OPERANDS] = {0};

    type = params->args[0]->data_type;
    if (type->class == HLSL_CLASS_MATRIX)
    {
        struct vkd3d_string_buffer *string;
        if ((string = hlsl_type_to_string(ctx, type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Wrong argument type for noise(): expected vector or scalar, but got '%s'.", string->buffer);
        hlsl_release_string_buffer(ctx, string);
    }

    args[0] = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);
    ret_type = hlsl_get_scalar_type(ctx, args[0]->data_type->e.numeric.type);

    return !!add_expr(ctx, params->instrs, HLSL_OP1_NOISE, args, ret_type, loc);
}

static bool intrinsic_pow(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *log, *mul;

    if (!elementwise_intrinsic_float_convert_args(ctx, params, loc))
        return false;

    if (!(log = add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_LOG2, params->args[0], loc)))
        return NULL;

    if (!(mul = add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MUL, params->args[1], log, loc)))
        return NULL;

    return add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_EXP2, mul, loc);
}

static bool intrinsic_radians(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg, *rad;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    /* 1 degree = pi/180 rad = 0.0174532925f rad */
    rad = hlsl_block_add_float_constant(ctx, params->instrs, 0.0174532925f, loc);
    return !!add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MUL, arg, rad, loc);
}

static bool intrinsic_rcp(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_RCP, arg, loc);
}

static bool intrinsic_reflect(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *i = params->args[0], *n = params->args[1];
    struct hlsl_ir_node *dot, *mul_n, *two_dot, *neg;

    if (!(dot = add_binary_dot_expr(ctx, params->instrs, i, n, loc)))
        return false;

    if (!(two_dot = add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_ADD, dot, dot, loc)))
        return false;

    if (!(mul_n = add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MUL, n, two_dot, loc)))
        return false;

    if (!(neg = add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_NEG, mul_n, loc)))
        return false;

    return !!add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_ADD, i, neg, loc);
}

static bool intrinsic_refract(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_type *type, *scalar_type;
    struct hlsl_ir_function_decl *func;
    char *body;

    static const char template[] =
            "%s refract(%s r, %s n, %s i)\n"
            "{\n"
            "    %s d, t;\n"
            "    d = dot(r, n);\n"
            "    t = 1 - i.x * i.x * (1 - d * d);\n"
            "    return t >= 0.0 ? i.x * r - (i.x * d + sqrt(t)) * n : 0;\n"
            "}";

    if (params->args[0]->data_type->class == HLSL_CLASS_MATRIX
            || params->args[1]->data_type->class == HLSL_CLASS_MATRIX
            || params->args[2]->data_type->class == HLSL_CLASS_MATRIX)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE, "Matrix arguments are not supported.");
        return false;
    }

    /* This is technically not an elementwise intrinsic, but the first two
     * arguments are.
     * The third argument is a scalar, but can be passed as a vector,
     * which should generate an implicit truncation warning.
     * Cast down to scalar explicitly, then we can just use
     * elementwise_intrinsic_float_convert_args().
     * This may result in casting the scalar back to a vector,
     * which we will only use the first component of. */

    scalar_type = hlsl_get_scalar_type(ctx, params->args[2]->data_type->e.numeric.type);
    params->args[2] = add_implicit_conversion(ctx, params->instrs, params->args[2], scalar_type, loc);

    if (!elementwise_intrinsic_float_convert_args(ctx, params, loc))
        return false;
    type = params->args[0]->data_type;

    if (!(body = hlsl_sprintf_alloc(ctx, template, type->name, type->name,
            type->name, type->name, scalar_type->name)))
        return false;

    func = hlsl_compile_internal_function(ctx, "refract", body);
    vkd3d_free(body);
    if (!func)
        return false;

    return !!add_user_call(ctx, func, params, loc);
}

static bool intrinsic_round(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_ROUND, arg, loc);
}

static bool intrinsic_rsqrt(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_RSQ, arg, loc);
}

static bool intrinsic_saturate(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_SAT, arg, loc);
}

static bool intrinsic_sign(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *lt, *neg, *op1, *op2, *zero, *arg = params->args[0];
    static const struct hlsl_constant_value zero_value;

    struct hlsl_type *int_type = hlsl_change_base_type(ctx, arg->data_type, HLSL_TYPE_INT);

    zero = hlsl_block_add_constant(ctx, params->instrs,
            hlsl_get_scalar_type(ctx, arg->data_type->e.numeric.type), &zero_value, loc);

    /* Check if 0 < arg, cast bool to int */

    if (!(lt = add_binary_comparison_expr(ctx, params->instrs, HLSL_OP2_LESS, zero, arg, loc)))
        return false;

    op1 = add_implicit_conversion(ctx, params->instrs, lt, int_type, loc);

    /* Check if arg < 0, cast bool to int and invert (meaning true is -1) */

    if (!(lt = add_binary_comparison_expr(ctx, params->instrs, HLSL_OP2_LESS, arg, zero, loc)))
        return false;

    op2 = add_implicit_conversion(ctx, params->instrs, lt, int_type, loc);

    if (!(neg = add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_NEG, op2, loc)))
        return false;

    /* Adding these two together will make 1 when > 0, -1 when < 0, and 0 when neither */
    return !!add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_ADD, neg, op1, loc);
}

static bool intrinsic_normalize(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_type *type = params->args[0]->data_type;
    struct hlsl_ir_node *dot, *rsq, *arg;

    if (type->class == HLSL_CLASS_MATRIX)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Invalid type %s.", string->buffer);
        hlsl_release_string_buffer(ctx, string);
    }

    if (hlsl_version_lt(ctx, 4, 0) && (type->e.numeric.dimx == 1))
        return intrinsic_sign(ctx, params, loc);

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    if (!(dot = add_binary_dot_expr(ctx, params->instrs, arg, arg, loc)))
        return false;

    if (!(rsq = add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_RSQ, dot, loc)))
        return false;

    return !!add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MUL, rsq, arg, loc);
}

static bool intrinsic_sin(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_SIN, arg, loc);
}

static bool intrinsic_sincos(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_function_decl *func;
    struct hlsl_type *type;
    char *body;

    static const char template[] =
            "void sincos(%s f, out %s s, out %s c)\n"
            "{\n"
            "    s = sin(f);\n"
            "    c = cos(f);\n"
            "}";

    if (!elementwise_intrinsic_float_convert_args(ctx, params, loc))
        return false;
    type = params->args[0]->data_type;

    if (!(body = hlsl_sprintf_alloc(ctx, template,
            type->name, type->name, type->name)))
        return false;
    func = hlsl_compile_internal_function(ctx, "sincos", body);
    vkd3d_free(body);
    if (!func)
        return false;

    return !!add_user_call(ctx, func, params, loc);
}

static bool intrinsic_sinh(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return write_cosh_or_sinh(ctx, params, loc, true);
}

/* smoothstep(a, b, x) = p^2 (3 - 2p), where p = saturate((x - a)/(b - a)) */
static bool intrinsic_smoothstep(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_function_decl *func;
    struct hlsl_type *type;
    char *body;

    static const char template[] =
            "%s smoothstep(%s low, %s high, %s x)\n"
            "{\n"
            "    %s p = saturate((x - low) / (high - low));\n"
            "    return (p * p) * (3 - 2 * p);\n"
            "}";

    if (!elementwise_intrinsic_float_convert_args(ctx, params, loc))
        return false;
    type = params->args[0]->data_type;

    if (!(body = hlsl_sprintf_alloc(ctx, template, type->name, type->name, type->name, type->name, type->name)))
        return false;
    func = hlsl_compile_internal_function(ctx, "smoothstep", body);
    vkd3d_free(body);
    if (!func)
        return false;

    return !!add_user_call(ctx, func, params, loc);
}

static bool intrinsic_sqrt(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg;

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_SQRT, arg, loc);
}

static bool intrinsic_step(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *ge;
    struct hlsl_type *type;

    if (!elementwise_intrinsic_float_convert_args(ctx, params, loc))
        return false;
    type = params->args[0]->data_type;

    if (!(ge = add_binary_comparison_expr(ctx, params->instrs, HLSL_OP2_GEQUAL,
            params->args[1], params->args[0], loc)))
        return false;

    add_implicit_conversion(ctx, params->instrs, ge, type, loc);
    return true;
}

static bool intrinsic_tan(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg = params->args[0], *sin, *cos;

    if (!(sin = add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_SIN, arg, loc)))
        return false;

    if (!(cos = add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_COS, arg, loc)))
        return false;

    return !!add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_DIV, sin, cos, loc);
}

static bool intrinsic_tanh(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_function_decl *func;
    struct hlsl_ir_node *arg;
    struct hlsl_type *type;
    char *body;

    static const char template[] =
            "%s tanh(%s x)\n"
            "{\n"
            "    %s exp_pos, exp_neg;\n"
            "    exp_pos = exp(x);\n"
            "    exp_neg = exp(-x);\n"
            "    return (exp_pos - exp_neg) / (exp_pos + exp_neg);\n"
            "}\n";

    arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);
    type = arg->data_type;

    if (!(body = hlsl_sprintf_alloc(ctx, template,
            type->name, type->name, type->name)))
        return false;

    func = hlsl_compile_internal_function(ctx, "tanh", body);
    vkd3d_free(body);
    if (!func)
        return false;

    return !!add_user_call(ctx, func, params, loc);
}

static bool intrinsic_tex(struct hlsl_ctx *ctx, const struct parse_initializer *params,
        const struct vkd3d_shader_location *loc, const char *name,
        enum hlsl_sampler_dim dim, enum hlsl_resource_load_type type)
{
    struct hlsl_resource_load_params load_params = {.type = type};
    unsigned int sampler_dim = hlsl_sampler_dim_count(dim);
    const struct hlsl_type *sampler_type;
    struct hlsl_ir_node *coords;

    if (params->args_count != 2 && params->args_count != 4)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                "Wrong number of arguments to function '%s': expected 2 or 4, but got %u.", name, params->args_count);
        return false;
    }

    sampler_type = params->args[0]->data_type;
    if (sampler_type->class != HLSL_CLASS_SAMPLER
            || (sampler_type->sampler_dim != dim && sampler_type->sampler_dim != HLSL_SAMPLER_DIM_GENERIC))
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, sampler_type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Wrong type for argument 1 of '%s': expected 'sampler' or '%s', but got '%s'.",
                    name, ctx->builtin_types.sampler[dim]->name, string->buffer);
        hlsl_release_string_buffer(ctx, string);
    }

    if (type == HLSL_RESOURCE_SAMPLE_LOD || type == HLSL_RESOURCE_SAMPLE_LOD_BIAS)
    {
        struct hlsl_ir_node *lod, *c;

        c = hlsl_block_add_swizzle(ctx, params->instrs, HLSL_SWIZZLE(X, Y, Z, W), sampler_dim, params->args[1], loc);
        coords = add_implicit_conversion(ctx, params->instrs, c,
                hlsl_get_vector_type(ctx, HLSL_TYPE_FLOAT, sampler_dim), loc);

        lod = hlsl_block_add_swizzle(ctx, params->instrs, HLSL_SWIZZLE(W, W, W, W), 1, params->args[1], loc);
        load_params.lod = add_implicit_conversion(ctx, params->instrs, lod,
                hlsl_get_scalar_type(ctx, HLSL_TYPE_FLOAT), loc);
    }
    else if (type == HLSL_RESOURCE_SAMPLE_PROJ)
    {
        coords = add_implicit_conversion(ctx, params->instrs, params->args[1],
                hlsl_get_vector_type(ctx, HLSL_TYPE_FLOAT, 4), loc);

        if (hlsl_version_ge(ctx, 4, 0))
        {
            struct hlsl_ir_node *divisor;

            divisor = hlsl_block_add_swizzle(ctx, params->instrs, HLSL_SWIZZLE(W, W, W, W), sampler_dim, coords, loc);
            coords = hlsl_block_add_swizzle(ctx, params->instrs, HLSL_SWIZZLE(X, Y, Z, W), sampler_dim, coords, loc);

            if (!(coords = add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_DIV, coords, divisor, loc)))
                return false;

            load_params.type = HLSL_RESOURCE_SAMPLE;
        }
    }
    else if (type == HLSL_RESOURCE_SAMPLE_GRAD)
    {
        coords = add_implicit_conversion(ctx, params->instrs, params->args[1],
                hlsl_get_vector_type(ctx, HLSL_TYPE_FLOAT, sampler_dim), loc);
        load_params.ddx = add_implicit_conversion(ctx, params->instrs, params->args[2],
                hlsl_get_vector_type(ctx, HLSL_TYPE_FLOAT, sampler_dim), loc);
        load_params.ddy = add_implicit_conversion(ctx, params->instrs, params->args[3],
                hlsl_get_vector_type(ctx, HLSL_TYPE_FLOAT, sampler_dim), loc);
    }
    else
    {
        coords = add_implicit_conversion(ctx, params->instrs, params->args[1],
                hlsl_get_vector_type(ctx, HLSL_TYPE_FLOAT, sampler_dim), loc);
    }

    /* tex1D() functions never produce 1D resource declarations. For newer profiles half offset
       is used for the second coordinate, while older ones appear to replicate first coordinate.*/
    if (dim == HLSL_SAMPLER_DIM_1D)
    {
        struct hlsl_ir_var *var;
        unsigned int idx = 0;

        if (!(var = hlsl_new_synthetic_var(ctx, "coords", hlsl_get_vector_type(ctx, HLSL_TYPE_FLOAT, 2), loc)))
            return false;

        initialize_var_components(ctx, params->instrs, var, &idx, coords, false);
        if (hlsl_version_ge(ctx, 4, 0))
            coords = hlsl_block_add_float_constant(ctx, params->instrs, 0.5f, loc);
        initialize_var_components(ctx, params->instrs, var, &idx, coords, false);

        coords = hlsl_block_add_simple_load(ctx, params->instrs, var, loc);
        dim = HLSL_SAMPLER_DIM_2D;
    }

    load_params.coords = coords;
    load_params.resource = params->args[0];
    load_params.format = hlsl_get_vector_type(ctx, HLSL_TYPE_FLOAT, 4);
    load_params.sampling_dim = dim;

    hlsl_block_add_resource_load(ctx, params->instrs, &load_params, loc);
    return true;
}

static bool intrinsic_tex1D(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_tex(ctx, params, loc, "tex1D", HLSL_SAMPLER_DIM_1D,
            params->args_count == 4 ? HLSL_RESOURCE_SAMPLE_GRAD : HLSL_RESOURCE_SAMPLE);
}

static bool intrinsic_tex1Dgrad(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_tex(ctx, params, loc, "tex1Dgrad", HLSL_SAMPLER_DIM_1D, HLSL_RESOURCE_SAMPLE_GRAD);
}

static bool intrinsic_tex2D(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_tex(ctx, params, loc, "tex2D", HLSL_SAMPLER_DIM_2D,
            params->args_count == 4 ? HLSL_RESOURCE_SAMPLE_GRAD : HLSL_RESOURCE_SAMPLE);
}

static bool intrinsic_tex2Dbias(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_tex(ctx, params, loc, "tex2Dbias", HLSL_SAMPLER_DIM_2D, HLSL_RESOURCE_SAMPLE_LOD_BIAS);
}

static bool intrinsic_tex2Dgrad(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_tex(ctx, params, loc, "tex2Dgrad", HLSL_SAMPLER_DIM_2D, HLSL_RESOURCE_SAMPLE_GRAD);
}

static bool intrinsic_tex2Dlod(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_tex(ctx, params, loc, "tex2Dlod", HLSL_SAMPLER_DIM_2D, HLSL_RESOURCE_SAMPLE_LOD);
}

static bool intrinsic_tex2Dproj(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_tex(ctx, params, loc, "tex2Dproj", HLSL_SAMPLER_DIM_2D, HLSL_RESOURCE_SAMPLE_PROJ);
}

static bool intrinsic_tex3D(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_tex(ctx, params, loc, "tex3D", HLSL_SAMPLER_DIM_3D,
            params->args_count == 4 ? HLSL_RESOURCE_SAMPLE_GRAD : HLSL_RESOURCE_SAMPLE);
}

static bool intrinsic_tex3Dbias(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_tex(ctx, params, loc, "tex3Dbias", HLSL_SAMPLER_DIM_3D, HLSL_RESOURCE_SAMPLE_LOD_BIAS);
}

static bool intrinsic_tex3Dgrad(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_tex(ctx, params, loc, "tex3Dgrad", HLSL_SAMPLER_DIM_3D, HLSL_RESOURCE_SAMPLE_GRAD);
}

static bool intrinsic_tex3Dlod(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_tex(ctx, params, loc, "tex3Dlod", HLSL_SAMPLER_DIM_3D, HLSL_RESOURCE_SAMPLE_LOD);
}

static bool intrinsic_tex3Dproj(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_tex(ctx, params, loc, "tex3Dproj", HLSL_SAMPLER_DIM_3D, HLSL_RESOURCE_SAMPLE_PROJ);
}

static bool intrinsic_texCUBE(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_tex(ctx, params, loc, "texCUBE", HLSL_SAMPLER_DIM_CUBE,
            params->args_count == 4 ? HLSL_RESOURCE_SAMPLE_GRAD : HLSL_RESOURCE_SAMPLE);
}

static bool intrinsic_texCUBEbias(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_tex(ctx, params, loc, "texCUBEbias", HLSL_SAMPLER_DIM_CUBE, HLSL_RESOURCE_SAMPLE_LOD_BIAS);
}

static bool intrinsic_texCUBEgrad(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_tex(ctx, params, loc, "texCUBEgrad", HLSL_SAMPLER_DIM_CUBE, HLSL_RESOURCE_SAMPLE_GRAD);
}

static bool intrinsic_texCUBElod(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_tex(ctx, params, loc, "texCUBElod", HLSL_SAMPLER_DIM_CUBE, HLSL_RESOURCE_SAMPLE_LOD);
}

static bool intrinsic_texCUBEproj(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_tex(ctx, params, loc, "texCUBEproj", HLSL_SAMPLER_DIM_CUBE, HLSL_RESOURCE_SAMPLE_PROJ);
}

static bool intrinsic_transpose(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg = params->args[0];
    struct hlsl_type *arg_type = arg->data_type;
    struct hlsl_deref var_deref;
    struct hlsl_type *mat_type;
    struct hlsl_ir_node *load;
    struct hlsl_ir_var *var;
    unsigned int i, j;

    if (arg_type->class != HLSL_CLASS_SCALAR && arg_type->class != HLSL_CLASS_MATRIX)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, arg_type)))
            hlsl_error(ctx, &arg->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                   "Wrong type for argument 1 of transpose(): expected a matrix or scalar type, but got '%s'.",
                   string->buffer);
        hlsl_release_string_buffer(ctx, string);
        return false;
    }

    if (arg_type->class == HLSL_CLASS_SCALAR)
    {
        hlsl_block_add_instr(params->instrs, arg);
        return true;
    }

    mat_type = hlsl_get_matrix_type(ctx, arg_type->e.numeric.type, arg_type->e.numeric.dimy, arg_type->e.numeric.dimx);

    if (!(var = hlsl_new_synthetic_var(ctx, "transpose", mat_type, loc)))
        return false;
    hlsl_deref_init_simple(&var_deref, var);

    for (i = 0; i < arg_type->e.numeric.dimx; ++i)
    {
        for (j = 0; j < arg_type->e.numeric.dimy; ++j)
        {
            load = hlsl_add_load_component(ctx, params->instrs, arg,
                    j * arg->data_type->e.numeric.dimx + i, loc);
            hlsl_block_add_store_component(ctx, params->instrs, &var_deref,
                    i * var->data_type->e.numeric.dimx + j, load);
        }
    }

    hlsl_block_add_simple_load(ctx, params->instrs, var, loc);
    return true;
}

static bool intrinsic_trunc(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg = intrinsic_float_convert_arg(ctx, params, params->args[0], loc);

    return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_TRUNC, arg, loc);
}

static bool intrinsic_d3dcolor_to_ubyte4(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *arg = params->args[0], *ret, *c;
    struct hlsl_type *arg_type = arg->data_type;

    if (arg_type->class != HLSL_CLASS_SCALAR && !(arg_type->class == HLSL_CLASS_VECTOR
            && arg_type->e.numeric.dimx == 4))
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, arg_type)))
        {
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE, "Wrong argument type '%s'.", string->buffer);
            hlsl_release_string_buffer(ctx, string);
        }

        return false;
    }

    arg = intrinsic_float_convert_arg(ctx, params, arg, loc);
    c = hlsl_block_add_float_constant(ctx, params->instrs, 255.0f + (0.5f / 256.0f), loc);

    if (arg_type->class == HLSL_CLASS_VECTOR)
        arg = hlsl_block_add_swizzle(ctx, params->instrs, HLSL_SWIZZLE(Z, Y, X, W), 4, arg, loc);

    if (!(ret = add_binary_arithmetic_expr(ctx, params->instrs, HLSL_OP2_MUL, arg, c, loc)))
        return false;

    if (hlsl_version_ge(ctx, 4, 0))
        return !!add_unary_arithmetic_expr(ctx, params->instrs, HLSL_OP1_TRUNC, ret, loc);

    return true;
}

static bool intrinsic_GetRenderTargetSampleCount(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *operands[HLSL_MAX_OPERANDS] = {0};

    if (ctx->profile->type != VKD3D_SHADER_TYPE_PIXEL || hlsl_version_lt(ctx, 4, 1))
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INCOMPATIBLE_PROFILE,
                "GetRenderTargetSampleCount() can only be used from a pixel shader using version 4.1 or higher.");

    hlsl_block_add_expr(ctx, params->instrs, HLSL_OP0_RASTERIZER_SAMPLE_COUNT,
            operands, hlsl_get_vector_type(ctx, HLSL_TYPE_UINT, 1), loc);
    return true;
}

static bool intrinsic_interlocked(struct hlsl_ctx *ctx, enum hlsl_interlocked_op op,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc, const char *name)
{
    struct hlsl_ir_node *interlocked, *lhs, *val, *cmp_val = NULL, *orig_val = NULL;
    struct hlsl_type *lhs_type, *val_type, *ret_type = NULL;
    struct vkd3d_string_buffer *string;
    unsigned int writemask, component;
    struct hlsl_deref dst_deref;
    bool matrix_writemask;

    if (hlsl_version_lt(ctx, 5, 0))
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INCOMPATIBLE_PROFILE,
                "Interlocked functions can only be used in shader model 5.0 or higher.");

    if (op != HLSL_INTERLOCKED_CMP_EXCH && op != HLSL_INTERLOCKED_EXCH
            && params->args_count != 2 && params->args_count != 3)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                "Unexpected number of arguments to function '%s': expected 2 or 3, but got %u.",
                        name, params->args_count);
        return false;
    }

    lhs = params->args[0];
    lhs_type = lhs->data_type;

    if (op == HLSL_INTERLOCKED_CMP_EXCH)
    {
        cmp_val = params->args[1];
        val = params->args[2];
        if (params->args_count == 4)
            orig_val = params->args[3];
    }
    else
    {
        val = params->args[1];
        if (params->args_count == 3)
            orig_val = params->args[2];
    }

    if (orig_val)
        ret_type = lhs_type;

    if (lhs_type->class != HLSL_CLASS_SCALAR || (lhs_type->e.numeric.type != HLSL_TYPE_UINT
            && lhs_type->e.numeric.type != HLSL_TYPE_INT))
    {
        if ((string = hlsl_type_to_string(ctx, lhs_type)))
        {
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Unexpected type for argument 0 of '%s': expected 'uint' or 'int', but got '%s'.",
                    name, string->buffer);
            hlsl_release_string_buffer(ctx, string);
        }
        return false;
    }

    /* Interlocked*() functions always take uint for the value parameters,
     * except for InterlockedMax()/InterlockedMin(). */
    if (op == HLSL_INTERLOCKED_MAX || op == HLSL_INTERLOCKED_MIN)
    {
        enum hlsl_base_type val_base_type = val->data_type->e.numeric.type;

        /* Floating values are always cast to signed integers. */
        if (val_base_type == HLSL_TYPE_FLOAT || val_base_type == HLSL_TYPE_HALF || val_base_type == HLSL_TYPE_DOUBLE)
            val_type = hlsl_get_scalar_type(ctx, HLSL_TYPE_INT);
        else if (val_base_type != lhs_type->e.numeric.type)
            val_type = hlsl_get_scalar_type(ctx, HLSL_TYPE_UINT);
        else
            val_type = hlsl_get_scalar_type(ctx, val_base_type);
    }
    else
    {
        val_type = hlsl_get_scalar_type(ctx, HLSL_TYPE_UINT);
    }

    if (cmp_val && !(cmp_val = add_implicit_conversion(ctx, params->instrs, cmp_val, val_type, loc)))
        return false;
    if (!(val = add_implicit_conversion(ctx, params->instrs, val, val_type, loc)))
        return false;

    if (!(lhs = resolve_assignment_lhs(ctx, params->instrs, false, lhs,
            &lhs_type, &val, &writemask, &matrix_writemask)))
        return false;

    VKD3D_ASSERT(writemask);
    /* The writemask should be single component. */
    VKD3D_ASSERT(!(writemask & (writemask - 1)));

    component = vkd3d_log2i(writemask);
    if (matrix_writemask)
    {
        unsigned int i = component / 4, j = component % 4;

        component = i * lhs->data_type->e.numeric.dimx + j;
    }

    if (lhs->type == HLSL_IR_INDEX && hlsl_index_is_noncontiguous(hlsl_ir_index(lhs)))
    {
        struct hlsl_ir_node *c, *cell;

        VKD3D_ASSERT(!matrix_writemask);

        c = hlsl_block_add_uint_constant(ctx, params->instrs, component, &lhs->loc);
        cell = hlsl_block_add_index(ctx, params->instrs, lhs, c, &lhs->loc);

        if (!hlsl_deref_init_from_index_chain(&dst_deref, cell, ctx))
            return false;
    }
    else
    {
        struct hlsl_block component_path_block;
        struct hlsl_deref dst_deref_prefix;

        if (!hlsl_deref_init_from_index_chain(&dst_deref_prefix, lhs, ctx))
            return false;
        if (!hlsl_deref_init_from_component_index(&dst_deref, &component_path_block,
                &dst_deref_prefix, component, &lhs->loc, ctx))
            return false;

        hlsl_block_add_block(params->instrs, &component_path_block);
        hlsl_deref_cleanup(&dst_deref_prefix);
    }

    interlocked = hlsl_new_interlocked(ctx, op, ret_type, &dst_deref, NULL, cmp_val, val, loc);
    hlsl_deref_cleanup(&dst_deref);
    if (!interlocked)
        return false;
    hlsl_block_add_instr(params->instrs, interlocked);

    if (orig_val)
    {
        if (orig_val->data_type->modifiers & HLSL_MODIFIER_CONST)
            hlsl_error(ctx, &orig_val->loc, VKD3D_SHADER_ERROR_HLSL_MODIFIES_CONST,
                    "Output argument to '%s' is const.", name);

        if (!add_assignment(ctx, params->instrs, orig_val, ASSIGN_OP_ASSIGN, interlocked, true))
            return false;
    }

    add_void_expr(ctx, params->instrs, loc);
    return true;
}

static bool intrinsic_InterlockedAdd(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_interlocked(ctx, HLSL_INTERLOCKED_ADD, params, loc, "InterlockedAdd");
}

static bool intrinsic_InterlockedAnd(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_interlocked(ctx, HLSL_INTERLOCKED_AND, params, loc, "InterlockedAnd");
}

static bool intrinsic_InterlockedCompareExchange(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_interlocked(ctx, HLSL_INTERLOCKED_CMP_EXCH, params, loc, "InterlockedCompareExchange");
}

static bool intrinsic_InterlockedCompareStore(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_interlocked(ctx, HLSL_INTERLOCKED_CMP_EXCH, params, loc, "InterlockedCompareStore");
}

static bool intrinsic_InterlockedExchange(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_interlocked(ctx, HLSL_INTERLOCKED_EXCH, params, loc, "InterlockedExchange");
}

static bool intrinsic_InterlockedMax(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_interlocked(ctx, HLSL_INTERLOCKED_MAX, params, loc, "InterlockedMax");
}

static bool intrinsic_InterlockedMin(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_interlocked(ctx, HLSL_INTERLOCKED_MIN, params, loc, "InterlockedMin");
}

static bool intrinsic_InterlockedOr(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_interlocked(ctx, HLSL_INTERLOCKED_OR, params, loc, "InterlockedOr");
}

static bool intrinsic_InterlockedXor(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return intrinsic_interlocked(ctx, HLSL_INTERLOCKED_XOR, params, loc, "InterlockedXor");
}

static bool intrinsic_AllMemoryBarrier(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return !!hlsl_block_add_sync(ctx, params->instrs, VKD3DSSF_GLOBAL_UAV
            | VKD3DSSF_GROUP_SHARED_MEMORY, loc);
}

static bool intrinsic_AllMemoryBarrierWithGroupSync(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return !!hlsl_block_add_sync(ctx, params->instrs, VKD3DSSF_GLOBAL_UAV
            | VKD3DSSF_GROUP_SHARED_MEMORY | VKD3DSSF_THREAD_GROUP, loc);
}

static bool intrinsic_ConstructGSWithSO(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    const char *strings[HLSL_STREAM_OUTPUT_MAX] = {0};
    size_t string_count = params->args_count - 1;
    struct hlsl_ir_compile *compile;
    struct hlsl_ir_node *node;
    uint32_t stream_index = 0;
    struct hlsl_ir_var *var;

    if (params->args_count != 2 && params->args_count != 6)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                "Unexpected number of arguments to ConstructGSWithSO(): expected 2 or 6, but got %u.",
                params->args_count);
        return false;
    }

    if (ctx->profile->type != VKD3D_SHADER_TYPE_EFFECT)
    {
        if (!(node = hlsl_new_compile_with_so(ctx, NULL, 0, 0, NULL, loc)))
            return false;
        hlsl_block_add_instr(params->instrs, node);
        return true;
    }

    node = params->args[0];
    switch (node->type)
    {
        case HLSL_IR_COMPILE:
            compile = hlsl_ir_compile(node);
            break;

        case HLSL_IR_LOAD:
            var = hlsl_ir_load(node)->src.var;
            if (var->data_type->class == HLSL_CLASS_ARRAY || !hlsl_type_is_shader(var->data_type))
            {
                hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                        "\"%s\" is not a shader compilation.", var->name);
                return false;
            }

            if (!(compile = var->default_values[0].shader))
            {
                hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                        "Stream output shaders can't be constructed with NULL shaders.");
                return false;
            }
            break;

        case HLSL_IR_INDEX:
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Stream output shaders can't be constructed with array indexes.");
            return false;

        default:
            hlsl_fixme(ctx, loc, "Unhandled node type in ConstructGSWithSO().");
            return false;
    }

    if (compile->profile->type != VKD3D_SHADER_TYPE_VERTEX && compile->profile->type != VKD3D_SHADER_TYPE_GEOMETRY)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                "Stream output shaders can only be constructed with vertex or geometry shaders.");
        return false;
    }

    if (params->args_count == 6)
    {
        struct hlsl_block stream_index_block;

        --string_count;

        if (!(node = hlsl_clone_instr(ctx, params->args[5])))
            return false;

        hlsl_block_init(&stream_index_block);
        hlsl_block_add_instr(&stream_index_block, node);

        stream_index = evaluate_static_expression_as_uint(ctx, &stream_index_block, loc);

        hlsl_block_cleanup(&stream_index_block);
    }

    for (size_t i = 0; i < string_count; ++i)
    {
        struct hlsl_ir_node *stream_node = params->args[i + 1];

        switch (stream_node->type)
        {
            case HLSL_IR_STRING_CONSTANT:
                strings[i] = hlsl_ir_string_constant(stream_node)->string;
                break;

            case HLSL_IR_CONSTANT:
                if (stream_node->data_type->class == HLSL_CLASS_NULL)
                    continue;
            /* fall-through */
            default:
                hlsl_error(ctx, &stream_node->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                        "Stream Output declarations must be a literal string.");
                return false;
        }
    }

    VKD3D_ASSERT(string_count <= ARRAY_SIZE(strings));

    if (!(node = hlsl_new_compile_with_so(ctx, compile, stream_index, string_count, strings, loc)))
        return false;
    hlsl_block_add_instr(params->instrs, node);

    return true;
}

static bool intrinsic_DeviceMemoryBarrier(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return !!hlsl_block_add_sync(ctx, params->instrs, VKD3DSSF_GLOBAL_UAV, loc);
}

static bool intrinsic_DeviceMemoryBarrierWithGroupSync(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return !!hlsl_block_add_sync(ctx, params->instrs, VKD3DSSF_GLOBAL_UAV
            | VKD3DSSF_THREAD_GROUP, loc);
}

static bool intrinsic_GroupMemoryBarrier(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return !!hlsl_block_add_sync(ctx, params->instrs,
            VKD3DSSF_GROUP_SHARED_MEMORY, loc);
}

static bool intrinsic_GroupMemoryBarrierWithGroupSync(struct hlsl_ctx *ctx,
        const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    return !!hlsl_block_add_sync(ctx, params->instrs,
            VKD3DSSF_GROUP_SHARED_MEMORY | VKD3DSSF_THREAD_GROUP, loc);
}

static const struct intrinsic_function
{
    const char *name;
    int param_count;
    bool check_numeric;
    bool (*handler)(struct hlsl_ctx *ctx, const struct parse_initializer *params,
            const struct vkd3d_shader_location *loc);
}
intrinsic_functions[] =
{
    /* Note: these entries should be kept in alphabetical order. */
    {"AllMemoryBarrier",                    0, true,  intrinsic_AllMemoryBarrier},
    {"AllMemoryBarrierWithGroupSync",       0, true,  intrinsic_AllMemoryBarrierWithGroupSync},
    {"ConstructGSWithSO",                  -1, false, intrinsic_ConstructGSWithSO},
    {"D3DCOLORtoUBYTE4",                    1, true,  intrinsic_d3dcolor_to_ubyte4},
    {"DeviceMemoryBarrier",                 0, true,  intrinsic_DeviceMemoryBarrier},
    {"DeviceMemoryBarrierWithGroupSync",    0, true,  intrinsic_DeviceMemoryBarrierWithGroupSync},
    {"GetRenderTargetSampleCount",          0, true,  intrinsic_GetRenderTargetSampleCount},
    {"GroupMemoryBarrier",                  0, true,  intrinsic_GroupMemoryBarrier},
    {"GroupMemoryBarrierWithGroupSync",     0, true,  intrinsic_GroupMemoryBarrierWithGroupSync},
    {"InterlockedAdd",                     -1, true,  intrinsic_InterlockedAdd},
    {"InterlockedAnd",                     -1, true,  intrinsic_InterlockedAnd},
    {"InterlockedCompareExchange",          4, true,  intrinsic_InterlockedCompareExchange},
    {"InterlockedCompareStore",             3, true,  intrinsic_InterlockedCompareStore},
    {"InterlockedExchange",                 3, true,  intrinsic_InterlockedExchange},
    {"InterlockedMax",                     -1, true,  intrinsic_InterlockedMax},
    {"InterlockedMin",                     -1, true,  intrinsic_InterlockedMin},
    {"InterlockedOr",                      -1, true,  intrinsic_InterlockedOr},
    {"InterlockedXor",                     -1, true,  intrinsic_InterlockedXor},
    {"abs",                                 1, true,  intrinsic_abs},
    {"acos",                                1, true,  intrinsic_acos},
    {"all",                                 1, true,  intrinsic_all},
    {"any",                                 1, true,  intrinsic_any},
    {"asfloat",                             1, true,  intrinsic_asfloat},
    {"asin",                                1, true,  intrinsic_asin},
    {"asint",                               1, true,  intrinsic_asint},
    {"asuint",                             -1, true,  intrinsic_asuint},
    {"atan",                                1, true,  intrinsic_atan},
    {"atan2",                               2, true,  intrinsic_atan2},
    {"ceil",                                1, true,  intrinsic_ceil},
    {"clamp",                               3, true,  intrinsic_clamp},
    {"clip",                                1, true,  intrinsic_clip},
    {"cos",                                 1, true,  intrinsic_cos},
    {"cosh",                                1, true,  intrinsic_cosh},
    {"countbits",                           1, true,  intrinsic_countbits},
    {"cross",                               2, true,  intrinsic_cross},
    {"ddx",                                 1, true,  intrinsic_ddx},
    {"ddx_coarse",                          1, true,  intrinsic_ddx_coarse},
    {"ddx_fine",                            1, true,  intrinsic_ddx_fine},
    {"ddy",                                 1, true,  intrinsic_ddy},
    {"ddy_coarse",                          1, true,  intrinsic_ddy_coarse},
    {"ddy_fine",                            1, true,  intrinsic_ddy_fine},
    {"degrees",                             1, true,  intrinsic_degrees},
    {"determinant",                         1, true,  intrinsic_determinant},
    {"distance",                            2, true,  intrinsic_distance},
    {"dot",                                 2, true,  intrinsic_dot},
    {"dst",                                 2, true,  intrinsic_dst},
    {"exp",                                 1, true,  intrinsic_exp},
    {"exp2",                                1, true,  intrinsic_exp2},
    {"f16tof32",                            1, true,  intrinsic_f16tof32},
    {"f32tof16",                            1, true,  intrinsic_f32tof16},
    {"faceforward",                         3, true,  intrinsic_faceforward},
    {"firstbithigh",                        1, true,  intrinsic_firstbithigh},
    {"firstbitlow",                         1, true,  intrinsic_firstbitlow},
    {"floor",                               1, true,  intrinsic_floor},
    {"fmod",                                2, true,  intrinsic_fmod},
    {"frac",                                1, true,  intrinsic_frac},
    {"frexp",                               2, true,  intrinsic_frexp},
    {"fwidth",                              1, true,  intrinsic_fwidth},
    {"isfinite",                            1, true,  intrinsic_isfinite},
    {"isinf",                               1, true,  intrinsic_isinf},
    {"ldexp",                               2, true,  intrinsic_ldexp},
    {"length",                              1, true,  intrinsic_length},
    {"lerp",                                3, true,  intrinsic_lerp},
    {"lit",                                 3, true,  intrinsic_lit},
    {"log",                                 1, true,  intrinsic_log},
    {"log10",                               1, true,  intrinsic_log10},
    {"log2",                                1, true,  intrinsic_log2},
    {"mad",                                 3, true,  intrinsic_mad},
    {"max",                                 2, true,  intrinsic_max},
    {"min",                                 2, true,  intrinsic_min},
    {"modf",                                2, true,  intrinsic_modf},
    {"mul",                                 2, true,  intrinsic_mul},
    {"noise",                               1, true,  intrinsic_noise},
    {"normalize",                           1, true,  intrinsic_normalize},
    {"pow",                                 2, true,  intrinsic_pow},
    {"radians",                             1, true,  intrinsic_radians},
    {"rcp",                                 1, true,  intrinsic_rcp},
    {"reflect",                             2, true,  intrinsic_reflect},
    {"refract",                             3, true,  intrinsic_refract},
    {"round",                               1, true,  intrinsic_round},
    {"rsqrt",                               1, true,  intrinsic_rsqrt},
    {"saturate",                            1, true,  intrinsic_saturate},
    {"sign",                                1, true,  intrinsic_sign},
    {"sin",                                 1, true,  intrinsic_sin},
    {"sincos",                              3, true,  intrinsic_sincos},
    {"sinh",                                1, true,  intrinsic_sinh},
    {"smoothstep",                          3, true,  intrinsic_smoothstep},
    {"sqrt",                                1, true,  intrinsic_sqrt},
    {"step",                                2, true,  intrinsic_step},
    {"tan",                                 1, true,  intrinsic_tan},
    {"tanh",                                1, true,  intrinsic_tanh},
    {"tex1D",                              -1, false, intrinsic_tex1D},
    {"tex1Dgrad",                           4, false, intrinsic_tex1Dgrad},
    {"tex2D",                              -1, false, intrinsic_tex2D},
    {"tex2Dbias",                           2, false, intrinsic_tex2Dbias},
    {"tex2Dgrad",                           4, false, intrinsic_tex2Dgrad},
    {"tex2Dlod",                            2, false, intrinsic_tex2Dlod},
    {"tex2Dproj",                           2, false, intrinsic_tex2Dproj},
    {"tex3D",                              -1, false, intrinsic_tex3D},
    {"tex3Dbias",                           2, false, intrinsic_tex3Dbias},
    {"tex3Dgrad",                           4, false, intrinsic_tex3Dgrad},
    {"tex3Dlod",                            2, false, intrinsic_tex3Dlod},
    {"tex3Dproj",                           2, false, intrinsic_tex3Dproj},
    {"texCUBE",                            -1, false, intrinsic_texCUBE},
    {"texCUBEbias",                         2, false, intrinsic_texCUBEbias},
    {"texCUBEgrad",                         4, false, intrinsic_texCUBEgrad},
    {"texCUBElod",                          2, false, intrinsic_texCUBElod},
    {"texCUBEproj",                         2, false, intrinsic_texCUBEproj},
    {"transpose",                           1, true,  intrinsic_transpose},
    {"trunc",                               1, true,  intrinsic_trunc},
};

static int intrinsic_function_name_compare(const void *a, const void *b)
{
    const struct intrinsic_function *func = b;

    return strcmp(a, func->name);
}

static struct hlsl_block *add_call(struct hlsl_ctx *ctx, const struct hlsl_identifier *identifier,
        struct parse_initializer *args, const struct vkd3d_shader_location *loc)
{
    const struct intrinsic_function *intrinsic;
    const char *name = identifier->name;
    struct hlsl_ir_function_decl *decl;

    if (hlsl_identifier_is_qualified(identifier))
    {
        hlsl_fixme(ctx, loc, "Qualified function calls.");
        goto fail;
    }

    for (unsigned int i = 0; i < args->args_count; ++i)
    {
        if (args->args[i]->data_type->class == HLSL_CLASS_ERROR)
            goto fail;
    }

    if ((decl = find_function_call(ctx, name, args, false, loc)))
    {
        if (!add_user_call(ctx, decl, args, loc))
            goto fail;
    }
    else if ((intrinsic = bsearch(name, intrinsic_functions, ARRAY_SIZE(intrinsic_functions),
            sizeof(*intrinsic_functions), intrinsic_function_name_compare)))
    {
        if (intrinsic->param_count >= 0 && args->args_count != intrinsic->param_count)
        {
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                    "Wrong number of arguments to function '%s': expected %u, but got %u.",
                    name, intrinsic->param_count, args->args_count);
            goto fail;
        }

        if (intrinsic->check_numeric)
        {
            unsigned int i;

            for (i = 0; i < args->args_count; ++i)
            {
                if (!hlsl_is_numeric_type(args->args[i]->data_type))
                {
                    struct vkd3d_string_buffer *string;

                    if ((string = hlsl_type_to_string(ctx, args->args[i]->data_type)))
                        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                                "Wrong type for argument %u of '%s': expected a numeric type, but got '%s'.",
                                i + 1, name, string->buffer);
                    hlsl_release_string_buffer(ctx, string);
                    goto fail;
                }
            }
        }

        if (!intrinsic->handler(ctx, args, loc))
            goto fail;
    }
    else if (rb_get(&ctx->functions, name))
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_NOT_DEFINED, "No compatible %u parameter declaration for \"%s\" found.",
                args->args_count, name);
        goto fail;
    }
    else
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_NOT_DEFINED, "Function \"%s\" is not defined.", name);
        goto fail;
    }
    vkd3d_free(args->args);
    return args->instrs;

fail:
    args->instrs->value = ctx->error_instr;
    vkd3d_free(args->args);
    return args->instrs;
}

static struct hlsl_block *add_shader_compilation(struct hlsl_ctx *ctx, const char *profile_name,
        const char *function_name, struct parse_initializer *args, const struct vkd3d_shader_location *loc)
{
    const struct hlsl_profile_info *profile_info;
    struct hlsl_ir_function_decl *decl;
    struct hlsl_block *block = NULL;
    struct hlsl_ir_node *compile;

    if (!ctx->in_state_block && ctx->cur_scope != ctx->globals)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_MISPLACED_COMPILE,
                "Shader compilation statements must be in global scope or a state block.");
        goto out;
    }

    if (!(decl = find_function_call(ctx, function_name, args, true, loc)))
    {
        if (rb_get(&ctx->functions, function_name))
        {
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_NOT_DEFINED,
                    "No compatible \"%s\" declaration with %u uniform parameters found.",
                    function_name, args->args_count);
        }
        else
        {
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_NOT_DEFINED,
                    "Function \"%s\" is not defined.", function_name);
        }
        goto out;
    }

    if (!(profile_info = hlsl_get_target_info(profile_name)))
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_PROFILE, "Unknown profile \"%s\".", profile_name);
        goto out;
    }

    for (unsigned int i = 0; i < args->args_count; ++i)
    {
        if (args->args[i]->data_type->class == HLSL_CLASS_ERROR)
        {
            args->instrs->value = ctx->error_instr;
            free(args->args);
            return args->instrs;
        }
    }

    if (!parse_function_call_arguments(ctx, decl, args, true, loc))
        goto out;

    if (!(compile = hlsl_new_compile(ctx, profile_info, decl, args->instrs, loc)))
        goto out;

    block = make_block(ctx, compile);

out:
    cleanup_parse_initializer(args);
    return block;
}

static struct hlsl_block *add_constructor(struct hlsl_ctx *ctx, struct hlsl_type *type,
        struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_var *var;

    if (!hlsl_is_numeric_type(type))
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Constructor data type %s is not numeric.", string->buffer);
        hlsl_release_string_buffer(ctx, string);
        return NULL;
    }

    if (!(var = hlsl_new_synthetic_var(ctx, "constructor", type, loc)))
        return NULL;

    initialize_var(ctx, var, params, false);

    hlsl_block_add_simple_load(ctx, params->instrs, var, loc);

    vkd3d_free(params->args);
    return params->instrs;
}

static bool add_ternary(struct hlsl_ctx *ctx, struct hlsl_block *block,
        struct hlsl_ir_node *cond, struct hlsl_ir_node *first, struct hlsl_ir_node *second)
{
    struct hlsl_ir_node *args[HLSL_MAX_OPERANDS] = {0};
    struct hlsl_type *cond_type = cond->data_type;
    struct hlsl_type *common_type;

    if (cond->data_type->class == HLSL_CLASS_ERROR
            || first->data_type->class == HLSL_CLASS_ERROR
            || second->data_type->class == HLSL_CLASS_ERROR)
    {
        block->value = ctx->error_instr;
        return true;
    }

    if (cond_type->class > HLSL_CLASS_LAST_NUMERIC)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, cond_type)))
            hlsl_error(ctx, &cond->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Ternary condition type '%s' is not numeric.", string->buffer);
        hlsl_release_string_buffer(ctx, string);
        return false;
    }

    if (first->data_type->class <= HLSL_CLASS_LAST_NUMERIC
            && second->data_type->class <= HLSL_CLASS_LAST_NUMERIC)
    {
        if (!(common_type = get_common_numeric_type(ctx, first, second, &first->loc)))
            return false;

        if (cond_type->e.numeric.dimx == 1 && cond_type->e.numeric.dimy == 1)
        {
            cond_type = hlsl_change_base_type(ctx, common_type, HLSL_TYPE_BOOL);
            cond = add_implicit_conversion(ctx, block, cond, cond_type, &cond->loc);
        }
        else
        {
            if (common_type->e.numeric.dimx == 1 && common_type->e.numeric.dimy == 1)
            {
                common_type = hlsl_change_base_type(ctx, cond_type, common_type->e.numeric.type);
            }
            else if (cond_type->e.numeric.dimx != common_type->e.numeric.dimx
                    || cond_type->e.numeric.dimy != common_type->e.numeric.dimy)
            {
                /* This condition looks wrong but is correct.
                * floatN is compatible with float1xN, but not with floatNx1. */

                struct vkd3d_string_buffer *cond_string, *value_string;

                cond_string = hlsl_type_to_string(ctx, cond_type);
                value_string = hlsl_type_to_string(ctx, common_type);
                if (cond_string && value_string)
                    hlsl_error(ctx, &first->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                            "Ternary condition type '%s' is not compatible with value type '%s'.",
                            cond_string->buffer, value_string->buffer);
                hlsl_release_string_buffer(ctx, cond_string);
                hlsl_release_string_buffer(ctx, value_string);
            }

            cond_type = hlsl_change_base_type(ctx, common_type, HLSL_TYPE_BOOL);
            cond = add_implicit_conversion(ctx, block, cond, cond_type, &cond->loc);
        }

        first = add_implicit_conversion(ctx, block, first, common_type, &first->loc);
        second = add_implicit_conversion(ctx, block, second, common_type, &second->loc);
    }
    else
    {
        struct vkd3d_string_buffer *first_string, *second_string;

        if (!hlsl_types_are_equal(first->data_type, second->data_type))
        {
            first_string = hlsl_type_to_string(ctx, first->data_type);
            second_string = hlsl_type_to_string(ctx, second->data_type);
            if (first_string && second_string)
                hlsl_error(ctx, &first->loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                        "Ternary argument types '%s' and '%s' do not match.",
                        first_string->buffer, second_string->buffer);
            hlsl_release_string_buffer(ctx, first_string);
            hlsl_release_string_buffer(ctx, second_string);
        }

        cond_type = hlsl_change_base_type(ctx, cond_type, HLSL_TYPE_BOOL);
        cond = add_implicit_conversion(ctx, block, cond, cond_type, &cond->loc);
        common_type = first->data_type;
    }

    VKD3D_ASSERT(cond->data_type->e.numeric.type == HLSL_TYPE_BOOL);

    args[0] = cond;
    args[1] = first;
    args[2] = second;
    return add_expr(ctx, block, HLSL_OP3_TERNARY, args, common_type, &first->loc);
}

static unsigned int hlsl_offset_dim_count(enum hlsl_sampler_dim dim)
{
    switch (dim)
    {
        case HLSL_SAMPLER_DIM_1D:
        case HLSL_SAMPLER_DIM_1DARRAY:
            return 1;
        case HLSL_SAMPLER_DIM_2D:
        case HLSL_SAMPLER_DIM_2DMS:
        case HLSL_SAMPLER_DIM_2DARRAY:
        case HLSL_SAMPLER_DIM_2DMSARRAY:
            return 2;
        case HLSL_SAMPLER_DIM_3D:
            return 3;
        case HLSL_SAMPLER_DIM_CUBE:
        case HLSL_SAMPLER_DIM_CUBEARRAY:
        case HLSL_SAMPLER_DIM_BUFFER:
        case HLSL_SAMPLER_DIM_RAW_BUFFER:
        case HLSL_SAMPLER_DIM_STRUCTURED_BUFFER:
            /* Offset parameters not supported for these types. */
            return 0;
        default:
            vkd3d_unreachable();
    }
}

static bool raise_invalid_method_object_type(struct hlsl_ctx *ctx, const struct hlsl_type *object_type,
        const char *method, const struct vkd3d_shader_location *loc)
{
    struct vkd3d_string_buffer *string;

    if ((string = hlsl_type_to_string(ctx, object_type)))
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_NOT_DEFINED,
                "Method '%s' is not defined on type '%s'.", method, string->buffer);
    hlsl_release_string_buffer(ctx, string);
    return false;
}

static bool add_raw_load_method_call(struct hlsl_ctx *ctx, struct hlsl_block *block, struct hlsl_ir_node *object,
        const char *name, const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_resource_load_params load_params = {.type = HLSL_RESOURCE_LOAD};
    unsigned int value_dim;

    if (params->args_count != 1 && params->args_count != 2)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                "Wrong number of arguments to method 'Load': expected between 1 and 2, but got %u.",
                params->args_count);
        return false;
    }

    if (params->args_count == 2)
    {
        hlsl_fixme(ctx, loc, "Tiled resource status argument.");
        return false;
    }

    if (params->args[0]->data_type->class != HLSL_CLASS_SCALAR)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE, "Scalar address argument expected for '%s'.", name);
        return false;
    }

    if (!strcmp(name, "Load"))
        value_dim = 1;
    else if (!strcmp(name, "Load2"))
        value_dim = 2;
    else if (!strcmp(name, "Load3"))
        value_dim = 3;
    else
        value_dim = 4;

    load_params.coords = add_implicit_conversion(ctx, block, params->args[0],
            hlsl_get_scalar_type(ctx, HLSL_TYPE_UINT), loc);
    load_params.format = hlsl_get_vector_type(ctx, HLSL_TYPE_UINT, value_dim);
    load_params.resource = object;
    hlsl_block_add_resource_load(ctx, block, &load_params, loc);
    return true;
}

static bool add_load_method_call(struct hlsl_ctx *ctx, struct hlsl_block *block, struct hlsl_ir_node *object,
        const char *name, const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    const struct hlsl_type *object_type = object->data_type;
    struct hlsl_resource_load_params load_params = {.type = HLSL_RESOURCE_LOAD};
    unsigned int sampler_dim, offset_dim;
    bool multisampled;

    if (object_type->sampler_dim == HLSL_SAMPLER_DIM_RAW_BUFFER)
        return add_raw_load_method_call(ctx, block, object, name, params, loc);

    if (object_type->sampler_dim == HLSL_SAMPLER_DIM_STRUCTURED_BUFFER)
    {
        hlsl_fixme(ctx, loc, "Method '%s' for structured buffers.", name);
        return false;
    }

    sampler_dim = hlsl_sampler_dim_count(object_type->sampler_dim);
    offset_dim = hlsl_offset_dim_count(object_type->sampler_dim);

    multisampled = object_type->sampler_dim == HLSL_SAMPLER_DIM_2DMS
            || object_type->sampler_dim == HLSL_SAMPLER_DIM_2DMSARRAY;

    if (params->args_count < 1 + multisampled || params->args_count > 2 + multisampled + !!offset_dim)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                "Wrong number of arguments to method 'Load': expected between %u and %u, but got %u.",
                1 + multisampled, 2 + multisampled + !!offset_dim, params->args_count);
        return false;
    }

    if (multisampled)
        load_params.sample_index = add_implicit_conversion(ctx, block, params->args[1],
                hlsl_get_scalar_type(ctx, HLSL_TYPE_INT), loc);

    if (!!offset_dim && params->args_count > 1 + multisampled)
        load_params.texel_offset = add_implicit_conversion(ctx, block, params->args[1 + multisampled],
                hlsl_get_vector_type(ctx, HLSL_TYPE_INT, offset_dim), loc);

    if (params->args_count > 1 + multisampled + !!offset_dim)
    {
        hlsl_fixme(ctx, loc, "Tiled resource status argument.");
    }

    /* +1 for the mipmap level for non-multisampled textures */
    load_params.coords = add_implicit_conversion(ctx, block, params->args[0],
            hlsl_get_vector_type(ctx, HLSL_TYPE_INT, sampler_dim + !multisampled), loc);
    load_params.format = object_type->e.resource.format;
    load_params.resource = object;
    hlsl_block_add_resource_load(ctx, block, &load_params, loc);
    return true;
}

static bool add_sample_method_call(struct hlsl_ctx *ctx, struct hlsl_block *block, struct hlsl_ir_node *object,
        const char *name, const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    const struct hlsl_type *object_type = object->data_type;
    struct hlsl_resource_load_params load_params = {.type = HLSL_RESOURCE_SAMPLE};
    unsigned int sampler_dim, offset_dim;
    const struct hlsl_type *sampler_type;

    sampler_dim = hlsl_sampler_dim_count(object_type->sampler_dim);
    offset_dim = hlsl_offset_dim_count(object_type->sampler_dim);

    if (params->args_count < 2 || params->args_count > 4 + !!offset_dim)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                "Wrong number of arguments to method 'Sample': expected from 2 to %u, but got %u.",
                4 + !!offset_dim, params->args_count);
        return false;
    }

    sampler_type = params->args[0]->data_type;
    if (sampler_type->class != HLSL_CLASS_SAMPLER || sampler_type->sampler_dim != HLSL_SAMPLER_DIM_GENERIC)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, sampler_type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Wrong type for argument 0 of Sample(): expected 'sampler', but got '%s'.", string->buffer);
        hlsl_release_string_buffer(ctx, string);
        return false;
    }

    load_params.coords = add_implicit_conversion(ctx, block, params->args[1],
            hlsl_get_vector_type(ctx, HLSL_TYPE_FLOAT, sampler_dim), loc);

    if (offset_dim && params->args_count > 2)
        load_params.texel_offset = add_implicit_conversion(ctx, block, params->args[2],
                hlsl_get_vector_type(ctx, HLSL_TYPE_INT, offset_dim), loc);

    if (params->args_count > 2 + !!offset_dim)
        hlsl_fixme(ctx, loc, "Sample() clamp parameter.");
    if (params->args_count > 3 + !!offset_dim)
        hlsl_fixme(ctx, loc, "Tiled resource status argument.");

    load_params.format = object_type->e.resource.format;
    load_params.resource = object;
    load_params.sampler = params->args[0];
    hlsl_block_add_resource_load(ctx, block, &load_params, loc);
    return true;
}

static bool add_sample_cmp_method_call(struct hlsl_ctx *ctx, struct hlsl_block *block, struct hlsl_ir_node *object,
        const char *name, const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    const struct hlsl_type *object_type = object->data_type;
    struct hlsl_resource_load_params load_params = { 0 };
    unsigned int sampler_dim, offset_dim;
    const struct hlsl_type *sampler_type;

    sampler_dim = hlsl_sampler_dim_count(object_type->sampler_dim);
    offset_dim = hlsl_offset_dim_count(object_type->sampler_dim);

    if (!strcmp(name, "SampleCmpLevelZero"))
        load_params.type = HLSL_RESOURCE_SAMPLE_CMP_LZ;
    else
        load_params.type = HLSL_RESOURCE_SAMPLE_CMP;

    if (params->args_count < 3 || params->args_count > 5 + !!offset_dim)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                "Wrong number of arguments to method '%s': expected from 3 to %u, but got %u.",
                name, 5 + !!offset_dim, params->args_count);
        return false;
    }

    sampler_type = params->args[0]->data_type;
    if (sampler_type->class != HLSL_CLASS_SAMPLER || sampler_type->sampler_dim != HLSL_SAMPLER_DIM_COMPARISON)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, sampler_type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Wrong type for argument 0 of %s(): expected 'SamplerComparisonState', but got '%s'.",
                    name, string->buffer);
        hlsl_release_string_buffer(ctx, string);
        return false;
    }

    load_params.coords = add_implicit_conversion(ctx, block, params->args[1],
            hlsl_get_vector_type(ctx, HLSL_TYPE_FLOAT, sampler_dim), loc);
    load_params.cmp = add_implicit_conversion(ctx, block, params->args[2],
            hlsl_get_scalar_type(ctx, HLSL_TYPE_FLOAT), loc);

    if (offset_dim && params->args_count > 3)
        load_params.texel_offset = add_implicit_conversion(ctx, block, params->args[2],
                hlsl_get_vector_type(ctx, HLSL_TYPE_INT, offset_dim), loc);

    if (params->args_count > 3 + !!offset_dim)
        hlsl_fixme(ctx, loc, "%s() clamp parameter.", name);
    if (params->args_count > 4 + !!offset_dim)
        hlsl_fixme(ctx, loc, "Tiled resource status argument.");

    load_params.format = object_type->e.resource.format;
    load_params.resource = object;
    load_params.sampler = params->args[0];
    hlsl_block_add_resource_load(ctx, block, &load_params, loc);
    return true;
}

static bool add_gather_method_call(struct hlsl_ctx *ctx, struct hlsl_block *block, struct hlsl_ir_node *object,
        const char *name, const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    const struct hlsl_type *object_type = object->data_type;
    struct hlsl_resource_load_params load_params = {0};
    unsigned int sampler_dim, offset_dim;
    const struct hlsl_type *sampler_type;
    unsigned int read_channel;

    sampler_dim = hlsl_sampler_dim_count(object_type->sampler_dim);
    offset_dim = hlsl_offset_dim_count(object_type->sampler_dim);

    if (!strcmp(name, "GatherGreen"))
    {
        load_params.type = HLSL_RESOURCE_GATHER_GREEN;
        read_channel = 1;
    }
    else if (!strcmp(name, "GatherBlue"))
    {
        load_params.type = HLSL_RESOURCE_GATHER_BLUE;
        read_channel = 2;
    }
    else if (!strcmp(name, "GatherAlpha"))
    {
        load_params.type = HLSL_RESOURCE_GATHER_ALPHA;
        read_channel = 3;
    }
    else
    {
        load_params.type = HLSL_RESOURCE_GATHER_RED;
        read_channel = 0;
    }

    if (!strcmp(name, "Gather") || !offset_dim)
    {
        if (params->args_count < 2 || params->args_count > 3 + !!offset_dim)
        {
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                    "Wrong number of arguments to method '%s': expected from 2 to %u, but got %u.",
                    name, 3 + !!offset_dim, params->args_count);
            return false;
        }
    }
    else if (params->args_count < 2 || params->args_count == 5 || params->args_count > 7)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                "Wrong number of arguments to method '%s': expected 2, 3, 4, 6 or 7, but got %u.",
                name, params->args_count);
        return false;
    }

    if (params->args_count == 3 + !!offset_dim || params->args_count == 7)
        hlsl_fixme(ctx, loc, "Tiled resource status argument.");

    if (params->args_count == 6 || params->args_count == 7)
    {
        hlsl_fixme(ctx, loc, "Multiple %s() offset parameters.", name);
    }
    else if (offset_dim && params->args_count > 2)
    {
        load_params.texel_offset = add_implicit_conversion(ctx, block, params->args[2],
                hlsl_get_vector_type(ctx, HLSL_TYPE_INT, offset_dim), loc);
    }

    sampler_type = params->args[0]->data_type;
    if (sampler_type->class != HLSL_CLASS_SAMPLER || sampler_type->sampler_dim != HLSL_SAMPLER_DIM_GENERIC)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, sampler_type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Wrong type for argument 1 of %s(): expected 'sampler', but got '%s'.", name, string->buffer);
        hlsl_release_string_buffer(ctx, string);
        return false;
    }

    if (read_channel >= object_type->e.resource.format->e.numeric.dimx)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                "Method %s() requires at least %u channels.", name, read_channel + 1);
        return false;
    }

    load_params.coords = add_implicit_conversion(ctx, block, params->args[1],
            hlsl_get_vector_type(ctx, HLSL_TYPE_FLOAT, sampler_dim), loc);
    load_params.format = hlsl_get_vector_type(ctx, object_type->e.resource.format->e.numeric.type, 4);
    load_params.resource = object;
    load_params.sampler = params->args[0];
    hlsl_block_add_resource_load(ctx, block, &load_params, loc);
    return true;
}

static bool add_gather_cmp_method_call(struct hlsl_ctx *ctx, struct hlsl_block *block, struct hlsl_ir_node *object,
        const char *name, const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    const struct hlsl_type *object_type = object->data_type;
    struct hlsl_resource_load_params load_params = {0};
    unsigned int sampler_dim, offset_dim;
    const struct hlsl_type *sampler_type;

    sampler_dim = hlsl_sampler_dim_count(object_type->sampler_dim);
    offset_dim = hlsl_offset_dim_count(object_type->sampler_dim);

    if (!strcmp(name, "GatherCmpGreen"))
        load_params.type = HLSL_RESOURCE_GATHER_CMP_GREEN;
    else if (!strcmp(name, "GatherCmpBlue"))
        load_params.type = HLSL_RESOURCE_GATHER_CMP_BLUE;
    else if (!strcmp(name, "GatherCmpAlpha"))
        load_params.type = HLSL_RESOURCE_GATHER_CMP_ALPHA;
    else
        load_params.type = HLSL_RESOURCE_GATHER_CMP_RED;

    if (!strcmp(name, "GatherCmp") || !offset_dim)
    {
        if (params->args_count < 3 || params->args_count > 4 + !!offset_dim)
        {
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                    "Wrong number of arguments to method '%s': expected from 3 to %u, but got %u.",
                    name, 4 + !!offset_dim, params->args_count);
            return false;
        }
    }
    else if (params->args_count < 3 || params->args_count == 6 || params->args_count > 8)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                "Wrong number of arguments to method '%s': expected 3, 4, 5, 7, or 8, but got %u.",
                name, params->args_count);
        return false;
    }

    if (params->args_count == 5 || params->args_count == 8)
    {
        hlsl_fixme(ctx, loc, "Tiled resource status argument.");
    }
    else if (offset_dim && params->args_count > 3)
    {
        if (!(load_params.texel_offset = add_implicit_conversion(ctx, block, params->args[3],
                hlsl_get_vector_type(ctx, HLSL_TYPE_INT, offset_dim), loc)))
            return false;
    }

    sampler_type = params->args[0]->data_type;
    if (sampler_type->class != HLSL_CLASS_SAMPLER || sampler_type->sampler_dim != HLSL_SAMPLER_DIM_COMPARISON)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, sampler_type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Wrong type for argument 0 of %s(): expected 'SamplerComparisonState', but got '%s'.",
                    name, string->buffer);
        hlsl_release_string_buffer(ctx, string);
        return false;
    }

    if (!(load_params.coords = add_implicit_conversion(ctx, block, params->args[1],
            hlsl_get_vector_type(ctx, HLSL_TYPE_FLOAT, sampler_dim), loc)))
        return false;

    if (!(load_params.cmp = add_implicit_conversion(ctx, block, params->args[2],
            hlsl_get_scalar_type(ctx, HLSL_TYPE_FLOAT), loc)))
        return false;

    load_params.format = hlsl_get_vector_type(ctx, object_type->e.resource.format->e.numeric.type, 4);
    load_params.resource = object;
    load_params.sampler = params->args[0];
    hlsl_block_add_resource_load(ctx, block, &load_params, loc);
    return true;
}

static bool add_assignment_from_component(struct hlsl_ctx *ctx, struct hlsl_block *instrs, struct hlsl_ir_node *dest,
        struct hlsl_ir_node *src, unsigned int component, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *load;

    if (!dest)
        return true;

    load = hlsl_add_load_component(ctx, instrs, src, component, loc);
    if (!add_assignment(ctx, instrs, dest, ASSIGN_OP_ASSIGN, load, false))
        return false;

    return true;
}

static bool add_getdimensions_method_call(struct hlsl_ctx *ctx, struct hlsl_block *block, struct hlsl_ir_node *object,
        const char *name, const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    const struct hlsl_type *object_type = object->data_type;
    bool uint_resinfo, has_uint_arg, has_float_arg;
    struct hlsl_resource_load_params load_params;
    struct hlsl_ir_node *sample_info, *res_info;
    struct hlsl_type *uint_type, *float_type;
    unsigned int i, j;
    enum func_argument
    {
        ARG_MIP_LEVEL,
        ARG_WIDTH,
        ARG_HEIGHT,
        ARG_ELEMENT_COUNT,
        ARG_LEVEL_COUNT,
        ARG_SAMPLE_COUNT,
        ARG_MAX_ARGS,
    };
    struct hlsl_ir_node *args[ARG_MAX_ARGS] = { 0 };
    static const struct overload
    {
        enum hlsl_sampler_dim sampler_dim;
        unsigned int args_count;
        enum func_argument args[ARG_MAX_ARGS];
        bool srv_only;
    }
    overloads[] =
    {
        { HLSL_SAMPLER_DIM_1D, 1, { ARG_WIDTH } },
        { HLSL_SAMPLER_DIM_1D, 3, { ARG_MIP_LEVEL, ARG_WIDTH, ARG_LEVEL_COUNT }, true },
        { HLSL_SAMPLER_DIM_1DARRAY, 2, { ARG_WIDTH, ARG_ELEMENT_COUNT } },
        { HLSL_SAMPLER_DIM_1DARRAY, 4, { ARG_MIP_LEVEL, ARG_WIDTH, ARG_ELEMENT_COUNT, ARG_LEVEL_COUNT }, true },
        { HLSL_SAMPLER_DIM_2D, 2, { ARG_WIDTH, ARG_HEIGHT } },
        { HLSL_SAMPLER_DIM_2D, 4, { ARG_MIP_LEVEL, ARG_WIDTH, ARG_HEIGHT, ARG_LEVEL_COUNT }, true },
        { HLSL_SAMPLER_DIM_2DARRAY, 3, { ARG_WIDTH, ARG_HEIGHT, ARG_ELEMENT_COUNT } },
        { HLSL_SAMPLER_DIM_2DARRAY, 5, { ARG_MIP_LEVEL, ARG_WIDTH, ARG_HEIGHT, ARG_ELEMENT_COUNT, ARG_LEVEL_COUNT }, true },
        { HLSL_SAMPLER_DIM_3D, 3, { ARG_WIDTH, ARG_HEIGHT, ARG_ELEMENT_COUNT } },
        { HLSL_SAMPLER_DIM_3D, 5, { ARG_MIP_LEVEL, ARG_WIDTH, ARG_HEIGHT, ARG_ELEMENT_COUNT, ARG_LEVEL_COUNT }, true },
        { HLSL_SAMPLER_DIM_CUBE, 2, { ARG_WIDTH, ARG_HEIGHT } },
        { HLSL_SAMPLER_DIM_CUBE, 4, { ARG_MIP_LEVEL, ARG_WIDTH, ARG_HEIGHT, ARG_LEVEL_COUNT } },
        { HLSL_SAMPLER_DIM_CUBEARRAY, 3, { ARG_WIDTH, ARG_HEIGHT, ARG_ELEMENT_COUNT } },
        { HLSL_SAMPLER_DIM_CUBEARRAY, 5, { ARG_MIP_LEVEL, ARG_WIDTH, ARG_HEIGHT, ARG_ELEMENT_COUNT, ARG_LEVEL_COUNT } },
        { HLSL_SAMPLER_DIM_2DMS, 3, { ARG_WIDTH, ARG_HEIGHT, ARG_SAMPLE_COUNT } },
        { HLSL_SAMPLER_DIM_2DMSARRAY, 4, { ARG_WIDTH, ARG_HEIGHT, ARG_ELEMENT_COUNT, ARG_SAMPLE_COUNT } },
        { HLSL_SAMPLER_DIM_BUFFER, 1, { ARG_WIDTH } },
    };
    const struct overload *o = NULL;

    if (object_type->sampler_dim == HLSL_SAMPLER_DIM_STRUCTURED_BUFFER)
    {
        hlsl_fixme(ctx, loc, "Method '%s' for structured buffers.", name);
        return false;
    }

    uint_type = hlsl_get_scalar_type(ctx, HLSL_TYPE_UINT);
    float_type = hlsl_get_scalar_type(ctx, HLSL_TYPE_FLOAT);
    has_uint_arg = has_float_arg = false;
    for (i = 0; i < ARRAY_SIZE(overloads); ++i)
    {
        const struct overload *iter = &overloads[i];

        if (object_type->class == HLSL_CLASS_UAV && iter->srv_only)
            continue;

        if (iter->sampler_dim == object_type->sampler_dim && iter->args_count == params->args_count)
        {
            for (j = 0; j < params->args_count; ++j)
            {
                args[iter->args[j]] = params->args[j];

                /* Input parameter. */
                if (iter->args[j] == ARG_MIP_LEVEL)
                {
                    args[ARG_MIP_LEVEL] = add_implicit_conversion(ctx, block, args[ARG_MIP_LEVEL],
                            hlsl_get_scalar_type(ctx, HLSL_TYPE_UINT), loc);
                    continue;
                }

                has_float_arg |= hlsl_types_are_equal(params->args[j]->data_type, float_type);
                has_uint_arg |= hlsl_types_are_equal(params->args[j]->data_type, uint_type);

                if (params->args[j]->data_type->class != HLSL_CLASS_SCALAR)
                {
                    hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE, "Expected scalar arguments.");
                    break;
                }
            }
            o = iter;
            break;
        }
    }
    uint_resinfo = !has_float_arg && has_uint_arg;

    if (!o)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, object_type)))
        {
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX,
                    "Unexpected number of arguments %u for %s.%s().", params->args_count, string->buffer, name);
            hlsl_release_string_buffer(ctx, string);
        }
    }

    if (!args[ARG_MIP_LEVEL])
        args[ARG_MIP_LEVEL] = hlsl_block_add_uint_constant(ctx, block, 0, loc);

    memset(&load_params, 0, sizeof(load_params));
    load_params.type = HLSL_RESOURCE_RESINFO;
    load_params.resource = object;
    load_params.lod = args[ARG_MIP_LEVEL];
    load_params.format = hlsl_get_vector_type(ctx, uint_resinfo ? HLSL_TYPE_UINT : HLSL_TYPE_FLOAT, 4);
    res_info = hlsl_block_add_resource_load(ctx, block, &load_params, loc);

    if (!add_assignment_from_component(ctx, block, args[ARG_WIDTH], res_info, 0, loc))
        return false;

    if (!add_assignment_from_component(ctx, block, args[ARG_HEIGHT], res_info, 1, loc))
        return false;

    if (!add_assignment_from_component(ctx, block, args[ARG_ELEMENT_COUNT], res_info,
            object_type->sampler_dim == HLSL_SAMPLER_DIM_1DARRAY ? 1 : 2, loc))
    {
        return false;
    }

    if (!add_assignment_from_component(ctx, block, args[ARG_LEVEL_COUNT], res_info, 3, loc))
        return false;

    if (args[ARG_SAMPLE_COUNT])
    {
        memset(&load_params, 0, sizeof(load_params));
        load_params.type = HLSL_RESOURCE_SAMPLE_INFO;
        load_params.resource = object;
        load_params.format = args[ARG_SAMPLE_COUNT]->data_type;
        sample_info = hlsl_block_add_resource_load(ctx, block, &load_params, loc);

        if (!add_assignment(ctx, block, args[ARG_SAMPLE_COUNT], ASSIGN_OP_ASSIGN, sample_info, false))
            return false;
    }

    add_void_expr(ctx, block, loc);
    return true;
}

static bool add_sample_lod_method_call(struct hlsl_ctx *ctx, struct hlsl_block *block, struct hlsl_ir_node *object,
        const char *name, const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    const struct hlsl_type *object_type = object->data_type;
    struct hlsl_resource_load_params load_params = { 0 };
    unsigned int sampler_dim, offset_dim;
    const struct hlsl_type *sampler_type;

    sampler_dim = hlsl_sampler_dim_count(object_type->sampler_dim);
    offset_dim = hlsl_offset_dim_count(object_type->sampler_dim);

    if (!strcmp(name, "SampleLevel"))
        load_params.type = HLSL_RESOURCE_SAMPLE_LOD;
    else
        load_params.type = HLSL_RESOURCE_SAMPLE_LOD_BIAS;

    if (params->args_count < 3 || params->args_count > 4 + !!offset_dim)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                "Wrong number of arguments to method '%s': expected from 3 to %u, but got %u.",
                name, 4 + !!offset_dim, params->args_count);
        return false;
    }

    sampler_type = params->args[0]->data_type;
    if (sampler_type->class != HLSL_CLASS_SAMPLER || sampler_type->sampler_dim != HLSL_SAMPLER_DIM_GENERIC)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, sampler_type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Wrong type for argument 0 of %s(): expected 'sampler', but got '%s'.", name, string->buffer);
        hlsl_release_string_buffer(ctx, string);
        return false;
    }

    load_params.coords = add_implicit_conversion(ctx, block, params->args[1],
            hlsl_get_vector_type(ctx, HLSL_TYPE_FLOAT, sampler_dim), loc);
    load_params.lod = add_implicit_conversion(ctx, block, params->args[2],
            hlsl_get_scalar_type(ctx, HLSL_TYPE_FLOAT), loc);

    if (offset_dim && params->args_count > 3)
        load_params.texel_offset = add_implicit_conversion(ctx, block, params->args[3],
                hlsl_get_vector_type(ctx, HLSL_TYPE_INT, offset_dim), loc);

    if (params->args_count > 3 + !!offset_dim)
        hlsl_fixme(ctx, loc, "Tiled resource status argument.");

    load_params.format = object_type->e.resource.format;
    load_params.resource = object;
    load_params.sampler = params->args[0];
    hlsl_block_add_resource_load(ctx, block, &load_params, loc);
    return true;
}

static bool add_sample_grad_method_call(struct hlsl_ctx *ctx, struct hlsl_block *block, struct hlsl_ir_node *object,
        const char *name, const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    const struct hlsl_type *object_type = object->data_type;
    struct hlsl_resource_load_params load_params = { 0 };
    unsigned int sampler_dim, offset_dim;
    const struct hlsl_type *sampler_type;

    sampler_dim = hlsl_sampler_dim_count(object_type->sampler_dim);
    offset_dim = hlsl_offset_dim_count(object_type->sampler_dim);

    load_params.type = HLSL_RESOURCE_SAMPLE_GRAD;

    if (params->args_count < 4 || params->args_count > 5 + !!offset_dim)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                "Wrong number of arguments to method '%s': expected from 4 to %u, but got %u.",
                name, 5 + !!offset_dim, params->args_count);
        return false;
    }

    sampler_type = params->args[0]->data_type;
    if (sampler_type->class != HLSL_CLASS_SAMPLER || sampler_type->sampler_dim != HLSL_SAMPLER_DIM_GENERIC)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, sampler_type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Wrong type for argument 0 of %s(): expected 'sampler', but got '%s'.", name, string->buffer);
        hlsl_release_string_buffer(ctx, string);
        return false;
    }

    load_params.coords = add_implicit_conversion(ctx, block, params->args[1],
            hlsl_get_vector_type(ctx, HLSL_TYPE_FLOAT, sampler_dim), loc);
    load_params.ddx = add_implicit_conversion(ctx, block, params->args[2],
            hlsl_get_vector_type(ctx, HLSL_TYPE_FLOAT, sampler_dim), loc);
    load_params.ddy = add_implicit_conversion(ctx, block, params->args[3],
            hlsl_get_vector_type(ctx, HLSL_TYPE_FLOAT, sampler_dim), loc);

    if (offset_dim && params->args_count > 4)
        load_params.texel_offset = add_implicit_conversion(ctx, block, params->args[4],
                hlsl_get_vector_type(ctx, HLSL_TYPE_INT, offset_dim), loc);

    if (params->args_count > 4 + !!offset_dim)
        hlsl_fixme(ctx, loc, "Tiled resource status argument.");

    load_params.format = object_type->e.resource.format;
    load_params.resource = object;
    load_params.sampler = params->args[0];
    hlsl_block_add_resource_load(ctx, block, &load_params, loc);
    return true;
}

static bool add_store_method_call(struct hlsl_ctx *ctx, struct hlsl_block *block, struct hlsl_ir_node *object,
        const char *name, const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *offset, *rhs;
    struct hlsl_deref resource_deref;
    unsigned int value_dim;
    uint32_t writemask;

    if (params->args_count != 2)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                "Wrong number of arguments to method '%s': expected 2.", name);
        return false;
    }

    if (!strcmp(name, "Store"))
        value_dim = 1;
    else if (!strcmp(name, "Store2"))
        value_dim = 2;
    else if (!strcmp(name, "Store3"))
        value_dim = 3;
    else
        value_dim = 4;

    offset = add_implicit_conversion(ctx, block, params->args[0],
            hlsl_get_scalar_type(ctx, HLSL_TYPE_UINT), loc);
    rhs = add_implicit_conversion(ctx, block, params->args[1],
            hlsl_get_vector_type(ctx, HLSL_TYPE_UINT, value_dim), loc);
    writemask = vkd3d_write_mask_from_component_count(value_dim);

    if (!hlsl_deref_init_from_index_chain(&resource_deref, object, ctx))
        return false;

    hlsl_block_add_resource_store(ctx, block, HLSL_RESOURCE_STORE,
            &resource_deref, NULL, offset, rhs, writemask, loc);
    hlsl_deref_cleanup(&resource_deref);

    return true;
}

static bool add_so_append_method_call(struct hlsl_ctx *ctx, struct hlsl_block *block, struct hlsl_ir_node *object,
        const char *name, const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_deref so_deref;
    struct hlsl_ir_node *rhs;

    if (params->args_count != 1)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                "Wrong number of arguments to method '%s': expected 1.", name);
        return false;
    }

    if (!hlsl_deref_init_from_index_chain(&so_deref, object, ctx))
        return false;

    if (!(rhs = add_implicit_conversion(ctx, block, params->args[0], object->data_type->e.so.type, loc)))
        return false;

    hlsl_block_add_resource_store(ctx, block, HLSL_RESOURCE_STREAM_APPEND, &so_deref, NULL, NULL, rhs, 0, loc);
    hlsl_deref_cleanup(&so_deref);

    return true;
}

static bool add_so_restartstrip_method_call(struct hlsl_ctx *ctx, struct hlsl_block *block, struct hlsl_ir_node *object,
        const char *name, const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_deref so_deref;

    if (params->args_count)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_WRONG_PARAMETER_COUNT,
                "Wrong number of arguments to method '%s': expected 0.", name);
        return false;
    }

    if (!hlsl_deref_init_from_index_chain(&so_deref, object, ctx))
        return false;

    hlsl_block_add_resource_store(ctx, block, HLSL_RESOURCE_STREAM_RESTART, &so_deref, NULL, NULL, NULL, 0, loc);
    hlsl_deref_cleanup(&so_deref);

    return true;
}

static const struct method_function
{
    const char *name;
    bool (*handler)(struct hlsl_ctx *ctx, struct hlsl_block *block, struct hlsl_ir_node *object,
            const char *name, const struct parse_initializer *params, const struct vkd3d_shader_location *loc);
    char valid_dims[HLSL_SAMPLER_DIM_MAX + 1];
}
texture_methods[] =
{
    { "Gather",             add_gather_method_call,        "00010101001000" },
    { "GatherAlpha",        add_gather_method_call,        "00010101001000" },
    { "GatherBlue",         add_gather_method_call,        "00010101001000" },
    { "GatherCmp",          add_gather_cmp_method_call,    "00010101001000" },
    { "GatherCmpAlpha",     add_gather_cmp_method_call,    "00010101001000" },
    { "GatherCmpBlue",      add_gather_cmp_method_call,    "00010101001000" },
    { "GatherCmpGreen",     add_gather_cmp_method_call,    "00010101001000" },
    { "GatherCmpRed",       add_gather_cmp_method_call,    "00010101001000" },
    { "GatherGreen",        add_gather_method_call,        "00010101001000" },
    { "GatherRed",          add_gather_method_call,        "00010101001000" },

    { "GetDimensions",      add_getdimensions_method_call, "00111111111110" },

    { "Load",               add_load_method_call,          "00111011110111" },
    { "Load2",              add_raw_load_method_call,      "00000000000001" },
    { "Load3",              add_raw_load_method_call,      "00000000000001" },
    { "Load4",              add_raw_load_method_call,      "00000000000001" },

    { "Sample",             add_sample_method_call,        "00111111001000" },
    { "SampleBias",         add_sample_lod_method_call,    "00111111001000" },
    { "SampleCmp",          add_sample_cmp_method_call,    "00111111001000" },
    { "SampleCmpLevelZero", add_sample_cmp_method_call,    "00111111001000" },
    { "SampleGrad",         add_sample_grad_method_call,   "00111111001000" },
    { "SampleLevel",        add_sample_lod_method_call,    "00111111001000" },
};

static const struct method_function uav_methods[] =
{
    { "GetDimensions", add_getdimensions_method_call, "00111011000000" },

    { "Store",         add_store_method_call,         "00000000000001" },
    { "Store2",        add_store_method_call,         "00000000000001" },
    { "Store3",        add_store_method_call,         "00000000000001" },
    { "Store4",        add_store_method_call,         "00000000000001" },
};

static const struct method_function so_methods[] =
{
    { "Append",       add_so_append_method_call,       "" },
    { "RestartStrip", add_so_restartstrip_method_call, "" },
};

static int object_method_function_name_compare(const void *a, const void *b)
{
    const struct method_function *func = b;

    return strcmp(a, func->name);
}

static bool add_method_call(struct hlsl_ctx *ctx, struct hlsl_block *block, struct hlsl_ir_node *object,
        const char *name, const struct parse_initializer *params, const struct vkd3d_shader_location *loc)
{
    const struct hlsl_type *object_type = object->data_type;
    const struct method_function *method;

    if (object_type->class == HLSL_CLASS_ERROR)
    {
        block->value = ctx->error_instr;
        return true;
    }

    for (unsigned int i = 0; i < params->args_count; ++i)
    {
        if (params->args[i]->data_type->class == HLSL_CLASS_ERROR)
        {
            block->value = ctx->error_instr;
            return true;
        }
    }

    if (object_type->class == HLSL_CLASS_TEXTURE)
    {
        method = bsearch(name, texture_methods, ARRAY_SIZE(texture_methods), sizeof(*method),
                object_method_function_name_compare);

        if (method && method->valid_dims[object_type->sampler_dim] != '1')
            method = NULL;
    }
    else if (object_type->class == HLSL_CLASS_UAV)
    {
        method = bsearch(name, uav_methods, ARRAY_SIZE(uav_methods), sizeof(*method),
                object_method_function_name_compare);

        if (method && method->valid_dims[object_type->sampler_dim] != '1')
            method = NULL;
    }
    else if (object_type->class == HLSL_CLASS_STREAM_OUTPUT)
    {
        method = bsearch(name, so_methods, ARRAY_SIZE(so_methods), sizeof(*method),
                object_method_function_name_compare);
    }
    else
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_type_to_string(ctx, object_type)))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "Type '%s' does not have methods.", string->buffer);
        hlsl_release_string_buffer(ctx, string);
        return false;
    }

    if (method)
        return method->handler(ctx, block, object, name, params, loc);
    else
        return raise_invalid_method_object_type(ctx, object_type, name, loc);
}

static bool add_object_property_access(struct hlsl_ctx *ctx,
        struct hlsl_block *block, struct hlsl_ir_node *object, const char *name,
        const struct vkd3d_shader_location *loc)
{
    const struct hlsl_type *object_type = object->data_type;
    struct hlsl_resource_load_params load_params;
    struct hlsl_ir_node *zero;
    unsigned int sampler_dim;

    if (!strcmp(name, "Length"))
    {
        if (object_type->class != HLSL_CLASS_TEXTURE && object_type->class != HLSL_CLASS_UAV)
            return false;

        sampler_dim = hlsl_sampler_dim_count(object_type->sampler_dim);

        switch (object_type->sampler_dim)
        {
            case HLSL_SAMPLER_DIM_1D:
            case HLSL_SAMPLER_DIM_2D:
            case HLSL_SAMPLER_DIM_3D:
            case HLSL_SAMPLER_DIM_1DARRAY:
            case HLSL_SAMPLER_DIM_2DARRAY:
            case HLSL_SAMPLER_DIM_2DMS:
            case HLSL_SAMPLER_DIM_2DMSARRAY:
                break;

            case HLSL_SAMPLER_DIM_BUFFER:
            case HLSL_SAMPLER_DIM_STRUCTURED_BUFFER:
                hlsl_fixme(ctx, loc, "'Length' property for buffers.");
                block->value = ctx->error_instr;
                return true;

            default:
                return false;
        }

        if (hlsl_version_lt(ctx, 4, 0))
        {
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INCOMPATIBLE_PROFILE,
                    "'Length' property can only be used on profiles 4.0 or higher.");
            block->value = ctx->error_instr;
            return true;
        }

        zero = hlsl_block_add_uint_constant(ctx, block, 0, loc);

        memset(&load_params, 0, sizeof(load_params));
        load_params.type = HLSL_RESOURCE_RESINFO;
        load_params.resource = object;
        load_params.lod = zero;
        load_params.format = hlsl_get_vector_type(ctx, HLSL_TYPE_UINT, sampler_dim);
        hlsl_block_add_resource_load(ctx, block, &load_params, loc);

        return true;
    }

    if (!strcmp(name, "mips") && object_type->class == HLSL_CLASS_TEXTURE)
    {
        struct hlsl_ir_node *operands[HLSL_MAX_OPERANDS] = {object};
        struct hlsl_type *type;

        switch (object_type->sampler_dim)
        {
            case HLSL_SAMPLER_DIM_1D:
            case HLSL_SAMPLER_DIM_1DARRAY:
            case HLSL_SAMPLER_DIM_2D:
            case HLSL_SAMPLER_DIM_2DARRAY:
            case HLSL_SAMPLER_DIM_3D:
                break;

            default:
                return false;
        }

        if (hlsl_version_lt(ctx, 4, 0))
        {
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INCOMPATIBLE_PROFILE,
                    "'mips' property can only be used on profiles 4.0 or higher.");
            block->value = ctx->error_instr;
            return true;
        }

        if (!(type = hlsl_new_mips_type(ctx, object->data_type, HLSL_MIPS_PROPERTY)))
            return false;
        hlsl_block_add_expr(ctx, block, HLSL_OP1_MIPS, operands, type, loc);
        return true;
    }

    return false;
}

static void validate_texture_format_type(struct hlsl_ctx *ctx, enum hlsl_sampler_dim dim,
        struct hlsl_type *format, const struct vkd3d_shader_location *loc)
{
    struct vkd3d_string_buffer *string;

    if (!(string = hlsl_type_to_string(ctx, format)))
        return;

    if (dim == HLSL_SAMPLER_DIM_STRUCTURED_BUFFER)
    {
        if (!type_contains_only_numerics(format))
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "SRV type %s is not numeric.", string->buffer);
    }
    else if (format->class > HLSL_CLASS_VECTOR)
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                "Texture data type %s is not scalar or vector.", string->buffer);

    hlsl_release_string_buffer(ctx, string);
}

static bool check_continue(struct hlsl_ctx *ctx, const struct hlsl_scope *scope, const struct vkd3d_shader_location *loc)
{
    if (scope->_switch)
    {
        hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX,
                "The 'continue' statement is not allowed in 'switch' statements.");
        return false;
    }

    if (scope->loop)
        return true;

    if (scope->upper)
        return check_continue(ctx, scope->upper, loc);

    hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX, "The 'continue' statement is only allowed in loops.");
    return false;
}

static bool is_break_allowed(const struct hlsl_scope *scope)
{
    if (scope->loop || scope->_switch)
        return true;

    return scope->upper ? is_break_allowed(scope->upper) : false;
}

static void check_duplicated_switch_cases(struct hlsl_ctx *ctx, const struct hlsl_ir_switch_case *check, struct list *cases)
{
    struct hlsl_ir_switch_case *c;
    bool found_duplicate = false;

    LIST_FOR_EACH_ENTRY(c, cases, struct hlsl_ir_switch_case, entry)
    {
        if (check->is_default)
        {
            if ((found_duplicate = c->is_default))
            {
                hlsl_error(ctx, &check->loc, VKD3D_SHADER_ERROR_HLSL_DUPLICATE_SWITCH_CASE,
                        "Found multiple 'default' statements.");
                hlsl_note(ctx, &c->loc, VKD3D_SHADER_LOG_ERROR, "The 'default' statement was previously found here.");
            }
        }
        else
        {
            if (c->is_default) continue;
            if ((found_duplicate = (c->value == check->value)))
            {
                hlsl_error(ctx, &check->loc, VKD3D_SHADER_ERROR_HLSL_DUPLICATE_SWITCH_CASE,
                        "Found duplicate 'case' statement.");
                hlsl_note(ctx, &c->loc, VKD3D_SHADER_LOG_ERROR, "The same 'case %d' statement was previously found here.",
                        c->value);
            }
        }

        if (found_duplicate)
            break;
    }
}

static bool add_switch(struct hlsl_ctx *ctx, struct hlsl_block *block,
        struct parse_attribute_list *attributes, struct list *cases, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *selector = node_from_block(block);
    struct hlsl_ir_node *s;

    if (selector->data_type->class == HLSL_CLASS_ERROR)
    {
        destroy_switch_cases(cases);
        destroy_block(block);
        cleanup_parse_attribute_list(attributes);
        return true;
    }

    selector = add_implicit_conversion(ctx, block, selector,
            hlsl_get_scalar_type(ctx, HLSL_TYPE_UINT), &selector->loc);
    s = hlsl_new_switch(ctx, selector, cases, loc);

    destroy_switch_cases(cases);

    if (!s)
    {
        destroy_block(block);
        cleanup_parse_attribute_list(attributes);
        return false;
    }

    hlsl_block_add_instr(block, s);

    cleanup_parse_attribute_list(attributes);
    return true;
}

static void validate_uav_type(struct hlsl_ctx *ctx, enum hlsl_sampler_dim dim,
        struct hlsl_type *format, const struct vkd3d_shader_location* loc)
{
    struct vkd3d_string_buffer *string = hlsl_type_to_string(ctx, format);

    if (!type_contains_only_numerics(format))
    {
        if (string)
            hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                    "UAV type %s is not numeric.", string->buffer);
    }

    switch (dim)
    {
        case HLSL_SAMPLER_DIM_BUFFER:
        case HLSL_SAMPLER_DIM_1D:
        case HLSL_SAMPLER_DIM_1DARRAY:
        case HLSL_SAMPLER_DIM_2D:
        case HLSL_SAMPLER_DIM_2DARRAY:
        case HLSL_SAMPLER_DIM_3D:
            if (format->class == HLSL_CLASS_ARRAY)
            {
                if (string)
                    hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                            "This type of UAV does not support array type.");
            }
            else if (hlsl_type_component_count(format) > 4)
            {
                if (string)
                    hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                            "UAV data type %s size exceeds maximum size.", string->buffer);
            }
            break;
        case HLSL_SAMPLER_DIM_STRUCTURED_BUFFER:
            break;
        default:
            vkd3d_unreachable();
    }

    hlsl_release_string_buffer(ctx, string);
}

static char *take_unqualified_identifier(struct hlsl_ctx *ctx, struct hlsl_identifier *identifier,
        const struct vkd3d_shader_location *loc)
{
    char *name;

    /* For an unqualified identifier, "name" and "qualified_name" are the same string. */
    if (!hlsl_identifier_is_qualified(identifier))
        return identifier->qualified_name;

    hlsl_error(ctx, loc, VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX,
            "Unexpected qualified identifier '%s'.", identifier->qualified_name);
    /* Copy the unqualified name, so that parsing can continue. */
    name = hlsl_strdup(ctx, identifier->name);
    hlsl_identifier_cleanup(identifier);
    return name;
}



#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined HLSL_YYLTYPE_IS_TRIVIAL && HLSL_YYLTYPE_IS_TRIVIAL \
             && defined HLSL_YYSTYPE_IS_TRIVIAL && HLSL_YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4653

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  169
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  116
/* YYNRULES -- Number of rules.  */
#define YYNRULES  355
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  665

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   399


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   161,     2,     2,     2,   164,   165,     2,
     153,   154,   162,   158,   155,   159,   156,   163,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   150,   145,
     148,   157,   149,   168,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   151,     2,   152,   166,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   146,   167,   147,   160,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144
};

#if HLSL_YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,  7509,  7509,  7510,  7511,  7512,  7513,  7518,  7519,  7520,
    7521,  7524,  7530,  7537,  7541,  7544,  7550,  7557,  7566,  7570,
    7575,  7584,  7585,  7588,  7589,  7592,  7601,  7610,  7623,  7624,
    7625,  7628,  7629,  7632,  7633,  7636,  7644,  7666,  7672,  7676,
    7682,  7683,  7689,  7707,  7719,  7720,  7723,  7768,  7774,  7775,
    7776,  7777,  7780,  7788,  7792,  7806,  7812,  7840,  7841,  7844,
    7865,  7877,  7898,  7908,  7923,  7928,  7931,  7984,  7993,  8163,
    8164,  8181,  8186,  8192,  8198,  8205,  8212,  8219,  8220,  8223,
    8230,  8231,  8238,  8244,  8250,  8260,  8276,  8308,  8317,  8329,
    8351,  8369,  8384,  8401,  8424,  8430,  8439,  8443,  8447,  8453,
    8462,  8470,  8471,  8478,  8535,  8539,  8543,  8547,  8551,  8555,
    8559,  8563,  8567,  8573,  8577,  8583,  8587,  8591,  8595,  8599,
    8603,  8607,  8613,  8617,  8621,  8625,  8629,  8633,  8637,  8643,
    8647,  8651,  8657,  8666,  8670,  8676,  8699,  8703,  8732,  8736,
    8740,  8744,  8748,  8752,  8756,  8760,  8764,  8771,  8776,  8782,
    8798,  8802,  8807,  8812,  8816,  8830,  8834,  8838,  8855,  8877,
    8887,  8891,  8895,  8899,  8903,  8907,  8911,  8915,  8919,  8923,
    8930,  8934,  8940,  8941,  8947,  8948,  8953,  8960,  8961,  8964,
    8984,  8990,  8997,  9006,  9013,  9019,  9026,  9034,  9053,  9065,
    9071,  9072,  9077,  9082,  9087,  9094,  9099,  9112,  9117,  9141,
    9161,  9191,  9201,  9212,  9213,  9218,  9228,  9243,  9258,  9275,
    9279,  9300,  9305,  9322,  9326,  9330,  9334,  9338,  9342,  9346,
    9350,  9354,  9358,  9362,  9366,  9370,  9374,  9378,  9382,  9386,
    9390,  9394,  9398,  9402,  9406,  9410,  9414,  9418,  9422,  9426,
    9441,  9454,  9459,  9466,  9467,  9488,  9491,  9504,  9522,  9526,
    9532,  9533,  9541,  9542,  9543,  9548,  9549,  9550,  9551,  9554,
    9566,  9574,  9580,  9590,  9601,  9653,  9658,  9665,  9671,  9677,
    9683,  9691,  9700,  9716,  9731,  9743,  9753,  9763,  9771,  9776,
    9779,  9785,  9794,  9797,  9803,  9809,  9815,  9821,  9838,  9850,
    9869,  9874,  9885,  9896,  9901,  9920,  9946,  9947,  9956,  9965,
    9998, 10014, 10026, 10046, 10047, 10056, 10065, 10069, 10074, 10079,
   10085, 10097, 10098, 10102, 10106, 10112, 10113, 10117, 10127, 10128,
   10132, 10138, 10139, 10143, 10147, 10151, 10157, 10158, 10162, 10168,
   10169, 10175, 10176, 10182, 10183, 10189, 10190, 10196, 10197, 10203,
   10204, 10225, 10226, 10243, 10247, 10251, 10255, 10259, 10263, 10267,
   10271, 10275, 10279, 10283, 10289, 10290
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "KW_BLENDSTATE",
  "KW_BREAK", "KW_BUFFER", "KW_BYTEADDRESSBUFFER", "KW_CASE",
  "KW_CONSTANTBUFFER", "KW_CBUFFER", "KW_CENTROID", "KW_COLUMN_MAJOR",
  "KW_COMPILE", "KW_COMPILESHADER", "KW_COMPUTESHADER", "KW_CONST",
  "KW_CONTINUE", "KW_DEFAULT", "KW_DEPTHSTENCILSTATE",
  "KW_DEPTHSTENCILVIEW", "KW_DISCARD", "KW_DO", "KW_DOMAINSHADER",
  "KW_ELSE", "KW_EXPORT", "KW_EXTERN", "KW_FALSE", "KW_FOR", "KW_FXGROUP",
  "KW_GEOMETRYSHADER", "KW_GROUPSHARED", "KW_HULLSHADER", "KW_IF", "KW_IN",
  "KW_INLINE", "KW_INOUT", "KW_INPUTPATCH", "KW_LINE", "KW_LINEADJ",
  "KW_LINEAR", "KW_LINESTREAM", "KW_MATRIX", "KW_NAMESPACE",
  "KW_NOINTERPOLATION", "KW_NOPERSPECTIVE", "KW_NULL", "KW_OUT",
  "KW_OUTPUTPATCH", "KW_PACKOFFSET", "KW_PASS", "KW_PIXELSHADER",
  "KW_POINT", "KW_POINTSTREAM", "KW_RASTERIZERORDEREDBUFFER",
  "KW_RASTERIZERORDEREDSTRUCTUREDBUFFER", "KW_RASTERIZERORDEREDTEXTURE1D",
  "KW_RASTERIZERORDEREDTEXTURE1DARRAY", "KW_RASTERIZERORDEREDTEXTURE2D",
  "KW_RASTERIZERORDEREDTEXTURE2DARRAY", "KW_RASTERIZERORDEREDTEXTURE3D",
  "KW_RASTERIZERSTATE", "KW_RENDERTARGETVIEW", "KW_RETURN", "KW_REGISTER",
  "KW_ROW_MAJOR", "KW_RWBUFFER", "KW_RWBYTEADDRESSBUFFER",
  "KW_RWSTRUCTUREDBUFFER", "KW_RWTEXTURE1D", "KW_RWTEXTURE1DARRAY",
  "KW_RWTEXTURE2D", "KW_RWTEXTURE2DARRAY", "KW_RWTEXTURE3D", "KW_SAMPLER",
  "KW_SAMPLER1D", "KW_SAMPLER2D", "KW_SAMPLER3D", "KW_SAMPLERCUBE",
  "KW_SAMPLER_STATE", "KW_SAMPLERCOMPARISONSTATE", "KW_SHARED", "KW_SNORM",
  "KW_STATEBLOCK", "KW_STATEBLOCK_STATE", "KW_STATIC", "KW_STRING",
  "KW_STRUCT", "KW_STRUCTUREDBUFFER", "KW_SWITCH", "KW_TBUFFER",
  "KW_TECHNIQUE", "KW_TECHNIQUE10", "KW_TECHNIQUE11", "KW_TEXTURE",
  "KW_TEXTURE1D", "KW_TEXTURE1DARRAY", "KW_TEXTURE2D", "KW_TEXTURE2DARRAY",
  "KW_TEXTURE2DMS", "KW_TEXTURE2DMSARRAY", "KW_TEXTURE3D",
  "KW_TEXTURECUBE", "KW_TEXTURECUBEARRAY", "KW_TRIANGLE", "KW_TRIANGLEADJ",
  "KW_TRIANGLESTREAM", "KW_TRUE", "KW_TYPEDEF", "KW_UNSIGNED",
  "KW_UNIFORM", "KW_UNORM", "KW_VECTOR", "KW_VERTEXSHADER", "KW_VOID",
  "KW_VOLATILE", "KW_WHILE", "OP_INC", "OP_DEC", "OP_AND", "OP_OR",
  "OP_EQ", "OP_LEFTSHIFT", "OP_LEFTSHIFTASSIGN", "OP_RIGHTSHIFT",
  "OP_RIGHTSHIFTASSIGN", "OP_LE", "OP_GE", "OP_NE", "OP_ADDASSIGN",
  "OP_SUBASSIGN", "OP_MULASSIGN", "OP_DIVASSIGN", "OP_MODASSIGN",
  "OP_ANDASSIGN", "OP_ORASSIGN", "OP_XORASSIGN", "C_FLOAT", "C_INTEGER",
  "C_UNSIGNED", "PRE_LINE", "VAR_IDENTIFIER", "NEW_IDENTIFIER",
  "NAMESPACE_IDENTIFIER", "STRING", "TYPE_IDENTIFIER", "';'", "'{'", "'}'",
  "'<'", "'>'", "':'", "'['", "']'", "'('", "')'", "','", "'.'", "'='",
  "'+'", "'-'", "'~'", "'!'", "'*'", "'/'", "'%'", "'&'", "'^'", "'|'",
  "'?'", "$accept", "hlsl_prog", "namespace_declaration",
  "namespace_prefix", "name_opt", "pass", "annotations_list",
  "annotations_opt", "pass_list", "passes", "technique9", "technique10",
  "technique11", "global_technique", "group_technique", "group_techniques",
  "effect_group", "buffer_declaration", "buffer_body", "buffer_type",
  "declaration_statement_list", "preproc_directive",
  "struct_declaration_without_vars", "struct_spec", "named_struct_spec",
  "unnamed_struct_spec", "any_identifier", "unqualified_identifier",
  "base_optional", "fields_list", "field_type", "field", "attribute",
  "attribute_list", "attribute_list_optional", "func_declaration",
  "func_prototype_no_attrs", "func_prototype", "compound_statement",
  "scope_start", "loop_scope_start", "switch_scope_start",
  "annotations_scope_start", "var_identifier",
  "unqualified_var_identifier", "var_modifier_identifier",
  "colon_attributes", "semantic", "register_reservation",
  "packoffset_reservation", "parameters", "param_list", "parameter",
  "parameter_decl", "texture_type", "texture_ms_type", "uav_type",
  "rov_type", "so_type", "resource_format", "patch_type", "type_no_void",
  "type", "declaration_statement", "typedef_type", "typedef", "type_specs",
  "type_spec", "declaration", "variables_def", "variables_def_typed",
  "variable_decl", "state_block_start", "stateblock_lhs_identifier",
  "state_block_index_opt", "state_block", "state_block_list",
  "variable_def", "variable_def_typed", "array", "arrays", "var_modifiers",
  "complex_initializer", "complex_initializer_list", "initializer_expr",
  "initializer_expr_list", "boolean", "statement_list", "statement",
  "jump_statement", "selection_statement", "if_body", "loop_statement",
  "switch_statement", "switch_case", "switch_cases", "expr_optional",
  "expr_statement", "func_arguments", "primary_expr", "postfix_expr",
  "unary_expr", "mul_expr", "add_expr", "shift_expr", "relational_expr",
  "equality_expr", "bitand_expr", "bitxor_expr", "bitor_expr",
  "logicand_expr", "logicor_expr", "conditional_expr", "assignment_expr",
  "assign_op", "expr", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-392)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-296)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -392,  3080,  -392,  4433,  4433,  4433,  4433,  4433,   216,  4433,
    4433,  4433,  4433,  4433,  4433,  4433,   216,  4433,  4433,  4433,
    4433,  4433,  4433,  4433,  4433,   216,   216,   216,  4433,  4433,
    4433,  4433,  4433,  4433,    55,  -392,  -392,  -392,  -392,   216,
    -392,    23,  -392,  -392,  -392,  -392,  -392,    67,  -392,  -392,
    -392,  3936,  -392,  -392,   207,  -392,  -392,  4433,  -392,  -392,
    -392,     9,  -392,  3314,  -392,  -392,  -392,  -392,  -392,  -392,
    -392,  -392,  -392,  -392,    70,  -392,  -392,  -392,  -392,  -392,
    -392,  -392,  -392,  -392,  -392,  -392,  -392,  -392,  -392,  -392,
    -392,    70,  -392,    70,    70,  -392,  -392,  3415,  -392,  -392,
    -392,  -392,   226,  -392,  -392,  -392,  -392,  -392,  3516,  -392,
    1334,  -392,  -392,  -392,   216,  -392,  -392,  -392,    74,  -392,
    -392,  -392,  -392,  -392,  -392,  -392,  -392,  -392,    87,  -392,
    -392,  -392,  -392,  -392,  -392,  -392,  -392,  -392,  -392,  -392,
    -392,  -392,  -392,  -392,  -392,  -392,  -392,  -392,  -392,  -392,
    -392,  -392,  -392,  -392,  -392,  -392,   157,  -392,  -392,  -392,
    -392,  -392,  -392,  -392,  -392,  -392,  -392,  -392,  -392,  -392,
     104,   117,  -392,  -392,  -392,   216,   171,  -392,  -392,   126,
     133,   147,   159,   173,   186,  -392,   229,  -392,   202,   228,
     249,   258,  -392,  -392,   216,  -392,  2941,  3817,  4182,   108,
     250,   198,   216,   214,   291,   299,  -392,  -392,  2068,   306,
    -392,  2941,  2941,  -392,  -392,  -392,  4432,  4485,  -392,  -392,
    2941,  2941,  2941,  2941,  2941,   307,    59,   311,   308,  -392,
    3415,  -392,  1486,  -392,  -392,  -392,  -392,  -392,   315,  -392,
    -392,     0,     2,   219,   234,   179,    18,    19,   298,   300,
     304,   347,    -6,  -392,  -392,   313,   321,   -34,  -392,  3516,
    3516,    41,  -392,   325,  -392,  3516,  -392,  -392,  -392,  4433,
    4433,  4433,  4433,  3516,  3516,   323,   324,   326,  -392,  4018,
    -392,  -392,  -392,  -392,   321,    15,  -392,  3516,  -392,   247,
    -392,  -392,  -392,  -392,  3415,  -392,  -392,   250,   216,  -392,
    -392,  -392,    29,  -392,  -392,  -392,  3516,   252,  -392,  -392,
    -392,  -392,   327,    71,   390,  -392,  2207,    39,  -392,  -392,
    -392,  -392,  -392,  2941,   216,  -392,  -392,  -392,  -392,  -392,
    -392,  -392,  -392,  -392,  -392,  -392,  2941,  2941,  2941,  2941,
    2941,  2941,  2941,  2941,  2941,  2941,  2941,  2941,  2941,  2941,
    2941,  2941,  2941,  2941,  2941,  2941,  2941,  2359,   321,  -392,
    -392,  2511,   332,   328,  4269,   338,   340,   333,   160,   343,
    3516,   -30,   344,   345,   346,   334,  -392,  -392,  4100,    33,
    3415,   318,   349,   438,   351,   352,  -392,  -392,   216,   348,
     354,  2941,   216,   350,   355,  -392,  -392,   180,  -392,  2941,
    1638,   356,   358,   359,   360,   362,  2941,    94,   361,  -392,
    -392,  -392,  -392,  -392,   219,   219,   234,   234,   179,   179,
     179,   179,    18,    18,    19,   298,   300,   304,   347,    21,
    -392,  -392,   121,  -392,   160,   367,  2511,  -392,  -392,  -392,
     363,  -392,  -392,  3617,  -392,  -392,   368,   152,  -392,  -392,
    -392,  -392,  -392,  -392,  -392,  2941,  -392,  -392,  -392,   371,
    3199,   364,  -392,    48,  -392,   216,  -392,  -392,  -392,     5,
    -392,   216,  -392,   438,  -392,  -392,  -392,  -392,  -392,  2207,
     250,    65,   366,   257,   406,  2650,  2941,  2941,  -392,   261,
    -392,  2207,  2941,  -392,  -392,  -392,    92,   -27,  -392,    -7,
     369,   215,  -392,   216,  -392,  4351,   374,   372,   373,  -392,
       6,   378,  -392,   375,  -392,   376,  3718,  -392,  -392,  -392,
    -392,    70,  -392,   377,   381,  -392,  -392,  -392,  -392,  -392,
     382,   385,  2941,  1638,   386,  2650,  3415,  2650,   265,   270,
    -392,   384,  -392,   197,  -392,  -392,   383,  -392,  1790,   391,
     106,  -392,  -392,  -392,   216,   216,  -392,  -392,  4433,  2511,
     216,   392,   394,  -392,  2207,  2207,   404,   387,  -392,   520,
    -392,  2941,  2789,  2789,  1638,   399,  -392,  -392,  -392,  -392,
    -392,   397,  -392,   216,   210,   191,  -392,  -392,   321,  -392,
     393,   395,   396,  1929,  1638,   272,   398,   400,  -392,   246,
     256,  -392,  -392,  -392,   216,  2941,  -392,   216,  -392,  -392,
     401,   405,  -392,   182,   408,  -392,   411,  1638,  1638,  2941,
     409,  -392,    42,  -392,   407,   128,   221,   392,   301,  -392,
    -392,  2941,   308,   402,  -392,  -392,  -392,  -392,    61,   706,
    -392,  -392,  -392,   275,  2941,  -392,   216,  -392,   413,   866,
    1024,  -392,   216,   134,   412,  -392,  1182,   414,   279,  -392,
    -392,  -392,   216,   416,  -392
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       2,   213,     1,   213,   213,   213,   213,   213,     0,   213,
     213,   213,   213,   213,   213,   213,     0,   213,   213,   213,
     213,   213,   213,   213,   213,    13,    13,    13,   213,   213,
     213,   213,   213,   213,     0,    77,    78,    81,    10,     0,
       3,     0,    28,    29,    30,     8,     9,     0,     7,   175,
      62,   213,     4,    69,     0,    79,    80,   213,     6,   176,
     174,     0,   186,     0,   216,   226,   224,   231,   214,    48,
      50,    51,    49,    52,    18,   220,   227,   230,   229,   234,
     235,   217,    12,   215,   218,   228,   236,   225,   219,   233,
     221,    18,    14,    18,    18,   237,   238,     0,   222,   232,
     223,    42,     0,     2,    40,     5,    63,    70,     0,    67,
     213,    66,   239,   183,     0,   171,   104,   150,     0,    38,
     165,   161,   162,   166,   168,   167,   133,   130,   138,   134,
     164,   129,   122,   123,   124,   125,   126,   127,   128,   170,
     160,   115,   155,   116,   117,   118,   119,   120,   121,   139,
     141,   142,   143,   144,   140,   156,     0,   105,    39,   145,
     106,   110,   107,   111,   113,   114,   108,   109,   112,   131,
       0,   136,   163,   173,   157,     0,     0,    44,    45,   146,
       0,     0,     0,     0,     0,   172,     0,    76,     0,     0,
       0,     0,   178,   177,     0,    60,   213,   213,   213,     0,
       0,     0,     0,     0,     0,     0,   249,   288,   213,     0,
     248,   213,   213,   283,   284,   285,    77,    78,   287,    71,
     213,   213,   213,   213,   213,    65,    74,     0,    79,   252,
       0,   286,   213,   250,   255,   256,   257,   258,     0,   253,
     296,   303,   311,   315,   318,   321,   326,   329,   331,   333,
     335,   337,   339,   341,   354,   279,   211,   203,   187,     0,
       0,   159,    55,    53,   158,     0,    82,    43,   207,   213,
     213,   213,   213,     0,     0,    48,    50,     0,   208,   213,
      73,    73,    73,    73,   211,     0,   180,     0,   246,     0,
     245,    11,    37,    41,     0,   159,   259,     0,     0,   260,
     263,   262,     0,   189,   304,   305,     0,     0,   306,   307,
     308,   309,     0,     0,     0,   254,   213,     0,    72,   251,
     280,   297,   298,   213,     0,   349,   350,   344,   345,   346,
     347,   348,   351,   352,   353,   343,   213,   213,   213,   213,
     213,   213,   213,   213,   213,   213,   213,   213,   213,   213,
     213,   213,   213,   213,   213,   213,   213,   213,   211,    82,
     189,   213,     0,     0,   213,     0,     0,     0,    18,     0,
       0,     0,     0,     0,     0,     0,    73,    19,   213,     0,
       0,     0,     0,    23,     0,     0,   182,   179,     0,     0,
       0,   213,     0,     0,     0,   261,   197,   211,   290,   213,
     213,     0,     0,     0,   282,     0,   213,     0,   299,   342,
     312,   313,   314,   311,   316,   317,   319,   320,   324,   325,
     322,   323,   327,   328,   330,   332,   334,   336,   338,     0,
     355,   209,     0,   212,    18,   197,   213,   204,   240,   169,
       0,    47,    56,     0,    54,    55,     0,     0,    36,    83,
      84,    85,   147,   132,   148,   213,   151,   152,   153,     0,
     213,     0,    20,     0,    16,     0,    31,    32,    33,     0,
      25,    13,    21,    24,    26,    27,   181,    61,   247,   213,
       0,     0,     0,     0,     0,   213,   213,   213,   293,     0,
     300,   213,   213,   210,   188,   197,     0,     0,   243,     0,
       0,     0,    58,     0,    57,   213,     0,     0,     0,    86,
       0,     0,    97,    98,    99,   101,     0,    82,    17,    35,
      34,    18,    22,     0,     0,   194,   191,   192,   193,   294,
     190,   195,   213,   213,     0,   213,     0,   213,     0,     0,
     301,     0,   340,     0,   205,   206,     0,   241,   213,     0,
       0,   184,    46,   135,     0,     0,   149,   154,   213,   213,
       0,    68,     0,   291,   213,   213,     0,     0,   310,   265,
     264,   213,   213,   213,   213,     0,   302,   201,   197,   242,
     244,     0,    59,     0,     0,     0,   100,   102,   211,   189,
       0,     0,     0,   213,   213,     0,     0,     0,   267,     0,
       0,   137,   185,    94,     0,   213,    87,     0,    82,   197,
       0,     0,   196,     0,     0,   266,     0,   213,   213,   213,
       0,   276,     0,   202,     0,     0,     0,   103,     0,   292,
     200,   213,     0,     0,   198,   268,   270,   269,     0,   213,
     271,   277,    95,     0,   213,    89,     0,    15,     0,   213,
     213,    88,     0,     0,     0,   199,   213,     0,     0,    92,
      90,    91,     0,     0,    93
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -392,   457,  -392,  -392,    -2,    90,  -392,   -54,  -392,   156,
    -392,  -324,  -297,  -392,    98,  -392,  -392,  -392,  -392,  -392,
    -392,  -392,  -392,   -96,  -392,   129,  -392,    26,  -392,   137,
    -392,  -392,     4,    43,  -392,  -392,   527,  -392,   357,  -177,
    -392,  -392,  -392,    57,  -214,  -392,  -352,  -392,  -392,  -392,
    -392,  -392,    13,  -392,  -392,  -392,  -392,  -392,  -392,   115,
    -392,    63,   -15,    40,  -392,  -392,  -392,   195,   100,  -392,
    -225,  -392,  -325,  -392,  -392,  -388,  -392,   -81,  -392,  -392,
    -248,    -1,  -391,  -392,  -318,  -181,  -392,  -372,  -186,  -392,
    -392,  -392,  -392,  -392,   -35,  -392,  -126,  -374,  -341,   -25,
    -392,   353,   110,   111,  -245,   107,   239,   240,   238,   241,
     242,  -392,  -392,   -74,  -392,  -182
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,    40,    41,    91,   472,   378,   188,   473,   382,
      42,    43,    44,    45,   468,   469,    46,    47,   105,   175,
     198,    48,    49,   176,   177,   178,    73,   256,   366,   364,
     503,   442,    50,   225,   226,    52,    53,    54,   111,   227,
     313,   314,   279,   228,    56,    57,   368,   449,   450,   451,
     461,   513,   514,   515,   179,   180,   181,   182,   183,   369,
     184,   185,   317,   229,   194,    59,   285,   286,    60,   550,
      61,   257,   396,   531,   567,   481,   497,   278,    62,   358,
     359,   287,   437,   499,   288,   404,   231,   232,   233,   234,
     235,   570,   236,   237,   621,   622,   238,   239,   405,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   336,   255
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      63,   192,    64,    65,    66,    67,    68,   434,    75,    76,
      77,    78,    79,    80,    81,   289,    83,    84,    85,    86,
      87,    88,    89,    90,    93,    94,   302,    95,    96,    97,
      98,    99,   100,   258,    74,   435,   386,   189,   307,   190,
     191,    58,    82,   438,    51,   498,   319,   496,   186,   619,
     108,    92,    92,    92,   379,   106,   112,   466,    55,   620,
      55,    55,    55,    55,    55,   102,    55,    55,    55,    55,
      55,    55,    55,   478,    55,    55,    55,    55,    55,    55,
      55,    55,   193,   393,   467,    55,    55,    55,    55,    55,
      55,   312,   400,   200,   525,   268,    26,    27,   401,   418,
     419,   420,   421,   381,   383,   383,   383,   543,    55,   230,
     433,   537,   360,   354,    55,   526,   321,   322,   438,   454,
     545,   525,   290,   361,   325,   455,   326,   342,   546,   343,
     327,   328,   329,   330,   331,   332,   333,   334,   523,   348,
     547,   407,   526,   344,   345,   466,   349,   -75,   548,   482,
     541,   323,   519,   463,   113,   556,   324,   580,   527,   335,
     387,   572,   355,   573,   114,   561,   346,   347,   587,   103,
     388,   492,   467,   429,   395,   432,   356,   528,   464,    69,
      70,    71,   263,    72,   356,   527,   402,   -49,   114,   640,
     600,   -49,   406,   518,   202,   203,    63,   294,   101,   460,
     507,   266,   614,   114,   528,    69,    70,    71,   206,    72,
     510,   649,   529,   104,   484,   508,   356,   483,   187,   306,
     284,   628,   259,   590,   591,   489,   525,   207,   297,   106,
     438,   230,    69,    70,    71,   260,    72,    58,   293,   544,
      51,   438,   290,   277,   362,   363,   490,   526,   264,   356,
     367,   582,   295,   619,    55,    55,   627,   277,   374,   375,
     209,   583,   409,   620,   609,   265,   524,   650,   370,   370,
     370,   370,   389,   493,   269,   438,   356,   656,   380,   392,
     643,   270,   430,   356,   465,   525,   658,   290,   210,   356,
     527,   397,    69,    70,    71,   271,    72,    69,    70,    71,
     342,   261,   343,   262,   538,   539,   526,   272,   187,   528,
     447,    69,    70,    71,   448,    72,   267,   290,   213,   214,
     215,   273,   216,   217,   394,   218,    55,    55,    55,    55,
     525,   357,   290,   406,   274,   631,    55,    69,    70,    71,
     608,    72,   605,   296,   577,   606,   607,   569,   280,   527,
     408,   526,   109,   110,    55,   453,    69,    70,    71,   295,
      72,   262,   290,   443,   603,   392,   604,   298,   528,   275,
     276,    71,   644,    72,   281,   645,   646,   380,   195,   196,
     494,   337,   338,   339,   268,   371,   372,   373,   598,   595,
      35,    36,   340,   341,   527,   282,    69,    70,    71,   230,
      72,   390,   391,   623,   283,   290,   398,   356,   615,    26,
      27,   533,   356,   528,   284,   540,   391,   290,   542,   574,
     356,    55,   551,   625,   575,   356,   616,   356,   504,   651,
     652,   636,   637,   661,   662,    55,   299,   638,   384,   385,
     465,    69,    70,    71,   300,    72,   596,   597,   647,   307,
     414,   415,   303,   416,   417,   422,   423,   110,    39,   516,
     320,   316,   653,   350,   319,   353,   351,   562,   356,   521,
     319,   352,   357,   509,   290,   365,   -77,   -78,   403,   376,
     399,   439,   444,   440,   536,   290,   445,   471,   446,   459,
     290,   290,   452,   456,   457,   458,   470,    92,   474,   475,
     500,   406,   602,   479,   443,   506,   477,   530,   511,   485,
     480,   486,   487,   495,   491,   391,   488,    55,   517,   290,
     532,   534,   530,   553,   549,   554,   555,   557,   581,   578,
     558,   563,   230,   559,   564,   565,   566,    55,   576,   571,
     589,   592,   447,   594,   593,   599,   601,   610,   612,   611,
     630,   648,   617,   634,   618,   629,   635,   516,   655,   639,
     197,   642,    55,   522,   304,   305,   659,   520,   660,   530,
     664,   586,   502,   230,   308,   309,   310,   311,   107,   560,
     584,   585,   505,   476,   315,   535,   588,   641,   633,   424,
     426,   425,     0,   230,   427,     0,   428,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    55,   230,   230,     0,     0,
       0,     0,     0,     0,     0,     0,   530,     0,     0,     0,
     624,     0,     0,   626,     0,     0,     0,     0,   230,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   230,   230,
       0,     0,     0,     0,   530,   230,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     632,     0,   654,     0,     0,     0,     0,     0,   657,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   663,     0,
     410,   411,   412,   413,   413,   413,   413,   413,   413,   413,
     413,   413,   413,   413,   413,   413,   413,   413,     0,     0,
     201,     0,     0,  -275,     0,     0,     3,     4,   202,   203,
       0,     5,   204,  -275,     0,     0,   205,   -64,     0,     0,
       6,     7,   206,   -64,     0,     0,     9,     0,   -64,    10,
      11,    12,     0,    13,    14,    15,     0,     0,     0,    17,
      18,   207,    19,     0,     0,     0,     0,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   208,     0,
      21,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   209,     0,    22,    23,     0,     0,
      24,     0,     0,     0,   -64,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   413,    28,
      29,     0,   210,    30,     0,    31,    32,     0,     0,     0,
      33,   -64,   211,   212,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   213,   214,   215,     0,   216,   217,    37,   218,
       0,  -278,   -73,  -275,     0,     0,     0,    39,     0,   220,
       0,     0,     0,     0,   221,   222,   223,   224,     0,     0,
     201,     0,     0,  -273,     0,     0,     3,     4,   202,   203,
       0,     5,   204,  -273,     0,   568,   205,   -64,     0,     0,
       6,     7,   206,   -64,     0,     0,     9,     0,   -64,    10,
      11,    12,     0,    13,    14,    15,     0,     0,     0,    17,
      18,   207,    19,     0,     0,     0,     0,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   208,     0,
      21,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   209,     0,    22,    23,     0,     0,
      24,     0,     0,     0,   -64,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    28,
      29,     0,   210,    30,     0,    31,    32,     0,     0,     0,
      33,   -64,   211,   212,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   213,   214,   215,     0,   216,   217,    37,   218,
       0,  -278,   -73,  -273,     0,     0,     0,    39,     0,   220,
       0,     0,     0,     0,   221,   222,   223,   224,   201,     0,
       0,  -274,     0,     0,     3,     4,   202,   203,     0,     5,
     204,  -274,     0,     0,   205,   -64,     0,     0,     6,     7,
     206,   -64,     0,     0,     9,     0,   -64,    10,    11,    12,
       0,    13,    14,    15,     0,     0,     0,    17,    18,   207,
      19,     0,     0,     0,     0,    20,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   208,     0,    21,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   209,     0,    22,    23,     0,     0,    24,     0,
       0,     0,   -64,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    28,    29,     0,
     210,    30,     0,    31,    32,     0,     0,     0,    33,   -64,
     211,   212,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     213,   214,   215,     0,   216,   217,    37,   218,     0,  -278,
     -73,  -274,     0,     0,     0,    39,     0,   220,     0,     0,
       0,     0,   221,   222,   223,   224,   201,     0,     0,  -272,
       0,     0,     3,     4,   202,   203,     0,     5,   204,  -272,
       0,     0,   205,   -64,     0,     0,     6,     7,   206,   -64,
       0,     0,     9,     0,   -64,    10,    11,    12,     0,    13,
      14,    15,     0,     0,     0,    17,    18,   207,    19,     0,
       0,     0,     0,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   208,     0,    21,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     209,     0,    22,    23,     0,     0,    24,     0,     0,     0,
     -64,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    28,    29,     0,   210,    30,
       0,    31,    32,     0,     0,     0,    33,   -64,   211,   212,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   213,   214,
     215,     0,   216,   217,    37,   218,     0,  -278,   -73,  -272,
       0,     0,     0,    39,     0,   220,     0,     0,   201,     0,
     221,   222,   223,   224,     3,     4,   202,   203,     0,     5,
     204,     0,     0,     0,   205,   -64,     0,     0,     6,     7,
     206,   -64,     0,     0,     9,     0,   -64,    10,    11,    12,
       0,    13,    14,    15,     0,     0,     0,    17,    18,   207,
      19,     0,     0,     0,     0,    20,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   208,     0,    21,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   209,     0,    22,    23,     0,     0,    24,     0,
       0,     0,   -64,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    28,    29,     0,
     210,    30,     0,    31,    32,     0,     0,     0,    33,   -64,
     211,   212,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     213,   214,   215,     0,   216,   217,    37,   218,     0,  -278,
     -73,   219,     0,     0,     0,    39,     0,   220,     0,     0,
     201,     0,   221,   222,   223,   224,     3,     4,   202,   203,
       0,     5,   204,     0,     0,     0,   205,   -64,     0,     0,
       6,     7,   206,   -64,     0,     0,     9,     0,   -64,    10,
      11,    12,     0,    13,    14,    15,     0,     0,     0,    17,
      18,   207,    19,     0,     0,     0,     0,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   208,     0,
      21,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   209,     0,    22,    23,     0,     0,
      24,     0,     0,     0,   -64,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    28,
      29,     0,   210,    30,     0,    31,    32,     0,     0,     0,
      33,   -64,   211,   212,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   213,   214,   215,     0,   216,   217,    37,   218,
       0,  -278,   -73,   318,     0,     0,     0,    39,     0,   220,
       0,     0,   201,     0,   221,   222,   223,   224,     3,     4,
     202,   203,     0,     5,   204,     0,     0,     0,   205,   -64,
       0,     0,     6,     7,   206,   -64,     0,     0,     9,     0,
     -64,    10,    11,    12,     0,    13,    14,    15,     0,     0,
       0,    17,    18,   207,    19,     0,     0,     0,     0,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     208,     0,    21,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   209,     0,    22,    23,
       0,     0,    24,     0,     0,     0,   -64,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    28,    29,     0,   210,    30,     0,    31,    32,     0,
       0,     0,    33,   -64,   211,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   213,   214,   215,     0,   216,   217,
      37,   218,     0,  -278,   -73,     0,     0,     0,     0,    39,
       0,   220,     0,     0,     0,     0,   221,   222,   223,   224,
       3,     4,   202,   203,     0,     5,     0,     0,     0,     0,
       0,     0,     0,     0,     6,     7,   206,     0,     0,     0,
       9,     0,     0,    10,    11,    12,     0,    13,    14,    15,
       0,     0,     0,    17,    18,   207,    19,     0,     0,     0,
       0,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   209,     0,
      22,    23,     0,     0,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    28,    29,     0,   210,     0,     0,    31,
      32,     0,     0,     0,    33,     0,   211,   212,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   213,   214,   215,     0,
     216,   217,    37,   218,     0,     0,   436,   579,     0,     3,
       4,   202,   203,   220,     5,     0,     0,     0,   221,   222,
     223,   224,     0,     6,     7,   206,     0,     0,     0,     9,
       0,     0,    10,    11,    12,     0,    13,    14,    15,     0,
       0,     0,    17,    18,   207,    19,     0,     0,     0,     0,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   209,     0,    22,
      23,     0,     0,    24,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    28,    29,     0,   210,     0,     0,    31,    32,
       0,     0,     0,    33,     0,   211,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   213,   214,   215,     0,   216,
     217,    37,   218,     0,     0,   436,     0,   613,     3,     4,
     202,   203,   220,     5,     0,     0,     0,   221,   222,   223,
     224,     0,     6,     7,   206,     0,     0,     0,     9,     0,
       0,    10,    11,    12,     0,    13,    14,    15,     0,     0,
       0,    17,    18,   207,    19,     0,     0,     0,     0,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    21,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   209,     0,    22,    23,
       0,     0,    24,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    28,    29,     0,   210,     0,     0,    31,    32,     0,
       0,     0,    33,     0,   211,   212,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   213,   214,   215,     0,   216,   217,
      37,   218,     0,   301,     0,     0,     0,     3,     4,   202,
     203,   220,     5,     0,     0,     0,   221,   222,   223,   224,
       0,     6,     7,   206,     0,     0,     0,     9,     0,     0,
      10,    11,    12,     0,    13,    14,    15,     0,     0,     0,
      17,    18,   207,    19,     0,     0,     0,     0,    20,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   209,     0,    22,    23,     0,
       0,    24,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      28,    29,     0,   210,     0,     0,    31,    32,     0,     0,
       0,    33,     0,   211,   212,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   213,   214,   215,     0,   216,   217,    37,
     218,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     220,  -281,     0,     0,     0,   221,   222,   223,   224,     3,
       4,   202,   203,     0,     5,     0,     0,     0,     0,     0,
       0,     0,     0,     6,     7,   206,     0,     0,     0,     9,
       0,     0,    10,    11,    12,     0,    13,    14,    15,     0,
       0,     0,    17,    18,   207,    19,     0,     0,     0,     0,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   209,     0,    22,
      23,     0,     0,    24,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    28,    29,     0,   210,     0,     0,    31,    32,
       0,     0,     0,    33,     0,   211,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   213,   214,   215,     0,   216,
     217,    37,   218,     0,     0,     0,     0,     0,     0,     0,
       0,   431,   220,     0,     0,     0,     0,   221,   222,   223,
     224,     3,     4,   202,   203,     0,     5,     0,     0,     0,
       0,     0,     0,     0,     0,     6,     7,   206,     0,     0,
       0,     9,     0,     0,    10,    11,    12,     0,    13,    14,
      15,     0,     0,     0,    17,    18,   207,    19,     0,     0,
       0,     0,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   209,
       0,    22,    23,     0,     0,    24,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    28,    29,     0,   210,     0,     0,
      31,    32,     0,     0,     0,    33,     0,   211,   212,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   213,   214,   215,
       0,   216,   217,    37,   218,     0,     0,   436,     0,     0,
       3,     4,   202,   203,   220,     5,     0,     0,     0,   221,
     222,   223,   224,     0,     6,     7,   206,     0,     0,     0,
       9,     0,     0,    10,    11,    12,     0,    13,    14,    15,
       0,     0,     0,    17,    18,   207,    19,     0,     0,     0,
       0,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   209,     0,
      22,    23,     0,     0,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    28,    29,     0,   210,     0,     0,    31,
      32,     0,     0,     0,    33,     0,   211,   212,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   213,   214,   215,     0,
     216,   217,    37,   218,     0,  -278,     0,     0,     0,     3,
       4,   202,   203,   220,     5,     0,     0,     0,   221,   222,
     223,   224,     0,     6,     7,   206,     0,     0,     0,     9,
       0,     0,    10,    11,    12,     0,    13,    14,    15,     0,
       0,     0,    17,    18,   207,    19,     0,     0,     0,     0,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   209,     0,    22,
      23,     0,     0,    24,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    28,    29,     0,   210,     0,     0,    31,    32,
       0,     0,     0,    33,     0,   211,   212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   213,   214,   215,     0,   216,
     217,    37,   218,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   220,  -278,     0,     0,     0,   221,   222,   223,
     224,     3,     4,   202,   203,     0,     5,     0,     0,     0,
       0,     0,     0,     0,     0,     6,     7,   206,     0,     0,
       0,     9,     0,     0,    10,    11,    12,     0,    13,    14,
      15,     0,     0,     0,    17,    18,   207,    19,     0,     0,
       0,     0,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   209,
       0,    22,    23,     0,     0,    24,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    28,    29,     0,   210,     0,     0,
      31,    32,     0,     0,     0,    33,     0,   211,   212,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   213,   214,   215,
       2,   216,   217,    37,   218,     0,     0,     0,     0,     0,
       3,     4,     0,     0,   220,     5,     0,     0,     0,   221,
     222,   223,   224,     0,     6,     7,     0,     0,     8,     0,
       9,     0,     0,    10,    11,    12,     0,    13,    14,    15,
       0,     0,    16,    17,    18,     0,    19,     0,     0,     0,
       0,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      22,    23,     0,     0,    24,     0,     0,     0,     0,     0,
      25,    26,    27,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    28,    29,     0,     0,    30,     0,    31,
      32,     0,     0,     0,    33,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     3,
       4,     0,     0,     0,     5,     0,     0,     0,     0,    34,
      35,    36,    37,     6,     7,    38,     0,     0,     0,     9,
       0,    39,    10,    11,    12,     0,    13,    14,    15,     0,
       0,     0,    17,    18,     0,    19,     0,     0,     0,     0,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    22,
      23,     0,     0,    24,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    28,    29,     0,     0,     0,     0,    31,    32,
       0,     0,   512,    33,     0,     0,     0,   115,     0,   116,
     117,     0,   118,   119,     0,     0,     0,     0,   120,     0,
       0,     0,   121,   122,     0,     0,   123,     0,     0,    35,
      36,    37,     0,   124,     0,   125,     0,     0,     0,     0,
     126,     0,     0,   -96,   127,   128,     0,     0,     0,     0,
       0,   129,     0,     0,   130,     0,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,     0,     0,     0,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,     0,   154,     0,     0,     0,     0,     0,   155,
     156,   157,     0,   158,     0,     0,     0,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,     0,   115,   169,
     116,   117,   170,   118,     0,   171,   172,   173,     0,   120,
       0,     0,     0,   121,   122,     0,     0,   123,     0,     0,
       0,     0,     0,     0,   124,     0,   125,     0,     0,     0,
       0,   126,     0,     0,     0,   127,   128,     0,   174,     0,
       0,     0,   129,     0,     0,   130,     0,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,     0,     0,     0,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,     0,   154,     0,     0,     0,     0,     0,
     155,   156,   157,     0,     0,     0,     0,     0,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,     0,   115,
     169,   116,   117,   170,   118,     0,   171,   172,   173,     0,
     120,     0,     0,     0,   121,   122,     0,     0,   123,     0,
       0,     0,     0,     0,     0,   124,     0,   125,     0,     0,
       0,     0,   126,     0,     0,     0,   127,   128,     0,   174,
       0,     0,     0,   129,     0,     0,   130,     0,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,     0,     0,
       0,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,     0,   154,     0,     0,     0,     0,
       0,   155,   199,   157,     0,     0,     0,     0,     0,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,     0,
     115,   169,   116,   117,   170,   118,     0,   171,   172,   173,
       0,   120,     0,     0,     0,   121,   122,     0,     0,   123,
       0,     0,     0,     0,     0,     0,   124,     0,   125,     0,
       0,     0,     0,   126,     0,     0,     0,   127,   128,     0,
     174,     0,     0,     0,   129,     0,     0,   130,     0,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,     0,
       0,     0,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,     0,   154,     0,     0,     0,
       0,     0,   155,   501,   157,     0,     0,     0,     0,     0,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
       0,   115,   169,   116,   117,   170,   118,     0,   171,   172,
     173,     0,   120,     0,     0,     0,   121,   122,     0,     0,
     123,     0,     0,     0,     0,     0,     0,   124,     0,   125,
       0,     0,     0,     0,   126,     0,     0,     0,   127,   128,
       0,   174,     0,     0,     0,   129,     0,     0,   130,     0,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
       0,     0,     0,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,     0,   154,     0,     0,
       0,     0,     0,   155,   199,   157,     0,     0,     0,     0,
       0,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,     0,     0,   169,     0,     0,   170,     3,     4,   171,
     172,     0,     5,     0,     0,     0,     0,     0,     0,     0,
       0,     6,     7,     0,     0,     8,     0,     9,     0,     0,
      10,    11,    12,     0,    13,    14,    15,     0,     0,    16,
      17,    18,   174,    19,     0,     0,     0,     0,    20,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    22,    23,     0,
       0,    24,     0,     0,     0,     0,     0,    25,    26,    27,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      28,    29,     0,     0,    30,     0,    31,    32,     0,     0,
       0,    33,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     3,     4,     0,     0,
       0,     5,     0,     0,     0,     0,    34,    35,    36,    37,
       6,     7,    38,     0,   291,     0,     9,     0,    39,    10,
      11,    12,     0,    13,    14,    15,     0,     0,     0,    17,
      18,     0,    19,     0,     0,     0,     0,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      21,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,    23,     0,     0,
      24,     0,     0,     0,     0,     0,     0,     0,     3,     4,
       0,     0,     0,     5,     0,     0,     0,     0,     0,    28,
      29,     0,     6,     7,     0,    31,    32,     0,     9,     0,
      33,    10,    11,    12,     0,    13,    14,    15,     0,     0,
       0,    17,    18,     0,    19,     0,     0,     0,     0,    20,
       0,     0,     0,     0,     0,     0,    35,    36,    37,     0,
       0,     0,    21,     0,     0,     0,     0,    39,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    22,    23,
       0,     0,    24,     0,     0,     0,     0,     0,     0,     0,
       3,     4,     0,     0,     0,     5,     0,     0,     0,     0,
       0,    28,    29,     0,     6,     7,     0,    31,    32,     0,
       9,     0,    33,    10,    11,    12,     0,    13,    14,    15,
       0,     0,     0,    17,    18,     0,    19,     0,     0,     0,
       0,    20,     0,     0,     0,     0,     0,     0,    35,    36,
      37,     0,     0,     0,    21,     0,     0,   377,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      22,    23,     0,     0,    24,     0,     0,     0,     0,     0,
       0,     0,     3,     4,     0,     0,     0,     5,     0,     0,
       0,     0,     0,    28,    29,     0,     6,     7,     0,    31,
      32,     0,     9,     0,    33,    10,    11,    12,     0,    13,
      14,    15,     0,     0,     0,    17,    18,     0,    19,     0,
       0,     0,     0,    20,     0,     0,     0,     0,     0,     0,
      35,    36,    37,     0,     0,     0,    21,     0,     0,   462,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    22,    23,     0,     0,    24,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     3,
       4,     0,     0,     0,     5,    28,    29,     0,     0,    30,
       0,    31,    32,     6,     7,     0,    33,     0,     0,     9,
       0,     0,    10,    11,    12,     0,    13,    14,    15,     0,
       0,     0,    17,    18,     0,    19,     0,     0,     0,     0,
      20,     0,    35,    36,    37,     0,     0,     0,     0,   292,
       0,     0,     0,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    22,
      23,     0,     0,    24,     0,     0,     0,     0,     0,     0,
       0,     3,     4,     0,     0,     0,     5,     0,     0,     0,
       0,     0,    28,    29,     0,     6,     7,     0,    31,    32,
       0,     9,     0,    33,    10,    11,    12,     0,    13,    14,
      15,     0,     0,     0,    17,    18,     0,    19,     0,     0,
       0,     0,    20,     0,     0,     0,     0,     0,     0,    35,
      36,    37,     0,     0,     0,    21,   441,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,    23,     0,     0,    24,     0,     0,     0,     0,
       0,     0,     0,     3,     4,     0,     0,     0,     5,     0,
       0,     0,     0,     0,    28,    29,     0,     6,     7,     0,
      31,    32,     0,     9,     0,    33,    10,    11,    12,     0,
      13,    14,    15,     0,     0,     0,    17,    18,     0,    19,
       0,     0,     0,     0,    20,     0,     0,     0,     0,     0,
       0,    35,    36,    37,     0,     0,     0,    21,   552,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    22,    23,     0,     0,    24,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    28,    29,     0,     0,
       0,     0,    31,    32,     0,     0,     0,    33,  -289,  -289,
    -289,  -289,  -289,  -289,  -289,  -289,  -289,  -289,  -289,  -289,
    -289,  -289,  -289,  -289,  -289,  -289,  -289,  -289,     0,     0,
       0,     0,     0,    35,    36,    37,     0,  -289,     0,  -289,
    -289,  -289,  -289,  -289,  -289,     0,  -289,  -289,  -289,  -289,
    -289,  -289,     0,     0,  -289,  -289,  -289,  -289,  -289,  -289,
    -289,  -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,
    -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,  -295,
    -295,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    -295,     0,  -295,  -295,  -295,  -295,  -295,  -295,     0,  -295,
    -295,  -295,  -295,  -295,  -295,     0,     0,  -295,  -295,  -295,
    -295,  -295,  -295,  -295
};

static const yytype_int16 yycheck[] =
{
       1,    97,     3,     4,     5,     6,     7,   359,     9,    10,
      11,    12,    13,    14,    15,   196,    17,    18,    19,    20,
      21,    22,    23,    24,    26,    27,   208,    28,    29,    30,
      31,    32,    33,   114,     8,   360,   284,    91,   220,    93,
      94,     1,    16,   361,     1,   436,   232,   435,    63,     7,
      51,    25,    26,    27,   279,    51,    57,   381,     1,    17,
       3,     4,     5,     6,     7,    39,     9,    10,    11,    12,
      13,    14,    15,   391,    17,    18,    19,    20,    21,    22,
      23,    24,    97,   297,   381,    28,    29,    30,    31,    32,
      33,    32,    21,   108,    29,   176,    91,    92,    27,   344,
     345,   346,   347,   280,   281,   282,   283,   495,    51,   110,
     358,   485,   146,   119,    57,    50,   116,   117,   436,   149,
     147,    29,   196,   157,   122,   155,   124,   121,   155,   123,
     128,   129,   130,   131,   132,   133,   134,   135,   479,   120,
     147,   323,    50,   125,   126,   469,   127,    88,   155,   397,
     491,   151,   147,   378,   145,   149,   156,   548,    93,   157,
     145,   535,   168,   537,   155,   517,   148,   149,   559,   146,
     155,   150,   469,   355,   145,   357,   155,   112,   145,   140,
     141,   142,   156,   144,   155,    93,   115,   146,   155,   147,
     578,   150,   153,   145,    12,    13,   197,   198,   143,   376,
      48,   175,   593,   155,   112,   140,   141,   142,    26,   144,
     455,   150,   147,   146,   400,    63,   155,   399,   148,   220,
     194,   609,   148,   564,   565,   406,    29,    45,   202,   225,
     548,   232,   140,   141,   142,   148,   144,   197,   198,   147,
     197,   559,   316,   186,   259,   260,   152,    50,   144,   155,
     265,   145,   144,     7,   197,   198,   608,   200,   273,   274,
      78,   155,   336,    17,   589,   148,   480,   639,   269,   270,
     271,   272,   287,   152,   148,   593,   155,   649,   279,   294,
     152,   148,   356,   155,   380,    29,   152,   361,   106,   155,
      93,   306,   140,   141,   142,   148,   144,   140,   141,   142,
     121,   144,   123,   146,   486,   487,    50,   148,   148,   112,
     150,   140,   141,   142,   368,   144,   145,   391,   136,   137,
     138,   148,   140,   141,   298,   143,   269,   270,   271,   272,
      29,   151,   406,   153,   148,   153,   279,   140,   141,   142,
     588,   144,   151,   145,   147,   154,   155,   533,   146,    93,
     324,    50,   145,   146,   297,   370,   140,   141,   142,   144,
     144,   146,   436,   364,   154,   380,   156,   153,   112,   140,
     141,   142,   151,   144,   146,   154,   155,   378,   152,   153,
     434,   162,   163,   164,   465,   270,   271,   272,   574,   571,
     140,   141,   158,   159,    93,   146,   140,   141,   142,   400,
     144,   154,   155,   147,   146,   479,   154,   155,   594,    91,
      92,   154,   155,   112,   388,   154,   155,   491,   492,   154,
     155,   364,   503,   605,   154,   155,   154,   155,   443,   154,
     155,   617,   618,   154,   155,   378,   145,   619,   282,   283,
     536,   140,   141,   142,   145,   144,   572,   573,   147,   631,
     340,   341,   146,   342,   343,   348,   349,   146,   151,   460,
     145,   153,   644,   165,   650,   118,   166,   521,   155,   471,
     656,   167,   151,   447,   548,   150,   153,   153,    88,   153,
     153,   149,   144,   155,   485,   559,   146,    49,   155,   155,
     564,   565,   149,   149,   149,   149,   147,   471,   147,   147,
     137,   153,   583,   153,   505,   137,   152,   481,   137,   153,
     155,   153,   153,   146,   153,   155,   154,   460,   154,   593,
     154,   115,   496,   149,   155,   153,   153,   149,   137,   146,
     155,   154,   533,   157,   153,   153,   151,   480,   154,   153,
     146,   137,   150,    23,   157,   146,   149,   154,   152,   154,
     145,   149,   154,   145,   154,   154,   145,   558,   145,   150,
     103,   154,   505,   473,   211,   212,   154,   469,   154,   543,
     154,   558,   443,   574,   221,   222,   223,   224,    51,   516,
     554,   555,   445,   388,   227,   485,   560,   622,   613,   350,
     352,   351,    -1,   594,   353,    -1,   354,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   558,   617,   618,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   600,    -1,    -1,    -1,
     604,    -1,    -1,   607,    -1,    -1,    -1,    -1,   639,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   649,   650,
      -1,    -1,    -1,    -1,   628,   656,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     613,    -1,   646,    -1,    -1,    -1,    -1,    -1,   652,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   662,    -1,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,    -1,    -1,
       4,    -1,    -1,     7,    -1,    -1,    10,    11,    12,    13,
      -1,    15,    16,    17,    -1,    -1,    20,    21,    -1,    -1,
      24,    25,    26,    27,    -1,    -1,    30,    -1,    32,    33,
      34,    35,    -1,    37,    38,    39,    -1,    -1,    -1,    43,
      44,    45,    46,    -1,    -1,    -1,    -1,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    -1,
      64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    78,    -1,    80,    81,    -1,    -1,
      84,    -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   455,   103,
     104,    -1,   106,   107,    -1,   109,   110,    -1,    -1,    -1,
     114,   115,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,   137,   138,    -1,   140,   141,   142,   143,
      -1,   145,   146,   147,    -1,    -1,    -1,   151,    -1,   153,
      -1,    -1,    -1,    -1,   158,   159,   160,   161,    -1,    -1,
       4,    -1,    -1,     7,    -1,    -1,    10,    11,    12,    13,
      -1,    15,    16,    17,    -1,   532,    20,    21,    -1,    -1,
      24,    25,    26,    27,    -1,    -1,    30,    -1,    32,    33,
      34,    35,    -1,    37,    38,    39,    -1,    -1,    -1,    43,
      44,    45,    46,    -1,    -1,    -1,    -1,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    -1,
      64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    78,    -1,    80,    81,    -1,    -1,
      84,    -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
     104,    -1,   106,   107,    -1,   109,   110,    -1,    -1,    -1,
     114,   115,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,   137,   138,    -1,   140,   141,   142,   143,
      -1,   145,   146,   147,    -1,    -1,    -1,   151,    -1,   153,
      -1,    -1,    -1,    -1,   158,   159,   160,   161,     4,    -1,
      -1,     7,    -1,    -1,    10,    11,    12,    13,    -1,    15,
      16,    17,    -1,    -1,    20,    21,    -1,    -1,    24,    25,
      26,    27,    -1,    -1,    30,    -1,    32,    33,    34,    35,
      -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,
      46,    -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    62,    -1,    64,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    -1,    80,    81,    -1,    -1,    84,    -1,
      -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,   104,    -1,
     106,   107,    -1,   109,   110,    -1,    -1,    -1,   114,   115,
     116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     136,   137,   138,    -1,   140,   141,   142,   143,    -1,   145,
     146,   147,    -1,    -1,    -1,   151,    -1,   153,    -1,    -1,
      -1,    -1,   158,   159,   160,   161,     4,    -1,    -1,     7,
      -1,    -1,    10,    11,    12,    13,    -1,    15,    16,    17,
      -1,    -1,    20,    21,    -1,    -1,    24,    25,    26,    27,
      -1,    -1,    30,    -1,    32,    33,    34,    35,    -1,    37,
      38,    39,    -1,    -1,    -1,    43,    44,    45,    46,    -1,
      -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    62,    -1,    64,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    -1,    80,    81,    -1,    -1,    84,    -1,    -1,    -1,
      88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   103,   104,    -1,   106,   107,
      -1,   109,   110,    -1,    -1,    -1,   114,   115,   116,   117,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,   137,
     138,    -1,   140,   141,   142,   143,    -1,   145,   146,   147,
      -1,    -1,    -1,   151,    -1,   153,    -1,    -1,     4,    -1,
     158,   159,   160,   161,    10,    11,    12,    13,    -1,    15,
      16,    -1,    -1,    -1,    20,    21,    -1,    -1,    24,    25,
      26,    27,    -1,    -1,    30,    -1,    32,    33,    34,    35,
      -1,    37,    38,    39,    -1,    -1,    -1,    43,    44,    45,
      46,    -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    62,    -1,    64,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    78,    -1,    80,    81,    -1,    -1,    84,    -1,
      -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,   104,    -1,
     106,   107,    -1,   109,   110,    -1,    -1,    -1,   114,   115,
     116,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     136,   137,   138,    -1,   140,   141,   142,   143,    -1,   145,
     146,   147,    -1,    -1,    -1,   151,    -1,   153,    -1,    -1,
       4,    -1,   158,   159,   160,   161,    10,    11,    12,    13,
      -1,    15,    16,    -1,    -1,    -1,    20,    21,    -1,    -1,
      24,    25,    26,    27,    -1,    -1,    30,    -1,    32,    33,
      34,    35,    -1,    37,    38,    39,    -1,    -1,    -1,    43,
      44,    45,    46,    -1,    -1,    -1,    -1,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    -1,
      64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    78,    -1,    80,    81,    -1,    -1,
      84,    -1,    -1,    -1,    88,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   103,
     104,    -1,   106,   107,    -1,   109,   110,    -1,    -1,    -1,
     114,   115,   116,   117,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,   137,   138,    -1,   140,   141,   142,   143,
      -1,   145,   146,   147,    -1,    -1,    -1,   151,    -1,   153,
      -1,    -1,     4,    -1,   158,   159,   160,   161,    10,    11,
      12,    13,    -1,    15,    16,    -1,    -1,    -1,    20,    21,
      -1,    -1,    24,    25,    26,    27,    -1,    -1,    30,    -1,
      32,    33,    34,    35,    -1,    37,    38,    39,    -1,    -1,
      -1,    43,    44,    45,    46,    -1,    -1,    -1,    -1,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      62,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,    80,    81,
      -1,    -1,    84,    -1,    -1,    -1,    88,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,   104,    -1,   106,   107,    -1,   109,   110,    -1,
      -1,    -1,   114,   115,   116,   117,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   136,   137,   138,    -1,   140,   141,
     142,   143,    -1,   145,   146,    -1,    -1,    -1,    -1,   151,
      -1,   153,    -1,    -1,    -1,    -1,   158,   159,   160,   161,
      10,    11,    12,    13,    -1,    15,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    24,    25,    26,    -1,    -1,    -1,
      30,    -1,    -1,    33,    34,    35,    -1,    37,    38,    39,
      -1,    -1,    -1,    43,    44,    45,    46,    -1,    -1,    -1,
      -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,
      80,    81,    -1,    -1,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,   104,    -1,   106,    -1,    -1,   109,
     110,    -1,    -1,    -1,   114,    -1,   116,   117,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   136,   137,   138,    -1,
     140,   141,   142,   143,    -1,    -1,   146,   147,    -1,    10,
      11,    12,    13,   153,    15,    -1,    -1,    -1,   158,   159,
     160,   161,    -1,    24,    25,    26,    -1,    -1,    -1,    30,
      -1,    -1,    33,    34,    35,    -1,    37,    38,    39,    -1,
      -1,    -1,    43,    44,    45,    46,    -1,    -1,    -1,    -1,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,    80,
      81,    -1,    -1,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,   104,    -1,   106,    -1,    -1,   109,   110,
      -1,    -1,    -1,   114,    -1,   116,   117,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   136,   137,   138,    -1,   140,
     141,   142,   143,    -1,    -1,   146,    -1,   148,    10,    11,
      12,    13,   153,    15,    -1,    -1,    -1,   158,   159,   160,
     161,    -1,    24,    25,    26,    -1,    -1,    -1,    30,    -1,
      -1,    33,    34,    35,    -1,    37,    38,    39,    -1,    -1,
      -1,    43,    44,    45,    46,    -1,    -1,    -1,    -1,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,    80,    81,
      -1,    -1,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   103,   104,    -1,   106,    -1,    -1,   109,   110,    -1,
      -1,    -1,   114,    -1,   116,   117,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   136,   137,   138,    -1,   140,   141,
     142,   143,    -1,   145,    -1,    -1,    -1,    10,    11,    12,
      13,   153,    15,    -1,    -1,    -1,   158,   159,   160,   161,
      -1,    24,    25,    26,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    35,    -1,    37,    38,    39,    -1,    -1,    -1,
      43,    44,    45,    46,    -1,    -1,    -1,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    -1,    80,    81,    -1,
      -1,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,   104,    -1,   106,    -1,    -1,   109,   110,    -1,    -1,
      -1,   114,    -1,   116,   117,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   136,   137,   138,    -1,   140,   141,   142,
     143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     153,   154,    -1,    -1,    -1,   158,   159,   160,   161,    10,
      11,    12,    13,    -1,    15,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    24,    25,    26,    -1,    -1,    -1,    30,
      -1,    -1,    33,    34,    35,    -1,    37,    38,    39,    -1,
      -1,    -1,    43,    44,    45,    46,    -1,    -1,    -1,    -1,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,    80,
      81,    -1,    -1,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,   104,    -1,   106,    -1,    -1,   109,   110,
      -1,    -1,    -1,   114,    -1,   116,   117,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   136,   137,   138,    -1,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   152,   153,    -1,    -1,    -1,    -1,   158,   159,   160,
     161,    10,    11,    12,    13,    -1,    15,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    24,    25,    26,    -1,    -1,
      -1,    30,    -1,    -1,    33,    34,    35,    -1,    37,    38,
      39,    -1,    -1,    -1,    43,    44,    45,    46,    -1,    -1,
      -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      -1,    80,    81,    -1,    -1,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,   104,    -1,   106,    -1,    -1,
     109,   110,    -1,    -1,    -1,   114,    -1,   116,   117,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,   137,   138,
      -1,   140,   141,   142,   143,    -1,    -1,   146,    -1,    -1,
      10,    11,    12,    13,   153,    15,    -1,    -1,    -1,   158,
     159,   160,   161,    -1,    24,    25,    26,    -1,    -1,    -1,
      30,    -1,    -1,    33,    34,    35,    -1,    37,    38,    39,
      -1,    -1,    -1,    43,    44,    45,    46,    -1,    -1,    -1,
      -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,
      80,    81,    -1,    -1,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,   104,    -1,   106,    -1,    -1,   109,
     110,    -1,    -1,    -1,   114,    -1,   116,   117,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   136,   137,   138,    -1,
     140,   141,   142,   143,    -1,   145,    -1,    -1,    -1,    10,
      11,    12,    13,   153,    15,    -1,    -1,    -1,   158,   159,
     160,   161,    -1,    24,    25,    26,    -1,    -1,    -1,    30,
      -1,    -1,    33,    34,    35,    -1,    37,    38,    39,    -1,
      -1,    -1,    43,    44,    45,    46,    -1,    -1,    -1,    -1,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,    80,
      81,    -1,    -1,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,   104,    -1,   106,    -1,    -1,   109,   110,
      -1,    -1,    -1,   114,    -1,   116,   117,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   136,   137,   138,    -1,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   153,   154,    -1,    -1,    -1,   158,   159,   160,
     161,    10,    11,    12,    13,    -1,    15,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    24,    25,    26,    -1,    -1,
      -1,    30,    -1,    -1,    33,    34,    35,    -1,    37,    38,
      39,    -1,    -1,    -1,    43,    44,    45,    46,    -1,    -1,
      -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,
      -1,    80,    81,    -1,    -1,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   103,   104,    -1,   106,    -1,    -1,
     109,   110,    -1,    -1,    -1,   114,    -1,   116,   117,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,   137,   138,
       0,   140,   141,   142,   143,    -1,    -1,    -1,    -1,    -1,
      10,    11,    -1,    -1,   153,    15,    -1,    -1,    -1,   158,
     159,   160,   161,    -1,    24,    25,    -1,    -1,    28,    -1,
      30,    -1,    -1,    33,    34,    35,    -1,    37,    38,    39,
      -1,    -1,    42,    43,    44,    -1,    46,    -1,    -1,    -1,
      -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      80,    81,    -1,    -1,    84,    -1,    -1,    -1,    -1,    -1,
      90,    91,    92,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   103,   104,    -1,    -1,   107,    -1,   109,
     110,    -1,    -1,    -1,   114,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    -1,    -1,    -1,    15,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,    24,    25,   145,    -1,    -1,    -1,    30,
      -1,   151,    33,    34,    35,    -1,    37,    38,    39,    -1,
      -1,    -1,    43,    44,    -1,    46,    -1,    -1,    -1,    -1,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,
      81,    -1,    -1,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   103,   104,    -1,    -1,    -1,    -1,   109,   110,
      -1,    -1,   113,   114,    -1,    -1,    -1,     3,    -1,     5,
       6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,    -1,
      -1,    -1,    18,    19,    -1,    -1,    22,    -1,    -1,   140,
     141,   142,    -1,    29,    -1,    31,    -1,    -1,    -1,    -1,
      36,    -1,    -1,   154,    40,    41,    -1,    -1,    -1,    -1,
      -1,    47,    -1,    -1,    50,    -1,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    -1,    -1,    -1,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,    85,
      86,    87,    -1,    89,    -1,    -1,    -1,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,    -1,     3,   105,
       5,     6,   108,     8,    -1,   111,   112,   113,    -1,    14,
      -1,    -1,    -1,    18,    19,    -1,    -1,    22,    -1,    -1,
      -1,    -1,    -1,    -1,    29,    -1,    31,    -1,    -1,    -1,
      -1,    36,    -1,    -1,    -1,    40,    41,    -1,   144,    -1,
      -1,    -1,    47,    -1,    -1,    50,    -1,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    -1,    -1,    -1,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      85,    86,    87,    -1,    -1,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,    -1,     3,
     105,     5,     6,   108,     8,    -1,   111,   112,   113,    -1,
      14,    -1,    -1,    -1,    18,    19,    -1,    -1,    22,    -1,
      -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    -1,    -1,
      -1,    -1,    36,    -1,    -1,    -1,    40,    41,    -1,   144,
      -1,    -1,    -1,    47,    -1,    -1,    50,    -1,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    -1,    -1,
      -1,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    85,    86,    87,    -1,    -1,    -1,    -1,    -1,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,    -1,
       3,   105,     5,     6,   108,     8,    -1,   111,   112,   113,
      -1,    14,    -1,    -1,    -1,    18,    19,    -1,    -1,    22,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,    -1,
      -1,    -1,    -1,    36,    -1,    -1,    -1,    40,    41,    -1,
     144,    -1,    -1,    -1,    47,    -1,    -1,    50,    -1,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    -1,
      -1,    -1,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    85,    86,    87,    -1,    -1,    -1,    -1,    -1,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
      -1,     3,   105,     5,     6,   108,     8,    -1,   111,   112,
     113,    -1,    14,    -1,    -1,    -1,    18,    19,    -1,    -1,
      22,    -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    31,
      -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    40,    41,
      -1,   144,    -1,    -1,    -1,    47,    -1,    -1,    50,    -1,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      -1,    -1,    -1,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    85,    86,    87,    -1,    -1,    -1,    -1,
      -1,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,    -1,    -1,   105,    -1,    -1,   108,    10,    11,   111,
     112,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    24,    25,    -1,    -1,    28,    -1,    30,    -1,    -1,
      33,    34,    35,    -1,    37,    38,    39,    -1,    -1,    42,
      43,    44,   144,    46,    -1,    -1,    -1,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,    81,    -1,
      -1,    84,    -1,    -1,    -1,    -1,    -1,    90,    91,    92,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     103,   104,    -1,    -1,   107,    -1,   109,   110,    -1,    -1,
      -1,   114,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    11,    -1,    -1,
      -1,    15,    -1,    -1,    -1,    -1,   139,   140,   141,   142,
      24,    25,   145,    -1,   147,    -1,    30,    -1,   151,    33,
      34,    35,    -1,    37,    38,    39,    -1,    -1,    -1,    43,
      44,    -1,    46,    -1,    -1,    -1,    -1,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    80,    81,    -1,    -1,
      84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,    11,
      -1,    -1,    -1,    15,    -1,    -1,    -1,    -1,    -1,   103,
     104,    -1,    24,    25,    -1,   109,   110,    -1,    30,    -1,
     114,    33,    34,    35,    -1,    37,    38,    39,    -1,    -1,
      -1,    43,    44,    -1,    46,    -1,    -1,    -1,    -1,    51,
      -1,    -1,    -1,    -1,    -1,    -1,   140,   141,   142,    -1,
      -1,    -1,    64,    -1,    -1,    -1,    -1,   151,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,    81,
      -1,    -1,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    11,    -1,    -1,    -1,    15,    -1,    -1,    -1,    -1,
      -1,   103,   104,    -1,    24,    25,    -1,   109,   110,    -1,
      30,    -1,   114,    33,    34,    35,    -1,    37,    38,    39,
      -1,    -1,    -1,    43,    44,    -1,    46,    -1,    -1,    -1,
      -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
     142,    -1,    -1,    -1,    64,    -1,    -1,   149,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      80,    81,    -1,    -1,    84,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    11,    -1,    -1,    -1,    15,    -1,    -1,
      -1,    -1,    -1,   103,   104,    -1,    24,    25,    -1,   109,
     110,    -1,    30,    -1,   114,    33,    34,    35,    -1,    37,
      38,    39,    -1,    -1,    -1,    43,    44,    -1,    46,    -1,
      -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,
     140,   141,   142,    -1,    -1,    -1,    64,    -1,    -1,   149,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    80,    81,    -1,    -1,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    -1,    -1,    -1,    15,   103,   104,    -1,    -1,   107,
      -1,   109,   110,    24,    25,    -1,   114,    -1,    -1,    30,
      -1,    -1,    33,    34,    35,    -1,    37,    38,    39,    -1,
      -1,    -1,    43,    44,    -1,    46,    -1,    -1,    -1,    -1,
      51,    -1,   140,   141,   142,    -1,    -1,    -1,    -1,   147,
      -1,    -1,    -1,    64,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,
      81,    -1,    -1,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    10,    11,    -1,    -1,    -1,    15,    -1,    -1,    -1,
      -1,    -1,   103,   104,    -1,    24,    25,    -1,   109,   110,
      -1,    30,    -1,   114,    33,    34,    35,    -1,    37,    38,
      39,    -1,    -1,    -1,    43,    44,    -1,    46,    -1,    -1,
      -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,   140,
     141,   142,    -1,    -1,    -1,    64,   147,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    80,    81,    -1,    -1,    84,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    10,    11,    -1,    -1,    -1,    15,    -1,
      -1,    -1,    -1,    -1,   103,   104,    -1,    24,    25,    -1,
     109,   110,    -1,    30,    -1,   114,    33,    34,    35,    -1,
      37,    38,    39,    -1,    -1,    -1,    43,    44,    -1,    46,
      -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,   142,    -1,    -1,    -1,    64,   147,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    80,    81,    -1,    -1,    84,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   103,   104,    -1,    -1,
      -1,    -1,   109,   110,    -1,    -1,    -1,   114,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,    -1,    -1,
      -1,    -1,    -1,   140,   141,   142,    -1,   145,    -1,   147,
     148,   149,   150,   151,   152,    -1,   154,   155,   156,   157,
     158,   159,    -1,    -1,   162,   163,   164,   165,   166,   167,
     168,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     145,    -1,   147,   148,   149,   150,   151,   152,    -1,   154,
     155,   156,   157,   158,   159,    -1,    -1,   162,   163,   164,
     165,   166,   167,   168
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,   170,     0,    10,    11,    15,    24,    25,    28,    30,
      33,    34,    35,    37,    38,    39,    42,    43,    44,    46,
      51,    64,    80,    81,    84,    90,    91,    92,   103,   104,
     107,   109,   110,   114,   139,   140,   141,   142,   145,   151,
     171,   172,   179,   180,   181,   182,   185,   186,   190,   191,
     201,   202,   204,   205,   206,   212,   213,   214,   232,   234,
     237,   239,   247,   250,   250,   250,   250,   250,   250,   140,
     141,   142,   144,   195,   196,   250,   250,   250,   250,   250,
     250,   250,   196,   250,   250,   250,   250,   250,   250,   250,
     250,   173,   196,   173,   173,   250,   250,   250,   250,   250,
     250,   143,   196,   146,   146,   187,   201,   205,   250,   145,
     146,   207,   250,   145,   155,     3,     5,     6,     8,     9,
      14,    18,    19,    22,    29,    31,    36,    40,    41,    47,
      50,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    79,    85,    86,    87,    89,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   105,
     108,   111,   112,   113,   144,   188,   192,   193,   194,   223,
     224,   225,   226,   227,   229,   230,   231,   148,   176,   176,
     176,   176,   192,   231,   233,   152,   153,   170,   189,    86,
     231,     4,    12,    13,    16,    20,    26,    45,    62,    78,
     106,   116,   117,   136,   137,   138,   140,   141,   143,   147,
     153,   158,   159,   160,   161,   202,   203,   208,   212,   232,
     250,   255,   256,   257,   258,   259,   261,   262,   265,   266,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   284,   196,   240,   246,   148,
     148,   144,   146,   196,   144,   148,   196,   145,   246,   148,
     148,   148,   148,   148,   148,   140,   141,   212,   246,   211,
     146,   146,   146,   146,   196,   235,   236,   250,   253,   254,
     282,   147,   147,   232,   250,   144,   145,   196,   153,   145,
     145,   145,   284,   146,   270,   270,   250,   284,   270,   270,
     270,   270,    32,   209,   210,   207,   153,   231,   147,   257,
     145,   116,   117,   151,   156,   122,   124,   128,   129,   130,
     131,   132,   133,   134,   135,   157,   283,   162,   163,   164,
     158,   159,   121,   123,   125,   126,   148,   149,   120,   127,
     165,   166,   167,   118,   119,   168,   155,   151,   248,   249,
     146,   157,   231,   231,   198,   150,   197,   231,   215,   228,
     250,   228,   228,   228,   231,   231,   153,   149,   175,   239,
     250,   208,   178,   208,   178,   178,   249,   145,   155,   231,
     154,   155,   231,   213,   196,   145,   241,   231,   154,   153,
      21,    27,   115,    88,   254,   267,   153,   284,   196,   282,
     270,   270,   270,   270,   271,   271,   272,   272,   273,   273,
     273,   273,   274,   274,   275,   276,   277,   278,   279,   284,
     282,   152,   284,   249,   215,   241,   146,   251,   253,   149,
     155,   147,   200,   250,   144,   146,   155,   150,   176,   216,
     217,   218,   149,   231,   149,   155,   149,   149,   149,   155,
     208,   219,   149,   239,   145,   192,   180,   181,   183,   184,
     147,    49,   174,   177,   147,   147,   236,   152,   253,   153,
     155,   244,   249,   284,   257,   153,   153,   153,   154,   254,
     152,   153,   150,   152,   176,   146,   244,   245,   251,   252,
     137,    86,   194,   199,   231,   198,   137,    48,    63,   196,
     273,   137,   113,   220,   221,   222,   250,   154,   145,   147,
     183,   173,   174,   267,   213,    29,    50,    93,   112,   147,
     196,   242,   154,   154,   115,   237,   250,   266,   284,   284,
     154,   267,   282,   244,   147,   147,   155,   147,   155,   155,
     238,   246,   147,   149,   153,   153,   149,   149,   155,   157,
     230,   215,   176,   154,   153,   153,   151,   243,   270,   257,
     260,   153,   266,   266,   154,   154,   154,   147,   146,   147,
     251,   137,   145,   155,   196,   196,   221,   251,   196,   146,
     267,   267,   137,   157,    23,   284,   265,   265,   257,   146,
     244,   149,   246,   154,   156,   151,   154,   155,   249,   241,
     154,   154,   152,   148,   251,   257,   154,   154,   154,     7,
      17,   263,   264,   147,   196,   284,   196,   215,   244,   154,
     145,   153,   212,   268,   145,   145,   257,   257,   284,   150,
     147,   263,   154,   152,   151,   154,   155,   147,   149,   150,
     256,   154,   155,   284,   196,   145,   256,   196,   152,   154,
     154,   154,   155,   196,   154
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   169,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   171,   172,   173,   173,   174,   175,   175,   176,   176,
     176,   177,   177,   178,   178,   179,   180,   181,   182,   182,
     182,   183,   183,   184,   184,   185,   186,   187,   188,   188,
     189,   189,   190,   191,   192,   192,   193,   194,   195,   195,
     195,   195,   196,   197,   197,   198,   198,   199,   199,   200,
     201,   201,   202,   202,   203,   203,   204,   204,   205,   206,
     206,   207,   207,   208,   209,   210,   211,   212,   212,   213,
     214,   214,   215,   215,   215,   215,   216,   217,   217,   217,
     217,   217,   217,   217,   218,   218,   219,   219,   219,   220,
     220,   221,   221,   222,   223,   223,   223,   223,   223,   223,
     223,   223,   223,   224,   224,   225,   225,   225,   225,   225,
     225,   225,   226,   226,   226,   226,   226,   226,   226,   227,
     227,   227,   228,   229,   229,   230,   230,   230,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   230,   230,   230,
     230,   230,   231,   231,   232,   232,   232,   233,   233,   234,
     235,   235,   236,   237,   238,   238,   239,   239,   240,   241,
     242,   242,   242,   242,   242,   243,   243,   244,   244,   244,
     244,   245,   245,   246,   246,   246,   246,   247,   247,   248,
     248,   249,   249,   250,   250,   250,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   250,   250,
     251,   251,   251,   252,   252,   253,   254,   254,   255,   255,
     256,   256,   257,   257,   257,   257,   257,   257,   257,   258,
     258,   258,   258,   258,   259,   260,   260,   261,   261,   261,
     261,   262,   263,   263,   263,   263,   264,   264,   265,   265,
     266,   267,   267,   268,   268,   268,   268,   268,   268,   268,
     268,   268,   268,   268,   268,   268,   269,   269,   269,   269,
     269,   269,   269,   270,   270,   270,   270,   270,   270,   270,
     270,   271,   271,   271,   271,   272,   272,   272,   273,   273,
     273,   274,   274,   274,   274,   274,   275,   275,   275,   276,
     276,   277,   277,   278,   278,   279,   279,   280,   280,   281,
     281,   282,   282,   283,   283,   283,   283,   283,   283,   283,
     283,   283,   283,   283,   284,   284
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     2,     2,     3,     2,     2,     2,     2,
       2,     4,     2,     0,     1,     7,     2,     3,     0,     3,
       4,     1,     2,     1,     2,     6,     6,     6,     1,     1,
       1,     1,     1,     1,     2,     7,     5,     3,     1,     1,
       0,     2,     2,     3,     1,     1,     6,     4,     1,     1,
       1,     1,     1,     0,     2,     0,     2,     1,     1,     4,
       3,     6,     1,     2,     0,     1,     2,     2,     7,     1,
       2,     2,     3,     0,     0,     0,     0,     1,     1,     1,
       1,     1,     0,     2,     2,     2,     2,     5,     8,     7,
      10,    10,     9,    12,     5,     7,     1,     2,     2,     1,
       3,     1,     3,     5,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     1,     1,     6,     1,     8,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     4,     6,
       1,     4,     4,     4,     6,     1,     1,     1,     2,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     5,
       1,     3,     2,     2,     1,     3,     1,     3,     4,     0,
       1,     1,     1,     1,     1,     0,     3,     0,     6,     8,
       6,     3,     5,     1,     3,     5,     5,     3,     3,     2,
       3,     0,     2,     0,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       1,     3,     4,     1,     3,     1,     1,     3,     1,     1,
       1,     2,     1,     1,     2,     1,     1,     1,     1,     2,
       2,     3,     2,     2,     6,     1,     3,     7,     9,     9,
       9,     9,     4,     3,     3,     2,     1,     2,     0,     1,
       2,     0,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     6,     9,     4,     5,     1,     1,     2,     2,     3,
       4,     5,     6,     1,     2,     2,     2,     2,     2,     2,
       6,     1,     3,     3,     3,     1,     3,     3,     1,     3,
       3,     1,     3,     3,     3,     3,     1,     3,     3,     1,
       3,     1,     3,     1,     3,     1,     3,     1,     3,     1,
       5,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = HLSL_YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == HLSL_YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (&yylloc, scanner, ctx, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use HLSL_YYerror or HLSL_YYUNDEF. */
#define YYERRCODE HLSL_YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if HLSL_YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined HLSL_YYLTYPE_IS_TRIVIAL && HLSL_YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location, scanner, ctx); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void *scanner, struct hlsl_ctx *ctx)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  YY_USE (scanner);
  YY_USE (ctx);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, void *scanner, struct hlsl_ctx *ctx)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp, scanner, ctx);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule, void *scanner, struct hlsl_ctx *ctx)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]), scanner, ctx);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, scanner, ctx); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !HLSL_YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !HLSL_YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, void *scanner, struct hlsl_ctx *ctx)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  YY_USE (scanner);
  YY_USE (ctx);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_VAR_IDENTIFIER: /* VAR_IDENTIFIER  */
            { hlsl_identifier_cleanup(&((*yyvaluep).identifier)); }
        break;

    case YYSYMBOL_NEW_IDENTIFIER: /* NEW_IDENTIFIER  */
            { hlsl_identifier_cleanup(&((*yyvaluep).identifier)); }
        break;

    case YYSYMBOL_NAMESPACE_IDENTIFIER: /* NAMESPACE_IDENTIFIER  */
            { hlsl_identifier_cleanup(&((*yyvaluep).identifier)); }
        break;

    case YYSYMBOL_STRING: /* STRING  */
            { vkd3d_free(((*yyvaluep).name)); }
        break;

    case YYSYMBOL_TYPE_IDENTIFIER: /* TYPE_IDENTIFIER  */
            { hlsl_identifier_cleanup(&((*yyvaluep).identifier)); }
        break;

    case YYSYMBOL_name_opt: /* name_opt  */
            { vkd3d_free(((*yyvaluep).name)); }
        break;

    case YYSYMBOL_any_identifier: /* any_identifier  */
            { hlsl_identifier_cleanup(&((*yyvaluep).identifier)); }
        break;

    case YYSYMBOL_unqualified_identifier: /* unqualified_identifier  */
            { vkd3d_free(((*yyvaluep).name)); }
        break;

    case YYSYMBOL_attribute_list: /* attribute_list  */
            { cleanup_parse_attribute_list(&((*yyvaluep).attr_list)); }
        break;

    case YYSYMBOL_attribute_list_optional: /* attribute_list_optional  */
            { cleanup_parse_attribute_list(&((*yyvaluep).attr_list)); }
        break;

    case YYSYMBOL_compound_statement: /* compound_statement  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_var_identifier: /* var_identifier  */
            { hlsl_identifier_cleanup(&((*yyvaluep).identifier)); }
        break;

    case YYSYMBOL_unqualified_var_identifier: /* unqualified_var_identifier  */
            { vkd3d_free(((*yyvaluep).name)); }
        break;

    case YYSYMBOL_var_modifier_identifier: /* var_modifier_identifier  */
            { vkd3d_free(((*yyvaluep).name)); }
        break;

    case YYSYMBOL_colon_attributes: /* colon_attributes  */
            { parse_colon_attributes_cleanup(&((*yyvaluep).colon_attributes)); }
        break;

    case YYSYMBOL_parameters: /* parameters  */
            { hlsl_func_parameters_cleanup(&((*yyvaluep).parameters)); }
        break;

    case YYSYMBOL_param_list: /* param_list  */
            { hlsl_func_parameters_cleanup(&((*yyvaluep).parameters)); }
        break;

    case YYSYMBOL_declaration_statement: /* declaration_statement  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_type_specs: /* type_specs  */
            { destroy_parse_variable_defs(((*yyvaluep).list)); }
        break;

    case YYSYMBOL_type_spec: /* type_spec  */
            { free_parse_variable_def(((*yyvaluep).variable_def)); }
        break;

    case YYSYMBOL_declaration: /* declaration  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_variables_def: /* variables_def  */
            { destroy_parse_variable_defs(((*yyvaluep).list)); }
        break;

    case YYSYMBOL_variables_def_typed: /* variables_def_typed  */
            { destroy_parse_variable_defs(((*yyvaluep).list)); }
        break;

    case YYSYMBOL_variable_decl: /* variable_decl  */
            { free_parse_variable_def(((*yyvaluep).variable_def)); }
        break;

    case YYSYMBOL_stateblock_lhs_identifier: /* stateblock_lhs_identifier  */
            { vkd3d_free(((*yyvaluep).name)); }
        break;

    case YYSYMBOL_state_block: /* state_block  */
            { hlsl_free_state_block(((*yyvaluep).state_block)); }
        break;

    case YYSYMBOL_state_block_list: /* state_block_list  */
            { free_parse_variable_def(((*yyvaluep).variable_def)); }
        break;

    case YYSYMBOL_variable_def: /* variable_def  */
            { free_parse_variable_def(((*yyvaluep).variable_def)); }
        break;

    case YYSYMBOL_variable_def_typed: /* variable_def_typed  */
            { free_parse_variable_def(((*yyvaluep).variable_def)); }
        break;

    case YYSYMBOL_complex_initializer: /* complex_initializer  */
            { cleanup_parse_initializer(&((*yyvaluep).initializer)); }
        break;

    case YYSYMBOL_complex_initializer_list: /* complex_initializer_list  */
            { cleanup_parse_initializer(&((*yyvaluep).initializer)); }
        break;

    case YYSYMBOL_initializer_expr: /* initializer_expr  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_initializer_expr_list: /* initializer_expr_list  */
            { cleanup_parse_initializer(&((*yyvaluep).initializer)); }
        break;

    case YYSYMBOL_statement_list: /* statement_list  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_statement: /* statement  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_jump_statement: /* jump_statement  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_selection_statement: /* selection_statement  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_loop_statement: /* loop_statement  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_switch_statement: /* switch_statement  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_switch_case: /* switch_case  */
            { hlsl_free_ir_switch_case(((*yyvaluep).switch_case)); }
        break;

    case YYSYMBOL_switch_cases: /* switch_cases  */
            { destroy_switch_cases(((*yyvaluep).list)); }
        break;

    case YYSYMBOL_expr_optional: /* expr_optional  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_expr_statement: /* expr_statement  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_func_arguments: /* func_arguments  */
            { cleanup_parse_initializer(&((*yyvaluep).initializer)); }
        break;

    case YYSYMBOL_primary_expr: /* primary_expr  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_postfix_expr: /* postfix_expr  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_unary_expr: /* unary_expr  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_mul_expr: /* mul_expr  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_add_expr: /* add_expr  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_shift_expr: /* shift_expr  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_relational_expr: /* relational_expr  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_equality_expr: /* equality_expr  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_bitand_expr: /* bitand_expr  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_bitxor_expr: /* bitxor_expr  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_bitor_expr: /* bitor_expr  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_logicand_expr: /* logicand_expr  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_logicor_expr: /* logicor_expr  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_conditional_expr: /* conditional_expr  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_assignment_expr: /* assignment_expr  */
            { destroy_block(((*yyvaluep).block)); }
        break;

    case YYSYMBOL_expr: /* expr  */
            { destroy_block(((*yyvaluep).block)); }
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *scanner, struct hlsl_ctx *ctx)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined HLSL_YYLTYPE_IS_TRIVIAL && HLSL_YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs YY_ATTRIBUTE_UNUSED = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = HLSL_YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == HLSL_YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, &yylloc, scanner);
    }

  if (yychar <= HLSL_YYEOF)
    {
      yychar = HLSL_YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == HLSL_YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = HLSL_YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = HLSL_YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 6: /* hlsl_prog: hlsl_prog declaration_statement  */
        {
            hlsl_block_add_block(&ctx->static_initializers, (yyvsp[0].block));
            destroy_block((yyvsp[0].block));
        }
    break;

  case 11: /* namespace_declaration: namespace_prefix '{' hlsl_prog '}'  */
        {
            hlsl_pop_scope(ctx);
        }
    break;

  case 12: /* namespace_prefix: KW_NAMESPACE unqualified_identifier  */
        {
            if (!hlsl_push_namespace(ctx, (yyvsp[0].name), &(yylsp[0])))
                YYABORT;
        }
    break;

  case 13: /* name_opt: %empty  */
        {
            (yyval.name) = NULL;
        }
    break;

  case 15: /* pass: KW_PASS name_opt annotations_opt '{' state_block_start state_block '}'  */
        {
            add_pass(ctx, (yyvsp[-5].name), (yyvsp[-4].scope), (yyvsp[-1].state_block), &(yylsp[-6]));
        }
    break;

  case 16: /* annotations_list: variables_def_typed ';'  */
        {
            struct hlsl_block *block;

            block = initialize_vars(ctx, (yyvsp[-1].list));
            destroy_block(block);
        }
    break;

  case 17: /* annotations_list: annotations_list variables_def_typed ';'  */
        {
            struct hlsl_block *block;

            block = initialize_vars(ctx, (yyvsp[-1].list));
            destroy_block(block);
        }
    break;

  case 18: /* annotations_opt: %empty  */
        {
            (yyval.scope) = NULL;
        }
    break;

  case 19: /* annotations_opt: '<' annotations_scope_start '>'  */
        {
            hlsl_pop_scope(ctx);
            (yyval.scope) = NULL;
        }
    break;

  case 20: /* annotations_opt: '<' annotations_scope_start annotations_list '>'  */
        {
            struct hlsl_scope *scope = ctx->cur_scope;

            hlsl_pop_scope(ctx);
            (yyval.scope) = scope;
        }
    break;

  case 25: /* technique9: KW_TECHNIQUE name_opt annotations_opt '{' passes '}'  */
        {
            struct hlsl_scope *scope = ctx->cur_scope;
            hlsl_pop_scope(ctx);

            add_technique(ctx, (yyvsp[-4].name), scope, (yyvsp[-3].scope), "technique", &(yylsp[-5]));
        }
    break;

  case 26: /* technique10: KW_TECHNIQUE10 name_opt annotations_opt '{' passes '}'  */
        {
            struct hlsl_scope *scope = ctx->cur_scope;
            hlsl_pop_scope(ctx);

            add_technique(ctx, (yyvsp[-4].name), scope, (yyvsp[-3].scope), "technique10", &(yylsp[-5]));
        }
    break;

  case 27: /* technique11: KW_TECHNIQUE11 name_opt annotations_opt '{' passes '}'  */
        {
            struct hlsl_scope *scope = ctx->cur_scope;
            hlsl_pop_scope(ctx);

            if (ctx->profile->type == VKD3D_SHADER_TYPE_EFFECT && ctx->profile->major_version == 2)
                hlsl_error(ctx, &(yylsp[-5]), VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX,
                        "The 'technique11' keyword is invalid for this profile.");

            add_technique(ctx, (yyvsp[-4].name), scope, (yyvsp[-3].scope), "technique11", &(yylsp[-5]));
        }
    break;

  case 35: /* effect_group: KW_FXGROUP unqualified_identifier annotations_opt '{' scope_start group_techniques '}'  */
        {
            struct hlsl_scope *scope = ctx->cur_scope;
            hlsl_pop_scope(ctx);
            add_effect_group(ctx, (yyvsp[-5].name), scope, (yyvsp[-4].scope), &(yylsp[-5]));
        }
    break;

  case 36: /* buffer_declaration: var_modifiers buffer_type unqualified_identifier colon_attributes annotations_opt  */
        {
            if (ctx->cur_scope->namespace)
            {
                hlsl_fixme(ctx, &(yylsp[-2]), "Buffers inside namespaces.");
                parse_colon_attributes_cleanup(&(yyvsp[-1].colon_attributes));
                vkd3d_free((yyvsp[-2].name));
                YYABORT;
            }

            if ((yyvsp[-1].colon_attributes).semantic.name)
                hlsl_error(ctx, &(yylsp[-1]), VKD3D_SHADER_ERROR_HLSL_INVALID_SEMANTIC, "Semantics are not allowed on buffers.");

            if (!(ctx->cur_buffer = hlsl_new_buffer(ctx, (yyvsp[-3].buffer_type), (yyvsp[-2].name), (yyvsp[-4].modifiers), &(yyvsp[-1].colon_attributes).reg_reservation, (yyvsp[0].scope), &(yylsp[-2]))))
            {
                parse_colon_attributes_cleanup(&(yyvsp[-1].colon_attributes));
                vkd3d_free((yyvsp[-2].name));
                YYABORT;
            }
        }
    break;

  case 37: /* buffer_body: '{' declaration_statement_list '}'  */
        {
            ctx->cur_buffer = ctx->globals_buffer;
        }
    break;

  case 38: /* buffer_type: KW_CBUFFER  */
        {
            (yyval.buffer_type) = HLSL_BUFFER_CONSTANT;
        }
    break;

  case 39: /* buffer_type: KW_TBUFFER  */
        {
            (yyval.buffer_type) = HLSL_BUFFER_TEXTURE;
        }
    break;

  case 41: /* declaration_statement_list: declaration_statement_list declaration_statement  */
        {
            destroy_block((yyvsp[0].block));
        }
    break;

  case 42: /* preproc_directive: PRE_LINE STRING  */
        {
            if (strcmp((yyvsp[0].name), ctx->location.source_name))
            {
                if (!vkd3d_shader_source_list_append(ctx->source_files, (yyvsp[0].name)))
                {
                    ctx->result = VKD3D_ERROR_OUT_OF_MEMORY;
                }
                else
                {
                    ctx->location.line = (yyvsp[-1].intval);
                    ctx->location.source_name = ctx->source_files->sources[ctx->source_files->count - 1];
                }
            }
            vkd3d_free((yyvsp[0].name));
        }
    break;

  case 43: /* struct_declaration_without_vars: var_modifiers struct_spec ';'  */
        {
            if (!(yyvsp[-1].type)->name)
                hlsl_error(ctx, &(yylsp[-1]), VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX,
                    "Anonymous struct type must declare a variable.");

            if ((yyvsp[-2].modifiers))
                hlsl_error(ctx, &(yylsp[-2]), VKD3D_SHADER_ERROR_HLSL_INVALID_MODIFIER,
                        "Modifiers are not allowed on struct type declarations.");
        }
    break;

  case 46: /* named_struct_spec: KW_STRUCT unqualified_identifier base_optional '{' fields_list '}'  */
        {
            bool ret;

            if ((yyvsp[-3].type))
            {
                char *name;

                if (!(name = hlsl_strdup(ctx, "$super")))
                    YYABORT;
                if (!hlsl_array_reserve(ctx, (void **)&(yyvsp[-1].fields).fields, &(yyvsp[-1].fields).capacity, 1 + (yyvsp[-1].fields).count, sizeof(*(yyvsp[-1].fields).fields)))
                    YYABORT;
                memmove(&(yyvsp[-1].fields).fields[1], (yyvsp[-1].fields).fields, (yyvsp[-1].fields).count * sizeof(*(yyvsp[-1].fields).fields));
                ++(yyvsp[-1].fields).count;

                memset(&(yyvsp[-1].fields).fields[0], 0, sizeof((yyvsp[-1].fields).fields[0]));
                (yyvsp[-1].fields).fields[0].type = (yyvsp[-3].type);
                (yyvsp[-1].fields).fields[0].loc = (yylsp[-3]);
                (yyvsp[-1].fields).fields[0].name = name;
            }

            (yyval.type) = hlsl_new_struct_type(ctx, (yyvsp[-4].name), (yyvsp[-1].fields).fields, (yyvsp[-1].fields).count);

            if (hlsl_get_var_in_scope(ctx->cur_scope, (yyvsp[-4].name)))
            {
                hlsl_error(ctx, &(yylsp[-4]), VKD3D_SHADER_ERROR_HLSL_REDEFINED,
                        "\"%s\" is already declared as a variable.", (yyvsp[-4].name));
                YYABORT;
            }
            if (hlsl_get_namespace(ctx->cur_scope, (yyvsp[-4].name)))
            {
                hlsl_error(ctx, &(yylsp[-4]), VKD3D_SHADER_ERROR_HLSL_REDEFINED,
                        "\"%s\" is already declared as a namespace.", (yyvsp[-4].name));
                YYABORT;
            }

            ret = hlsl_scope_add_type(ctx->cur_scope, (yyval.type));
            if (!ret)
            {
                hlsl_error(ctx, &(yylsp[-4]), VKD3D_SHADER_ERROR_HLSL_REDEFINED, "Struct \"%s\" is already defined.", (yyvsp[-4].name));
                YYABORT;
            }
        }
    break;

  case 47: /* unnamed_struct_spec: KW_STRUCT '{' fields_list '}'  */
        {
            (yyval.type) = hlsl_new_struct_type(ctx, NULL, (yyvsp[-1].fields).fields, (yyvsp[-1].fields).count);
        }
    break;

  case 52: /* unqualified_identifier: any_identifier  */
        {
            if (!((yyval.name) = take_unqualified_identifier(ctx, &(yyvsp[0].identifier), &(yylsp[0]))))
                YYABORT;
        }
    break;

  case 53: /* base_optional: %empty  */
        {
            (yyval.type) = NULL;
        }
    break;

  case 54: /* base_optional: ':' TYPE_IDENTIFIER  */
        {
            (yyval.type) = hlsl_get_type((yyvsp[0].identifier).scope, (yyvsp[0].identifier).name, false, true);
            if ((yyval.type)->class != HLSL_CLASS_STRUCT)
            {
                hlsl_error(ctx, &(yylsp[0]), VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                        "Base type \"%s\" is not a struct.", (yyvsp[0].identifier).qualified_name);
                hlsl_identifier_cleanup(&(yyvsp[0].identifier));
                YYABORT;
            }
            hlsl_identifier_cleanup(&(yyvsp[0].identifier));
        }
    break;

  case 55: /* fields_list: %empty  */
        {
            (yyval.fields).fields = NULL;
            (yyval.fields).count = 0;
            (yyval.fields).capacity = 0;
        }
    break;

  case 56: /* fields_list: fields_list field  */
        {
            size_t i;

            for (i = 0; i < (yyvsp[0].fields).count; ++i)
            {
                const struct hlsl_struct_field *field = &(yyvsp[0].fields).fields[i];
                const struct hlsl_struct_field *existing;

                if ((existing = get_struct_field((yyvsp[-1].fields).fields, (yyvsp[-1].fields).count, field->name)))
                {
                    hlsl_error(ctx, &field->loc, VKD3D_SHADER_ERROR_HLSL_REDEFINED,
                            "Field \"%s\" is already defined.", field->name);
                    hlsl_note(ctx, &existing->loc, VKD3D_SHADER_LOG_ERROR,
                            "'%s' was previously defined here.", field->name);
                }
            }

            if (!hlsl_array_reserve(ctx, (void **)&(yyvsp[-1].fields).fields, &(yyvsp[-1].fields).capacity, (yyvsp[-1].fields).count + (yyvsp[0].fields).count, sizeof(*(yyvsp[-1].fields).fields)))
                YYABORT;
            memcpy((yyvsp[-1].fields).fields + (yyvsp[-1].fields).count, (yyvsp[0].fields).fields, (yyvsp[0].fields).count * sizeof(*(yyvsp[0].fields).fields));
            (yyvsp[-1].fields).count += (yyvsp[0].fields).count;
            vkd3d_free((yyvsp[0].fields).fields);

            (yyval.fields) = (yyvsp[-1].fields);
        }
    break;

  case 59: /* field: var_modifiers field_type variables_def ';'  */
        {
            struct hlsl_type *type;
            uint32_t modifiers = (yyvsp[-3].modifiers);

            if (!(type = apply_type_modifiers(ctx, (yyvsp[-2].type), &modifiers, true, &(yylsp[-3]))))
                YYABORT;
            if (modifiers & ~HLSL_INTERPOLATION_MODIFIERS_MASK)
            {
                struct vkd3d_string_buffer *string;

                if ((string = hlsl_modifiers_to_string(ctx, modifiers)))
                    hlsl_error(ctx, &(yylsp[-3]), VKD3D_SHADER_ERROR_HLSL_INVALID_MODIFIER,
                            "Modifiers '%s' are not allowed on struct fields.", string->buffer);
                hlsl_release_string_buffer(ctx, string);
            }
            if (!gen_struct_fields(ctx, &(yyval.fields), type, modifiers, (yyvsp[-1].list)))
                YYABORT;
        }
    break;

  case 60: /* attribute: '[' unqualified_identifier ']'  */
        {
            if (!((yyval.attr) = hlsl_alloc(ctx, offsetof(struct hlsl_attribute, args[0]))))
            {
                vkd3d_free((yyvsp[-1].name));
                YYABORT;
            }
            (yyval.attr)->name = (yyvsp[-1].name);
            hlsl_block_init(&(yyval.attr)->instrs);
            (yyval.attr)->loc = (yyloc);
            (yyval.attr)->args_count = 0;
        }
    break;

  case 61: /* attribute: '[' unqualified_identifier '(' initializer_expr_list ')' ']'  */
        {
            unsigned int i;

            if (!((yyval.attr) = hlsl_alloc(ctx, offsetof(struct hlsl_attribute, args[(yyvsp[-2].initializer).args_count]))))
            {
                vkd3d_free((yyvsp[-4].name));
                cleanup_parse_initializer(&(yyvsp[-2].initializer));
                YYABORT;
            }
            (yyval.attr)->name = (yyvsp[-4].name);
            hlsl_block_init(&(yyval.attr)->instrs);
            hlsl_block_add_block(&(yyval.attr)->instrs, (yyvsp[-2].initializer).instrs);
            (yyval.attr)->loc = (yyloc);
            (yyval.attr)->args_count = (yyvsp[-2].initializer).args_count;
            for (i = 0; i < (yyvsp[-2].initializer).args_count; ++i)
                hlsl_src_from_node(&(yyval.attr)->args[i], (yyvsp[-2].initializer).args[i]);
            cleanup_parse_initializer(&(yyvsp[-2].initializer));
        }
    break;

  case 62: /* attribute_list: attribute  */
        {
            (yyval.attr_list).count = 1;
            if (!((yyval.attr_list).attrs = hlsl_alloc(ctx, sizeof(*(yyval.attr_list).attrs))))
            {
                hlsl_free_attribute((yyvsp[0].attr));
                YYABORT;
            }
            (yyval.attr_list).attrs[0] = (yyvsp[0].attr);
        }
    break;

  case 63: /* attribute_list: attribute_list attribute  */
        {
            struct hlsl_attribute **new_array;

            (yyval.attr_list) = (yyvsp[-1].attr_list);
            if (!(new_array = vkd3d_realloc((yyval.attr_list).attrs, ((yyval.attr_list).count + 1) * sizeof(*(yyval.attr_list).attrs))))
            {
                cleanup_parse_attribute_list(&(yyval.attr_list));
                YYABORT;
            }
            (yyval.attr_list).attrs = new_array;
            (yyval.attr_list).attrs[(yyval.attr_list).count++] = (yyvsp[0].attr);
        }
    break;

  case 64: /* attribute_list_optional: %empty  */
        {
            (yyval.attr_list).count = 0;
            (yyval.attr_list).attrs = NULL;
        }
    break;

  case 66: /* func_declaration: func_prototype compound_statement  */
        {
            struct hlsl_ir_function_decl *decl = (yyvsp[-1].function).decl;

            if (decl->has_body)
            {
                hlsl_error(ctx, &decl->loc, VKD3D_SHADER_ERROR_HLSL_REDEFINED,
                         "Function \"%s\" is already defined.", decl->func->name);
                hlsl_note(ctx, &decl->loc, VKD3D_SHADER_LOG_ERROR,
                         "\"%s\" was previously defined here.", decl->func->name);
                destroy_block((yyvsp[0].block));
            }
            else
            {
                size_t i;

                decl->has_body = true;
                hlsl_block_add_block(&decl->body, (yyvsp[0].block));
                destroy_block((yyvsp[0].block));

                /* Semantics are taken from whichever definition has a body.
                 * We can't just replace the hlsl_ir_var pointers, though: if
                 * the function was already declared but not defined, the
                 * callers would have used the old declaration's parameters to
                 * transfer arguments. */

                if (!(yyvsp[-1].function).first)
                {
                    VKD3D_ASSERT(decl->parameters.count == (yyvsp[-1].function).parameters.count);

                    for (i = 0; i < (yyvsp[-1].function).parameters.count; ++i)
                    {
                        struct hlsl_ir_var *dst = decl->parameters.vars[i];
                        struct hlsl_ir_var *src = (yyvsp[-1].function).parameters.vars[i];

                        hlsl_cleanup_semantic(&dst->semantic);
                        dst->semantic = src->semantic;
                        memset(&src->semantic, 0, sizeof(src->semantic));
                    }

                    if (decl->return_var)
                    {
                        hlsl_cleanup_semantic(&decl->return_var->semantic);
                        decl->return_var->semantic = (yyvsp[-1].function).return_semantic;
                        memset(&(yyvsp[-1].function).return_semantic, 0, sizeof((yyvsp[-1].function).return_semantic));
                    }
                }
            }
            hlsl_pop_scope(ctx);

            if (!(yyvsp[-1].function).first)
                parse_function_cleanup(&(yyvsp[-1].function));
        }
    break;

  case 67: /* func_declaration: func_prototype ';'  */
        {
            hlsl_pop_scope(ctx);
            if (!(yyvsp[-1].function).first)
                parse_function_cleanup(&(yyvsp[-1].function));
        }
    break;

  case 68: /* func_prototype_no_attrs: var_modifiers type var_identifier '(' parameters ')' colon_attributes  */
        {
            struct hlsl_scope *upper = ctx->cur_scope->upper;
            struct hlsl_scope *namespace;
            uint32_t modifiers = (yyvsp[-6].modifiers);
            struct hlsl_ir_var *var;
            struct hlsl_type *type;
            char *func_name;

            if (hlsl_identifier_is_qualified(&(yyvsp[-4].identifier)))
            {
                hlsl_fixme(ctx, &(yylsp[-4]), "Qualified function definitions.");
                parse_colon_attributes_cleanup(&(yyvsp[0].colon_attributes));
                hlsl_func_parameters_cleanup(&(yyvsp[-2].parameters));
                hlsl_identifier_cleanup(&(yyvsp[-4].identifier));
                YYABORT;
            }
            func_name = (yyvsp[-4].identifier).qualified_name;

            if (upper->namespace)
            {
                hlsl_fixme(ctx, &(yylsp[-4]), "Functions inside namespaces.");
                parse_colon_attributes_cleanup(&(yyvsp[0].colon_attributes));
                hlsl_func_parameters_cleanup(&(yyvsp[-2].parameters));
                hlsl_identifier_cleanup(&(yyvsp[-4].identifier));
                YYABORT;
            }

            /* Functions are unconditionally inlined. */
            modifiers &= ~HLSL_MODIFIER_INLINE;

            if (modifiers & ~(HLSL_MODIFIERS_MAJORITY_MASK | HLSL_STORAGE_EXPORT | HLSL_STORAGE_STATIC))
                hlsl_error(ctx, &(yylsp[-6]), VKD3D_SHADER_ERROR_HLSL_INVALID_MODIFIER,
                        "Unexpected modifier used on a function.");
            if (!(type = apply_type_modifiers(ctx, (yyvsp[-5].type), &modifiers, true, &(yylsp[-6]))))
            {
                parse_colon_attributes_cleanup(&(yyvsp[0].colon_attributes));
                hlsl_func_parameters_cleanup(&(yyvsp[-2].parameters));
                hlsl_identifier_cleanup(&(yyvsp[-4].identifier));
                YYABORT;
            }
            if ((var = hlsl_get_var(ctx->globals, func_name)))
            {
                hlsl_error(ctx, &(yylsp[-4]), VKD3D_SHADER_ERROR_HLSL_REDEFINED,
                        "\"%s\" is already declared as a variable.", func_name);
                hlsl_note(ctx, &var->loc, VKD3D_SHADER_LOG_ERROR,
                        "\"%s\" was previously declared here.", func_name);
            }
            if ((namespace = hlsl_get_namespace(upper, func_name)))
            {
                hlsl_error(ctx, &(yylsp[-4]), VKD3D_SHADER_ERROR_HLSL_REDEFINED,
                        "\"%s\" is already declared as a namespace.", func_name);
                hlsl_note(ctx, &namespace->namespace_loc, VKD3D_SHADER_LOG_ERROR,
                        "\"%s\" was previously declared here.", func_name);
            }
            if (hlsl_types_are_equal(type, ctx->builtin_types.Void) && (yyvsp[0].colon_attributes).semantic.name)
            {
                hlsl_error(ctx, &(yylsp[0]), VKD3D_SHADER_ERROR_HLSL_INVALID_SEMANTIC,
                        "Semantics are not allowed on void functions.");
            }

            if ((yyvsp[0].colon_attributes).reg_reservation.reg_type)
                FIXME("Unexpected register reservation for a function.\n");
            if ((yyvsp[0].colon_attributes).reg_reservation.offset_type)
                hlsl_error(ctx, &(yylsp[-2]), VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                        "packoffset() is not allowed on functions.");

            if (((yyval.function).decl = hlsl_get_func_decl(ctx, func_name, &(yyvsp[-2].parameters))))
            {
                const struct hlsl_func_parameters *params = &(yyval.function).decl->parameters;
                size_t i;

                if (!hlsl_types_are_equal((yyvsp[-5].type), (yyval.function).decl->return_type))
                {
                    hlsl_error(ctx, &(yylsp[-4]), VKD3D_SHADER_ERROR_HLSL_REDEFINED,
                            "\"%s\" was already declared with a different return type.", func_name);
                    hlsl_note(ctx, &(yyval.function).decl->loc, VKD3D_SHADER_LOG_ERROR,
                            "\"%s\" was previously declared here.", func_name);
                }

                if (((yyval.function).decl->storage_modifiers & HLSL_STORAGE_STATIC) != (modifiers & HLSL_STORAGE_STATIC))
                {
                    hlsl_error(ctx, &(yylsp[-4]), VKD3D_SHADER_ERROR_HLSL_REDEFINED,
                            "\"%s\" was already declared with different storage modifiers.", func_name);
                    hlsl_note(ctx, &(yyval.function).decl->loc, VKD3D_SHADER_LOG_ERROR,
                            "\"%s\" was previously declared here.", func_name);
                }

                hlsl_identifier_cleanup(&(yyvsp[-4].identifier));

                /* We implement function invocation by copying to input
                 * parameters, emitting a HLSL_IR_CALL instruction, then copying
                 * from output parameters. As a result, we need to use the same
                 * parameter variables for every invocation of this function,
                 * which means we use the parameters created by the first
                 * declaration. If we're not the first declaration, the
                 * parameter variables that just got created will end up being
                 * mostly ignored—in particular, they won't be used in actual
                 * IR.
                 *
                 * There is a hitch: if this is the actual definition, the
                 * function body will look up parameter variables by name. We
                 * must return the original parameters, and not the ones we just
                 * created, but we're in the wrong scope, and the parameters
                 * might not even have the same names.
                 *
                 * Therefore we need to shuffle the parameters we just created
                 * into a dummy scope where they'll never be looked up, and
                 * rename the original parameters so they have the expected
                 * names. We actually do this for every prototype: we don't know
                 * whether this is the function definition yet, but it doesn't
                 * really matter. The variables can only be used in the
                 * actual definition, and don't do anything in a declaration.
                 *
                 * This is complex, and it seems tempting to avoid this logic by
                 * putting arguments into the HLSL_IR_CALL instruction, letting
                 * the canonical variables be the ones attached to the function
                 * definition, and resolving the copies when inlining. The
                 * problem with this is output parameters. We would have to use
                 * a lot of parsing logic on already lowered IR, which is
                 * brittle and ugly.
                 */

                VKD3D_ASSERT((yyvsp[-2].parameters).count == params->count);
                for (i = 0; i < params->count; ++i)
                {
                    struct hlsl_ir_var *orig_param = params->vars[i];
                    struct hlsl_ir_var *new_param = (yyvsp[-2].parameters).vars[i];
                    char *new_name;

                    list_remove(&orig_param->scope_entry);
                    list_add_tail(&ctx->cur_scope->vars, &orig_param->scope_entry);

                    list_remove(&new_param->scope_entry);
                    list_add_tail(&ctx->dummy_scope->vars, &new_param->scope_entry);

                    if (!(new_name = hlsl_strdup(ctx, new_param->name)))
                    {
                        parse_colon_attributes_cleanup(&(yyvsp[0].colon_attributes));
                        hlsl_func_parameters_cleanup(&(yyvsp[-2].parameters));
                        hlsl_identifier_cleanup(&(yyvsp[-4].identifier));
                        YYABORT;
                    }
                    vkd3d_free((void *)orig_param->name);
                    orig_param->name = new_name;
                }

                (yyval.function).first = false;
                (yyval.function).parameters = (yyvsp[-2].parameters);
                (yyval.function).return_semantic = (yyvsp[0].colon_attributes).semantic;
            }
            else
            {
                if (!((yyval.function).decl = hlsl_new_func_decl(ctx, modifiers, type, &(yyvsp[-2].parameters), &(yyvsp[0].colon_attributes).semantic, &(yylsp[-4]))))
                {
                    parse_colon_attributes_cleanup(&(yyvsp[0].colon_attributes));
                    hlsl_func_parameters_cleanup(&(yyvsp[-2].parameters));
                    hlsl_identifier_cleanup(&(yyvsp[-4].identifier));
                    YYABORT;
                }

                hlsl_add_function(ctx, func_name, (yyval.function).decl);

                (yyval.function).first = true;
            }

            ctx->cur_function = (yyval.function).decl;
        }
    break;

  case 70: /* func_prototype: attribute_list func_prototype_no_attrs  */
        {
            check_attribute_list_for_duplicates(ctx, &(yyvsp[-1].attr_list));

            if ((yyvsp[0].function).first)
            {
                (yyvsp[0].function).decl->attr_count = (yyvsp[-1].attr_list).count;
                (yyvsp[0].function).decl->attrs = (yyvsp[-1].attr_list).attrs;
            }
            else
            {
                cleanup_parse_attribute_list(&(yyvsp[-1].attr_list));
            }
            (yyval.function) = (yyvsp[0].function);
        }
    break;

  case 71: /* compound_statement: '{' '}'  */
        {
            if (!((yyval.block) = make_empty_block(ctx)))
                YYABORT;
        }
    break;

  case 72: /* compound_statement: '{' statement_list '}'  */
        {
            (yyval.block) = (yyvsp[-1].block);
        }
    break;

  case 73: /* scope_start: %empty  */
        {
            hlsl_push_scope(ctx);
        }
    break;

  case 74: /* loop_scope_start: %empty  */
        {
            hlsl_push_scope(ctx);
            ctx->cur_scope->loop = true;
        }
    break;

  case 75: /* switch_scope_start: %empty  */
        {
            hlsl_push_scope(ctx);
            ctx->cur_scope->_switch = true;
        }
    break;

  case 76: /* annotations_scope_start: %empty  */
        {
            hlsl_push_scope(ctx);
            ctx->cur_scope->annotations = true;
        }
    break;

  case 79: /* unqualified_var_identifier: var_identifier  */
        {
            if (!((yyval.name) = take_unqualified_identifier(ctx, &(yyvsp[0].identifier), &(yylsp[0]))))
                YYABORT;
        }
    break;

  case 81: /* var_modifier_identifier: NAMESPACE_IDENTIFIER  */
        {
            if (!((yyval.name) = take_unqualified_identifier(ctx, &(yyvsp[0].identifier), &(yylsp[0]))))
                YYABORT;
        }
    break;

  case 82: /* colon_attributes: %empty  */
        {
            (yyval.colon_attributes).semantic = (struct hlsl_semantic){0};
            (yyval.colon_attributes).reg_reservation.reg_type = 0;
            (yyval.colon_attributes).reg_reservation.offset_type = 0;
        }
    break;

  case 83: /* colon_attributes: colon_attributes semantic  */
        {
            (yyval.colon_attributes) = (yyvsp[-1].colon_attributes);
            hlsl_cleanup_semantic(&(yyval.colon_attributes).semantic);
            (yyval.colon_attributes).semantic = (yyvsp[0].semantic);
        }
    break;

  case 84: /* colon_attributes: colon_attributes register_reservation  */
        {
            (yyval.colon_attributes) = (yyvsp[-1].colon_attributes);
            if ((yyval.colon_attributes).reg_reservation.reg_type)
                hlsl_fixme(ctx, &(yylsp[0]), "Multiple register() reservations.");

            (yyval.colon_attributes).reg_reservation.reg_type = (yyvsp[0].reg_reservation).reg_type;
            (yyval.colon_attributes).reg_reservation.reg_index = (yyvsp[0].reg_reservation).reg_index;
            (yyval.colon_attributes).reg_reservation.reg_space = (yyvsp[0].reg_reservation).reg_space;
        }
    break;

  case 85: /* colon_attributes: colon_attributes packoffset_reservation  */
        {
            (yyval.colon_attributes) = (yyvsp[-1].colon_attributes);
            if (ctx->cur_buffer == ctx->globals_buffer)
            {
                hlsl_error(ctx, &(yylsp[0]), VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                        "The packoffset() reservation is only allowed within 'cbuffer' blocks.");
            }
            else
            {
                (yyval.colon_attributes).reg_reservation.offset_type = (yyvsp[0].reg_reservation).offset_type;
                (yyval.colon_attributes).reg_reservation.offset_index = (yyvsp[0].reg_reservation).offset_index;
            }
        }
    break;

  case 86: /* semantic: ':' unqualified_identifier  */
        {
            static const char *centroid_suffix = "_centroid";
            uint32_t modifiers = 0;
            size_t len;
            char *p;

            if (!((yyval.semantic).raw_name = hlsl_strdup(ctx, (yyvsp[0].name))))
                YYABORT;

            len = strlen((yyvsp[0].name));
            if (ascii_strncasecmp((yyvsp[0].name), "sv_", 3)
                    && len > strlen(centroid_suffix)
                    && !ascii_strcasecmp((yyvsp[0].name) + (len - strlen(centroid_suffix)), centroid_suffix))
            {
                modifiers = HLSL_STORAGE_CENTROID;
                len -= strlen(centroid_suffix);
            }

            for (p = (yyvsp[0].name) + len; p > (yyvsp[0].name) && isdigit(p[-1]); --p)
                ;
            (yyval.semantic).name = (yyvsp[0].name);
            (yyval.semantic).index = atoi(p);
            (yyval.semantic).modifiers = modifiers;
            (yyval.semantic).reported_missing = false;
            (yyval.semantic).reported_duplicated_output_next_index = 0;
            (yyval.semantic).reported_duplicated_input_incompatible_next_index = 0;
            *p = 0;
        }
    break;

  case 87: /* register_reservation: ':' KW_REGISTER '(' unqualified_identifier ')'  */
        {
            memset(&(yyval.reg_reservation), 0, sizeof((yyval.reg_reservation)));
            if (!parse_reservation_index(ctx, (yyvsp[-1].name), 0, &(yyval.reg_reservation)))
                hlsl_error(ctx, &(yylsp[-1]), VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                        "Invalid register reservation '%s'.", (yyvsp[-1].name));

            vkd3d_free((yyvsp[-1].name));
        }
    break;

  case 88: /* register_reservation: ':' KW_REGISTER '(' unqualified_identifier '[' expr ']' ')'  */
        {
            memset(&(yyval.reg_reservation), 0, sizeof((yyval.reg_reservation)));
            if (!parse_reservation_index(ctx, (yyvsp[-4].name), evaluate_static_expression_as_uint(ctx, (yyvsp[-2].block), &(yylsp[-2])), &(yyval.reg_reservation)))
            {
                hlsl_error(ctx, &(yylsp[-4]), VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                        "Invalid register reservation '%s'.", (yyvsp[-4].name));
            }

            vkd3d_free((yyvsp[-4].name));
            vkd3d_free((yyvsp[-2].block));
        }
    break;

  case 89: /* register_reservation: ':' KW_REGISTER '(' unqualified_identifier ',' unqualified_identifier ')'  */
        {
            memset(&(yyval.reg_reservation), 0, sizeof((yyval.reg_reservation)));
            if (parse_reservation_index(ctx, (yyvsp[-1].name), 0, &(yyval.reg_reservation)))
            {
                hlsl_fixme(ctx, &(yylsp[-3]), "Reservation shader target %s.", (yyvsp[-3].name));
            }
            else if (parse_reservation_space((yyvsp[-1].name), &(yyval.reg_reservation).reg_space))
            {
                if (!parse_reservation_index(ctx, (yyvsp[-3].name), 0, &(yyval.reg_reservation)))
                    hlsl_error(ctx, &(yylsp[-3]), VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                            "Invalid register reservation '%s'.", (yyvsp[-3].name));
            }
            else
            {
                hlsl_error(ctx, &(yylsp[-1]), VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                        "Invalid register or space reservation '%s'.", (yyvsp[-1].name));
            }

            vkd3d_free((yyvsp[-3].name));
            vkd3d_free((yyvsp[-1].name));
        }
    break;

  case 90: /* register_reservation: ':' KW_REGISTER '(' unqualified_identifier '[' expr ']' ',' unqualified_identifier ')'  */
        {
            memset(&(yyval.reg_reservation), 0, sizeof((yyval.reg_reservation)));

            if (!parse_reservation_space((yyvsp[-1].name), &(yyval.reg_reservation).reg_space))
                hlsl_error(ctx, &(yylsp[-1]), VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                        "Invalid register space reservation '%s'.", (yyvsp[-1].name));

            if (!parse_reservation_index(ctx, (yyvsp[-6].name), evaluate_static_expression_as_uint(ctx, (yyvsp[-4].block), &(yylsp[-4])), &(yyval.reg_reservation)))
            {
                hlsl_error(ctx, &(yylsp[-6]), VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                        "Invalid register reservation '%s'.", (yyvsp[-6].name));
            }

            vkd3d_free((yyvsp[-6].name));
            vkd3d_free((yyvsp[-4].block));
            vkd3d_free((yyvsp[-1].name));
        }
    break;

  case 91: /* register_reservation: ':' KW_REGISTER '(' unqualified_identifier ',' unqualified_identifier '[' expr ']' ')'  */
        {
            hlsl_fixme(ctx, &(yylsp[-6]), "Reservation shader target %s.", (yyvsp[-6].name));

            memset(&(yyval.reg_reservation), 0, sizeof((yyval.reg_reservation)));
            if (!parse_reservation_index(ctx, (yyvsp[-4].name), evaluate_static_expression_as_uint(ctx, (yyvsp[-2].block), &(yylsp[-2])), &(yyval.reg_reservation)))
            {
                hlsl_error(ctx, &(yylsp[-4]), VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                        "Invalid register reservation '%s'.", (yyvsp[-4].name));
            }

            vkd3d_free((yyvsp[-6].name));
            vkd3d_free((yyvsp[-4].name));
            vkd3d_free((yyvsp[-2].block));
        }
    break;

  case 92: /* register_reservation: ':' KW_REGISTER '(' unqualified_identifier ',' unqualified_identifier ',' unqualified_identifier ')'  */
        {
            hlsl_fixme(ctx, &(yylsp[-5]), "Reservation shader target %s.", (yyvsp[-5].name));

            memset(&(yyval.reg_reservation), 0, sizeof((yyval.reg_reservation)));
            if (!parse_reservation_index(ctx, (yyvsp[-3].name), 0, &(yyval.reg_reservation)))
                hlsl_error(ctx, &(yylsp[-3]), VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                        "Invalid register reservation '%s'.", (yyvsp[-3].name));

            if (!parse_reservation_space((yyvsp[-1].name), &(yyval.reg_reservation).reg_space))
                hlsl_error(ctx, &(yylsp[-1]), VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                        "Invalid register space reservation '%s'.", (yyvsp[-1].name));

            vkd3d_free((yyvsp[-5].name));
            vkd3d_free((yyvsp[-3].name));
            vkd3d_free((yyvsp[-1].name));
        }
    break;

  case 93: /* register_reservation: ':' KW_REGISTER '(' unqualified_identifier ',' unqualified_identifier '[' expr ']' ',' unqualified_identifier ')'  */
        {
            hlsl_fixme(ctx, &(yylsp[-8]), "Reservation shader target %s.", (yyvsp[-8].name));

            memset(&(yyval.reg_reservation), 0, sizeof((yyval.reg_reservation)));
            if (!parse_reservation_index(ctx, (yyvsp[-6].name), evaluate_static_expression_as_uint(ctx, (yyvsp[-4].block), &(yylsp[-4])), &(yyval.reg_reservation)))
            {
                hlsl_error(ctx, &(yylsp[-6]), VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                        "Invalid register reservation '%s'.", (yyvsp[-6].name));
            }

            if (!parse_reservation_space((yyvsp[-1].name), &(yyval.reg_reservation).reg_space))
                hlsl_error(ctx, &(yylsp[-1]), VKD3D_SHADER_ERROR_HLSL_INVALID_RESERVATION,
                        "Invalid register space reservation '%s'.", (yyvsp[-1].name));

            vkd3d_free((yyvsp[-8].name));
            vkd3d_free((yyvsp[-6].name));
            vkd3d_free((yyvsp[-4].block));
            vkd3d_free((yyvsp[-1].name));
        }
    break;

  case 94: /* packoffset_reservation: ':' KW_PACKOFFSET '(' unqualified_identifier ')'  */
        {
            (yyval.reg_reservation) = parse_packoffset(ctx, (yyvsp[-1].name), NULL, &(yyloc));

            vkd3d_free((yyvsp[-1].name));
        }
    break;

  case 95: /* packoffset_reservation: ':' KW_PACKOFFSET '(' unqualified_identifier '.' unqualified_identifier ')'  */
        {
            (yyval.reg_reservation) = parse_packoffset(ctx, (yyvsp[-3].name), (yyvsp[-1].name), &(yyloc));

            vkd3d_free((yyvsp[-3].name));
            vkd3d_free((yyvsp[-1].name));
        }
    break;

  case 96: /* parameters: scope_start  */
        {
            memset(&(yyval.parameters), 0, sizeof((yyval.parameters)));
        }
    break;

  case 97: /* parameters: scope_start KW_VOID  */
        {
            memset(&(yyval.parameters), 0, sizeof((yyval.parameters)));
        }
    break;

  case 98: /* parameters: scope_start param_list  */
        {
            (yyval.parameters) = (yyvsp[0].parameters);
        }
    break;

  case 99: /* param_list: parameter  */
        {
            memset(&(yyval.parameters), 0, sizeof((yyval.parameters)));
            if (!add_func_parameter(ctx, &(yyval.parameters), &(yyvsp[0].parameter), &(yylsp[0])))
            {
                ERR("Error adding function parameter %s.\n", (yyvsp[0].parameter).name);
                YYABORT;
            }
        }
    break;

  case 100: /* param_list: param_list ',' parameter  */
        {
            (yyval.parameters) = (yyvsp[-2].parameters);
            if (!add_func_parameter(ctx, &(yyval.parameters), &(yyvsp[0].parameter), &(yylsp[0])))
                YYABORT;
        }
    break;

  case 102: /* parameter: parameter_decl '=' complex_initializer  */
        {
            (yyval.parameter) = (yyvsp[-2].parameter);
            (yyval.parameter).initializer = (yyvsp[0].initializer);
        }
    break;

  case 103: /* parameter_decl: var_modifiers type_no_void unqualified_identifier arrays colon_attributes  */
        {
            uint32_t prim_modifiers = (yyvsp[-4].modifiers) & HLSL_PRIMITIVE_MODIFIERS_MASK;
            uint32_t modifiers = (yyvsp[-4].modifiers) & ~HLSL_PRIMITIVE_MODIFIERS_MASK;
            struct hlsl_type *type;
            unsigned int i;

            if (!(type = apply_type_modifiers(ctx, (yyvsp[-3].type), &modifiers, true, &(yylsp[-4]))))
                YYABORT;

            (yyval.parameter).modifiers = modifiers;
            if (!((yyval.parameter).modifiers & (HLSL_STORAGE_IN | HLSL_STORAGE_OUT)))
                (yyval.parameter).modifiers |= HLSL_STORAGE_IN;

            for (i = 0; i < (yyvsp[-1].arrays).count; ++i)
            {
                if ((yyvsp[-1].arrays).sizes[i] == HLSL_ARRAY_ELEMENTS_COUNT_IMPLICIT)
                {
                    hlsl_error(ctx, &(yylsp[-2]), VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                            "Implicit size arrays not allowed in function parameters.");
                    type = ctx->builtin_types.error;
                    break;
                }

                type = hlsl_new_array_type(ctx, type, (yyvsp[-1].arrays).sizes[i], HLSL_ARRAY_GENERIC);
            }
            vkd3d_free((yyvsp[-1].arrays).sizes);

            if (prim_modifiers && (prim_modifiers & (prim_modifiers - 1)))
            {
                hlsl_error(ctx, &(yylsp[-4]), VKD3D_SHADER_ERROR_HLSL_INVALID_MODIFIER,
                        "Primitive type modifiers are mutually exclusive.");
                prim_modifiers = 0;
            }

            if (prim_modifiers)
            {
                if (type->class != HLSL_CLASS_ARRAY)
                    hlsl_error(ctx, &(yylsp[-4]), VKD3D_SHADER_ERROR_HLSL_INVALID_MODIFIER,
                        "Primitive type modifiers can only be applied to arrays.");
                else
                    type->modifiers |= prim_modifiers;
            }

            (yyval.parameter).type = type;

            if (hlsl_version_ge(ctx, 5, 1) && type->class == HLSL_CLASS_ARRAY && hlsl_type_is_resource(type))
                hlsl_fixme(ctx, &(yylsp[-3]), "Shader model 5.1+ resource array.");

            (yyval.parameter).name = (yyvsp[-2].name);
            (yyval.parameter).semantic = (yyvsp[0].colon_attributes).semantic;
            (yyval.parameter).reg_reservation = (yyvsp[0].colon_attributes).reg_reservation;

            memset(&(yyval.parameter).initializer, 0, sizeof((yyval.parameter).initializer));
        }
    break;

  case 104: /* texture_type: KW_BUFFER  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_BUFFER;
        }
    break;

  case 105: /* texture_type: KW_STRUCTUREDBUFFER  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_STRUCTURED_BUFFER;
        }
    break;

  case 106: /* texture_type: KW_TEXTURE1D  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_1D;
        }
    break;

  case 107: /* texture_type: KW_TEXTURE2D  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_2D;
        }
    break;

  case 108: /* texture_type: KW_TEXTURE3D  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_3D;
        }
    break;

  case 109: /* texture_type: KW_TEXTURECUBE  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_CUBE;
        }
    break;

  case 110: /* texture_type: KW_TEXTURE1DARRAY  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_1DARRAY;
        }
    break;

  case 111: /* texture_type: KW_TEXTURE2DARRAY  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_2DARRAY;
        }
    break;

  case 112: /* texture_type: KW_TEXTURECUBEARRAY  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_CUBEARRAY;
        }
    break;

  case 113: /* texture_ms_type: KW_TEXTURE2DMS  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_2DMS;
        }
    break;

  case 114: /* texture_ms_type: KW_TEXTURE2DMSARRAY  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_2DMSARRAY;
        }
    break;

  case 115: /* uav_type: KW_RWBUFFER  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_BUFFER;
        }
    break;

  case 116: /* uav_type: KW_RWSTRUCTUREDBUFFER  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_STRUCTURED_BUFFER;
        }
    break;

  case 117: /* uav_type: KW_RWTEXTURE1D  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_1D;
        }
    break;

  case 118: /* uav_type: KW_RWTEXTURE1DARRAY  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_1DARRAY;
        }
    break;

  case 119: /* uav_type: KW_RWTEXTURE2D  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_2D;
        }
    break;

  case 120: /* uav_type: KW_RWTEXTURE2DARRAY  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_2DARRAY;
        }
    break;

  case 121: /* uav_type: KW_RWTEXTURE3D  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_3D;
        }
    break;

  case 122: /* rov_type: KW_RASTERIZERORDEREDBUFFER  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_BUFFER;
        }
    break;

  case 123: /* rov_type: KW_RASTERIZERORDEREDSTRUCTUREDBUFFER  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_STRUCTURED_BUFFER;
        }
    break;

  case 124: /* rov_type: KW_RASTERIZERORDEREDTEXTURE1D  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_1D;
        }
    break;

  case 125: /* rov_type: KW_RASTERIZERORDEREDTEXTURE1DARRAY  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_1DARRAY;
        }
    break;

  case 126: /* rov_type: KW_RASTERIZERORDEREDTEXTURE2D  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_2D;
        }
    break;

  case 127: /* rov_type: KW_RASTERIZERORDEREDTEXTURE2DARRAY  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_2DARRAY;
        }
    break;

  case 128: /* rov_type: KW_RASTERIZERORDEREDTEXTURE3D  */
        {
            (yyval.sampler_dim) = HLSL_SAMPLER_DIM_3D;
        }
    break;

  case 129: /* so_type: KW_POINTSTREAM  */
        {
            (yyval.so_type) = HLSL_STREAM_OUTPUT_POINT_STREAM;
        }
    break;

  case 130: /* so_type: KW_LINESTREAM  */
        {
            (yyval.so_type) = HLSL_STREAM_OUTPUT_LINE_STREAM;
        }
    break;

  case 131: /* so_type: KW_TRIANGLESTREAM  */
        {
            (yyval.so_type) = HLSL_STREAM_OUTPUT_TRIANGLE_STREAM;
        }
    break;

  case 132: /* resource_format: var_modifiers type  */
        {
            uint32_t modifiers = (yyvsp[-1].modifiers);

            if (!((yyval.type) = apply_type_modifiers(ctx, (yyvsp[0].type), &modifiers, true, &(yylsp[-1]))))
                YYABORT;
        }
    break;

  case 133: /* patch_type: KW_INPUTPATCH  */
        {
            (yyval.patch_type) = HLSL_ARRAY_PATCH_INPUT;
        }
    break;

  case 134: /* patch_type: KW_OUTPUTPATCH  */
        {
            (yyval.patch_type) = HLSL_ARRAY_PATCH_OUTPUT;
        }
    break;

  case 135: /* type_no_void: KW_VECTOR '<' type ',' C_INTEGER '>'  */
        {
            if ((yyvsp[-3].type)->class != HLSL_CLASS_SCALAR)
            {
                struct vkd3d_string_buffer *string;

                string = hlsl_type_to_string(ctx, (yyvsp[-3].type));
                if (string)
                    hlsl_error(ctx, &(yylsp[-3]), VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                            "Vector base type %s is not scalar.", string->buffer);
                hlsl_release_string_buffer(ctx, string);
                YYABORT;
            }
            if ((yyvsp[-1].intval) < 1 || (yyvsp[-1].intval) > 4)
            {
                hlsl_error(ctx, &(yylsp[-1]), VKD3D_SHADER_ERROR_HLSL_INVALID_SIZE,
                        "Vector size %d is not between 1 and 4.", (yyvsp[-1].intval));
                YYABORT;
            }

            (yyval.type) = hlsl_type_clone(ctx, hlsl_get_vector_type(ctx, (yyvsp[-3].type)->e.numeric.type, (yyvsp[-1].intval)), 0, 0);
            (yyval.type)->is_minimum_precision = (yyvsp[-3].type)->is_minimum_precision;
        }
    break;

  case 136: /* type_no_void: KW_VECTOR  */
        {
            (yyval.type) = hlsl_get_vector_type(ctx, HLSL_TYPE_FLOAT, 4);
        }
    break;

  case 137: /* type_no_void: KW_MATRIX '<' type ',' C_INTEGER ',' C_INTEGER '>'  */
        {
            if ((yyvsp[-5].type)->class != HLSL_CLASS_SCALAR)
            {
                struct vkd3d_string_buffer *string;

                string = hlsl_type_to_string(ctx, (yyvsp[-5].type));
                if (string)
                    hlsl_error(ctx, &(yylsp[-5]), VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                            "Matrix base type %s is not scalar.", string->buffer);
                hlsl_release_string_buffer(ctx, string);
                YYABORT;
            }
            if ((yyvsp[-3].intval) < 1 || (yyvsp[-3].intval) > 4)
            {
                hlsl_error(ctx, &(yylsp[-3]), VKD3D_SHADER_ERROR_HLSL_INVALID_SIZE,
                        "Matrix row count %d is not between 1 and 4.", (yyvsp[-3].intval));
                YYABORT;
            }
            if ((yyvsp[-1].intval) < 1 || (yyvsp[-1].intval) > 4)
            {
                hlsl_error(ctx, &(yylsp[-1]), VKD3D_SHADER_ERROR_HLSL_INVALID_SIZE,
                        "Matrix column count %d is not between 1 and 4.", (yyvsp[-1].intval));
                YYABORT;
            }

            (yyval.type) = hlsl_type_clone(ctx, hlsl_get_matrix_type(ctx, (yyvsp[-5].type)->e.numeric.type, (yyvsp[-1].intval), (yyvsp[-3].intval)), 0, 0);
            (yyval.type)->is_minimum_precision = (yyvsp[-5].type)->is_minimum_precision;
        }
    break;

  case 138: /* type_no_void: KW_MATRIX  */
        {
            (yyval.type) = hlsl_get_matrix_type(ctx, HLSL_TYPE_FLOAT, 4, 4);
        }
    break;

  case 139: /* type_no_void: KW_SAMPLER  */
        {
            (yyval.type) = ctx->builtin_types.sampler[HLSL_SAMPLER_DIM_GENERIC];
        }
    break;

  case 140: /* type_no_void: KW_SAMPLERCOMPARISONSTATE  */
        {
            (yyval.type) = ctx->builtin_types.sampler[HLSL_SAMPLER_DIM_COMPARISON];
        }
    break;

  case 141: /* type_no_void: KW_SAMPLER1D  */
        {
            (yyval.type) = ctx->builtin_types.sampler[HLSL_SAMPLER_DIM_1D];
        }
    break;

  case 142: /* type_no_void: KW_SAMPLER2D  */
        {
            (yyval.type) = ctx->builtin_types.sampler[HLSL_SAMPLER_DIM_2D];
        }
    break;

  case 143: /* type_no_void: KW_SAMPLER3D  */
        {
            (yyval.type) = ctx->builtin_types.sampler[HLSL_SAMPLER_DIM_3D];
        }
    break;

  case 144: /* type_no_void: KW_SAMPLERCUBE  */
        {
            (yyval.type) = ctx->builtin_types.sampler[HLSL_SAMPLER_DIM_CUBE];
        }
    break;

  case 145: /* type_no_void: KW_TEXTURE  */
        {
            (yyval.type) = hlsl_new_texture_type(ctx, HLSL_SAMPLER_DIM_GENERIC, NULL, 0);
        }
    break;

  case 146: /* type_no_void: texture_type  */
        {
            if ((yyvsp[0].sampler_dim) == HLSL_SAMPLER_DIM_STRUCTURED_BUFFER)
                hlsl_error(ctx, &(yylsp[0]), VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                        "Structured buffer type requires an explicit format.");
            (yyval.type) = hlsl_new_texture_type(ctx, (yyvsp[0].sampler_dim), hlsl_get_vector_type(ctx, HLSL_TYPE_FLOAT, 4), 0);
        }
    break;

  case 147: /* type_no_void: texture_type '<' resource_format '>'  */
        {
            validate_texture_format_type(ctx, (yyvsp[-3].sampler_dim), (yyvsp[-1].type), &(yylsp[-1]));
            (yyval.type) = hlsl_new_texture_type(ctx, (yyvsp[-3].sampler_dim), (yyvsp[-1].type), 0);
        }
    break;

  case 148: /* type_no_void: texture_ms_type '<' resource_format '>'  */
        {
            validate_texture_format_type(ctx, (yyvsp[-3].sampler_dim), (yyvsp[-1].type), &(yylsp[-1]));

            (yyval.type) = hlsl_new_texture_type(ctx, (yyvsp[-3].sampler_dim), (yyvsp[-1].type), 0);
        }
    break;

  case 149: /* type_no_void: texture_ms_type '<' resource_format ',' shift_expr '>'  */
        {
            unsigned int sample_count;
            struct hlsl_block block;

            hlsl_block_init(&block);
            hlsl_block_add_block(&block, (yyvsp[-1].block));

            sample_count = evaluate_static_expression_as_uint(ctx, &block, &(yylsp[-1]));

            hlsl_block_cleanup(&block);

            vkd3d_free((yyvsp[-1].block));

            (yyval.type) = hlsl_new_texture_type(ctx, (yyvsp[-5].sampler_dim), (yyvsp[-3].type), sample_count);
        }
    break;

  case 150: /* type_no_void: KW_BYTEADDRESSBUFFER  */
        {
            (yyval.type) = hlsl_new_texture_type(ctx, HLSL_SAMPLER_DIM_RAW_BUFFER, hlsl_get_scalar_type(ctx, HLSL_TYPE_UINT), 0);
        }
    break;

  case 151: /* type_no_void: uav_type '<' resource_format '>'  */
        {
            validate_uav_type(ctx, (yyvsp[-3].sampler_dim), (yyvsp[-1].type), &(yylsp[-1]));
            (yyval.type) = hlsl_new_uav_type(ctx, (yyvsp[-3].sampler_dim), (yyvsp[-1].type), false);
        }
    break;

  case 152: /* type_no_void: rov_type '<' resource_format '>'  */
        {
            validate_uav_type(ctx, (yyvsp[-3].sampler_dim), (yyvsp[-1].type), &(yylsp[0]));
            (yyval.type) = hlsl_new_uav_type(ctx, (yyvsp[-3].sampler_dim), (yyvsp[-1].type), true);
        }
    break;

  case 153: /* type_no_void: so_type '<' type '>'  */
        {
            (yyval.type) = hlsl_new_stream_output_type(ctx, (yyvsp[-3].so_type), (yyvsp[-1].type));
        }
    break;

  case 154: /* type_no_void: patch_type '<' type ',' C_INTEGER '>'  */
        {
            struct hlsl_type *type;

            if ((yyvsp[-1].intval) < 1)
            {
                hlsl_error(ctx, &(yylsp[-1]), VKD3D_SHADER_ERROR_HLSL_INVALID_SIZE,
                        "Control point size %d is not positive.", (yyvsp[-1].intval));
                YYABORT;
            }

            type = hlsl_new_array_type(ctx, (yyvsp[-3].type), (yyvsp[-1].intval), (yyvsp[-5].patch_type));
            (yyval.type) = hlsl_type_clone(ctx, type, 0, HLSL_MODIFIER_CONST);
        }
    break;

  case 155: /* type_no_void: KW_RWBYTEADDRESSBUFFER  */
        {
            (yyval.type) = hlsl_new_uav_type(ctx, HLSL_SAMPLER_DIM_RAW_BUFFER, hlsl_get_scalar_type(ctx, HLSL_TYPE_UINT), false);
        }
    break;

  case 156: /* type_no_void: KW_STRING  */
        {
            (yyval.type) = ctx->builtin_types.string;
        }
    break;

  case 157: /* type_no_void: TYPE_IDENTIFIER  */
        {
            (yyval.type) = hlsl_get_type((yyvsp[0].identifier).scope, (yyvsp[0].identifier).name, false, true);
            if ((yyval.type)->is_minimum_precision || hlsl_type_is_minimum_precision((yyval.type)))
            {
                if (hlsl_version_lt(ctx, 4, 0))
                {
                    hlsl_error(ctx, &(yylsp[0]), VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                            "Target profile doesn't support minimum-precision types.");
                }
                else if ((yyval.type)->is_minimum_precision)
                {
                    FIXME("Reinterpreting type %s.\n", (yyval.type)->name);
                }
            }
            hlsl_identifier_cleanup(&(yyvsp[0].identifier));
        }
    break;

  case 158: /* type_no_void: KW_UNSIGNED TYPE_IDENTIFIER  */
        {
            struct hlsl_type *type = hlsl_get_type((yyvsp[0].identifier).scope, (yyvsp[0].identifier).name, false, true);

            if (hlsl_is_numeric_type(type) && type->e.numeric.type == HLSL_TYPE_INT)
            {
                hlsl_identifier_cleanup(&(yyvsp[0].identifier));
                if (!(type = hlsl_type_clone(ctx, type, 0, 0)))
                    YYABORT;
                vkd3d_free((void *)type->name);
                type->name = NULL;
                type->e.numeric.type = HLSL_TYPE_UINT;
            }
            else
            {
                hlsl_error(ctx, &(yylsp[0]), VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                        "The 'unsigned' keyword can't be used with type %s.", (yyvsp[0].identifier).qualified_name);
                hlsl_identifier_cleanup(&(yyvsp[0].identifier));
            }

            (yyval.type) = type;
        }
    break;

  case 159: /* type_no_void: KW_STRUCT TYPE_IDENTIFIER  */
        {
            (yyval.type) = hlsl_get_type((yyvsp[0].identifier).scope, (yyvsp[0].identifier).name, false, true);
            if ((yyval.type)->class != HLSL_CLASS_STRUCT || (yyval.type)->is_typedef)
            {
                hlsl_error(ctx, &(yylsp[-1]), VKD3D_SHADER_ERROR_HLSL_REDEFINED,
                        "\"%s\" is not a structure.", (yyvsp[0].identifier).qualified_name);
            }
            hlsl_identifier_cleanup(&(yyvsp[0].identifier));
        }
    break;

  case 160: /* type_no_void: KW_RENDERTARGETVIEW  */
        {
            (yyval.type) = hlsl_get_type(ctx->cur_scope, "RenderTargetView", true, true);
        }
    break;

  case 161: /* type_no_void: KW_DEPTHSTENCILSTATE  */
        {
            (yyval.type) = hlsl_get_type(ctx->cur_scope, "DepthStencilState", true, true);
        }
    break;

  case 162: /* type_no_void: KW_DEPTHSTENCILVIEW  */
        {
            (yyval.type) = hlsl_get_type(ctx->cur_scope, "DepthStencilView", true, true);
        }
    break;

  case 163: /* type_no_void: KW_VERTEXSHADER  */
        {
            (yyval.type) = hlsl_get_type(ctx->cur_scope, "VertexShader", true, true);
        }
    break;

  case 164: /* type_no_void: KW_PIXELSHADER  */
        {
            (yyval.type) = hlsl_get_type(ctx->cur_scope, "PixelShader", true, true);
        }
    break;

  case 165: /* type_no_void: KW_COMPUTESHADER  */
        {
            (yyval.type) = hlsl_get_type(ctx->cur_scope, "ComputeShader", true, true);
        }
    break;

  case 166: /* type_no_void: KW_DOMAINSHADER  */
        {
            (yyval.type) = hlsl_get_type(ctx->cur_scope, "DomainShader", true, true);
        }
    break;

  case 167: /* type_no_void: KW_HULLSHADER  */
        {
            (yyval.type) = hlsl_get_type(ctx->cur_scope, "HullShader", true, true);
        }
    break;

  case 168: /* type_no_void: KW_GEOMETRYSHADER  */
        {
            (yyval.type) = hlsl_get_type(ctx->cur_scope, "GeometryShader", true, true);
        }
    break;

  case 169: /* type_no_void: KW_CONSTANTBUFFER '<' type '>'  */
        {
            if ((yyvsp[-1].type)->class != HLSL_CLASS_STRUCT)
                hlsl_error(ctx, &(yylsp[-3]), VKD3D_SHADER_ERROR_HLSL_INVALID_TYPE,
                        "ConstantBuffer<...> requires user-defined structure type.");
            (yyval.type) = hlsl_new_cb_type(ctx, (yyvsp[-1].type));
        }
    break;

  case 170: /* type_no_void: KW_RASTERIZERSTATE  */
        {
            (yyval.type) = hlsl_get_type(ctx->cur_scope, "RasterizerState", true, true);
        }
    break;

  case 171: /* type_no_void: KW_BLENDSTATE  */
        {
            (yyval.type) = hlsl_get_type(ctx->cur_scope, "BlendState", true, true);
        }
    break;

  case 173: /* type: KW_VOID  */
        {
            (yyval.type) = ctx->builtin_types.Void;
        }
    break;

  case 175: /* declaration_statement: struct_declaration_without_vars  */
        {
            if (!((yyval.block) = make_empty_block(ctx)))
                YYABORT;
        }
    break;

  case 176: /* declaration_statement: typedef  */
        {
            if (!((yyval.block) = make_empty_block(ctx)))
                YYABORT;
        }
    break;

  case 179: /* typedef: KW_TYPEDEF var_modifiers typedef_type type_specs ';'  */
        {
            uint32_t modifiers = (yyvsp[-3].modifiers);
            struct hlsl_type *type;

            if (!(type = apply_type_modifiers(ctx, (yyvsp[-2].type), &modifiers, false, &(yylsp[-3]))))
            {
                destroy_parse_variable_defs((yyvsp[-1].list));
                YYABORT;
            }

            if (modifiers)
                hlsl_error(ctx, &(yylsp[-4]), VKD3D_SHADER_ERROR_HLSL_INVALID_MODIFIER,
                        "Storage modifiers are not allowed on typedefs.");

            if (!add_typedef(ctx, type, (yyvsp[-1].list)))
                YYABORT;
        }
    break;

  case 180: /* type_specs: type_spec  */
        {
            if (!((yyval.list) = make_empty_list(ctx)))
                YYABORT;
            list_add_head((yyval.list), &(yyvsp[0].variable_def)->entry);
        }
    break;

  case 181: /* type_specs: type_specs ',' type_spec  */
        {
            (yyval.list) = (yyvsp[-2].list);
            list_add_tail((yyval.list), &(yyvsp[0].variable_def)->entry);
        }
    break;

  case 182: /* type_spec: unqualified_identifier arrays  */
        {
            (yyval.variable_def) = hlsl_alloc(ctx, sizeof(*(yyval.variable_def)));
            (yyval.variable_def)->loc = (yylsp[-1]);
            (yyval.variable_def)->name = (yyvsp[-1].name);
            (yyval.variable_def)->arrays = (yyvsp[0].arrays);
        }
    break;

  case 183: /* declaration: variables_def_typed ';'  */
        {
            if (!((yyval.block) = initialize_vars(ctx, (yyvsp[-1].list))))
                YYABORT;
        }
    break;

  case 184: /* variables_def: variable_def  */
        {
            if (!((yyval.list) = make_empty_list(ctx)))
                YYABORT;
            list_add_head((yyval.list), &(yyvsp[0].variable_def)->entry);
        }
    break;

  case 185: /* variables_def: variables_def ',' variable_def  */
        {
            (yyval.list) = (yyvsp[-2].list);
            list_add_tail((yyval.list), &(yyvsp[0].variable_def)->entry);
        }
    break;

  case 186: /* variables_def_typed: variable_def_typed  */
        {
            if (!((yyval.list) = make_empty_list(ctx)))
                YYABORT;
            list_add_head((yyval.list), &(yyvsp[0].variable_def)->entry);

            declare_var(ctx, (yyvsp[0].variable_def));
        }
    break;

  case 187: /* variables_def_typed: variables_def_typed ',' variable_def  */
        {
            struct parse_variable_def *head_def;

            VKD3D_ASSERT(!list_empty((yyvsp[-2].list)));
            head_def = LIST_ENTRY(list_head((yyvsp[-2].list)), struct parse_variable_def, entry);

            VKD3D_ASSERT(head_def->basic_type);
            (yyvsp[0].variable_def)->basic_type = head_def->basic_type;
            (yyvsp[0].variable_def)->modifiers = head_def->modifiers;
            (yyvsp[0].variable_def)->modifiers_loc = head_def->modifiers_loc;

            declare_var(ctx, (yyvsp[0].variable_def));

            (yyval.list) = (yyvsp[-2].list);
            list_add_tail((yyval.list), &(yyvsp[0].variable_def)->entry);
        }
    break;

  case 188: /* variable_decl: unqualified_identifier arrays colon_attributes annotations_opt  */
        {
            (yyval.variable_def) = hlsl_alloc(ctx, sizeof(*(yyval.variable_def)));
            (yyval.variable_def)->loc = (yylsp[-3]);
            (yyval.variable_def)->name = (yyvsp[-3].name);
            (yyval.variable_def)->arrays = (yyvsp[-2].arrays);
            (yyval.variable_def)->semantic = (yyvsp[-1].colon_attributes).semantic;
            (yyval.variable_def)->reg_reservation = (yyvsp[-1].colon_attributes).reg_reservation;
            (yyval.variable_def)->annotations = (yyvsp[0].scope);
        }
    break;

  case 189: /* state_block_start: %empty  */
        {
            ctx->in_state_block = 1;
        }
    break;

  case 191: /* stateblock_lhs_identifier: KW_PIXELSHADER  */
        {
            if (!((yyval.name) = hlsl_strdup(ctx, "pixelshader")))
                YYABORT;
        }
    break;

  case 192: /* stateblock_lhs_identifier: KW_TEXTURE  */
        {
            if (!((yyval.name) = hlsl_strdup(ctx, "texture")))
                YYABORT;
        }
    break;

  case 193: /* stateblock_lhs_identifier: KW_VERTEXSHADER  */
        {
            if (!((yyval.name) = hlsl_strdup(ctx, "vertexshader")))
                YYABORT;
        }
    break;

  case 194: /* stateblock_lhs_identifier: KW_GEOMETRYSHADER  */
        {
            if (!((yyval.name) = hlsl_strdup(ctx, "geometryshader")))
                YYABORT;
        }
    break;

  case 195: /* state_block_index_opt: %empty  */
        {
            (yyval.state_block_index).has_index = false;
            (yyval.state_block_index).index = 0;
        }
    break;

  case 196: /* state_block_index_opt: '[' C_INTEGER ']'  */
       {
            if ((yyvsp[-1].intval) < 0)
            {
                hlsl_error(ctx, &(yylsp[-1]), VKD3D_SHADER_ERROR_HLSL_INVALID_INDEX,
                        "State block array index is not a positive integer constant.");
                YYABORT;
            }
            (yyval.state_block_index).has_index = true;
            (yyval.state_block_index).index = (yyvsp[-1].intval);
       }
    break;

  case 197: /* state_block: %empty  */
        {
            if (!((yyval.state_block) = hlsl_alloc(ctx, sizeof(*(yyval.state_block)))))
                YYABORT;
        }
    break;

  case 198: /* state_block: state_block stateblock_lhs_identifier state_block_index_opt '=' complex_initializer ';'  */
        {
            struct hlsl_state_block_entry *entry;
            unsigned int i;

            if (!(entry = hlsl_alloc(ctx, sizeof(*entry))))
                YYABORT;

            entry->name = (yyvsp[-4].name);
            entry->lhs_has_index = (yyvsp[-3].state_block_index).has_index;
            entry->lhs_index = (yyvsp[-3].state_block_index).index;

            entry->instrs = (yyvsp[-1].initializer).instrs;

            entry->args_count = (yyvsp[-1].initializer).args_count;
            if (!(entry->args = hlsl_alloc(ctx, sizeof(*entry->args) * entry->args_count)))
                YYABORT;
            for (i = 0; i < entry->args_count; ++i)
                hlsl_src_from_node(&entry->args[i], (yyvsp[-1].initializer).args[i]);
            vkd3d_free((yyvsp[-1].initializer).args);

            (yyval.state_block) = (yyvsp[-5].state_block);
            hlsl_state_block_add_entry((yyval.state_block), entry);
        }
    break;

  case 199: /* state_block: state_block stateblock_lhs_identifier state_block_index_opt '=' '<' primary_expr '>' ';'  */
        {
            struct hlsl_state_block_entry *entry;

            if (!(entry = hlsl_alloc(ctx, sizeof(*entry))))
                YYABORT;

            entry->name = (yyvsp[-6].name);
            entry->lhs_has_index = (yyvsp[-5].state_block_index).has_index;
            entry->lhs_index = (yyvsp[-5].state_block_index).index;

            entry->instrs = (yyvsp[-2].block);
            entry->args_count = 1;
            if (!(entry->args = hlsl_alloc(ctx, sizeof(*entry->args) * entry->args_count)))
                YYABORT;
            hlsl_src_from_node(&entry->args[0], node_from_block((yyvsp[-2].block)));

            (yyval.state_block) = (yyvsp[-7].state_block);
            hlsl_state_block_add_entry((yyval.state_block), entry);
        }
    break;

  case 200: /* state_block: state_block unqualified_identifier '(' func_arguments ')' ';'  */
        {
            struct hlsl_state_block_entry *entry;
            unsigned int i;

            if (!(entry = hlsl_alloc(ctx, sizeof(*entry))))
                YYABORT;

            entry->is_function_call = true;

            entry->name = (yyvsp[-4].name);
            entry->lhs_has_index = false;
            entry->lhs_index = 0;

            entry->instrs = (yyvsp[-2].initializer).instrs;

            entry->args_count = (yyvsp[-2].initializer).args_count;
            if (!(entry->args = hlsl_alloc(ctx, sizeof(*entry->args) * entry->args_count)))
                YYABORT;
            for (i = 0; i < entry->args_count; ++i)
                hlsl_src_from_node(&entry->args[i], (yyvsp[-2].initializer).args[i]);
            vkd3d_free((yyvsp[-2].initializer).args);

            hlsl_validate_state_block_entry(ctx, entry, &(yylsp[-2]));

            (yyval.state_block) = (yyvsp[-5].state_block);
            hlsl_state_block_add_entry((yyval.state_block), entry);
        }
    break;

  case 201: /* state_block_list: '{' state_block '}'  */
        {
            if (!((yyval.variable_def) = hlsl_alloc(ctx, sizeof(*(yyval.variable_def)))))
                YYABORT;

            if(!(vkd3d_array_reserve((void **)&(yyval.variable_def)->state_blocks, &(yyval.variable_def)->state_block_capacity,
                    (yyval.variable_def)->state_block_count + 1, sizeof(*(yyval.variable_def)->state_blocks))))
                YYABORT;
            (yyval.variable_def)->state_blocks[(yyval.variable_def)->state_block_count++] = (yyvsp[-1].state_block);
        }
    break;

  case 202: /* state_block_list: state_block_list ',' '{' state_block '}'  */
        {
            (yyval.variable_def) = (yyvsp[-4].variable_def);

            if(!(vkd3d_array_reserve((void **)&(yyval.variable_def)->state_blocks, &(yyval.variable_def)->state_block_capacity,
                    (yyval.variable_def)->state_block_count + 1, sizeof(*(yyval.variable_def)->state_blocks))))
                YYABORT;
            (yyval.variable_def)->state_blocks[(yyval.variable_def)->state_block_count++] = (yyvsp[-1].state_block);
        }
    break;

  case 204: /* variable_def: variable_decl '=' complex_initializer  */
        {
            (yyval.variable_def) = (yyvsp[-2].variable_def);
            (yyval.variable_def)->initializer = (yyvsp[0].initializer);
        }
    break;

  case 205: /* variable_def: variable_decl '{' state_block_start state_block '}'  */
        {
            (yyval.variable_def) = (yyvsp[-4].variable_def);
            ctx->in_state_block = 0;

            if(!(vkd3d_array_reserve((void **)&(yyval.variable_def)->state_blocks, &(yyval.variable_def)->state_block_capacity,
                    (yyval.variable_def)->state_block_count + 1, sizeof(*(yyval.variable_def)->state_blocks))))
                YYABORT;
            (yyval.variable_def)->state_blocks[(yyval.variable_def)->state_block_count++] = (yyvsp[-1].state_block);
        }
    break;

  case 206: /* variable_def: variable_decl '{' state_block_start state_block_list '}'  */
        {
            (yyval.variable_def) = (yyvsp[-4].variable_def);
            ctx->in_state_block = 0;

            (yyval.variable_def)->state_blocks = (yyvsp[-1].variable_def)->state_blocks;
            (yyval.variable_def)->state_block_count = (yyvsp[-1].variable_def)->state_block_count;
            (yyval.variable_def)->state_block_capacity = (yyvsp[-1].variable_def)->state_block_capacity;
            (yyvsp[-1].variable_def)->state_blocks = NULL;
            (yyvsp[-1].variable_def)->state_block_count = 0;
            (yyvsp[-1].variable_def)->state_block_capacity = 0;
            free_parse_variable_def((yyvsp[-1].variable_def));
        }
    break;

  case 207: /* variable_def_typed: var_modifiers struct_spec variable_def  */
        {
            uint32_t modifiers = (yyvsp[-2].modifiers);
            struct hlsl_type *type;

            if (!(type = apply_type_modifiers(ctx, (yyvsp[-1].type), &modifiers, true, &(yylsp[-2]))))
                YYABORT;

            check_invalid_non_parameter_modifiers(ctx, modifiers, &(yylsp[-2]));

            (yyval.variable_def) = (yyvsp[0].variable_def);
            (yyval.variable_def)->basic_type = type;
            (yyval.variable_def)->modifiers = modifiers;
            (yyval.variable_def)->modifiers_loc = (yylsp[-2]);
        }
    break;

  case 208: /* variable_def_typed: var_modifiers type variable_def  */
        {
            uint32_t modifiers = (yyvsp[-2].modifiers);
            struct hlsl_type *type;

            if (!(type = apply_type_modifiers(ctx, (yyvsp[-1].type), &modifiers, true, &(yylsp[-2]))))
                YYABORT;

            check_invalid_non_parameter_modifiers(ctx, modifiers, &(yylsp[-2]));

            (yyval.variable_def) = (yyvsp[0].variable_def);
            (yyval.variable_def)->basic_type = type;
            (yyval.variable_def)->modifiers = modifiers;
            (yyval.variable_def)->modifiers_loc = (yylsp[-2]);
        }
    break;

  case 209: /* array: '[' ']'  */
        {
            (yyval.intval) = HLSL_ARRAY_ELEMENTS_COUNT_IMPLICIT;
        }
    break;

  case 210: /* array: '[' expr ']'  */
        {
            (yyval.intval) = evaluate_static_expression_as_uint(ctx, (yyvsp[-1].block), &(yylsp[-1]));
            destroy_block((yyvsp[-1].block));

            if (!(yyval.intval))
            {
                hlsl_error(ctx, &(yylsp[-1]), VKD3D_SHADER_ERROR_HLSL_INVALID_SIZE,
                        "Array size is not a positive integer constant.");
                YYABORT;
            }

            if ((yyval.intval) > 65536)
            {
                hlsl_error(ctx, &(yylsp[-1]), VKD3D_SHADER_ERROR_HLSL_INVALID_SIZE,
                        "Array size %u is not between 1 and 65536.", (yyval.intval));
                YYABORT;
            }
        }
    break;

  case 211: /* arrays: %empty  */
        {
            (yyval.arrays).sizes = NULL;
            (yyval.arrays).count = 0;
        }
    break;

  case 212: /* arrays: array arrays  */
        {
            uint32_t *new_array;

            (yyval.arrays) = (yyvsp[0].arrays);

            if (!(new_array = hlsl_realloc(ctx, (yyval.arrays).sizes, ((yyval.arrays).count + 1) * sizeof(*new_array))))
            {
                vkd3d_free((yyval.arrays).sizes);
                YYABORT;
            }

            (yyval.arrays).sizes = new_array;
            (yyval.arrays).sizes[(yyval.arrays).count++] = (yyvsp[-1].intval);
        }
    break;

  case 213: /* var_modifiers: %empty  */
        {
            (yyval.modifiers) = 0;
        }
    break;

  case 214: /* var_modifiers: KW_EXTERN var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_STORAGE_EXTERN, &(yylsp[-1]));
        }
    break;

  case 215: /* var_modifiers: KW_NOINTERPOLATION var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_STORAGE_NOINTERPOLATION, &(yylsp[-1]));
        }
    break;

  case 216: /* var_modifiers: KW_CENTROID var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_STORAGE_CENTROID, &(yylsp[-1]));
        }
    break;

  case 217: /* var_modifiers: KW_LINEAR var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_STORAGE_LINEAR, &(yylsp[-1]));
        }
    break;

  case 218: /* var_modifiers: KW_NOPERSPECTIVE var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_STORAGE_NOPERSPECTIVE, &(yylsp[-1]));
        }
    break;

  case 219: /* var_modifiers: KW_SHARED var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_STORAGE_SHARED, &(yylsp[-1]));
        }
    break;

  case 220: /* var_modifiers: KW_GROUPSHARED var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_STORAGE_GROUPSHARED, &(yylsp[-1]));
        }
    break;

  case 221: /* var_modifiers: KW_STATIC var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_STORAGE_STATIC, &(yylsp[-1]));
        }
    break;

  case 222: /* var_modifiers: KW_UNIFORM var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_STORAGE_UNIFORM, &(yylsp[-1]));
        }
    break;

  case 223: /* var_modifiers: KW_VOLATILE var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_MODIFIER_VOLATILE, &(yylsp[-1]));
        }
    break;

  case 224: /* var_modifiers: KW_CONST var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_MODIFIER_CONST, &(yylsp[-1]));
        }
    break;

  case 225: /* var_modifiers: KW_ROW_MAJOR var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_MODIFIER_ROW_MAJOR, &(yylsp[-1]));
        }
    break;

  case 226: /* var_modifiers: KW_COLUMN_MAJOR var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_MODIFIER_COLUMN_MAJOR, &(yylsp[-1]));
        }
    break;

  case 227: /* var_modifiers: KW_IN var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_STORAGE_IN, &(yylsp[-1]));
        }
    break;

  case 228: /* var_modifiers: KW_OUT var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_STORAGE_OUT, &(yylsp[-1]));
        }
    break;

  case 229: /* var_modifiers: KW_INOUT var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_STORAGE_IN | HLSL_STORAGE_OUT, &(yylsp[-1]));
        }
    break;

  case 230: /* var_modifiers: KW_INLINE var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_MODIFIER_INLINE, &(yylsp[-1]));
        }
    break;

  case 231: /* var_modifiers: KW_EXPORT var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_STORAGE_EXPORT, &(yylsp[-1]));
        }
    break;

  case 232: /* var_modifiers: KW_UNORM var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_MODIFIER_UNORM, &(yylsp[-1]));
        }
    break;

  case 233: /* var_modifiers: KW_SNORM var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_MODIFIER_SNORM, &(yylsp[-1]));
        }
    break;

  case 234: /* var_modifiers: KW_LINE var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_PRIMITIVE_LINE, &(yylsp[-1]));
        }
    break;

  case 235: /* var_modifiers: KW_LINEADJ var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_PRIMITIVE_LINEADJ, &(yylsp[-1]));
        }
    break;

  case 236: /* var_modifiers: KW_POINT var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_PRIMITIVE_POINT, &(yylsp[-1]));
        }
    break;

  case 237: /* var_modifiers: KW_TRIANGLE var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_PRIMITIVE_TRIANGLE, &(yylsp[-1]));
        }
    break;

  case 238: /* var_modifiers: KW_TRIANGLEADJ var_modifiers  */
        {
            (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_PRIMITIVE_TRIANGLEADJ, &(yylsp[-1]));
        }
    break;

  case 239: /* var_modifiers: var_modifier_identifier var_modifiers  */
        {
            (yyval.modifiers) = (yyvsp[0].modifiers);

            if (!strcmp((yyvsp[-1].name), "precise"))
                (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_MODIFIER_PRECISE, &(yylsp[-1]));
            else if (!strcmp((yyvsp[-1].name), "single"))
                (yyval.modifiers) = add_modifiers(ctx, (yyvsp[0].modifiers), HLSL_MODIFIER_SINGLE, &(yylsp[-1]));
            else
                hlsl_error(ctx, &(yylsp[-1]), VKD3D_SHADER_ERROR_HLSL_UNKNOWN_MODIFIER,
                        "Unknown modifier %s.", debugstr_a((yyvsp[-1].name)));
            vkd3d_free((yyvsp[-1].name));
        }
    break;

  case 240: /* complex_initializer: initializer_expr  */
        {
            (yyval.initializer).args_count = 1;
            if (!((yyval.initializer).args = hlsl_alloc(ctx, sizeof(*(yyval.initializer).args))))
            {
                destroy_block((yyvsp[0].block));
                YYABORT;
            }
            (yyval.initializer).args[0] = node_from_block((yyvsp[0].block));
            (yyval.initializer).instrs = (yyvsp[0].block);
            (yyval.initializer).braces = false;
            (yyval.initializer).loc = (yyloc);
        }
    break;

  case 241: /* complex_initializer: '{' complex_initializer_list '}'  */
        {
            (yyval.initializer) = (yyvsp[-1].initializer);
            (yyval.initializer).braces = true;
        }
    break;

  case 242: /* complex_initializer: '{' complex_initializer_list ',' '}'  */
        {
            (yyval.initializer) = (yyvsp[-2].initializer);
            (yyval.initializer).braces = true;
        }
    break;

  case 244: /* complex_initializer_list: complex_initializer_list ',' complex_initializer  */
        {
            struct hlsl_ir_node **new_args;
            unsigned int i;

            (yyval.initializer) = (yyvsp[-2].initializer);
            if (!(new_args = hlsl_realloc(ctx, (yyval.initializer).args, ((yyval.initializer).args_count + (yyvsp[0].initializer).args_count) * sizeof(*(yyval.initializer).args))))
            {
                cleanup_parse_initializer(&(yyval.initializer));
                cleanup_parse_initializer(&(yyvsp[0].initializer));
                YYABORT;
            }
            (yyval.initializer).args = new_args;
            for (i = 0; i < (yyvsp[0].initializer).args_count; ++i)
                (yyval.initializer).args[(yyval.initializer).args_count++] = (yyvsp[0].initializer).args[i];
            hlsl_block_add_block((yyval.initializer).instrs, (yyvsp[0].initializer).instrs);
            cleanup_parse_initializer(&(yyvsp[0].initializer));
            (yyval.initializer).loc = (yyloc);
        }
    break;

  case 246: /* initializer_expr_list: initializer_expr  */
        {
            (yyval.initializer).args_count = 1;
            if (!((yyval.initializer).args = hlsl_alloc(ctx, sizeof(*(yyval.initializer).args))))
            {
                destroy_block((yyvsp[0].block));
                YYABORT;
            }
            (yyval.initializer).args[0] = node_from_block((yyvsp[0].block));
            (yyval.initializer).instrs = (yyvsp[0].block);
            (yyval.initializer).braces = false;
            (yyval.initializer).loc = (yyloc);
        }
    break;

  case 247: /* initializer_expr_list: initializer_expr_list ',' initializer_expr  */
        {
            struct hlsl_ir_node **new_args;

            (yyval.initializer) = (yyvsp[-2].initializer);
            if (!(new_args = hlsl_realloc(ctx, (yyval.initializer).args, ((yyval.initializer).args_count + 1) * sizeof(*(yyval.initializer).args))))
            {
                cleanup_parse_initializer(&(yyval.initializer));
                destroy_block((yyvsp[0].block));
                YYABORT;
            }
            (yyval.initializer).args = new_args;
            (yyval.initializer).args[(yyval.initializer).args_count++] = node_from_block((yyvsp[0].block));
            hlsl_block_add_block((yyval.initializer).instrs, (yyvsp[0].block));
            destroy_block((yyvsp[0].block));
        }
    break;

  case 248: /* boolean: KW_TRUE  */
        {
            (yyval.boolval) = true;
        }
    break;

  case 249: /* boolean: KW_FALSE  */
        {
            (yyval.boolval) = false;
        }
    break;

  case 251: /* statement_list: statement_list statement  */
        {
            (yyval.block) = (yyvsp[-1].block);
            hlsl_block_add_block((yyval.block), (yyvsp[0].block));
            destroy_block((yyvsp[0].block));
        }
    break;

  case 254: /* statement: scope_start compound_statement  */
        {
            hlsl_pop_scope(ctx);
            (yyval.block) = (yyvsp[0].block);
        }
    break;

  case 259: /* jump_statement: KW_BREAK ';'  */
        {
            if (!is_break_allowed(ctx->cur_scope))
            {
                hlsl_error(ctx, &(yylsp[-1]), VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX,
                        "The 'break' statement must be used inside of a loop or a switch.");
            }

            if (!((yyval.block) = make_empty_block(ctx)))
                YYABORT;
            hlsl_block_add_jump(ctx, (yyval.block), HLSL_IR_JUMP_BREAK, NULL, &(yylsp[-1]));
        }
    break;

  case 260: /* jump_statement: KW_CONTINUE ';'  */
        {
            check_continue(ctx, ctx->cur_scope, &(yylsp[-1]));

            if (!((yyval.block) = make_empty_block(ctx)))
                YYABORT;
            hlsl_block_add_jump(ctx, (yyval.block), HLSL_IR_JUMP_UNRESOLVED_CONTINUE, NULL, &(yylsp[-1]));
        }
    break;

  case 261: /* jump_statement: KW_RETURN expr ';'  */
        {
            (yyval.block) = (yyvsp[-1].block);
            if (!add_return(ctx, (yyval.block), node_from_block((yyval.block)), &(yylsp[-2])))
                YYABORT;
        }
    break;

  case 262: /* jump_statement: KW_RETURN ';'  */
        {
            if (!((yyval.block) = make_empty_block(ctx)))
                YYABORT;
            if (!add_return(ctx, (yyval.block), NULL, &(yylsp[-1])))
            {
                destroy_block((yyval.block));
                YYABORT;
            }
        }
    break;

  case 263: /* jump_statement: KW_DISCARD ';'  */
        {
            struct hlsl_ir_node *c;

            if (!((yyval.block) = make_empty_block(ctx)))
                YYABORT;
            c = hlsl_block_add_uint_constant(ctx, (yyval.block), ~0u, &(yylsp[-1]));
            hlsl_block_add_jump(ctx, (yyval.block), HLSL_IR_JUMP_DISCARD_NZ, c, &(yylsp[-1]));
        }
    break;

  case 264: /* selection_statement: attribute_list_optional KW_IF '(' expr ')' if_body  */
        {
            struct hlsl_ir_node *condition = node_from_block((yyvsp[-2].block));
            const struct parse_attribute_list *attributes = &(yyvsp[-5].attr_list);
            enum hlsl_if_flatten_type flatten_type = HLSL_IF_FLATTEN_DEFAULT;
            unsigned int i;

            check_attribute_list_for_duplicates(ctx, attributes);

            for (i = 0; i < attributes->count; ++i)
            {
                const struct hlsl_attribute *attr = attributes->attrs[i];

                if (!strcmp(attr->name, "branch"))
                {
                    if (flatten_type == HLSL_IF_FORCE_FLATTEN)
                        hlsl_error(ctx, &(yylsp[-5]), VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX,
                                "The 'branch' and 'flatten' attributes are mutually exclusive.");
                    flatten_type = HLSL_IF_FORCE_BRANCH;
                }
                else if (!strcmp(attr->name, "flatten"))
                {
                    if (flatten_type == HLSL_IF_FORCE_BRANCH)
                        hlsl_error(ctx, &(yylsp[-5]), VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX,
                                "The 'branch' and 'flatten' attributes are mutually exclusive.");
                    flatten_type = HLSL_IF_FORCE_FLATTEN;
                }
                else
                {
                    hlsl_warning(ctx, &(yylsp[-5]), VKD3D_SHADER_WARNING_HLSL_UNKNOWN_ATTRIBUTE, "Unrecognized attribute '%s'.", attr->name);
                }
            }

            if (flatten_type == HLSL_IF_FORCE_BRANCH && hlsl_version_lt(ctx, 2, 1))
            {
                hlsl_error(ctx, &(yylsp[-5]), VKD3D_SHADER_ERROR_HLSL_INCOMPATIBLE_PROFILE,
                        "The 'branch' attribute requires shader model 2.1 or higher.");
            }

            check_condition_type(ctx, condition);

            condition = add_cast(ctx, (yyvsp[-2].block), condition, hlsl_get_scalar_type(ctx, HLSL_TYPE_BOOL), &(yylsp[-2]));
            hlsl_block_add_if(ctx, (yyvsp[-2].block), condition, (yyvsp[0].if_body).then_block, (yyvsp[0].if_body).else_block, flatten_type, false, &(yylsp[-4]));

            destroy_block((yyvsp[0].if_body).then_block);
            destroy_block((yyvsp[0].if_body).else_block);
            cleanup_parse_attribute_list(&(yyvsp[-5].attr_list));

            (yyval.block) = (yyvsp[-2].block);
        }
    break;

  case 265: /* if_body: statement  */
        {
            (yyval.if_body).then_block = (yyvsp[0].block);
            (yyval.if_body).else_block = NULL;
        }
    break;

  case 266: /* if_body: statement KW_ELSE statement  */
        {
            (yyval.if_body).then_block = (yyvsp[-2].block);
            (yyval.if_body).else_block = (yyvsp[0].block);
        }
    break;

  case 267: /* loop_statement: attribute_list_optional loop_scope_start KW_WHILE '(' expr ')' statement  */
        {
            (yyval.block) = create_loop(ctx, HLSL_LOOP_WHILE, &(yyvsp[-6].attr_list), NULL, (yyvsp[-2].block), NULL, (yyvsp[0].block), &(yylsp[-4]));
            hlsl_pop_scope(ctx);
            cleanup_parse_attribute_list(&(yyvsp[-6].attr_list));
        }
    break;

  case 268: /* loop_statement: attribute_list_optional loop_scope_start KW_DO statement KW_WHILE '(' expr ')' ';'  */
        {
            (yyval.block) = create_loop(ctx, HLSL_LOOP_DO_WHILE, &(yyvsp[-8].attr_list), NULL, (yyvsp[-2].block), NULL, (yyvsp[-5].block), &(yylsp[-6]));
            hlsl_pop_scope(ctx);
            cleanup_parse_attribute_list(&(yyvsp[-8].attr_list));
        }
    break;

  case 269: /* loop_statement: attribute_list_optional loop_scope_start KW_FOR '(' expr_statement expr_statement expr_optional ')' statement  */
        {
            (yyval.block) = create_loop(ctx, HLSL_LOOP_FOR, &(yyvsp[-8].attr_list), (yyvsp[-4].block), (yyvsp[-3].block), (yyvsp[-2].block), (yyvsp[0].block), &(yylsp[-6]));
            hlsl_pop_scope(ctx);
            cleanup_parse_attribute_list(&(yyvsp[-8].attr_list));
        }
    break;

  case 270: /* loop_statement: attribute_list_optional loop_scope_start KW_FOR '(' declaration expr_statement expr_optional ')' statement  */
        {
            (yyval.block) = create_loop(ctx, HLSL_LOOP_FOR, &(yyvsp[-8].attr_list), (yyvsp[-4].block), (yyvsp[-3].block), (yyvsp[-2].block), (yyvsp[0].block), &(yylsp[-6]));
            hlsl_pop_scope(ctx);
            cleanup_parse_attribute_list(&(yyvsp[-8].attr_list));
        }
    break;

  case 271: /* switch_statement: attribute_list_optional switch_scope_start KW_SWITCH '(' expr ')' '{' switch_cases '}'  */
        {
            (yyval.block) = (yyvsp[-4].block);
            if (!add_switch(ctx, (yyval.block), &(yyvsp[-8].attr_list), (yyvsp[-1].list), &(yylsp[-6])))
                YYABORT;
            hlsl_pop_scope(ctx);
        }
    break;

  case 272: /* switch_case: KW_CASE expr ':' statement_list  */
        {
            struct hlsl_ir_switch_case *c;
            unsigned int value;

            value = evaluate_static_expression_as_uint(ctx, (yyvsp[-2].block), &(yylsp[-2]));

            c = hlsl_new_switch_case(ctx, value, false, (yyvsp[0].block), &(yylsp[-2]));

            destroy_block((yyvsp[-2].block));
            destroy_block((yyvsp[0].block));

            if (!c)
                YYABORT;
            (yyval.switch_case) = c;
        }
    break;

  case 273: /* switch_case: KW_CASE expr ':'  */
        {
            struct hlsl_ir_switch_case *c;
            unsigned int value;

            value = evaluate_static_expression_as_uint(ctx, (yyvsp[-1].block), &(yylsp[-1]));

            c = hlsl_new_switch_case(ctx, value, false, NULL, &(yylsp[-1]));

            destroy_block((yyvsp[-1].block));

            if (!c)
                YYABORT;
            (yyval.switch_case) = c;
        }
    break;

  case 274: /* switch_case: KW_DEFAULT ':' statement_list  */
        {
            struct hlsl_ir_switch_case *c;

            c = hlsl_new_switch_case(ctx, 0, true, (yyvsp[0].block), &(yylsp[-2]));

            destroy_block((yyvsp[0].block));

            if (!c)
                YYABORT;
            (yyval.switch_case) = c;
        }
    break;

  case 275: /* switch_case: KW_DEFAULT ':'  */
        {
            struct hlsl_ir_switch_case *c;

            if (!(c = hlsl_new_switch_case(ctx, 0, true, NULL, &(yylsp[-1]))))
                YYABORT;
            (yyval.switch_case) = c;
        }
    break;

  case 276: /* switch_cases: switch_case  */
        {
            struct hlsl_ir_switch_case *c = LIST_ENTRY((yyvsp[0].switch_case), struct hlsl_ir_switch_case, entry);
            if (!((yyval.list) = make_empty_list(ctx)))
            {
                hlsl_free_ir_switch_case(c);
                YYABORT;
            }
            list_add_head((yyval.list), &(yyvsp[0].switch_case)->entry);
        }
    break;

  case 277: /* switch_cases: switch_cases switch_case  */
        {
            (yyval.list) = (yyvsp[-1].list);
            check_duplicated_switch_cases(ctx, (yyvsp[0].switch_case), (yyval.list));
            list_add_tail((yyval.list), &(yyvsp[0].switch_case)->entry);
        }
    break;

  case 278: /* expr_optional: %empty  */
        {
            if (!((yyval.block) = make_empty_block(ctx)))
                YYABORT;
        }
    break;

  case 280: /* expr_statement: expr_optional ';'  */
        {
            (yyval.block) = (yyvsp[-1].block);
        }
    break;

  case 281: /* func_arguments: %empty  */
        {
            (yyval.initializer).args = NULL;
            (yyval.initializer).args_count = 0;
            if (!((yyval.initializer).instrs = make_empty_block(ctx)))
                YYABORT;
            (yyval.initializer).braces = false;
            (yyval.initializer).loc = (yyloc);
        }
    break;

  case 283: /* primary_expr: C_FLOAT  */
        {
            if (!((yyval.block) = make_empty_block(ctx)))
                YYABORT;
            hlsl_block_add_float_constant(ctx, (yyval.block), (yyvsp[0].floatval), &(yylsp[0]));
        }
    break;

  case 284: /* primary_expr: C_INTEGER  */
        {
            if (!((yyval.block) = make_empty_block(ctx)))
                YYABORT;
            hlsl_block_add_int_constant(ctx, (yyval.block), (yyvsp[0].intval), &(yylsp[0]));
        }
    break;

  case 285: /* primary_expr: C_UNSIGNED  */
        {
            if (!((yyval.block) = make_empty_block(ctx)))
                YYABORT;
            hlsl_block_add_uint_constant(ctx, (yyval.block), (yyvsp[0].intval), &(yylsp[0]));
        }
    break;

  case 286: /* primary_expr: boolean  */
        {
            if (!((yyval.block) = make_empty_block(ctx)))
                YYABORT;
            hlsl_block_add_bool_constant(ctx, (yyval.block), (yyvsp[0].boolval), &(yylsp[0]));
        }
    break;

  case 287: /* primary_expr: STRING  */
        {
            struct hlsl_ir_node *c;

            if (!(c = hlsl_new_string_constant(ctx, (yyvsp[0].name), &(yylsp[0]))))
            {
                vkd3d_free((yyvsp[0].name));
                YYABORT;
            }
            vkd3d_free((yyvsp[0].name));

            if (!((yyval.block) = make_block(ctx, c)))
            {
                hlsl_free_instr(c);
                YYABORT;
            }
        }
    break;

  case 288: /* primary_expr: KW_NULL  */
        {
            struct hlsl_ir_node *c;

            if (!(c = hlsl_new_null_constant(ctx, &(yylsp[0]))))
                YYABORT;
            if (!((yyval.block) = make_block(ctx, c)))
            {
                hlsl_free_instr(c);
                YYABORT;
            }
        }
    break;

  case 289: /* primary_expr: VAR_IDENTIFIER  */
        {
            struct hlsl_ir_var *var;

            if (!((yyval.block) = make_empty_block(ctx)))
                YYABORT;

            if ((var = hlsl_get_var_in_scope((yyvsp[0].identifier).scope, (yyvsp[0].identifier).name)))
            {
                hlsl_block_add_simple_load(ctx, (yyval.block), var, &(yylsp[0]));
            }
            else
            {
                hlsl_error(ctx, &(yylsp[0]), VKD3D_SHADER_ERROR_HLSL_NOT_DEFINED,
                        "Variable \"%s\" is not defined.", (yyvsp[0].identifier).qualified_name);
                (yyval.block)->value = ctx->error_instr;
            }
            hlsl_identifier_cleanup(&(yyvsp[0].identifier));
        }
    break;

  case 290: /* primary_expr: '(' expr ')'  */
        {
            (yyval.block) = (yyvsp[-1].block);
        }
    break;

  case 291: /* primary_expr: KW_COMPILE unqualified_identifier unqualified_var_identifier '(' func_arguments ')'  */
        {
            if (!((yyval.block) = add_shader_compilation(ctx, (yyvsp[-4].name), (yyvsp[-3].name), &(yyvsp[-1].initializer), &(yylsp[-5]))))
            {
                vkd3d_free((yyvsp[-4].name));
                vkd3d_free((yyvsp[-3].name));
                YYABORT;
            }
            vkd3d_free((yyvsp[-4].name));
            vkd3d_free((yyvsp[-3].name));
        }
    break;

  case 292: /* primary_expr: KW_COMPILESHADER '(' unqualified_identifier ',' unqualified_var_identifier '(' func_arguments ')' ')'  */
        {
            if (!((yyval.block) = add_shader_compilation(ctx, (yyvsp[-6].name), (yyvsp[-4].name), &(yyvsp[-2].initializer), &(yylsp[-8]))))
            {
                vkd3d_free((yyvsp[-6].name));
                vkd3d_free((yyvsp[-4].name));
                YYABORT;
            }
            vkd3d_free((yyvsp[-6].name));
            vkd3d_free((yyvsp[-4].name));
        }
    break;

  case 293: /* primary_expr: var_identifier '(' func_arguments ')'  */
        {
            (yyval.block) = add_call(ctx, &(yyvsp[-3].identifier), &(yyvsp[-1].initializer), &(yylsp[-3]));
            hlsl_identifier_cleanup(&(yyvsp[-3].identifier));
        }
    break;

  case 294: /* primary_expr: KW_SAMPLER_STATE '{' state_block_start state_block '}'  */
        {
            struct hlsl_ir_node *sampler_state;
            ctx->in_state_block = 0;

            if (!ctx->in_state_block && ctx->cur_scope != ctx->globals)
                hlsl_error(ctx, &(yylsp[-4]), VKD3D_SHADER_ERROR_HLSL_MISPLACED_SAMPLER_STATE,
                        "sampler_state must be in global scope or a state block.");

            if (!(sampler_state = hlsl_new_sampler_state(ctx, (yyvsp[-1].state_block), &(yylsp[-4]))))
            {
                hlsl_free_state_block((yyvsp[-1].state_block));
                YYABORT;
            }
            hlsl_free_state_block((yyvsp[-1].state_block));

            if (!((yyval.block) = make_block(ctx, sampler_state)))
                YYABORT;
        }
    break;

  case 295: /* primary_expr: NEW_IDENTIFIER  */
        {
            if (ctx->in_state_block)
            {
                struct hlsl_ir_node *constant;

                if (!(constant = hlsl_new_stateblock_constant(ctx, (yyvsp[0].identifier).qualified_name, &(yylsp[0]))))
                    YYABORT;
                hlsl_identifier_cleanup(&(yyvsp[0].identifier));

                if (!((yyval.block) = make_block(ctx, constant)))
                    YYABORT;
            }
            else
            {
                hlsl_error(ctx, &(yylsp[0]), VKD3D_SHADER_ERROR_HLSL_NOT_DEFINED,
                        "Identifier \"%s\" is not declared.", (yyvsp[0].identifier).qualified_name);
                hlsl_identifier_cleanup(&(yyvsp[0].identifier));

                if (!((yyval.block) = make_empty_block(ctx)))
                    YYABORT;
                (yyval.block)->value = ctx->error_instr;
            }
        }
    break;

  case 297: /* postfix_expr: postfix_expr OP_INC  */
        {
            if (!add_increment(ctx, (yyvsp[-1].block), false, true, &(yylsp[0])))
            {
                destroy_block((yyvsp[-1].block));
                YYABORT;
            }
            (yyval.block) = (yyvsp[-1].block);
        }
    break;

  case 298: /* postfix_expr: postfix_expr OP_DEC  */
        {
            if (!add_increment(ctx, (yyvsp[-1].block), true, true, &(yylsp[0])))
            {
                destroy_block((yyvsp[-1].block));
                YYABORT;
            }
            (yyval.block) = (yyvsp[-1].block);
        }
    break;

  case 299: /* postfix_expr: postfix_expr '.' unqualified_identifier  */
        {
            struct hlsl_ir_node *node = node_from_block((yyvsp[-2].block));

            if (node->data_type->class == HLSL_CLASS_STRUCT)
            {
                add_record_access_recurse(ctx, (yyvsp[-2].block), (yyvsp[0].name), &(yylsp[-1]));
            }
            else if (hlsl_is_numeric_type(node->data_type))
            {
                struct hlsl_ir_node *swizzle;

                if ((swizzle = get_swizzle(ctx, node, (yyvsp[0].name), &(yylsp[0]))))
                {
                    hlsl_block_add_instr((yyvsp[-2].block), swizzle);
                }
                else
                {
                    hlsl_error(ctx, &(yylsp[0]), VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX, "Invalid swizzle \"%s\".", (yyvsp[0].name));
                    (yyvsp[-2].block)->value = ctx->error_instr;
                }
            }
            else if (add_object_property_access(ctx, (yyvsp[-2].block), node, (yyvsp[0].name), &(yylsp[-1])))
            {
            }
            else if (node->data_type->class != HLSL_CLASS_ERROR)
            {
                hlsl_error(ctx, &(yylsp[0]), VKD3D_SHADER_ERROR_HLSL_INVALID_SYNTAX, "Invalid subscript \"%s\".", (yyvsp[0].name));
                (yyvsp[-2].block)->value = ctx->error_instr;
            }
            vkd3d_free((yyvsp[0].name));
            (yyval.block) = (yyvsp[-2].block);
        }
    break;

  case 300: /* postfix_expr: postfix_expr '[' expr ']'  */
        {
            struct hlsl_ir_node *array = node_from_block((yyvsp[-3].block)), *index = node_from_block((yyvsp[-1].block));

            hlsl_block_add_block((yyvsp[-1].block), (yyvsp[-3].block));
            destroy_block((yyvsp[-3].block));

            if (!add_array_access(ctx, (yyvsp[-1].block), array, index, &(yylsp[-2])))
            {
                destroy_block((yyvsp[-1].block));
                YYABORT;
            }
            (yyval.block) = (yyvsp[-1].block);
        }
    break;

  case 301: /* postfix_expr: var_modifiers type '(' initializer_expr_list ')'  */
        {
            if ((yyvsp[-4].modifiers))
                hlsl_error(ctx, &(yylsp[-4]), VKD3D_SHADER_ERROR_HLSL_INVALID_MODIFIER,
                        "Modifiers are not allowed on constructors.");

            if (!((yyval.block) = add_constructor(ctx, (yyvsp[-3].type), &(yyvsp[-1].initializer), &(yylsp[-3]))))
            {
                cleanup_parse_initializer(&(yyvsp[-1].initializer));
                YYABORT;
            }
        }
    break;

  case 302: /* postfix_expr: postfix_expr '.' unqualified_identifier '(' func_arguments ')'  */
        {
            struct hlsl_ir_node *object = node_from_block((yyvsp[-5].block));

            hlsl_block_add_block((yyvsp[-5].block), (yyvsp[-1].initializer).instrs);
            vkd3d_free((yyvsp[-1].initializer).instrs);

            if (!add_method_call(ctx, (yyvsp[-5].block), object, (yyvsp[-3].name), &(yyvsp[-1].initializer), &(yylsp[-3])))
            {
                destroy_block((yyvsp[-5].block));
                vkd3d_free((yyvsp[-3].name));
                vkd3d_free((yyvsp[-1].initializer).args);
                YYABORT;
            }
            vkd3d_free((yyvsp[-3].name));
            vkd3d_free((yyvsp[-1].initializer).args);
            (yyval.block) = (yyvsp[-5].block);
        }
    break;

  case 304: /* unary_expr: OP_INC unary_expr  */
        {
            if (!add_increment(ctx, (yyvsp[0].block), false, false, &(yylsp[-1])))
            {
                destroy_block((yyvsp[0].block));
                YYABORT;
            }
            (yyval.block) = (yyvsp[0].block);
        }
    break;

  case 305: /* unary_expr: OP_DEC unary_expr  */
        {
            if (!add_increment(ctx, (yyvsp[0].block), true, false, &(yylsp[-1])))
            {
                destroy_block((yyvsp[0].block));
                YYABORT;
            }
            (yyval.block) = (yyvsp[0].block);
        }
    break;

  case 306: /* unary_expr: '+' unary_expr  */
        {
            (yyval.block) = (yyvsp[0].block);
        }
    break;

  case 307: /* unary_expr: '-' unary_expr  */
        {
            add_unary_arithmetic_expr(ctx, (yyvsp[0].block), HLSL_OP1_NEG, node_from_block((yyvsp[0].block)), &(yylsp[-1]));
            (yyval.block) = (yyvsp[0].block);
        }
    break;

  case 308: /* unary_expr: '~' unary_expr  */
        {
            add_unary_bitwise_expr(ctx, (yyvsp[0].block), HLSL_OP1_BIT_NOT, node_from_block((yyvsp[0].block)), &(yylsp[-1]));
            (yyval.block) = (yyvsp[0].block);
        }
    break;

  case 309: /* unary_expr: '!' unary_expr  */
        {
            add_unary_logical_expr(ctx, (yyvsp[0].block), HLSL_OP1_LOGIC_NOT, node_from_block((yyvsp[0].block)), &(yylsp[-1]));
            (yyval.block) = (yyvsp[0].block);
        }
    break;

  case 310: /* unary_expr: '(' var_modifiers type arrays ')' unary_expr  */
        {
            if ((yyvsp[-4].modifiers))
                hlsl_error(ctx, &(yylsp[-4]), VKD3D_SHADER_ERROR_HLSL_INVALID_MODIFIER,
                        "Modifiers are not allowed on casts.");

            add_explicit_conversion(ctx, (yyvsp[0].block), (yyvsp[-3].type), &(yyvsp[-2].arrays), &(yylsp[-3]));
            vkd3d_free((yyvsp[-2].arrays).sizes);
            (yyval.block) = (yyvsp[0].block);
        }
    break;

  case 312: /* mul_expr: mul_expr '*' unary_expr  */
        {
            (yyval.block) = add_binary_expr_merge(ctx, (yyvsp[-2].block), (yyvsp[0].block), HLSL_OP2_MUL, &(yylsp[-1]));
        }
    break;

  case 313: /* mul_expr: mul_expr '/' unary_expr  */
        {
            (yyval.block) = add_binary_expr_merge(ctx, (yyvsp[-2].block), (yyvsp[0].block), HLSL_OP2_DIV, &(yylsp[-1]));
        }
    break;

  case 314: /* mul_expr: mul_expr '%' unary_expr  */
        {
            (yyval.block) = add_binary_expr_merge(ctx, (yyvsp[-2].block), (yyvsp[0].block), HLSL_OP2_MOD, &(yylsp[-1]));
        }
    break;

  case 316: /* add_expr: add_expr '+' mul_expr  */
        {
            (yyval.block) = add_binary_expr_merge(ctx, (yyvsp[-2].block), (yyvsp[0].block), HLSL_OP2_ADD, &(yylsp[-1]));
        }
    break;

  case 317: /* add_expr: add_expr '-' mul_expr  */
        {
            struct hlsl_ir_node *neg;

            if (!(neg = add_unary_arithmetic_expr(ctx, (yyvsp[0].block), HLSL_OP1_NEG, node_from_block((yyvsp[0].block)), &(yylsp[-1]))))
                YYABORT;
            (yyval.block) = add_binary_expr_merge(ctx, (yyvsp[-2].block), (yyvsp[0].block), HLSL_OP2_ADD, &(yylsp[-1]));
        }
    break;

  case 319: /* shift_expr: shift_expr OP_LEFTSHIFT add_expr  */
        {
            (yyval.block) = add_binary_expr_merge(ctx, (yyvsp[-2].block), (yyvsp[0].block), HLSL_OP2_LSHIFT, &(yylsp[-1]));
        }
    break;

  case 320: /* shift_expr: shift_expr OP_RIGHTSHIFT add_expr  */
        {
            (yyval.block) = add_binary_expr_merge(ctx, (yyvsp[-2].block), (yyvsp[0].block), HLSL_OP2_RSHIFT, &(yylsp[-1]));
        }
    break;

  case 322: /* relational_expr: relational_expr '<' shift_expr  */
        {
            (yyval.block) = add_binary_expr_merge(ctx, (yyvsp[-2].block), (yyvsp[0].block), HLSL_OP2_LESS, &(yylsp[-1]));
        }
    break;

  case 323: /* relational_expr: relational_expr '>' shift_expr  */
        {
            (yyval.block) = add_binary_expr_merge(ctx, (yyvsp[0].block), (yyvsp[-2].block), HLSL_OP2_LESS, &(yylsp[-1]));
        }
    break;

  case 324: /* relational_expr: relational_expr OP_LE shift_expr  */
        {
            (yyval.block) = add_binary_expr_merge(ctx, (yyvsp[0].block), (yyvsp[-2].block), HLSL_OP2_GEQUAL, &(yylsp[-1]));
        }
    break;

  case 325: /* relational_expr: relational_expr OP_GE shift_expr  */
        {
            (yyval.block) = add_binary_expr_merge(ctx, (yyvsp[-2].block), (yyvsp[0].block), HLSL_OP2_GEQUAL, &(yylsp[-1]));
        }
    break;

  case 327: /* equality_expr: equality_expr OP_EQ relational_expr  */
        {
            (yyval.block) = add_binary_expr_merge(ctx, (yyvsp[-2].block), (yyvsp[0].block), HLSL_OP2_EQUAL, &(yylsp[-1]));
        }
    break;

  case 328: /* equality_expr: equality_expr OP_NE relational_expr  */
        {
            (yyval.block) = add_binary_expr_merge(ctx, (yyvsp[-2].block), (yyvsp[0].block), HLSL_OP2_NEQUAL, &(yylsp[-1]));
        }
    break;

  case 330: /* bitand_expr: bitand_expr '&' equality_expr  */
        {
            (yyval.block) = add_binary_expr_merge(ctx, (yyvsp[-2].block), (yyvsp[0].block), HLSL_OP2_BIT_AND, &(yylsp[-1]));
        }
    break;

  case 332: /* bitxor_expr: bitxor_expr '^' bitand_expr  */
        {
            (yyval.block) = add_binary_expr_merge(ctx, (yyvsp[-2].block), (yyvsp[0].block), HLSL_OP2_BIT_XOR, &(yylsp[-1]));
        }
    break;

  case 334: /* bitor_expr: bitor_expr '|' bitxor_expr  */
        {
            (yyval.block) = add_binary_expr_merge(ctx, (yyvsp[-2].block), (yyvsp[0].block), HLSL_OP2_BIT_OR, &(yylsp[-1]));
        }
    break;

  case 336: /* logicand_expr: logicand_expr OP_AND bitor_expr  */
        {
            (yyval.block) = add_binary_expr_merge(ctx, (yyvsp[-2].block), (yyvsp[0].block), HLSL_OP2_LOGIC_AND, &(yylsp[-1]));
        }
    break;

  case 338: /* logicor_expr: logicor_expr OP_OR logicand_expr  */
        {
            (yyval.block) = add_binary_expr_merge(ctx, (yyvsp[-2].block), (yyvsp[0].block), HLSL_OP2_LOGIC_OR, &(yylsp[-1]));
        }
    break;

  case 340: /* conditional_expr: logicor_expr '?' expr ':' assignment_expr  */
        {
            struct hlsl_ir_node *cond = node_from_block((yyvsp[-4].block));
            struct hlsl_ir_node *first = node_from_block((yyvsp[-2].block));
            struct hlsl_ir_node *second = node_from_block((yyvsp[0].block));

            hlsl_block_add_block((yyvsp[-4].block), (yyvsp[-2].block));
            hlsl_block_add_block((yyvsp[-4].block), (yyvsp[0].block));
            destroy_block((yyvsp[-2].block));
            destroy_block((yyvsp[0].block));

            if (!add_ternary(ctx, (yyvsp[-4].block), cond, first, second))
            {
                destroy_block((yyvsp[-4].block));
                YYABORT;
            }
            (yyval.block) = (yyvsp[-4].block);
        }
    break;

  case 342: /* assignment_expr: unary_expr assign_op assignment_expr  */
        {
            struct hlsl_ir_node *lhs = node_from_block((yyvsp[-2].block)), *rhs = node_from_block((yyvsp[0].block));

            if (lhs->data_type->modifiers & HLSL_MODIFIER_CONST)
                hlsl_error(ctx, &(yylsp[-1]), VKD3D_SHADER_ERROR_HLSL_MODIFIES_CONST, "Statement modifies a const expression.");
            hlsl_block_add_block((yyvsp[0].block), (yyvsp[-2].block));
            destroy_block((yyvsp[-2].block));
            if (!add_assignment(ctx, (yyvsp[0].block), lhs, (yyvsp[-1].assign_op), rhs, false))
            {
                destroy_block((yyvsp[0].block));
                YYABORT;
            }
            (yyval.block) = (yyvsp[0].block);
        }
    break;

  case 343: /* assign_op: '='  */
        {
            (yyval.assign_op) = ASSIGN_OP_ASSIGN;
        }
    break;

  case 344: /* assign_op: OP_ADDASSIGN  */
        {
            (yyval.assign_op) = ASSIGN_OP_ADD;
        }
    break;

  case 345: /* assign_op: OP_SUBASSIGN  */
        {
            (yyval.assign_op) = ASSIGN_OP_SUB;
        }
    break;

  case 346: /* assign_op: OP_MULASSIGN  */
        {
            (yyval.assign_op) = ASSIGN_OP_MUL;
        }
    break;

  case 347: /* assign_op: OP_DIVASSIGN  */
        {
            (yyval.assign_op) = ASSIGN_OP_DIV;
        }
    break;

  case 348: /* assign_op: OP_MODASSIGN  */
        {
            (yyval.assign_op) = ASSIGN_OP_MOD;
        }
    break;

  case 349: /* assign_op: OP_LEFTSHIFTASSIGN  */
        {
            (yyval.assign_op) = ASSIGN_OP_LSHIFT;
        }
    break;

  case 350: /* assign_op: OP_RIGHTSHIFTASSIGN  */
        {
            (yyval.assign_op) = ASSIGN_OP_RSHIFT;
        }
    break;

  case 351: /* assign_op: OP_ANDASSIGN  */
        {
            (yyval.assign_op) = ASSIGN_OP_AND;
        }
    break;

  case 352: /* assign_op: OP_ORASSIGN  */
        {
            (yyval.assign_op) = ASSIGN_OP_OR;
        }
    break;

  case 353: /* assign_op: OP_XORASSIGN  */
        {
            (yyval.assign_op) = ASSIGN_OP_XOR;
        }
    break;

  case 355: /* expr: expr ',' assignment_expr  */
        {
            (yyval.block) = (yyvsp[-2].block);
            hlsl_block_add_block((yyval.block), (yyvsp[0].block));
            destroy_block((yyvsp[0].block));
        }
    break;



      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == HLSL_YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (&yylloc, scanner, ctx, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= HLSL_YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == HLSL_YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc, scanner, ctx);
          yychar = HLSL_YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp, scanner, ctx);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, scanner, ctx, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != HLSL_YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, scanner, ctx);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp, scanner, ctx);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

