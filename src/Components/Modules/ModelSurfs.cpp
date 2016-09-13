#include "STDInclude.hpp"

namespace Components
{
	std::map<void*, IUnknown*> ModelSurfs::BufferMap;

	IUnknown* ModelSurfs::GetBuffer(void* buffer)
	{
		return ModelSurfs::BufferMap[buffer];
	}

	void ModelSurfs::SetBuffer(char /*streamHandle*/, void* buffer, IUnknown** bufferOut, int* offsetOut)
	{
		*offsetOut;
		*bufferOut = ModelSurfs::BufferMap[buffer];
	}

	void ModelSurfs::ReleaseModelSurf(Game::XAssetHeader header)
	{
		Game::XModelSurfs* surfaces = header.surfaces;

		(surfaces);
	}

	void ModelSurfs::BeginRecover()
	{
		for (auto& buffer : ModelSurfs::BufferMap)
		{
			buffer.second->Release();
		}

		ModelSurfs::BufferMap.clear();
	}

	void ModelSurfs::EndRecover()
	{
		Game::DB_EnumXAssets_Internal(Game::XAssetType::ASSET_TYPE_XMODELSURFS, [] (Game::XAssetHeader header, void* /*userdata*/)
		{
			Game::XModelSurfs* surfaces = header.surfaces;

			// TODO: Recreate all buffers here
			(surfaces);
		}, nullptr, false);
	}

	__declspec(naked) void ModelSurfs::GetIndexBufferStub()
	{
		__asm
		{
			mov eax, [esp + 4h]
			cmp al, 0FFh

			jne returnSafe

			jmp ModelSurfs::SetBuffer

		returnSafe:
			movzx eax, [esp + 4h]
			mov edx, 4B4DE5h
			jmp edx
		}
	}

	__declspec(naked) void ModelSurfs::GetIndexBufferStub2()
	{
		__asm
		{
			mov eax, [esp + 4h]
			cmp al, 0FFh

			jne returnSafe

			mov eax, [edi + 0Ch]
			push eax
			call ModelSurfs::GetBuffer
			add esp, 4h
			retn

		returnSafe:
			mov eax, 4FDC20h
			jmp eax
		}
	}

	__declspec(naked) void ModelSurfs::GetVertexBufferStub()
	{
		__asm
		{
			mov eax, [esp + 4h]
			cmp al, 0FFh

			jne returnSafe

			jmp ModelSurfs::SetBuffer

		returnSafe:
			movzx eax, [esp + 4h]
			mov edx, 5BC055h
			jmp edx
		}
	}

	ModelSurfs::ModelSurfs()
	{
		ModelSurfs::BufferMap.clear();

		// Install release handler
		Game::DB_ReleaseXAssetHandlers[Game::XAssetType::ASSET_TYPE_XMODELSURFS] = ModelSurfs::ReleaseModelSurf;

		// Install device recovery handlers
		Renderer::OnDeviceRecoveryBegin(ModelSurfs::BeginRecover);
		Renderer::OnDeviceRecoveryEnd(ModelSurfs::EndRecover);

		// Install hooks
		Utils::Hook(0x5BC050, ModelSurfs::GetIndexBufferStub, HOOK_JUMP).Install()->Quick();
		Utils::Hook(0x558E70, ModelSurfs::GetIndexBufferStub2, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x5BC050, ModelSurfs::GetVertexBufferStub, HOOK_JUMP).Install()->Quick();
	}

	ModelSurfs::~ModelSurfs()
	{
		assert(ModelSurfs::BufferMap.empty());
	}
}
