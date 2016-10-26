#include "STDInclude.hpp"

namespace Components
{
	std::string ZoneBuilder::TraceZone;
	std::vector<std::pair<Game::XAssetType, std::string>> ZoneBuilder::TraceAssets;

	ZoneBuilder::Zone::Zone(std::string name) : DataMap("zone_source/" + name + ".csv"), ZoneName(name), IndexStart(0), Branding { 0 },

		// Reserve 100MB by default.
		// That's totally fine, as the dedi doesn't load images and therefore doesn't need much memory.
		// That way we can be sure it won't need to reallocate memory.
		// Side note: if you need a fastfile larger than 100MB, you're doing it wrong-
		// Well, decompressed maps can get way larger than 100MB, so let's increase that.
		Buffer(0xC800000)
	{}

	ZoneBuilder::Zone::~Zone()
	{
		// Unload our fastfiles
		Game::XZoneInfo info;
		info.name = nullptr;
		info.allocFlags = 0;
		info.freeFlags = 0x20;

		Game::DB_LoadXAssets(&info, 1, true);

		AssetHandler::ClearTemporaryAssets();
		Localization::ClearTemp();

		ZoneBuilder::Zone::LoadedAssets.clear();
		ZoneBuilder::Zone::ScriptStrings.clear();
		ZoneBuilder::Zone::ScriptStringMap.clear();
	}

	Utils::Stream* ZoneBuilder::Zone::GetBuffer()
	{
		return &Buffer;
	}

	Utils::Memory::Allocator* ZoneBuilder::Zone::GetAllocator()
	{
		return &MemAllocator;
	}

	void ZoneBuilder::Zone::Zone::Build()
	{
		ZoneBuilder::Zone::LoadFastFiles();

		Logger::Print("Linking assets...\n");
		if (!ZoneBuilder::Zone::LoadAssets()) return;

		ZoneBuilder::Zone::AddBranding();

		Logger::Print("Saving...\n");
		ZoneBuilder::Zone::SaveData();

		Logger::Print("Compressing...\n");
		ZoneBuilder::Zone::WriteZone();
	}

	void ZoneBuilder::Zone::LoadFastFiles()
	{
		Logger::Print("Loading required FastFiles...\n");

		for (int i = 0; i < DataMap.GetRows(); ++i)
		{
			if (DataMap.GetElementAt(i, 0) == "require")
			{
				std::string fastfile = DataMap.GetElementAt(i, 1);

				if (!Game::DB_IsZoneLoaded(fastfile.data()))
				{
					Game::XZoneInfo info;
					info.name = fastfile.data();
					info.allocFlags = 0x20;
					info.freeFlags = 0;

					Game::DB_LoadXAssets(&info, 1, true);
				}
				else
				{
					Logger::Print("Zone '%s' already loaded\n", fastfile.data());
				}
			}
		}
	}

	bool ZoneBuilder::Zone::LoadAssets()
	{
		for (int i = 0; i < DataMap.GetRows(); ++i)
		{
			if (DataMap.GetElementAt(i, 0) != "require")
			{
				if (DataMap.GetColumns(i) > 2)
				{
					if (DataMap.GetElementAt(i, 0) == "localize")
					{
						std::string stringOverride = DataMap.GetElementAt(i, 2);
						Utils::String::Replace(stringOverride, "\\n", "\n");

						Localization::SetTemp(DataMap.GetElementAt(i, 1), stringOverride);
					}
					else
					{
						std::string oldName = DataMap.GetElementAt(i, 1);
						std::string newName = DataMap.GetElementAt(i, 2);
						std::string typeName = DataMap.GetElementAt(i, 0).data();
						Game::XAssetType type = Game::DB_GetXAssetNameType(typeName.data());

						if (type < Game::XAssetType::ASSET_TYPE_COUNT && type >= 0)
						{
							ZoneBuilder::Zone::RenameAsset(type, oldName, newName);
						}
						else
						{
							Logger::Error("Unable to rename '%s' to '%s' as the asset type '%s' is invalid!", oldName.data(), newName.data(), typeName.data());
						}
					}
				}

				if (!ZoneBuilder::Zone::LoadAsset(DataMap.GetElementAt(i, 0), DataMap.GetElementAt(i, 1)))
				{
					return false;
				}
			}
		}

		return true;
	}

	bool ZoneBuilder::Zone::LoadAsset(Game::XAssetType type, std::string name)
	{
		return ZoneBuilder::Zone::LoadAsset(Game::DB_GetXAssetTypeName(type), name);
	}

