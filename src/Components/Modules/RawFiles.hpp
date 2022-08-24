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
	};
}
