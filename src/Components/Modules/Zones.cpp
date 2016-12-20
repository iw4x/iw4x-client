#include "STDInclude.hpp"

namespace Components
{
	int Zones::ZoneVersion;

	int Zones::FxEffectIndex;
	char* Zones::FxEffectStrings[64];

	bool Zones::LoadFxEffectDef(bool atStreamStart, char* buffer, int size)
	{
		int count = 0;

		if (Zones::Version() >= VERSION_ALPHA2)
		{
			size /= 252;
			count = size;
			size *= 260;
		}

		bool result = Game::Load_Stream(atStreamStart, buffer, size);

		Zones::FxEffectIndex = 0;

		if (Zones::Version() >= VERSION_ALPHA2)
		{
			Utils::Memory::Allocator allocator;
			Game::FxElemDef* elems = allocator.allocateArray<Game::FxElemDef>(count);

			for (int i = 0; i < count; ++i)
			{
				AssetHandler::Relocate(buffer + (260 * i), buffer + (252 * i), 252);
				std::memcpy(&elems[i], buffer + (260 * i), 252);
				Zones::FxEffectStrings[i] = *reinterpret_cast<char**>(buffer + (260 * i) + 256);
			}

			std::memcpy(buffer, elems, sizeof(Game::FxElemDef) * count);
		}

		return result;
	}

	bool Zones::LoadFxElemDefStub(bool atStreamStart, Game::FxElemDef* fxElem, int size)
	{
		if (Zones::Version() >= VERSION_ALPHA2)
		{
			if (fxElem->elemType == 3)
			{
				fxElem->elemType = 2;
			}
			else if (fxElem->elemType >= 5)
			{
				fxElem->elemType -= 2;
			}
		}

		return Game::Load_Stream(atStreamStart, fxElem, size);
	}

	void Zones::LoadFxElemDefArrayStub(bool atStreamStart)
	{
		Game::Load_FxElemDef(atStreamStart);

		if (Zones::Version() >= VERSION_ALPHA2)
		{
			*Game::varXString = &Zones::FxEffectStrings[Zones::FxEffectIndex++];
			Game::Load_XString(false);
		}
	}

	bool Zones::LoadXModel(bool atStreamStart, char* xmodel, int size)
	{
		if (Zones::Version() >= VERSION_ALPHA2)
		{
			if (Zones::Version() == VERSION_ALPHA2)
			{
				size = 0x16C;
			}
			else
			{
				size = 0x168;
			}
		}

		bool result = Game::Load_Stream(atStreamStart, xmodel, size);

		if (Zones::Version() >= VERSION_ALPHA2)
		{
			Game::XModel model[2]; // Allocate 2 models, as we exceed the buffer

			std::memcpy(model, xmodel, 36);
			std::memcpy(&model->pad3[0x1C], &xmodel[44], 28);

			for (int i = 0; i < 4; ++i)
			{
				AssertOffset(Game::XModelLodInfo, partBits, 12);

				std::memcpy(&model->lods[i], &xmodel[72 + (i * 56)], 12);
				std::memcpy(&model->lods[i].partBits, &xmodel[72 + (i * 56) + 16], 32);

				std::memcpy(reinterpret_cast<char*>(&model) + (size - 4) - (i * 4), &xmodel[72 + (i * 56) + 12], 4);
			}

			std::memcpy(&model->lods[3].pad4[0], &xmodel[292], (size - 292 - 4)/*68*/);
			std::memcpy(&model->physPreset, &xmodel[(size - 8)], 8);

			model[1].name = reinterpret_cast<char*>(0xDEADC0DE);

			std::memcpy(xmodel, &model, size);
		}

		return result;
	}

