#include "STDInclude.hpp"

namespace Components
{
	int Zones::ZoneVersion;

	int Zones::FxEffectIndex;
	char* Zones::FxEffectStrings[64];

	Utils::Hook Zones::LoadFxElemDefHook;
	Utils::Hook Zones::LoadFxElemDefArrayHook;
	Utils::Hook Zones::LoadXModelLodInfoHook;
	Utils::Hook Zones::LoadXModelHook;
	Utils::Hook Zones::LoadXSurfaceArrayHook;
	Utils::Hook Zones::LoadGameWorldSpHook;
	Utils::Hook Zones::LoadPathDataHook;
	Utils::Hook Zones::LoadVehicleDefHook;
	Utils::Hook Zones::Loadsnd_alias_tArrayHook;
	Utils::Hook Zones::LoadLoadedSoundHook;
	Utils::Hook Zones::LoadmenuDef_tHook;
	Utils::Hook Zones::LoadFxEffectDefHook;
	Utils::Hook Zones::LoadMaterialShaderArgumentArrayHook;
	Utils::Hook Zones::LoadStructuredDataStructPropertyArrayHook;
	Utils::Hook Zones::LoadPathDataTailHook;
	Utils::Hook Zones::LoadWeaponAttachHook;
	Utils::Hook Zones::LoadWeaponCompleteDefHook;
	Utils::Hook Zones::LoadGfxImageHook;
	Utils::Hook Zones::LoadXAssetHook;
	Utils::Hook Zones::LoadMaterialTechniqueHook;
	Utils::Hook Zones::LoadMaterialHook;
	Utils::Hook Zones::LoadGfxWorldHook;
	Utils::Hook Zones::Loadsunflare_tHook;

	bool Zones::LoadFxEffectDef(bool atStreamStart, char* buffer, int size)
	{
		size /= 252;
		int count = size;
		size *= 260;

		bool result = Game::Load_Stream(atStreamStart, buffer, size);

		Zones::FxEffectIndex = 0;

		Utils::Memory::Allocator allocator;
		Game::FxElemDef* elems = allocator.AllocateArray<Game::FxElemDef>(count);

		for (int i = 0; i < count; ++i)
		{
			AssetHandler::Relocate(buffer + (260 * i), buffer + (252 * i), 252);
			std::memcpy(&elems[i], buffer + (260 * i), 252);
			Zones::FxEffectStrings[i] = *reinterpret_cast<char**>(buffer + (260 * i) + 256);
		}

		std::memcpy(buffer, elems, sizeof(Game::FxElemDef) * count);

		return result;
	}

	bool Zones::LoadFxElemDefStub(bool atStreamStart, Game::FxElemDef* fxElem, int size)
	{
		if (fxElem->elemType == 3)
		{
			fxElem->elemType = 2;
		}
		else if (fxElem->elemType >= 5)
		{
			fxElem->elemType -= 2;
		}

		return Game::Load_Stream(atStreamStart, fxElem, size);
	}

	void Zones::LoadFxElemDefArrayStub(bool atStreamStart)
	{
		Game::Load_FxElemDef(atStreamStart);
		*Game::varXString = &Zones::FxEffectStrings[Zones::FxEffectIndex++];
		Game::Load_XString(false);
	}

	bool Zones::LoadXModel(bool atStreamStart, char* xmodel, int size)
	{
		bool result = Game::Load_Stream(atStreamStart, xmodel, size);

		int elSize = (Zones::ZoneVersion == VERSION_ALPHA2) ? 364 : 360;

		Game::XModel model[2]; // Allocate 2 models, as we exceed the buffer

		std::memcpy(model, xmodel, 36);
		std::memcpy(&model->pad3[0x1C], &xmodel[44], 28);

		for (int i = 0; i < 4; ++i)
		{
			std::memcpy(&model->lods[i], &xmodel[72 + (i * 56)], 12);
			std::memcpy(&model->lods[i].pad3, &xmodel[72 + (i * 56) + 16], 32);

			std::memcpy(reinterpret_cast<char*>(&model) + (elSize - 4) - (i * 4), &xmodel[72 + (i * 56) + 12], 4);
		}

		std::memcpy(&model->lods[3].pad4[0], &xmodel[292], (elSize - 292 - 4)/*68*/);
		std::memcpy(&model->physPreset, &xmodel[(elSize - 8)], 8);

		model[1].name = reinterpret_cast<char*>(0xDEADC0DE);

		std::memcpy(xmodel, &model, elSize);

		return result;
	}

	void Zones::LoadXModelLodInfo(int i)
	{
		int elSize = (Zones::ZoneVersion == VERSION_ALPHA2) ? 364 : 360;
		*Game::varXString = reinterpret_cast<char**>(reinterpret_cast<char*>(*Game::varXModel) + (elSize - 4) - (4 * (4 - i)));
		Game::Load_XString(false);
	}

	__declspec(naked) void Zones::LoadXModelLodInfoStub()
	{
		__asm
		{
			push edi
			call Zones::LoadXModelLodInfo
			add esp, 4h

			mov eax, [esp + 8h]
			push eax
			add eax, 8
			push eax
			call Game::Load_XModelSurfsFixup
			add esp, 8h

			retn
		}
	}

	bool Zones::LoadXSurfaceArray(bool atStreamStart, char* buffer, int size)
	{
		size >>= 6;

		int count = size;
		size *= 84;

		bool result = Game::Load_Stream(atStreamStart, buffer, size);

		Utils::Memory::Allocator allocator;
		Game::XSurface* tempSurfaces = allocator.AllocateArray<Game::XSurface>(count);

		for (int i = 0; i < count; ++i)
		{
			char* source = &buffer[i * 84];

			std::memcpy(&tempSurfaces[i], source, 12);
			std::memcpy(&tempSurfaces[i].indexBuffer, source + 16, 20);
			std::memcpy(&tempSurfaces[i].numCT, source + 40, 8);
			std::memcpy(&tempSurfaces[i].something, source + 52, 24);

			if (Zones::ZoneVersion >= 332)
			{
				struct
				{
					short pad;                    // +0
					char flag;                    // +2
					unsigned char streamHandle;   // +3
					unsigned short numVertices;   // +4
					unsigned short numPrimitives; // +6
					// [...]
				} surface332;

				// Copy the data to our new structure
				std::memcpy(&surface332, &tempSurfaces[i], sizeof(surface332));

				// Check if that special flag is set
				if (!(surface332.flag & 0x20))
				{
					Logger::Error("We're not able to handle XSurface buffer allocation yet!");
				}

				// Copy the correct data back to our surface
				tempSurfaces[i].streamHandle = surface332.streamHandle;
				tempSurfaces[i].numVertices = surface332.numVertices;
				tempSurfaces[i].numPrimitives = surface332.numPrimitives;

				//std::memmove(&tempSurfaces[i].numVertices, &tempSurfaces[i].numPrimitives, 6);
			}
		}

		std::memcpy(buffer, tempSurfaces, sizeof(Game::XSurface) * count);

		return result;
	}

