#pragma once

namespace Components
{
	class RawFiles : public Component
	{
	public:
		RawFiles();

		static void* RawFiles::LoadModdableRawfileFunc(const char* filename);
	};
}
