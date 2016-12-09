#include <STDInclude.hpp>

namespace Assets
{
	void IFxEffectDef::load(Game::XAssetHeader* /*header*/, std::string name, Components::ZoneBuilder::Zone* /*builder*/)
	{
		if (0)
		{
			Components::FileSystem::File rawFx(fmt::sprintf("fx/%s.efx", name.data()));
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
					Components::Logger::Error("Effect needs to be updated from the legacy format.\n");
				}

				int version = atoi(Game::Com_Parse(&session));
				if (version > 2)
				{
					Game::Com_EndParseSession();
					Components::Logger::Error("Version %i is too high. I can only handle up to %i.\n", version, 2);
				}

				Game::FxEditorEffectDef efx;
				ZeroMemory(&efx, sizeof(efx));

// 				for (int i = 0; i < FX_ELEM_FIELD_COUNT; ++i)
// 				{
// 					Game::s_elemFields[i].handler(&session, efx.elems);
// 				}

				Game::Com_EndParseSession();
			}
		}
	}

	void IFxEffectDef::markFxElemVisuals(Game::FxElemVisuals* visuals, char elemType, Components::ZoneBuilder::Zone* builder)
	{
		switch (elemType)
		{
			case 7:
			{
				if (visuals->xmodel)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, visuals->xmodel->name);
				}

				break;
			}

			case 8:
			case 9:
				break;

			case 0xA:
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_SOUND, visuals->soundName);
				break;
			}

			case 0xC:
			{
				if (visuals->effectDef)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, visuals->effectDef->name);
				}

				break;
			}

			default:
			{
				if (visuals->material)
				{
					builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, visuals->material->name);
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
							if (elemDef->visuals.markArray[j].data[0])
							{
								builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, elemDef->visuals.markArray[j].data[0]->name);
							}

							if (elemDef->visuals.markArray[j].data[1])
							{
								builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, elemDef->visuals.markArray[j].data[1]->name);
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

			if (elemDef->effectOnImpact)
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, elemDef->effectOnImpact->name);
			}

			if (elemDef->effectOnDeath)
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, elemDef->effectOnDeath->name);
			}

			if (elemDef->effectEmitted)
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, elemDef->effectEmitted->name);
			}
		}
	}

	void IFxEffectDef::saveFxElemVisuals(Game::FxElemVisuals* visuals, Game::FxElemVisuals* destVisuals, char elemType, Components::ZoneBuilder::Zone* builder)
	{
		Utils::Stream* buffer = builder->getBuffer();

		switch (elemType)
		{
			case 7:
			{
				if (visuals->xmodel)
				{
					destVisuals->xmodel = builder->requireAsset(Game::XAssetType::ASSET_TYPE_XMODEL, visuals->xmodel->name).model;
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
					buffer->saveString(visuals->soundName);
					Utils::Stream::ClearPointer(&destVisuals->soundName);
				}

				break;
			}

			case 0xC:
			{
				if (visuals->effectDef)
				{
					buffer->saveString(visuals->effectDef->name);
					Utils::Stream::ClearPointer(&destVisuals->effectDef);
				}

				break;
			}

			default:
			{
				if (visuals->material)
				{
					destVisuals->material = builder->requireAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, visuals->material->name).material;
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
					if (elemDef->elemType == 11)
					{
						if (elemDef->visuals.markArray)
						{
							AssertSize(Game::FxElemMarkVisuals, 8);
							buffer->align(Utils::Stream::ALIGN_4);

							Game::FxElemMarkVisuals* destMarkArray = buffer->dest<Game::FxElemMarkVisuals>();
							buffer->saveArray(elemDef->visuals.markArray, elemDef->visualCount);

							for (char j = 0; j < elemDef->visualCount; ++j)
							{
								if (elemDef->visuals.markArray[j].data[0])
								{
									destMarkArray[j].data[0] = builder->requireAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, elemDef->visuals.markArray[j].data[0]->name).material;
								}

								if (elemDef->visuals.markArray[j].data[1])
								{
									destMarkArray[j].data[1] = builder->requireAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, elemDef->visuals.markArray[j].data[1]->name).material;
								}
							}

							Utils::Stream::ClearPointer(&destElemDef->visuals.markArray);
						}
					}
					else if(elemDef->visualCount > 1)
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

				if (elemDef->effectOnImpact)
				{
					buffer->saveString(elemDef->effectOnImpact->name);
					Utils::Stream::ClearPointer(&destElemDef->effectOnImpact);
				}

				if (elemDef->effectOnDeath)
				{
					buffer->saveString(elemDef->effectOnDeath->name);
					Utils::Stream::ClearPointer(&destElemDef->effectOnDeath);
				}

				if (elemDef->effectEmitted)
				{
					buffer->saveString(elemDef->effectEmitted->name);
					Utils::Stream::ClearPointer(&destElemDef->effectEmitted);
				}

				// Save_FxElemExtendedDefPtr
				{
					AssertSize(Game::FxElemExtendedDef, 4);

					if (elemDef->elemType == 3)
					{
						// Save_FxTrailDef
						{
							if (elemDef->extendedDef.trailDef)
							{
								AssertSize(Game::FxTrailDef, 36);
								buffer->align(Utils::Stream::ALIGN_4);

								Game::FxTrailDef* trailDef = elemDef->extendedDef.trailDef;
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

								Utils::Stream::ClearPointer(&destElemDef->extendedDef.trailDef);
							}
						}
					}
					else if (elemDef->elemType == 6)
					{
						if (elemDef->extendedDef.sparkFountain)
						{
							AssertSize(Game::FxSparkFountain, 52);
							buffer->align(Utils::Stream::ALIGN_4);

							buffer->save(elemDef->extendedDef.sparkFountain);
							Utils::Stream::ClearPointer(&destElemDef->extendedDef.sparkFountain);
						}
					}
					else
					{
						if (elemDef->extendedDef.unknownBytes)
						{
							buffer->save(elemDef->extendedDef.unknownBytes);
							Utils::Stream::ClearPointer(&destElemDef->extendedDef.unknownBytes);
						}
					}
				}
			}

			Utils::Stream::ClearPointer(&dest->elemDefs);
		}

		buffer->popBlock();
	}
}
