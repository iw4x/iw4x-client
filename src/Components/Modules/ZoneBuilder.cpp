#include "STDInclude.hpp"

namespace Components
{
	Utils::Memory::Allocator ZoneBuilder::MemAllocator;

	std::string ZoneBuilder::TraceZone;
	std::vector<std::pair<Game::XAssetType, std::string>> ZoneBuilder::TraceAssets;

	ZoneBuilder::Zone::Zone(std::string name) : dataMap("zone_source/" + name + ".csv"), zoneName(name), indexStart(0), externalSize(0), branding { 0 },

		// Reserve 100MB by default.
		// That's totally fine, as the dedi doesn't load images and therefore doesn't need much memory.
		// That way we can be sure it won't need to reallocate memory.
		// Side note: if you need a fastfile larger than 100MB, you're doing it wrong-
		// Well, decompressed maps can get way larger than 100MB, so let's increase that.
		buffer(0xC800000)
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

		this->loadedAssets.clear();
		this->scriptStrings.clear();
		this->scriptStringMap.clear();
	}

	Utils::Stream* ZoneBuilder::Zone::getBuffer()
	{
		return &this->buffer;
	}

	Utils::Memory::Allocator* ZoneBuilder::Zone::getAllocator()
	{
		return &this->memAllocator;
	}

	void ZoneBuilder::Zone::Zone::build()
	{
		this->loadFastFiles();

		Logger::Print("Linking assets...\n");
		if (!this->loadAssets()) return;

		this->addBranding();

		Logger::Print("Saving...\n");
		this->saveData();

		Logger::Print("Compressing...\n");
		this->writeZone();
	}

	void ZoneBuilder::Zone::loadFastFiles()
	{
		Logger::Print("Loading required FastFiles...\n");

		for (int i = 0; i < this->dataMap.getRows(); ++i)
		{
			if (this->dataMap.getElementAt(i, 0) == "require")
			{
				std::string fastfile = this->dataMap.getElementAt(i, 1);

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

	bool ZoneBuilder::Zone::loadAssets()
	{
		for (int i = 0; i < this->dataMap.getRows(); ++i)
		{
			if (this->dataMap.getElementAt(i, 0) != "require")
			{
				if (this->dataMap.getColumns(i) > 2)
				{
					if (this->dataMap.getElementAt(i, 0) == "localize")
					{
						std::string stringOverride = this->dataMap.getElementAt(i, 2);
						Utils::String::Replace(stringOverride, "\\n", "\n");

						Localization::SetTemp(this->dataMap.getElementAt(i, 1), stringOverride);
					}
					else
					{
						std::string oldName = this->dataMap.getElementAt(i, 1);
						std::string newName = this->dataMap.getElementAt(i, 2);
						std::string typeName = this->dataMap.getElementAt(i, 0).data();
						Game::XAssetType type = Game::DB_GetXAssetNameType(typeName.data());

						if (type < Game::XAssetType::ASSET_TYPE_COUNT && type >= 0)
						{
							this->renameAsset(type, oldName, newName);
						}
						else
						{
							Logger::Error("Unable to rename '%s' to '%s' as the asset type '%s' is invalid!", oldName.data(), newName.data(), typeName.data());
						}
					}
				}

				if (!this->loadAsset(this->dataMap.getElementAt(i, 0), this->dataMap.getElementAt(i, 1)))
				{
					return false;
				}
			}
		}

		return true;
	}

	bool ZoneBuilder::Zone::loadAsset(Game::XAssetType type, std::string name)
	{
		return this->loadAsset(Game::DB_GetXAssetTypeName(type), name);
	}

	bool ZoneBuilder::Zone::loadAsset(std::string typeName, std::string name)
	{
		Game::XAssetType type = Game::DB_GetXAssetNameType(typeName.data());

		if (this->findAsset(type, name) != -1) return true;

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

		this->loadedAssets.push_back(asset);
		return true;
	}

	int ZoneBuilder::Zone::findAsset(Game::XAssetType type, std::string name)
	{
		for (unsigned int i = 0; i < this->loadedAssets.size(); ++i)
		{
			Game::XAsset* asset = &this->loadedAssets[i];

			if (asset->type != type) continue;

			const char* assetName = DB_GetXAssetName(asset);

			if (name == assetName)
			{
				return i;
			}
		}

		return -1;
	}

	Game::XAsset* ZoneBuilder::Zone::getAsset(int index)
	{
		if (static_cast<uint32_t>(index) < this->loadedAssets.size())
		{
			return &this->loadedAssets[index];
		}

		return nullptr;
	}

	uint32_t ZoneBuilder::Zone::getAssetOffset(int index)
	{
		Utils::Stream::Offset offset;
		offset.block = Game::XFILE_BLOCK_VIRTUAL;
		offset.offset = (this->indexStart + (index * sizeof(Game::XAsset)) + 4);
		return offset.getPackedOffset();
	}

	Game::XAssetHeader ZoneBuilder::Zone::requireAsset(Game::XAssetType type, const char* name)
	{
		Game::XAssetHeader header;
		Utils::Stream::ClearPointer(&header.data);

		int assetIndex = this->findAsset(type, name);

		if (assetIndex != -1)
		{
			header.data = reinterpret_cast<void*>(this->getAssetOffset(assetIndex));
		}
		else
		{
			Logger::Error("Missing required asset '%s' (%s). Export failed!", name, Game::DB_GetXAssetTypeName(type));
		}

		return header;
	}

	void ZoneBuilder::Zone::writeZone()
	{
		FILETIME fileTime;
		GetSystemTimeAsFileTime(&fileTime);

		Game::XFileHeader header =
		{
			XFILE_MAGIC_UNSIGNED,
#ifdef DEBUG
			XFILE_VERSION,
#else
			XFILE_VERSION_IW4X,
#endif
			Game::XFileLanguage::XLANG_NONE,
			fileTime.dwHighDateTime,
			fileTime.dwLowDateTime
		};

		std::string outBuffer;
		outBuffer.append(reinterpret_cast<char*>(&header), sizeof(header));

		std::string zoneBuffer = this->buffer.toBuffer();
		zoneBuffer = Utils::Compression::ZLib::Compress(zoneBuffer);
		outBuffer.append(zoneBuffer);

		std::string outFile = "zone/" + this->zoneName + ".ff";
		Utils::IO::WriteFile(outFile, outBuffer);

		Logger::Print("done.\n");
		Logger::Print("Zone '%s' written with %d assets\n", outFile.data(), this->loadedAssets.size());
	}

	void ZoneBuilder::Zone::saveData()
	{
		// Add header
		Game::ZoneHeader zoneHeader = { 0 };
		zoneHeader.assetList.assetCount = this->loadedAssets.size();
		Utils::Stream::ClearPointer(&zoneHeader.assetList.assets);

		// Increment ScriptStrings count (for empty script string) if available
		if (!this->scriptStrings.empty())
		{
			zoneHeader.assetList.stringList.count = this->scriptStrings.size() + 1;
			Utils::Stream::ClearPointer(&zoneHeader.assetList.stringList.strings);
		}

		// Write header
		this->buffer.save(&zoneHeader, sizeof(Game::ZoneHeader));
		this->buffer.pushBlock(Game::XFILE_BLOCK_VIRTUAL); // Push main stream onto the stream stack

		// Write ScriptStrings, if available
		if (!this->scriptStrings.empty())
		{
			this->buffer.saveNull(4); // Empty script string?
                                      // This actually represents a NULL string, but as scriptString. 
                                      // So scriptString loading for NULL scriptStrings from fastfile results in a NULL scriptString.
									  // That's the reason why the count is incremented by 1, if scriptStrings are available.

			// Write ScriptString pointer table
			for (size_t i = 0; i < this->scriptStrings.size(); ++i)
			{
				this->buffer.saveMax(4);
			}

			this->buffer.align(Utils::Stream::ALIGN_4);

			// Write ScriptStrings
			for (auto ScriptString : this->scriptStrings)
			{
				this->buffer.saveString(ScriptString.data());
			}
		}

		// Align buffer (4 bytes) to get correct offsets for pointers
		this->buffer.align(Utils::Stream::ALIGN_4);
		this->indexStart = this->buffer.getBlockSize(Game::XFILE_BLOCK_VIRTUAL); // Mark AssetTable offset

		// AssetTable
		for (auto asset : this->loadedAssets)
		{
			Game::XAsset entry = { asset.type, 0 };
			Utils::Stream::ClearPointer(&entry.header.data);

			this->buffer.save(&entry);
		}

		// Assets
		for (auto asset : this->loadedAssets)
		{
			this->buffer.pushBlock(Game::XFILE_BLOCK_TEMP);
			this->buffer.align(Utils::Stream::ALIGN_4);

#ifdef DEBUG
			Components::Logger::Print("Saving (%s): %s\n", Game::DB_GetXAssetTypeName(asset.type), Game::DB_GetXAssetName(&asset));
#endif

			this->store(asset.header);
			AssetHandler::ZoneSave(asset, this);

			this->buffer.popBlock();
		}

		// Adapt header
		this->buffer.enterCriticalSection();
		Game::XFile* header = reinterpret_cast<Game::XFile*>(this->buffer.data());
		header->size = this->buffer.length() - sizeof(Game::XFile); // Write correct data size
		header->externalSize = this->externalSize; // This actually stores how much external data has to be loaded. It's used to calculate the loadscreen progress

		// Write stream sizes
		for (int i = 0; i < Game::MAX_XFILE_COUNT; ++i)
		{
			header->blockSize[i] = this->buffer.getBlockSize(static_cast<Game::XFILE_BLOCK_TYPES>(i));
		}

		this->buffer.leaveCriticalSection();
		this->buffer.popBlock();
	}

	// Add branding asset
	void ZoneBuilder::Zone::addBranding()
	{
		char* data = "FastFile built using IW4x ZoneTool!";
		this->branding = { this->zoneName.data(), (int)strlen(data), 0, data };

		if (this->findAsset(Game::XAssetType::ASSET_TYPE_RAWFILE, this->branding.name) != -1)
		{
			Logger::Error("Unable to add branding. Asset '%s' already exists!", this->branding.name);
		}

		Game::XAssetHeader header = { &this->branding };
		Game::XAsset brandingAsset = { Game::XAssetType::ASSET_TYPE_RAWFILE, header };
		this->loadedAssets.push_back(brandingAsset);
	}

	// Check if the given pointer has already been mapped
	bool ZoneBuilder::Zone::hasPointer(const void* pointer)
	{
		return (this->pointerMap.find(pointer) != this->pointerMap.end());
	}

	// Get stored offset for given file pointer
	uint32_t ZoneBuilder::Zone::safeGetPointer(const void* pointer)
	{
		if (this->hasPointer(pointer))
		{
			return this->pointerMap[pointer];
		}

		return NULL;
	}

	void ZoneBuilder::Zone::storePointer(const void* pointer)
	{
		this->pointerMap[pointer] = this->buffer.getPackedOffset();
	}

	int ZoneBuilder::Zone::addScriptString(std::string str)
	{
		return this->addScriptString(Game::SL_GetString(str.data(), 0));
	}

	// Mark a scriptString for writing and map it.
	int ZoneBuilder::Zone::addScriptString(unsigned short gameIndex)
	{
		// Handle NULL scriptStrings
		// Might optimize that later
		if (!gameIndex)
		{
			if (this->scriptStrings.empty())
			{
				this->scriptStrings.push_back("");
			}

			return 0;
		}

		std::string str = Game::SL_ConvertToString(gameIndex);
		int prev = this->findScriptString(str);

		if (prev > 0)
		{
			this->scriptStringMap[gameIndex] = prev;
			return prev;
		}

		this->scriptStrings.push_back(str);
		this->scriptStringMap[gameIndex] = this->scriptStrings.size();
		return this->scriptStrings.size();
	}

	// Find a local scriptString
	int ZoneBuilder::Zone::findScriptString(std::string str)
	{
		for (unsigned int i = 0; i < this->scriptStrings.size(); ++i)
		{
			if (this->scriptStrings[i] == str)
			{
				return (i + 1);
			}
		}

		return -1;
	}

	// Remap a scriptString to it's corresponding value in the local scriptString table.
	void ZoneBuilder::Zone::mapScriptString(unsigned short* gameIndex)
	{
		*gameIndex = 0xFFFF & this->scriptStringMap[*gameIndex];
	}

	// Store a new name for a given asset
	void ZoneBuilder::Zone::renameAsset(Game::XAssetType type, std::string asset, std::string newName)
	{
		if (type < Game::XAssetType::ASSET_TYPE_COUNT && type >= 0)
		{
			this->renameMap[type][asset] = newName;
		}
		else
		{
			Logger::Error("Unable to rename '%s' to '%s' as the asset type is invalid!", asset.data(), newName.data());
		}
	}

	// Return the new name for a given asset
	std::string ZoneBuilder::Zone::getAssetName(Game::XAssetType type, std::string asset)
	{
		if (type < Game::XAssetType::ASSET_TYPE_COUNT && type >= 0)
		{
			if (this->renameMap[type].find(asset) != this->renameMap[type].end())
			{
				return this->renameMap[type][asset];
			}
		}
		else
		{
			Logger::Error("Unable to get name for '%s' as the asset type is invalid!", asset.data());
		}

		return asset;
	}

	void ZoneBuilder::Zone::store(Game::XAssetHeader header)
	{
		if (!this->hasPointer(header.data)) // We should never have to restore a pointer, so this expression should always resolve into false
		{
			this->storePointer(header.data);
		}
	}

	void ZoneBuilder::Zone::incrementExternalSize(unsigned int size)
	{
		this->externalSize += size;
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

	int ZoneBuilder::StoreTexture(Game::GfxImageLoadDef **loadDef, Game::GfxImage *image)
	{
		size_t size = 16 + (*loadDef)->resourceSize;
		void* data = ZoneBuilder::MemAllocator.allocate(size);
		std::memcpy(data, *loadDef, size);

		image->loadDef = reinterpret_cast<Game::GfxImageLoadDef *>(data);

		return 0;
	}

	void ZoneBuilder::ReleaseTexture(Game::XAssetHeader header)
	{
		if (header.image && header.image->loadDef)
		{
			ZoneBuilder::MemAllocator.free(header.image->loadDef);
		}
	}

	ZoneBuilder::ZoneBuilder()
	{
		AssertSize(Game::XFileHeader, 21);
		AssertSize(Game::XFile, 40);
		static_assert(Game::MAX_XFILE_COUNT == 8, "XFile block enum is invalid!");

		ZoneBuilder::EndAssetTrace();
		
		if (ZoneBuilder::IsEnabled())
		{
			// Prevent loading textures (preserves loaddef)
			//Utils::Hook::Set<BYTE>(Game::Load_Texture, 0xC3);

			// Store the loaddef
			Utils::Hook(Game::Load_Texture, StoreTexture, HOOK_JUMP).install()->quick();

			// Release the loaddef
			Game::DB_ReleaseXAssetHandlers[Game::XAssetType::ASSET_TYPE_IMAGE] = ZoneBuilder::ReleaseTexture;

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

			// Don't mark clip maps as 'in use'
			Utils::Hook::Nop(0x405E07, 7);

			// Don't mark assets
			//Utils::Hook::Nop(0x5BB632, 5);

			// Don't load sounds
			//Utils::Hook::Set<BYTE>(0x413430, 0xC3);

			// Don't display errors when assets are missing (we might manually build those)
			Utils::Hook::Nop(0x5BB3F2, 5);
			Utils::Hook::Nop(0x5BB422, 5);
			Utils::Hook::Nop(0x5BB43A, 5);

			// Increase asset pools
			Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_MAP_ENTS, 10);
			Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_XMODELSURFS, 8192);
			Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_IMAGE, 14336);
			Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_TECHSET, 1536);

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

			Command::Add("verifyzone", [] (Command::Params* params)
			{
				if (params->length() < 2) return;

				std::string zone = params->get(1);

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

			Command::Add("buildzone", [] (Command::Params* params)
			{
				if (params->length() < 2) return;

				std::string zoneName = params->get(1);
				Logger::Print("Building zone '%s'...\n", zoneName.data());

				Zone(zoneName).build();
			});

			Command::Add("buildall", [] (Command::Params*)
			{
				auto zoneSources = FileSystem::GetSysFileList(Dvar::Var("fs_basepath").get<std::string>() + "\\zone_source", "csv", false);

				for (auto source : zoneSources)
				{
					if (Utils::String::EndsWith(source, ".csv"))
					{
						source = source.substr(0, source.find(".csv"));
					}

					Command::Execute(fmt::sprintf("buildzone %s", source.data()), true);
				}
			});

			Command::Add("listassets", [] (Command::Params* params)
			{
				if (params->length() < 2) return;
				Game::XAssetType type = Game::DB_GetXAssetNameType(params->get(1));

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

	ZoneBuilder::~ZoneBuilder()
	{
		assert(ZoneBuilder::MemAllocator.empty());
	}
}
