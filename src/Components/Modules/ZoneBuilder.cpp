#include <STDInclude.hpp>

#include <version.hpp>

#include "AssetInterfaces/ILocalizeEntry.hpp"

namespace Components
{
	std::string ZoneBuilder::TraceZone;
	std::vector<std::pair<Game::XAssetType, std::string>> ZoneBuilder::TraceAssets;

	bool ZoneBuilder::MainThreadInterrupted;
	DWORD ZoneBuilder::InterruptingThreadId;
	volatile bool ZoneBuilder::Terminate = false;
	std::thread ZoneBuilder::CommandThread;

	Dvar::Var ZoneBuilder::PreferDiskAssetsDvar;

	ZoneBuilder::Zone::Zone(const std::string& name) : indexStart(0), externalSize(0),
		// Reserve 100MB by default.
		// That's totally fine, as the dedi doesn't load images and therefore doesn't need much memory.
		// That way we can be sure it won't need to reallocate memory.
		// Side note: if you need a fastfile larger than 100MB, you're doing it wrong-
		// Well, decompressed maps can get way larger than 100MB, so let's increase that.
		buffer(0xC800000),
		zoneName(name), dataMap("zone_source/" + name + ".csv"), branding{nullptr}, assetDepth(0)
	{
	}

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
				Logger::Print("Asset {} of type {} was loaded, but not written!", name, Game::DB_GetXAssetTypeName(subAsset.type));
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
				Logger::Error(Game::ERR_FATAL, "Asset {} of type {} was written, but not loaded!", name, Game::DB_GetXAssetTypeName(alias.first.type));
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
		if (!this->dataMap.isValid())
		{
			Logger::Print("Unable to load CSV for '{}'!\n", this->zoneName);
			return;
		}

		this->loadFastFiles();

		Logger::Print("Linking assets...\n");
		if (!this->loadAssets()) return;

		this->addBranding();

		Logger::Print("Saving...\n");
		this->saveData();

		if (this->buffer.hasBlock())
		{
			Logger::Error(Game::ERR_FATAL, "Non-popped blocks left!\n");
		}

