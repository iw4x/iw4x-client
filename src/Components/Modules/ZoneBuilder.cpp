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

		XFile file;
		ZeroMemory(&file, sizeof(file));
		zoneBuffer.append((char*)&file, sizeof(file));

		// Fill zone
		XAssetList list;
		list.assetCount = 0;
		list.assets = 0;
		list.stringList.count = 0;
		list.stringList.strings = 0;

		list.assetCount = 2;

		list.assets = (Game::XAsset*)0xFFFFFFFF;

		zoneBuffer.append((char*)&list, sizeof(list));

		// Crappy assetlist entry
		DWORD type = Game::XAssetType::ASSET_TYPE_LOCALIZE;
		zoneBuffer.append((char*)&type, sizeof(type));
		zoneBuffer.append((char*)&list.assets, sizeof(list.assets)); // Hue

		type = Game::XAssetType::ASSET_TYPE_RAWFILE;
		zoneBuffer.append((char*)&type, sizeof(type));
		zoneBuffer.append((char*)&list.assets, sizeof(list.assets)); // Hue

		// Localized entry
		zoneBuffer.append((char*)&list.assets, sizeof(list.assets)); // Hue
		zoneBuffer.append((char*)&list.assets, sizeof(list.assets)); // Hue
		zoneBuffer.append("MENU_PENIS");
		zoneBuffer.append("\0", 1);

		zoneBuffer.append("PENIS");
		zoneBuffer.append("\0", 1);

		// Obligatory rawfile
		struct Rawfile
		{
			const char* name;
			int sizeCompressed;
			int sizeUnCompressed;
			char * compressedData;
		};

		char* _data = "map mp_rust";

		Rawfile data;
		data.name = (char*)0xFFFFFFFF;
		data.compressedData = (char*)0xFFFFFFFF;
		data.sizeUnCompressed = 0;
		data.sizeCompressed = strlen(_data) + 1;

		zoneBuffer.append((char*)&data, sizeof(data));

		zoneBuffer.append("zob.cfg");
		zoneBuffer.append("\0", 1);

		zoneBuffer.append(_data);
		zoneBuffer.append("\0", 1);

		XFile* zone = (XFile*)zoneBuffer.data();
		ZeroMemory(zone, sizeof(XFile));

		zone->size = zoneBuffer.size() - 40;

		zone->blockSize[3] = zoneBuffer.size() * 2;
		zone->blockSize[0] = zoneBuffer.size() * 2;

// 		for (int i = 0; i < 8; i++)
// 		{
// 			zone->blockSize[i] = zoneBuffer.size() * 2;
// 		}

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

	void TestZoneLoading(Game::XZoneInfo *zoneInfo, unsigned int zoneCount, int sync)
	{
		std::vector<Game::XZoneInfo> data;
		Utils::Merge(data, zoneInfo, zoneCount);

		data.push_back({ "penis", zoneInfo->allocFlags, zoneInfo->freeFlags });

		Game::DB_LoadXAssets(data.data(), data.size(), sync);
	}

	ZoneBuilder::ZoneBuilder()
	{
		if (ZoneBuilder::IsEnabled())
		{
			auto data = Zone("penis").Build();

			FILE* fp;
			fopen_s(&fp, "zone/patch/penis.ff", "wb");

			if (fp)
			{
				fwrite(data.data(), 1, data.size(), fp);
				fclose(fp);
			}

			ExitProcess(0);
		}

		//Utils::Hook(0x60B4AC, TestZoneLoading, HOOK_CALL).Install()->Quick();
	}
}
