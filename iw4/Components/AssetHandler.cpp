#include "..\STDInclude.hpp"

namespace Components
{
	bool AssetHandler::BypassState = false;
	std::map<Game::XAssetType, AssetHandler::Callback> AssetHandler::TypeCallbacks;

	Game::XAssetHeader AssetHandler::FindAsset(Game::XAssetType type, const char* filename)
	{
		Game::XAssetHeader header = nullptr;

		AssetHandler::BypassState = true;

		if (AssetHandler::TypeCallbacks.find(type) != AssetHandler::TypeCallbacks.end())
		{
			header = AssetHandler::TypeCallbacks[type](type, filename);
		}

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

	void AssetHandler::On(Game::XAssetType type, AssetHandler::Callback callback)
	{
		AssetHandler::TypeCallbacks[type] = callback;
	}

	AssetHandler::AssetHandler()
	{
		Utils::Hook(Game::DB_FindXAssetHeader, AssetHandler::FindAssetStub).Install()->Quick();
	}

	AssetHandler::~AssetHandler()
	{
		AssetHandler::TypeCallbacks.clear();
	}
}
