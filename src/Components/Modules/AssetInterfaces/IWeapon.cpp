#include <STDInclude.hpp>
#include "IWeapon.hpp"

namespace Assets
{
	void IWeapon::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* /*builder*/)
	{
		// Try loading raw weapon
		if (Components::FileSystem::File(std::format("weapons/mp/{}", name)))
		{
			// let the function see temporary assets when calling DB_FindXAssetHeader during the loading function
			// otherwise it fails to link things properly
			Components::AssetHandler::ExposeTemporaryAssets(true);
			header->data = Game::BG_LoadWeaponDef_LoadObj(name.data());
			Components::AssetHandler::ExposeTemporaryAssets(false);
		}
	}

	void IWeapon::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::WeaponCompleteDef* asset = header.weapon;

		// convert all script strings
		if (asset->hideTags)
		{
			for (char i = 0; i < 32; ++i)
			{
				if (asset->hideTags[i] == NULL) break; // no more strings
				builder->addScriptString(asset->hideTags[i]);
			}
		}

		if (asset->weapDef->notetrackSoundMapKeys)
		{
			for (char i = 0; i < 16; ++i)
			{
				if (asset->weapDef->notetrackSoundMapKeys[i] == NULL) break; // no more strings
				builder->addScriptString(asset->weapDef->notetrackSoundMapKeys[i]);
			}
		}

		if (asset->weapDef->notetrackSoundMapValues)
		{
			for (char i = 0; i < 16; ++i)
			{
				if (asset->weapDef->notetrackSoundMapValues[i] == NULL) break; // no more strings
				builder->addScriptString(asset->weapDef->notetrackSoundMapValues[i]);
			}
		}

		if (asset->weapDef->notetrackRumbleMapKeys)
		{
			for (char i = 0; i < 16; ++i)
			{
				if (asset->weapDef->notetrackRumbleMapKeys[i] == NULL) break; // no more strings
				builder->addScriptString(asset->weapDef->notetrackRumbleMapKeys[i]);
			}
		}

		if (asset->weapDef->notetrackRumbleMapValues)
		{
			for (char i = 0; i < 16; ++i)
			{
				if (asset->weapDef->notetrackRumbleMapValues[i] == NULL) break; // no more strings
				builder->addScriptString(asset->weapDef->notetrackRumbleMapValues[i]);
			}
		}


		// now load all sub-assets properly
		if (asset->killIcon) builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->killIcon);
		if (asset->dpadIcon) builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->dpadIcon);
		if (asset->weapDef->reticleCenter) builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->weapDef->reticleCenter);
		if (asset->weapDef->reticleSide) builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->weapDef->reticleSide);
		if (asset->weapDef->hudIcon) builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->weapDef->hudIcon);
		if (asset->weapDef->pickupIcon) builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->weapDef->pickupIcon);
		if (asset->weapDef->ammoCounterIcon) builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->weapDef->ammoCounterIcon);
		if (asset->weapDef->overlayMaterial) builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->weapDef->overlayMaterial);
		if (asset->weapDef->overlayMaterialLowRes) builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->weapDef->overlayMaterialLowRes);
		if (asset->weapDef->overlayMaterialEMP) builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->weapDef->overlayMaterialEMP);
		if (asset->weapDef->overlayMaterialEMPLowRes) builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->weapDef->overlayMaterialEMPLowRes);

		if (asset->weapDef->gunXModel)
		{
			for (int i = 0; i < 16; i++)
			{
				if (asset->weapDef->gunXModel[i]) builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->weapDef->gunXModel[i]);
			}
		}

		if (asset->weapDef->handXModel) builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->weapDef->handXModel);

		if (asset->weapDef->worldModel)
		{
			for (int i = 0; i < 16; i++)
			{
				if (asset->weapDef->worldModel[i]) builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->weapDef->worldModel[i]);
			}
		}

		if (asset->weapDef->worldClipModel) builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->weapDef->worldClipModel);
		if (asset->weapDef->rocketModel) builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->weapDef->rocketModel);
		if (asset->weapDef->knifeModel) builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->weapDef->knifeModel);
		if (asset->weapDef->worldKnifeModel) builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->weapDef->worldKnifeModel);
		if (asset->weapDef->projectileModel) builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->weapDef->projectileModel);

		if (asset->weapDef->physCollmap) builder->loadAsset(Game::XAssetType::ASSET_TYPE_PHYSCOLLMAP, asset->weapDef->physCollmap);

		if (asset->weapDef->tracerType) builder->loadAsset(Game::XAssetType::ASSET_TYPE_TRACER, asset->weapDef->tracerType);

		if (asset->weapDef->viewFlashEffect) builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, asset->weapDef->viewFlashEffect);
		if (asset->weapDef->worldFlashEffect) builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, asset->weapDef->worldFlashEffect);
		if (asset->weapDef->viewShellEjectEffect) builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, asset->weapDef->viewShellEjectEffect);
		if (asset->weapDef->worldShellEjectEffect) builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, asset->weapDef->worldShellEjectEffect);
		if (asset->weapDef->viewLastShotEjectEffect) builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, asset->weapDef->viewLastShotEjectEffect);
		if (asset->weapDef->worldLastShotEjectEffect) builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, asset->weapDef->worldLastShotEjectEffect);
		if (asset->weapDef->projExplosionEffect) builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, asset->weapDef->projExplosionEffect);
		if (asset->weapDef->projDudEffect) builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, asset->weapDef->projDudEffect);
		if (asset->weapDef->projTrailEffect) builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, asset->weapDef->projTrailEffect);
		if (asset->weapDef->projBeaconEffect) builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, asset->weapDef->projBeaconEffect);
		if (asset->weapDef->projIgnitionEffect) builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, asset->weapDef->projIgnitionEffect);
		if (asset->weapDef->turretOverheatEffect) builder->loadAsset(Game::XAssetType::ASSET_TYPE_FX, asset->weapDef->turretOverheatEffect);

