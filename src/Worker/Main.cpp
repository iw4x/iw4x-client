#include "STDInclude.hpp"
#include <conio.h>

int main(int /*argc*/, char /*argv*/[])
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CONOUT$", "w", stdout);

	printf("Hi");
	_getch();


	return 0;
}
