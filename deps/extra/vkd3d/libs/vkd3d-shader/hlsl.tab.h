/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_HLSL_YY_HOME_WROY_PROJECTS_IW4X_CLIENT_VK_DEPS_EXTRA_VKD3D_LIBS_VKD3D_SHADER_HLSL_TAB_H_INCLUDED
# define YY_HLSL_YY_HOME_WROY_PROJECTS_IW4X_CLIENT_VK_DEPS_EXTRA_VKD3D_LIBS_VKD3D_SHADER_HLSL_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef HLSL_YYDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define HLSL_YYDEBUG 1
#  else
#   define HLSL_YYDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define HLSL_YYDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined HLSL_YYDEBUG */
#if HLSL_YYDEBUG
extern int hlsl_yydebug;
#endif
/* "%code requires" blocks.  */


#include "hlsl.h"
#include <stdio.h>

#define HLSL_YYLTYPE struct vkd3d_shader_location

struct parse_fields
{
    struct hlsl_struct_field *fields;
    size_t count, capacity;
};

struct parse_initializer
{
    struct hlsl_ir_node **args;
    unsigned int args_count;
    struct hlsl_block *instrs;
    bool braces;
    struct vkd3d_shader_location loc;
};

struct parse_parameter
{
    struct hlsl_type *type;
    const char *name;
    struct hlsl_semantic semantic;
    struct hlsl_reg_reservation reg_reservation;
    uint32_t modifiers;
    struct parse_initializer initializer;
};

struct parse_colon_attributes
{
    struct hlsl_semantic semantic;
    struct hlsl_reg_reservation reg_reservation;
};

struct parse_array_sizes
{
    uint32_t *sizes; /* innermost first */
    unsigned int count;
};

struct parse_variable_def
{
    struct list entry;
    struct vkd3d_shader_location loc;

    char *name;
    struct parse_array_sizes arrays;
    struct hlsl_semantic semantic;
    struct hlsl_reg_reservation reg_reservation;
    struct parse_initializer initializer;
    struct hlsl_scope *annotations;

    struct hlsl_type *basic_type;
    uint32_t modifiers;
    struct vkd3d_shader_location modifiers_loc;

    struct hlsl_state_block **state_blocks;
    unsigned int state_block_count;
    size_t state_block_capacity;
};

struct parse_function
{
    struct hlsl_ir_function_decl *decl;
    struct hlsl_func_parameters parameters;
    struct hlsl_semantic return_semantic;
    bool first;
};

struct parse_if_body
{
    struct hlsl_block *then_block;
    struct hlsl_block *else_block;
};

enum parse_assign_op
{
    ASSIGN_OP_ASSIGN,
    ASSIGN_OP_ADD,
    ASSIGN_OP_SUB,
    ASSIGN_OP_MUL,
    ASSIGN_OP_DIV,
    ASSIGN_OP_MOD,
    ASSIGN_OP_LSHIFT,
    ASSIGN_OP_RSHIFT,
    ASSIGN_OP_AND,
    ASSIGN_OP_OR,
    ASSIGN_OP_XOR,
};

struct parse_attribute_list
{
    unsigned int count;
    struct hlsl_attribute **attrs;
};

struct state_block_index
{
    bool has_index;
    unsigned int index;
};