#define LoadWeapSound(sound) if (asset->weapDef->##sound##) builder->loadAsset(Game::XAssetType::ASSET_TYPE_SOUND, asset->weapDef->##sound##)

		LoadWeapSound(pickupSound);
		LoadWeapSound(pickupSoundPlayer);
		LoadWeapSound(ammoPickupSound);
		LoadWeapSound(ammoPickupSoundPlayer);
		LoadWeapSound(projectileSound);
		LoadWeapSound(pullbackSound);
		LoadWeapSound(pullbackSoundPlayer);
		LoadWeapSound(fireSound);
		LoadWeapSound(fireSoundPlayer);
		LoadWeapSound(fireSoundPlayerAkimbo);
		LoadWeapSound(fireLoopSound);
		LoadWeapSound(fireLoopSoundPlayer);
		LoadWeapSound(fireStopSound);
		LoadWeapSound(fireStopSoundPlayer);
		LoadWeapSound(fireLastSound);
		LoadWeapSound(fireLastSoundPlayer);
		LoadWeapSound(emptyFireSound);
		LoadWeapSound(emptyFireSoundPlayer);
		LoadWeapSound(meleeSwipeSound);
		LoadWeapSound(meleeSwipeSoundPlayer);
		LoadWeapSound(meleeHitSound);
		LoadWeapSound(meleeMissSound);
		LoadWeapSound(rechamberSound);
		LoadWeapSound(rechamberSoundPlayer);
		LoadWeapSound(reloadSound);
		LoadWeapSound(reloadSoundPlayer);
		LoadWeapSound(reloadEmptySound);
		LoadWeapSound(reloadEmptySoundPlayer);
		LoadWeapSound(reloadStartSound);
		LoadWeapSound(reloadStartSoundPlayer);
		LoadWeapSound(reloadEndSound);
		LoadWeapSound(reloadEndSoundPlayer);
		LoadWeapSound(detonateSound);
		LoadWeapSound(detonateSoundPlayer);
		LoadWeapSound(nightVisionWearSound);
		LoadWeapSound(nightVisionWearSoundPlayer);
		LoadWeapSound(nightVisionRemoveSound);
		LoadWeapSound(nightVisionRemoveSoundPlayer);
		LoadWeapSound(altSwitchSound);
		LoadWeapSound(altSwitchSoundPlayer);
		LoadWeapSound(raiseSound);
		LoadWeapSound(raiseSoundPlayer);
		LoadWeapSound(firstRaiseSound);
		LoadWeapSound(firstRaiseSoundPlayer);
		LoadWeapSound(putawaySound);
		LoadWeapSound(putawaySoundPlayer);
		LoadWeapSound(scanSound);

		if (asset->weapDef->bounceSound)
		{
			for (size_t i = 0; i < 31; i++)
			{
				LoadWeapSound(bounceSound[i]);
			}
		}

		LoadWeapSound(projExplosionSound);
		LoadWeapSound(projDudSound);
		LoadWeapSound(projIgnitionSound);
		LoadWeapSound(turretOverheatSound);
		LoadWeapSound(turretBarrelSpinMaxSnd);

		for (size_t i = 0; i < 4; i++)
		{
			LoadWeapSound(turretBarrelSpinUpSnd[i]);
			LoadWeapSound(turretBarrelSpinDownSnd[i]);
		}

		LoadWeapSound(missileConeSoundAlias);
		LoadWeapSound(missileConeSoundAliasAtBase);
	}

	void IWeapon::writeWeaponDef(Game::WeaponDef* def, Components::ZoneBuilder::Zone* builder, Utils::Stream* buffer)
	{
		AssertSize(Game::WeaponDef, 0x684);

		Game::WeaponDef* dest = buffer->dest<Game::WeaponDef>();
		buffer->save(def);

		if (def->szOverlayName)
		{
			buffer->saveString(def->szOverlayName);
			Utils::Stream::ClearPointer(&dest->szOverlayName);
		}

		if (def->gunXModel)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			Game::XModel** pointerTable = buffer->dest<Game::XModel*>();
			buffer->saveMax(16 * sizeof(Game::XModel*));
			for (int i = 0; i < 16; i++)
			{
				if (!def->gunXModel[i])
				{
					pointerTable[i] = NULL;
					continue;
				}
				pointerTable[i] = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_XMODEL, def->gunXModel[i]).model;
			}
			Utils::Stream::ClearPointer(&dest->gunXModel);
		}

		if (def->handXModel)
		{
			dest->handXModel = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_XMODEL, def->handXModel).model;
		}

		if (def->szXAnimsRightHanded)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			int* poinerTable = buffer->dest<int>();
			buffer->saveMax(37 * sizeof(char*)); // array of 37 string pointers
			for (int i = 0; i < 37; i++)
			{
				if (!def->szXAnimsRightHanded[i]) {
					poinerTable[i] = 0; // clear poiner if there isn't a string here
					continue;
				}

				// save string if it is present
				buffer->saveString(def->szXAnimsRightHanded[i]);
			}

			Utils::Stream::ClearPointer(&dest->szXAnimsRightHanded);
		}

		if (def->szXAnimsLeftHanded)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			int* poinerTable = buffer->dest<int>();
			buffer->saveMax(37 * sizeof(char*)); // array of 37 string pointers
			for (int i = 0; i < 37; i++)
			{
				if (!def->szXAnimsLeftHanded[i]) {
					poinerTable[i] = 0; // clear poiner if there isn't a string here
					continue;
				}

				// save string if it is present
				buffer->saveString(def->szXAnimsLeftHanded[i]);
			}

			Utils::Stream::ClearPointer(&dest->szXAnimsLeftHanded);
		}

		if (def->szModeName)
		{
			buffer->saveString(def->szModeName);
			Utils::Stream::ClearPointer(&dest->szModeName);
		}

		if (def->notetrackSoundMapKeys)
		{
			buffer->align(Utils::Stream::ALIGN_2);
			unsigned short* scriptStringTable = buffer->dest<unsigned short>();
			buffer->saveArray(def->notetrackSoundMapKeys, 16);
			for (int i = 0; i < 16; i++) {
				builder->mapScriptString(scriptStringTable[i]);
			}

			Utils::Stream::ClearPointer(&dest->notetrackSoundMapKeys);
		}

		if (def->notetrackSoundMapValues)
		{
			buffer->align(Utils::Stream::ALIGN_2);
			unsigned short* scriptStringTable = buffer->dest<unsigned short>();
			buffer->saveArray(def->notetrackSoundMapValues, 16);
			for (int i = 0; i < 16; i++) {
				builder->mapScriptString(scriptStringTable[i]);
			}

			Utils::Stream::ClearPointer(&dest->notetrackSoundMapValues);
		}

		if (def->notetrackRumbleMapKeys)
		{
			buffer->align(Utils::Stream::ALIGN_2);
			unsigned short* scriptStringTable = buffer->dest<unsigned short>();
			buffer->saveArray(def->notetrackRumbleMapKeys, 16);
			for (int i = 0; i < 16; i++) {
				builder->mapScriptString(scriptStringTable[i]);
			}

			Utils::Stream::ClearPointer(&dest->notetrackRumbleMapKeys);
		}

		if (def->notetrackRumbleMapValues)
		{
			buffer->align(Utils::Stream::ALIGN_2);
			unsigned short* scriptStringTable = buffer->dest<unsigned short>();
			buffer->saveArray(def->notetrackRumbleMapValues, 16);
			for (int i = 0; i < 16; i++) {
				builder->mapScriptString(scriptStringTable[i]);
			}

			Utils::Stream::ClearPointer(&dest->notetrackRumbleMapValues);
		}

		if (def->viewFlashEffect)
		{
			dest->viewFlashEffect = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_FX, def->viewFlashEffect).fx;
		}

		if (def->worldFlashEffect)
		{
			dest->worldFlashEffect = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_FX, def->worldFlashEffect).fx;
		}

		// This is compressed because I don't want to write the same piece of code 47 times
		// TODO: verify that this is saving the aliases correctly because the old code looks wrong and this looks right but the old code worked so go figure
		Game::snd_alias_list_t ** allSounds = &def->pickupSound;
		Game::snd_alias_list_t ** allSoundsDest = &dest->pickupSound;
		for (int i = 0; i < 47; i++) {
			if (!allSounds[i]) continue;
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveMax(sizeof(Game::snd_alias_list_t*));
			buffer->saveString(allSounds[i]->aliasName);
			Utils::Stream::ClearPointer(&allSoundsDest[i]);
		}

		if (def->bounceSound)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			int* ptrs = buffer->dest<int>();
			buffer->saveMax(31 * sizeof(Game::snd_alias_list_t*));

			for (int i = 0; i < 31; i++)
			{
				if (!def->bounceSound[i])
				{
					ptrs[i] = 0;
					continue;
				}

				buffer->saveMax(sizeof(Game::snd_alias_list_t*));
				buffer->saveString(def->bounceSound[i]->aliasName);
			}

			Utils::Stream::ClearPointer(&dest->bounceSound);
		}

		if (def->viewShellEjectEffect)
		{
			dest->viewShellEjectEffect = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_FX, def->viewShellEjectEffect).fx;
		}

		if (def->worldShellEjectEffect)
		{
			dest->worldShellEjectEffect = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_FX, def->worldShellEjectEffect).fx;
		}

		if (def->viewLastShotEjectEffect)
		{
			dest->viewLastShotEjectEffect = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_FX, def->viewLastShotEjectEffect).fx;
		}

		if (def->worldLastShotEjectEffect)
		{
			dest->worldLastShotEjectEffect = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_FX, def->worldLastShotEjectEffect).fx;
		}

		if (def->reticleCenter)
		{
			dest->reticleCenter = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, def->reticleCenter).material;
		}

		if (def->reticleSide)
		{
			dest->reticleSide = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, def->reticleSide).material;
		}

		if (def->worldModel)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			Game::XModel** pointerTable = buffer->dest<Game::XModel*>();
			buffer->saveMax(16 * sizeof(Game::XModel*));
			for (int i = 0; i < 16; i++)
			{
				if (!def->worldModel[i])
				{
					pointerTable[i] = NULL;
					continue;
				}
				pointerTable[i] = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_XMODEL, def->worldModel[i]).model;
			}
			Utils::Stream::ClearPointer(&dest->worldModel);
		}

		if (def->worldClipModel)
		{
			dest->worldClipModel = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_XMODEL, def->worldClipModel).model;
		}

		if (def->rocketModel)
		{
			dest->rocketModel = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_XMODEL, def->rocketModel).model;
		}

		if (def->knifeModel)
		{
			dest->knifeModel = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_XMODEL, def->knifeModel).model;
		}

		if (def->worldKnifeModel)
		{
			dest->worldKnifeModel = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_XMODEL, def->worldKnifeModel).model;
		}

		if (def->hudIcon)
		{
			dest->hudIcon = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, def->hudIcon).material;
		}

		if (def->pickupIcon)
		{
			dest->pickupIcon = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, def->pickupIcon).material;
		}

		if (def->ammoCounterIcon)
		{
			dest->ammoCounterIcon = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, def->ammoCounterIcon).material;
		}

		if (def->szAmmoName)
		{
			buffer->saveString(def->szAmmoName);
			Utils::Stream::ClearPointer(&dest->szAmmoName);
		}

		if (def->szClipName)
		{
			buffer->saveString(def->szClipName);
			Utils::Stream::ClearPointer(&dest->szClipName);
		}

		if (def->szSharedAmmoCapName)
		{
			buffer->saveString(def->szSharedAmmoCapName);
			Utils::Stream::ClearPointer(&dest->szSharedAmmoCapName);
		}

		if (def->overlayMaterial)
		{
			dest->overlayMaterial = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, def->overlayMaterial).material;
		}

		if (def->overlayMaterialLowRes)
		{
			dest->overlayMaterialLowRes = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, def->overlayMaterialLowRes).material;
		}

		if (def->overlayMaterialEMP)
		{
			dest->overlayMaterialEMP = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, def->overlayMaterialEMP).material;
		}

		if (def->overlayMaterialEMPLowRes)
		{
			dest->overlayMaterialEMPLowRes = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, def->overlayMaterialEMPLowRes).material;
		}

		if (def->physCollmap)
		{
			dest->physCollmap = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_PHYSCOLLMAP, def->overlayMaterialEMPLowRes).physCollmap;
		}

		if (def->projectileModel)
		{
			dest->projectileModel = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_XMODEL, def->projectileModel).model;
		}

		if (def->projExplosionEffect)
		{
			dest->projExplosionEffect = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_FX, def->projExplosionEffect).fx;
		}

		if (def->projDudEffect)
		{
			dest->projDudEffect = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_FX, def->projDudEffect).fx;
		}

		if (def->projExplosionSound)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveMax(sizeof(Game::snd_alias_list_t*));
			buffer->saveString(def->projExplosionSound->aliasName);
			Utils::Stream::ClearPointer(&dest->projExplosionSound);
		}

		if (def->projDudSound)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveMax(sizeof(Game::snd_alias_list_t*));
			buffer->saveString(def->projDudSound->aliasName);
			Utils::Stream::ClearPointer(&dest->projDudSound);
		}

		if (def->parallelBounce)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(def->parallelBounce, 31);
			Utils::Stream::ClearPointer(&dest->parallelBounce);
		}

		if (def->perpendicularBounce)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(def->perpendicularBounce, 31);
			Utils::Stream::ClearPointer(&dest->perpendicularBounce);
		}

		if (def->projTrailEffect)
		{
			dest->projTrailEffect = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_FX, def->projTrailEffect).fx;
		}

		if (def->projBeaconEffect)
		{
			dest->projBeaconEffect = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_FX, def->projBeaconEffect).fx;
		}

		if (def->projIgnitionEffect)
		{
			dest->projIgnitionEffect = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_FX, def->projIgnitionEffect).fx;
		}

		if (def->projIgnitionSound)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveMax(sizeof(Game::snd_alias_list_t*));
			buffer->saveString(def->projIgnitionSound->aliasName);
			Utils::Stream::ClearPointer(&dest->projIgnitionSound);
		}

		if (def->accuracyGraphName[0])
		{
			buffer->saveString(def->accuracyGraphName[0]);
			Utils::Stream::ClearPointer(&dest->accuracyGraphName[0]);
		}

		if (def->originalAccuracyGraphKnots[0])
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(def->originalAccuracyGraphKnots[0], def->originalAccuracyGraphKnotCount[0]);
			Utils::Stream::ClearPointer(&dest->originalAccuracyGraphKnots[0]);
		}

		if (def->accuracyGraphName[1])
		{
			buffer->saveString(def->accuracyGraphName[1]);
			Utils::Stream::ClearPointer(&dest->accuracyGraphName[1]);
		}

		if (def->originalAccuracyGraphKnots[1])
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(def->originalAccuracyGraphKnots[1], def->originalAccuracyGraphKnotCount[1]);
			Utils::Stream::ClearPointer(&dest->originalAccuracyGraphKnots[1]);
		}

		if (def->szUseHintString)
		{
			buffer->saveString(def->szUseHintString);
			Utils::Stream::ClearPointer(&dest->szUseHintString);
		}

		if (def->dropHintString)
		{
			buffer->saveString(def->dropHintString);
			Utils::Stream::ClearPointer(&dest->dropHintString);
		}

		if (def->szScript)
		{
			buffer->saveString(def->szScript);
			Utils::Stream::ClearPointer(&dest->szScript);
		}
		
		if (def->locationDamageMultipliers)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(def->locationDamageMultipliers, 20);
			Utils::Stream::ClearPointer(&dest->locationDamageMultipliers);
		}

		if (def->fireRumble)
		{
			buffer->saveString(def->fireRumble);
			Utils::Stream::ClearPointer(&dest->fireRumble);
		}

		if (def->meleeImpactRumble)
		{
			buffer->saveString(def->meleeImpactRumble);
			Utils::Stream::ClearPointer(&dest->meleeImpactRumble);
		}

		if (def->tracerType)
		{
			dest->tracerType = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_TRACER, def->tracerType).tracerDef;
		}

		if (def->turretOverheatSound)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveMax(sizeof(Game::snd_alias_list_t*));
			buffer->saveString(def->turretOverheatSound->aliasName);
			Utils::Stream::ClearPointer(&dest->turretOverheatSound);
		}

		if (def->turretOverheatEffect)
		{
			dest->turretOverheatEffect = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_FX, def->turretOverheatEffect).fx;
		}

		if (def->turretBarrelSpinRumble)
		{
			buffer->saveString(def->turretBarrelSpinRumble);
			Utils::Stream::ClearPointer(&dest->turretBarrelSpinRumble);
		}

		if (def->turretBarrelSpinMaxSnd)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveMax(sizeof(Game::snd_alias_list_t*));
			buffer->saveString(def->turretBarrelSpinMaxSnd->aliasName);
			Utils::Stream::ClearPointer(&dest->turretBarrelSpinMaxSnd);
		}

		for (int i = 0; i < 4; i++) {
			if (!def->turretBarrelSpinUpSnd[i]) continue;

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveMax(sizeof(Game::snd_alias_list_t*));
			buffer->saveString(def->turretBarrelSpinUpSnd[i]->aliasName);
			Utils::Stream::ClearPointer(&dest->turretBarrelSpinUpSnd[i]);
		}

		for (int i = 0; i < 4; i++) {
			if (!def->turretBarrelSpinDownSnd[i]) continue;

			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveMax(sizeof(Game::snd_alias_list_t*));
			buffer->saveString(def->turretBarrelSpinDownSnd[i]->aliasName);
			Utils::Stream::ClearPointer(&dest->turretBarrelSpinDownSnd[i]);
		}

		if (def->missileConeSoundAlias)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveMax(sizeof(Game::snd_alias_list_t*));
			buffer->saveString(def->missileConeSoundAlias->aliasName);
			Utils::Stream::ClearPointer(&dest->missileConeSoundAlias);
		}

		if (def->missileConeSoundAliasAtBase)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveMax(sizeof(Game::snd_alias_list_t*));
			buffer->saveString(def->missileConeSoundAliasAtBase->aliasName);
			Utils::Stream::ClearPointer(&dest->missileConeSoundAliasAtBase);
		}
	}

	void IWeapon::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::WeaponCompleteDef, 0x74);

		Utils::Stream* buffer = builder->getBuffer();
		Game::WeaponCompleteDef* asset = header.weapon;
		Game::WeaponCompleteDef* dest = buffer->dest<Game::WeaponCompleteDef>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->szInternalName)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->szInternalName));
			Utils::Stream::ClearPointer(&dest->szInternalName);
		}

		if (asset->weapDef)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			IWeapon::writeWeaponDef(asset->weapDef, builder, buffer);

			Utils::Stream::ClearPointer(&dest->weapDef);
		}

		if (asset->szDisplayName)
		{
			buffer->saveString(asset->szDisplayName);
			Utils::Stream::ClearPointer(&dest->szDisplayName);
		}

		if (asset->hideTags)
		{
			buffer->align(Utils::Stream::ALIGN_2);
			unsigned short* scriptStringTable = buffer->dest<unsigned short>();
			buffer->saveArray(asset->hideTags, 32);
			for (int i = 0; i < 32; i++) {
				builder->mapScriptString(scriptStringTable[i]);
			}

			Utils::Stream::ClearPointer(&dest->hideTags);
		}

		if (asset->szXAnims)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			int* poinerTable = buffer->dest<int>();
			buffer->saveMax(37 * sizeof(char*)); // array of 37 string pointers
			for (int i = 0; i < 37; i++)
			{
				if (!asset->szXAnims[i]) {
					poinerTable[i] = 0; // clear poiner if there isn't a string here
					continue;
				}

				// save string if it is present
				buffer->saveString(asset->szXAnims[i]);
			}

			Utils::Stream::ClearPointer(&dest->szXAnims);
		}

		if (asset->szAltWeaponName)
		{
			buffer->saveString(asset->szAltWeaponName);
			Utils::Stream::ClearPointer(&dest->szAltWeaponName);
		}

		if (asset->killIcon)
		{
			dest->killIcon = builder->saveSubAsset(Game::ASSET_TYPE_MATERIAL, asset->killIcon).material;
		}

		if (asset->dpadIcon)
		{
			dest->dpadIcon = builder->saveSubAsset(Game::ASSET_TYPE_MATERIAL, asset->dpadIcon).material;
		}

		if (asset->accuracyGraphKnots[0])
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->accuracyGraphKnots[0], asset->accuracyGraphKnotCount[0]);
			Utils::Stream::ClearPointer(&dest->accuracyGraphKnots[0]);
		}

		if (asset->accuracyGraphKnots[1])
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->accuracyGraphKnots[1], asset->accuracyGraphKnotCount[1]);
			Utils::Stream::ClearPointer(&dest->accuracyGraphKnots[1]);
		}

		buffer->popBlock();
	}
}
