#pragma once

namespace Components::BotLib
{
	class lPrecomp : public Component
	{
	public:
		lPrecomp();

	private:
		enum
		{
			INDENT_IF = 1,
			INDENT_ELSE = 2,
			INDENT_ELIF = 4,
			INDENT_IFDEF = 8,
			INDENT_IFNDEF = 10,
		};

		enum
		{
			TT_STRING = 1,
			TT_LITERAL = 2,
			TT_NUMBER = 3,
			TT_NAME = 4,
			TT_PUNCTUATION = 5,
			TT_DECIMAL = 8,
			TT_HEX = 0x100,
			TT_OCTAL = 0x200,
			TT_BINARY = 0x400,
			TT_FLOAT = 0x800,
			TT_INTEGER = 0x1000,
			TT_LONG = 0x2000,
			TT_UNSIGNED = 0x4000,
		};

		static Game::directive_s directives[];

		static int PC_Directive_elif_def(Game::source_s* source, int* value, int type);

		static int PC_Directive_elifdef(Game::source_s* source);
		static int PC_Directive_elifndef(Game::source_s* source);

		static int PC_ReadDirective(Game::source_s* source);
	};
}
