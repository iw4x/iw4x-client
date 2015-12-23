#include "..\STDInclude.hpp"

namespace Components
{
	Menus::Menus()
	{
		Command::Add("openmenu", [] (Command::Params params)
		{
			if (params.Length() != 2)
			{
				Logger::Print("USAGE: openmenu <menu name>\n");
				return;
			}

			char* menu = params[1];

			__asm
			{
				push menu
				push 62E2858h
				mov eax, 4CCE60h
				call eax
			}
		});
	}
}
