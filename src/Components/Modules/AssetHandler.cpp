#include "STDInclude.hpp"

namespace Components
{
	thread_local bool AssetHandler::BypassState;
	std::map<Game::XAssetType, AssetHandler::IAsset*> AssetHandler::AssetInterfaces;
	std::map<Game::XAssetType, Utils::Slot<AssetHandler::Callback>> AssetHandler::TypeCallbacks;
	Utils::Signal<AssetHandler::RestrictCallback> AssetHandler::RestrictSignal;

	std::map<void*, void*> AssetHandler::Relocations;

	std::vector<std::pair<Game::XAssetType, std::string>> AssetHandler::EmptyAssets;

	std::map<std::string, Game::XAssetHeader> AssetHandler::TemporaryAssets[Game::XAssetType::ASSET_TYPE_COUNT];

	void AssetHandler::RegisterInterface(IAsset* iAsset)
	{
		if (!iAsset) return;
		if (iAsset->getType() == Game::XAssetType::ASSET_TYPE_INVALID)
		{
			delete iAsset;
			return;
		}

		if (AssetHandler::AssetInterfaces.find(iAsset->getType()) != AssetHandler::AssetInterfaces.end())
		{
			Logger::Print("Duplicate asset interface: %s\n", Game::DB_GetXAssetTypeName(iAsset->getType()));
			delete AssetHandler::AssetInterfaces[iAsset->getType()];
		}
		else
		{
			Logger::Print("Asset interface registered: %s\n", Game::DB_GetXAssetTypeName(iAsset->getType()));
		}

		AssetHandler::AssetInterfaces[iAsset->getType()] = iAsset;
	}

	void AssetHandler::ClearTemporaryAssets()
	{
		for (int i = 0; i < Game::XAssetType::ASSET_TYPE_COUNT; ++i)
		{
			AssetHandler::TemporaryAssets[i].clear();
		}
	}

	void AssetHandler::StoreTemporaryAsset(Game::XAssetType type, Game::XAssetHeader asset)
	{
		AssetHandler::TemporaryAssets[type][Game::DB_GetXAssetNameHandlers[type](&asset)] = asset;
	}

	Game::XAssetHeader AssetHandler::FindAsset(Game::XAssetType type, const char* filename)
	{
		Game::XAssetHeader header = { 0 };

		if (filename)
		{
			// Allow call DB_FindXAssetHeader within the hook
			AssetHandler::BypassState = true;

			if (AssetHandler::TypeCallbacks.find(type) != AssetHandler::TypeCallbacks.end())
			{
				header = AssetHandler::TypeCallbacks[type](type, filename);
			}

			// Disallow calling DB_FindXAssetHeader ;)
			AssetHandler::BypassState = false;
		}

		return header;
	}

	int AssetHandler::HasThreadBypass()
	{
		return AssetHandler::BypassState & 1;
	}

	__declspec(naked) void AssetHandler::FindAssetStub()
	{
		__asm
		{
			push ecx
			push ebx
			push ebp
			push esi
			push edi

			// Check if custom handler should be bypassed
			call AssetHandler::HasThreadBypass

			test al, al
			jnz finishOriginal

			mov ecx, [esp + 18h] // Asset type
			mov ebx, [esp + 1Ch] // Filename

			push ebx
			push ecx

			call AssetHandler::FindAsset

			add esp, 8h

			test eax, eax
			jnz finishFound

		finishOriginal:
			// Asset not found using custom handlers, redirect to DB_FindXAssetHeader
			mov ebx, ds:6D7190h // InterlockedDecrement
			mov eax, 40793Bh
			jmp eax

		finishFound:
			pop edi
			pop esi
			pop ebp
			pop ebx
			pop ecx
			retn
		}
	}

	void AssetHandler::ModifyAsset(Game::XAssetType type, Game::XAssetHeader asset, std::string name)
	{
		if (type == Game::XAssetType::ASSET_TYPE_MATERIAL && (name == "wc/codo_ui_viewer_black_decal3" || name == "wc/codo_ui_viewer_black_decal2" || name == "wc/hint_arrows01" || name == "wc/hint_arrows02"))
		{
			asset.material->sortKey = 0xE;
		}

		if (type == Game::XAssetType::ASSET_TYPE_VEHICLE && Zones::Version() >= VERSION_ALPHA2)
		{
			asset.vehicle->weaponDef = nullptr;
		}

		// Fix shader const stuff
		if (type == Game::XAssetType::ASSET_TYPE_TECHNIQUE_SET && Zones::Version() >= 359)
		{
			for (int i = 0; i < 48; ++i)
			{
				if (asset.techniqueSet->techniques[i])
				{
					for (int j = 0; j < asset.techniqueSet->techniques[i]->numPasses; ++j)
					{
						Game::MaterialPass* pass = &asset.techniqueSet->techniques[i]->passes[j];

						for (int k = 0; k < (pass->argCount1 + pass->argCount2 + pass->argCount3); ++k)
						{
							if (pass->argumentDef[k].type == D3DSHADER_PARAM_REGISTER_TYPE::D3DSPR_CONSTINT)
							{
								if (pass->argumentDef[k].paramID == -28132)
								{
									pass->argumentDef[k].paramID = 2644;
								}
							}
						}
					}
				}
			}
		}
	}