	void Zones::LoadWeaponCompleteDef()
	{
		// setup structures we use
		char* varWeaponCompleteDef = *reinterpret_cast<char**>(0x112A9F4);

		int size = 3112;

		if (Zones::ZoneVersion >= 318)
		{
			size = 3156;

			if (Zones::ZoneVersion >= 332)
			{
				size = 3068; // We didn't adapt that, but who the fuck cares!

				if (Zones::ZoneVersion >= 359)
				{
					size = 3120;
				}
			}
		}

		// and do the stuff
		Game::Load_Stream(true, varWeaponCompleteDef, size);

		Game::DB_PushStreamPos(3);

		*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 0);
		Game::Load_XString(false);

		*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 4);
		Game::Load_XString(false);

		*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 8);
		Game::Load_XString(false);

		*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 12);
		Game::Load_XString(false);

		*Game::varXModelPtr = reinterpret_cast<Game::XModel**>(varWeaponCompleteDef + 16);
		Game::Load_XModelPtr(false);

		if (Zones::ZoneVersion >= 359)
		{
			for (int i = 0, offset = 20; i < 4; ++i, offset += 4)
			{
				*Game::varXModelPtr = reinterpret_cast<Game::XModel**>(varWeaponCompleteDef + offset);
				Game::Load_XModelPtr(false);
			}

			// 148
			for (int offset = 28; offset <= 56; offset += 4)
			{
				*Game::varXModelPtr = reinterpret_cast<Game::XModel**>(varWeaponCompleteDef + offset);
				Game::Load_XModelPtr(false);
			}
		}
		else
		{
			for (int i = 0, offset = 20; i < 32; ++i, offset += 4)
			{
				*Game::varXModelPtr = reinterpret_cast<Game::XModel**>(varWeaponCompleteDef + offset);
				Game::Load_XModelPtr(false);
			}

			// 148
			for (int offset = 148; offset <= 168; offset += 4)
			{
				*Game::varXModelPtr = reinterpret_cast<Game::XModel**>(varWeaponCompleteDef + offset);
				Game::Load_XModelPtr(false);
			}
		}


		// 172
		// 32 scriptstrings, should not need to be loaded

		if (Zones::ZoneVersion >= 359)
		{
			// 236
			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 124);
			Game::Load_XStringArray(false, 52);

			// 428
			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 332);
			Game::Load_XStringArray(false, 52);

			// 620
			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 540);
			Game::Load_XStringArray(false, 52);
		}
		else
		{
			// 236
			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 236);
			Game::Load_XStringArray(false, 48);

			// 428
			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 428);
			Game::Load_XStringArray(false, 48);

			// 620
			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 620);
			Game::Load_XStringArray(false, 48);
		}

		// 812
		// 16 * 4 scriptstrings

		if (Zones::ZoneVersion >= 359)
		{
			// 972
			*Game::varFxEffectDefHandle = reinterpret_cast<Game::FxEffectDef**>(varWeaponCompleteDef + 908);
			Game::Load_FxEffectDefHandle(false);

			*Game::varFxEffectDefHandle = reinterpret_cast<Game::FxEffectDef**>(varWeaponCompleteDef + 912);
			Game::Load_FxEffectDefHandle(false);
		}
		else
		{
			// 972
			*Game::varFxEffectDefHandle = reinterpret_cast<Game::FxEffectDef**>(varWeaponCompleteDef + 972);
			Game::Load_FxEffectDefHandle(false);

			*Game::varFxEffectDefHandle = reinterpret_cast<Game::FxEffectDef**>(varWeaponCompleteDef + 976);
			Game::Load_FxEffectDefHandle(false);
		}

		// 980
		if (Zones::ZoneVersion >= 359)
		{
			// 53 soundalias name references; up to and including 1124
			for (int i = 0, offset = 912; i < 53; ++i, offset += 4)
			{
				*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + offset);
				Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);
			}
		}
		else
		{
			// 50 soundalias name references; up to and including 1180
			for (int i = 0, offset = 980; i < 50; ++i, offset += 4)
			{
				*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + offset);
				Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);
			}

			if (Zones::ZoneVersion >= 318)
			{
				for (int i = 0, offset = 1184; i < 2; ++i, offset += 4)
				{
					*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + offset);
					Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);
				}

				varWeaponCompleteDef += 8; // to compensate for the 2 in between here
			}
		}

		if (Zones::ZoneVersion >= 359)
		{
			if (*reinterpret_cast<void**>(varWeaponCompleteDef + 1128))
			{
				if (*reinterpret_cast<DWORD*>(varWeaponCompleteDef + 1128) == -1)
				{
					*reinterpret_cast<void**>(varWeaponCompleteDef + 1128) = Game::DB_AllocStreamPos(3);
					*Game::varsnd_alias_list_name = *reinterpret_cast<Game::snd_alias_list_t***>(varWeaponCompleteDef + 1128);

					Game::Load_snd_alias_list_nameArray(true, 31);
				}
				else
				{
					// full usability requires ConvertOffsetToPointer here
				}
			}

			if (*reinterpret_cast<void**>(varWeaponCompleteDef + 1132))
			{
				if (*reinterpret_cast<DWORD*>(varWeaponCompleteDef + 1132) == -1)
				{
					*reinterpret_cast<void**>(varWeaponCompleteDef + 1132) = Game::DB_AllocStreamPos(3);
					*Game::varsnd_alias_list_name = *reinterpret_cast<Game::snd_alias_list_t***>(varWeaponCompleteDef + 1132);

					Game::Load_snd_alias_list_nameArray(true, 31);
				}
				else
				{
					// full usability requires ConvertOffsetToPointer here
				}
			}
		}
		else
		{
			if (*reinterpret_cast<void**>(varWeaponCompleteDef + 1184))
			{
				if (*reinterpret_cast<DWORD*>(varWeaponCompleteDef + 1184) == -1)
				{
					*reinterpret_cast<void**>(varWeaponCompleteDef + 1184) = Game::DB_AllocStreamPos(3);
					*Game::varsnd_alias_list_name = *reinterpret_cast<Game::snd_alias_list_t***>(varWeaponCompleteDef + 1184);

					Game::Load_snd_alias_list_nameArray(true, 31);
				}
				else
				{
					// full usability requires ConvertOffsetToPointer here
				}
			}

			if (*reinterpret_cast<void**>(varWeaponCompleteDef + 1188))
			{
				if (*reinterpret_cast<DWORD*>(varWeaponCompleteDef + 1188) == -1)
				{
					*reinterpret_cast<void**>(varWeaponCompleteDef + 1188) = Game::DB_AllocStreamPos(3);
					*Game::varsnd_alias_list_name = *reinterpret_cast<Game::snd_alias_list_t***>(varWeaponCompleteDef + 1188);

					Game::Load_snd_alias_list_nameArray(true, 31);
				}
				else
				{
					// full usability requires ConvertOffsetToPointer here
				}
			}
		}

		if (Zones::ZoneVersion >= 359)
		{
			// 1192
			for (int offset = 1136; offset <= 1148; offset += 4)
			{
				*Game::varFxEffectDefHandle = reinterpret_cast<Game::FxEffectDef**>(varWeaponCompleteDef + offset);
				Game::Load_FxEffectDefHandle(false);
			}
		}
		else
		{
			// 1192
			for (int offset = 1192; offset <= 1204; offset += 4)
			{
				*Game::varFxEffectDefHandle = reinterpret_cast<Game::FxEffectDef**>(varWeaponCompleteDef + offset);
				Game::Load_FxEffectDefHandle(false);
			}
		}

		if (Zones::ZoneVersion >= 359)
		{
			// 1208
			static int matOffsets1[] = { 1152, 1156, 1372,1376,1380, 1384, 1388, 1392, 1400, 1408 };
			for (int i = 0; i < ARRAYSIZE(matOffsets1); ++i)
			{
				*Game::varMaterialHandle = reinterpret_cast<Game::Material**>(varWeaponCompleteDef + matOffsets1[i]);
				Game::Load_MaterialHandle(false);
			}
		}
		else
		{			// 1208
			static int matOffsets1[] = { 1208, 1212, 1428, 1432, 1436, 1440, 1444, 1448, 1456, 1464 };
			for (int i = 0; i < ARRAYSIZE(matOffsets1); ++i)
			{
				*Game::varMaterialHandle = reinterpret_cast<Game::Material**>(varWeaponCompleteDef + matOffsets1[i]);
				Game::Load_MaterialHandle(false);
			}
		}

		if (Zones::ZoneVersion >= 359)
		{
			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 1428);
			Game::Load_XString(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 1436);
			Game::Load_XString(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 1452);
			Game::Load_XString(false);
		}
		else
		{
			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 1484);
			Game::Load_XString(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 1492);
			Game::Load_XString(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 1508);
			Game::Load_XString(false);
		}

		if (Zones::ZoneVersion >= 359)
		{
			for (int offset = 1716; offset <= 1728; offset += 4)
			{
				*Game::varMaterialHandle = reinterpret_cast<Game::Material**>(varWeaponCompleteDef + offset);
				Game::Load_MaterialHandle(false);
			}
		}
		else
		{
			for (int offset = 1764; offset <= 1776; offset += 4)
			{
				*Game::varMaterialHandle = reinterpret_cast<Game::Material**>(varWeaponCompleteDef + offset);
				Game::Load_MaterialHandle(false);
			}
		}

		if (Zones::ZoneVersion >= 359)
		{
			*Game::varPhysCollmapPtr = reinterpret_cast<Game::PhysCollmap**>(varWeaponCompleteDef + 1928);
			Game::Load_PhysCollmapPtr(false);

			*Game::varPhysPresetPtr = reinterpret_cast<Game::PhysPreset**>(varWeaponCompleteDef + 1932);
			Game::Load_PhysPresetPtr(false);
		}
		else
		{
			*Game::varPhysCollmapPtr = reinterpret_cast<Game::PhysCollmap**>(varWeaponCompleteDef + 1964);
			Game::Load_PhysCollmapPtr(false);
		}

		if (Zones::ZoneVersion >= 359)
		{
			*Game::varXModelPtr = reinterpret_cast<Game::XModel**>(varWeaponCompleteDef + 2020);
			Game::Load_XModelPtr(false);
		}
		else
		{
			*Game::varXModelPtr = reinterpret_cast<Game::XModel**>(varWeaponCompleteDef + 2052);
			Game::Load_XModelPtr(false);
		}

		if (Zones::ZoneVersion >= 359)
		{
			*Game::varFxEffectDefHandle = reinterpret_cast<Game::FxEffectDef**>(varWeaponCompleteDef + 2028);
			Game::Load_FxEffectDefHandle(false);

			*Game::varFxEffectDefHandle = reinterpret_cast<Game::FxEffectDef**>(varWeaponCompleteDef + 2032);
			Game::Load_FxEffectDefHandle(false);
		}
		else
		{
			*Game::varFxEffectDefHandle = reinterpret_cast<Game::FxEffectDef**>(varWeaponCompleteDef + 2060);
			Game::Load_FxEffectDefHandle(false);

			*Game::varFxEffectDefHandle = reinterpret_cast<Game::FxEffectDef**>(varWeaponCompleteDef + 2064);
			Game::Load_FxEffectDefHandle(false);
		}

		if (Zones::ZoneVersion >= 359)
		{
			*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + 2036);
			Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);

			*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + 2040);
			Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);
		}
		else
		{
			*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + 2068);
			Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);

			*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + 2072);
			Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);
		}

		if (Zones::ZoneVersion >= 359)
		{
			*Game::varFxEffectDefHandle = reinterpret_cast<Game::FxEffectDef**>(varWeaponCompleteDef + 2304);
			Game::Load_FxEffectDefHandle(false);

			*Game::varFxEffectDefHandle = reinterpret_cast<Game::FxEffectDef**>(varWeaponCompleteDef + 2308);
			Game::Load_FxEffectDefHandle(false);

			*Game::varFxEffectDefHandle = reinterpret_cast<Game::FxEffectDef**>(varWeaponCompleteDef + 2336); 
			Game::Load_FxEffectDefHandle(false);
		}
		else
		{
			*Game::varFxEffectDefHandle = reinterpret_cast<Game::FxEffectDef**>(varWeaponCompleteDef + 2336);
			Game::Load_FxEffectDefHandle(false);

			*Game::varFxEffectDefHandle = reinterpret_cast<Game::FxEffectDef**>(varWeaponCompleteDef + 2340);
			Game::Load_FxEffectDefHandle(false);

			*Game::varFxEffectDefHandle = reinterpret_cast<Game::FxEffectDef**>(varWeaponCompleteDef + 2368); // 2376
			Game::Load_FxEffectDefHandle(false);
		}

		if (Zones::ZoneVersion >= 359)
		{
			*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + 2340);
			Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2516);
			Game::Load_XString(false);
		}
		else
		{
			*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + 2372); // 2380
			Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2548); // 2556
			Game::Load_XString(false);
		}

		if (Zones::ZoneVersion >= 359)
		{
			if (*reinterpret_cast<DWORD*>(varWeaponCompleteDef + 2524) == -1) 
			{
				void* vec2 = Game::DB_AllocStreamPos(3);
				*reinterpret_cast<void**>(varWeaponCompleteDef + 2524) = vec2;

				Game::Load_Stream(true, (void*)vec2, 8 * *reinterpret_cast<short*>(varWeaponCompleteDef + 3044));
			}

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2520);
			Game::Load_XString(false);

			if (*reinterpret_cast<DWORD*>(varWeaponCompleteDef + 2528) == -1)
			{
				void* vec2 = Game::DB_AllocStreamPos(3);
				*reinterpret_cast<void**>(varWeaponCompleteDef + 2528) = vec2;

				Game::Load_Stream(true, (void*)vec2, 8 * *reinterpret_cast<short*>(varWeaponCompleteDef + 3046));
			}
		}
		else
		{
			if (*reinterpret_cast<DWORD*>(varWeaponCompleteDef + 2556) == -1) // 2564
			{
				void* vec2 = Game::DB_AllocStreamPos(3);
				*reinterpret_cast<void**>(varWeaponCompleteDef + 2556) = vec2;

				Game::Load_Stream(true, (void*)vec2, 8 * *reinterpret_cast<short*>(varWeaponCompleteDef + ((Zones::ZoneVersion >= 318) ? 3076 : 3040)));
			}

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2552);
			Game::Load_XString(false);

			if (*reinterpret_cast<DWORD*>(varWeaponCompleteDef + 2560) == -1)
			{
				void* vec2 = Game::DB_AllocStreamPos(3);
				*reinterpret_cast<void**>(varWeaponCompleteDef + 2560) = vec2;

				Game::Load_Stream(true, (void*)vec2, 8 * *reinterpret_cast<short*>(varWeaponCompleteDef + ((Zones::ZoneVersion >= 318) ? 3078 : 3042)));
			}
		}

		if (Zones::ZoneVersion >= 359)
		{
			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2608);
			Game::Load_XString(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2612);
			Game::Load_XString(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2644);
			Game::Load_XString(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2648);
			Game::Load_XString(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2772);
			Game::Load_XString(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2776);
			Game::Load_XString(false);
		}
		else
		{
			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2640);
			Game::Load_XString(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2644);
			Game::Load_XString(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2676);
			Game::Load_XString(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2680);
			Game::Load_XString(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2804);
			Game::Load_XString(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2808);
			Game::Load_XString(false);
		}

		if (Zones::ZoneVersion >= 359)
		{
			*Game::varTracerDefPtr = reinterpret_cast<Game::TracerDef**>(varWeaponCompleteDef + 2780);
			Game::Load_TracerDefPtr(false);

			*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + 2808);
			Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name); // 2848

			*Game::varFxEffectDefHandle = reinterpret_cast<Game::FxEffectDef**>(varWeaponCompleteDef + 2812);
			Game::Load_FxEffectDefHandle(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2816);
			Game::Load_XString(false);

			*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + 2832);
			Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);

			*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + 2836);
			Game::Load_snd_alias_list_nameArray(false, 4);

			*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + 2852);
			Game::Load_snd_alias_list_nameArray(false, 4);

			*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + 2868);
			Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);

			*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + 2872);
			Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);
		}
		else
		{
			*Game::varTracerDefPtr = reinterpret_cast<Game::TracerDef**>(varWeaponCompleteDef + 2812);
			Game::Load_TracerDefPtr(false);

			*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + 2840);
			Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name); // 2848

			*Game::varFxEffectDefHandle = reinterpret_cast<Game::FxEffectDef**>(varWeaponCompleteDef + 2844);
			Game::Load_FxEffectDefHandle(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2848);
			Game::Load_XString(false);

			*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + 2864);
			Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);

			*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + 2868);
			Game::Load_snd_alias_list_nameArray(false, 4);

			*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + 2884);
			Game::Load_snd_alias_list_nameArray(false, 4);

			*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + 2900);
			Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);

			*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + 2904); // 2912
			Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);
		}

		if (Zones::ZoneVersion >= 359)
		{
			for (int i = 0, offset = 2940; i < 6; ++i, offset += 4)
			{
				*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + offset);
				Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);
			}
		}
		else
		{
			if (Zones::ZoneVersion >= 318)
			{
				for (int i = 0, offset = 2972; i < 6; ++i, offset += 4)
				{
					*Game::varsnd_alias_list_name = reinterpret_cast<Game::snd_alias_list_t**>(varWeaponCompleteDef + offset);
					Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);
				}

				varWeaponCompleteDef += (6 * 4);
				varWeaponCompleteDef += 12;
			}
			else
			{

			}
		}

		if (Zones::ZoneVersion >= 359)
		{
			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2988);
			Game::Load_XString(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 3000);
			Game::Load_XString(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 3004);
			Game::Load_XString(false);

			*Game::varMaterialHandle = reinterpret_cast<Game::Material**>(varWeaponCompleteDef + 3012);
			Game::Load_MaterialHandle(false);

			*Game::varMaterialHandle = reinterpret_cast<Game::Material**>(varWeaponCompleteDef + 3016);
			Game::Load_MaterialHandle(false);

			*Game::varMaterialHandle = reinterpret_cast<Game::Material**>(varWeaponCompleteDef + 3020);
			Game::Load_MaterialHandle(false);
		}
		else
		{
			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2984);
			Game::Load_XString(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 2996);
			Game::Load_XString(false);

			*Game::varXString = reinterpret_cast<char**>(varWeaponCompleteDef + 3000);
			Game::Load_XString(false);

			*Game::varMaterialHandle = reinterpret_cast<Game::Material**>(varWeaponCompleteDef + 3008);
			Game::Load_MaterialHandle(false);

			*Game::varMaterialHandle = reinterpret_cast<Game::Material**>(varWeaponCompleteDef + 3012);
			Game::Load_MaterialHandle(false);

			*Game::varMaterialHandle = reinterpret_cast<Game::Material**>(varWeaponCompleteDef + 3016);
			Game::Load_MaterialHandle(false);
		}

		if (Zones::ZoneVersion >= 359)
		{
			if (*reinterpret_cast<DWORD*>(varWeaponCompleteDef + 3048) == -1)
			{
				void* vec2 = Game::DB_AllocStreamPos(3);
				*reinterpret_cast<void**>(varWeaponCompleteDef + 3048) = vec2;

				Game::Load_Stream(true, (void*)vec2, 8 * *reinterpret_cast<short*>(varWeaponCompleteDef + 3044));
			}

			if (*reinterpret_cast<DWORD*>(varWeaponCompleteDef + 3052) == -1)
			{
				void* vec2 = Game::DB_AllocStreamPos(3);
				*reinterpret_cast<void**>(varWeaponCompleteDef + 3052) = vec2;

				Game::Load_Stream(true, (void*)vec2, 8 * *reinterpret_cast<short*>(varWeaponCompleteDef + 3046));
			}
		}
		else
		{
			if (*reinterpret_cast<DWORD*>(varWeaponCompleteDef + 3044) == -1)
			{
				void* vec2 = Game::DB_AllocStreamPos(3);
				*reinterpret_cast<void**>(varWeaponCompleteDef + 3044) = vec2;

				Game::Load_Stream(true, (void*)vec2, 8 * *reinterpret_cast<short*>(varWeaponCompleteDef + 3040));
			}

			if (*reinterpret_cast<DWORD*>(varWeaponCompleteDef + 3048) == -1)
			{
				void* vec2 = Game::DB_AllocStreamPos(3);
				*reinterpret_cast<void**>(varWeaponCompleteDef + 3048) = vec2;

				Game::Load_Stream(true, (void*)vec2, 8 * *reinterpret_cast<short*>(varWeaponCompleteDef + 3042));
			}
		}

		Game::DB_PopStreamPos();
	}

	bool Zones::LoadGameWorldSp(bool atStreamStart, char* buffer)
	{
		bool result = Game::Load_Stream(atStreamStart, buffer, 84);

		Game::GameWorldSp world[2];
		std::memcpy(&world, buffer, 44);
		std::memcpy(&world[1], &buffer[44], 28);
		std::memcpy(&world->vehicleTrack, &buffer[72], 12);

		std::memcpy(buffer, world, 84);

		return result;
	}

	void Zones::LoadPathDataTail()
	{
		char* varPathData = reinterpret_cast<char*>(*Game::varPathData);

		if (*reinterpret_cast<char**>(varPathData + 56))
		{
			*reinterpret_cast<char**>(varPathData + 56) = Game::DB_AllocStreamPos(0);
			Game::Load_Stream(true, *reinterpret_cast<char**>(varPathData + 56), *reinterpret_cast<int*>(varPathData + 52));
		}

		if (*reinterpret_cast<char**>(varPathData + 64))
		{
			*reinterpret_cast<char**>(varPathData + 64) = Game::DB_AllocStreamPos(0);
			Game::Load_Stream(true, *reinterpret_cast<char**>(varPathData + 64), *reinterpret_cast<int*>(varPathData + 60));
		}

		if (*reinterpret_cast<char**>(varPathData + 76))
		{
			*reinterpret_cast<char**>(varPathData + 76) = Game::DB_AllocStreamPos(0);
			Game::Load_Stream(true, *reinterpret_cast<char**>(varPathData + 76), *reinterpret_cast<int*>(varPathData + 72));
		}
	}

	bool Zones::Loadsnd_alias_tArray(bool atStreamStart, char* buffer, int len)
	{
		len /= 100;
		int count = len;
		len *= 108;

		bool result = Game::Load_Stream(atStreamStart, buffer, len);

		Utils::Memory::Allocator allocator;
		Game::snd_alias_t* tempSounds = allocator.AllocateArray<Game::snd_alias_t>(count);

		for (int i = 0; i < count; ++i)
		{
			char* src = &buffer[i * 108];

			std::memcpy(&tempSounds[i], src + 0, 60);
			std::memcpy(&tempSounds[i].pad2[36], src + 68, 20);
			std::memcpy(&tempSounds[i].pad2[56], src + 88, 20);

			AssetHandler::Relocate(src + 0, buffer + (i * 100) + 0, 60);
			AssetHandler::Relocate(src + 68, buffer + (i * 100) + 60, 20);
			AssetHandler::Relocate(src + 88, buffer + (i * 100) + 80, 20);
		}

		std::memcpy(buffer, tempSounds, sizeof(Game::snd_alias_t) * count);

		return result;
	}

	bool Zones::LoadLoadedSound(bool atStreamStart, char* buffer)
	{
		bool result = Game::Load_Stream(atStreamStart, buffer, 48);

		std::memmove(buffer + 28, buffer + 32, 16);
		AssetHandler::Relocate(buffer + 32, buffer + 28, 16);

		return result;
	}

	bool Zones::LoadVehicleDef(bool atStreamStart, char* buffer)
	{
		bool result = Game::Load_Stream(atStreamStart, buffer, 788);

		Game::VehicleDef vehicle[2];
		std::memcpy(vehicle, &buffer[0], 400);
		std::memcpy(&vehicle->pad[404], &buffer[400], 388);

		AssetHandler::Relocate(buffer + 400, buffer + 408, 388);

		std::memcpy(buffer, vehicle, 788);

		return result;
	}

	void Zones::LoadWeaponAttachStuff(DWORD* varWeaponAttachStuff, int count)
	{
		Game::Load_Stream(true, varWeaponAttachStuff, 12 * count);

		for (int i = 0; i < count; ++i)
		{
			if (varWeaponAttachStuff[1] < 16 || varWeaponAttachStuff[1] == 39)
			{
				if (varWeaponAttachStuff[2] == -1)
				{
					varWeaponAttachStuff[2] = reinterpret_cast<DWORD>(Game::DB_AllocStreamPos(0));
					*Game::varConstChar = reinterpret_cast<const char*>(varWeaponAttachStuff[2]);
					Game::Load_XStringCustom(Game::varConstChar);
				}
			}

			varWeaponAttachStuff += 3;
		}
	}

	bool Zones::LoadmenuDef_t(bool atStreamStart, char* buffer, int size)
	{
		if (Zones::ZoneVersion < 359) size += 4;

		bool result = Game::Load_Stream(atStreamStart, buffer, size);
		std::memmove(buffer + 168, buffer + 172, (Zones::ZoneVersion < 359 ? 232 : 228));
		AssetHandler::Relocate(buffer + 172, buffer + 168, (Zones::ZoneVersion < 359 ? 232 : 228));

		return result;
	}

	void Zones::LoadWeaponAttach()
	{
		// setup structures we use
		char* varWeaponAttach = *reinterpret_cast<char**>(0x112ADE0); // varAddonMapEnts

		// and do the stuff
		Game::Load_Stream(true, varWeaponAttach, 12);

		Game::DB_PushStreamPos(3);

		*Game::varXString = reinterpret_cast<char**>(varWeaponAttach);
		Game::Load_XString(false);

		*reinterpret_cast<void**>(varWeaponAttach + 8) = Game::DB_AllocStreamPos(3);
		Zones::LoadWeaponAttachStuff(*reinterpret_cast<DWORD**>(varWeaponAttach + 8), *reinterpret_cast<int*>(varWeaponAttach + 4));

		Game::DB_PopStreamPos();
	}

	bool Zones::LoadMaterialShaderArgumentArray(bool atStreamStart, Game::MaterialShaderArgument* argument, int size)
	{
		bool result = Game::Load_Stream(atStreamStart, argument, size);

		Game::MaterialPass* curPass = *Game::varMaterialPass;
		int count = curPass->argCount1 + curPass->argCount2 + curPass->argCount3;

		for (int i = 0; i < count; ++i)
		{
			Game::MaterialShaderArgument* arg = &argument[i];

			if (arg->type != 3 && arg->type != 5)
			{
				continue;
			}

			// should be min 68 currently
			// >= 58 fixes foliage without bad side effects
			// >= 53 still has broken shadow mapping
			// >= 23 is still broken somehow
			if (arg->paramID >= 58 && arg->paramID <= 135) // >= 34 would be 31 in iw4 terms
			{
				arg->paramID -= 3;
			}
			// >= 21 works fine for specular, but breaks trees
			// >= 4 is too low, breaks specular
			else if (arg->paramID >= 11 && arg->paramID < 58)
			{
				arg->paramID -= 2;
			}
		}

		return result;
	}

	bool Zones::LoadStructuredDataStructPropertyArray(bool atStreamStart, char* data, int size)
	{
		size /= 16;
		int count = size;
		size *= 24;

		bool result = Game::Load_Stream(atStreamStart, data, size);

		for (int i = 0; i < count; ++i)
		{
			std::memmove(data + (i * 16), data + (i * 24), 16);
			AssetHandler::Relocate(data + (i * 24), data + (i * 16), 16);
		}

		return result;
	}

	bool Zones::LoadGfxImage(bool atStreamStart, char* buffer, int size)
	{
		size = (Zones::ZoneVersion >= 359) ? 52 : 36;
		bool result = Game::Load_Stream(atStreamStart, buffer, size);

		AssetHandler::Relocate(buffer + (size - 4), buffer + 28, 4);

		if (Zones::Version() >= 359)
		{
			struct
			{
				Game::GfxImageLoadDef* texture;
				char mapType;
				char semantic;
				char category;
				char flags;
				int cardMemory;
				char pad[8]; // ?
				int dataLen1;
				int dataLen2;
				char pad2[4]; // ?
				short height;
				short width;
				short depth;
				char loaded;
				char pad3[5];
				Game::GfxImageLoadDef* storedTexture;
				char* name;
			} image359;

			Assert_Size(image359, 52);

			// Copy to new struct
			memcpy(&image359, buffer, sizeof(image359));

			// Convert to old struct
			Game::GfxImage* image = reinterpret_cast<Game::GfxImage*>(buffer);
			image->mapType = image359.mapType;
			image->semantic = image359.semantic;
			image->category = image359.category;
			image->flags = image359.flags;
			image->cardMemory = image359.cardMemory;
			image->dataLen1 = image359.dataLen1;
			image->dataLen2 = image359.dataLen2;
			image->height = image359.height;
			image->width = image359.width;
			image->depth = image359.depth;
			image->loaded = image359.loaded;
			image->name = image359.name;

			// Used for later stuff
			image->pad = image359.pad3[1];
		}
		else
		{
			memcpy(buffer + 28, buffer + (size - 4), 4);
		}

		return result;
	}

	Utils::Hook LoadTextureHook;

	int LoadTexture(Game::GfxImageLoadDef **loadDef, Game::GfxImage *image)
	{
		if (Zones::Version() >= 359 && (((image->pad & 1) && !(image->pad & 2))))
		{
			image->loaded = 1;
			image->texture = 0;
			return 1;
		}

		return Game::Load_Texture(loadDef, image);
	}

	bool Zones::LoadXAsset(bool atStreamStart, char* buffer, int size)
	{
		size /= 8;
		int count = size;
		size *= 16;

		bool result = Game::Load_Stream(atStreamStart, buffer, size);

		Utils::Memory::Allocator allocator;
		Game::XAsset* tempAssets = allocator.AllocateArray<Game::XAsset>(count);

		for (int i = 0; i < count; ++i)
		{
			char* src = &buffer[i * 16];

			std::memcpy(&tempAssets[i].type, src + 0, 4);
			std::memcpy(&tempAssets[i].header, src + 8, 4);

			AssetHandler::Relocate(src + 0, buffer + (i * 8) + 0, 4);
			AssetHandler::Relocate(src + 8, buffer + (i * 8) + 4, 4);
		}

		std::memcpy(buffer, tempAssets, sizeof(Game::XAsset) * count);

		return result;
	}

	bool Zones::LoadMaterialTechnique(bool atStreamStart, char* buffer, int size)
	{
		bool result = Game::Load_Stream(atStreamStart, buffer, size + 4);

		int shiftTest = 4;
		std::memmove(buffer + 8 + shiftTest, buffer + 12 + shiftTest, 196 - shiftTest);
		AssetHandler::Relocate(buffer + 12 + shiftTest, buffer + 8 + shiftTest, 196 - shiftTest);

		return result;
	}

	bool Zones::LoadMaterial(bool atStreamStart, char* buffer, int size)
	{
		bool result = Game::Load_Stream(atStreamStart, buffer, size);

		struct material339_s
		{
			char drawSurfBegin[8]; // 4
			//int surfaceTypeBits;
			const char *name;
			char drawSurf[6];
			char gameFlags;
			char pad;
			char sortKey;
			char textureAtlasRowCount;
			char textureAtlasColumnCount;
		} material359;

		static_assert(offsetof(material339_s, gameFlags) == 18, "");
		static_assert(offsetof(material339_s, sortKey) == 20, "");
		static_assert(offsetof(material339_s, textureAtlasColumnCount) == 22, "");

		static_assert(offsetof(Game::Material, stateBitsEntry) == 24, "");

		Game::Material* material = (Game::Material*)buffer;
		memcpy(&material359, material, sizeof(material359));

		material->name = material359.name;
		material->sortKey = material359.sortKey;
		material->textureAtlasRowCount = material359.textureAtlasRowCount;
		material->textureAtlasColumnCount = material359.textureAtlasColumnCount;
		material->gameFlags = material359.gameFlags;

		// Probably wrong
		material->surfaceTypeBits = 0;//material359.surfaceTypeBits;

		// Pretty sure that's wrong
		// Actually, it's not
		memcpy(material->drawSurf, material359.drawSurfBegin, 8);

		material->drawSurf[8] = material359.drawSurf[0];
		material->drawSurf[9] = material359.drawSurf[1];
		material->drawSurf[10] = material359.drawSurf[2];
		material->drawSurf[11] = material359.drawSurf[3];

		return result;
	}

	bool Zones::LoadGfxWorld(bool atStreamStart, char* buffer, int size)
	{
		bool result = Game::Load_Stream(atStreamStart, buffer, size + 968);

		int sunDiff = 8; // Stuff that is part of the sunflare we would overwrite
		std::memmove(buffer + 348 + sunDiff, buffer + 1316 + sunDiff, 280 - sunDiff);
		AssetHandler::Relocate(buffer + 1316, buffer + 348, 280);

		return result;
	}

	void Zones::Loadsunflare_t(bool atStreamStart)
	{
		Game::Load_MaterialHandle(atStreamStart);

		char* varsunflare_t = *reinterpret_cast<char**>(0x112A848);

		*Game::varMaterialHandle = reinterpret_cast<Game::Material**>(varsunflare_t + 12);
		Game::Load_MaterialHandle(atStreamStart);

		*Game::varMaterialHandle = reinterpret_cast<Game::Material**>(varsunflare_t + 16);
		Game::Load_MaterialHandle(atStreamStart);

		std::memmove(varsunflare_t + 12, varsunflare_t + 20, 84);

		// Copy the remaining struct data we couldn't copy in LoadGfxWorld
		char* varGfxWorld = *reinterpret_cast<char**>(0x112A7F4);
		std::memmove(varGfxWorld + 348, varGfxWorld + 1316, 8);
	}

	void Zones::InstallPatches(int version)
	{
		AssetHandler::ClearRelocations();

		if (Zones::ZoneVersion == version) return;
		Zones::ZoneVersion = version;

		bool patch = (version >= VERSION_ALPHA2);
		if (Zones::ZoneVersion == VERSION_ALPHA2 || Zones::ZoneVersion == VERSION_ALPHA3 || Zones::ZoneVersion == VERSION_ALPHA3_DEC || Zones::ZoneVersion == XFILE_VERSION || Zones::ZoneVersion >= 332)
		{
			Utils::Hook::Set<DWORD>(0x4158F4, version);
			Utils::Hook::Set<DWORD>(0x4158FB, version);
		}

		// physpreset size
		Utils::Hook::Set<BYTE>(0x49CE0A, (patch) ? 68 : 44);

		// XModel size
		Utils::Hook::Set<DWORD>(0x410D8A, (patch) ? ((Zones::ZoneVersion == VERSION_ALPHA2) ? 0x16C : 0x168) : 0x130);

		// XSurface size
		Utils::Hook::Set<BYTE>(0x48E84A, (patch) ? 48 : 36);

		// impactfx internal size/count
		Utils::Hook::Set<DWORD>(0x4447B6, (patch) ? 0x8C0 : 0x834);
		Utils::Hook::Set<DWORD>(0x4447D1, (patch) ? 16 : 15);

		// GameWorldSp asset type
		Utils::Hook::Set<BYTE>(0x41899A, (patch) ? 18 : 17);

		// PathData internal struct size
		Utils::Hook::Set<DWORD>(0x4D6A04, (patch) ? 148 : 136);
		Utils::Hook::Set<DWORD>(0x4D6A49, (patch) ? 148 : 136);

		// PathData internal struct data size
		Utils::Hook::Set<WORD>(0x463D63, (patch) ? 0x9090 : 0x048D);
		Utils::Hook::Set<BYTE>(0x463D65, (patch) ? 0x90 : 0x40);
		Utils::Hook::Set<DWORD>(0x463D66, (patch) ? 0x9004E0C1 : 0xC003C003); // shl eax, 4 instead of add eax, eax * 2

		// addon_map_ents asset type (we reuse it for weaponattach)
		Utils::Hook::Set<BYTE>(0x418B30, (patch) ? 43 : Game::ASSET_TYPE_ADDON_MAP_ENTS);

		// Change block for image load defs
		Utils::Hook::Set<BYTE>(0x4D3224, ((Zones::ZoneVersion >= 332) ? 3 : 0));

		// This is needed if we want the original lightning on cargoship_sh, but original maps are darker
		//Utils::Hook::Set<BYTE>(0x525333, 61);

		if (patch)
		{
			Zones::LoadFxElemDefArrayHook.Install();
			Zones::LoadFxElemDefHook.Install();

			Zones::LoadXModelLodInfoHook.Install();
			Zones::LoadXModelHook.Install();

			Zones::LoadXSurfaceArrayHook.Install();
			Zones::LoadGameWorldSpHook.Install();
			Zones::LoadPathDataTailHook.Install();

			Zones::LoadWeaponCompleteDefHook.Install();
			Zones::LoadVehicleDefHook.Install();

			Zones::Loadsnd_alias_tArrayHook.Install();
			Zones::LoadLoadedSoundHook.Install();
			Zones::LoadmenuDef_tHook.Install();
			Zones::LoadFxEffectDefHook.Install();

			Zones::LoadWeaponAttachHook.Install();

			if (Zones::ZoneVersion >= VERSION_ALPHA3)
			{
				Zones::LoadPathDataHook.Install();
			}
			else
			{
				Zones::LoadPathDataHook.Uninstall();
			}

			if (Zones::ZoneVersion >= 332)
			{
				Zones::LoadGfxImageHook.Install();
			}
			else
			{
				Zones::LoadGfxImageHook.Uninstall();
			}

			if (Zones::ZoneVersion >= 359)
			{
				Zones::LoadXAssetHook.Install();
				Zones::LoadMaterialTechniqueHook.Install();
				Zones::LoadMaterialHook.Install();
				Zones::LoadGfxWorldHook.Install();
				Zones::Loadsunflare_tHook.Install();

				LoadTextureHook.Install();

				// menu stuff
				Utils::Hook::Nop(0x41A590, 5);
			}
			else
			{
				Zones::LoadXAssetHook.Uninstall();
				Zones::LoadMaterialTechniqueHook.Uninstall();
				Zones::LoadMaterialHook.Uninstall();
				Zones::LoadGfxWorldHook.Uninstall();
				Zones::Loadsunflare_tHook.Uninstall();

				LoadTextureHook.Uninstall();

				Utils::Hook(0x41A590, 0x4AF680, HOOK_CALL).Install()->Quick();
			}

			Zones::LoadMaterialShaderArgumentArrayHook.Install();
			Zones::LoadStructuredDataStructPropertyArrayHook.Install();
		}
		else
		{
			Zones::LoadFxElemDefArrayHook.Uninstall();
			Zones::LoadFxElemDefHook.Uninstall();

			Zones::LoadXModelLodInfoHook.Uninstall();
			Zones::LoadXModelHook.Uninstall();

			Zones::LoadXSurfaceArrayHook.Uninstall();
			Zones::LoadGameWorldSpHook.Uninstall();
			Zones::LoadPathDataTailHook.Uninstall();

			Zones::LoadWeaponCompleteDefHook.Uninstall();
			Zones::LoadVehicleDefHook.Uninstall();

			Zones::Loadsnd_alias_tArrayHook.Uninstall();
			Zones::LoadLoadedSoundHook.Uninstall();
			Zones::LoadmenuDef_tHook.Uninstall();
			Zones::LoadFxEffectDefHook.Uninstall();

			Zones::LoadWeaponAttachHook.Uninstall();

			Zones::LoadPathDataHook.Uninstall();

			Zones::LoadMaterialShaderArgumentArrayHook.Uninstall();
			Zones::LoadStructuredDataStructPropertyArrayHook.Uninstall();

			Zones::LoadGfxImageHook.Uninstall();

			Zones::LoadXAssetHook.Uninstall();
			Zones::LoadMaterialTechniqueHook.Uninstall();
			Zones::LoadMaterialHook.Uninstall();
			Zones::LoadGfxWorldHook.Uninstall();
			Zones::Loadsunflare_tHook.Uninstall();

			LoadTextureHook.Uninstall();

			Utils::Hook(0x41A590, 0x4AF680, HOOK_CALL).Install()->Quick();
		}

		AntiCheat::EmptyHash();
	}

	int ___test()
	{
		return 0;
	}

	Zones::Zones()
	{
// 		Utils::Hook(0x525A90, ___test, HOOK_JUMP).Install()->Quick();
// 		Utils::Hook(0x50C4F0, ___test, HOOK_JUMP).Install()->Quick();
// 		Utils::Hook(0x514F90, ___test, HOOK_JUMP).Install()->Quick();
// 		Utils::Hook(0x54A2E0, ___test, HOOK_JUMP).Install()->Quick();

		Zones::ZoneVersion = 0;

		// Ignore missing soundaliases for now
		// TODO: Include them in the dependency zone!
		Utils::Hook::Nop(0x644207, 5);

		//Utils::Hook::Nop(0x50AAFE, 5); 
		//Utils::Hook::Nop(0x51B4A6, 5);

		// Block Mark_pathnode_constant_t
		Utils::Hook::Set<BYTE>(0x4F74B0, 0xC3);

		Zones::LoadFxElemDefArrayHook.Initialize(0x495938, Zones::LoadFxElemDefArrayStub, HOOK_CALL);
		Zones::LoadFxElemDefHook.Initialize(0x45ADA0, Zones::LoadFxElemDefStub, HOOK_CALL);
		Zones::LoadXModelLodInfoHook.Initialize(0x4EA6FE, Zones::LoadXModelLodInfoStub, HOOK_CALL);
		Zones::LoadXModelHook.Initialize(0x410D90, Zones::LoadXModel, HOOK_CALL);
		Zones::LoadXSurfaceArrayHook.Initialize(0x4925C8, Zones::LoadXSurfaceArray, HOOK_CALL);
		Zones::LoadGameWorldSpHook.Initialize(0x4F4D0D, Zones::LoadGameWorldSp, HOOK_CALL);
		Zones::LoadWeaponCompleteDefHook.Initialize(0x47CCD2, Zones::LoadWeaponCompleteDef, HOOK_CALL);
		Zones::LoadVehicleDefHook.Initialize(0x483DA0, Zones::LoadVehicleDef, HOOK_CALL);
		Zones::Loadsnd_alias_tArrayHook.Initialize(0x4F0AC8, Zones::Loadsnd_alias_tArray, HOOK_CALL);
		Zones::LoadLoadedSoundHook.Initialize(0x403A5D, Zones::LoadLoadedSound, HOOK_CALL);
		Zones::LoadWeaponAttachHook.Initialize(0x463022, Zones::LoadWeaponAttach, HOOK_CALL);
		Zones::LoadmenuDef_tHook.Initialize(0x41A570, Zones::LoadmenuDef_t, HOOK_CALL);
		Zones::LoadFxEffectDefHook.Initialize(0x49591B, Zones::LoadFxEffectDef, HOOK_CALL);
		Zones::LoadMaterialShaderArgumentArrayHook.Initialize(0x428F0A, Zones::LoadMaterialShaderArgumentArray, HOOK_CALL);
		Zones::LoadStructuredDataStructPropertyArrayHook.Initialize(0x4B1EB8, Zones::LoadStructuredDataStructPropertyArray, HOOK_CALL);

		Zones::LoadGfxImageHook.Initialize(0x4471AD, Zones::LoadGfxImage, HOOK_CALL);

		Zones::LoadXAssetHook.Initialize(0x5B9AA5, Zones::LoadXAsset, HOOK_CALL);
		Zones::LoadMaterialTechniqueHook.Initialize(0x461710, Zones::LoadMaterialTechnique, HOOK_CALL);
		Zones::LoadMaterialHook.Initialize(0x40330D, Zones::LoadMaterial, HOOK_CALL);
		Zones::LoadGfxWorldHook.Initialize(0x4B8DC0, Zones::LoadGfxWorld, HOOK_CALL);
		Zones::Loadsunflare_tHook.Initialize(0x4B8FF5, Zones::Loadsunflare_t, HOOK_CALL);

		Zones::LoadPathDataTailHook.Initialize(0x427A1B, Zones::LoadPathDataTail, HOOK_JUMP);
		Zones::LoadPathDataHook.Initialize(0x4F4D3B, [] ()
		{
			ZeroMemory(*Game::varPathData, sizeof(Game::PathData));
		}, HOOK_CALL);

		LoadTextureHook.Initialize(0x4D32BC, LoadTexture, HOOK_CALL);
	}

	Zones::~Zones()
	{

	}
}