	bool ZoneBuilder::Zone::LoadAsset(std::string typeName, std::string name)
	{
		Game::XAssetType type = Game::DB_GetXAssetNameType(typeName.data());

		if (ZoneBuilder::Zone::FindAsset(type, name) != -1) return true;

		if (type == Game::XAssetType::ASSET_TYPE_INVALID || type >= Game::XAssetType::ASSET_TYPE_COUNT)
		{
			Logger::Error("Error: Invalid asset type '%s'\n", typeName.data());
			return false;
		}

		Game::XAssetHeader assetHeader = AssetHandler::FindAssetForZone(type, name, this);

		if (!assetHeader.data)
		{
			Logger::Error("Error: Missing asset '%s' of type '%s'\n", name.data(), Game::DB_GetXAssetTypeName(type));
			return false;
		}

		Game::XAsset asset;
		asset.type = type;
		asset.header = assetHeader;

		// Handle script strings and referenced assets
		AssetHandler::ZoneMark(asset, this);

		ZoneBuilder::Zone::LoadedAssets.push_back(asset);
		return true;
	}

	int ZoneBuilder::Zone::FindAsset(Game::XAssetType type, std::string name)
	{
		for (unsigned int i = 0; i < ZoneBuilder::Zone::LoadedAssets.size(); ++i)
		{
			Game::XAsset* asset = &ZoneBuilder::Zone::LoadedAssets[i];

			if (asset->type != type) continue;

			const char* assetName = DB_GetXAssetName(asset);

			if (name == assetName)
			{
				return i;
			}
		}

		return -1;
	}

	Game::XAsset* ZoneBuilder::Zone::GetAsset(int index)
	{
		if ((uint32_t)index < ZoneBuilder::Zone::LoadedAssets.size())
		{
			return &ZoneBuilder::Zone::LoadedAssets[index];
		}

		return nullptr;
	}

	uint32_t ZoneBuilder::Zone::GetAssetOffset(int index)
	{
		Utils::Stream::Offset offset;
		offset.block = Game::XFILE_BLOCK_VIRTUAL;
		offset.offset = (ZoneBuilder::Zone::IndexStart + (index * sizeof(Game::XAsset)) + 4);
		return offset.GetPackedOffset();
	}

	Game::XAssetHeader ZoneBuilder::Zone::RequireAsset(Game::XAssetType type, const char* name)
	{
		Game::XAssetHeader header;
		Utils::Stream::ClearPointer(&header.data);

		int assetIndex = ZoneBuilder::Zone::FindAsset(type, name);

		if (assetIndex != -1)
		{
			header.data = reinterpret_cast<void*>(ZoneBuilder::Zone::GetAssetOffset(assetIndex));
		}
		else
		{
			Logger::Error("Missing required asset '%s' (%s). Export failed!", name, Game::DB_GetXAssetTypeName(type));
		}

		return header;
	}

	void ZoneBuilder::Zone::WriteZone()
	{
		FILETIME fileTime;
		GetSystemTimeAsFileTime(&fileTime);

		Game::XFileHeader header = { XFILE_MAGIC_UNSIGNED, XFILE_VERSION_IW4X, Game::XFileLanguage::XLANG_NONE, fileTime.dwHighDateTime, fileTime.dwLowDateTime };

		std::string outBuffer;
		outBuffer.append(reinterpret_cast<char*>(&header), sizeof(header));

		std::string zoneBuffer = ZoneBuilder::Zone::Buffer.ToBuffer();
		zoneBuffer = Utils::Compression::ZLib::Compress(zoneBuffer);
		outBuffer.append(zoneBuffer);

		std::string outFile = "zone/" + ZoneBuilder::Zone::ZoneName + ".ff";
		Utils::IO::WriteFile(outFile, outBuffer);

		Logger::Print("done.\n");
		Logger::Print("Zone '%s' written with %d assets\n", outFile.data(), ZoneBuilder::Zone::LoadedAssets.size());
	}

