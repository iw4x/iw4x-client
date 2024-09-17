#include <STDInclude.hpp>
#include "IFxEffectDef.hpp"

namespace Assets
{
	void IFxEffectDef::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		if (!header->data)                                   this->loadEfx(header, name, builder);    // Check if we have an editor fx
		if (!header->data)                                   this->loadFromIW4OF(header, name, builder); // Check if we need to import a new one into the game
		if (!header->data /*&& !builder->isPrimaryAsset()*/) this->loadNative(header, name, builder); // Check if there is a native one

		assert(header->data);
	}

	void IFxEffectDef::loadFromIW4OF(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		header->fx = builder->getIW4OfApi()->read<Game::FxEffectDef>(Game::XAssetType::ASSET_TYPE_FX, name);
	}

	void IFxEffectDef::loadEfx(Game::XAssetHeader* /*header*/, const std::string& name, Components::ZoneBuilder::Zone* /*builder*/)
	{
#ifdef DEBUG
		Components::FileSystem::File rawFx(Utils::String::VA("fx/%s.efx", name.data()));
		if (rawFx.exists())
		{
			const char* session = rawFx.getBuffer().data();
			Game::Com_BeginParseSession("fx");
			Game::Com_SetSpaceDelimited(0);
			Game::Com_SetParseNegativeNumbers(1);

			const char* format = Game::Com_Parse(&session);
			if (format != "iwfx"s)
			{
				Game::Com_EndParseSession();
				Components::Logger::Error(Game::ERR_FATAL, "Effect needs to be updated from the legacy format.\n");
			}

			int version = atoi(Game::Com_Parse(&session));
			if (version > 2)
			{
				Game::Com_EndParseSession();
				Components::Logger::Error(Game::ERR_FATAL, "Version {} is too high. I can only handle up to 2.\n", version);
			}

			Utils::Memory::Allocator allocator;
			Game::FxEditorEffectDef* efx = allocator.allocate<Game::FxEditorEffectDef>();
			ZeroMemory(efx, sizeof(efx));

			for (efx->elemCount = 0; ; ++efx->elemCount)
			{
				const char* value = Game::Com_Parse(&session);
				if (!value) break;
				if (*value != '{')
				{
					Components::Logger::Error(Game::ERR_FATAL, "Expected '{' to start a new segment, found '{}' instead.\n", value);
				}

				if (efx->elemCount >= ARRAYSIZE(efx->elems))
				{
					Components::Logger::Error(Game::ERR_FATAL, "Cannot have more than {} segments.\n", ARRAYSIZE(efx->elems));
				}

				Game::FxEditorElemDef* element = &efx->elems[efx->elemCount];
				// TODO: Initialize some stuff here

				while (true)
				{
					std::string newValue = Game::Com_Parse(&session);
					if (newValue[0] != '}') break;

					for (int i = 0; i < FX_ELEM_FIELD_COUNT; ++i)
					{
						if (Game::s_elemFields[i].keyName == std::string(newValue))
						{
							// TODO: Allow loading assets from raw!
							if (Game::s_elemFields[i].handler(&session, element)) break;
							Components::Logger::Error(Game::ERR_FATAL, "Failed to parse element {}!\n", newValue);
						}
					}

					if (!Game::Com_MatchToken(&session, ";", 1))
					{
						Components::Logger::Error(Game::ERR_FATAL, "Expected token ';'\n");
					}
				}
			}

			Game::Com_EndParseSession();

			// TODO: Convert editor fx to real fx
		}
#else
		(void)name;
#endif
	}

	void IFxEffectDef::loadNative(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* /*builder*/)
	{
		header->fx = Components::AssetHandler::FindOriginalAsset(this->getType(), name.data()).fx;
	}

	void IFxEffectDef::markFxElemVisuals(Game::FxElemVisuals* visuals, char elemType, Components::ZoneBuilder::Zone* builder)
	{
		switch (elemType)
		{
		case 7:
		{
			if (visuals->model)
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, visuals->model);
			}

			break;
		}

		case 8:
		case 9:
			break;

		case 0xA:
		{
			if (visuals->soundName)
			{
				// Double "find call" but we have to because otherwise we'd crash on missing asset
				// Sometimes Fx reference by name a sound that does not exist. IW oversight ? 
				// Never happens on iw3 but often happens on iw5, especially DLC maps.
				if (Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_SOUND, visuals->soundName, builder, false).data)
				{
					builder->loadAssetByName(Game::XAssetType::ASSET_TYPE_SOUND, visuals->soundName, false);
				}
			}
			break;
		}

		case 0xC:
		{
			if (visuals->effectDef.handle)
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, visuals->effectDef.handle, false);
			}

			break;
		}

		default:
		{
			if (visuals->material)
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, visuals->material);
			}

			break;
		}
		}
	}

	void IFxEffectDef::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::FxEffectDef* asset = header.fx;

		for (int i = 0; i < (asset->elemDefCountEmission + asset->elemDefCountLooping + asset->elemDefCountOneShot); ++i)
		{
			Game::FxElemDef* elemDef = &asset->elemDefs[i];

			{
				if (elemDef->elemType == 11)
				{
					if (elemDef->visuals.markArray)
					{
						for (char j = 0; j < elemDef->visualCount; ++j)
						{
							if (elemDef->visuals.markArray[j].materials[0])
							{
								builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, elemDef->visuals.markArray[j].materials[0]);
							}

							if (elemDef->visuals.markArray[j].materials[1])
							{
								builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, elemDef->visuals.markArray[j].materials[1]);
							}
						}
					}
				}
				else if (elemDef->visualCount > 1)
				{
					if (elemDef->visuals.array)
					{
						for (char j = 0; j < elemDef->visualCount; ++j)
						{
							this->markFxElemVisuals(&elemDef->visuals.array[j], elemDef->elemType, builder);
						}
					}
				}
				else
				{
					this->markFxElemVisuals(&elemDef->visuals.instance, elemDef->elemType, builder);
				}
			}

			if (elemDef->effectOnImpact.handle)
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, elemDef->effectOnImpact.handle, false);
			}

			if (elemDef->effectOnDeath.handle)
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, elemDef->effectOnDeath.handle, false);
			}

			if (elemDef->effectEmitted.handle)
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, elemDef->effectEmitted.handle, false);
			}
		}
	}

	void IFxEffectDef::saveFxElemVisuals(Game::FxElemVisuals* visuals, Game::FxElemVisuals* destVisuals, char elemType, Components::ZoneBuilder::Zone* builder)
	{
		Utils::Stream* buffer = builder->getBuffer();

		switch (elemType)
		{
		case Game::FX_ELEM_TYPE_MODEL:
		{
			if (visuals->model)
			{
				destVisuals->model = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_XMODEL, visuals->model).model;
			}

			break;
		}

		case Game::FX_ELEM_TYPE_OMNI_LIGHT:
		case Game::FX_ELEM_TYPE_SPOT_LIGHT:
			break;

		case Game::FX_ELEM_TYPE_SOUND:
		{
			if (visuals->soundName)
			{
				buffer->saveString(visuals->soundName);
				Utils::Stream::ClearPointer(&destVisuals->soundName);
			}

			break;
		}

		case Game::FX_ELEM_TYPE_RUNNER:
		{
			if (visuals->effectDef.handle)
			{
				buffer->saveString(visuals->effectDef.handle->name);
				Utils::Stream::ClearPointer(&destVisuals->effectDef.handle);
			}

			break;
		}

		default:
		{
			if (visuals->material)
			{
				destVisuals->material = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, visuals->material).material;
			}

			break;
		}
		}
	}

	void IFxEffectDef::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::FxEffectDef, 32);

		Utils::Stream* buffer = builder->getBuffer();
		Game::FxEffectDef* asset = header.fx;
		Game::FxEffectDef* dest = buffer->dest<Game::FxEffectDef>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->elemDefs)
		{
			AssertSize(Game::FxElemDef, 252);
			buffer->align(Utils::Stream::ALIGN_4);

			Game::FxElemDef* destElemDefs = buffer->dest<Game::FxElemDef>();
			buffer->saveArray(asset->elemDefs, asset->elemDefCountEmission + asset->elemDefCountLooping + asset->elemDefCountOneShot);

			for (int i = 0; i < (asset->elemDefCountEmission + asset->elemDefCountLooping + asset->elemDefCountOneShot); ++i)
			{
				Game::FxElemDef* destElemDef = &destElemDefs[i];
				Game::FxElemDef* elemDef = &asset->elemDefs[i];

				if (elemDef->velSamples)
				{
					AssertSize(Game::FxElemVelStateSample, 96);
					buffer->align(Utils::Stream::ALIGN_4);

					buffer->saveArray(elemDef->velSamples, elemDef->velIntervalCount + 1);

					Utils::Stream::ClearPointer(&destElemDef->velSamples);
				}

				if (elemDef->visSamples)
				{
					AssertSize(Game::FxElemVisStateSample, 48);
					buffer->align(Utils::Stream::ALIGN_4);

					buffer->saveArray(elemDef->visSamples, elemDef->visStateIntervalCount + 1);

					Utils::Stream::ClearPointer(&destElemDef->visSamples);
				}

				// Save_FxElemDefVisuals
				{
					if (elemDef->elemType == Game::FX_ELEM_TYPE_DECAL)
					{
						if (elemDef->visuals.markArray)
						{
							AssertSize(Game::FxElemMarkVisuals, 8);
							buffer->align(Utils::Stream::ALIGN_4);

							Game::FxElemMarkVisuals* destMarkArray = buffer->dest<Game::FxElemMarkVisuals>();
							buffer->saveArray(elemDef->visuals.markArray, elemDef->visualCount);

							for (auto j = 0; j < elemDef->visualCount; ++j)
							{
								if (elemDef->visuals.markArray[j].materials[0])
								{
									destMarkArray[j].materials[0] = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, elemDef->visuals.markArray[j].materials[0]).material;
								}

								if (elemDef->visuals.markArray[j].materials[1])
								{
									destMarkArray[j].materials[1] = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, elemDef->visuals.markArray[j].materials[1]).material;
								}
							}

							Utils::Stream::ClearPointer(&destElemDef->visuals.markArray);
						}
					}
					else if (elemDef->visualCount > 1)
					{
						if (elemDef->visuals.array)
						{
							AssertSize(Game::FxElemVisuals, 4);
							buffer->align(Utils::Stream::ALIGN_4);

							Game::FxElemVisuals* destVisuals = buffer->dest<Game::FxElemVisuals>();
							buffer->saveArray(elemDef->visuals.array, elemDef->visualCount);

							for (char j = 0; j < elemDef->visualCount; ++j)
							{
								this->saveFxElemVisuals(&elemDef->visuals.array[j], &destVisuals[j], elemDef->elemType, builder);
							}

							Utils::Stream::ClearPointer(&destElemDef->visuals.array);
						}
					}
					else
					{
						this->saveFxElemVisuals(&elemDef->visuals.instance, &destElemDef->visuals.instance, elemDef->elemType, builder);
					}
				}

				if (elemDef->effectOnImpact.handle)
				{
					buffer->saveString(elemDef->effectOnImpact.handle->name);
					Utils::Stream::ClearPointer(&destElemDef->effectOnImpact.handle);
				}

				if (elemDef->effectOnDeath.handle)
				{
					buffer->saveString(elemDef->effectOnDeath.handle->name);
					Utils::Stream::ClearPointer(&destElemDef->effectOnDeath.handle);
				}

				if (elemDef->effectEmitted.handle)
				{
					buffer->saveString(elemDef->effectEmitted.handle->name);
					Utils::Stream::ClearPointer(&destElemDef->effectEmitted.handle);
				}

				// Save_FxElemExtendedDefPtr
				{
					AssertSize(Game::FxElemExtendedDefPtr, 4);

					if (elemDef->elemType == Game::FX_ELEM_TYPE_TRAIL)
					{
						// Save_FxTrailDef
						{
							if (elemDef->extended.trailDef)
							{
								AssertSize(Game::FxTrailDef, 36);
								buffer->align(Utils::Stream::ALIGN_4);

								Game::FxTrailDef* trailDef = elemDef->extended.trailDef;
								Game::FxTrailDef* destTrailDef = buffer->dest<Game::FxTrailDef>();
								buffer->save(trailDef);

								if (trailDef->verts)
								{
									AssertSize(Game::FxTrailVertex, 20);
									buffer->align(Utils::Stream::ALIGN_4);

									buffer->saveArray(trailDef->verts, trailDef->vertCount);
									Utils::Stream::ClearPointer(&destTrailDef->verts);
								}

								if (trailDef->inds)
								{
									buffer->align(Utils::Stream::ALIGN_2);

									buffer->saveArray(trailDef->inds, trailDef->indCount);
									Utils::Stream::ClearPointer(&destTrailDef->inds);
								}

								Utils::Stream::ClearPointer(&destElemDef->extended.trailDef);
							}
						}
					}
					else if (elemDef->elemType == Game::FX_ELEM_TYPE_SPARK_FOUNTAIN)
					{
						if (elemDef->extended.sparkFountainDef)
						{
							AssertSize(Game::FxSparkFountainDef, 52);
							buffer->align(Utils::Stream::ALIGN_4);

							buffer->save(elemDef->extended.sparkFountainDef);
							Utils::Stream::ClearPointer(&destElemDef->extended.sparkFountainDef);
						}
					}
					else
					{
						if (elemDef->extended.unknownDef)
						{
							buffer->save(reinterpret_cast<char*>(elemDef->extended.unknownDef));
							Utils::Stream::ClearPointer(&destElemDef->extended.unknownDef);
						}
					}
				}
			}

			Utils::Stream::ClearPointer(&dest->elemDefs);
		}

		buffer->popBlock();
	}
}