	bool AssetHandler::IsAssetEligible(Game::XAssetType type, Game::XAssetHeader *asset)
	{
		const char* name = Game::DB_GetXAssetNameHandlers[type](asset);
		if (!name) return false;

		for (auto i = AssetHandler::EmptyAssets.begin(); i != AssetHandler::EmptyAssets.end();)
		{
			if (i->first == type && i->second == name)
			{
				i = AssetHandler::EmptyAssets.erase(i);
			}
			else
			{
				++i;
			}
		}

		if (Flags::HasFlag("entries"))
		{
			OutputDebugStringA(Utils::String::VA("%s: %d: %s\n", FastFiles::Current().data(), type, name));
		}

		bool restrict = false;
		AssetHandler::RestrictSignal(type, *asset, name, &restrict);

		if (!restrict)
		{
			AssetHandler::ModifyAsset(type, *asset, name);
		}

		// If no slot restricts the loading, we can load the asset
		return (!restrict);
	}

	__declspec(naked) void AssetHandler::AddAssetStub()
	{
		__asm
		{
			push [esp + 8]
			push [esp + 8]
			call AssetHandler::IsAssetEligible
			add esp, 08h

			test al, al
			jz doNotLoad

			mov eax, [esp + 8]
			sub esp, 14h
			mov ecx, 5BB657h
			jmp ecx

		doNotLoad:
			mov eax, [esp + 8]
			retn
		}
	}

	void AssetHandler::OnFind(Game::XAssetType type, Utils::Slot<AssetHandler::Callback> callback)
	{
		AssetHandler::TypeCallbacks[type] = callback;
	}

	void AssetHandler::OnLoad(Utils::Slot<AssetHandler::RestrictCallback> callback)
	{
		AssetHandler::RestrictSignal.connect(callback);
	}

	void AssetHandler::ClearRelocations()
	{
		AssetHandler::Relocations.clear();
	}

	void AssetHandler::Relocate(void* start, void* to, DWORD size)
	{
		for (DWORD i = 0; i < size; i += 4)
		{
			AssetHandler::Relocations[reinterpret_cast<char*>(start) + i] = reinterpret_cast<char*>(to) + i;
		}
	}

	void AssetHandler::OffsetToAlias(Utils::Stream::Offset* offset)
	{
		void* pointer = (*Game::g_streamBlocks)[offset->getUnpackedBlock()].data + offset->getUnpackedOffset();

		if (AssetHandler::Relocations.find(pointer) != AssetHandler::Relocations.end())
		{
			pointer = AssetHandler::Relocations[pointer];
		}

		offset->pointer = *reinterpret_cast<void**>(pointer);
 	}

	void AssetHandler::ZoneSave(Game::XAsset asset, ZoneBuilder::Zone* builder)
	{
		if (AssetHandler::AssetInterfaces.find(asset.type) != AssetHandler::AssetInterfaces.end())
		{
			AssetHandler::AssetInterfaces[asset.type]->save(asset.header, builder);
		}
		else
		{
			Logger::Error("No interface for type '%s'!", Game::DB_GetXAssetTypeName(asset.type));
		}
	}

	void AssetHandler::ZoneMark(Game::XAsset asset, ZoneBuilder::Zone* builder)
	{
		if (AssetHandler::AssetInterfaces.find(asset.type) != AssetHandler::AssetInterfaces.end())
		{
			AssetHandler::AssetInterfaces[asset.type]->mark(asset.header, builder);
		}
		else
		{
			Logger::Error("No interface for type '%s'!", Game::DB_GetXAssetTypeName(asset.type));
		}
	}

	Game::XAssetHeader AssetHandler::FindAssetForZone(Game::XAssetType type, std::string filename, ZoneBuilder::Zone* builder, bool isSubAsset)
	{
		Game::XAssetHeader header = { 0 };
		if (type >= Game::XAssetType::ASSET_TYPE_COUNT) return header;

		auto tempPool = &AssetHandler::TemporaryAssets[type];
		auto entry = tempPool->find(filename);
		if (entry != tempPool->end())
		{
			return { entry->second };
		}

		if (AssetHandler::AssetInterfaces.find(type) != AssetHandler::AssetInterfaces.end())
		{
			AssetHandler::AssetInterfaces[type]->load(&header, filename, builder);

			if (header.data)
			{
				Components::AssetHandler::StoreTemporaryAsset(type, header);
			}
		}

		if (!header.data && isSubAsset)
		{
			header = ZoneBuilder::GetEmptyAssetIfCommon(type, filename, builder);
		}

		if (!header.data)
		{
			header = Game::DB_FindXAssetHeader(type, filename.data());
			if(header.data) Components::AssetHandler::StoreTemporaryAsset(type, header); // Might increase efficiency...
		}

		return header;
	}

	Game::XAssetHeader AssetHandler::FindOriginalAsset(Game::XAssetType type, const char* filename)
	{
		int originalState = AssetHandler::HasThreadBypass();

		AssetHandler::BypassState = true;
		Game::XAssetHeader header = Game::DB_FindXAssetHeader(type, filename);
		if (!originalState) AssetHandler::BypassState = false;

		return header;
	}