	void ZoneBuilder::Zone::SaveData()
	{
		// Add header
		Game::ZoneHeader zoneHeader = { 0 };
		zoneHeader.assetList.assetCount = ZoneBuilder::Zone::LoadedAssets.size();
		Utils::Stream::ClearPointer(&zoneHeader.assetList.assets);

		// Increment ScriptStrings count (for empty script string) if available
		if (!ZoneBuilder::Zone::ScriptStrings.empty())
		{
			zoneHeader.assetList.stringList.count = ZoneBuilder::Zone::ScriptStrings.size() + 1;
			Utils::Stream::ClearPointer(&zoneHeader.assetList.stringList.strings);
		}

		// Write header
		ZoneBuilder::Zone::Buffer.Save(&zoneHeader, sizeof(Game::ZoneHeader));
		ZoneBuilder::Zone::Buffer.PushBlock(Game::XFILE_BLOCK_VIRTUAL); // Push main stream onto the stream stack

																   // Write ScriptStrings, if available
		if (!ZoneBuilder::Zone::ScriptStrings.empty())
		{
			ZoneBuilder::Zone::Buffer.SaveNull(4); // Empty script string?
												   // This actually represents a NULL string, but as scriptString. 
												   // So scriptString loading for NULL scriptStrings from fastfile results in a NULL scriptString.
												   // That's the reason why the count is incremented by 1, if scriptStrings are available.

			// Write ScriptString pointer table
			for (size_t i = 0; i < ZoneBuilder::Zone::ScriptStrings.size(); ++i)
			{
				ZoneBuilder::Zone::Buffer.SaveMax(4);
			}

			ZoneBuilder::Zone::Buffer.Align(Utils::Stream::ALIGN_4);

			// Write ScriptStrings
			for (auto ScriptString : ZoneBuilder::Zone::ScriptStrings)
			{
				ZoneBuilder::Zone::Buffer.SaveString(ScriptString.data());
			}
		}

		// Align buffer (4 bytes) to get correct offsets for pointers
		ZoneBuilder::Zone::Buffer.Align(Utils::Stream::ALIGN_4);
		ZoneBuilder::Zone::IndexStart = ZoneBuilder::Zone::Buffer.GetBlockSize(Game::XFILE_BLOCK_VIRTUAL); // Mark AssetTable offset

		// AssetTable
		for (auto asset : ZoneBuilder::Zone::LoadedAssets)
		{
			Game::XAsset entry = { asset.type, 0 };
			Utils::Stream::ClearPointer(&entry.header.data);

			ZoneBuilder::Zone::Buffer.Save(&entry);
		}

		// Assets
		for (auto asset : ZoneBuilder::Zone::LoadedAssets)
		{
			ZoneBuilder::Zone::Buffer.PushBlock(Game::XFILE_BLOCK_TEMP);
			ZoneBuilder::Zone::Buffer.Align(Utils::Stream::ALIGN_4);

#ifdef DEBUG
			Components::Logger::Print("Saving (%s): %s\n", Game::DB_GetXAssetTypeName(asset.type), Game::DB_GetXAssetName(&asset));
#endif

			ZoneBuilder::Zone::Store(asset.header);
			AssetHandler::ZoneSave(asset, this);

			ZoneBuilder::Zone::Buffer.PopBlock();
		}

		// Adapt header
		ZoneBuilder::Zone::Buffer.EnterCriticalSection();
		Game::XFile* header = reinterpret_cast<Game::XFile*>(ZoneBuilder::Zone::Buffer.Data());
		header->size = ZoneBuilder::Zone::Buffer.Length() - sizeof(Game::XFile); // Write correct data size
		header->externalSize = 0; // ? 

		// Write stream sizes
		for (int i = 0; i < Game::MAX_XFILE_COUNT; ++i)
		{
			header->blockSize[i] = ZoneBuilder::Zone::Buffer.GetBlockSize(static_cast<Game::XFILE_BLOCK_TYPES>(i));
		}

		ZoneBuilder::Zone::Buffer.LeaveCriticalSection();
		ZoneBuilder::Zone::Buffer.PopBlock();
	}

	// Add branding asset
	void ZoneBuilder::Zone::AddBranding()
	{
		char* data = "FastFile built using IW4x ZoneTool!";
		ZoneBuilder::Zone::Branding = { ZoneBuilder::Zone::ZoneName.data(), (int)strlen(data), 0, data };

		if (ZoneBuilder::Zone::FindAsset(Game::XAssetType::ASSET_TYPE_RAWFILE, ZoneBuilder::Zone::Branding.name) != -1)
		{
			Logger::Error("Unable to add branding. Asset '%s' already exists!", ZoneBuilder::Zone::Branding.name);
		}

		Game::XAssetHeader header = { &Branding };
		Game::XAsset brandingAsset = { Game::XAssetType::ASSET_TYPE_RAWFILE, header };
		ZoneBuilder::Zone::LoadedAssets.push_back(brandingAsset);
	}