	void Zones::LoadXModelLodInfo(int i)
	{
		if (Zones::Version() >= VERSION_ALPHA2)
		{
			int elSize = (Zones::ZoneVersion == VERSION_ALPHA2) ? 364 : 360;
			*Game::varXString = reinterpret_cast<char**>(reinterpret_cast<char*>(*Game::varXModel) + (elSize - 4) - (4 * (4 - i)));
			Game::Load_XString(false);
		}
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
		int count = 0;
		
		if (Zones::Version() >= VERSION_ALPHA2)
		{
			size >>= 6;

			count = size;
			size *= 84;
		}

		bool result = Game::Load_Stream(atStreamStart, buffer, size);

		if (Zones::Version() >= VERSION_ALPHA2)
		{
			Utils::Memory::Allocator allocator;
			Game::XSurface* tempSurfaces = allocator.allocateArray<Game::XSurface>(count);

			for (int i = 0; i < count; ++i)
			{
				char* source = &buffer[i * 84];

				std::memcpy(&tempSurfaces[i], source, 12);
				std::memcpy(&tempSurfaces[i].indexBuffer, source + 16, 20);
				std::memcpy(&tempSurfaces[i].numCT, source + 40, 8);
				std::memcpy(&tempSurfaces[i].partBits, source + 52, 24);

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
		}

		return result;
	}

	void Zones::LoadWeaponCompleteDef()
	{
		if (Zones::ZoneVersion < VERSION_ALPHA2)
		{
			return Utils::Hook::Call<void(bool)>(0x4AE7B0)(true);
		}

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

					if (Zones::ZoneVersion >= 365)
					{
						size = 3124;
					}
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
			for (int offset = 20; offset <= 56; offset += 4)
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
			for (int i = 0, offset = 916; i < 52; ++i, offset += 4)
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

			if (Zones::ZoneVersion >= 365)
			{
				varWeaponCompleteDef += 4;
			}

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

// Code-analysis has a bug, the first memcpy makes it believe size of tempVar is 44 instead of 84
#pragma warning(push)
#pragma warning(disable: 6385)
	bool Zones::LoadGameWorldSp(bool atStreamStart, char* buffer, int size)
	{
		if (Zones::Version() >= VERSION_ALPHA2)
		{
			size = 84;
		}

		bool result = Game::Load_Stream(atStreamStart, buffer, size);

		if (Zones::Version() >= VERSION_ALPHA2)
		{
			char tempVar[84] = { 0 };
			std::memcpy(&tempVar[0], &buffer[0], 44);
			std::memcpy(&tempVar[56], &buffer[44], 28);
			std::memcpy(&tempVar[44], &buffer[72], 12);

			std::memcpy(buffer, tempVar, sizeof(tempVar));
		}

		return result;
	}
#pragma warning(pop)

	void Zones::LoadPathDataTail()
	{
		if (Zones::ZoneVersion >= VERSION_ALPHA2)
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
	}

	bool Zones::Loadsnd_alias_tArray(bool atStreamStart, char* buffer, int len)
	{
		int count = 0;

		if (Zones::Version() >= VERSION_ALPHA2)
		{
			len /= 100;
			count = len;
			len *= 108;
		}

		bool result = Game::Load_Stream(atStreamStart, buffer, len);

		if (Zones::Version() >= VERSION_ALPHA2)
		{
			Utils::Memory::Allocator allocator;
			Game::snd_alias_t* tempSounds = allocator.allocateArray<Game::snd_alias_t>(count);

			for (int i = 0; i < count; ++i)
			{
				char* src = &buffer[i * 108];
				char* dest = reinterpret_cast<char*>(&tempSounds[i]);

				std::memcpy(dest + 0, src + 0, 60);
				std::memcpy(dest + 60, src + 68, 20);
				std::memcpy(dest + 80, src + 88, 20);

				AssetHandler::Relocate(src + 0, buffer + (i * 100) + 0, 60);
				AssetHandler::Relocate(src + 68, buffer + (i * 100) + 60, 20);
				AssetHandler::Relocate(src + 88, buffer + (i * 100) + 80, 20);
			}

			std::memcpy(buffer, tempSounds, sizeof(Game::snd_alias_t) * count);
		}

		return result;
	}

	bool Zones::LoadLoadedSound(bool atStreamStart, char* buffer, int size)
	{
		if (Zones::Version() >= VERSION_ALPHA2)
		{
			size = 48;
		}

		bool result = Game::Load_Stream(atStreamStart, buffer, size);

		if (Zones::Version() >= VERSION_ALPHA2)
		{
			std::memmove(buffer + 28, buffer + 32, 16);
			AssetHandler::Relocate(buffer + 32, buffer + 28, 16);
		}

		return result;
	}

// Code-analysis has a bug, the first memcpy makes it believe size of tempVar is 400 instead of 788
#pragma warning(push)
#pragma warning(disable: 6385)
	bool Zones::LoadVehicleDef(bool atStreamStart, char* buffer, int size)
	{
		if (Zones::Version() >= VERSION_ALPHA2)
		{
			size = 788;
		}

		bool result = Game::Load_Stream(atStreamStart, buffer, size);

		if (Zones::Version() >= VERSION_ALPHA2)
		{
			char tempVar[788] = { 0 };
			std::memcpy(&tempVar[0], &buffer[0], 400);
			std::memcpy(&tempVar[408], &buffer[400], 380);

			AssetHandler::Relocate(buffer + 400, buffer + 408, 388);

			std::memmove(buffer, tempVar, sizeof(tempVar));
		}

		return result;
	}
#pragma warning(pop)

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
		if (Zones::ZoneVersion != 359 && Zones::ZoneVersion >= VERSION_ALPHA2) size += 4;

		bool result = Game::Load_Stream(atStreamStart, buffer, size);

		if (Zones::ZoneVersion >= VERSION_ALPHA2)
		{
			std::memmove(buffer + 168, buffer + 172, (Zones::ZoneVersion != 359 ? 232 : 228));
			AssetHandler::Relocate(buffer + 172, buffer + 168, (Zones::ZoneVersion != 359 ? 232 : 228));

			reinterpret_cast<Game::menuDef_t*>(buffer)->expressionData = nullptr;
		}

		return result;
	}

	void Zones::LoadWeaponAttach()
	{
		if (Zones::ZoneVersion < VERSION_ALPHA2)
		{
			return Utils::Hook::Call<void(bool)>(0x4F4160)(true);
		}

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

		for (int i = 0; i < count && (Zones::ZoneVersion >= VERSION_ALPHA2); ++i)
		{
			Game::MaterialShaderArgument* arg = &argument[i];

			if (arg->type != D3DSHADER_PARAM_REGISTER_TYPE::D3DSPR_TEXTURE && arg->type != D3DSHADER_PARAM_REGISTER_TYPE::D3DSPR_ATTROUT)
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

				if (Zones::Version() >= 359/* && arg->paramID <= 113*/)
				{
					arg->paramID -= 7;

					if (arg->paramID <= 53)
					{
						arg->paramID += 1;
					}
				}
			}
			// >= 21 works fine for specular, but breaks trees
			// >= 4 is too low, breaks specular
			else if (arg->paramID >= 11 && arg->paramID < 58)
			{
				arg->paramID -= 2;

				if (Zones::Version() >= 359)
				{
					if (arg->paramID > 15 && arg->paramID < 30)
					{
						arg->paramID -= 1;

						if (arg->paramID == 19)
						{
							arg->paramID = 21;
						}
					}
					else if (arg->paramID >= 50)
					{
						arg->paramID += 6;
					}
				}
			}
		}

		return result;
	}

