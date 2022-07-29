#include <STDInclude.hpp>

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
	Dvar::Var Renderer::r_drawAABBTrees;
	Dvar::Var Renderer::r_playerDrawDebugDistance;

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

	void Renderer::R_TextureFromCodeError(const char* sampler, Game::GfxCmdBufState* state)
	{
		Logger::Error(Game::ERR_FATAL, "Tried to use sampler '{}' when it isn't valid for material '{}' and technique '{}'",
			sampler, state->material->info.name, state->technique->name);
	}

	__declspec(naked) void Renderer::StoreGfxBufContextPtrStub1()
	{
		__asm
		{
			// original code
			mov eax, dword ptr [eax * 4 + 0x66E600C]

			// show error
			pushad
			push [esp + 0x24 + 0x20]
			push eax
			call R_TextureFromCodeError
			add esp, 8
			popad

			// go back
			push 0x54CAC1
			retn
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
			push ebx
			push edx
			call R_TextureFromCodeError
			add esp, 8
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

		auto entities = Game::g_entities;

		for (std::size_t i = 0; i < Game::MAX_GENTITIES; ++i)
		{
			auto* ent = &entities[i];

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
		Game::gentity_t* clientEntity = &Game::g_entities[clientNum];

		// Ingame only & player only
		if (!Game::CL_IsCgameInitialized() || clientEntity->client == nullptr)
		{
			return;
		}

		float playerPosition[3]{ clientEntity->r.currentOrigin[0], clientEntity->r.currentOrigin[1], clientEntity->r.currentOrigin[2] };

		const auto mapName = Dvar::Var("mapname").get<const char*>();
		auto scene = Game::scene;
		auto gfxAsset = Game::DB_FindXAssetEntry(Game::XAssetType::ASSET_TYPE_GFXWORLD, Utils::String::VA("maps/mp/%s.d3dbsp", mapName));

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
				auto staticModel = world->dpvs.smodelDrawInsts[i];

				if (Utils::Maths::Vec3SqrDistance(playerPosition, staticModel.placement.origin) < sqrDist)
				{
					if (staticModel.model)
					{
						Game::Bounds b = staticModel.model->bounds;
						b.midPoint[0] += staticModel.placement.origin[0];
						b.midPoint[1] += staticModel.placement.origin[1];
						b.midPoint[2] += staticModel.placement.origin[2];
						b.halfSize[0] *= staticModel.placement.scale;
						b.halfSize[1] *= staticModel.placement.scale;
						b.halfSize[2] *= staticModel.placement.scale;

						Game::R_AddDebugBounds(staticModelsColor, &b);
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
		Game::gentity_t* clientEntity = &Game::g_entities[clientNum];

		// Ingame only & player only
		if (!Game::CL_IsCgameInitialized() || clientEntity->client == nullptr)
		{
			return;
		}

		float playerPosition[3]{ clientEntity->r.currentOrigin[0], clientEntity->r.currentOrigin[1], clientEntity->r.currentOrigin[2] };

		const auto mapName = Dvar::Var("mapname").get<const char*>();
		auto scene = Game::scene;
		auto gfxAsset = Game::DB_FindXAssetEntry(Game::XAssetType::ASSET_TYPE_GFXWORLD, Utils::String::VA("maps/mp/%s.d3dbsp", mapName));

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
						Game::R_AddDebugString(staticModelsColor, staticModel.placement.origin, 1.0, staticModel.model->name);
					}
				}
			}
			break;
		default:
			break;
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

	Renderer::Renderer()
	{
		if (Dedicated::IsEnabled()) return;

		Scheduler::Loop([]
		{
			if (Game::CL_IsCgameInitialized())
			{
				DebugDrawAABBTrees();
				DebugDrawModelNames();
				DebugDrawModelBoundingBoxes();
				DebugDrawSceneModelCollisions();
				DebugDrawTriggers();
			}
		}, Scheduler::Pipeline::RENDERER);

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

		Scheduler::Once([]
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
			Renderer::r_drawAABBTrees = Game::Dvar_RegisterBool("r_drawAabbTrees", false, Game::DVAR_CHEAT, "Draw aabb trees");
			Renderer::r_playerDrawDebugDistance = Game::Dvar_RegisterInt("r_drawDebugDistance", 1000, 0, 50000, Game::DVAR_ARCHIVE, "r_draw debug functions draw distance, relative to the player");
		}, Scheduler::Pipeline::MAIN);
	}

	Renderer::~Renderer()
	{
		Renderer::BackendFrameSignal.clear();
		Renderer::SingleBackendFrameSignal.clear();

		Renderer::EndRecoverDeviceSignal.clear();
		Renderer::BeginRecoverDeviceSignal.clear();
	}
}