	// Check if the given pointer has already been mapped
	bool ZoneBuilder::Zone::HasPointer(const void* pointer)
	{
		return (ZoneBuilder::Zone::PointerMap.find(pointer) != ZoneBuilder::Zone::PointerMap.end());
	}

	// Get stored offset for given file pointer
	uint32_t ZoneBuilder::Zone::SafeGetPointer(const void* pointer)
	{
		if (ZoneBuilder::Zone::HasPointer(pointer))
		{
			return ZoneBuilder::Zone::PointerMap[pointer];
		}

		return NULL;
	}

	void ZoneBuilder::Zone::StorePointer(const void* pointer)
	{
		ZoneBuilder::Zone::PointerMap[pointer] = ZoneBuilder::Zone::Buffer.GetPackedOffset();
	}

	int ZoneBuilder::Zone::AddScriptString(std::string str)
	{
		return ZoneBuilder::Zone::AddScriptString(Game::SL_GetString(str.data(), 0));
	}

	// Mark a scriptString for writing and map it.
	int ZoneBuilder::Zone::AddScriptString(unsigned short gameIndex)
	{
		// Handle NULL scriptStrings
		// Might optimize that later
		if (!gameIndex)
		{
			if (ZoneBuilder::Zone::ScriptStrings.empty())
			{
				ZoneBuilder::Zone::ScriptStrings.push_back("");
			}

			return 0;
		}

		std::string str = Game::SL_ConvertToString(gameIndex);
		int prev = ZoneBuilder::Zone::FindScriptString(str);

		if (prev > 0)
		{
			ZoneBuilder::Zone::ScriptStringMap[gameIndex] = prev;
			return prev;
		}

		ZoneBuilder::Zone::ScriptStrings.push_back(str);
		ZoneBuilder::Zone::ScriptStringMap[gameIndex] = ZoneBuilder::Zone::ScriptStrings.size();
		return ZoneBuilder::Zone::ScriptStrings.size();
	}

	// Find a local scriptString
	int ZoneBuilder::Zone::FindScriptString(std::string str)
	{
		for (unsigned int i = 0; i < ZoneBuilder::Zone::ScriptStrings.size(); ++i)
		{
			if (ZoneBuilder::Zone::ScriptStrings[i] == str)
			{
				return (i + 1);
			}
		}

		return -1;
	}

	// Remap a scriptString to it's corresponding value in the local scriptString table.
	void ZoneBuilder::Zone::MapScriptString(unsigned short* gameIndex)
	{
		*gameIndex = 0xFFFF & ZoneBuilder::Zone::ScriptStringMap[*gameIndex];
	}

	// Store a new name for a given asset
	void ZoneBuilder::Zone::RenameAsset(Game::XAssetType type, std::string asset, std::string newName)
	{
		if (type < Game::XAssetType::ASSET_TYPE_COUNT && type >= 0)
		{
			ZoneBuilder::Zone::RenameMap[type][asset] = newName;
		}
		else
		{
			Logger::Error("Unable to rename '%s' to '%s' as the asset type is invalid!", asset.data(), newName.data());
		}
	}

	// Return the new name for a given asset
	std::string ZoneBuilder::Zone::GetAssetName(Game::XAssetType type, std::string asset)
	{
		if (type < Game::XAssetType::ASSET_TYPE_COUNT && type >= 0)
		{
			if (ZoneBuilder::Zone::RenameMap[type].find(asset) != ZoneBuilder::Zone::RenameMap[type].end())
			{
				return ZoneBuilder::Zone::RenameMap[type][asset];
			}
		}
		else
		{
			Logger::Error("Unable to get name for '%s' as the asset type is invalid!", asset.data());
		}

		return asset;
	}

	void ZoneBuilder::Zone::Store(Game::XAssetHeader header)
	{
		if (!ZoneBuilder::Zone::HasPointer(header.data)) // We should never have to restore a pointer, so this expression should always resolve into false
		{
			ZoneBuilder::Zone::StorePointer(header.data);
		}
	}

	bool ZoneBuilder::IsEnabled()
	{
		return (Flags::HasFlag("zonebuilder") && !Dedicated::IsEnabled());
	}

	void ZoneBuilder::BeginAssetTrace(std::string zone)
	{
		ZoneBuilder::TraceZone = zone;
	}

	std::vector<std::pair<Game::XAssetType, std::string>> ZoneBuilder::EndAssetTrace()
	{
		ZoneBuilder::TraceZone.clear();

		std::vector<std::pair<Game::XAssetType, std::string>> AssetTrace;
		Utils::Merge(&AssetTrace, ZoneBuilder::TraceAssets);

		ZoneBuilder::TraceAssets.clear();

		return AssetTrace;
	}

