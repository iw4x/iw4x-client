#include "STDInclude.hpp"

namespace Components
{
	std::string ZoneBuilder::Zone::Build()
	{
		static_assert(sizeof(XFileHeader) == 21, "Invalid XFileHeader structure!");
		static_assert(sizeof(XFile) == 40, "Invalid XFile structure!");

		std::string buffer;

		XFileHeader header = { XFILE_MAGIC_UNSIGNED, XFILE_VERSION, 0, 0, 0 };

		FILETIME fileTime;
		GetSystemTimeAsFileTime(&fileTime);

		header.lowDateTime = fileTime.dwLowDateTime;
		header.highDateTime = fileTime.dwHighDateTime;

		buffer.append((char*)&header, sizeof(header));

		std::string zoneBuffer;
		zoneBuffer.resize(sizeof(XFile));

		// Fill zone
		XAssetList list;
		list.assetCount = 0;
		list.assets = 0;
		list.stringList.count = 0;
		list.stringList.strings = 0;

		zoneBuffer.append((char*)&list, sizeof(list));

		XFile* zone = (XFile*)zoneBuffer.data();
		ZeroMemory(zone, sizeof(XFile));

		zone->size = zoneBuffer.size() - sizeof(XFile);

		auto compressedData = Utils::Compression::ZLib::Compress(zoneBuffer);
		buffer.append(compressedData);

		return buffer;
	}

	ZoneBuilder::Zone::Zone(std::string zoneName) : ZoneName(zoneName)
	{

	}

	ZoneBuilder::Zone::~Zone()
	{

	}

	bool ZoneBuilder::IsEnabled()
	{
		return Flags::HasFlag("zonebuilder");
	}

	ZoneBuilder::ZoneBuilder()
	{
		if (ZoneBuilder::IsEnabled())
		{
			auto data = Zone("").Build();

			FILE* fp;
			fopen_s(&fp, "penis.ff", "wb");

			if (fp)
			{
				fwrite(data.data(), 1, data.size(), fp);
				fclose(fp);
			}

			ExitProcess(0);
		}
	}
}