	bool Zones::LoadStructuredDataStructPropertyArray(bool atStreamStart, char* data, int size)
	{
		int count = 0;

		if (Zones::ZoneVersion >= VERSION_ALPHA2)
		{
			size /= 16;
			count = size;
			size *= 24;
		}

		bool result = Game::Load_Stream(atStreamStart, data, size);

		if (Zones::ZoneVersion >= VERSION_ALPHA2)
		{
			for (int i = 0; i < count; ++i)
			{
				std::memmove(data + (i * 16), data + (i * 24), 16);
				AssetHandler::Relocate(data + (i * 24), data + (i * 16), 16);
			}
		}

		return result;
	}

	bool Zones::LoadGfxImage(bool atStreamStart, char* buffer, int size)
	{
		if (Zones::ZoneVersion >= 332)
		{
			size = (Zones::ZoneVersion >= 359) ? 52 : 36;
		}

		bool result = Game::Load_Stream(atStreamStart, buffer, size);

		if (Zones::ZoneVersion >= 332)
		{
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

				AssertSize(image359, 52);

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
		}

		return result;
	}

	bool Zones::LoadXAsset(bool atStreamStart, char* buffer, int size)
	{
		int count = 0;

		if (Zones::ZoneVersion >= 359)
		{
			size /= 8;
			count = size;
			size *= 16;
		}

		bool result = Game::Load_Stream(atStreamStart, buffer, size);

		if (Zones::ZoneVersion >= 359)
		{
			Utils::Memory::Allocator allocator;
			Game::XAsset* tempAssets = allocator.allocateArray<Game::XAsset>(count);

			for (int i = 0; i < count; ++i)
			{
				char* src = &buffer[i * 16];

				std::memcpy(&tempAssets[i].type, src + 0, 4);
				std::memcpy(&tempAssets[i].header, src + 8, 4);

				AssetHandler::Relocate(src + 0, buffer + (i * 8) + 0, 4);
				AssetHandler::Relocate(src + 8, buffer + (i * 8) + 4, 4);
			}

			std::memcpy(buffer, tempAssets, sizeof(Game::XAsset) * count);
		}

		return result;
	}

