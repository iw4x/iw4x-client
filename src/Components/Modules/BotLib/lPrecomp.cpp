#include <STDInclude.hpp>
#include "lPrecomp.hpp"

namespace Components::BotLib
{
	// Two new directives! Refer to (https://en.cppreference.com/w/cpp/preprocessor/conditional)
	Game::directive_s lPrecomp::directives[] =
	{
		{"if", Game::PC_Directive_if},
		{"ifdef", Game::PC_Directive_ifdef},
		{"ifndef", Game::PC_Directive_ifndef},
		{"elifdef", lPrecomp::PC_Directive_elifdef},
		{"elifndef", lPrecomp::PC_Directive_elifndef},
		{"elif", Game::PC_Directive_elif},
		{"else", Game::PC_Directive_else},
		{"endif", Game::PC_Directive_endif},
		{"include", Game::PC_Directive_include},
		{"define", Game::PC_Directive_define},
		{"undef", Game::PC_Directive_undef},
		{"line", Game::PC_Directive_line},
		{"error", Game::PC_Directive_error},
		{"pragma", Game::PC_Directive_pragma},
		{"eval", Game::PC_Directive_eval},
		{"evalfloat", Game::PC_Directive_evalfloat},
		{nullptr, nullptr}
	};

	int lPrecomp::PC_Directive_elif_def(Game::source_s* source, int* value, int type)
	{
		Game::token_s token;
		int skip;

		if (!Game::PC_ReadLine(source, &token, false))
		{
			Game::SourceError(source, "#elifdef without name");
			return false;
		}

		if (token.type != TT_NAME)
		{
			Game::PC_UnreadSourceToken(source, &token);
			Game::SourceError(source, "expected name after #elifdef, found %s", token.string);
			return false;
		}

		auto* d = Game::PC_FindHashedDefine(source->definehash, token.string);
		*value = skip = (type == INDENT_IFDEF) == (d == nullptr);
		return true;
	}

	// #elifdef identifier is essentially equivalent to #elif defined identifier
	int lPrecomp::PC_Directive_elifdef(Game::source_s* source)
	{
		int type;
		int skip;

		Game::PC_PopIndent(source, &type, &skip);
		if (!type || type == INDENT_ELSE)
		{
			Game::SourceError(source, "misplaced #elifdef");
			return false;
		}

		int value;
		if (PC_Directive_elif_def(source, &value, INDENT_IFDEF))
		{
			if (skip == Game::SKIP_YES)
			{
				skip = value;
			}
			else
			{
				skip = Game::SKIP_ALL_ELIFS;
			}

			Game::PC_PushIndent(source, INDENT_ELIF, skip);
			return true;
		}

		return false;
	}

	// #elifndef identifier is essentially equivalent to #elif !defined identifier
	int lPrecomp::PC_Directive_elifndef(Game::source_s* source)
	{
		int type;
		int skip;

		Game::PC_PopIndent(source, &type, &skip);
		if (!type || type == INDENT_ELSE)
		{
			Game::SourceError(source, "misplaced #elifndef");
			return false;
		}

		int value;
		if (PC_Directive_elif_def(source, &value, INDENT_IFNDEF))
		{
			if (skip == Game::SKIP_YES)
			{
				skip = value;
			}
			else
			{
				skip = Game::SKIP_ALL_ELIFS;
			}

			Game::PC_PushIndent(source, INDENT_ELIF, skip);
			return true;
		}

		return false;
	}

	int lPrecomp::PC_ReadDirective(Game::source_s* source)
	{
		Game::token_s token;

		// Read the directive name
		if (!Game::PC_ReadSourceToken(source, &token))
		{
			Game::SourceError(source, "found # without name");
			return false;
		}

		// Directive name must be on the same line
		if (token.linescrossed > 0)
		{
			Game::PC_UnreadSourceToken(source, &token);
			Game::SourceError(source, "found # at end of line");
			return false;
		}

		// If it is a name
		if (token.type == TT_NAME)
		{
			// Find the precompiler directive
			for (auto i = 0; directives[i].name; ++i)
			{
				if (!std::strcmp(directives[i].name, token.string))
				{
					return directives[i].func(source);
				}
			}
		}

		Game::SourceError(source, "unknown precompiler directive %s", token.string);
		return false;
	}

	lPrecomp::lPrecomp()
	{
		Utils::Hook(0x4ACD19, PC_ReadDirective, HOOK_CALL).install()->quick();
	}
}
