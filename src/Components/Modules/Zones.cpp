#include "STDInclude.hpp"

namespace Components
{
	int Zones::ZoneVersion;

	void Zones::InstallPatches(int version)
	{
		Zones::ZoneVersion = version;
	}

	Zones::Zones()
	{

	}

	Zones::~Zones()
	{

	}
}
