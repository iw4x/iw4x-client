#include "STDInclude.hpp"

#define IW4X_MODEL_VERSION 5

namespace Assets
{
	void IWeapon::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
	}

	void IWeapon::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::WeaponCompleteDef* asset = header.weapon;

        // convert all script strings
        for (char i = 0; i < 32; ++i)
        {
            if (asset->hideTags[i] == NULL) break; // no more strings
            builder->addScriptString(asset->hideTags[i]);
        }

        for (char i = 0; i < 16; ++i)
        {
            if (asset->weapDef->notetrackSoundMapKeys[i] == NULL) break; // no more strings
            builder->addScriptString(asset->weapDef->notetrackSoundMapKeys[i]);
        }

        for (char i = 0; i < 16; ++i)
        {
            if (asset->weapDef->notetrackSoundMapValues[i] == NULL) break; // no more strings
            builder->addScriptString(asset->weapDef->notetrackSoundMapValues[i]);
        }

        for (char i = 0; i < 16; ++i)
        {
            if (asset->weapDef->notetrackRumbleMapKeys[i] == NULL) break; // no more strings
            builder->addScriptString(asset->weapDef->notetrackRumbleMapKeys[i]);
        }

        for (char i = 0; i < 16; ++i)
        {
            if (asset->weapDef->notetrackRumbleMapValues[i] == NULL) break; // no more strings
            builder->addScriptString(asset->weapDef->notetrackRumbleMapValues[i]);
        }
        

        // now load all sub-assets properly
        builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->killIcon);
        builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->dpadIcon);
        builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->weapDef->reticleCenter);
        builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->weapDef->reticleSide);
        builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->weapDef->hudIcon);
        builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->weapDef->pickupIcon);
        builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->weapDef->ammoCounterIcon);
        builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->weapDef->overlayMaterial);
        builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->weapDef->overlayMaterialLowRes);
        builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->weapDef->overlayMaterialEMP);
        builder->loadAsset(Game::XAssetType::ASSET_TYPE_MATERIAL, asset->weapDef->overlayMaterialEMPLowRes);

        for (int i = 0; i < 16; i++)
        {
            builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->weapDef->gunXModel[i]);
        }

        builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->weapDef->handXModel);

        for (int i = 0; i < 16; i++)
        {
            builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->weapDef->worldModel[i]);
        }

        builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->weapDef->worldClipModel);
        builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->weapDef->rocketModel);
        builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->weapDef->knifeModel);
        builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->weapDef->worldKnifeModel);
        builder->loadAsset(Game::XAssetType::ASSET_TYPE_XMODEL, asset->weapDef->projectileModel);

        if (asset->weapDef->physCollmap)
        {
            builder->loadAsset(Game::XAssetType::ASSET_TYPE_PHYSCOLLMAP, asset->weapDef->physCollmap);
        }

        if (asset->weapDef->tracerType)
        {
            //builder->loadAsset(Game::XAssetType::ASSET_TYPE_TRACER, asset->weapDef->tracerType);
            asset->weapDef->tracerType = NULL;
        }

        // don't write effects for now
        asset->weapDef->viewFlashEffect = NULL;
        asset->weapDef->worldFlashEffect = NULL;
        asset->weapDef->viewShellEjectEffect = NULL;
        asset->weapDef->worldShellEjectEffect = NULL;
        asset->weapDef->viewLastShotEjectEffect = NULL;
        asset->weapDef->worldLastShotEjectEffect = NULL;
        asset->weapDef->projExplosionEffect = NULL;
        asset->weapDef->projDudEffect = NULL;
        asset->weapDef->projTrailEffect = NULL;
        asset->weapDef->projBeaconEffect = NULL;
        asset->weapDef->projIgnitionEffect = NULL;
        asset->weapDef->turretOverheatEffect = NULL;
	}

    void IWeapon::writeWeaponDef(Game::WeaponDef* def, Components::ZoneBuilder::Zone* builder, Utils::Stream* buffer)
    {
        Game::WeaponDef* dest = buffer->dest<Game::WeaponDef>();

        if (def->szOverlayName)
        {
            buffer->saveString(def->szOverlayName);
            Utils::Stream::ClearPointer(&dest->szOverlayName);
        }

        if (def->gunXModel)
        {
            Game::XModel** pointerTable = buffer->dest<Game::XModel*>();
            buffer->saveMax(16);
            for (int i = 0; i < 16; i++)
            {
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
            buffer->saveMax(37); // array of 37 string pointers
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
            buffer->saveMax(37); // array of 37 string pointers
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
                builder->mapScriptString(&scriptStringTable[i]);
            }
        }

        if (def->notetrackSoundMapValues)
        {
            buffer->align(Utils::Stream::ALIGN_2);
            unsigned short* scriptStringTable = buffer->dest<unsigned short>();
            buffer->saveArray(def->notetrackSoundMapValues, 16);
            for (int i = 0; i < 16; i++) {
                builder->mapScriptString(&scriptStringTable[i]);
            }
        }

        if (def->notetrackRumbleMapKeys)
        {
            buffer->align(Utils::Stream::ALIGN_2);
            unsigned short* scriptStringTable = buffer->dest<unsigned short>();
            buffer->saveArray(def->notetrackRumbleMapKeys, 16);
            for (int i = 0; i < 16; i++) {
                builder->mapScriptString(&scriptStringTable[i]);
            }
        }

        if (def->notetrackRumbleMapValues)
        {
            buffer->align(Utils::Stream::ALIGN_2);
            unsigned short* scriptStringTable = buffer->dest<unsigned short>();
            buffer->saveArray(def->notetrackRumbleMapValues, 16);
            for (int i = 0; i < 16; i++) {
                builder->mapScriptString(&scriptStringTable[i]);
            }
        }

        if (def->viewFlashEffect)
        {
            // not implemented yet
        }

        if (def->worldFlashEffect)
        {
            // not implemented yet
        }

        // etc. i'm bored so i'm going to work somewhere else for a bit
    }

	void IWeapon::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
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
			buffer->save(asset->hideTags, 32);
			Utils::Stream::ClearPointer(&dest->hideTags);
		}

		if (asset->szXAnims)
		{
			buffer->align(Utils::Stream::ALIGN_4);
            int* poinerTable = buffer->dest<int>();
            buffer->saveMax(37); // array of 37 string pointers
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
            asset->killIcon = builder->saveSubAsset(Game::ASSET_TYPE_MATERIAL, asset->killIcon).material;
        }

        if (asset->dpadIcon)
        {
            asset->dpadIcon = builder->saveSubAsset(Game::ASSET_TYPE_MATERIAL, asset->dpadIcon).material;
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
