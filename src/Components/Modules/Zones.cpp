#include "STDInclude.hpp"

namespace Components
{
	int Zones::ZoneVersion;

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

	Utils::Hook fxEffectLoadHook;

	static char* fxEffectStringValue[64];
	static int fxEffectIndex = 0;

	void FxEffectLoadHookFunc(int a1, char* buffer, int len)
	{
		len /= 252;

		int count = len;
		len *= 260;

		__asm
		{
			push len
			push buffer
			push a1
			call fxEffectLoadHook.Original
			add esp, 0Ch
		}

		fxEffectIndex = 0;
		char* tempVar = new char[len];

		for (int i = 0; i < count; i++)
		{
			AssetHandler::Relocate((DWORD)buffer + (260 * i), 252, (DWORD)buffer + (252 * i));

			std::memcpy(tempVar + (252 * i), buffer + (260 * i), 252);

			fxEffectStringValue[i] = *(char**)(buffer + (260 * i) + 256);
		}

		std::memcpy(buffer, tempVar, len);

		delete[] tempVar;
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
		*Game::varXString = &fxEffectStringValue[fxEffectIndex++];
		Game::Load_XString(0);
	}

	bool Zones::LoadXModel(bool atStreamStart, char* xmodel, int size)
	{
		bool result = Game::Load_Stream(atStreamStart, xmodel, size);

		int elSize = (Zones::ZoneVersion == VERSION_ALPHA2) ? 364 : 360;

		Game::XModel model[2]; // Allocate 2 models, as we exceed the buffer

		std::memcpy(model, xmodel, 36);
		std::memcpy(&model->pad3[0x1C], &xmodel[44], 28);

		for (int i = 0; i < 4; i++)
		{
			std::memcpy(&model->lods[i], &xmodel[72 + (i * 56)], 12);
			std::memcpy(&model->lods[i].pad3, &xmodel[72 + (i * 56) + 16], 32);

			std::memcpy(reinterpret_cast<char*>(&model) + (elSize - 4) - (i * 4), &xmodel[72 + (i * 56) + 12], 4);
		}

		std::memcpy(&model->lods[3].pad4[0], &xmodel[292], (elSize - 292 - 4)/*68*/);
		std::memcpy(&model->physPreset, &xmodel[(elSize - 8)], 8);

		model[1].name = reinterpret_cast<char*>(0xDEC0ADDE);

		std::memcpy(xmodel, &model, elSize);

		return result;
	}

	void Zones::LoadXModelLodInfo(int i)
	{
		int elSize = (Zones::ZoneVersion == VERSION_ALPHA2) ? 364 : 360;
		*Game::varXString = reinterpret_cast<char**>(reinterpret_cast<char*>(*Game::varXModel) + (elSize - 4) - (4 * (4 - i)));
		Game::Load_XString(false);
	}

	void __declspec(naked) Zones::LoadXModelLodInfoStub()
	{
		__asm
		{
			push edi
			call Zones::LoadXModelLodInfo
			pop edi

			jmp Game::Load_XModelSurfsFixup
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
		}

		std::memcpy(buffer, tempSurfaces, sizeof(Game::XSurface) * count);

		return result;
	}

	Utils::Hook loadWeaponDefHook;