		Logger::Print("Compressing...\n");
		this->writeZone();
	}

	void ZoneBuilder::Zone::loadFastFiles()
	{
		Logger::Print("Loading required FastFiles...\n");

		for (std::size_t i = 0; i < this->dataMap.getRows(); ++i)
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
					Logger::Print("Zone '{}' already loaded\n", fastfile);
				}
			}
		}
	}

	bool ZoneBuilder::Zone::loadAssets()
	{
		for (std::size_t i = 0; i < this->dataMap.getRows(); ++i)
		{
			if (this->dataMap.getElementAt(i, 0) == "require"s)
			{
				continue;
			}

			if (this->dataMap.getElementAt(i, 0) == "localize"s)
			{
				const auto filename = this->dataMap.getElementAt(i, 1);
				if (FileSystem::File file = std::format("localizedstrings/{}.str", filename))
				{
					Assets::ILocalizeEntry::ParseLocalizedStringsFile(this, filename, file.getName());
					continue;
				}

				if (FileSystem::File file = std::format("localizedstrings/{}.json", filename))
				{
					Assets::ILocalizeEntry::ParseLocalizedStringsJson(this, file);
					continue;
				}
			}

			if (this->dataMap.getColumns(i) > 2)
			{
				auto oldName = this->dataMap.getElementAt(i, 1);
				auto newName = this->dataMap.getElementAt(i, 2);
				auto typeName = this->dataMap.getElementAt(i, 0);
				auto type = Game::DB_GetXAssetNameType(typeName.data());

				if (type < Game::XAssetType::ASSET_TYPE_COUNT && type >= 0)
				{
					this->renameAsset(type, oldName, newName);
				}
				else
				{
					Logger::Error(Game::ERR_FATAL, "Unable to rename '{}' to '{}' as the asset type '{}' is invalid!", oldName, newName, typeName);
				}
			}

			if (!this->loadAssetByName(this->dataMap.getElementAt(i, 0), this->dataMap.getElementAt(i, 1), false))
			{
				return false;
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

	bool ZoneBuilder::Zone::loadAssetByName(Game::XAssetType type, const std::string& name, bool isSubAsset)
	{
		return this->loadAssetByName(Game::DB_GetXAssetTypeName(type), name, isSubAsset);
	}

	bool ZoneBuilder::Zone::loadAssetByName(const std::string& typeName, std::string name, bool isSubAsset)
	{
		Game::XAssetType type = Game::DB_GetXAssetNameType(typeName.data());

		if (name.find(' ', 0) != std::string::npos)
		{
			Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Asset with name '{}' contains spaces. Check your zone source file to ensure this is correct!\n", name);
		}

		// Sanitize name for empty assets
		if (name[0] == ',') name.erase(name.begin());

		// Fix forward slashes for FXEffectDef (and probably other assets)
		std::replace(name.begin(), name.end(), '\\', '/');

		if (this->findAsset(type, name) != -1 || this->findSubAsset(type, name).data) return true;

		if (type == Game::XAssetType::ASSET_TYPE_INVALID || type >= Game::XAssetType::ASSET_TYPE_COUNT)
		{
			Logger::Error(Game::ERR_FATAL, "Invalid asset type '{}'\n", typeName);
			return false;
		}

		Game::XAssetHeader assetHeader = AssetHandler::FindAssetForZone(type, name, this, isSubAsset);

		if (!assetHeader.data)
		{
			Logger::Error(Game::ERR_FATAL, "Missing asset '{}' of type '{}'\n", name, Game::DB_GetXAssetTypeName(type));
			return false;
		}

		Game::XAsset asset;
		asset.type = type;
		asset.header = assetHeader;

		// Handle script strings
		AssetHandler::ZoneMark(asset, this);

		if (isSubAsset)
		{
			this->loadedSubAssets.push_back(asset);
		}
		else
		{
			this->loadedAssets.push_back(asset);
		}


		return true;
	}

	int ZoneBuilder::Zone::findAsset(Game::XAssetType type, std::string name)
	{
		if (name[0] == ',') name.erase(name.begin());

		for (unsigned int i = 0; i < this->loadedAssets.size(); ++i)
		{
			Game::XAsset* asset = &this->loadedAssets[i];

			if (asset->type != type) continue;

			const auto* assetName = Game::DB_GetXAssetName(asset);
			if (!assetName) return -1;
			if (assetName[0] == ',' && assetName[1] != '\0') ++assetName;
			else return -1;

			if (this->getAssetName(type, assetName) == name)
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
			if (this->hasAlias(asset))
			{
				header.data = reinterpret_cast<void*>(this->getAlias(asset));
			}
			else
			{
				asset.header = this->findSubAsset(type, name);
				if (!asset.header.data)
				{
					Logger::Error(Game::ERR_FATAL, "Missing required asset '{}' ({}). Export failed!", name, Game::DB_GetXAssetTypeName(type));
				}

				Logger::Debug("Saving require ({}): {}", Game::DB_GetXAssetTypeName(type), Game::DB_GetXAssetNameHandlers[type](&header));

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
#ifndef GENERATE_IW4X_SPECIFIC_ZONES
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

#ifdef GENERATE_IW4X_SPECIFIC_ZONES
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

		Utils::IO::WriteFile("uncompressed", zoneBuffer);
		const auto _0 = gsl::finally([]
		{
			Utils::IO::RemoveFile("uncompressed");
		});

		zoneBuffer = Utils::Compression::ZLib::Compress(zoneBuffer);
		outBuffer.append(zoneBuffer);

		std::string outFile = "zone/" + this->zoneName + ".ff";
		Utils::IO::WriteFile(outFile, outBuffer);

		Logger::Print("done.\n");
		Logger::Print("Zone '{}' written with {} assets and {} script strings\n", outFile, (this->aliasList.size() + this->loadedAssets.size()), this->scriptStrings.size());
	}

	void ZoneBuilder::Zone::saveData()
	{
		// Add header
		Game::ZoneHeader zoneHeader = { 0 };
		zoneHeader.assetList.assetCount = this->loadedAssets.size();
		Utils::Stream::ClearPointer(&zoneHeader.assetList.assets);

		// Increment ScriptStrings count (for empty script string) if available
		zoneHeader.assetList.stringList.count = this->scriptStrings.size() + 1;
		Utils::Stream::ClearPointer(&zoneHeader.assetList.stringList.strings);

		// Write header
		this->buffer.save(&zoneHeader, sizeof(Game::ZoneHeader));
		this->buffer.pushBlock(Game::XFILE_BLOCK_VIRTUAL); // Push main stream onto the stream stack

		// Write ScriptStrings, if available
		this->buffer.saveNull(4);
		// Empty script string?
		// This actually represents a NULL string, but as scriptString.
		// So scriptString loading for NULL scriptStrings from fastfile results in a NULL scriptString.
		// That's the reason why the count is incremented by 1, if scriptStrings are available.

		// Write ScriptString pointer table
		for (std::size_t i = 0; i < this->scriptStrings.size(); ++i)
		{
			this->buffer.saveMax(4);
		}

		// Write ScriptStrings
		for (auto ScriptString : this->scriptStrings)
		{
			this->buffer.saveString(ScriptString.data());
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

			Logger::Debug("Saving ({}): {}", Game::DB_GetXAssetTypeName(asset.type), Game::DB_GetXAssetNameHandlers[asset.type](&asset.header));

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
		constexpr auto* data = "Built using the IW4x Zone:B:uilder Version 4";
		auto dataLen = std::strlen(data); // + 1 is added by the save code

		this->branding = {this->zoneName.data(), 0, static_cast<int>(dataLen), data};

		if (this->findAsset(Game::XAssetType::ASSET_TYPE_RAWFILE, this->branding.name) != -1)
		{
			Logger::Error(Game::ERR_FATAL, "Unable to add branding. Asset '{}' already exists!", this->branding.name);
		}

		Game::XAssetHeader header = { &this->branding };
		Game::XAsset brandingAsset = { Game::XAssetType::ASSET_TYPE_RAWFILE, header };
		this->loadedAssets.push_back(brandingAsset);
	}

	// Check if the given pointer has already been mapped
	bool ZoneBuilder::Zone::hasPointer(const void* pointer)
	{
		return this->pointerMap.contains(pointer);
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

	int ZoneBuilder::Zone::addScriptString(const std::string& str)
	{
		return this->addScriptString(static_cast<std::uint16_t>(Game::SL_GetString(str.data(), 0)));
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
	int ZoneBuilder::Zone::findScriptString(const std::string& str)
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

	void ZoneBuilder::Zone::addRawAsset(Game::XAssetType type, void* ptr)
	{
		this->loadedAssets.push_back({type, {ptr}});
	}

	// Remap a scriptString to it's corresponding value in the local scriptString table.
	void ZoneBuilder::Zone::mapScriptString(unsigned short& gameIndex)
	{
		gameIndex = 0xFFFF & this->scriptStringMap[gameIndex];
	}

	// Store a new name for a given asset
	void ZoneBuilder::Zone::renameAsset(Game::XAssetType type, const std::string& asset, const std::string& newName)
	{
		if (type < Game::XAssetType::ASSET_TYPE_COUNT && type >= 0)
		{
			this->renameMap[type][asset] = newName;
		}
		else
		{
			Logger::Error(Game::ERR_FATAL, "Unable to rename '{}' to '{}' as the asset type is invalid!", asset, newName);
		}
	}

	// Return the new name for a given asset
	std::string ZoneBuilder::Zone::getAssetName(Game::XAssetType type, const std::string& asset)
	{
		if (type < Game::XAssetType::ASSET_TYPE_COUNT && type >= 0)
		{
			if (this->renameMap[type].contains(asset))
			{
				return this->renameMap[type][asset];
			}
		}
		else
		{
			Logger::Error(Game::ERR_FATAL, "Unable to get name for '{}' as the asset type is invalid!", asset);
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
		static std::optional<bool> flag;

		if (!flag.has_value())
		{
			flag.emplace(Flags::HasFlag("zonebuilder"));
		}

		return (flag.value() && !Dedicated::IsEnabled());
	}

	void ZoneBuilder::BeginAssetTrace(const std::string& zone)
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

	Game::XAssetHeader ZoneBuilder::GetEmptyAssetIfCommon(Game::XAssetType type, const std::string& name, ZoneBuilder::Zone* builder)
	{
		Game::XAssetHeader header = { nullptr };

		if (type >= 0 && type < Game::XAssetType::ASSET_TYPE_COUNT)
		{
			int zoneIndex = Game::DB_GetZoneIndex("common_mp");

			if (zoneIndex > 0)
			{
				Game::XAssetEntry* entry = Game::DB_FindXAssetEntry(type, name.data());

				if (entry && entry->zoneIndex == zoneIndex)
				{
					// Allocate an empty asset (filled with zeros)
					header.data = builder->getAllocator()->allocate(Game::DB_GetXAssetSizeHandlers[type]());

					// Set the name to the original name, so it can be stored
					Game::DB_SetXAssetNameHandlers[type](&header, name.data());
					AssetHandler::StoreTemporaryAsset(type, header);

					// Set the name to the empty name
					Game::DB_SetXAssetNameHandlers[type](&header, builder->getAllocator()->duplicateString("," + name));
				}
			}
		}

		return header;
	}

	int ZoneBuilder::StoreTexture(Game::GfxImageLoadDef **loadDef, Game::GfxImage *image)
	{
		size_t size = 16 + (*loadDef)->resourceSize;
		void* data = Utils::Memory::GetAllocator()->allocate(size);
		std::memcpy(data, *loadDef, size);

		image->texture.loadDef = reinterpret_cast<Game::GfxImageLoadDef *>(data);

		return 0;
	}

	void ZoneBuilder::ReleaseTexture(Game::XAssetHeader header)
	{
		if (header.image && header.image->texture.loadDef)
		{
			Utils::Memory::GetAllocator()->free(header.image->texture.loadDef);
		}
	}

	void ZoneBuilder::AssumeMainThreadRole()
	{
		ZoneBuilder::MainThreadInterrupted = true;
		ZoneBuilder::InterruptingThreadId = GetCurrentThreadId();
	}

	void ZoneBuilder::ResetThreadRole()
	{
		ZoneBuilder::MainThreadInterrupted = false;
		ZoneBuilder::InterruptingThreadId = 0x0;
	}

	bool ZoneBuilder::IsThreadMainThreadHook()
	{
		// this is the thread that is interrupting so let it act as the main thread
		if (ZoneBuilder::MainThreadInterrupted && GetCurrentThreadId() == ZoneBuilder::InterruptingThreadId)
		{
			return true;
		}

		// block the main thread from doing anything "main thread" specific while
		// the other thread is interrupting
		
		//while (ZoneBuilder::mainThreadInterrupted) std::this_thread::sleep_for(100ms);

		// normal functionality
		return GetCurrentThreadId() == Utils::Hook::Get<DWORD>(0x1CDE7FC);
	}

	static Game::XZoneInfo baseZones_old[] = {
		{ "code_pre_gfx_mp", Game::DB_ZONE_CODE, 0 },
		{ "localized_code_pre_gfx_mp", Game::DB_ZONE_CODE_LOC, 0 },
		{ "code_post_gfx_mp", Game::DB_ZONE_CODE, 0 },
		{ "localized_code_post_gfx_mp", Game::DB_ZONE_CODE_LOC, 0 },
		{ "common_mp", Game::DB_ZONE_COMMON, 0 },
		{ "localized_common_mp", Game::DB_ZONE_COMMON_LOC, 0 },
		{ "ui_mp", Game::DB_ZONE_GAME, 0 },
		{ "localized_ui_mp", Game::DB_ZONE_GAME, 0 }
	};


	static Game::XZoneInfo baseZones[] = {
		{ "defaults", Game::DB_ZONE_CODE, 0 },
		{ "techsets",  Game::DB_ZONE_CODE, 0 },
		{ "common_mp",  Game::DB_ZONE_COMMON, 0 },
		{ "localized_common_mp",  Game::DB_ZONE_COMMON_LOC, 0 },
		{ "ui_mp",  Game::DB_ZONE_GAME, 0 },
		{ "localized_ui_mp",  Game::DB_ZONE_GAME, 0 }
	};

	void ZoneBuilder::Com_Quitf_t()
	{
		ExitProcess(0);
	}

	int APIENTRY ZoneBuilder::EntryPoint(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
	{
		Utils::Hook::Call<void()>(0x42F0A0)();	// Com_InitCriticalSections
		Utils::Hook::Call<void()>(0x4301B0)();  // Com_InitMainThread
		Utils::Hook::Call<void(int)>(0x406D10)(0);  // Win_InitLocalization
		Utils::Hook::Call<void()>(0x4FF220)();  // Com_InitParse
		Utils::Hook::Call<void()>(0x4D8220)();  // Dvar_Init
		Utils::Hook::Call<void()>(0x4D2280)();  // SL_Init
		Utils::Hook::Call<void()>(0x48F660)();  // Cmd_Init
		Utils::Hook::Call<void()>(0x4D9210)();  // Cbuf_Init
		Utils::Hook::Call<void()>(0x47F390)();  // Swap_Init
		Utils::Hook::Call<void()>(0x60AD10)();  // Com_InitDvars
		Utils::Hook::Call<void()>(0x420830)();  // Com_InitHunkMemory
		Utils::Hook::Call<void()>(0x4A62A0)();  // LargeLocalInit
		Utils::Hook::Call<void()>(0x4DCC10)();  // Sys_InitCmdEvents
		Utils::Hook::Call<void()>(0x64A020)();  // PMem_Init
		if (!Flags::HasFlag("stdout"))
		{
			Console::ShowAsyncConsole();
			Utils::Hook::Call<void()>(0x43D140)(); // Com_EventLoop
		}
		Utils::Hook::Call<void(unsigned int)>(0x502580)(static_cast<unsigned int>(__rdtsc())); // Netchan_Init
		Utils::Hook::Call<void()>(0x429080)();  // FS_InitFileSystem
		Utils::Hook::Call<void()>(0x4BFBE0)();  // Con_InitChannels
		Utils::Hook::Call<void()>(0x4E0FB0)();  // DB_InitThread
		Utils::Hook::Call<void()>(0x5196C0)();  // R_RegisterDvars
		Game::NET_Init();
		Utils::Hook::Call<void()>(0x4F5090)();  // SND_InitDriver
		Utils::Hook::Call<void()>(0x46A630)();  // SND_Init
		//Utils::Hook::Call<void()>(0x4D3660)();  // SV_Init
		//Utils::Hook::Call<void()>(0x4121E0)();  // SV_InitServerThread
		//Utils::Hook::Call<void()>(0x464A90)();  // Com_ParseCommandLine
		Utils::Hook::Call<void()>(0x43D140)(); // Com_EventLoop

		ZoneBuilder::CommandThread = std::thread([]
		{
			while (!ZoneBuilder::Terminate)
			{
				ZoneBuilder::AssumeMainThreadRole();
				Utils::Hook::Call<void(int, int)>(0x4E2C80)(0, 0); // Cbuf_Execute
				ZoneBuilder::ResetThreadRole();
				std::this_thread::sleep_for(1ms);
			}
		});

		Command::Add("quit", ZoneBuilder::Com_Quitf_t);

		// now load default assets and shaders
		if (FastFiles::Exists("defaults") && FastFiles::Exists("techsets"))
		{
			Game::DB_LoadXAssets(baseZones, ARRAYSIZE(baseZones), 0);
		}
		else
		{
			Logger::Warning(Game::CON_CHANNEL_DONT_FILTER,
				"Missing new init zones (defaults.ff & techsets.ff). You will need to load fastfiles to manually obtain techsets.\n");
			Game::DB_LoadXAssets(baseZones_old, ARRAYSIZE(baseZones_old), 0);
		}

		Logger::Print("Waiting for fastiles to load...\n");
		while (!Game::Sys_IsDatabaseReady())
		{
			Utils::Hook::Call<void()>(0x43D140)(); // Com_EventLoop
			std::this_thread::sleep_for(100ms);
		}
		Logger::Print("Done!\n");

		// defaults need to load before we do this
		Utils::Hook::Call<void()>(0x4E1F30)();  // G_SetupWeaponDef
		Utils::Hook::Call<void()>(0x4454C0)();  // Item_SetupKeywordHash (for loading menus)
		Utils::Hook::Call<void()>(0x501BC0)();  // Menu_SetupKeywordHash (for loading menus)
		Utils::Hook::Call<void()>(0x4A1280)();  // something related to uiInfoArray
		

		Utils::Hook::Call<void(const char*)>(0x464A90)(GetCommandLineA()); // Com_ParseCommandLine
		Utils::Hook::Call<void()>(0x60C3D0)(); // Com_AddStartupCommands

		// so for this i'm going to say that we just run the commands (after + signs) 
		// and don't spawn into a shell because why not?
		if (Flags::HasFlag("stdout"))
		{
			Utils::Hook::Call<void()>(0x440EC0)(); // DB_Update
			Utils::Hook::Call<void()>(0x43D140)(); // Com_EventLoop
			Utils::Hook::Call<void(int, int)>(0x4E2C80)(0, 0); // Cbuf_Execute

			return 0;
		}

		Logger::Print(" --------------------------------------------------------------------------------\n");
		Logger::Print(" IW4x ZoneBuilder (" VERSION ")\n");
		Logger::Print(" Commands:\n");
		Logger::Print("\t-buildzone [zone]: builds a zone from a csv located in zone_source\n");
		Logger::Print("\t-buildall: builds all zones in zone_source\n");
		Logger::Print("\t-verifyzone [zone]: loads and verifies the specified zone\n");
		Logger::Print("\t-listassets [assettype]: lists all loaded assets of the specified type\n");
		Logger::Print("\t-quit: quits the program\n");
		Logger::Print(" --------------------------------------------------------------------------------\n");

		// now run main loop until quit
		unsigned int frames = 0;
		while (true)
		{
			if (frames % 100 == 0)
			{
				Utils::Hook::Call<void()>(0x440EC0)(); // DB_Update
				Utils::Hook::Call<void()>(0x43EBB0)(); // check for quit
			}

			Utils::Hook::Call<void()>(0x43D140)(); // Com_EventLoop
			std::this_thread::sleep_for(1ms);
			frames++;
		}

		return 0;
	}

	void ZoneBuilder::HandleError(Game::errorParm_t code, const char* fmt, ...)
	{
		char buffer[0x1000]{};
		va_list ap;

		va_start(ap, fmt);
		vsnprintf_s(buffer, _TRUNCATE, fmt, ap);
		va_end(ap);

		if (!Flags::HasFlag("stdout"))
		{
			MessageBoxA(nullptr, buffer, "Error!", MB_OK | MB_ICONERROR);
		}
		else
		{
			perror(buffer);
			fflush(stderr);
		}

		if (code == Game::ERR_FATAL)
		{
			ExitProcess(1);
		}	
	}

	__declspec(naked) void ZoneBuilder::SoftErrorAssetOverflow()
	{
		// fuck with the stack to return to before DB_AddXAsset
		__asm
		{
			add esp, 12 // exit from DB_AllocXAssetEntry
			pop edi
			pop esi
			pop ebp
			pop ebx
			add esp, 14h
			mov eax, [esp + 8]
			retn
		}
	}

	std::string ZoneBuilder::FindMaterialByTechnique(const std::string& techniqueName)
	{
		static bool replacementFound = false;
		replacementFound = false; // the above one only runs the first time

		const char* ret = "default";

		Game::DB_EnumXAssetEntries(Game::XAssetType::ASSET_TYPE_MATERIAL, [techniqueName, &ret](Game::XAssetEntry* entry)
		{
			if (!replacementFound)
			{
				Game::XAssetHeader header = entry->asset.header;
				std::string name = techniqueName;
				if (name[0] == ',') name = name.substr(1);
				if (name == header.material->techniqueSet->name)
				{
					ret = header.material->info.name;
					replacementFound = true;
				}
			}
		}, false);

		if (replacementFound) return ret;
		return "";
	}

	void ZoneBuilder::ReallocateLoadedSounds(void*& data, [[maybe_unused]] void* a2)
	{
		assert(data);
		auto* sound = Utils::Hook::Get<Game::MssSound*>(0x112AE04);
		auto length = sound->info.data_len;
		auto allocatedSpace = Utils::Memory::AllocateArray<char>(length);
		memcpy_s(allocatedSpace, length, data, length);

		data = allocatedSpace;
		sound->data = allocatedSpace;
		sound->info.data_ptr = allocatedSpace;
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
			Utils::Hook(0x492EFC, ReallocateLoadedSounds, HOOK_CALL).install()->quick();

			// Don't display errors when assets are missing (we might manually build those)
			Utils::Hook::Nop(0x5BB3F2, 5);
			Utils::Hook::Nop(0x5BB422, 5);
			Utils::Hook::Nop(0x5BB43A, 5);

			// Don't read stats
			Utils::Hook(0x4875E1, 0x487717, HOOK_JUMP).install()->quick();

			// patch g_copyInfo because we're using so many more assets than originally intended
			int newLimit = 0x2000;
			int* g_copyInfo_new = Utils::Memory::GetAllocator()->allocateArray<int>(newLimit);
			Utils::Hook::Set<int*>(0x494083, g_copyInfo_new); // DB_DelayLoadImages
			Utils::Hook::Set<int*>(0x5BB9CC, g_copyInfo_new); // DB_AddXAsset
			Utils::Hook::Set<int*>(0x5BC723, g_copyInfo_new); // DB_PostLoadXZone
			Utils::Hook::Set<int*>(0x5BC759, g_copyInfo_new);
			Utils::Hook::Set<int>(0x5BB9AD, newLimit); // limit check

			// this one lets us keep loading zones and it will ignore assets when the pool is filled
			/*
			AssetHandler::OnLoad([](Game::XAssetType type, Game::XAssetHeader, const std::string&, bool* restrict)
			{
				//if (*static_cast<int*>(Game::DB_XAssetPool[type].data) == 0)
				if (Game::g_poolSize[type] == 0)
				{
					*restrict = true;
				}
			});
			*/

			// hunk size (was 300 MiB)
			Utils::Hook::Set<DWORD>(0x64A029, 0x38400000); // 900 MiB
			Utils::Hook::Set<DWORD>(0x64A057, 0x38400000);

			// change fs_game domain func
			Utils::Hook::Set<int(*)(Game::dvar_t*, Game::DvarValue)>(0x643203, [](Game::dvar_t* dvar, Game::DvarValue value)
			{
				int result = Utils::Hook::Call<int(Game::dvar_t*, Game::DvarValue)>(0x642FC0)(dvar, value);

				if (result)
				{
					if (std::string(value.string) != dvar->current.string)
					{
						dvar->current.string = value.string;
						Game::FS_Restart(0, 0);
					}
				}

				return result;
			});

			// set new entry point
			Utils::Hook(0x4513DA, ZoneBuilder::EntryPoint, HOOK_JUMP).install()->quick();

			// set quit handler
			Utils::Hook(0x4D4000, ZoneBuilder::Com_Quitf_t, HOOK_JUMP).install()->quick();

			// handle Com_error Calls
			Utils::Hook(Game::Com_Error, ZoneBuilder::HandleError, HOOK_JUMP).install()->quick();

			// thread fuckery hooks
			Utils::Hook(0x4C37D0, ZoneBuilder::IsThreadMainThreadHook, HOOK_JUMP).install()->quick();

			// Don't exec startup config in fs_restart
			Utils::Hook::Set<BYTE>(0x461B48, 0xEB);
			
			// remove overriding asset messages
			Utils::Hook::Nop(0x5BC74E, 5);

			// don't remap techsets
			Utils::Hook::Nop(0x5BC791, 5);

			AssetHandler::OnLoad([](Game::XAssetType type, Game::XAssetHeader /*asset*/, const std::string& name, bool* /*restrict*/)
			{
				if (!ZoneBuilder::TraceZone.empty() && ZoneBuilder::TraceZone == FastFiles::Current())
				{
					ZoneBuilder::TraceAssets.push_back({ type, name });
					OutputDebugStringA((name + "\n").data());
				}
			});

			Command::Add("verifyzone", [](Command::Params* params)
			{
				if (params->size() < 2) return;
				/*
				Utils::Hook(0x4AE9C2, []
				{
					Game::WeaponCompleteDef** varPtr = (Game::WeaponCompleteDef**)0x112A9F4;
					Game::WeaponCompleteDef* var = *varPtr;
					OutputDebugStringA("");
					Utils::Hook::Call<void()>(0x4D1D60)(); // DB_PopStreamPos
				}, HOOK_JUMP).install()->quick();

				Utils::Hook(0x4AE9B4, []
				{
					Game::WeaponCompleteDef** varPtr = (Game::WeaponCompleteDef**)0x112A9F4;
					Game::WeaponCompleteDef* var = *varPtr;
					OutputDebugStringA("");
					Utils::Hook::Call<void()>(0x4D1D60)(); // DB_PopStreamPos
				}, HOOK_JUMP).install()->quick();
				*/

				std::string zone = params->get(1);

				ZoneBuilder::BeginAssetTrace(zone);

				Game::XZoneInfo info;
				info.name = zone.data();
				info.allocFlags = Game::DB_ZONE_MOD;
				info.freeFlags = 0;

				Logger::Print("Loading zone '{}'...\n", zone);

				Game::DB_LoadXAssets(&info, 1, true);
				AssetHandler::FindOriginalAsset(Game::XAssetType::ASSET_TYPE_RAWFILE, zone.data()); // Lock until zone is loaded

				auto assets = ZoneBuilder::EndAssetTrace();

				Logger::Print("Unloading zone '{}'...\n", zone);
				info.freeFlags = Game::DB_ZONE_MOD;
				info.allocFlags = 0;
				info.name = nullptr;

				Game::DB_LoadXAssets(&info, 1, true);
				AssetHandler::FindOriginalAsset(Game::XAssetType::ASSET_TYPE_RAWFILE, "default"); // Lock until zone is unloaded

				Logger::Print("Zone '{}' loaded with {} assets:\n", zone, assets.size());

				int count = 0;
				for (auto i = assets.begin(); i != assets.end(); ++i, ++count)
				{
					Logger::Print(" {}: {}: {}\n", count, Game::DB_GetXAssetTypeName(i->first), i->second);
				}

				Logger::Print("\n");
			});

			Command::Add("buildzone", [](Command::Params* params)
			{
				if (params->size() < 2) return;

				std::string zoneName = params->get(1);
				Logger::Print("Building zone '{}'...\n", zoneName);

				Zone(zoneName).build();
			});

			Command::Add("buildall", []([[maybe_unused]] Command::Params* params)
			{
				auto path = std::format("{}\\zone_source", Dvar::Var("fs_basepath").get<std::string>());
				auto zoneSources = FileSystem::GetSysFileList(path, "csv", false);

				for (auto source : zoneSources)
				{
					if (Utils::String::EndsWith(source, ".csv"))
					{
						source = source.substr(0, source.find(".csv"));
					}

					Command::Execute(std::format("buildzone {}", source), true);
				}
			});

			static std::set<std::string> curTechsets_list;
			static std::set<std::string> techsets_list;

			AssetHandler::OnLoad([](Game::XAssetType type, Game::XAssetHeader, const std::string& name, bool*)
			{
				if (type == Game::ASSET_TYPE_TECHNIQUE_SET)
				{
					if (name[0] == ',') return; // skip techsets from common_mp
					if (techsets_list.find(name) == techsets_list.end())
					{
						curTechsets_list.emplace(name);
						techsets_list.emplace(name);
					}
				}
			});


			AssetHandler::OnLoad([](Game::XAssetType type, Game::XAssetHeader asset, [[maybe_unused]] const std::string& name, [[maybe_unused]] bool* restrict)
			{
				if (type != Game::ASSET_TYPE_SOUND)
				{
					return;
				}

				auto sound = asset.sound;

				for (size_t i = 0; i < sound->count; i++)
				{
					auto thisSound = sound->head[i];

					if (thisSound.soundFile->type == Game::SAT_LOADED)
					{
						if (thisSound.soundFile->u.loadSnd->sound.data == nullptr)
						{
							// ouch
							// This should never happen and will cause a memory leak
							// Let's change it to a streamed sound instead
							thisSound.soundFile->type = Game::SAT_STREAMED;

							auto virtualPath = std::filesystem::path(thisSound.soundFile->u.loadSnd->name);

							thisSound.soundFile->u.streamSnd.filename.info.raw.name = Utils::Memory::DuplicateString(virtualPath.filename().string());

							auto dir = virtualPath.remove_filename().string();
							dir = dir.substr(0, dir.size() - 1); // remove /
							thisSound.soundFile->u.streamSnd.filename.info.raw.dir = Utils::Memory::DuplicateString(dir);
						}
					}
				}
			});

			Command::Add("buildtechsets", [](Command::Params*)
			{
				Utils::IO::CreateDir("zone_source/techsets");
				Utils::IO::CreateDir("zone/techsets");

				std::string csvStr;

				auto fileList = Utils::IO::ListFiles(Utils::String::VA("zone/%s", Game::Win_GetLanguage()));
				for (auto zone : fileList)
				{
					Utils::String::Replace(zone, Utils::String::VA("zone/%s/", Game::Win_GetLanguage()), "");
					Utils::String::Replace(zone, ".ff", "");

					if (Utils::IO::FileExists("zone/techsets/" + zone + "_techsets.ff"))
					{
						Logger::Print("Skipping previously generated zone {}\n", zone);
						continue;
					}

					if (zone.find("_load") != std::string::npos)
					{
						Logger::Print("Skipping loadscreen zone {}\n", zone);
						continue;
					}

					if (Game::DB_IsZoneLoaded(zone.c_str()) || !FastFiles::Exists(zone))
					{
						continue;
					}

					if (zone[0] == '.') continue; // fucking mac dotfiles

					curTechsets_list.clear(); // clear from last run

					// load the zone
					Game::XZoneInfo info;
					info.name = zone.c_str();
					info.allocFlags = Game::DB_ZONE_MOD;
					info.freeFlags = 0x0;
					Game::DB_LoadXAssets(&info, 1, 0);

					while (!Game::Sys_IsDatabaseReady()) std::this_thread::sleep_for(100ms); // wait till its fully loaded

					if (curTechsets_list.size() == 0)
					{
						Logger::Print("Skipping empty zone {}\n", zone);
						// unload zone
						info.name = nullptr;
						info.allocFlags = 0x0;
						info.freeFlags = Game::DB_ZONE_MOD;
						Game::DB_LoadXAssets(&info, 1, true);
						continue;
					}

					// ok so we're just gonna use the materials because they will use the techsets
					csvStr.clear();
					for (auto tech : curTechsets_list)
					{
						std::string mat = ZoneBuilder::FindMaterialByTechnique(tech);
						if (mat.length() == 0) 
						{
							csvStr.append("techset," + tech + "\n");
						}
						else
						{
							csvStr.append("material," + mat + "\n");
						}
					}

					// save csv
					Utils::IO::WriteFile("zone_source/techsets/" + zone + "_techsets.csv", csvStr.data());

					// build the techset zone
					std::string zoneName = "techsets/" + zone + "_techsets";
					Logger::Print("Building zone '{}'...\n", zoneName);
					Zone(zoneName).build();

					// unload original zone
					info.name = nullptr;
					info.allocFlags = 0x0;
					info.freeFlags = Game::DB_ZONE_MOD;
					Game::DB_LoadXAssets(&info, 1, true);

					while (!Game::Sys_IsDatabaseReady()) std::this_thread::sleep_for(10ms); // wait till its fully loaded
				}

				curTechsets_list.clear();
				techsets_list.clear();

				Game::DB_EnumXAssets(Game::ASSET_TYPE_TECHNIQUE_SET, [](Game::XAssetHeader header, void*)
				{
					curTechsets_list.emplace(header.techniqueSet->name);
					techsets_list.emplace(header.techniqueSet->name);
				}, nullptr, false);

				// HACK: set language to 'techsets' to load from that dir
				const char* language = Utils::Hook::Get<const char*>(0x649E740);
				Utils::Hook::Set<const char*>(0x649E740, "techsets");

				// load generated techset fastfiles
				auto list = Utils::IO::ListFiles("zone/techsets");
				int i = 0;
				int subCount = 0;
				for (auto it : list)
				{
					Utils::String::Replace(it, "zone/techsets/", "");
					Utils::String::Replace(it, ".ff", "");

					if (it.find("_techsets") == std::string::npos) continue; // skip files we didn't generate for this

					if (!Game::DB_IsZoneLoaded(it.data()))
					{
						Game::XZoneInfo info;
						info.name = it.data();
						info.allocFlags = Game::DB_ZONE_MOD;
						info.freeFlags = 0;

						Game::DB_LoadXAssets(&info, 1, 0);
						while (!Game::Sys_IsDatabaseReady()) std::this_thread::sleep_for(10ms); // wait till its fully loaded
					}
					else
					{
						Logger::Print("Zone '{}' already loaded\n", it);
					}

					if (i == 20) // cap at 20 just to be safe
					{
						// create csv with the techsets in it
						csvStr.clear();
						for (auto tech : curTechsets_list)
						{
							std::string mat = ZoneBuilder::FindMaterialByTechnique(tech);
							if (mat.length() == 0)
							{
								csvStr.append("techset," + tech + "\n");
							}
							else
							{
								csvStr.append("material," + mat + "\n");
							}
						}

						std::string tempZoneFile = Utils::String::VA("zone_source/techsets/techsets%d.csv", subCount);
						std::string tempZone = Utils::String::VA("techsets/techsets%d", subCount);

						Utils::IO::WriteFile(tempZoneFile, csvStr.data());

						Logger::Print("Building zone '{}'...\n", tempZone);
						Zone(tempZone).build();

						// unload all zones
						Game::XZoneInfo info;
						info.name = nullptr;
						info.allocFlags = 0x0;
						info.freeFlags = Game::DB_ZONE_MOD;
						Game::DB_LoadXAssets(&info, 1, true);

						Utils::Hook::Set<const char*>(0x649E740, "techsets");

						i = 0;
						subCount++;
						curTechsets_list.clear();
						techsets_list.clear();
					}

					i++;
				}

				// last iteration
				if (i != 0)
				{
					// create csv with the techsets in it
					csvStr.clear();
					for (auto tech : curTechsets_list)
					{
						std::string mat = ZoneBuilder::FindMaterialByTechnique(tech);
						if (mat.length() == 0)
						{
							Logger::Print("Couldn't find a material for techset {}. Sort Keys will be incorrect.\n", tech);
							csvStr.append("techset," + tech + "\n");
						}
						else
						{
							csvStr.append("material," + mat + "\n");
						}
					}

					std::string tempZoneFile = Utils::String::VA("zone_source/techsets/techsets%d.csv", subCount);
					std::string tempZone = Utils::String::VA("techsets/techsets%d", subCount);

					Utils::IO::WriteFile(tempZoneFile, csvStr.data());

					Logger::Print("Building zone '{}'...\n", tempZone);
					Zone(tempZone).build();

					// unload all zones
					Game::XZoneInfo info;
					info.name = nullptr;
					info.allocFlags = 0x0;
					info.freeFlags = Game::DB_ZONE_MOD;
					Game::DB_LoadXAssets(&info, 1, true);

					subCount++;
				}

				// build final techsets fastfile
				if (subCount > 24)
				{
					Logger::Error(Game::ERR_DROP, "How did you have 576 fastfiles?\n");
				}

				curTechsets_list.clear();
				techsets_list.clear();

				for (int j = 0; j < subCount; ++j)
				{
					Game::XZoneInfo info;
					info.name = Utils::String::VA("techsets%d", j);
					info.allocFlags = Game::DB_ZONE_MOD;
					info.freeFlags = 0;

					Game::DB_LoadXAssets(&info, 1, 0);
					while (!Game::Sys_IsDatabaseReady()) std::this_thread::sleep_for(10ms); // wait till its fully loaded
				}

				// create csv with the techsets in it
				csvStr.clear();
				for (auto tech : curTechsets_list)
				{
					std::string mat = ZoneBuilder::FindMaterialByTechnique(tech);
					if (mat.length() == 0)
					{
						csvStr.append("techset," + tech + "\n");
					}
					else
					{
						csvStr.append("material," + mat + "\n");
					}
				}

				Utils::IO::WriteFile("zone_source/techsets/techsets.csv", csvStr.data());

				// set language back
				Utils::Hook::Set<const char*>(0x649E740, language);

				Logger::Print("Building zone 'techsets/techsets'...\n");
				Zone("techsets/techsets").build();
			});

			Command::Add("listassets", [](Command::Params* params)
			{
				if (params->size() < 2) return;
				Game::XAssetType type = Game::DB_GetXAssetNameType(params->get(1));

				if (type != Game::XAssetType::ASSET_TYPE_INVALID)
				{
					Game::DB_EnumXAssets(type, [](Game::XAssetHeader header, void* data)
					{
						Game::XAsset asset = { *reinterpret_cast<Game::XAssetType*>(data), header };
						Logger::Print("{}\n", Game::DB_GetXAssetName(&asset));
					}, &type, false);
				}
			});

			Command::Add("loadtempzone", [](Command::Params* params)
			{
				if (params->size() < 2) return;

				if (FastFiles::Exists(params->get(1)))
				{
					Game::XZoneInfo info;
					info.name = params->get(1);
					info.allocFlags = 0x80;
					info.freeFlags = 0x0;
					Game::DB_LoadXAssets(&info, 1, 0);
				}
			});

			Command::Add("unloadtempzones", [](Command::Params*)
			{
				Game::XZoneInfo info;
				info.name = nullptr;
				info.allocFlags = 0x0;
				info.freeFlags = 0x80;
				Game::DB_LoadXAssets(&info, 1, true);
				AssetHandler::FindOriginalAsset(Game::XAssetType::ASSET_TYPE_RAWFILE, "default"); // Lock until zone is unloaded
			});

			Command::Add("materialInfoDump", [](Command::Params*)
			{
				Game::DB_EnumXAssets(Game::ASSET_TYPE_MATERIAL, [](Game::XAssetHeader header, void*)
				{
					Logger::Print("{}: {:#X} {:#X} {:#X}\n",
						header.material->info.name, header.material->info.sortKey & 0xFF, header.material->info.gameFlags & 0xFF, header.material->stateFlags & 0xFF);
				}, nullptr, false);
			});

			Command::Add("iwiDump", [](Command::Params* params)
			{
				if (params->size() < 2) return;

				auto path = std::format("{}\\mods\\{}\\images", Dvar::Var("fs_basepath").get<std::string>(), params->get(1));
				auto images = FileSystem::GetSysFileList(path, "iwi", false);

				for (auto i = images.begin(); i != images.end();)
				{
					*i = std::format("images/{}", *i);

					if (FileSystem::File(*i).exists())
					{
						i = images.erase(i);
						continue;
					}

					++i;
				}

				Logger::Print("------------------- BEGIN IWI DUMP -------------------\n");
				Logger::Print("{}\n", nlohmann::json(images).dump());
				Logger::Print("------------------- END IWI DUMP -------------------\n");
			});

			// True by default, but can be put to zero for backward compatibility if needed
			ZoneBuilder::PreferDiskAssetsDvar = Dvar::Register<bool>("zb_prefer_disk_assets", true, Game::DVAR_NONE, "Should zonebuilder prefer in-memory assets (requirements) or disk assets, when both are present?");
		}
	}

	ZoneBuilder::~ZoneBuilder()
	{
		ZoneBuilder::Terminate = true;
		if (ZoneBuilder::CommandThread.joinable())
		{
			ZoneBuilder::CommandThread.join();
		}
	}

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
	bool ZoneBuilder::unitTest()
	{
		printf("Testing circular bit shifting (left)...");

		unsigned int integer = 0x80000000;
		Utils::RotLeft(integer, 1);

		if (integer != 1)
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
