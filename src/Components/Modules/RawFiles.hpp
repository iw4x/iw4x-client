#pragma once

namespace Components
{
	class RawFiles : public Component
	{
	public:
		RawFiles();

	private:
		static char* ReadRawFile(const char* filename, char* buf, int size);
		static char* GetMenuBuffer(const char* filename);
	};
}
