#include "STDInclude.hpp"

namespace Assets
{
	void Isnd_alias_list_t::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File aliasFile(Utils::String::VA("sounds/%s", name.c_str()));
		if (!aliasFile.exists())
		{
			header->sound = Components::AssetHandler::FindOriginalAsset(this->getType(), name.c_str()).sound;
			return;
		}

		Game::snd_alias_list_t* aliasList = builder->getAllocator()->allocate<Game::snd_alias_list_t>();
		if (!aliasList)
		{
			Components::Logger::Print("Error allocating memory for sound alias structure!\n");
			return;
		}

		std::string errors;
		json11::Json infoData = json11::Json::parse(aliasFile.getBuffer(), errors);
		json11::Json aliasesContainer = infoData["head"];

		auto aliases = aliasesContainer.array_items();

		aliasList->count = aliases.size();

		// Allocate
		aliasList->head = builder->getAllocator()->allocateArray<Game::snd_alias_t>(aliasList->count);
		if (!aliasList->head)
		{
			Components::Logger::Print("Error allocating memory for sound alias structure!\n");
			return;
		}

		aliasList->aliasName = builder->getAllocator()->duplicateString(infoData["aliasName"].string_value().c_str());

		for (size_t i = 0; i < aliasList->count; i++)
		{
			json11::Json head = aliasesContainer[i];

			if (!infoData.is_object())
			{
				Components::Logger::Error("Failed to load sound %s!", name.c_str());
				return;
			}

			aliasList->head->soundFile = builder->getAllocator()->allocate<Game::SoundFile>();
			if (!aliasList->head->soundFile)
			{
				Components::Logger::Print("Error allocating memory for sound alias structure!\n");
				return;
			}

			Game::snd_alias_t* alias = aliasList->head;

			// try and parse everything and if it fails then fail for the whole file
			auto type = head["type"];
			auto subtitle = head["subtitle"];
			auto secondaryAliasName = head["secondaryAliasName"];
			auto chainAliasName = head["chainAliasName"];
			auto soundFile = head["soundFile"];
			auto sequence = head["sequence"];
			auto volMin = head["volMin"];
			auto volMax = head["volMax"];
			auto pitchMin = head["pitchMin"];
			auto pitchMax = head["pitchMax"];
			auto distMin = head["distMin"];
			auto distMax = head["distMax"];
			auto flags = head["flags"];
			auto slavePercentage = head["slavePercentage"];
			auto probability = head["probability"];
			auto lfePercentage = head["lfePercentage"];
			auto centerPercentage = head["centerPercentage"];
			auto startDelay = head["startDelay"];
			auto volumeFalloffCurve = head["volumeFalloffCurve"];
			auto envelopMin = head["envelopMin"];
			auto envelopMax = head["envelopMax"];
			auto envelopPercentage = head["envelopPercentage"];
			auto speakerMap = head["speakerMap"];

			// Fix casing
			if (soundFile.is_null())
			{
				soundFile = head["soundfile"];

				Components::Logger::Print("Fixed casing on %s\n", name.c_str());
			}

			if (type.is_null() || soundFile.is_null())
			{
				Components::Logger::Print("Type is %s\n", type.dump().c_str());
				Components::Logger::Print("SoundFile is %s\n", soundFile.dump().c_str());
				Components::Logger::Error("Failed to parse sound %s! Each alias must have at least a type and a soundFile\n", name.c_str());
				return;
			}

#define CHECK(x, type) (x.is_##type##() || x.is_null())

			// TODO: actually support all of those properties
			if (!CHECK(type, number))
			{
				Components::Logger::Print("%s is not number but %d (%s)\n", "type", type.type(), type.dump().c_str());
			}

			if (!CHECK(subtitle, string))
			{
				Components::Logger::Print("%s is not string but %d (%s)\n", "subtitle", subtitle.type(), subtitle.dump().c_str());
			}

			if (!CHECK(secondaryAliasName, string))
			{
				Components::Logger::Print("%s is not string but %d (%s)\n", "secondaryAliasName", secondaryAliasName.type(), secondaryAliasName.dump().c_str());
			}

			if (!CHECK(chainAliasName, string))
			{
				Components::Logger::Print("%s is not string but %d (%s)\n", "chainAliasName", chainAliasName.type(), chainAliasName.dump().c_str());
			}

			if (!CHECK(soundFile, string))
			{
				Components::Logger::Print("%s is not string but %d (%s)\n", "soundFile", soundFile.type(), soundFile.dump().c_str());
			}

			if (!CHECK(sequence, number))
			{
				Components::Logger::Print("%s is not number but %d (%s)\n", "sequence", sequence.type(), sequence.dump().c_str());
			}

			if (!CHECK(volMin, number))
			{
				Components::Logger::Print("%s is not number but %d (%s)\n", "volMin", volMin.type(), volMin.dump().c_str());
			}

			if (!CHECK(volMax, number))
			{
				Components::Logger::Print("%s is not number but %d (%s)\n", "volMax", volMax.type(), volMax.dump().c_str());
			}

			if (!CHECK(pitchMin, number))
			{
				Components::Logger::Print("%s is not number but %d (%s)\n", "pitchMin", pitchMin.type(), pitchMin.dump().c_str());
			}

			if (!CHECK(pitchMax, number))
			{
				Components::Logger::Print("%s is not number but %d (%s)\n", "pitchMax", pitchMax.type(), pitchMax.dump().c_str());
			}

			if (!CHECK(probability, number))
			{
				Components::Logger::Print("%s is not number but %d (%s)\n", "probability", probability.type(), probability.dump().c_str());
			}

			if (!CHECK(lfePercentage, number))
			{
				Components::Logger::Print("%s is not number but %d (%s)\n", "lfePercentage", lfePercentage.type(), lfePercentage.dump().c_str());
			}

			if (!CHECK(centerPercentage, number))
			{
				Components::Logger::Print("%s is not number but %d (%s)\n", "centerPercentage", centerPercentage.type(), centerPercentage.dump().c_str());
			}

			if (!CHECK(startDelay, number))
			{
				Components::Logger::Print("%s is not number but %d (%s)\n", "startDelay", startDelay.type(), startDelay.dump().c_str());
			}

			if (!CHECK(volumeFalloffCurve, string))
			{
				Components::Logger::Print("%s is not string but %d (%s)\n", "volumeFalloffCurve", volumeFalloffCurve.type(), volumeFalloffCurve.dump().c_str());
			}

			if (!CHECK(envelopMin, number))
			{
				Components::Logger::Print("%s is not number but %d (%s)\n", "envelopMin", envelopMin.type(), envelopMin.dump().c_str());
			}

			if (!CHECK(envelopMax, number))
			{
				Components::Logger::Print("%s is not number but %d (%s)\n", "envelopMax", envelopMax.type(), envelopMax.dump().c_str());
			}

			if (!CHECK(envelopPercentage, number))
			{
				Components::Logger::Print("%s is not number but %d (%s)\n", "envelopPercentage", envelopPercentage.type(), envelopPercentage.dump().c_str());
			}

			if (!CHECK(speakerMap, object))
			{
				Components::Logger::Print("%s is not object but %d (%s)\n", "speakerMap", speakerMap.type(), speakerMap.dump().c_str());
			}


			if (CHECK(type, number) && CHECK(subtitle, string) && CHECK(secondaryAliasName, string) && CHECK(chainAliasName, string) &&
				CHECK(soundFile, string) && CHECK(sequence, number) && CHECK(volMin, number) && CHECK(volMax, number) && CHECK(pitchMin, number) &&
				CHECK(pitchMax, number) && CHECK(distMin, number) && CHECK(distMax, number) && CHECK(flags, number) && CHECK(slavePercentage, number) &&
				CHECK(probability, number) && CHECK(lfePercentage, number) && CHECK(centerPercentage, number) && CHECK(startDelay, number) &&
				CHECK(volumeFalloffCurve, string) && CHECK(envelopMin, number) && CHECK(envelopMax, number) && CHECK(envelopPercentage, number) &&
				CHECK(speakerMap, object))
			{

				alias->soundFile->exists = true;
				alias->aliasName = aliasList->aliasName;

				if (subtitle.is_string())
				{
					alias->subtitle = builder->getAllocator()->duplicateString(subtitle.string_value().c_str());
				}
				if (secondaryAliasName.is_string())
				{
					alias->secondaryAliasName = builder->getAllocator()->duplicateString(secondaryAliasName.string_value().c_str());
				}
				if (chainAliasName.is_string())
				{
					alias->chainAliasName = builder->getAllocator()->duplicateString(chainAliasName.string_value().c_str());
				}

				alias->sequence = sequence.int_value();
				alias->volMin = float(volMin.number_value());
				alias->volMax = float(volMax.number_value());
				alias->pitchMin = float(pitchMin.number_value());
				alias->pitchMax = float(pitchMax.number_value());
				alias->distMin = float(distMin.number_value());
				alias->distMax = float(distMax.number_value());
				alias->flags = flags.int_value();
				alias->___u15.slavePercentage = float(slavePercentage.number_value());
				alias->probability = float(probability.number_value());
				alias->lfePercentage = float(lfePercentage.number_value());
				alias->centerPercentage = float(centerPercentage.number_value());
				alias->startDelay = startDelay.int_value();
				alias->envelopMin = float(envelopMin.number_value());
				alias->envelopMax = float(envelopMax.number_value());
				alias->envelopPercentage = float(envelopPercentage.number_value());

				// Speaker map object
				if (!speakerMap.is_null())
				{
					alias->speakerMap = builder->getAllocator()->allocate<Game::SpeakerMap>();
					if (!alias->speakerMap)
					{
						Components::Logger::Print("Error allocating memory for speakermap in sound alias%s!\n", alias->aliasName);
						return;
					}

					alias->speakerMap->name = builder->getAllocator()->duplicateString(speakerMap["name"].string_value().c_str());
					alias->speakerMap->isDefault = speakerMap["isDefault"].bool_value();

					if (speakerMap["channelMaps"].is_array())
					{
						json11::Json::array channelMaps = speakerMap["channelMaps"].array_items();

						assert(channelMaps.size() <= 4);

						// channelMapIndex should never exceed 1
						for (size_t channelMapIndex = 0; channelMapIndex < 2; channelMapIndex++)
						{
							// subChannelIndex should never exceed 1
							for (size_t subChannelIndex = 0; subChannelIndex < 2; subChannelIndex++)
							{
								json11::Json channelMap = channelMaps[channelMapIndex * 2 + subChannelIndex]; // 0-3

								auto speakers = channelMap["speakers"].array_items();

								alias->speakerMap->channelMaps[channelMapIndex][subChannelIndex].speakerCount = speakers.size();

								for (size_t speakerIndex = 0; speakerIndex < alias->speakerMap->channelMaps[channelMapIndex][subChannelIndex].speakerCount; speakerIndex++)
								{
									auto speaker = speakers[speakerIndex];
									alias->speakerMap->channelMaps[channelMapIndex][subChannelIndex].speakers[speakerIndex].levels[0] = static_cast<float>(speaker["levels0"].number_value());
									alias->speakerMap->channelMaps[channelMapIndex][subChannelIndex].speakers[speakerIndex].levels[1] = static_cast<float>(speaker["levels1"].number_value());
									alias->speakerMap->channelMaps[channelMapIndex][subChannelIndex].speakers[speakerIndex].numLevels = static_cast<int>(speaker["numLevels"].number_value());
									alias->speakerMap->channelMaps[channelMapIndex][subChannelIndex].speakers[speakerIndex].speaker = static_cast<int>(speaker["speaker"].number_value());
								}
							}
						}
					}
				}

				if (volumeFalloffCurve.is_string())
				{
					std::string fallOffCurve = volumeFalloffCurve.string_value();

					if (fallOffCurve.size() == 0)
					{
						fallOffCurve = "$default";
					}

					auto curve = Components::AssetHandler::FindAssetForZone(
						Game::XAssetType::ASSET_TYPE_SOUND_CURVE, 
						fallOffCurve.c_str(),
						builder
					).sndCurve;

					alias->volumeFalloffCurve = curve;
				}

				if (static_cast<Game::snd_alias_type_t>(type.number_value()) == Game::snd_alias_type_t::SAT_LOADED) // Loaded
				{
					alias->soundFile->type = Game::SAT_LOADED;
					alias->soundFile->u.loadSnd = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_LOADED_SOUND, soundFile.string_value().c_str(), builder).loadSnd;
				}
				else if (static_cast<Game::snd_alias_type_t>(type.number_value()) == Game::snd_alias_type_t::SAT_STREAMED) // Streamed 
				{
					alias->soundFile->type = Game::SAT_STREAMED;

					std::string streamedFile = soundFile.string_value();
					std::string directory = ""s;
					int split = streamedFile.find_last_of('/');

					if (split >= 0)
					{
						directory = streamedFile.substr(0, split);
						streamedFile = streamedFile.substr(split+1);
					}

					alias->soundFile->u.streamSnd.filename.info.raw.dir = builder->getAllocator()->duplicateString(directory.c_str());
					alias->soundFile->u.streamSnd.filename.info.raw.name = builder->getAllocator()->duplicateString(streamedFile.c_str());
				}
				else
				{
					Components::Logger::Error("Failed to parse sound %s! Invalid sound type %s\n", name.c_str(), type.string_value().c_str());
					return;
				}

				aliasList->head[i] = *alias;
			}
			else
			{
				Components::Logger::Error("Failed to parse sound %s!\n", name.c_str());
				return;
			}
		}

		header->sound = aliasList;

