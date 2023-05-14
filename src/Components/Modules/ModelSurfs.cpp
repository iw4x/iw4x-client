#include <STDInclude.hpp>
#include "ModelSurfs.hpp"

namespace Components
{
	std::unordered_map<void*, IUnknown*> ModelSurfs::BufferMap;
	std::unordered_map<std::string, Game::CModelAllocData*> ModelSurfs::AllocMap;

	IUnknown* ModelSurfs::GetBuffer(void* buffer)
	{
		return ModelSurfs::BufferMap[buffer];
	}

	void ModelSurfs::SetBuffer(char /*streamHandle*/, void* buffer, IUnknown** bufferOut, int* offsetOut)
	{
		*offsetOut = 0;
		*bufferOut = ModelSurfs::BufferMap[buffer];
	}

	void ModelSurfs::CreateBuffers(Game::XModelSurfs* surfs)
	{
		for (int i = 0; i < surfs->numsurfs; ++i)
		{
			Game::XSurface* surface = &surfs->surfs[i];
			if (surface->zoneHandle == -1)
			{
				IDirect3DVertexBuffer9* vertexBuffer = nullptr;
				IDirect3DIndexBuffer9* indexBuffer = nullptr;

				Game::Load_VertexBuffer(surface->verts0, &vertexBuffer, surface->vertCount * 32);
				Game::Load_IndexBuffer(surface->triIndices, &indexBuffer, surface->triCount * 3);

				if (vertexBuffer) ModelSurfs::BufferMap[surface->verts0] = vertexBuffer;
				if (indexBuffer) ModelSurfs::BufferMap[surface->triIndices] = indexBuffer;
			}
		}
	}

	Game::XModelSurfs* ModelSurfs::LoadXModelSurfaces(const std::string& name)
	{
		Utils::Memory::Allocator allocator;
		const auto path = std::format("models/{}", name);
		FileSystem::FileReader model(path);

		if (!model.exists())
		{
#ifdef DEBUG
			if (Flags::HasFlag("dump"))
			{
				FILE* fp = nullptr;
				if (!fopen_s(&fp, "dump.cfg", "a") && fp)
				{
					fprintf(fp, "dumpraw %s\n", model.getName().data());
					fclose(fp);
				}

				return nullptr;
			}
#endif

			if (ZoneBuilder::IsEnabled())
			{
				Logger::Print("Loading model surface {} at path \"{}\" failed!", name, path);
			}
			else
			{
				Logger::Error(Game::ERR_FATAL, "Loading model {} failed!", name);
			}

			return nullptr;
		}

		Game::CModelHeader header;
		if (!model.read(&header, sizeof header))
		{
			Logger::Error(Game::ERR_FATAL, "Reading header for model {} failed!", name);
		}

		if (header.version != 1)
		{
			Logger::Error(Game::ERR_FATAL, "Model {} has an invalid version {} (should be 1)!", name, header.version);
		}

		// Allocate section buffers
		header.sectionHeader[Game::SECTION_MAIN].buffer = Utils::Memory::Allocate(header.sectionHeader[Game::SECTION_MAIN].size);
		header.sectionHeader[Game::SECTION_INDEX].buffer = Utils::Memory::AllocateAlign(header.sectionHeader[Game::SECTION_INDEX].size, 16);
		header.sectionHeader[Game::SECTION_VERTEX].buffer = Utils::Memory::AllocateAlign(header.sectionHeader[Game::SECTION_VERTEX].size, 16);
		header.sectionHeader[Game::SECTION_FIXUP].buffer = allocator.allocateArray<char>(header.sectionHeader[Game::SECTION_FIXUP].size);

		// Load section data
		for (int i = 0; i < ARRAYSIZE(header.sectionHeader); ++i)
		{
			model.seek(header.sectionHeader[i].offset, Game::FS_SEEK_SET);
			if (!model.read(header.sectionHeader[i].buffer, header.sectionHeader[i].size))
			{
				Logger::Error(Game::ERR_FATAL, "Reading section {} for model {} failed!", i, name);
			}
		}

		// Fixup sections
		unsigned int* fixups = reinterpret_cast<unsigned int*>(header.sectionHeader[Game::SECTION_FIXUP].buffer);
		for (int i = 0; i < 3; ++i)
		{
			Game::CModelSectionHeader* section = &header.sectionHeader[i];
			for (int j = section->fixupStart; j < section->fixupStart + section->fixupCount; ++j)
			{
				unsigned int fixup = fixups[j];
				*reinterpret_cast<DWORD*>(reinterpret_cast<char*>(section->buffer) + (fixup >> 3)) += reinterpret_cast<DWORD>(header.sectionHeader[fixup & 3].buffer);
			}
		}

		// Store allocation data (not sure if this is correct)
		Game::CModelAllocData* allocationData = Utils::Memory::AllocateArray<Game::CModelAllocData>();
		allocationData->mainArray = header.sectionHeader[Game::SECTION_MAIN].buffer;
		allocationData->indexBuffer = header.sectionHeader[Game::SECTION_INDEX].buffer;
		allocationData->vertexBuffer = header.sectionHeader[Game::SECTION_VERTEX].buffer;

		AssertSize(Game::XSurface, 64);
		Game::XModelSurfs* modelSurfs = reinterpret_cast<Game::XModelSurfs*>(allocationData->mainArray);
		Game::XSurface* tempSurfaces = allocator.allocateArray<Game::XSurface>(modelSurfs->numsurfs);
		char* surfaceData = reinterpret_cast<char*>(modelSurfs->surfs);

		if (ModelSurfs::AllocMap.contains(modelSurfs->name))
		{
			Game::CModelAllocData* allocData = ModelSurfs::AllocMap[modelSurfs->name];

			if (allocData)
			{
				Utils::Memory::FreeAlign(allocData->indexBuffer);
				Utils::Memory::FreeAlign(allocData->vertexBuffer);
				Utils::Memory::Free(allocData->mainArray);
				Utils::Memory::Free(allocData);
			}
		}

		ModelSurfs::AllocMap[modelSurfs->name] = allocationData;
		*reinterpret_cast<void**>(reinterpret_cast<char*>(allocationData->mainArray) + 44) = allocationData;

		for (int i = 0; i < modelSurfs->numsurfs; ++i)
		{
			char* source = &surfaceData[i * 84];

			std::memcpy(&tempSurfaces[i], source, 12);
			std::memcpy(&tempSurfaces[i].triIndices, source + 16, 20);
			std::memcpy(&tempSurfaces[i].vertListCount, source + 40, 8);
			std::memcpy(&tempSurfaces[i].partBits, source + 52, 24);
			tempSurfaces[i].zoneHandle = -1; // Fake handle for buffer interception
		}

		std::memcpy(surfaceData, tempSurfaces, 64 * modelSurfs->numsurfs);

		ModelSurfs::CreateBuffers(modelSurfs);

		return modelSurfs;
	}

