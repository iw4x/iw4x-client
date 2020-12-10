#pragma once

namespace Components
{
	class RawFiles : public Component
	{
	public:
		RawFiles();

		static void* LoadModdableRawfileFunc(const char* filename);
	};
}
