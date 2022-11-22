#pragma once

namespace Components
{
	class RawFiles : public Component
	{
	public:
		RawFiles();

		static char* ReadRawFile(const char* filename, char* buf, int size);

	private:
		static char* GetMenuBuffer(const char* filename);
		static char* Com_LoadInfoString_LoadObj(const char* fileName, const char* fileDesc, const char* ident, char* loadBuffer);
		static const char* Com_LoadInfoString_Hk(const char* fileName, const char* fileDesc, const char* ident, char* loadBuffer);
	};
}
