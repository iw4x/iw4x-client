#include "STDInclude.hpp"

namespace Components
{
	Utils::Memory::Allocator ZoneBuilder::MemAllocator;

	std::string ZoneBuilder::TraceZone;
	std::vector<std::pair<Game::XAssetType, std::string>> ZoneBuilder::TraceAssets;

	ZoneBuilder::Zone::Zone(std::string name) : indexStart(0), externalSize(0),

		// Reserve 100MB by default.
		// That's totally fine, as the dedi doesn't load images and therefore doesn't need much memory.
		// That way we can be sure it won't need to reallocate memory.
		// Side note: if you need a fastfile larger than 100MB, you're doing it wrong-
		// Well, decompressed maps can get way larger than 100MB, so let's increase that.
		buffer(0xC800000),
		zoneName(name), dataMap("zone_source/" + name + ".csv"), branding{ nullptr }
	{}

	ZoneBuilder::Zone::Zone() : indexStart(0), externalSize(0), buffer(0xC800000),
		zoneName("null_zone"), dataMap(), branding{ nullptr }
	{}

	ZoneBuilder::Zone::~Zone()
	{
#ifdef DEBUG
		for (auto& subAsset : this->loadedSubAssets)
		{
			bool found = false;
			std::string name = Game::DB_GetXAssetName(&subAsset);

			for (auto& alias : this->aliasList)
			{
				if (subAsset.type == alias.first.type && name == Game::DB_GetXAssetName(&alias.first))
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				Logger::Error("Asset %s of type %s was loaded, but not written!", name.data(), Game::DB_GetXAssetTypeName(subAsset.type));
			}
		}

		for (auto& alias : this->aliasList)
		{
			bool found = false;
			std::string name = Game::DB_GetXAssetName(&alias.first);

			for (auto& subAsset : this->loadedSubAssets)
			{
				if (subAsset.type == alias.first.type && name == Game::DB_GetXAssetName(&subAsset))
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				Logger::Error("Asset %s of type %s was written, but not loaded!", name.data(), Game::DB_GetXAssetTypeName(alias.first.type));
			}
		}
#endif

		// Unload our fastfiles
		Game::XZoneInfo info;
		info.name = nullptr;
		info.allocFlags = 0;
		info.freeFlags = 0x20;

		Game::DB_LoadXAssets(&info, 1, true);

		AssetHandler::ClearTemporaryAssets();
		Localization::ClearTemp();
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
		if(!this->dataMap.isValid())
		{
			Logger::Print("Unable to load CSV for '%s'!\n", this->zoneName.data());
			return;
		}

		this->loadFastFiles();

		Logger::Print("Linking assets...\n");
		if (!this->loadAssets()) return;

		this->addBranding();

		Logger::Print("Saving...\n");
		this->saveData();

		if(this->buffer.hasBlock())
		{
			Logger::Error("Non-popped blocks left!\n");
		}

		Logger::Print("Compressing...\n");
		this->writeZone();
	}

	static bool buildingTechsets = false;
	static std::string techsetCSV = "";

