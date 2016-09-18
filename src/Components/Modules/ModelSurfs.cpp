#include "STDInclude.hpp"

namespace Components
{
	std::map<void*, IUnknown*> ModelSurfs::BufferMap;
	std::map<void*, Game::CModelAllocData*> ModelSurfs::AllocMap;

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
		for (int i = 0; i < surfs->numSurfaces; ++i)
		{
			Game::XSurface* surface = &surfs->surfaces[i];
			if (surface->streamHandle == 0xFF)
			{
				IDirect3DVertexBuffer9* vertexBuffer;
				IDirect3DIndexBuffer9* indexBuffer;

				Game::Load_VertexBuffer(surface->vertexBuffer, &vertexBuffer, surface->numVertices * 32);
				Game::Load_IndexBuffer(surface->indexBuffer, &indexBuffer, surface->numPrimitives * 3);

				ModelSurfs::BufferMap[surface->vertexBuffer] = vertexBuffer;
				ModelSurfs::BufferMap[surface->indexBuffer] = indexBuffer;
			}
		}
	}

	Game::XModelSurfs* ModelSurfs::LoadXModelSurfaces(std::string name)
	{
		Utils::Memory::Allocator allocator;
		FileSystem::File model(fmt::sprintf("models/%s", name.data()));

		if (!model.Exists())
		{
			Logger::Error("Loading model %s failed!", name.data());
		}

		Game::CModelHeader header;
		if (!model.Read(&header, sizeof header))
		{
			Logger::Error("Reading header for model %s failed!", name.data());
		}

		if (header.version != 1)
		{
			Logger::Error("Model %s has an invalid version %d (should be 1)!", name.data(), header.version);
		}

		// Allocate section buffers
		header.sectionHeader[Game::SECTION_MAIN].buffer = Utils::Memory::Allocate(header.sectionHeader[Game::SECTION_MAIN].size);
		header.sectionHeader[Game::SECTION_INDEX].buffer = Utils::Memory::AllocateAlign(header.sectionHeader[Game::SECTION_INDEX].size, 16);
		header.sectionHeader[Game::SECTION_VERTEX].buffer = Utils::Memory::AllocateAlign(header.sectionHeader[Game::SECTION_VERTEX].size, 16);
		header.sectionHeader[Game::SECTION_FIXUP].buffer = allocator.AllocateArray<char>(header.sectionHeader[Game::SECTION_FIXUP].size);

		// Load section data
		for (int i = 0; i < ARRAY_SIZE(header.sectionHeader); ++i)
		{
			model.Seek(header.sectionHeader[i].offset, FS_SEEK_SET);
			if (!model.Read(header.sectionHeader[i].buffer, header.sectionHeader[i].size))
			{
				Logger::Error("Reading section %d for model %s failed!", i, name.data());
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

		ModelSurfs::AllocMap[allocationData->vertexBuffer] = allocationData;
		*reinterpret_cast<void**>(reinterpret_cast<char*>(allocationData->mainArray) + 44) = allocationData;

		Assert_Size(Game::XSurface, 64);
		Game::XModelSurfs* modelSurfs = reinterpret_cast<Game::XModelSurfs*>(allocationData->mainArray);
		Game::XSurface* tempSurfaces = allocator.AllocateArray<Game::XSurface>(modelSurfs->numSurfaces);
		char* surfaceData = reinterpret_cast<char*>(modelSurfs->surfaces);

		for (int i = 0; i < modelSurfs->numSurfaces; ++i)
		{
			memcpy(&tempSurfaces[i], surfaceData + (i * 84), 12);
			memcpy(&tempSurfaces[i].indexBuffer, surfaceData + (i * 84) + 16, 20);
			memcpy(&tempSurfaces[i].numCT, surfaceData + (i * 84) + 40, 8);
			memcpy(&tempSurfaces[i].something, surfaceData + (i * 84) + 52, 24);
			tempSurfaces[i].streamHandle = 0xFF; // Fake handle for buffer interception
		}

		memcpy(surfaceData, tempSurfaces, 64 * modelSurfs->numSurfaces);

		ModelSurfs::CreateBuffers(modelSurfs);

		return modelSurfs;
	}

	Game::FS_FOpenFileRead_t FS_FOpenFileReadDatabase = (Game::FS_FOpenFileRead_t)0x42ECA0;

	void FS_FOpenFileReadCurrentThread(char* filename, int* handle)
	{
		if (GetCurrentThreadId() == *(DWORD*)0x1CDE7FC)
		{
			Game::FS_FOpenFileRead(filename, handle, 0);
		}
		else if (GetCurrentThreadId() == *(DWORD*)0x1CDE814)
		{
			FS_FOpenFileReadDatabase(filename, handle, 0);
		}
		else
		{
			*handle = NULL;
		}
	}

	struct CModelSectionHeader
	{
		int size;
		int offset;
		int fixupStart;
		int fixupCount;
		char* buffer;
	};

	struct CModelHeader
	{
		int version;
		unsigned int signature;
		CModelSectionHeader main;
		CModelSectionHeader index;
		CModelSectionHeader vertex;
		CModelSectionHeader fixup;
	};

	void ReadCModelSection(int handle, CModelSectionHeader& header)
	{
		Game::FS_Seek(handle, header.offset, FS_SEEK_SET);
		Game::FS_Read(header.buffer, header.size, handle);
	}

	void FixupCModelSection(CModelHeader& header, CModelSectionHeader& section, DWORD* fixups)
	{
		for (int i = section.fixupStart; i < section.fixupStart + section.fixupCount; i++)
		{
			DWORD fixup = fixups[i];
			int targetSectionNum = fixup & 3;
			CModelSectionHeader* targetSection;

			if (targetSectionNum == 0)
			{
				targetSection = &header.main;
			}
			else if (targetSectionNum == 1)
			{
				targetSection = &header.index;
			}
			else if (targetSectionNum == 2)
			{
				targetSection = &header.vertex;
			}

			*(DWORD*)(section.buffer + (fixup >> 3)) += (DWORD)targetSection->buffer;
		}
	}

	void Load_VertexBuffer(void* data, void** where, int len)
	{
		DWORD func = 0x5112C0;

		__asm
		{
			push edi

			mov eax, len
			mov edi, where
			push data

			call func

			add esp, 4
			pop edi
		}
	}

	typedef void* (__cdecl * R_AllocStaticIndexBuffer_t)(void** store, int length);
	R_AllocStaticIndexBuffer_t R_AllocStaticIndexBuffer = (R_AllocStaticIndexBuffer_t)0x51E7A0;

	void Load_IndexBuffer(void* data, void** storeHere, int count)
	{
		static Game::dvar_t* r_loadForRenderer = *(Game::dvar_t**)0x69F0ED4;

		if (r_loadForRenderer->current.boolean)
		{
			void* buffer = R_AllocStaticIndexBuffer(storeHere, 2 * count);
			memcpy(buffer, data, 2 * count);

			if (IsBadReadPtr(storeHere, 4) || IsBadReadPtr(*storeHere, 4))
			{
				Game::Com_Error(0, "Static index buffer allocation failed.");
			}

			__asm
			{
				push ecx
				mov ecx, storeHere
				mov ecx, [ecx]

				mov eax, [ecx]
				add eax, 30h
				mov eax, [eax]

				push ecx
				call eax

				pop ecx
			}
		}
	}

	void CreateCModelBuffers(void* surfs)
	{
		Game::XModelSurfs* model = (Game::XModelSurfs*)surfs;

		for (int i = 0; i < model->numSurfaces; i++)
		{
			Game::XSurface* surface = &model->surfaces[i];

			void* vertexBuffer;
			void* indexBuffer;

			Load_VertexBuffer(surface->vertexBuffer, &vertexBuffer, surface->numVertices * 32);
			Load_IndexBuffer(surface->indexBuffer, &indexBuffer, surface->numPrimitives * 3);

			ModelSurfs::BufferMap[surface->vertexBuffer] = (IUnknown*)vertexBuffer;
			ModelSurfs::BufferMap[surface->indexBuffer] = (IUnknown*)indexBuffer;
		}
	}

	char* LoadCModel(const char* name)
	{
		char filename[512];
		sprintf_s(filename, sizeof(filename), "models/%s", name);

		int handle;
		FS_FOpenFileReadCurrentThread(filename, &handle);

		if (handle <= 0)
		{
			Game::Com_Error(1, "Error opening %s", filename);
		}

		CModelHeader header;

		if (Game::FS_Read(&header, sizeof(header), handle) != sizeof(header))
		{
			Game::FS_FCloseFile(handle);
			Game::Com_Error(1, "%s: header could not be read", filename);
		}

		if (header.version != 1)
		{
			Game::FS_FCloseFile(handle);
			Game::Com_Error(1, "%s: header version is not '1'", filename);
		}

		static DWORD fixups[4096];

		if (header.fixup.size >= sizeof(fixups))
		{
			Game::FS_FCloseFile(handle);
			Game::Com_Error(1, "%s: fixup size too big", filename);
		}

		header.main.buffer = (char*)malloc(header.main.size);
		header.index.buffer = (char*)_aligned_malloc(header.index.size, 16);
		header.vertex.buffer = (char*)_aligned_malloc(header.vertex.size, 16);
		header.fixup.buffer = (char*)fixups;

		ReadCModelSection(handle, header.main);
		ReadCModelSection(handle, header.index);
		ReadCModelSection(handle, header.vertex);
		ReadCModelSection(handle, header.fixup);

		Game::FS_FCloseFile(handle);

		FixupCModelSection(header, header.main, fixups);
		FixupCModelSection(header, header.index, fixups);
		FixupCModelSection(header, header.vertex, fixups);

		Game::CModelAllocData* allocationData = (Game::CModelAllocData*)malloc(sizeof(Game::CModelAllocData));
		allocationData->mainArray = header.main.buffer;
		allocationData->indexBuffer = header.index.buffer;
		allocationData->vertexBuffer = header.vertex.buffer;

		*(void**)(header.main.buffer + 44) = allocationData;

		// maybe the +36/48 = 0 stuff here?

		// move buffers to work with iw4
		int numSurfaces = *(short*)(header.main.buffer + 8);

		char* tempSurface = new char[84 * numSurfaces];
		char* surface = *(char**)(header.main.buffer + 4);

		for (int i = 0; i < numSurfaces; i++)
		{
			char* source = &surface[84 * i];
			char* dest = &tempSurface[64 * i];

			memcpy(dest, source, 12);
			memcpy(dest + 12, source + 16, 20);
			memcpy(dest + 32, source + 40, 8);
			memcpy(dest + 40, source + 52, 24);

			dest[6] = 0xFF; // fake stream handle for the vertex/index buffer get code to use
		}

		memcpy(surface, tempSurface, 84 * numSurfaces);

		delete[] tempSurface;

		// create vertex/index buffers
		CreateCModelBuffers(header.main.buffer);

		// store the buffer bit
		ModelSurfs::AllocMap[header.vertex.buffer] = allocationData;

		return header.main.buffer;
	}

	bool ModelSurfs::LoadSurfaces(Game::XModel* model)
	{
		if (!model) return false;
		bool changed = false;

		short surfCount = 0;

		static_assert(offsetof(Game::XModel, lods) == 64, "");
		static_assert(offsetof(Game::XModelLodInfo, surfs) == 36, "");

		for (char i = 0; i < model->numLods; ++i)
		{
			Game::XModelSurfs* surfs = model->lods[i].surfaces;

			if (!surfs->surfaces)
			{
				Game::XModelSurfs* newSurfs = ModelSurfs::LoadXModelSurfaces(surfs->name);

				surfs->surfaces = newSurfs->surfaces;
				surfs->numSurfaces = newSurfs->numSurfaces;

				model->lods[i].surfaces = newSurfs;
				memcpy(model->lods[i].pad3, newSurfs->pad, 24);

				short numSurfs = static_cast<short>(newSurfs->numSurfaces);
				model->lods[i].someCount = numSurfs;
				model->lods[i].someTotalCount = surfCount;
				surfCount += numSurfs;

				changed = true;
			}
		}

		return changed;
	}

	bool Load_XModelAssetHookFunc(char* xmodel)
	{
		bool didStuff = false;
		short totalv = 0;

		for (int i = 0; i < xmodel[241]; i++)
		{
			Game::XModelSurfs* surfs = *(Game::XModelSurfs**)(xmodel + 72 + (44 * i));

			if (!surfs->surfaces)
			{
				char* newSurfs = LoadCModel(surfs->name);

				memcpy(xmodel + 76 + (44 * i), &newSurfs[12], 24);
				memcpy(xmodel + 100 + (44 * i), &newSurfs[4], 4);

				short v = *(short*)(newSurfs + 8);
				*(short*)(xmodel + 68 + (44 * i)) = v;
				*(short*)(xmodel + 70 + (44 * i)) = totalv;

				totalv += v;

				surfs->numSurfaces = ((Game::XModelSurfs*)newSurfs)->numSurfaces;
				surfs->surfaces = ((Game::XModelSurfs*)newSurfs)->surfaces;

				didStuff = true;
			}
		}

		return didStuff;
	}

	void ModelSurfs::ReleaseModelSurf(Game::XAssetHeader header)
	{
		for (int i = 0; i < header.surfaces->numSurfaces && header.surfaces->surfaces; ++i)
		{
			Game::XSurface* surface = &header.surfaces->surfaces[i];
			if (surface->streamHandle == 0xFF)
			{
				auto buffer = ModelSurfs::BufferMap.find(surface->indexBuffer);
				if (buffer != ModelSurfs::BufferMap.end())
				{
					buffer->second->Release();
					ModelSurfs::BufferMap.erase(buffer);
				}

				buffer = ModelSurfs::BufferMap.find(surface->vertexBuffer);
				if (buffer != ModelSurfs::BufferMap.end())
				{
					buffer->second->Release();
					ModelSurfs::BufferMap.erase(buffer);
				}

				auto allocData = ModelSurfs::AllocMap.find(surface->vertexBuffer);
				if (allocData != ModelSurfs::AllocMap.end())
				{
					Utils::Memory::Free(allocData->second->indexBuffer);
					Utils::Memory::Free(allocData->second->vertexBuffer);
					Utils::Memory::Free(allocData->second->mainArray);
					Utils::Memory::Free(allocData->second);

					ModelSurfs::AllocMap.erase(allocData);
				}
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
		Game::DB_EnumXAssets_Internal(Game::XAssetType::ASSET_TYPE_XMODELSURFS, [] (Game::XAssetHeader header, void* /*userdata*/)
		{
			ModelSurfs::CreateBuffers(header.surfaces);
		}, nullptr, false);
	}

	void ModelSurfs::XModelSurfsFixup(Game::XModel* model)
	{
		//if (!ModelSurfs::LoadSurfaces(model))
		if(!Load_XModelAssetHookFunc((char*)model))
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
		Game::DB_ReleaseXAssetHandlers[Game::XAssetType::ASSET_TYPE_XMODELSURFS] = ModelSurfs::ReleaseModelSurf;

		// Install device recovery handlers
		Renderer::OnDeviceRecoveryBegin(ModelSurfs::BeginRecover);
		Renderer::OnDeviceRecoveryEnd(ModelSurfs::EndRecover);

		// Install hooks
		Utils::Hook(0x47A6BD, ModelSurfs::XModelSurfsFixup, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x558F12, ModelSurfs::GetIndexBaseStub, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x5BC050, ModelSurfs::GetIndexBufferStub, HOOK_JUMP).Install()->Quick();
		Utils::Hook(0x558E70, ModelSurfs::GetIndexBufferStub2, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x5BC050, ModelSurfs::GetVertexBufferStub, HOOK_JUMP).Install()->Quick();
	}

	ModelSurfs::~ModelSurfs()
	{
		assert(ModelSurfs::BufferMap.empty());
		assert(ModelSurfs::AllocMap.empty());
	}
}