	bool Zones::LoadMaterialTechnique(bool atStreamStart, char* buffer, int size)
	{
		if (Zones::ZoneVersion >= 359)
		{
			size += 4;
		}

		bool result = Game::Load_Stream(atStreamStart, buffer, size);

		if (Zones::ZoneVersion >= 359)
		{
			// This shouldn't make any difference.
			// The new entry is an additional remapped techset which is linked at runtime.
			// It's used when the 0x100 gameFlag in a material is set.
			// As MW2 flags are only 1 byte large, this won't be possible anyways
			int shiftTest = 4;

			std::memmove(buffer + 8 + shiftTest, buffer + 12 + shiftTest, 196 - shiftTest);
			AssetHandler::Relocate(buffer + 12 + shiftTest, buffer + 8 + shiftTest, 196 - shiftTest);
		}

		return result;
	}

	bool Zones::LoadMaterial(bool atStreamStart, char* buffer, int size)
	{
		bool result = Game::Load_Stream(atStreamStart, buffer, size);

		if (Zones::ZoneVersion >= 359)
		{
			struct material339_s
			{
				char drawSurfBegin[8]; // 4
				//int surfaceTypeBits;
				const char *name;
				char drawSurf[6];

				union
				{
					char gameFlags;
					short sGameFlags;
				};
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

			if (material359.sGameFlags & 0x100)
			{
				//OutputDebugStringA("");
			}
		}

		return result;
	}

	bool Zones::LoadGfxWorld(bool atStreamStart, char* buffer, int size)
	{
		if (Zones::ZoneVersion >= 359)
		{
			size += 968;
		}

		bool result = Game::Load_Stream(atStreamStart, buffer, size);

		if (Zones::ZoneVersion >= 359)
		{
			int sunDiff = 8; // Stuff that is part of the sunflare we would overwrite
			std::memmove(buffer + 348 + sunDiff, buffer + 1316 + sunDiff, 280 - sunDiff);
			AssetHandler::Relocate(buffer + 1316, buffer + 348, 280);
		}

		return result;
	}

	void Zones::Loadsunflare_t(bool atStreamStart)
	{
		Game::Load_MaterialHandle(atStreamStart);

		if (Zones::ZoneVersion >= 359)
		{
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
	}

	bool Zones::LoadStatement(bool atStreamStart, char* buffer, int size)
	{
		if (Zones::Version() >= 359) size -= 4;

		bool result = Game::Load_Stream(atStreamStart, buffer, size);

		if (Zones::Version() >= 359)
		{
			std::memmove(buffer + 12, buffer + 8, 12);
		}

		return result;
	}

	void Zones::LoadWindowImage(bool atStreamStart)
	{
		Game::Load_MaterialHandle(atStreamStart);

		if (Zones::Version() >= 360)
		{
			char** varGfxImagePtr = reinterpret_cast<char**>(0x112B4A0);
			char** varwindowDef_t = reinterpret_cast<char**>(0x112AF94);

			*varGfxImagePtr = *varwindowDef_t + 164;
			Game::Load_GfxImagePtr(atStreamStart);
		}
	}

	void Zones::LoadPhysPreset(bool atStreamStart, char* buffer, int size)
	{
		if (Zones::Version() >= VERSION_ALPHA2)
		{
			size = 68;
		}

		Game::Load_Stream(atStreamStart, buffer, size);
	}

	void Zones::LoadXModelSurfs(bool atStreamStart, char* buffer, int size)
	{
		if (Zones::Version() >= VERSION_ALPHA2)
		{
			size = 48;
		}

		Game::Load_Stream(atStreamStart, buffer, size);
	}

	void Zones::LoadImpactFx(bool atStreamStart, char* buffer, int size)
	{
		if (Zones::Version() >= VERSION_ALPHA2)
		{
			size = 0x8C0;
		}

		Game::Load_Stream(atStreamStart, buffer, size);
	}

	int Zones::ImpactFxArrayCount()
	{
		if (Zones::Version() >= VERSION_ALPHA2)
		{
			return 16;
		}

		return 15;
	}

	__declspec(naked) void Zones::LoadImpactFxArray()
	{
		__asm
		{
			push edi
			pushad

			push edi
			call Zones::ImpactFxArrayCount
			pop edi

			mov [esp + 20h], eax

			popad
			pop edi

			push 4447E0h
			retn
		}
	}

	void Zones::LoadPathNodeArray(bool atStreamStart, char* buffer, int size)
	{
		if (Zones::Version() >= VERSION_ALPHA2)
		{
			size /= 136;
			size *= 148;
		}

		Game::Load_Stream(atStreamStart, buffer, size);
	}

	void Zones::LoadPathnodeConstantTail(bool atStreamStart, char* buffer, int size)
	{
		if (Zones::Version() >= VERSION_ALPHA2)
		{
			size /= 12;
			size *= 16;
		}

		Game::Load_Stream(atStreamStart, buffer, size);
	}

	void Zones::LoadExpressionSupportingDataPtr(bool atStreamStart)
	{
		if (Zones::Version() < 359)
		{
			Utils::Hook::Call<void(bool)>(0x4AF680)(atStreamStart);
		}
	}

	void Zones::SetVersion(int version)
	{
		AssetHandler::ClearRelocations();
		Zones::ZoneVersion = version;
	}

	__declspec(noinline) bool Zones::CheckGameMapSp(int type)
	{
		if (type == Game::XAssetType::ASSET_TYPE_GAME_MAP_SP)
		{
			return true;
		}

		if (Zones::Version() >= VERSION_ALPHA2 && type == Game::XAssetType::ASSET_TYPE_GAME_MAP_MP)
		{
			Maps::HandleAsSPMap();
			return true;
		}

		return false;
	}

	__declspec(naked) void Zones::GameMapSpPatchStub()
	{
		__asm
		{
			pushad

			push eax
			call Zones::CheckGameMapSp
			add esp, 4h

			test al, al
			jnz returnSafe

			popad
			push 4189AEh
			retn
			
		returnSafe:
			popad
			push 41899Dh
			retn
		}
	}

	int Zones::PathDataSize()
	{
		if (Zones::Version() >= VERSION_ALPHA2)
		{
			return 148;
		}

		return 136;
	}

	__declspec(naked) void Zones::LoadPathDataConstant()
	{
		__asm
		{
			push esi
			pushad

			call Zones::PathDataSize

			add [esp + 20h], eax

			popad
			pop esi

			push 4D6A4Dh
			retn
		}
	}

	Zones::Zones()
	{
		Zones::ZoneVersion = 0;

		// Ignore missing soundaliases for now
		// TODO: Include them in the dependency zone!
		Utils::Hook::Nop(0x644207, 5);

		// Block Mark_pathnode_constant_t
		Utils::Hook::Set<BYTE>(0x4F74B0, 0xC3);

		// addon_map_ents asset type (we reuse it for weaponattach)
		Utils::Hook::Set<BYTE>(0x418B31, 0x72);

		Utils::Hook(0x495938, Zones::LoadFxElemDefArrayStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x45ADA0, Zones::LoadFxElemDefStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x4EA6FE, Zones::LoadXModelLodInfoStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x410D90, Zones::LoadXModel, HOOK_CALL).install()->quick();
		Utils::Hook(0x4925C8, Zones::LoadXSurfaceArray, HOOK_CALL).install()->quick();
		Utils::Hook(0x4F4D0D, Zones::LoadGameWorldSp, HOOK_CALL).install()->quick();
		Utils::Hook(0x47CCD2, Zones::LoadWeaponCompleteDef, HOOK_CALL).install()->quick();
		Utils::Hook(0x483DA0, Zones::LoadVehicleDef, HOOK_CALL).install()->quick();
		Utils::Hook(0x4F0AC8, Zones::Loadsnd_alias_tArray, HOOK_CALL).install()->quick();
		Utils::Hook(0x403A5D, Zones::LoadLoadedSound, HOOK_CALL).install()->quick();
		Utils::Hook(0x463022, Zones::LoadWeaponAttach, HOOK_CALL).install()->quick();
		Utils::Hook(0x41A570, Zones::LoadmenuDef_t, HOOK_CALL).install()->quick();
		Utils::Hook(0x49591B, Zones::LoadFxEffectDef, HOOK_CALL).install()->quick();
		Utils::Hook(0x428F0A, Zones::LoadMaterialShaderArgumentArray, HOOK_CALL).install()->quick();
		Utils::Hook(0x4B1EB8, Zones::LoadStructuredDataStructPropertyArray, HOOK_CALL).install()->quick();
		Utils::Hook(0x49CE0D, Zones::LoadPhysPreset, HOOK_CALL).install()->quick();
		Utils::Hook(0x48E84D, Zones::LoadXModelSurfs, HOOK_CALL).install()->quick();
		Utils::Hook(0x4447C2, Zones::LoadImpactFx, HOOK_CALL).install()->quick();
		Utils::Hook(0x4447D0, Zones::LoadImpactFxArray, HOOK_JUMP).install()->quick();
		Utils::Hook(0x4D6A0B, Zones::LoadPathNodeArray, HOOK_CALL).install()->quick();
		Utils::Hook(0x4D6A47, Zones::LoadPathDataConstant, HOOK_JUMP).install()->quick();
		Utils::Hook(0x463D6E, Zones::LoadPathnodeConstantTail, HOOK_CALL).install()->quick();

		Utils::Hook(0x4471AD, Zones::LoadGfxImage, HOOK_CALL).install()->quick();

		Utils::Hook(0x41A590, Zones::LoadExpressionSupportingDataPtr, HOOK_CALL).install()->quick();
		Utils::Hook(0x459833, Zones::LoadExpressionSupportingDataPtr, HOOK_JUMP).install()->quick();

		Utils::Hook(0x5B9AA5, Zones::LoadXAsset, HOOK_CALL).install()->quick();
		Utils::Hook(0x461710, Zones::LoadMaterialTechnique, HOOK_CALL).install()->quick();
		Utils::Hook(0x40330D, Zones::LoadMaterial, HOOK_CALL).install()->quick();
		Utils::Hook(0x4B8DC0, Zones::LoadGfxWorld, HOOK_CALL).install()->quick();
		Utils::Hook(0x4B8FF5, Zones::Loadsunflare_t, HOOK_CALL).install()->quick();

		Utils::Hook(0x418998, Zones::GameMapSpPatchStub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x427A1B, Zones::LoadPathDataTail, HOOK_JUMP).install()->quick();
		Utils::Hook(0x4F4D3B, [] ()
		{
			if (Zones::ZoneVersion >= VERSION_ALPHA3)
			{
				ZeroMemory(*Game::varPathData, sizeof(Game::PathData));
			}
			else
			{
				// Load_PathData
				Utils::Hook::Call<void(int)>(0x4278A0)(false);
			}
		}, HOOK_CALL).install()->quick();

		// Change stream for images
		Utils::Hook(0x4D3225, [] ()
		{
			Game::DB_PushStreamPos((Zones::ZoneVersion >= 332) ? 3 : 0);
		}, HOOK_CALL).install()->quick();

		Utils::Hook(0x4597DD, Zones::LoadStatement, HOOK_CALL).install()->quick();
		Utils::Hook(0x471A39, Zones::LoadWindowImage, HOOK_JUMP).install()->quick();

#ifdef DEBUG
		// Easy dirty disk debugging
		Utils::Hook::Set<WORD>(0x4CF7F0, 0xC3CC);
#endif
	}

	Zones::~Zones()
	{

	}
}