	void ZoneBuilder::Zone::Zone::buildTechsets()
	{
		buildingTechsets = true;
		techsetCSV = "";

		this->zoneName = "iw4_techsets";

		// load all fastfiles in the zone dir and make a fastfile that contains all the techsets in those fastfiles

		std::string zone_dir = "zone/english";

		auto files = Utils::IO::ListFiles(zone_dir);
		std::string extension = ".ff";
		std::string load_suffix = "_load.ff";
		for (auto it : files)
		{
			if (!it.compare(it.length() - extension.length(), extension.length(), extension))
			{
				// ignore _load fastfiles
				if (!it.compare(it.length() - load_suffix.length(), load_suffix.length(), load_suffix))
					continue;
				techsetCSV += "require,"s + it.substr(zone_dir.length() + 1, it.length() - zone_dir.length() - 4) + "\r\n"s;
			}
		}

		this->dataMap = Utils::CSV(techsetCSV, false);

		this->loadFastFiles(); // this also builds the asset list cause we're just dumping the entire DB

		this->dataMap = Utils::CSV(techsetCSV, false); // update with asset list and not just requires

		Logger::Print("Linking assets...\n");
		if (!this->loadAssets()) return;

		this->addBranding();

		Logger::Print("Saving...\n");
		this->saveData();

		if (this->buffer.hasBlock())
		{
			Logger::Error("Non-popped blocks left!\n");
		}

		Logger::Print("Compressing...\n");
		this->writeZone();

		buildingTechsets = false;
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

				if (!this->loadAssetByName(this->dataMap.getElementAt(i, 0), this->dataMap.getElementAt(i, 1), false))
				{
					return false;
				}
			}
		}

		return true;
	}

	bool ZoneBuilder::Zone::loadAsset(Game::XAssetType type, void* data, bool isSubAsset)
	{
		Game::XAsset asset{ type, { data } };

		const char* name = Game::DB_GetXAssetName(&asset);

		if (name) return this->loadAssetByName(type, std::string(name), isSubAsset);
		else return false;
	}

	bool ZoneBuilder::Zone::loadAssetByName(Game::XAssetType type, std::string name, bool isSubAsset)
	{
		return this->loadAssetByName(Game::DB_GetXAssetTypeName(type), name, isSubAsset);
	}

	bool ZoneBuilder::Zone::loadAssetByName(std::string typeName, std::string name, bool isSubAsset)
	{
		Game::XAssetType type = Game::DB_GetXAssetNameType(typeName.data());

		// Sanitize name for empty assets
		if (name[0] == ',') name.erase(name.begin());

		if (this->findAsset(type, name) != -1 || this->findSubAsset(type, name).data) return true;

		if (type == Game::XAssetType::ASSET_TYPE_INVALID || type >= Game::XAssetType::ASSET_TYPE_COUNT)
		{
			Logger::Error("Error: Invalid asset type '%s'\n", typeName.data());
			return false;
		}

		Game::XAssetHeader assetHeader = AssetHandler::FindAssetForZone(type, name, this, isSubAsset);
		if (!assetHeader.data)
		{
			Logger::Error("Error: Missing asset '%s' of type '%s'\n", name.data(), Game::DB_GetXAssetTypeName(type));
			return false;
		}

		Game::XAsset asset;
		asset.type = type;
		asset.header = assetHeader;

		if (isSubAsset)
		{
			this->loadedSubAssets.push_back(asset);
		}
		else
		{
			this->loadedAssets.push_back(asset);
		}

		// Handle script strings
		AssetHandler::ZoneMark(asset, this);

		return true;
	}

	int ZoneBuilder::Zone::findAsset(Game::XAssetType type, std::string name)
	{
		if (name[0] == ',') name.erase(name.begin());

		for (unsigned int i = 0; i < this->loadedAssets.size(); ++i)
		{
			Game::XAsset* asset = &this->loadedAssets[i];

			if (asset->type != type) continue;

			const char* assetName = Game::DB_GetXAssetName(asset);
			if (assetName[0] == ',') ++assetName;

			if(this->getAssetName(type, assetName) == name)
			{
				return i;
			}

			if (name == assetName)
			{
				return i;
			}
		}

		return -1;
	}

	Game::XAssetHeader ZoneBuilder::Zone::findSubAsset(Game::XAssetType type, std::string name)
	{
		if (name[0] == ',') name.erase(name.begin());

		for (unsigned int i = 0; i < this->loadedSubAssets.size(); ++i)
		{
			Game::XAsset* asset = &this->loadedSubAssets[i];

			if (asset->type != type) continue;

			const char* assetName = Game::DB_GetXAssetName(asset);
			if (assetName[0] == ',') ++assetName;

			if (name == assetName)
			{
				return asset->header;
			}
		}

		return { nullptr };
	}

	Game::XAsset* ZoneBuilder::Zone::getAsset(int index)
	{
		if (static_cast<uint32_t>(index) < this->loadedAssets.size())
		{
			return &this->loadedAssets[index];
		}

		return nullptr;
	}

	uint32_t ZoneBuilder::Zone::getAssetTableOffset(int index)
	{
		Utils::Stream::Offset offset;
		offset.block = Game::XFILE_BLOCK_VIRTUAL;
		offset.offset = (this->indexStart + (index * sizeof(Game::XAsset)) + 4);
		return offset.getPackedOffset();
	}

	bool ZoneBuilder::Zone::hasAlias(Game::XAsset asset)
	{
		return this->getAlias(asset) != 0;
	}

	Game::XAssetHeader ZoneBuilder::Zone::saveSubAsset(Game::XAssetType type, void* ptr)
	{
		Game::XAssetHeader header { ptr };
		Game::XAsset asset { type, header };
		std::string name = Game::DB_GetXAssetName(&asset);

		int assetIndex = this->findAsset(type, name);
		if (assetIndex == -1) // nested asset
		{
			// already written. find alias and store in ptr
			if(this->hasAlias(asset))
			{
				header.data = reinterpret_cast<void*>(this->getAlias(asset));
			}
			else
			{
				asset.header = this->findSubAsset(type, name);
				if (!asset.header.data)
				{
					Logger::Error("Missing required asset '%s' (%s). Export failed!", name.data(), Game::DB_GetXAssetTypeName(type));
				}

#ifdef DEBUG
				Components::Logger::Print("Saving require (%s): %s\n", Game::DB_GetXAssetTypeName(type), Game::DB_GetXAssetNameHandlers[type](&header));
#endif

				// we alias the next 4 (aligned) bytes of the stream b/c DB_InsertPointer gives us a nice pointer to use as the alias
				// otherwise it would be a fuckfest trying to figure out where the alias is in the stream
				this->buffer.pushBlock(Game::XFILE_BLOCK_VIRTUAL);
				this->buffer.align(Utils::Stream::ALIGN_4);
				this->storeAlias(asset);
				this->buffer.increaseBlockSize(4);
				this->buffer.popBlock();

				this->buffer.pushBlock(Game::XFILE_BLOCK_TEMP);
				this->buffer.align(Utils::Stream::ALIGN_4);
				AssetHandler::ZoneSave(asset, this);
				this->buffer.popBlock();

				header.data = reinterpret_cast<void*>(-2); // DB_InsertPointer marker
			}
		}
		else
		{
			// asset was written normally. not sure this is even possible but its here
			header.data = reinterpret_cast<void*>(this->getAssetTableOffset(assetIndex));
		}

		return header;
	}

	void ZoneBuilder::Zone::writeZone()
	{
		FILETIME fileTime;
		GetSystemTimeAsFileTime(&fileTime);

		Game::XFileHeader header =
		{
#ifdef DEBUG
			XFILE_MAGIC_UNSIGNED,
#else
			XFILE_HEADER_IW4X | (static_cast<unsigned __int64>(XFILE_VERSION_IW4X) << 32),
#endif
			XFILE_VERSION,
			Game::XFileLanguage::XLANG_NONE,
			fileTime.dwHighDateTime,
			fileTime.dwLowDateTime
		};

		std::string outBuffer;
		outBuffer.append(reinterpret_cast<char*>(&header), sizeof(header));

		std::string zoneBuffer = this->buffer.toBuffer();

#ifndef DEBUG
		// Insert a random byte, this will destroy the whole alignment and result in a crash, if not handled
		zoneBuffer.insert(zoneBuffer.begin(), static_cast<char>(Utils::Cryptography::Rand::GenerateInt()));

		char lastByte = 0;
		for(unsigned int i = 0; i < zoneBuffer.size(); ++i )
		{
			char oldLastByte = lastByte;
			lastByte = zoneBuffer[i];

			Utils::RotLeft(zoneBuffer[i], 6);
			zoneBuffer[i] ^= -1;
			Utils::RotRight(zoneBuffer[i], 4);
			zoneBuffer[i] ^= oldLastByte;
		}
#endif

		zoneBuffer = Utils::Compression::ZLib::Compress(zoneBuffer);
		outBuffer.append(zoneBuffer);

		std::string outFile = "zone/" + this->zoneName + ".ff";
		Utils::IO::WriteFile(outFile, outBuffer);

		Logger::Print("done.\n");
		Logger::Print("Zone '%s' written with %d assets and %d script strings\n", outFile.data(), (this->aliasList.size() + this->loadedAssets.size()), this->scriptStrings.size());
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
			Game::XAsset entry = { asset.type, nullptr };
			Utils::Stream::ClearPointer(&entry.header.data);

			this->buffer.save(&entry);
		}

		// Assets
		for (auto asset : this->loadedAssets)
		{
			this->buffer.pushBlock(Game::XFILE_BLOCK_TEMP);
			this->buffer.align(Utils::Stream::ALIGN_4);

#ifdef DEBUG
			Components::Logger::Print("Saving (%s): %s\n", Game::DB_GetXAssetTypeName(asset.type), Game::DB_GetXAssetNameHandlers[asset.type](&asset.header));
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
		this->branding = { this->zoneName.data(), static_cast<int>(strlen(data)), 0, data };

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
	unsigned int ZoneBuilder::Zone::safeGetPointer(const void* pointer)
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

	void ZoneBuilder::Zone::storeAlias(Game::XAsset asset)
	{
		if (!this->hasAlias(asset))
		{
			this->aliasList.push_back({ asset, this->buffer.getPackedOffset() });
		}
	}

	unsigned int ZoneBuilder::Zone::getAlias(Game::XAsset asset)
	{
		std::string name = Game::DB_GetXAssetName(&asset);

		for (auto& entry : this->aliasList)
		{
			if (asset.type == entry.first.type && name == Game::DB_GetXAssetName(&entry.first))
			{
				return entry.second;
			}
		}

		return 0;
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
		static Utils::Value<bool> flag;

		if (!flag.isValid())
		{
			flag.set(Flags::HasFlag("zonebuilder"));
		}

		return (flag.get() && !Dedicated::IsEnabled());
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

	Game::XAssetHeader ZoneBuilder::GetEmptyAssetIfCommon(Game::XAssetType type, std::string name, ZoneBuilder::Zone* builder)
	{
		Game::XAssetHeader header = { nullptr };

		if (type >= 0 && type < Game::XAssetType::ASSET_TYPE_COUNT)
		{
			int zoneIndex = Game::DB_GetZoneIndex("common_mp");

			if (zoneIndex > 0)
			{
				Game::DB_EnumXAssetEntries(type, [&](Game::XAssetEntry* entry)
				{
					if (!header.data && entry->zoneIndex == zoneIndex && Game::DB_GetXAssetName(&entry->asset) == name)
					{
						// Allocate an empty asset (filled with zeros)
						header.data = builder->getAllocator()->allocate(Game::DB_GetXAssetSizeHandlers[type]());

						// Set the name to the original name, so it can be stored
						Game::DB_SetXAssetNameHandlers[type](&header, name.data());
						AssetHandler::StoreTemporaryAsset(type, header);

						// Set the name to the empty name
						Game::DB_SetXAssetNameHandlers[type](&header, builder->getAllocator()->duplicateString("," + name));
					}
				}, true, true);
			}
		}

		return header;
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

	
	Game::XZoneInfo baseZones_old [] = { { "code_pre_gfx_mp", 0, 0 },
		{ "localized_code_pre_gfx_mp", 0, 0 },
		{ "code_post_gfx_mp", 0, 0 },
		{ "localized_code_post_gfx_mp", 0, 0 },
		{ "common_mp", 0, 0 }
	};
	

	Game::XZoneInfo baseZones [] = {
		{ "defaults", 0, 0 },
		{ "shaders", 0, 0 },
		{ "common_mp", 0, 0 }
	};

	void ZoneBuilder::EndInit()
	{
		// do other init stuff
		DWORD R_RegisterDvars = 0x5196C0;
		DWORD NET_Init = 0x491860;
		DWORD SND_InitDriver = 0x4F5090;
		DWORD SND_Init = 0x46A630;
		DWORD G_SetupWeaponDef = 0x4E1F30;
		__asm
		{
			call R_RegisterDvars
			call NET_Init
			call SND_InitDriver
			call SND_Init
		}

		// now load default assets and shaders
		if (FastFiles::Exists("defaults") && FastFiles::Exists("shaders"))
		{
			Game::DB_LoadXAssets(baseZones, 3, 0);
		}
		else
		{
			Logger::Print("Warning: Missing new init zones (defaults.ff & shaders.ff). You will need to load fastfiles to manually obtain techsets.\n");
			Game::DB_LoadXAssets(baseZones_old, 5, 0);
		}

		Logger::Print("Waiting for fastiles to load...");
		while (!Game::Sys_IsDatabaseReady()) std::this_thread::sleep_for(100ms);
		Logger::Print("Done!\n");

		// defaults need to load before we do this
		__asm
		{
			call G_SetupWeaponDef
		}
	}

	void __declspec(naked) ZoneBuilder::EndInitStub()
	{
		__asm
		{
			add esp, 0x14
			call ZoneBuilder::EndInit
			retn
		}
	}

	ZoneBuilder::ZoneBuilder()
	{
		// ReSharper disable CppStaticAssertFailure
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

			// Don't read stats
			Utils::Hook(0x4875E1, 0x487717, HOOK_JUMP).install()->quick();

			// Increase asset pools
			Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_MAP_ENTS, 10);
			Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_XMODELSURFS, 8192);
			Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_IMAGE, 14336);
			Game::ReallocateAssetPool(Game::XAssetType::ASSET_TYPE_TECHNIQUE_SET, 1536);

			// hunk size (was 300 MiB)
			Utils::Hook::Set<DWORD>(0x64A029, 0x38400000); // 900 MiB
			Utils::Hook::Set<DWORD>(0x64A057, 0x38400000);

			// Further neuter Com_Init
			Utils::Hook(0x60BBCE, ZoneBuilder::EndInitStub, HOOK_JUMP).install()->quick();

			// dont run Live_Frame
			Utils::Hook::Nop(0x48A0AF, 5);

			AssetHandler::OnLoad([](Game::XAssetType type, Game::XAssetHeader /*asset*/, std::string name, bool* /*restrict*/)
			{
				if (!ZoneBuilder::TraceZone.empty() && ZoneBuilder::TraceZone == FastFiles::Current())
				{
					ZoneBuilder::TraceAssets.push_back({ type, name });
				}
			});

			Command::Add("verifyzone", [](Command::Params* params)
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

			Command::Add("buildzone", [](Command::Params* params)
			{
				if (params->length() < 2) return;

				std::string zoneName = params->get(1);
				Logger::Print("Building zone '%s'...\n", zoneName.data());

				Zone(zoneName).build();
			});

			Command::Add("buildall", [](Command::Params*)
			{
				auto zoneSources = FileSystem::GetSysFileList(Dvar::Var("fs_basepath").get<std::string>() + "\\zone_source", "csv", false);

				for (auto source : zoneSources)
				{
					if (Utils::String::EndsWith(source, ".csv"))
					{
						source = source.substr(0, source.find(".csv"));
					}

					Command::Execute(Utils::String::VA("buildzone %s", source.data()), true);
				}
			});

			Command::Add("buildtechsets", [](Command::Params*)
			{
				Zone().buildTechsets();
			});

			AssetHandler::OnLoad([](Game::XAssetType type, Game::XAssetHeader /*asset*/, std::string name, bool* restrict)
			{
				if (buildingTechsets && type != Game::XAssetType::ASSET_TYPE_TECHNIQUE_SET)
				{
					*restrict = true;
					return;
				}

				techsetCSV += "techset,"s + name + "\r\n"s;
			});

			Command::Add("listassets", [](Command::Params* params)
			{
				if (params->length() < 2) return;
				Game::XAssetType type = Game::DB_GetXAssetNameType(params->get(1));

				if (type != Game::XAssetType::ASSET_TYPE_INVALID)
				{
					Game::DB_EnumXAssets(type, [](Game::XAssetHeader header, void* data)
					{
						Game::XAsset asset = { *reinterpret_cast<Game::XAssetType*>(data), header };
						Logger::Print("%s\n", Game::DB_GetXAssetName(&asset));
					}, &type, false);
				}
			});

			Command::Add("materialInfoDump", [](Command::Params*)
			{
				Game::DB_EnumXAssets(Game::ASSET_TYPE_MATERIAL, [](Game::XAssetHeader header, void*)
				{
					Logger::Print("%s: %X %X %X\n", header.material->name, header.material->sortKey & 0xFF, header.material->gameFlags & 0xFF, header.material->stateFlags & 0xFF);
				}, nullptr, false);
			});

			Command::Add("iwiDump", [](Command::Params* params)
			{
				if (params->length() < 2) return;

				std::string path = Utils::String::VA("%s\\mods\\%s\\images", Dvar::Var("fs_basepath").get<const char*>(), params->get(1));
				std::vector<std::string> images = FileSystem::GetSysFileList(path, "iwi", false);

				for(auto i = images.begin(); i != images.end();)
				{
					*i = Utils::String::VA("images/%s", i->data());

					if(FileSystem::File(*i).exists())
					{
						i = images.erase(i);
						continue;
					}

					++i;
				}

				Logger::Print("------------------- BEGIN IWI DUMP -------------------\n");
				Logger::Print("%s\n", json11::Json(images).dump().data());
				Logger::Print("------------------- END IWI DUMP -------------------\n");
			});
		}
	}

	ZoneBuilder::~ZoneBuilder()
	{
		assert(ZoneBuilder::MemAllocator.empty());
	}

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
	bool ZoneBuilder::unitTest()
	{
		printf("Testing circular bit shifting (left)...");

		unsigned int integer = 0x80000000;
		Utils::RotLeft(integer, 1);

		if(integer != 1)
		{
			printf("Error\n");
			printf("Bit shifting failed: %X\n", integer);
			return false;
		}

		printf("Success\n");
		printf("Testing circular bit shifting (right)...");

		unsigned char byte = 0b00000011;
		Utils::RotRight(byte, 2);

		if (byte != 0b11000000)
		{
			printf("Error\n");
			printf("Bit shifting failed %X\n", byte & 0xFF);
			return false;
		}

		printf("Success\n");
		return true;
	}
#endif
}