	void Load_WeaponDef_CodC(int /*doLoad*/)
	{
		// setup structures we use
		DWORD varWeaponDef = *(DWORD*)0x112A9F4;//*(DWORD*)0x112AE14;

		// and do the stuff
		Game::Load_Stream(1, (void*)varWeaponDef, (Zones::ZoneVersion >= 318) ? 3156 : 3112);

		Game::DB_PushStreamPos(3);

		*Game::varXString = (char**)(varWeaponDef + 0);
		Game::Load_XString(false);

		*Game::varXString = (char**)(varWeaponDef + 4);
		Game::Load_XString(false);

		*Game::varXString = (char**)(varWeaponDef + 8);
		Game::Load_XString(false);

		*Game::varXString = (char**)(varWeaponDef + 12);
		Game::Load_XString(false);

		*Game::varXModelPtr = (Game::XModel*)(varWeaponDef + 16);
		Game::Load_XModelPtr(false);

		for (int i = 0, offset = 20; i < 32; i++, offset += 4)
		{
			*Game::varXModelPtr = (Game::XModel*)(varWeaponDef + offset);
			Game::Load_XModelPtr(false);
		}

		// 148
		for (int offset = 148; offset <= 168; offset += 4)
		{
			*Game::varXModelPtr = (Game::XModel*)(varWeaponDef + offset);
			Game::Load_XModelPtr(false);
		}

		// 172
		// 32 scriptstrings, should not need to be loaded

		// 236
		*Game::varXString = (char**)(varWeaponDef + 236);
		Game::Load_XStringArray(false, 48);

		// 428
		*Game::varXString = (char**)(varWeaponDef + 428);
		Game::Load_XStringArray(false, 48);

		// 620
		*Game::varXString = (char**)(varWeaponDef + 620);
		Game::Load_XStringArray(false, 48);

		// 812
		// 16 * 4 scriptstrings

		// 972
		*Game::varFxEffectDefHandle = (Game::FxEffectDef*)(varWeaponDef + 972);;
		Game::Load_FxEffectDefHandle(false);

		*Game::varFxEffectDefHandle = (Game::FxEffectDef*)(varWeaponDef + 976);
		Game::Load_FxEffectDefHandle(false);

		// 980
		// 50 soundalias name references; up to and including 1180
		for (int i = 0, offset = 980; i < 50; i++, offset += 4)
		{
			*Game::varsnd_alias_list_name = (Game::snd_alias_list_t**)(varWeaponDef + offset);
			Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);
		}

		if (Zones::ZoneVersion >= 318)
		{
			for (int i = 0, offset = 1184; i < 2; i++, offset += 4)
			{
				*Game::varsnd_alias_list_name = (Game::snd_alias_list_t**)(varWeaponDef + offset);
				Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);
			}

