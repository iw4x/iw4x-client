#include "STDInclude.hpp"

namespace Components
{
	bool AssetHandler::BypassState = false;
	std::map<Game::XAssetType, AssetHandler::IAsset*> AssetHandler::AssetInterfaces;
	std::map<Game::XAssetType, AssetHandler::Callback> AssetHandler::TypeCallbacks;
	std::vector<AssetHandler::RestrictCallback> AssetHandler::RestrictCallbacks;

	std::map<void*, void*> AssetHandler::Relocations;

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
			delete AssetHandler::AssetInterfaces[iAsset->GetType()];
		}

		AssetHandler::AssetInterfaces[iAsset->GetType()] = iAsset;
	}

	void AssetHandler::ClearTemporaryAssets()
	{
		for (int i = 0; i < Game::XAssetType::ASSET_TYPE_COUNT; i++)
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

		// Allow call DB_FindXAssetHeader within the hook
		AssetHandler::BypassState = true;

		if (AssetHandler::TypeCallbacks.find(type) != AssetHandler::TypeCallbacks.end())
		{
			header = AssetHandler::TypeCallbacks[type](type, filename);
		}

		// Disallow calling DB_FindXAssetHeader ;)
		AssetHandler::BypassState = false;

		return header;
	}

	void __declspec(naked) AssetHandler::FindAssetStub()
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

	bool AssetHandler::IsAssetEligible(Game::XAssetType type, Game::XAssetHeader *asset)
	{
		const char* name = Game::DB_GetXAssetNameHandlers[type](asset);
		if (!name) return false;

		for (auto callback : AssetHandler::RestrictCallbacks)
		{
			if (!callback(type, *asset, name))
			{
				return false;
			}
		}

		return true;
	}

	void __declspec(naked) AssetHandler::AddAssetStub()
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

	void AssetHandler::OnFind(Game::XAssetType type, AssetHandler::Callback callback)
	{
		AssetHandler::TypeCallbacks[type] = callback;
	}

	void AssetHandler::OnLoad(RestrictCallback callback)
	{
		AssetHandler::RestrictCallbacks.push_back(callback);
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
		offset->pointer = *reinterpret_cast<void**>((*Game::g_streamBlocks)[offset->GetUnpackedBlock()].data + offset->GetUnpackedOffset());

		if (AssetHandler::Relocations.find(offset->pointer) != AssetHandler::Relocations.end())
		{
			offset->pointer = AssetHandler::Relocations[offset->pointer];
		}
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
		}

		if (!header.data)
		{
			header = Game::DB_FindXAssetHeader(type, filename.data());
		}

		return header;
	}

	Game::XAssetHeader AssetHandler::FindOriginalAsset(Game::XAssetType type, const char* filename)
	{
		Game::XAssetHeader header = { 0 };

		bool OriginalState = AssetHandler::BypassState;

		AssetHandler::BypassState = true;
		header = Game::DB_FindXAssetHeader(type, filename);
		if(!OriginalState) AssetHandler::BypassState = false;

		return header;
	}

	AssetHandler::AssetHandler()
	{
		AssetHandler::ClearTemporaryAssets();

		// DB_FindXAssetHeader
		Utils::Hook(Game::DB_FindXAssetHeader, AssetHandler::FindAssetStub).Install()->Quick();

		// DB_ConvertOffsetToAlias
		Utils::Hook(0x4FDFA0, AssetHandler::OffsetToAlias, HOOK_JUMP).Install()->Quick();

		// DB_AddXAsset
		Utils::Hook(0x5BB650, AssetHandler::AddAssetStub, HOOK_JUMP).Install()->Quick();

		// Register asset interfaces
		AssetHandler::RegisterInterface(new Assets::IRawFile());
		AssetHandler::RegisterInterface(new Assets::IGfxImage());
		AssetHandler::RegisterInterface(new Assets::IMaterial());
		AssetHandler::RegisterInterface(new Assets::ILocalizedEntry());
		AssetHandler::RegisterInterface(new Assets::IMaterialPixelShader());
		AssetHandler::RegisterInterface(new Assets::IMaterialTechniqueSet());
		AssetHandler::RegisterInterface(new Assets::IMaterialVertexShader());
		AssetHandler::RegisterInterface(new Assets::IMaterialVertexDeclaration());
	}

	AssetHandler::~AssetHandler()
	{
		ClearTemporaryAssets();

		for (auto i = AssetHandler::AssetInterfaces.begin(); i != AssetHandler::AssetInterfaces.end(); i++)
		{
			delete i->second;
		}

		AssetHandler::AssetInterfaces.clear();
		AssetHandler::TypeCallbacks.clear();
	}
}
