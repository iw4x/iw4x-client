#include <STDInclude.hpp>

#include "Events.hpp"

namespace Components
{
	Utils::Signal<Renderer::BackendCallback> Renderer::BackendFrameSignal;
	Utils::Signal<Renderer::BackendCallback> Renderer::SingleBackendFrameSignal;

	Utils::Signal<Renderer::Callback> Renderer::EndRecoverDeviceSignal;
	Utils::Signal<Renderer::Callback> Renderer::BeginRecoverDeviceSignal;

	Dvar::Var Renderer::r_drawTriggers;
	Dvar::Var Renderer::r_drawSceneModelCollisions;
	Dvar::Var Renderer::r_drawModelBoundingBoxes;
	Dvar::Var Renderer::r_drawModelNames;
	Dvar::Var Renderer::r_drawRunners;
	Dvar::Var Renderer::r_drawAABBTrees;
	Dvar::Var Renderer::r_playerDrawDebugDistance;
	Dvar::Var Renderer::r_forceTechnique;
	Dvar::Var Renderer::r_listSamplers;
	Dvar::Var Renderer::r_drawLights;

	float cyan[4] = { 0.0f, 0.5f, 0.5f, 1.0f };
	float red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

	// R_draw model names & collisions colors
	float sceneModelsColor[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
	float dobjsColor[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
	float staticModelsColor[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
	float gentitiesColor[4] = { 1.0f, 0.5f, 0.5f, 1.0f };

	// Trigger colors
	float hurt[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float hurtTouch[4] = { 0.75f, 0.0f, 0.0f, 1.0f };
	float damage[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float once[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
	float multiple[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

	__declspec(naked) void Renderer::BackendFrameStub()
	{
		__asm
		{
			pushad
			call Renderer::BackendFrameHandler
			popad

			mov eax, ds:66E1BF0h
			push 536A85h
			retn
		}
	}

	void Renderer::BackendFrameHandler()
	{
		IDirect3DDevice9* device = *Game::dx_ptr;

		if (device)
		{
			device->AddRef();

			Renderer::BackendFrameSignal(device);

			Utils::Signal<Renderer::BackendCallback> copy(Renderer::SingleBackendFrameSignal);
			Renderer::SingleBackendFrameSignal.clear();
			copy(device);

			device->Release();
		}
	}

	void Renderer::OnNextBackendFrame(Utils::Slot<Renderer::BackendCallback> callback)
	{
		Renderer::SingleBackendFrameSignal.connect(callback);
	}

	void Renderer::OnBackendFrame(Utils::Slot<Renderer::BackendCallback> callback)
	{
		Renderer::BackendFrameSignal.connect(callback);
	}

	void Renderer::OnDeviceRecoveryEnd(Utils::Slot<Renderer::Callback> callback)
	{
		Renderer::EndRecoverDeviceSignal.connect(callback);
	}

	void Renderer::OnDeviceRecoveryBegin(Utils::Slot<Renderer::Callback> callback)
	{
		Renderer::BeginRecoverDeviceSignal.connect(callback);
	}

	int Renderer::Width()
	{
		return reinterpret_cast<LPPOINT>(0x66E1C68)->x;
	}

	int Renderer::Height()
	{
		return reinterpret_cast<LPPOINT>(0x66E1C68)->y;
	}

	void Renderer::PreVidRestart()
	{
		Renderer::BeginRecoverDeviceSignal();
	}

	void Renderer::PostVidRestart()
	{
		Renderer::EndRecoverDeviceSignal();
	}

	__declspec(naked) void Renderer::PostVidRestartStub()
	{
		__asm
		{
			pushad
			call Renderer::PostVidRestart
			popad

			push 4F84C0h
			retn
		}
	}

	void Renderer::R_TextureFromCodeError(const char* sampler, Game::GfxCmdBufState* state, int samplerCode)
	{
		Logger::Error(Game::ERR_FATAL, "Tried to use sampler '{}' ({}) at the wrong time! Additional info:\nMaterial: '{}'\nTechnique '{}'\nTechnique slot: {}\nTechnique flags: {}\nPass: {}\nPixel shader: '{}'\n",
			samplerCode, sampler, state->material->info.name, state->technique->name, static_cast<int>(state->techType), state->technique->flags, state->passIndex, state->pixelShader->name
		);
	}

	__declspec(naked) void Renderer::StoreGfxBufContextPtrStub1()
	{
		__asm
		{
			// Game's code
			mov eax, dword ptr [eax * 4 + 0x66E600C]

			// Show error
			pushad

			push eax
			push [esp + 0x20 + 0x24]
			push eax
			call R_TextureFromCodeError
			add esp, 0xC

			popad

			// Jump back in
			push 0x54CAC1
			ret
		}
	}

	__declspec(naked) void Renderer::StoreGfxBufContextPtrStub2()
	{
		__asm
		{
			// original code
			mov edx, dword ptr [eax * 4 + 0x66E600C]

			// show error
			pushad
			push eax
			push ebx
			push edx
			call R_TextureFromCodeError
			add esp, 0xC
			popad

			// go back
			push 0x54CFA4
			retn
		}
	}

	int Renderer::DrawTechsetForMaterial(int a1, float a2, float a3, const char* material, Game::vec4_t* color, int a6)
	{
		auto mat = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, Utils::String::VA("wc/%s", material)).material;
		return Utils::Hook::Call<int(int, float, float, const char*, Game::vec4_t*, int)>(0x005033E0)(a1, a2, a3, Utils::String::VA("%s (^3%s^7)", mat->info.name, mat->techniqueSet->name), color, a6);
	}

	void Renderer::DebugDrawTriggers()
	{
		if (!r_drawTriggers.get<bool>()) return;

		for (std::size_t i = 0; i < Game::MAX_GENTITIES; ++i)
		{
			auto* ent = &Game::g_entities[i];

			if (ent->r.isInUse)
			{
				Game::Bounds b = ent->r.box;
				b.midPoint[0] += ent->r.currentOrigin[0];
				b.midPoint[1] += ent->r.currentOrigin[1];
				b.midPoint[2] += ent->r.currentOrigin[2];

				switch (ent->handler)
				{
				case Game::ENT_HANDLER_TRIGGER_HURT:
					Game::R_AddDebugBounds(hurt, &b);
					break;

				case Game::ENT_HANDLER_TRIGGER_HURT_TOUCH:
					Game::R_AddDebugBounds(hurtTouch, &b);
					break;

				case Game::ENT_HANDLER_TRIGGER_DAMAGE:
					Game::R_AddDebugBounds(damage, &b);
					break;

				case Game::ENT_HANDLER_TRIGGER_MULTIPLE:
					if (ent->spawnflags & 0x40)
						Game::R_AddDebugBounds(once, &b);
					else
						Game::R_AddDebugBounds(multiple, &b);
					break;

				default:
					auto rv = std::min(static_cast<float>(ent->handler), 5.0f) / 5.0f;
					auto gv = std::clamp(static_cast<float>(ent->handler - 5), 0.f, 5.0f) / 5.0f;
					auto bv = std::clamp(static_cast<float>(ent->handler - 10), 0.f, 5.0f) / 5.0f;

					float color[4] = { rv, gv, bv, 1.0f };

					Game::R_AddDebugBounds(color, &b);
					break;
				}
			}
		}
	}

	void Renderer::DebugDrawSceneModelCollisions()
	{
		if (!r_drawSceneModelCollisions.get<bool>()) return;

		auto scene = Game::scene;

		for (auto i = 0; i < scene->sceneModelCount; ++i)
		{
			if (!scene->sceneModel[i].model)
				continue;

			for (auto j = 0; j < scene->sceneModel[i].model->numCollSurfs; j++)
			{
				auto b = scene->sceneModel[i].model->collSurfs[j].bounds;
				b.midPoint[0] += scene->sceneModel[i].placement.base.origin[0];
				b.midPoint[1] += scene->sceneModel[i].placement.base.origin[1];
				b.midPoint[2] += scene->sceneModel[i].placement.base.origin[2];
				b.halfSize[0] *= scene->sceneModel[i].placement.scale;
				b.halfSize[1] *= scene->sceneModel[i].placement.scale;
				b.halfSize[2] *= scene->sceneModel[i].placement.scale;

				Game::R_AddDebugBounds(green, &b, &scene->sceneModel[i].placement.base.quat);
			}
		}
	}

	void Renderer::DebugDrawModelBoundingBoxes()
	{
		auto val = r_drawModelBoundingBoxes.get<int>();

		if (!val) return;

		auto clientNum = Game::CG_GetClientNum();
		auto* clientEntity = &Game::g_entities[clientNum];

		// Ingame only & player only
		if (!Game::CL_IsCgameInitialized() || clientEntity->client == nullptr)
		{
			return;
		}

		float playerPosition[3]{ clientEntity->r.currentOrigin[0], clientEntity->r.currentOrigin[1], clientEntity->r.currentOrigin[2] };

		auto scene = Game::scene;
		auto gfxAsset = Game::DB_FindXAssetEntry(Game::XAssetType::ASSET_TYPE_GFXWORLD, Utils::String::VA("maps/mp/%s.d3dbsp", (*Game::sv_mapname)->current.string));

		if (gfxAsset == nullptr)
		{
			return;
		}

		auto world = gfxAsset->asset.header.gfxWorld;

		auto drawDistance = r_playerDrawDebugDistance.get<int>();
		auto sqrDist = drawDistance * drawDistance;

		switch (val) 
		{
		case 1:
			for (auto i = 0; i < scene->sceneModelCount; i++)
			{
				if (!scene->sceneModel[i].model)
					continue;

				if (Utils::Maths::Vec3SqrDistance(playerPosition, scene->sceneModel[i].placement.base.origin) < sqrDist)
				{
					auto b = scene->sceneModel[i].model->bounds;
					b.midPoint[0] += scene->sceneModel[i].placement.base.origin[0];
					b.midPoint[1] += scene->sceneModel[i].placement.base.origin[1];
					b.midPoint[2] += scene->sceneModel[i].placement.base.origin[2];
					b.halfSize[0] *= scene->sceneModel[i].placement.scale;
					b.halfSize[1] *= scene->sceneModel[i].placement.scale;
					b.halfSize[2] *= scene->sceneModel[i].placement.scale;
					Game::R_AddDebugBounds(sceneModelsColor, &b, &scene->sceneModel[i].placement.base.quat);
				}
			}
			break;
		case 2:
			for (auto i = 0; i < scene->sceneDObjCount; i++)
			{

				if (Utils::Maths::Vec3SqrDistance(playerPosition, scene->sceneDObj[i].cull.bounds.midPoint) < sqrDist)
				{
					scene->sceneDObj[i].cull.bounds.halfSize[0] = std::abs(scene->sceneDObj[i].cull.bounds.halfSize[0]);
					scene->sceneDObj[i].cull.bounds.halfSize[1] = std::abs(scene->sceneDObj[i].cull.bounds.halfSize[1]);
					scene->sceneDObj[i].cull.bounds.halfSize[2] = std::abs(scene->sceneDObj[i].cull.bounds.halfSize[2]);

					if (scene->sceneDObj[i].cull.bounds.halfSize[0] < 0 ||
						scene->sceneDObj[i].cull.bounds.halfSize[1] < 0 ||
						scene->sceneDObj[i].cull.bounds.halfSize[2] < 0)
					{
						Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Negative half size for DOBJ {}, this will cause culling issues!",
							scene->sceneDObj[i].obj->models[0]->name);
					}

					Game::R_AddDebugBounds(dobjsColor, &scene->sceneDObj[i].cull.bounds);
				}
			}
			break;
		case 3:
			// Static models
			for (size_t i = 0; i < world->dpvs.smodelCount; i++)
			{
				auto staticModel = &world->dpvs.smodelDrawInsts[i];
				auto* b = &world->dpvs.smodelInsts[i].bounds;

				if (Utils::Maths::Vec3SqrDistance(playerPosition, staticModel->placement.origin) < sqrDist)
				{
					if (staticModel->model)
					{
						Game::R_AddDebugBounds(staticModelsColor, b);
					}
				}
			}
			break;
		default:
			break;
		}
	}

	void Renderer::DebugDrawModelNames()
	{
		auto val = r_drawModelNames.get<int>();

		if (!val) return;

		auto clientNum = Game::CG_GetClientNum();
		auto* clientEntity = &Game::g_entities[clientNum];

		// Ingame only & player only
		if (!Game::CL_IsCgameInitialized() || clientEntity->client == nullptr)
		{
			return;
		}

		float playerPosition[3]{ clientEntity->r.currentOrigin[0], clientEntity->r.currentOrigin[1], clientEntity->r.currentOrigin[2] };

		auto scene = Game::scene;
		auto gfxAsset = Game::DB_FindXAssetEntry(Game::XAssetType::ASSET_TYPE_GFXWORLD, Utils::String::VA("maps/mp/%s.d3dbsp", (*Game::sv_mapname)->current.string));

		if (gfxAsset == nullptr)
		{
			return;
		}

		auto world = gfxAsset->asset.header.gfxWorld;

		auto drawDistance = r_playerDrawDebugDistance.get<int>();
		auto sqrDist = drawDistance * drawDistance;

		switch (val)
		{
		case 1:
			for (auto i = 0; i < scene->sceneModelCount; i++)
			{
				if (!scene->sceneModel[i].model)
					continue;

				if (Utils::Maths::Vec3SqrDistance(playerPosition, scene->sceneModel[i].placement.base.origin) < static_cast<float>(sqrDist))
				{
					Game::R_AddDebugString(sceneModelsColor, scene->sceneModel[i].placement.base.origin, 1.0, scene->sceneModel[i].model->name);
				}
			}
			break;
		case 2:
			for (auto i = 0; i < scene->sceneDObjCount; i++)
			{
				if (scene->sceneDObj[i].obj)
				{
					for (int j = 0; j < scene->sceneDObj[i].obj->numModels; j++)
					{
						if (Utils::Maths::Vec3SqrDistance(playerPosition, scene->sceneDObj[i].placement.origin) < static_cast<float>(sqrDist))
						{
							Game::R_AddDebugString(dobjsColor, scene->sceneDObj[i].placement.origin, 1.0, scene->sceneDObj[i].obj->models[j]->name);
						}
					}
				}
			}
			break;
		case 3:
			// Static models
			for (size_t i = 0; i < world->dpvs.smodelCount; i++)
			{
				auto staticModel = world->dpvs.smodelDrawInsts[i];
				if (staticModel.model)
				{
					const auto dist = Utils::Maths::Vec3SqrDistance(playerPosition, staticModel.placement.origin);
					if (dist < static_cast<float>(sqrDist))
					{
						float rgb01Color[] =
						{
							staticModel.groundLighting.array[0] / 255.f,
							staticModel.groundLighting.array[1] / 255.f,
							staticModel.groundLighting.array[2] / 255.f,
							1.f,
						};

						Game::R_AddDebugString(staticModel.flags & 0x20 ? rgb01Color : staticModelsColor, staticModel.placement.origin, 1.0f, staticModel.model->name);
					}
				}
			}
			break;
		default:
			break;
		}
	}

	void Renderer::DebugDrawRunners()
	{
		if (!Game::CL_IsCgameInitialized())
		{
			return;
		}

		if (!r_drawRunners.get<bool>())
		{
			return;
		}

		auto* fxSystem = reinterpret_cast<Game::FxSystem*>(0x173F200);

		if (fxSystem)
		{
			for (auto i = 0; i < fxSystem->activeElemCount; i++)
			{
				auto* elem = &fxSystem->effects[i];
				if (elem->def)
				{
					Game::R_AddDebugString(sceneModelsColor, elem->frameNow.origin, 1.0f, elem->def->name);
				}
			}
		}

		auto soundCount = *reinterpret_cast<int*>(0x7C5C90);
		auto* sounds = reinterpret_cast<Game::ClientEntSound*>(0x7C5CA0);

		for (auto i = 0; i < soundCount; i++)
		{
			if (sounds[i].aliasList)
			{
				Game::R_AddDebugString(staticModelsColor, sounds[i].origin, 1.0f, sounds[i].aliasList->aliasName);
			}
		}
	}

	void Renderer::DebugDrawAABBTrees()
	{
		if (!r_drawAABBTrees.get<bool>()) return;

		Game::clipMap_t* clipMap = *reinterpret_cast<Game::clipMap_t**>(0x7998E0);
		if (!clipMap) return;

		for (unsigned short i = 0; i < clipMap->smodelNodeCount; ++i)
		{
			Game::R_AddDebugBounds(cyan, &clipMap->smodelNodes[i].bounds);
		}

		for (unsigned int i = 0; i < clipMap->numStaticModels; i += 2)
		{
			Game::R_AddDebugBounds(red, &clipMap->staticModelList[i].absBounds);
		}
	}

	void Renderer::ForceTechnique()
	{
		auto forceTechnique = r_forceTechnique.get<int>();

		if (forceTechnique > 0)
		{
			Utils::Hook::Set(0x6FABDF4, forceTechnique);
		}
	}

	void Renderer::ListSamplers()
	{
		if (!r_listSamplers.get<bool>())
		{
			return;
		}

		static auto* source = reinterpret_cast<Game::GfxCmdBufSourceState*>(0x6CAF080);

		auto* font = Game::R_RegisterFont("fonts/smallFont", 0);
		auto height = Game::R_TextHeight(font);
		auto scale = 1.0f;
		float color[] = {0.0f, 1.0f, 0.0f, 1.0f};

		for (std::size_t i = 0; i < 27; ++i)
		{
			if (source->input.codeImages[i] == nullptr)
			{
				color[0] = 1.f;
			}
			else
			{
				color[0] = 0.f;
			}

			const auto* str = Utils::String::Format("{}/{:#X} => {} {}", i, i,
				(source->input.codeImages[i] == nullptr ? "---" : source->input.codeImages[i]->name),
				std::to_string(source->input.codeImageSamplerStates[i])
			);

			Game::R_AddCmdDrawText(str, std::numeric_limits<int>::max(), font, 15.0f, (height * scale + 1) * (i + 1) + 14.0f, scale, scale, 0.0f, color, Game::ITEM_TEXTSTYLE_NORMAL);
		}
	}

	void Renderer::DrawPrimaryLights()
	{
		if (!r_drawLights.get<bool>())
		{
			return;
		}

		auto clientNum = Game::CG_GetClientNum();
		auto* clientEntity = &Game::g_entities[clientNum];

		// Ingame only & player only
		if (!Game::CL_IsCgameInitialized() || clientEntity->client == nullptr)
		{
			return;
		}

		auto scene = Game::scene;
		auto asset = Game::DB_FindXAssetEntry(Game::XAssetType::ASSET_TYPE_COMWORLD, Utils::String::VA("maps/mp/%s.d3dbsp", (*Game::sv_mapname)->current.string));

		if (asset == nullptr)
		{
			return;
		}

		auto world = asset->asset.header.comWorld;

		for (size_t i = 0; i < world->primaryLightCount; i++)
		{
			auto light = &world->primaryLights[i];

			float to[3];
			to[0] = light->origin[0] + light->dir[0] * 10;
			to[1] = light->origin[1] + light->dir[1] * 10;
			to[2] = light->origin[2] + light->dir[2] * 10;

			auto n = light->defName == nullptr ? "NONE" : light->defName;

			auto str = std::format("LIGHT #{} ({})", i, n);

			float color[4]{};
			color[3] = 1.0f;
			color[0] = light->color[0];
			color[1] = light->color[1];
			color[2] = light->color[2];


			Game::R_AddDebugLine(color, light->origin, to);
			Game::R_AddDebugString(color, light->origin, 1.0f, str.data());
		}

		if (scene)
		{
			for (size_t i = 0; i < scene->addedLightCount; i++)
			{
				auto light = &scene->addedLight[i];

				float color[4]{};
				color[3] = 1.0f;
				color[0] = light->color[0];
				color[1] = light->color[1];
				color[2] = light->color[2];

				float to[3];
				to[0] = light->origin[0] + light->dir[0] * 10;
				to[1] = light->origin[1] + light->dir[1] * 10;
				to[2] = light->origin[2] + light->dir[2] * 10;

				auto str = std::format("ADDED LIGHT #{}", i);

				Game::R_AddDebugLine(color, light->origin, to);
				Game::R_AddDebugString(color, light->origin, 1.0f, str.data());
				
			}
		}
	}

	int Renderer::FixSunShadowPartitionSize(Game::GfxCamera* camera, Game::GfxSunShadowMapMetrics* mapMetrics, Game::GfxSunShadow* sunShadow, Game::GfxSunShadowClip* clip, float* partitionFraction)
	{
		auto result = Utils::Hook::Call<int(Game::GfxCamera*, Game::GfxSunShadowMapMetrics*, Game::GfxSunShadow*, Game::GfxSunShadowClip*, float*)>(0x5463B0)(camera, mapMetrics, sunShadow, clip, partitionFraction);

		if (Maps::IsCustomMap()) 
		{
			// Fixes shadowmap viewport which fixes pixel adjustment shadowmap bug - partly, because the real problem lies within the way CoD4 shaders are programmed
			sunShadow->partition[Game::SunShadowPartition::R_SUNSHADOW_FAR].viewportParms.viewport = sunShadow->partition[Game::SunShadowPartition::R_SUNSHADOW_NEAR].viewportParms.viewport;
		}

		return result;
	}

	Renderer::Renderer()
	{
		if (Dedicated::IsEnabled()) return;

		Scheduler::Loop([]
		{
			if (Game::CL_IsCgameInitialized())
			{
				DebugDrawRunners();
				DebugDrawAABBTrees();
				DebugDrawModelNames();
				DebugDrawModelBoundingBoxes();
				DebugDrawSceneModelCollisions();
				DebugDrawTriggers();
				ForceTechnique();
				ListSamplers();
				DrawPrimaryLights();
			}
		}, Scheduler::Pipeline::RENDERER);

#ifdef _DEBUG
		// Disable ATI Radeon 4000 optimization that crashes Pixwin
		Utils::Hook::Set(0x5066F8, D3DFMT_UNKNOWN);
#endif

		// COD4 Map Fixes
		// The day map porting is perfect we should be able to remove these
		Utils::Hook(0x546A09, FixSunShadowPartitionSize, HOOK_CALL).install()->quick();

		// Log broken materials
		Utils::Hook(0x0054CAAA, Renderer::StoreGfxBufContextPtrStub1, HOOK_JUMP).install()->quick();
		Utils::Hook(0x0054CF8D, Renderer::StoreGfxBufContextPtrStub2, HOOK_JUMP).install()->quick();

		// Enhance cg_drawMaterial
		Utils::Hook::Set(0x005086DA, "^3solid^7");
		Utils::Hook(0x00580F53, Renderer::DrawTechsetForMaterial, HOOK_CALL).install()->quick();

		Utils::Hook(0x536A80, Renderer::BackendFrameStub, HOOK_JUMP).install()->quick();

		// Begin device recovery (not D3D9Ex)
		Utils::Hook(0x508298, []
		{
			Game::DB_BeginRecoverLostDevice();
			Renderer::BeginRecoverDeviceSignal();
		}, HOOK_CALL).install()->quick();

		// End device recovery (not D3D9Ex)
		Utils::Hook(0x508355, []
		{
			Renderer::EndRecoverDeviceSignal();
			Game::DB_EndRecoverLostDevice();
		}, HOOK_CALL).install()->quick();

		// Begin vid_restart
		Utils::Hook(0x4CA2FD, Renderer::PreVidRestart, HOOK_CALL).install()->quick();

		// End vid_restart
		Utils::Hook(0x4CA3A7, Renderer::PostVidRestartStub, HOOK_CALL).install()->quick();

		Events::OnDvarInit([]
		{
			static const char* values[] =
			{
				"Disabled",
				"Scene Models",
				"Scene Dynamic Objects",
				"GfxWorld Static Models",
				nullptr
			};

			Renderer::r_drawModelBoundingBoxes = Game::Dvar_RegisterEnum("r_drawModelBoundingBoxes", values, 0, Game::DVAR_CHEAT, "Draw scene model bounding boxes");
			Renderer::r_drawSceneModelCollisions = Game::Dvar_RegisterBool("r_drawSceneModelCollisions", false, Game::DVAR_CHEAT, "Draw scene model collisions");
			Renderer::r_drawTriggers = Game::Dvar_RegisterBool("r_drawTriggers", false, Game::DVAR_CHEAT, "Draw triggers");
			Renderer::r_drawModelNames = Game::Dvar_RegisterEnum("r_drawModelNames", values, 0, Game::DVAR_CHEAT, "Draw all model names");
			Renderer::r_drawRunners = Game::Dvar_RegisterBool("r_drawRunners", false, Game::DVAR_NONE, "Draw active sound & fx runners");
			Renderer::r_drawAABBTrees = Game::Dvar_RegisterBool("r_drawAabbTrees", false, Game::DVAR_CHEAT, "Draw aabb trees");
			Renderer::r_playerDrawDebugDistance = Game::Dvar_RegisterInt("r_drawDebugDistance", 1000, 0, 50000, Game::DVAR_ARCHIVE, "r_draw debug functions draw distance relative to the player");
			Renderer::r_forceTechnique = Game::Dvar_RegisterInt("r_forceTechnique", 0, 0, 14, Game::DVAR_NONE, "Force a base technique on the renderer");
			Renderer::r_listSamplers = Game::Dvar_RegisterBool("r_listSamplers", false, Game::DVAR_NONE, "List samplers & sampler states");
			Renderer::r_drawLights = Game::Dvar_RegisterBool("r_drawLights", false, Game::DVAR_NONE, "Draw every comworld light in the level");
		});
	}

	Renderer::~Renderer()
	{
		Renderer::BackendFrameSignal.clear();
		Renderer::SingleBackendFrameSignal.clear();

		Renderer::EndRecoverDeviceSignal.clear();
		Renderer::BeginRecoverDeviceSignal.clear();
	}
}
