#include "..\STDInclude.hpp"

namespace Components
{
	bool AssetHandler::BypassState = false;
	std::map<Game::XAssetType, AssetHandler::Callback> AssetHandler::TypeCallbacks;
	std::vector<AssetHandler::RestrictCallback> AssetHandler::RestrictCallbacks;

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

// 			push [esp + 8]
// 			push [esp + 8]
// 			call DoBeforeLoadAsset
// 			add esp, 08h

			mov eax, [esp + 8]
			sub esp, 14h
			mov ecx, 5BB657h
			jmp ecx

		doNotLoad:
			mov eax, [esp + 8]
			retn
		}
	}

	void AssetHandler::On(Game::XAssetType type, AssetHandler::Callback callback)
	{
		AssetHandler::TypeCallbacks[type] = callback;
	}

	void AssetHandler::Restrict(RestrictCallback callback)
	{
		AssetHandler::RestrictCallbacks.push_back(callback);
	}

	AssetHandler::AssetHandler()
	{
		Utils::Hook(Game::DB_FindXAssetHeader, AssetHandler::FindAssetStub).Install()->Quick();

		Utils::Hook(0x5BB650, AssetHandler::AddAssetStub, HOOK_JUMP).Install()->Quick();
	}

	AssetHandler::~AssetHandler()
	{
		AssetHandler::TypeCallbacks.clear();
	}
}
