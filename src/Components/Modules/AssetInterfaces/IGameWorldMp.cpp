#include <STDInclude.hpp>
#include "IGameWorldMp.hpp"

#define IW4X_GAMEWORLD_VERSION 1

namespace Assets
{
	void IGameWorldMp::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::GameWorldMp, 8);

		Utils::Stream* buffer = builder->getBuffer();
		Game::GameWorldMp* asset = header.gameWorldMp;
		Game::GameWorldMp* dest = buffer->dest<Game::GameWorldMp>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->g_glassData)
		{
			buffer->align(Utils::Stream::ALIGN_4);

			// Save_G_GlassData
			{
				AssertSize(Game::G_GlassData, 128);

				Game::G_GlassData* destGlass = buffer->dest<Game::G_GlassData>();
				buffer->save(asset->g_glassData);

				if (asset->g_glassData->glassPieces)
				{
					AssertSize(Game::G_GlassPiece, 12);
					buffer->align(Utils::Stream::ALIGN_4);
					buffer->saveArray(asset->g_glassData->glassPieces, asset->g_glassData->pieceCount);
					Utils::Stream::ClearPointer(&destGlass->glassPieces);
				}

				if (asset->g_glassData->glassNames)
				{
					AssertSize(Game::G_GlassName, 12);
					buffer->align(Utils::Stream::ALIGN_4);

					Game::G_GlassName* destGlassNames = buffer->dest<Game::G_GlassName>();
					buffer->saveArray(asset->g_glassData->glassNames, asset->g_glassData->glassNameCount);

					for (unsigned int i = 0; i < asset->g_glassData->glassNameCount; ++i)
					{
						Game::G_GlassName* destGlassName = &destGlassNames[i];
						Game::G_GlassName* glassName = &asset->g_glassData->glassNames[i];

						if (glassName->nameStr)
						{
							buffer->saveString(glassName->nameStr);
							Utils::Stream::ClearPointer(&destGlassName->nameStr);
						}

						if (glassName->pieceIndices)
						{
							buffer->align(Utils::Stream::ALIGN_2);
							buffer->saveArray(glassName->pieceIndices, glassName->pieceCount);
							Utils::Stream::ClearPointer(&destGlassName->pieceIndices);
						}
					}

					Utils::Stream::ClearPointer(&destGlass->glassNames);
				}
			}

			Utils::Stream::ClearPointer(&dest->g_glassData);
		}

		buffer->popBlock();
	}

	void IGameWorldMp::load(Game::XAssetHeader* header, const std::string& _name, Components::ZoneBuilder::Zone* builder)
	{
		std::string name = _name;
		Utils::String::Replace(name, "maps/mp/", "");
		Utils::String::Replace(name, ".d3dbsp", "");

		Components::FileSystem::File gameWorld(std::format("gameworld/{}.iw4x.json", name));

		if (gameWorld.exists())
		{
			nlohmann::json gameWorldJson;
			
			try
			{

				gameWorldJson = nlohmann::json::parse(gameWorld.getBuffer());
			}
			catch (const std::exception& e)
			{
				Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Invalid JSON for gameworld {}! {}", name, e.what());
				return;
			}

			auto* asset = builder->getAllocator()->allocate<Game::GameWorldMp>();

			if (!gameWorldJson.is_object())
			{
				Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Invalid GameWorldMp json for {}\n", name);
				return;
			}

			auto version = gameWorldJson["version"].is_number() ? gameWorldJson["version"].get<int>() : 0;
			if (version != IW4X_GAMEWORLD_VERSION)
			{
				Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Invalid GameWorld json version for {}, expected {} and got {}\n", name, IW4X_GAMEWORLD_VERSION, version);
				return;
			}

			if (!gameWorldJson["name"].is_string())
			{
				Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Missing gameworld name! on {}\n", name);
				return;
			}

			asset->name = builder->getAllocator()->duplicateString(gameWorldJson["name"].get<std::string>());
			auto glassData = builder->getAllocator()->allocate<Game::G_GlassData>();

			if (gameWorldJson["glassData"].is_object())
			{
				auto jsonGlassData = gameWorldJson["glassData"];

				try
				{
					glassData->damageToDestroy = jsonGlassData["damageToDestroy"].get<unsigned short>();
					glassData->damageToWeaken = jsonGlassData["damageToWeaken"].get<unsigned short>();

					if (jsonGlassData["glassNames"].is_array())
					{
						nlohmann::json::array_t glassNames = jsonGlassData["glassNames"];
						glassData->glassNameCount = glassNames.size();
						glassData->glassNames = builder->getAllocator()->allocateArray<Game::G_GlassName>(glassData->glassNameCount);

						for (size_t i = 0; i < glassData->glassNameCount; i++)
						{
							auto jsonGlassName = glassNames[i];
							glassData->glassNames[i].nameStr = builder->getAllocator()->duplicateString(jsonGlassName["nameStr"]);

							glassData->glassNames[i].name = jsonGlassName["name"].get<unsigned short>();

							if (jsonGlassName["piecesIndices"].is_array())
							{
								nlohmann::json::array_t jsonPiecesIndices = jsonGlassName["piecesIndices"];
								glassData->glassNames[i].pieceCount = static_cast<unsigned short>(jsonPiecesIndices.size());

								for (size_t j = 0; j < glassData->glassNames[i].pieceCount; j++)
								{
									glassData->glassNames[i].pieceIndices[j] = jsonPiecesIndices[j].get<unsigned short>();
								}
							}
						}
					}

					if (gameWorldJson["glassPieces"].is_array())
					{
						nlohmann::json::array_t glassPieces = gameWorldJson["glassPieces"];
						glassData->pieceCount = glassPieces.size();
						glassData->glassPieces = builder->getAllocator()->allocateArray<Game::G_GlassPiece>(glassData->pieceCount);

						for (size_t i = 0; i < glassData->pieceCount; i++)
						{
							glassData->glassPieces[i].collapseTime = glassPieces[i]["collapseTime"].get<unsigned short>();
							glassData->glassPieces[i].damageTaken = glassPieces[i]["damageTaken"].get<unsigned short>();
							glassData->glassPieces[i].lastStateChangeTime = glassPieces[i]["lastStateChangeTime"].get<int>();
							glassData->glassPieces[i].impactDir = glassPieces[i]["impactDir"].get<char>();

							nlohmann::json::array_t jsonPos = glassPieces[i]["impactPos"];
							glassData->glassPieces[i].impactPos[0] = jsonPos[0].get<char>();
							glassData->glassPieces[i].impactPos[1] = jsonPos[1].get<char>();
						}
					}
				}
				catch (const nlohmann::json::exception& e)
				{
					Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Malformed GameWorldMp json for {} ({})\n", name, e.what());
					return;
				}
			}

			asset->g_glassData = glassData;

			header->gameWorldMp = asset;
		}
	}
}
