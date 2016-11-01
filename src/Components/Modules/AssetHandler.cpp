#include "STDInclude.hpp"

namespace Components
{
	bool AssetHandler::BypassState = false;
	std::map<Game::XAssetType, AssetHandler::IAsset*> AssetHandler::AssetInterfaces;
	std::map<Game::XAssetType, wink::slot<AssetHandler::Callback>> AssetHandler::TypeCallbacks;
	wink::signal<wink::slot<AssetHandler::RestrictCallback>> AssetHandler::RestrictSignal;

	std::map<void*, void*> AssetHandler::Relocations;

	std::vector<std::pair<Game::XAssetType, std::string>> AssetHandler::EmptyAssets;

	std::map<std::string, Game::XAssetHeader> AssetHandler::TemporaryAssets[Game::XAssetType::ASSET_TYPE_COUNT];

	void AssetHandler::RegisterInterface(IAsset* iAsset)
	{
		if (!iAsset) return;
		if (iAsset->GetType() == Game::XAssetType::ASSET_TYPE_INVALID)
		{
			delete iAsset;
			return;
		}

		if (AssetHandler::AssetInterfaces.find(iAsset->GetType()) != AssetHandler::AssetInterfaces.end())
		{
			Logger::Print("Duplicate asset interface: %s\n", Game::DB_GetXAssetTypeName(iAsset->GetType()));
			delete AssetHandler::AssetInterfaces[iAsset->GetType()];
		}
		else
		{
			Logger::Print("Asset interface registered: %s\n", Game::DB_GetXAssetTypeName(iAsset->GetType()));
		}

		AssetHandler::AssetInterfaces[iAsset->GetType()] = iAsset;
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
			xor eax, eax
			mov al, AssetHandler::BypassState

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
		if (type == Game::XAssetType::ASSET_TYPE_MATERIAL && (name == "wc/codo_ui_viewer_black_decal3" || name == "wc/codo_ui_viewer_black_decal2" || name == "wc/hint_arrows02"))
		{
			asset.material->sortKey = 0xE;
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

	void AssetHandler::OnFind(Game::XAssetType type, AssetHandler::Callback* callback)
	{
		AssetHandler::TypeCallbacks[type] = callback;
	}

	void AssetHandler::OnLoad(AssetHandler::RestrictCallback* callback)
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
		// Same here, reinterpret the value, as we're operating inside the game's environment
		void* pointer = (*Game::g_streamBlocks)[offset->GetUnpackedBlock()].data + offset->GetUnpackedOffset();

		if (AssetHandler::Relocations.find(pointer) != AssetHandler::Relocations.end())
		{
			pointer = AssetHandler::Relocations[pointer];
		}

		offset->pointer = *reinterpret_cast<void**>(pointer);

#ifdef DEBUG
		Game::XAssetHeader zob{ offset->pointer };
#endif
 	}

	void AssetHandler::ZoneSave(Game::XAsset asset, ZoneBuilder::Zone* builder)
	{
		if (AssetHandler::AssetInterfaces.find(asset.type) != AssetHandler::AssetInterfaces.end())
		{
			AssetHandler::AssetInterfaces[asset.type]->Save(asset.header, builder);
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
			AssetHandler::AssetInterfaces[asset.type]->Mark(asset.header, builder);
		}
		else
		{
			Logger::Error("No interface for type '%s'!", Game::DB_GetXAssetTypeName(asset.type));
		}
	}

	Game::XAssetHeader AssetHandler::FindAssetForZone(Game::XAssetType type, std::string filename, ZoneBuilder::Zone* builder)
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
			AssetHandler::AssetInterfaces[type]->Load(&header, filename, builder);

			if (header.data)
			{
				Components::AssetHandler::StoreTemporaryAsset(type, header);
			}
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
		bool OriginalState = AssetHandler::BypassState;

		AssetHandler::BypassState = true;
		Game::XAssetHeader header = Game::DB_FindXAssetHeader(type, filename);
		if(!OriginalState) AssetHandler::BypassState = false;

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
		Utils::Hook(Game::DB_FindXAssetHeader, AssetHandler::FindAssetStub).Install()->Quick();

		// DB_ConvertOffsetToAlias
		Utils::Hook(0x4FDFA0, AssetHandler::OffsetToAlias, HOOK_JUMP).Install()->Quick();

		// DB_AddXAsset
		Utils::Hook(0x5BB650, AssetHandler::AddAssetStub, HOOK_JUMP).Install()->Quick();

		// Store empty assets
		Utils::Hook(0x5BB6EC, AssetHandler::StoreEmptyAssetStub, HOOK_CALL).Install()->Quick();

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
			if (Dvar::Var("r_noVoid").Get<bool>() && type == Game::XAssetType::ASSET_TYPE_XMODEL && name == "void")
			{
				asset.model->numLods = 0;
			}
		});

		// Register asset interfaces
		if (ZoneBuilder::IsEnabled())
		{
			AssetHandler::RegisterInterface(new Assets::IXModel());
			AssetHandler::RegisterInterface(new Assets::IMapEnts());
			AssetHandler::RegisterInterface(new Assets::IRawFile());
			AssetHandler::RegisterInterface(new Assets::IGfxImage());
			AssetHandler::RegisterInterface(new Assets::IMaterial());
			AssetHandler::RegisterInterface(new Assets::IPhysPreset());
			AssetHandler::RegisterInterface(new Assets::IXAnimParts());
			AssetHandler::RegisterInterface(new Assets::IPhysCollmap());
			AssetHandler::RegisterInterface(new Assets::IStringTable());
			//AssetHandler::RegisterInterface(new Assets::IXModelSurfs());
			AssetHandler::RegisterInterface(new Assets::ILocalizedEntry());
			AssetHandler::RegisterInterface(new Assets::IMaterialPixelShader());
			AssetHandler::RegisterInterface(new Assets::IMaterialTechniqueSet());
			AssetHandler::RegisterInterface(new Assets::IMaterialVertexShader());
			AssetHandler::RegisterInterface(new Assets::IStructuredDataDefSet());
			AssetHandler::RegisterInterface(new Assets::IMaterialVertexDeclaration());
		}
	}

	AssetHandler::~AssetHandler()
	{
		ClearTemporaryAssets();

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