/* Token kinds.  */
#ifndef HLSL_YYTOKENTYPE
# define HLSL_YYTOKENTYPE
  enum hlsl_yytokentype
  {
    HLSL_YYEMPTY = -2,
    HLSL_YYEOF = 0,                /* "end of file"  */
    HLSL_YYerror = 256,            /* error  */
    HLSL_YYUNDEF = 257,            /* "invalid token"  */
    KW_BLENDSTATE = 258,           /* KW_BLENDSTATE  */
    KW_BREAK = 259,                /* KW_BREAK  */
    KW_BUFFER = 260,               /* KW_BUFFER  */
    KW_BYTEADDRESSBUFFER = 261,    /* KW_BYTEADDRESSBUFFER  */
    KW_CASE = 262,                 /* KW_CASE  */
    KW_CONSTANTBUFFER = 263,       /* KW_CONSTANTBUFFER  */
    KW_CBUFFER = 264,              /* KW_CBUFFER  */
    KW_CENTROID = 265,             /* KW_CENTROID  */
    KW_COLUMN_MAJOR = 266,         /* KW_COLUMN_MAJOR  */
    KW_COMPILE = 267,              /* KW_COMPILE  */
    KW_COMPILESHADER = 268,        /* KW_COMPILESHADER  */
    KW_COMPUTESHADER = 269,        /* KW_COMPUTESHADER  */
    KW_CONST = 270,                /* KW_CONST  */
    KW_CONTINUE = 271,             /* KW_CONTINUE  */
    KW_DEFAULT = 272,              /* KW_DEFAULT  */
    KW_DEPTHSTENCILSTATE = 273,    /* KW_DEPTHSTENCILSTATE  */
    KW_DEPTHSTENCILVIEW = 274,     /* KW_DEPTHSTENCILVIEW  */
    KW_DISCARD = 275,              /* KW_DISCARD  */
    KW_DO = 276,                   /* KW_DO  */
    KW_DOMAINSHADER = 277,         /* KW_DOMAINSHADER  */
    KW_ELSE = 278,                 /* KW_ELSE  */
    KW_EXPORT = 279,               /* KW_EXPORT  */
    KW_EXTERN = 280,               /* KW_EXTERN  */
    KW_FALSE = 281,                /* KW_FALSE  */
    KW_FOR = 282,                  /* KW_FOR  */
    KW_FXGROUP = 283,              /* KW_FXGROUP  */
    KW_GEOMETRYSHADER = 284,       /* KW_GEOMETRYSHADER  */
    KW_GROUPSHARED = 285,          /* KW_GROUPSHARED  */
    KW_HULLSHADER = 286,           /* KW_HULLSHADER  */
    KW_IF = 287,                   /* KW_IF  */
    KW_IN = 288,                   /* KW_IN  */
    KW_INLINE = 289,               /* KW_INLINE  */
    KW_INOUT = 290,                /* KW_INOUT  */
    KW_INPUTPATCH = 291,           /* KW_INPUTPATCH  */
    KW_LINE = 292,                 /* KW_LINE  */
    KW_LINEADJ = 293,              /* KW_LINEADJ  */
    KW_LINEAR = 294,               /* KW_LINEAR  */
    KW_LINESTREAM = 295,           /* KW_LINESTREAM  */
    KW_MATRIX = 296,               /* KW_MATRIX  */
    KW_NAMESPACE = 297,            /* KW_NAMESPACE  */
    KW_NOINTERPOLATION = 298,      /* KW_NOINTERPOLATION  */
    KW_NOPERSPECTIVE = 299,        /* KW_NOPERSPECTIVE  */
    KW_NULL = 300,                 /* KW_NULL  */
    KW_OUT = 301,                  /* KW_OUT  */
    KW_OUTPUTPATCH = 302,          /* KW_OUTPUTPATCH  */
    KW_PACKOFFSET = 303,           /* KW_PACKOFFSET  */
    KW_PASS = 304,                 /* KW_PASS  */
    KW_PIXELSHADER = 305,          /* KW_PIXELSHADER  */
    KW_POINT = 306,                /* KW_POINT  */
    KW_POINTSTREAM = 307,          /* KW_POINTSTREAM  */
    KW_RASTERIZERORDEREDBUFFER = 308, /* KW_RASTERIZERORDEREDBUFFER  */
    KW_RASTERIZERORDEREDSTRUCTUREDBUFFER = 309, /* KW_RASTERIZERORDEREDSTRUCTUREDBUFFER  */
    KW_RASTERIZERORDEREDTEXTURE1D = 310, /* KW_RASTERIZERORDEREDTEXTURE1D  */
    KW_RASTERIZERORDEREDTEXTURE1DARRAY = 311, /* KW_RASTERIZERORDEREDTEXTURE1DARRAY  */
    KW_RASTERIZERORDEREDTEXTURE2D = 312, /* KW_RASTERIZERORDEREDTEXTURE2D  */
    KW_RASTERIZERORDEREDTEXTURE2DARRAY = 313, /* KW_RASTERIZERORDEREDTEXTURE2DARRAY  */
    KW_RASTERIZERORDEREDTEXTURE3D = 314, /* KW_RASTERIZERORDEREDTEXTURE3D  */
    KW_RASTERIZERSTATE = 315,      /* KW_RASTERIZERSTATE  */
    KW_RENDERTARGETVIEW = 316,     /* KW_RENDERTARGETVIEW  */
    KW_RETURN = 317,               /* KW_RETURN  */
    KW_REGISTER = 318,             /* KW_REGISTER  */
    KW_ROW_MAJOR = 319,            /* KW_ROW_MAJOR  */
    KW_RWBUFFER = 320,             /* KW_RWBUFFER  */
    KW_RWBYTEADDRESSBUFFER = 321,  /* KW_RWBYTEADDRESSBUFFER  */
    KW_RWSTRUCTUREDBUFFER = 322,   /* KW_RWSTRUCTUREDBUFFER  */
    KW_RWTEXTURE1D = 323,          /* KW_RWTEXTURE1D  */
    KW_RWTEXTURE1DARRAY = 324,     /* KW_RWTEXTURE1DARRAY  */
    KW_RWTEXTURE2D = 325,          /* KW_RWTEXTURE2D  */
    KW_RWTEXTURE2DARRAY = 326,     /* KW_RWTEXTURE2DARRAY  */
    KW_RWTEXTURE3D = 327,          /* KW_RWTEXTURE3D  */
    KW_SAMPLER = 328,              /* KW_SAMPLER  */
    KW_SAMPLER1D = 329,            /* KW_SAMPLER1D  */
    KW_SAMPLER2D = 330,            /* KW_SAMPLER2D  */
    KW_SAMPLER3D = 331,            /* KW_SAMPLER3D  */
    KW_SAMPLERCUBE = 332,          /* KW_SAMPLERCUBE  */
    KW_SAMPLER_STATE = 333,        /* KW_SAMPLER_STATE  */
    KW_SAMPLERCOMPARISONSTATE = 334, /* KW_SAMPLERCOMPARISONSTATE  */
    KW_SHARED = 335,               /* KW_SHARED  */
    KW_SNORM = 336,                /* KW_SNORM  */
    KW_STATEBLOCK = 337,           /* KW_STATEBLOCK  */
    KW_STATEBLOCK_STATE = 338,     /* KW_STATEBLOCK_STATE  */
    KW_STATIC = 339,               /* KW_STATIC  */
    KW_STRING = 340,               /* KW_STRING  */
    KW_STRUCT = 341,               /* KW_STRUCT  */
    KW_STRUCTUREDBUFFER = 342,     /* KW_STRUCTUREDBUFFER  */
    KW_SWITCH = 343,               /* KW_SWITCH  */
    KW_TBUFFER = 344,              /* KW_TBUFFER  */
    KW_TECHNIQUE = 345,            /* KW_TECHNIQUE  */
    KW_TECHNIQUE10 = 346,          /* KW_TECHNIQUE10  */
    KW_TECHNIQUE11 = 347,          /* KW_TECHNIQUE11  */
    KW_TEXTURE = 348,              /* KW_TEXTURE  */
    KW_TEXTURE1D = 349,            /* KW_TEXTURE1D  */
    KW_TEXTURE1DARRAY = 350,       /* KW_TEXTURE1DARRAY  */
    KW_TEXTURE2D = 351,            /* KW_TEXTURE2D  */
    KW_TEXTURE2DARRAY = 352,       /* KW_TEXTURE2DARRAY  */
    KW_TEXTURE2DMS = 353,          /* KW_TEXTURE2DMS  */
    KW_TEXTURE2DMSARRAY = 354,     /* KW_TEXTURE2DMSARRAY  */
    KW_TEXTURE3D = 355,            /* KW_TEXTURE3D  */
    KW_TEXTURECUBE = 356,          /* KW_TEXTURECUBE  */
    KW_TEXTURECUBEARRAY = 357,     /* KW_TEXTURECUBEARRAY  */
    KW_TRIANGLE = 358,             /* KW_TRIANGLE  */
    KW_TRIANGLEADJ = 359,          /* KW_TRIANGLEADJ  */
    KW_TRIANGLESTREAM = 360,       /* KW_TRIANGLESTREAM  */
    KW_TRUE = 361,                 /* KW_TRUE  */
    KW_TYPEDEF = 362,              /* KW_TYPEDEF  */
    KW_UNSIGNED = 363,             /* KW_UNSIGNED  */
    KW_UNIFORM = 364,              /* KW_UNIFORM  */
    KW_UNORM = 365,                /* KW_UNORM  */
    KW_VECTOR = 366,               /* KW_VECTOR  */
    KW_VERTEXSHADER = 367,         /* KW_VERTEXSHADER  */
    KW_VOID = 368,                 /* KW_VOID  */
    KW_VOLATILE = 369,             /* KW_VOLATILE  */
    KW_WHILE = 370,                /* KW_WHILE  */
    OP_INC = 371,                  /* OP_INC  */
    OP_DEC = 372,                  /* OP_DEC  */
    OP_AND = 373,                  /* OP_AND  */
    OP_OR = 374,                   /* OP_OR  */
    OP_EQ = 375,                   /* OP_EQ  */
    OP_LEFTSHIFT = 376,            /* OP_LEFTSHIFT  */
    OP_LEFTSHIFTASSIGN = 377,      /* OP_LEFTSHIFTASSIGN  */
    OP_RIGHTSHIFT = 378,           /* OP_RIGHTSHIFT  */
    OP_RIGHTSHIFTASSIGN = 379,     /* OP_RIGHTSHIFTASSIGN  */
    OP_LE = 380,                   /* OP_LE  */
    OP_GE = 381,                   /* OP_GE  */
    OP_NE = 382,                   /* OP_NE  */
    OP_ADDASSIGN = 383,            /* OP_ADDASSIGN  */
    OP_SUBASSIGN = 384,            /* OP_SUBASSIGN  */
    OP_MULASSIGN = 385,            /* OP_MULASSIGN  */
    OP_DIVASSIGN = 386,            /* OP_DIVASSIGN  */
    OP_MODASSIGN = 387,            /* OP_MODASSIGN  */
    OP_ANDASSIGN = 388,            /* OP_ANDASSIGN  */
    OP_ORASSIGN = 389,             /* OP_ORASSIGN  */
    OP_XORASSIGN = 390,            /* OP_XORASSIGN  */
    C_FLOAT = 391,                 /* C_FLOAT  */
    C_INTEGER = 392,               /* C_INTEGER  */
    C_UNSIGNED = 393,              /* C_UNSIGNED  */
    PRE_LINE = 394,                /* PRE_LINE  */
    VAR_IDENTIFIER = 395,          /* VAR_IDENTIFIER  */
    NEW_IDENTIFIER = 396,          /* NEW_IDENTIFIER  */
    NAMESPACE_IDENTIFIER = 397,    /* NAMESPACE_IDENTIFIER  */
    STRING = 398,                  /* STRING  */
    TYPE_IDENTIFIER = 399          /* TYPE_IDENTIFIER  */
  };
  typedef enum hlsl_yytokentype hlsl_yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined HLSL_YYSTYPE && ! defined HLSL_YYSTYPE_IS_DECLARED
