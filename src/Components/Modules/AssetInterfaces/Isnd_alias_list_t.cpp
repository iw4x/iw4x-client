#include "Isnd_alias_list_t.hpp"

namespace Assets
{
  void Isnd_alias_list_t::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
  {
    header->sound = builder->getIW4OfApi()->read<Game::snd_alias_list_t>(Game::XAssetType::ASSET_TYPE_SOUND, name);

    if (!header->sound)
    {
      header->sound = Components::AssetHandler::FindOriginalAsset(Game::XAssetType::ASSET_TYPE_SOUND_CURVE, name.data()).sound;
    }
  }

  void Isnd_alias_list_t::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
  {
    Game::snd_alias_list_t* asset = header.sound;

    // An alias list is only registered with the zone after it has been
    // marked, so aliases whose secondaries refer back to each other (or to
    // themselves) would otherwise be loaded over and over. Remember the lists
    // being marked and skip secondaries that name one of them.
    //
    static std::unordered_set<std::string> marking;

    const std::string name(asset->aliasName ? asset->aliasName : "");
    const bool inserted(marking.insert(name).second);

    const auto _0 = gsl::finally([&name, inserted]
    {
      if (inserted)
        marking.erase(name);
    });

    for (unsigned int i = 0; i < asset->count; ++i)
    {
      Game::snd_alias_t* alias = &asset->head[i];

      if (alias->soundFile && alias->soundFile->type == Game::snd_alias_type_t::SAT_LOADED)
      {
        builder->loadAsset(Game::XAssetType::ASSET_TYPE_LOADED_SOUND, alias->soundFile->u.loadSnd);
      }

      if (alias->volumeFalloffCurve)
      {
        if (!builder->loadAsset(Game::XAssetType::ASSET_TYPE_SOUND_CURVE, alias->volumeFalloffCurve))
        {
          // (Should never happen, but just in case)
          alias->volumeFalloffCurve->filename = "$default";
          builder->loadAsset(Game::XAssetType::ASSET_TYPE_SOUND_CURVE, alias->volumeFalloffCurve);
        }
      }

      // The secondary alias is only referenced by name, so it has to be built
      // into the zone as an asset of its own, like the sounds weapons refer to.
      //
      if (alias->secondaryAliasName && *alias->secondaryAliasName && !marking.contains(alias->secondaryAliasName))
      {
        builder->loadAssetByName(Game::XAssetType::ASSET_TYPE_SOUND, alias->secondaryAliasName, false);
      }
    }
  }

  bool Isnd_alias_list_t::IsSafeDumpName(const char* name)
  {
    // Note that the exporter joins asset names onto its work path as-is, so
    // a name from a third-party fastfile could otherwise write anywhere.
    //
    // @@ This really belongs in iw4of's get_work_path().
    //
    if (!name || !*name)
    {
      return true;
    }

    const std::string_view view(name);
    return view.find("..") == std::string_view::npos && view.find(':') == std::string_view::npos && view.front() != '/' && view.front() != '\\';
  }

  bool Isnd_alias_list_t::IsSafeToDump(const Game::snd_alias_list_t* asset)
  {
    if (!asset || !IsSafeDumpName(asset->aliasName))
    {
      return false;
    }

    for (unsigned int i = 0; i < asset->count; ++i)
    {
      const auto* alias = &asset->head[i];
      if (alias->soundFile && alias->soundFile->type == Game::snd_alias_type_t::SAT_LOADED && alias->soundFile->u.loadSnd
        && !IsSafeDumpName(alias->soundFile->u.loadSnd->name))
      {
        return false;
      }
    }

    return true;
  }

  void Isnd_alias_list_t::dump(Game::XAssetHeader header)
  {
    if (!IsSafeToDump(header.sound))
    {
      Components::Logger::Print("Refusing to dump sound with an unsafe asset name\n");
      return;
    }

    Components::ZoneBuilder::GetExporter()->write(Game::XAssetType::ASSET_TYPE_SOUND, header.data);
  }