	bool ModelSurfs::LoadSurfaces(Game::XModel* model)
	{
		if (!model) return false;

		bool changed = false;
		short surfCount = 0;

		for (char i = 0; i < model->numLods; ++i)
		{
			Game::XModelSurfs* surfs = model->lodInfo[i].modelSurfs;

			if (!surfs->surfs)
			{
				AssertOffset(Game::XModelLodInfo, partBits, 12);
				Game::XModelSurfs* newSurfs = ModelSurfs::LoadXModelSurfaces(surfs->name);
				if (!newSurfs) continue;

				surfs->surfs = newSurfs->surfs;
				surfs->numsurfs = newSurfs->numsurfs;

				model->lodInfo[i].surfs = newSurfs->surfs;
				std::memcpy(&model->lodInfo[i].partBits, &newSurfs->partBits, 24);

				short numSurfs = static_cast<short>(newSurfs->numsurfs);
				model->lodInfo[i].numsurfs = numSurfs;
				model->lodInfo[i].surfIndex = surfCount;
				surfCount += numSurfs;

				changed = true;
			}
		}

		return changed;
	}

	void ModelSurfs::ReleaseModelSurf(Game::XAssetHeader header)
	{
		bool hasCustomSurface = false;
		for (int i = 0; i < header.modelSurfs->numsurfs && header.modelSurfs->surfs; ++i)
		{
			Game::XSurface* surface = &header.modelSurfs->surfs[i];

			if (surface->zoneHandle == -1)
			{
				hasCustomSurface = true;

				if (!ModelSurfs::BufferMap.empty())
				{
					auto buffer = ModelSurfs::BufferMap.find(surface->triIndices);
					if (buffer != ModelSurfs::BufferMap.end())
					{
						if (buffer->second) buffer->second->Release();
						ModelSurfs::BufferMap.erase(buffer);
					}

					buffer = ModelSurfs::BufferMap.find(surface->verts0);
					if (buffer != ModelSurfs::BufferMap.end())
					{
						if (buffer->second) buffer->second->Release();
						ModelSurfs::BufferMap.erase(buffer);
					}
				}
			}
		}

		if (hasCustomSurface && !ModelSurfs::AllocMap.empty())
		{
			const auto allocData = ModelSurfs::AllocMap.find(header.modelSurfs->name);
			if (allocData != ModelSurfs::AllocMap.end())
			{
				Utils::Memory::FreeAlign(allocData->second->indexBuffer);
				Utils::Memory::FreeAlign(allocData->second->vertexBuffer);
				Utils::Memory::Free(allocData->second->mainArray);
				Utils::Memory::Free(allocData->second);

				ModelSurfs::AllocMap.erase(allocData);
			}
		}
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
		Game::DB_EnumXAssets_Internal(Game::XAssetType::ASSET_TYPE_XMODEL_SURFS, [](Game::XAssetHeader header, void* /*userdata*/)
		{
			ModelSurfs::CreateBuffers(header.modelSurfs);
		}, nullptr, false);
	}

	void ModelSurfs::XModelSurfsFixup(Game::XModel* model)
	{
		if (!ModelSurfs::LoadSurfaces(model))
		{
			Game::DB_XModelSurfsFixup(model);
		}
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

	__declspec(naked) void ModelSurfs::GetIndexBaseStub()
	{
		__asm
		{
			mov eax, [esp + 4h]
			cmp al, 0FFh

			jne returnSafe

			xor eax, eax
			retn

		returnSafe:
			mov eax, 48C5F0h
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
		Game::DB_ReleaseXAssetHandlers[Game::XAssetType::ASSET_TYPE_XMODEL_SURFS] = ModelSurfs::ReleaseModelSurf;

		// Install device recovery handlers
		Renderer::OnDeviceRecoveryBegin(ModelSurfs::BeginRecover);
		Renderer::OnDeviceRecoveryEnd(ModelSurfs::EndRecover);

		// Install hooks
		Utils::Hook(0x47A6BD, ModelSurfs::XModelSurfsFixup, HOOK_CALL).install()->quick();
		Utils::Hook(0x558F12, ModelSurfs::GetIndexBaseStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x4B4DE0, ModelSurfs::GetIndexBufferStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x558E70, ModelSurfs::GetIndexBufferStub2, HOOK_CALL).install()->quick();
		Utils::Hook(0x5BC050, ModelSurfs::GetVertexBufferStub, HOOK_JUMP).install()->quick();
	}

	ModelSurfs::~ModelSurfs()
	{
		assert(ModelSurfs::BufferMap.empty());
		assert(ModelSurfs::AllocMap.empty());
	}
}
