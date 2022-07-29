#include <STDInclude.hpp>
#include "Isnd_alias_list_t.hpp"

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
		nlohmann::json infoData = nlohmann::json::parse(aliasFile.getBuffer());
		nlohmann::json aliasesContainer = infoData["head"];

		nlohmann::json::array_t aliases = aliasesContainer;

		aliasList->count = aliases.size();

		// Allocate
		aliasList->head = builder->getAllocator()->allocateArray<Game::snd_alias_t>(aliasList->count);
		if (!aliasList->head)
		{
			Components::Logger::Print("Error allocating memory for sound alias structure!\n");
			return;
		}

		aliasList->aliasName = builder->getAllocator()->duplicateString(name.c_str());

		for (size_t i = 0; i < aliasList->count; i++)
		{
			nlohmann::json head = aliasesContainer[i];

			if (!infoData.is_object())
			{
				Components::Logger::Error(Game::ERR_FATAL, "Failed to load sound {}!", name);
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
			auto aliasName = head["aliasName"];

			// Fix casing
			if (soundFile.is_null())
			{
				soundFile = head["soundfile"];

				Components::Logger::Print("Fixed casing on {}\n", name);
			}

			if (type.is_null() || soundFile.is_null())
			{
				Components::Logger::Print("Type is {}\n", type.dump());
				Components::Logger::Print("SoundFile is {}\n", soundFile.dump());
				Components::Logger::Error(Game::ERR_FATAL, "Failed to parse sound {}! Each alias must have at least a type and a soundFile\n", name);
				return;
			}

#define CHECK(x, type) (x.is_##type##() || x.is_null())

			// TODO: actually support all of those properties
			if (!CHECK(type, number))
			{
				Components::Logger::Print("{} is not number but {} ({})\n", "type", Utils::Json::TypeToString(type.type()), type.dump());
			}

			if (!CHECK(subtitle, string))
			{
				Components::Logger::Print("{} is not string but {} ({})\n", "subtitle", Utils::Json::TypeToString(subtitle.type()), subtitle.dump());
			}

			if (!CHECK(aliasName, string))
			{
				Components::Logger::Print("{} is not string but {} ({})\n", "aliasName", Utils::Json::TypeToString(aliasName.type()), aliasName.dump());
			}

			if (!CHECK(secondaryAliasName, string))
			{
				Components::Logger::Print("{} is not string but {} ({})\n", "secondaryAliasName", Utils::Json::TypeToString(secondaryAliasName.type()), secondaryAliasName.dump());
			}

			if (!CHECK(chainAliasName, string))
			{
				Components::Logger::Print("{} is not string but {} ({})\n", "chainAliasName", Utils::Json::TypeToString(chainAliasName.type()), chainAliasName.dump());
			}

			if (!CHECK(soundFile, string))
			{
				Components::Logger::Print("{} is not string but {} ({})\n", "soundFile", Utils::Json::TypeToString(soundFile.type()), soundFile.dump());
			}

			if (!CHECK(sequence, number))
			{
				Components::Logger::Print("{} is not number but {} ({})\n", "sequence", Utils::Json::TypeToString(sequence.type()), sequence.dump());
			}

			if (!CHECK(volMin, number))
			{
				Components::Logger::Print("{} is not number but {} ({})\n", "volMin", Utils::Json::TypeToString(volMin.type()), volMin.dump());
			}

			if (!CHECK(volMax, number))
			{
				Components::Logger::Print("{} is not number but {} ({})\n", "volMax", Utils::Json::TypeToString(volMax.type()), volMax.dump());
			}

			if (!CHECK(pitchMin, number))
			{
				Components::Logger::Print("{} is not number but {} ({})\n", "pitchMin", Utils::Json::TypeToString(pitchMin.type()), pitchMin.dump());
			}

			if (!CHECK(pitchMax, number))
			{
				Components::Logger::Print("{} is not number but {} ()\n", "pitchMax", Utils::Json::TypeToString(pitchMax.type()), pitchMax.dump());
			}

			if (!CHECK(probability, number))
			{
				Components::Logger::Print("{} is not number but {} ({}))\n", "probability", Utils::Json::TypeToString(probability.type()), probability.dump());
			}

			if (!CHECK(lfePercentage, number))
			{
				Components::Logger::Print("{} is not number but {} ({})\n", "lfePercentage", Utils::Json::TypeToString(lfePercentage.type()), lfePercentage.dump());
			}

			if (!CHECK(centerPercentage, number))
			{
				Components::Logger::Print("{} is not number but {} ({})\n", "centerPercentage", Utils::Json::TypeToString(centerPercentage.type()), centerPercentage.dump());
			}

			if (!CHECK(startDelay, number))
			{
				Components::Logger::Print("{} is not number but {} ({})\n", "startDelay", Utils::Json::TypeToString(startDelay.type()), startDelay.dump());
			}

			if (!CHECK(volumeFalloffCurve, string))
			{
				Components::Logger::Print("{}s is not string but {} ({})\n", "volumeFalloffCurve", Utils::Json::TypeToString(volumeFalloffCurve.type()), volumeFalloffCurve.dump());
			}

			if (!CHECK(envelopMin, number))
			{
				Components::Logger::Print("{} is not number but {} ({})\n", "envelopMin", Utils::Json::TypeToString(envelopMin.type()), envelopMin.dump());
			}

			if (!CHECK(envelopMax, number))
			{
				Components::Logger::Print("{} is not number but {} ({})\n", "envelopMax", Utils::Json::TypeToString(envelopMax.type()), envelopMax.dump());
			}

			if (!CHECK(envelopPercentage, number))
			{
				Components::Logger::Print("{} is not number but {} ({})\n", "envelopPercentage", Utils::Json::TypeToString(envelopPercentage.type()), envelopPercentage.dump());
			}

			if (!CHECK(speakerMap, object))
			{
				Components::Logger::Print("{} is not object but {} ({})\n", "speakerMap", Utils::Json::TypeToString(speakerMap.type()), speakerMap.dump());
			}


			if (CHECK(type, number) && CHECK(aliasName, string) && CHECK(subtitle, string) && CHECK(secondaryAliasName, string) && CHECK(chainAliasName, string) &&
				CHECK(soundFile, string) && CHECK(sequence, number) && CHECK(volMin, number) && CHECK(volMax, number) && CHECK(pitchMin, number) &&
				CHECK(pitchMax, number) && CHECK(distMin, number) && CHECK(distMax, number) && CHECK(flags, number) && CHECK(slavePercentage, number) &&
				CHECK(probability, number) && CHECK(lfePercentage, number) && CHECK(centerPercentage, number) && CHECK(startDelay, number) &&
				CHECK(volumeFalloffCurve, string) && CHECK(envelopMin, number) && CHECK(envelopMax, number) && CHECK(envelopPercentage, number) &&
				CHECK(speakerMap, object))
			{

				alias->soundFile->exists = true;
				alias->aliasName = builder->getAllocator()->duplicateString(aliasName.get<std::string>());

				if (subtitle.is_string())
				{
					alias->subtitle = builder->getAllocator()->duplicateString(subtitle.get<std::string>());
				}
				if (secondaryAliasName.is_string())
				{
					alias->secondaryAliasName = builder->getAllocator()->duplicateString(secondaryAliasName.get<std::string>());
				}
				if (chainAliasName.is_string())
				{
					alias->chainAliasName = builder->getAllocator()->duplicateString(chainAliasName.get<std::string>());
				}

				alias->sequence = sequence.get<int>();
				alias->volMin = volMin.get<float>();
				alias->volMax = volMax.get<float>();
				alias->pitchMin = pitchMin.get<float>();
				alias->pitchMax = pitchMax.get<float>();
				alias->distMin = distMin.get<float>();
				alias->distMax = distMax.get<float>();
				alias->flags = flags.get<int>();
				alias->___u15.slavePercentage = slavePercentage.get<float>();
				alias->probability = probability.get<float>();
				alias->lfePercentage = lfePercentage.get<float>();
				alias->centerPercentage = centerPercentage.get<float>();
				alias->startDelay = startDelay.get<int>();
				alias->envelopMin = envelopMin.get<float>();
				alias->envelopMax = envelopMax.get<float>();
				alias->envelopPercentage = envelopPercentage.get<float>();

				// Speaker map object
				if (!speakerMap.is_null())
				{
					alias->speakerMap = builder->getAllocator()->allocate<Game::SpeakerMap>();
					if (!alias->speakerMap)
					{
						Components::Logger::Print("Error allocating memory for speakermap in sound alias{}!\n", alias->aliasName);
						return;
					}

					alias->speakerMap->name = builder->getAllocator()->duplicateString(speakerMap["name"].get<std::string>());
					alias->speakerMap->isDefault = speakerMap["isDefault"].get<bool>();

					if (speakerMap["channelMaps"].is_array())
					{
						nlohmann::json::array_t channelMaps = speakerMap["channelMaps"];

						assert(channelMaps.size() <= 4);

						// channelMapIndex should never exceed 1
						for (size_t channelMapIndex = 0; channelMapIndex < 2; channelMapIndex++)
						{
							// subChannelIndex should never exceed 1
							for (size_t subChannelIndex = 0; subChannelIndex < 2; subChannelIndex++)
							{
								nlohmann::json channelMap = channelMaps[channelMapIndex * 2 + subChannelIndex]; // 0-3

								nlohmann::json::array_t  speakers = channelMap["speakers"];

								alias->speakerMap->channelMaps[channelMapIndex][subChannelIndex].speakerCount = speakers.size();

								for (size_t speakerIndex = 0; speakerIndex < alias->speakerMap->channelMaps[channelMapIndex][subChannelIndex].speakerCount; speakerIndex++)
								{
									auto speaker = speakers[speakerIndex];
									alias->speakerMap->channelMaps[channelMapIndex][subChannelIndex].speakers[speakerIndex].levels[0] = speaker["levels0"].get<float>();
									alias->speakerMap->channelMaps[channelMapIndex][subChannelIndex].speakers[speakerIndex].levels[1] = speaker["levels1"].get<float>();
									alias->speakerMap->channelMaps[channelMapIndex][subChannelIndex].speakers[speakerIndex].numLevels = speaker["numLevels"].get<int>();
									alias->speakerMap->channelMaps[channelMapIndex][subChannelIndex].speakers[speakerIndex].speaker = speaker["speaker"].get<int>();
								}
							}
						}
					}
				}

				if (volumeFalloffCurve.is_string())
				{
					std::string fallOffCurve = volumeFalloffCurve.get<std::string>();

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

				if (static_cast<Game::snd_alias_type_t>(type.get<int>()) == Game::snd_alias_type_t::SAT_LOADED) // Loaded
				{
					alias->soundFile->type = Game::SAT_LOADED;
					alias->soundFile->u.loadSnd = Components::AssetHandler::FindAssetForZone(Game::XAssetType::ASSET_TYPE_LOADED_SOUND, soundFile.get<std::string>(), builder).loadSnd;
				}
				else if (static_cast<Game::snd_alias_type_t>(type.get<int>()) == Game::snd_alias_type_t::SAT_STREAMED) // Streamed 
				{
					alias->soundFile->type = Game::SAT_STREAMED;

					std::string streamedFile = soundFile.get<std::string>();
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
					Components::Logger::Error(Game::ERR_FATAL, "Failed to parse sound {}! Invalid sound type {}\n", name, type.get<std::string>());
					return;
				}

				aliasList->head[i] = *alias;
			}
			else
			{
				Components::Logger::Error(Game::ERR_FATAL, "Failed to parse sound {}!\n", name);
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