  Isnd_alias_list_t::Isnd_alias_list_t()
  {
    Components::Command::Add("dumpSound", [this](const Components::Command::Params* param)
    {
      const auto header = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_SOUND, param->get(1));
      if (header.data)
      {
        Components::ZoneBuilder::RefreshExporterWorkDirectory();
        this->dump(header);
      }
    });
  }

  void Isnd_alias_list_t::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
  {
    AssertSize(Game::snd_alias_list_t, 12);

    auto* buffer = builder->getBuffer();
    auto* asset = header.sound;
    auto* dest = buffer->dest<Game::snd_alias_list_t>();
    buffer->save(asset);

    buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

    if (asset->aliasName)
    {
      if (builder->hasPointer(asset->aliasName))
      {
        dest->aliasName = builder->getPointer(asset->aliasName);
      }
      else
      {
        builder->storePointer(asset->aliasName);
        buffer->saveString(asset->aliasName);
        Utils::Stream::ClearPointer(&dest->aliasName);
      }
    }

    if (asset->head)
    {
      if (builder->hasPointer(asset->head))
      {
        dest->head = builder->getPointer(asset->head);
      }
      else
      {
        AssertSize(Game::snd_alias_t, 100);

        buffer->align(Utils::Stream::ALIGN_4);
        builder->storePointer(asset->head);

        auto* destHead = buffer->dest<Game::snd_alias_t>();
        buffer->saveArray(asset->head, asset->count);

        for (unsigned int i = 0; i < asset->count; ++i)
        {
          Game::snd_alias_t* destAlias = &destHead[i];
          Game::snd_alias_t* alias = &asset->head[i];

          if (alias->aliasName)
          {
            if (builder->hasPointer(alias->aliasName))
            {
              destAlias->aliasName = builder->getPointer(alias->aliasName);
            }
            else
            {
              builder->storePointer(alias->aliasName);
              buffer->saveString(alias->aliasName);
              Utils::Stream::ClearPointer(&destAlias->aliasName);
            }
          }

          if (alias->subtitle)
          {
            buffer->saveString(alias->subtitle);
            Utils::Stream::ClearPointer(&destAlias->subtitle);
          }

          if (alias->secondaryAliasName)
          {
            buffer->saveString(alias->secondaryAliasName);
            Utils::Stream::ClearPointer(&destAlias->secondaryAliasName);
          }

          if (alias->chainAliasName)
          {
            buffer->saveString(alias->chainAliasName);
            Utils::Stream::ClearPointer(&destAlias->chainAliasName);
          }

          if (alias->mixerGroup)
          {
            buffer->saveString(alias->mixerGroup);
            Utils::Stream::ClearPointer(&destAlias->mixerGroup);
          }

          if (alias->soundFile)
          {
            if (builder->hasPointer(alias->soundFile))
            {
              destAlias->soundFile = builder->getPointer(alias->soundFile);
            }
            else
            {
              AssertSize(Game::snd_alias_t, 100);

              buffer->align(Utils::Stream::ALIGN_4);
              builder->storePointer(alias->soundFile);

              auto* destSoundFile = buffer->dest<Game::SoundFile>();
              buffer->save(alias->soundFile);

              // Save_SoundFileRef
              {
                if (alias->soundFile->type == Game::snd_alias_type_t::SAT_LOADED)
                {
                  destSoundFile->u.loadSnd = builder->saveSubAsset(Game::ASSET_TYPE_LOADED_SOUND, alias->soundFile->u.loadSnd).loadSnd;
                }
                else
                {
                  // Save_StreamedSound
                  {
                    if (alias->soundFile->u.streamSnd.filename.info.raw.dir)
                    {
                      buffer->saveString(alias->soundFile->u.streamSnd.filename.info.raw.dir);
                      Utils::Stream::ClearPointer(&destSoundFile->u.streamSnd.filename.info.raw.dir);
                    }

                    if (alias->soundFile->u.streamSnd.filename.info.raw.name)
                    {
                      buffer->saveString(alias->soundFile->u.streamSnd.filename.info.raw.name);
                      Utils::Stream::ClearPointer(&destSoundFile->u.streamSnd.filename.info.raw.name);
                    }
                  }
                }
              }

              Utils::Stream::ClearPointer(&destAlias->soundFile);
            }
          }

          if (alias->volumeFalloffCurve)
          {
            destAlias->volumeFalloffCurve = builder->saveSubAsset(Game::ASSET_TYPE_SOUND_CURVE, alias->volumeFalloffCurve).sndCurve;
          }

          if (alias->speakerMap)
          {
            if (builder->hasPointer(alias->speakerMap))
            {
              destAlias->speakerMap = builder->getPointer(alias->speakerMap);
            }
            else
            {
              AssertSize(Game::SpeakerMap, 408);

              buffer->align(Utils::Stream::ALIGN_4);
              builder->storePointer(alias->speakerMap);

              auto* destSoundFile = buffer->dest<Game::SpeakerMap>();
              buffer->save(alias->speakerMap);

              if (alias->speakerMap->name)
              {
                buffer->saveString(alias->speakerMap->name);
                Utils::Stream::ClearPointer(&destSoundFile->name);
              }

              Utils::Stream::ClearPointer(&destAlias->speakerMap);
            }
          }
        }

        Utils::Stream::ClearPointer(&dest->head);
      }
    }

    buffer->popBlock();
  }
}