	void AssetHandler::StoreEmptyAsset(Game::XAssetType type, const char* name)
	{
		AssetHandler::EmptyAssets.push_back({ type, name });
	}

	__declspec(naked) void AssetHandler::StoreEmptyAssetStub()
	{
		__asm
		{
			pushad
			push ebx
			push eax

			call AssetHandler::StoreEmptyAsset

			pop eax
			pop ebx
			popad

			push 5BB290h
			retn
		}
	}

	AssetHandler::AssetHandler()
	{
		Dvar::Register<bool>("r_noVoid", false, Game::DVAR_FLAG_SAVED, "Disable void model (red fx)");

		AssetHandler::ClearTemporaryAssets();

		// DB_FindXAssetHeader
		Utils::Hook(Game::DB_FindXAssetHeader, AssetHandler::FindAssetStub).install()->quick();

		// DB_ConvertOffsetToAlias
		Utils::Hook(0x4FDFA0, AssetHandler::OffsetToAlias, HOOK_JUMP).install()->quick();

		// DB_AddXAsset
		Utils::Hook(0x5BB650, AssetHandler::AddAssetStub, HOOK_JUMP).install()->quick();

		// Store empty assets
		Utils::Hook(0x5BB6EC, AssetHandler::StoreEmptyAssetStub, HOOK_CALL).install()->quick();

		// Log missing empty assets
		QuickPatch::OnFrame([] ()
		{
			if (FastFiles::Ready() && !AssetHandler::EmptyAssets.empty())
			{
				for (auto& asset : AssetHandler::EmptyAssets)
				{
					Game::Sys_Error(25, reinterpret_cast<char*>(0x724428), Game::DB_GetXAssetTypeName(asset.first), asset.second.data());
				}

				AssetHandler::EmptyAssets.clear();
			}
		});

		AssetHandler::OnLoad([] (Game::XAssetType type, Game::XAssetHeader asset, std::string name, bool*)
		{
			if (Dvar::Var("r_noVoid").get<bool>() && type == Game::XAssetType::ASSET_TYPE_XMODEL && name == "void")
			{
				asset.model->numLods = 0;
			}
		});

		// Register asset interfaces
		if (ZoneBuilder::IsEnabled())
		{
			AssetHandler::RegisterInterface(new Assets::IXModel());
			AssetHandler::RegisterInterface(new Assets::IFxWorld());
			AssetHandler::RegisterInterface(new Assets::IMapEnts());
			AssetHandler::RegisterInterface(new Assets::IRawFile());
			AssetHandler::RegisterInterface(new Assets::IComWorld());
			AssetHandler::RegisterInterface(new Assets::IGfxImage());
#ifdef ENABLE_EXPERIMENTAL_MAP_CODE
			AssetHandler::RegisterInterface(new Assets::IGfxWorld());
#endif
			AssetHandler::RegisterInterface(new Assets::ISndCurve());
			AssetHandler::RegisterInterface(new Assets::IMaterial());
#ifdef ENABLE_EXPERIMENTAL_MAP_CODE
			AssetHandler::RegisterInterface(new Assets::IclipMap_t());
#endif
			AssetHandler::RegisterInterface(new Assets::IPhysPreset());
			AssetHandler::RegisterInterface(new Assets::IXAnimParts());
			AssetHandler::RegisterInterface(new Assets::IFxEffectDef());
			AssetHandler::RegisterInterface(new Assets::IGameWorldMp());
			AssetHandler::RegisterInterface(new Assets::IGameWorldSp());
			AssetHandler::RegisterInterface(new Assets::IGfxLightDef());
			AssetHandler::RegisterInterface(new Assets::ILoadedSound());
			AssetHandler::RegisterInterface(new Assets::IPhysCollmap());
			AssetHandler::RegisterInterface(new Assets::IStringTable());
			AssetHandler::RegisterInterface(new Assets::IXModelSurfs());
			AssetHandler::RegisterInterface(new Assets::ILocalizeEntry());
			AssetHandler::RegisterInterface(new Assets::Isnd_alias_list_t());
			AssetHandler::RegisterInterface(new Assets::IMaterialPixelShader());
			AssetHandler::RegisterInterface(new Assets::IMaterialTechniqueSet());
			AssetHandler::RegisterInterface(new Assets::IMaterialVertexShader());
			AssetHandler::RegisterInterface(new Assets::IStructuredDataDefSet());
			AssetHandler::RegisterInterface(new Assets::IMaterialVertexDeclaration());
		}
	}

	AssetHandler::~AssetHandler()
	{
		AssetHandler::ClearTemporaryAssets();

		for (auto i = AssetHandler::AssetInterfaces.begin(); i != AssetHandler::AssetInterfaces.end(); ++i)
		{
			delete i->second;
		}

		AssetHandler::Relocations.clear();
		AssetHandler::AssetInterfaces.clear();
		AssetHandler::RestrictSignal.clear();
		AssetHandler::TypeCallbacks.clear();
	}
}