	ZoneBuilder::ZoneBuilder()
	{
		Assert_Size(Game::XFileHeader, 21);
		Assert_Size(Game::XFile, 40);
		static_assert(Game::MAX_XFILE_COUNT == 8, "XFile block enum is invalid!");

		ZoneBuilder::EndAssetTrace();
		
		if (ZoneBuilder::IsEnabled())
		{
			// Prevent loading textures (preserves loaddef)
			Utils::Hook::Set<BYTE>(0x51F4E0, 0xC3);

			//r_loadForrenderer = 0 
			Utils::Hook::Set<BYTE>(0x519DDF, 0);

			//r_delayloadimage retn
			Utils::Hook::Set<BYTE>(0x51F450, 0xC3);

			// r_registerDvars hack
			Utils::Hook::Set<BYTE>(0x51B1CD, 0xC3);

			// Prevent destroying textures
			Utils::Hook::Set<BYTE>(0x51F03D, 0xEB);

			// Don't create default assets
			Utils::Hook::Set<BYTE>(0x407BAA, 0xEB);

			// Don't display errors when assets are missing (we might manually build those)
			Utils::Hook::Nop(0x5BB3F2, 5);
			Utils::Hook::Nop(0x5BB422, 5);
			Utils::Hook::Nop(0x5BB43A, 5);

			// Increase asset pools
			Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_MAP_ENTS, 10);
			Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_XMODELSURFS, 8192);
			Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_IMAGE, 14336);

			// hunk size (was 300 MiB)
			Utils::Hook::Set<DWORD>(0x64A029, 0x38400000); // 900 MiB
			Utils::Hook::Set<DWORD>(0x64A057, 0x38400000);

			AssetHandler::OnLoad([](Game::XAssetType type, Game::XAssetHeader /*asset*/, std::string name, bool* /*restrict*/)
			{
				if (!ZoneBuilder::TraceZone.empty() && ZoneBuilder::TraceZone == FastFiles::Current())
				{
					ZoneBuilder::TraceAssets.push_back({ type, name });
				}
			});

			Command::Add("verifyzone", [] (Command::Params params)
			{
				if (params.Length() < 2) return;

				static std::string zone = params[1];

				ZoneBuilder::BeginAssetTrace(zone);

				Game::XZoneInfo info;
				info.name = zone.data();
				info.allocFlags = 0x20;
				info.freeFlags = 0;

				Logger::Print("Loading zone '%s'...\n", zone.data());

				Game::DB_LoadXAssets(&info, 1, true);
				AssetHandler::FindOriginalAsset(Game::XAssetType::ASSET_TYPE_RAWFILE, zone.data()); // Lock until zone is loaded

				auto assets = ZoneBuilder::EndAssetTrace();

				Logger::Print("Unloading zone '%s'...\n", zone.data());
				info.freeFlags = 0x20;
				info.allocFlags = 0;
				info.name = nullptr;

				Game::DB_LoadXAssets(&info, 1, true);
				AssetHandler::FindOriginalAsset(Game::XAssetType::ASSET_TYPE_RAWFILE, "default"); // Lock until zone is unloaded
				
				Logger::Print("Zone '%s' loaded with %d assets:\n", zone.data(), assets.size());
				
				int count = 0;
				for (auto i = assets.begin(); i != assets.end(); ++i, ++count)
				{
					Logger::Print(" %d: %s: %s\n", count, Game::DB_GetXAssetTypeName(i->first), i->second.data());
				}

				Logger::Print("\n");
			});

			Command::Add("buildzone", [] (Command::Params params)
			{
				if (params.Length() < 2) return;

				std::string zoneName = params[1];
				Logger::Print("Building zone '%s'...\n", zoneName.data());

				Zone(zoneName).Build();
			});

			Command::Add("listassets", [] (Command::Params params)
			{
				if (params.Length() < 2) return;
				Game::XAssetType type = Game::DB_GetXAssetNameType(params[1]);

				if (type != Game::XAssetType::ASSET_TYPE_INVALID)
				{
					Game::DB_EnumXAssets(type, [] (Game::XAssetHeader header, void* data)
					{
						Game::XAsset asset = { *reinterpret_cast<Game::XAssetType*>(data), header };
						Logger::Print("%s\n", Game::DB_GetXAssetName(&asset));
					}, &type, false);
				}
			});
		}
	}
}