#undef CHECK

	}

	void Isnd_alias_list_t::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::snd_alias_list_t* asset = header.sound;

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
		}
	}

	void Isnd_alias_list_t::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::snd_alias_list_t, 12);

		Utils::Stream* buffer = builder->getBuffer();
		Game::snd_alias_list_t* asset = header.sound;
		Game::snd_alias_list_t* dest = buffer->dest<Game::snd_alias_list_t>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->aliasName)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->aliasName));
			Utils::Stream::ClearPointer(&dest->aliasName);
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

				Game::snd_alias_t* destHead = buffer->dest<Game::snd_alias_t>();
				buffer->saveArray(asset->head, asset->count);

				for (unsigned int i = 0; i < asset->count; ++i)
				{
					Game::snd_alias_t* destAlias = &destHead[i];
					Game::snd_alias_t* alias = &asset->head[i];

					if (alias->aliasName)
					{
						buffer->saveString(alias->aliasName);
						Utils::Stream::ClearPointer(&destAlias->aliasName);
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

							Game::SoundFile* destSoundFile = buffer->dest<Game::SoundFile>();
							buffer->save(alias->soundFile);

							// Save_SoundFileRef
							{
								if (alias->soundFile->type == Game::snd_alias_type_t::SAT_LOADED)
								{
									destSoundFile->u.loadSnd = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_LOADED_SOUND, alias->soundFile->u.loadSnd).loadSnd;
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
						destAlias->volumeFalloffCurve = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_SOUND_CURVE, alias->volumeFalloffCurve).sndCurve;
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

							Game::SpeakerMap* destSoundFile = buffer->dest<Game::SpeakerMap>();
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
