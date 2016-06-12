#include "STDInclude.hpp"

// Rename sections
#pragma comment(linker, "/merge:.text=.fucker")
#pragma comment(linker, "/merge:.data=.bastard")
#pragma comment(linker, "/merge:.rdata=.faggot")
#pragma comment(linker, "/merge:.tls=.asshole")
#pragma comment(linker, "/merge:.gfids=.idiot")

// Do necessary assertions here
// Some compilers treat them differently which causes a size mismatch

// WinAPI types
Assert_Size(DWORD, 4);
Assert_Size(WORD, 2);
Assert_Size(BYTE, 1);

// 64 bit integers
Assert_Size(__int64, 8);
Assert_Size(unsigned __int64, 8);
Assert_Size(long long, 8);
Assert_Size(unsigned long long, 8);
Assert_Size(int64_t, 8);
Assert_Size(uint64_t, 8);

// 32 bit integers
Assert_Size(__int32, 4);
Assert_Size(unsigned __int32, 4);
Assert_Size(int, 4);
Assert_Size(unsigned int, 4);
Assert_Size(int32_t, 4);
Assert_Size(uint32_t, 4);

// 16 bit integers
Assert_Size(__int16, 2);
Assert_Size(unsigned __int16, 2);
Assert_Size(short, 2);
Assert_Size(unsigned short, 2);
Assert_Size(int16_t, 2);
Assert_Size(uint16_t, 2);

// 8 bit integers
Assert_Size(__int8, 1);
Assert_Size(unsigned __int8, 1);
Assert_Size(char, 1);
Assert_Size(unsigned char, 1);
Assert_Size(int8_t, 1);
Assert_Size(uint8_t, 1);
Assert_Size(bool, 1);

// Ensure pointers are 4 bytes in size (32-bit)
static_assert(sizeof(intptr_t) == 4 && sizeof(void*) == 4 && sizeof(size_t) == 4, "This doesn't seem to be a 32-bit environment!");

// Disable telemetry data logging
extern "C"
{
	void _cdecl __vcrt_initialize_telemetry_provider() {}
	void _cdecl __telemetry_main_invoke_trigger() {}
	void _cdecl __telemetry_main_return_trigger() {}
	void _cdecl __vcrt_uninitialize_telemetry_provider() {}
};