			varWeaponDef += 8; // to compensate for the 2 in between here
		}

		if (*(DWORD*)(varWeaponDef + 1184))
		{
			if (*(DWORD*)(varWeaponDef + 1184) == -1)
			{
				*(DWORD*)(varWeaponDef + 1184) = (DWORD)Game::DB_AllocStreamPos(3);
				*Game::varsnd_alias_list_name = *(Game::snd_alias_list_t***)(varWeaponDef + 1184);

				Game::Load_snd_alias_list_nameArray(true, 31);
			}
			else
			{
				// full usability requires ConvertOffsetToPointer here
			}
		}

		if (*(DWORD*)(varWeaponDef + 1188))
		{
			if (*(DWORD*)(varWeaponDef + 1188) == -1)
			{
				*(DWORD*)(varWeaponDef + 1188) = (DWORD)Game::DB_AllocStreamPos(3);
				*Game::varsnd_alias_list_name = *(Game::snd_alias_list_t***)(varWeaponDef + 1188);

				Game::Load_snd_alias_list_nameArray(true, 31);
			}
			else
			{
				// full usability requires ConvertOffsetToPointer here
			}
		}

		// 1192
		for (int offset = 1192; offset <= 1204; offset += 4)
		{
			*Game::varFxEffectDefHandle = (Game::FxEffectDef*)(varWeaponDef + offset);
			Game::Load_FxEffectDefHandle(false);
		}

		// 1208
		static int matOffsets1[] = { 1208, 1212, 1428, 1432, 1436, 1440, 1444, 1448, 1456, 1464 };

		for (int i = 0; i < sizeof(matOffsets1) / sizeof(int); i++)
		{
			*Game::varMaterialHandle = (Game::Material*)(varWeaponDef + matOffsets1[i]);
			Game::Load_MaterialHandle(false);
		}

		*Game::varXString = (char**)(varWeaponDef + 1484);
		Game::Load_XString(false);

		*Game::varXString = (char**)(varWeaponDef + 1492);
		Game::Load_XString(false);

		*Game::varXString = (char**)(varWeaponDef + 1508);
		Game::Load_XString(false);

		for (int offset = 1764; offset <= 1776; offset += 4)
		{
			*Game::varMaterialHandle = (Game::Material*)(varWeaponDef + offset);
			Game::Load_MaterialHandle(false);
		}

		*Game::varPhysCollmapPtr = (Game::PhysCollmap*)(varWeaponDef + 1964);
		Game::Load_PhysCollmapPtr(0);

		*Game::varXModelPtr = (Game::XModel*)(varWeaponDef + 2052);
		Game::Load_XModelPtr(0);

		*Game::varFxEffectDefHandle = (Game::FxEffectDef*)(varWeaponDef + 2060);
		Game::Load_FxEffectDefHandle(false);

		*Game::varFxEffectDefHandle = (Game::FxEffectDef*)(varWeaponDef + 2064);
		Game::Load_FxEffectDefHandle(false);

		*Game::varsnd_alias_list_name = (Game::snd_alias_list_t**)(varWeaponDef + 2068);
		Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);

		*Game::varsnd_alias_list_name = (Game::snd_alias_list_t**)(varWeaponDef + 2072);
		Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);

		*Game::varFxEffectDefHandle = (Game::FxEffectDef*)(varWeaponDef + 2336);
		Game::Load_FxEffectDefHandle(false);

		*Game::varFxEffectDefHandle = (Game::FxEffectDef*)(varWeaponDef + 2340);
		Game::Load_FxEffectDefHandle(false);

		*Game::varFxEffectDefHandle = (Game::FxEffectDef*)(varWeaponDef + 2368); // 2376
		Game::Load_FxEffectDefHandle(false);

		*Game::varsnd_alias_list_name = (Game::snd_alias_list_t**)(varWeaponDef + 2372); // 2380
		Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);

		*Game::varXString = (char**)(varWeaponDef + 2548); // 2556
		Game::Load_XString(false);

		if (*(DWORD*)(varWeaponDef + 2556) == -1) // 2564
		{
			DWORD vec2 = (DWORD)Game::DB_AllocStreamPos(3);
			*(DWORD*)(varWeaponDef + 2556) = vec2;

			Game::Load_Stream(1, (void*)vec2, 8 * *(short*)(varWeaponDef + ((Zones::ZoneVersion >= 318) ? 3076 : 3040)));
		}

		*Game::varXString = (char**)(varWeaponDef + 2552);
		Game::Load_XString(false);

		if (*(DWORD*)(varWeaponDef + 2560) == -1)
		{
			DWORD vec2 = (DWORD)Game::DB_AllocStreamPos(3);
			*(DWORD*)(varWeaponDef + 2560) = vec2;

			Game::Load_Stream(1, (void*)vec2, 8 * *(short*)(varWeaponDef + ((Zones::ZoneVersion >= 318) ? 3078 : 3042)));
		}

		*Game::varXString = (char**)(varWeaponDef + 2640);
		Game::Load_XString(false);

		*Game::varXString = (char**)(varWeaponDef + 2644);
		Game::Load_XString(false);

		*Game::varXString = (char**)(varWeaponDef + 2676);
		Game::Load_XString(false);

		*Game::varXString = (char**)(varWeaponDef + 2680);
		Game::Load_XString(false);

		*Game::varXString = (char**)(varWeaponDef + 2804);
		Game::Load_XString(false);

		*Game::varXString = (char**)(varWeaponDef + 2808);
		Game::Load_XString(false);

		*Game::varTracerDefPtr = (Game::TracerDef*)(varWeaponDef + 2812);
		Game::Load_TracerDefPtr(false);

		*Game::varsnd_alias_list_name = (Game::snd_alias_list_t**)(varWeaponDef + 2840);
		Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name); // 2848

		*Game::varFxEffectDefHandle = (Game::FxEffectDef*)(varWeaponDef + 2844);
		Game::Load_FxEffectDefHandle(false);

		*Game::varXString = (char**)(varWeaponDef + 2848);
		Game::Load_XString(false);

		*Game::varsnd_alias_list_name = (Game::snd_alias_list_t**)(varWeaponDef + 2864);
		Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);

		*Game::varsnd_alias_list_name = (Game::snd_alias_list_t**)(varWeaponDef + 2868);
		Game::Load_snd_alias_list_nameArray(false, 4);

		*Game::varsnd_alias_list_name = (Game::snd_alias_list_t**)(varWeaponDef + 2884);
		Game::Load_snd_alias_list_nameArray(false, 4);

		*Game::varsnd_alias_list_name = (Game::snd_alias_list_t**)(varWeaponDef + 2900);
		Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);

		*Game::varsnd_alias_list_name = (Game::snd_alias_list_t**)(varWeaponDef + 2904); // 2912
		Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);

		if (Zones::ZoneVersion >= 318)
		{
			for (int i = 0, offset = 2972; i < 6; i++, offset += 4)
			{
				*Game::varsnd_alias_list_name = (Game::snd_alias_list_t**)(varWeaponDef + offset);
				Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);
			}

			varWeaponDef += (6 * 4);
			varWeaponDef += 12;
		}
		else
		{

		}

		*Game::varXString = (char**)(varWeaponDef + 2984);
		Game::Load_XString(false);

		*Game::varXString = (char**)(varWeaponDef + 2996);
		Game::Load_XString(false);

		*Game::varXString = (char**)(varWeaponDef + 3000);
		Game::Load_XString(false);

		*Game::varMaterialHandle = (Game::Material*)(varWeaponDef + 3008);
		Game::Load_MaterialHandle(false);

		*Game::varMaterialHandle = (Game::Material*)(varWeaponDef + 3012);
		Game::Load_MaterialHandle(false);

		*Game::varMaterialHandle = (Game::Material*)(varWeaponDef + 3016);
		Game::Load_MaterialHandle(false);

		if (*(DWORD*)(varWeaponDef + 3044) == -1)
		{
			DWORD vec2 = (DWORD)Game::DB_AllocStreamPos(3);
			*(DWORD*)(varWeaponDef + 3044) = vec2;

			Game::Load_Stream(1, (void*)vec2, 8 * *(short*)(varWeaponDef + 3040));
		}

		if (*(DWORD*)(varWeaponDef + 3048) == -1)
		{
			DWORD vec2 = (DWORD)Game::DB_AllocStreamPos(3);
			*(DWORD*)(varWeaponDef + 3048) = vec2;

			Game::Load_Stream(1, (void*)vec2, 8 * *(short*)(varWeaponDef + 3042));
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

	Utils::Hook pathDataTailHook;

	void PathDataTailHookFunc()
	{
		DWORD varStuff = *(DWORD*)0x112AD7C;
		DWORD varThing;

		if (*(DWORD*)(varStuff + 56))
		{
			*(DWORD*)(varStuff + 56) = (DWORD)Game::DB_AllocStreamPos(0);
			varThing = *(DWORD*)(varStuff + 56);
			Game::Load_Stream(1, (void*)varThing, *(DWORD*)(varStuff + 52));
		}

		if (*(DWORD*)(varStuff + 64))
		{
			*(DWORD*)(varStuff + 64) = (DWORD)Game::DB_AllocStreamPos(0);
			varThing = *(DWORD*)(varStuff + 64);
			Game::Load_Stream(1, (void*)varThing, *(DWORD*)(varStuff + 60));
		}

		if (*(DWORD*)(varStuff + 76))
		{
			*(DWORD*)(varStuff + 76) = (DWORD)Game::DB_AllocStreamPos(0);
			varThing = *(DWORD*)(varStuff + 76);
			Game::Load_Stream(1, (void*)varThing, *(DWORD*)(varStuff + 72));
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

 		for (int i = 0; i < count; i++)
		{
			char* src = &buffer[i * 108];

			std::memcpy(&tempSounds[i],          src + 0,  60);
			std::memcpy(&tempSounds[i].pad2[36], src + 68, 20);
			std::memcpy(&tempSounds[i].pad2[56], src + 88, 20);

			AssetHandler::Relocate(src + 0,  buffer + (i * 100) + 0,  60);
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

	Utils::Hook loadWeaponAttachHook;

	DWORD varWeaponAttachStuff;

	void Load_WeaponAttachStuff(int count)
	{
		Game::Load_Stream(1, (void*)varWeaponAttachStuff, 12 * count);

		DWORD* varStuff = (DWORD*)varWeaponAttachStuff;

		for (int i = 0; i < count; i++)
		{
			//DWORD* varXString = (DWORD*)0x112B340;

			if (varStuff[1] < 16 || varStuff[1] == 39)
			{
				if (varStuff[2] == -1)
				{
					varStuff[2] = (DWORD)Game::DB_AllocStreamPos(0);
					*Game::varConstChar = (const char*)varStuff[2];
					Game::Load_XStringCustom(Game::varConstChar);

					//if (*useEntryNames)
					{
						//DBG(("wA: %s\n", *varXStringData));
					}
				}
				else if (varStuff[2])
				{
					// meh, no convertin' here
				}
			}

			varStuff += 3;
		}
	}

	Utils::Hook menuDefLoadHook;

	void MenuDefLoadHookFunc(int doLoad, char* buffer, int len)
	{
		len += 4;

		__asm
		{
			push len
			push buffer
			push doLoad
			call menuDefLoadHook.Original
			add esp, 0Ch
		}

		std::memcpy(buffer + 168, buffer + 172, 232);

		AssetHandler::Relocate((DWORD)buffer + 172, 232, (DWORD)buffer + 168);
	}

	void Load_WeaponAttach(int /*doLoad*/)
	{
		// setup structures we use
		DWORD varWeaponAttach = *(DWORD*)0x112ADE0;//*(DWORD*)0x112AE14;
		DWORD* varXString = (DWORD*)0x112B340;

		// and do the stuff
		Game::Load_Stream(1, (void*)varWeaponAttach, 12);

		Game::DB_PushStreamPos(3);

		*varXString = varWeaponAttach + 0;
		Game::Load_XString(false);

		*(void**)(varWeaponAttach + 8) = Game::DB_AllocStreamPos(3);

		varWeaponAttachStuff = *(DWORD*)(varWeaponAttach + 8);
		Load_WeaponAttachStuff(*(int*)(varWeaponAttach + 4));

		Game::DB_PopStreamPos();
	}

	Utils::Hook loadTechniquePassHook;

	void Load_TechniquePassHookFunc(bool atStreamStart, Game::ShaderArgumentDef* pass, size_t size)
	{
		int count = size / 8;

		Game::MaterialPass* curPass = *(Game::MaterialPass**)0x112A960;
		count = curPass->argCount1 + curPass->argCount2 + curPass->argCount3;

		Game::Load_Stream(atStreamStart, pass, size);

		for (int i = 0; i < count; i++)
		{
			Game::ShaderArgumentDef* arg = &pass[i];

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
	}

	Utils::Hook loadStructuredDataChildArrayHook;

	void Load_StructuredDataChildArrayHookFunc(bool atStreamStart, char* data, size_t size)
	{
		int count = size / 16;
		size = count * 24;

		Game::Load_Stream(atStreamStart, data, size);

		for (int i = 0; i < count; i++)
		{
			std::memcpy(data + (i * 16), data + (i * 24), 16);
			AssetHandler::Relocate((DWORD)data + (i * 24), 16, (DWORD)data + (i * 16));
		}
	}

	void Zones::InstallPatches(int version)
	{
		AssetHandler::ClearRelocations();

		if (Zones::ZoneVersion == version) return;
		Zones::ZoneVersion = version;

		bool patch = (version >= VERSION_ALPHA2);
		if (Zones::ZoneVersion == VERSION_ALPHA2 || Zones::ZoneVersion == VERSION_ALPHA3 || Zones::ZoneVersion == XFILE_VERSION)
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

		if (patch)
		{
			Zones::LoadFxElemDefArrayHook.Install();
			Zones::LoadFxElemDefHook.Install();

			Zones::LoadXModelLodInfoHook.Install();
			Zones::LoadXModelHook.Install();

			Zones::LoadXSurfaceArrayHook.Install();
			Zones::LoadGameWorldSpHook.Install();
			pathDataTailHook.Install();

			loadWeaponDefHook.Install();
			Zones::LoadVehicleDefHook.Install();

			Zones::Loadsnd_alias_tArrayHook.Install();
			Zones::LoadLoadedSoundHook.Install();
			menuDefLoadHook.Install();
			fxEffectLoadHook.Install();

			loadWeaponAttachHook.Install();

			if (Zones::ZoneVersion >= VERSION_ALPHA3)
			{
				Zones::LoadPathDataHook.Install();
			}

			loadTechniquePassHook.Install();
			loadStructuredDataChildArrayHook.Install();
		}
		else
		{
			Zones::LoadFxElemDefArrayHook.Uninstall();
			Zones::LoadFxElemDefHook.Uninstall();

			Zones::LoadXModelLodInfoHook.Uninstall();
			Zones::LoadXModelHook.Uninstall();

			Zones::LoadXSurfaceArrayHook.Uninstall();
			Zones::LoadGameWorldSpHook.Uninstall();
			pathDataTailHook.Uninstall();

			loadWeaponDefHook.Uninstall();
			Zones::LoadVehicleDefHook.Uninstall();

			Zones::Loadsnd_alias_tArrayHook.Uninstall();
			Zones::LoadLoadedSoundHook.Uninstall();
			menuDefLoadHook.Uninstall();
			fxEffectLoadHook.Uninstall();

			loadWeaponAttachHook.Uninstall();

			Zones::LoadPathDataHook.Uninstall();

			loadTechniquePassHook.Uninstall();
			loadStructuredDataChildArrayHook.Uninstall();
		}

		AntiCheat::EmptyHash();
	}

	Zones::Zones()
	{
		Zones::ZoneVersion = 0;

		// Ignore missing soundaliases for now
		// TODO: Include them in the dependency zone!
		Utils::Hook::Nop(0x644207, 5);

		// Block Mark_pathnode_constant_t
		Utils::Hook::Set<BYTE>(0x4F74B0, 0xC3);

		Zones::LoadFxElemDefArrayHook.Initialize(0x495938, Zones::LoadFxElemDefArrayStub, HOOK_CALL);
		Zones::LoadFxElemDefHook.Initialize(0x45ADA0, Zones::LoadFxElemDefStub, HOOK_CALL);
		Zones::LoadXModelLodInfoHook.Initialize(0x4EA6FE, Zones::LoadXModelLodInfoStub, HOOK_CALL);
		Zones::LoadXModelHook.Initialize(0x410D90, Zones::LoadXModel, HOOK_CALL);
		Zones::LoadXSurfaceArrayHook.Initialize(0x4925C8, Zones::LoadXSurfaceArray, HOOK_CALL);
		Zones::LoadGameWorldSpHook.Initialize(0x4F4D0D, Zones::LoadGameWorldSp, HOOK_CALL);
		loadWeaponDefHook.Initialize(0x47CCD2, Load_WeaponDef_CodC, HOOK_CALL);
		Zones::LoadVehicleDefHook.Initialize(0x483DA0, Zones::LoadVehicleDef, HOOK_CALL);
		Zones::Loadsnd_alias_tArrayHook.Initialize(0x4F0AC8, Zones::Loadsnd_alias_tArray, HOOK_CALL);
		Zones::LoadLoadedSoundHook.Initialize(0x403A5D, Zones::LoadLoadedSound, HOOK_CALL);
		loadWeaponAttachHook.Initialize(0x463022, Load_WeaponAttach, HOOK_CALL);
		menuDefLoadHook.Initialize(0x41A570, MenuDefLoadHookFunc, HOOK_CALL);
		fxEffectLoadHook.Initialize(0x49591B, FxEffectLoadHookFunc, HOOK_CALL);
		loadTechniquePassHook.Initialize(0x428F0A, Load_TechniquePassHookFunc, HOOK_CALL);
		loadStructuredDataChildArrayHook.Initialize(0x4B1EB8, Load_StructuredDataChildArrayHookFunc, HOOK_CALL);

		pathDataTailHook.Initialize(0x427A1B, PathDataTailHookFunc, HOOK_JUMP);

		Zones::LoadPathDataHook.Initialize(0x4F4D3B, [] ()
		{
			ZeroMemory(*Game::varPathData, sizeof(Game::PathData));
		}, HOOK_CALL);
	}

	Zones::~Zones()
	{

	}
}