union HLSL_YYSTYPE
{

    struct hlsl_type *type;
    INT intval;
    FLOAT floatval;
    bool boolval;
    char *name;
    struct hlsl_identifier identifier;
    uint32_t modifiers;
    struct hlsl_block *block;
    struct list *list;
    struct parse_fields fields;
    struct parse_function function;
    struct parse_parameter parameter;
    struct hlsl_func_parameters parameters;
    struct parse_initializer initializer;
    struct parse_array_sizes arrays;
    struct parse_variable_def *variable_def;
    struct parse_if_body if_body;
    enum parse_assign_op assign_op;
    struct hlsl_reg_reservation reg_reservation;
    struct parse_colon_attributes colon_attributes;
    struct hlsl_semantic semantic;
    enum hlsl_buffer_type buffer_type;
    enum hlsl_sampler_dim sampler_dim;
    enum hlsl_so_object_type so_type;
    enum hlsl_array_type patch_type;
    struct hlsl_attribute *attr;
    struct parse_attribute_list attr_list;
    struct hlsl_ir_switch_case *switch_case;
    struct hlsl_scope *scope;
    struct hlsl_state_block *state_block;
    struct state_block_index state_block_index;


};
typedef union HLSL_YYSTYPE HLSL_YYSTYPE;
# define HLSL_YYSTYPE_IS_TRIVIAL 1
# define HLSL_YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined HLSL_YYLTYPE && ! defined HLSL_YYLTYPE_IS_DECLARED
typedef struct HLSL_YYLTYPE HLSL_YYLTYPE;
struct HLSL_YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define HLSL_YYLTYPE_IS_DECLARED 1
# define HLSL_YYLTYPE_IS_TRIVIAL 1
#endif




int hlsl_yyparse (void *scanner, struct hlsl_ctx *ctx);

/* "%code provides" blocks.  */


int yylex(HLSL_YYSTYPE *yylval_param, HLSL_YYLTYPE *yylloc_param, void *yyscanner);



#endif /* !YY_HLSL_YY_HOME_WROY_PROJECTS_IW4X_CLIENT_VK_DEPS_EXTRA_VKD3D_LIBS_VKD3D_SHADER_HLSL_TAB_H_INCLUDED  */
