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

#ifndef YY_PREPROC_YY_HOME_WROY_PROJECTS_IW4X_CLIENT_VK_DEPS_EXTRA_VKD3D_LIBS_VKD3D_SHADER_PREPROC_TAB_H_INCLUDED
# define YY_PREPROC_YY_HOME_WROY_PROJECTS_IW4X_CLIENT_VK_DEPS_EXTRA_VKD3D_LIBS_VKD3D_SHADER_PREPROC_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef PREPROC_YYDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define PREPROC_YYDEBUG 1
#  else
#   define PREPROC_YYDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define PREPROC_YYDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined PREPROC_YYDEBUG */
#if PREPROC_YYDEBUG
extern int preproc_yydebug;
#endif
/* "%code requires" blocks.  */


#include "vkd3d_shader_private.h"
#include "preproc.h"
#include <stdio.h>

#define PREPROC_YYLTYPE struct vkd3d_shader_location

struct parse_arg_names
{
    char **args;
    size_t count;
};



/* Token kinds.  */
#ifndef PREPROC_YYTOKENTYPE
# define PREPROC_YYTOKENTYPE
  enum preproc_yytokentype
  {
    PREPROC_YYEMPTY = -2,
    PREPROC_YYEOF = 0,             /* "end of file"  */
    PREPROC_YYerror = 256,         /* error  */
    PREPROC_YYUNDEF = 257,         /* "invalid token"  */
    T_HASHSTRING = 258,            /* T_HASHSTRING  */
    T_IDENTIFIER = 259,            /* T_IDENTIFIER  */
    T_IDENTIFIER_PAREN = 260,      /* T_IDENTIFIER_PAREN  */
    T_INTEGER = 261,               /* T_INTEGER  */
    T_STRING = 262,                /* T_STRING  */
    T_TEXT = 263,                  /* T_TEXT  */
    T_NEWLINE = 264,               /* T_NEWLINE  */
    T_DEFINE = 265,                /* "#define"  */
    T_ERROR = 266,                 /* "#error"  */
    T_ELIF = 267,                  /* "#elif"  */
    T_ELSE = 268,                  /* "#else"  */
    T_ENDIF = 269,                 /* "#endif"  */
    T_IF = 270,                    /* "#if"  */
    T_IFDEF = 271,                 /* "#ifdef"  */
    T_IFNDEF = 272,                /* "#ifndef"  */
    T_INCLUDE = 273,               /* "#include"  */
    T_LINE = 274,                  /* "#line"  */
    T_PRAGMA = 275,                /* "#pragma"  */
    T_UNDEF = 276,                 /* "#undef"  */
    T_CONCAT = 277,                /* "##"  */
    T_LE = 278,                    /* "<="  */
    T_GE = 279,                    /* ">="  */
    T_EQ = 280,                    /* "=="  */
    T_NE = 281,                    /* "!="  */
    T_AND = 282,                   /* "&&"  */
    T_OR = 283,                    /* "||"  */
    T_DEFINED = 284                /* "defined"  */
  };
  typedef enum preproc_yytokentype preproc_yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined PREPROC_YYSTYPE && ! defined PREPROC_YYSTYPE_IS_DECLARED
union PREPROC_YYSTYPE
{

    char *string;
    const char *const_string;
    uint32_t integer;
    struct vkd3d_string_buffer string_buffer;
    struct parse_arg_names arg_names;


};
typedef union PREPROC_YYSTYPE PREPROC_YYSTYPE;
# define PREPROC_YYSTYPE_IS_TRIVIAL 1
# define PREPROC_YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined PREPROC_YYLTYPE && ! defined PREPROC_YYLTYPE_IS_DECLARED
typedef struct PREPROC_YYLTYPE PREPROC_YYLTYPE;
struct PREPROC_YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define PREPROC_YYLTYPE_IS_DECLARED 1
# define PREPROC_YYLTYPE_IS_TRIVIAL 1
#endif




int preproc_yyparse (void *scanner, struct preproc_ctx *ctx);

/* "%code provides" blocks.  */


int preproc_yylex(PREPROC_YYSTYPE *yylval_param, PREPROC_YYLTYPE *yylloc_param, void *scanner);



#endif /* !YY_PREPROC_YY_HOME_WROY_PROJECTS_IW4X_CLIENT_VK_DEPS_EXTRA_VKD3D_LIBS_VKD3D_SHADER_PREPROC_TAB_H_INCLUDED  */
