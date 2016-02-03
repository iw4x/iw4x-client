#include "STDInclude.hpp"

namespace Components
{

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
		info.freeFlags = 0x01000000;

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

				//Logger::Print("Loading '%s'...\n", fastfile.c_str());

				//if (!DB_IsZoneLoaded(fastfile.c_str()))
				{
					Game::XZoneInfo info;
					info.name = fastfile.data();
					info.allocFlags = 0x01000000;
					info.freeFlags = 0;

					Game::DB_LoadXAssets(&info, 1, true);

					//LoadFastFile(fastfile.c_str(), true);
				}
// 				else
// 				{
// 					Logger::Print("Zone '%s' already loaded\n", fastfile.c_str());
// 				}
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
						Utils::Replace(stringOverride, "\\n", "\n");

						Localization::SetTemp(DataMap.GetElementAt(i, 1), stringOverride);
					}
					else
					{
						ZoneBuilder::Zone::RenameAsset(Game::DB_GetXAssetNameType(DataMap.GetElementAt(i, 0).data()), DataMap.GetElementAt(i, 1), DataMap.GetElementAt(i, 2));
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
			Logger::Print("Error: Invalid asset type '%s'\n", typeName.data());
			return false;
		}

		Game::XAssetHeader assetHeader = AssetHandler::FindAssetForZone(type, name, this);

		if (!assetHeader.data)
		{
			Logger::Print("Error: Missing asset '%s' of type '%s'\n", name.data(), Game::DB_GetXAssetTypeName(type));
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
		header.data = reinterpret_cast<void*>(-1);

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

		Game::XFileHeader header = { XFILE_MAGIC_UNSIGNED, XFILE_VERSION, Game::XFileLanguage::XLANG_NONE, fileTime.dwHighDateTime,  fileTime.dwLowDateTime };

		std::string outBuffer;
		outBuffer.append(reinterpret_cast<char*>(&header), sizeof(header));

		std::string zoneBuffer = ZoneBuilder::Zone::Buffer.ToBuffer();
		zoneBuffer = Utils::Compression::ZLib::Compress(zoneBuffer);
		outBuffer.append(zoneBuffer);

		std::string outFile = "zone/" + ZoneBuilder::Zone::ZoneName + ".ff";
		Utils::WriteFile(outFile, outBuffer);

		Logger::Print("done.\n");
		Logger::Print("Zone '%s' written with %d assets\n", outFile.c_str(), ZoneBuilder::Zone::LoadedAssets.size());
	}

	void ZoneBuilder::Zone::SaveData()
	{
		// Add header
		Game::ZoneHeader zoneHeader = { 0 };
		zoneHeader.assetList.assetCount = ZoneBuilder::Zone::LoadedAssets.size();
		zoneHeader.assetList.assets = reinterpret_cast<Game::XAsset*>(-1);

		// Increment ScriptStrings count (for empty script string) if available
		if (ZoneBuilder::Zone::ScriptStrings.size())
		{
			zoneHeader.assetList.stringList.count = ZoneBuilder::Zone::ScriptStrings.size() + 1;
			zoneHeader.assetList.stringList.strings = reinterpret_cast<const char**>(-1);
		}

		// Write header
		ZoneBuilder::Zone::Buffer.Save(&zoneHeader, sizeof(Game::ZoneHeader));
		ZoneBuilder::Zone::Buffer.PushBlock(Game::XFILE_BLOCK_VIRTUAL); // Push main stream onto the stream stack

																   // Write ScriptStrings, if available
		if (ZoneBuilder::Zone::ScriptStrings.size())
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
				ZoneBuilder::Zone::Buffer.SaveString(ScriptString.c_str());
			}
		}

		// Align buffer (4 bytes) to get correct offsets for pointers
		ZoneBuilder::Zone::Buffer.Align(Utils::Stream::ALIGN_4);
		ZoneBuilder::Zone::IndexStart = ZoneBuilder::Zone::Buffer.GetBlockSize(Game::XFILE_BLOCK_VIRTUAL); // Mark AssetTable offset

		// AssetTable
		for (auto asset : ZoneBuilder::Zone::LoadedAssets)
		{
			Game::XAsset entry;
			entry.type = asset.type;
			entry.header.data = reinterpret_cast<void*>(-1);

			ZoneBuilder::Zone::Buffer.Save(&entry, sizeof(Game::XAsset));
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
			header->blockSize[i] = ZoneBuilder::Zone::Buffer.GetBlockSize((Game::XFILE_BLOCK_TYPES)i);
		}

		ZoneBuilder::Zone::Buffer.LeaveCriticalSection();
		ZoneBuilder::Zone::Buffer.PopBlock();
	}

	// Add branding asset
	// TODO: Check if a RawFile with the same name has already been added, to prevent conflicts.
	void ZoneBuilder::Zone::AddBranding()
	{
		char* data = "FastFile built using iw4x IW4 ZoneTool!";
		ZoneBuilder::Zone::Branding = { ZoneBuilder::Zone::ZoneName.data(), (int)strlen(data), 0, data };

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
			if (!ZoneBuilder::Zone::ScriptStrings.size())
			{
				ZoneBuilder::Zone::ScriptStrings.push_back("");
			}

			return 0;
		}

		std::string str = Game::SL_ConvertToString(gameIndex);
		int prev = FindScriptString(str);

		if (prev > 0)
		{
			ZoneBuilder::Zone::ScriptStringMap[gameIndex] = prev;
			return prev;
		}

		ZoneBuilder::Zone::ScriptStrings.push_back(str);
		ZoneBuilder::Zone::ScriptStringMap[gameIndex] = ScriptStrings.size();
		return ZoneBuilder::Zone::ScriptStrings.size();
	}

	// Find a local scriptString
	int ZoneBuilder::Zone::FindScriptString(std::string str)
	{
		int loc = 0;
		for (auto it : ScriptStrings)
		{
			++loc;
			if (!it.compare(str)) return loc;
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
		if (type < Game::XAssetType::ASSET_TYPE_COUNT)
		{
			ZoneBuilder::Zone::RenameMap[type][asset] = newName;
		}
	}

	// Return the new name for a given asset
	std::string ZoneBuilder::Zone::GetAssetName(Game::XAssetType type, std::string asset)
	{
		if (type < Game::XAssetType::ASSET_TYPE_COUNT)
		{
			if (ZoneBuilder::Zone::RenameMap[type].find(asset) != ZoneBuilder::Zone::RenameMap[type].end())
			{
				return ZoneBuilder::Zone::RenameMap[type][asset];
			}
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
		return (Flags::HasFlag("zonebuilder") && !Dedicated::IsDedicated());
	}

	ZoneBuilder::ZoneBuilder()
	{
		static_assert(sizeof(Game::XFileHeader) == 21, "Invalid XFileHeader structure!");
		static_assert(sizeof(Game::XFile) == 40, "Invalid XFile structure!");
		static_assert(Game::MAX_XFILE_COUNT == 8, "XFile block enum is invalid!");

		AssetHandler::OnLoad([] (Game::XAssetType type, Game::XAssetHeader asset, const char* name)
		{
// 			static void* blocTable = 0;
// 
// 			if (FastFiles::Current() == "iw4x_ui_mp" && type == Game::XAssetType::ASSET_TYPE_MATERIAL)
// 			{
// 				if (std::string(name) == "preview_mp_bloc")
// 				{
// 					blocTable = asset.material->stateBitTable;
// 				}
// 				else if (blocTable)
// 				{
// 					void* thisTable = asset.material->stateBitTable;
// 
// 					if (thisTable != blocTable)
// 					{
// 						OutputDebugStringA("DIFF!");
// 					}
// 					else
// 					{
// 						OutputDebugStringA("YAY!");
// 					}
// 				}
// 			}

			return true;
		});
		
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

			// Increase asset pools
			Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_MAP_ENTS, 10);

			// hunk size (was 300 MiB)
			Utils::Hook::Set<DWORD>(0x64A029, 0x38400000); // 900 MiB
			Utils::Hook::Set<DWORD>(0x64A057, 0x38400000);

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
						Logger::Print("%s\n", Game::DB_GetXAssetNameHandlers[*(Game::XAssetType*)data](&header));
					}, &type, false);
				}
			});
		}
	}
}
