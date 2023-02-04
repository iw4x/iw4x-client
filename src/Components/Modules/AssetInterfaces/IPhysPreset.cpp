#include <STDInclude.hpp>
#include <json.hpp>

#include "IPhysPreset.hpp"

namespace Assets
{
	void IPhysPreset::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::PhysPreset, 44);

		auto* buffer = builder->getBuffer();
		auto* asset = header.physPreset;
		auto* dest = buffer->dest<Game::PhysPreset>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->sndAliasPrefix)
		{
			buffer->saveString(asset->sndAliasPrefix);
			Utils::Stream::ClearPointer(&dest->sndAliasPrefix);
		}

		buffer->popBlock();
	}

	void IPhysPreset::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		loadFromDisk(header, name, builder);
	}

	void IPhysPreset::loadFromDisk(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File physPresetFile(std::format("physpreset/{}.iw4x.json", name));
		auto* asset = builder->getAllocator()->allocate<Game::PhysPreset>();

		if (physPresetFile.exists())
		{
			nlohmann::json physPresetJson;
			try
			{
				physPresetJson = nlohmann::json::parse(physPresetFile.getBuffer());
			}
			catch (const std::exception& e)
			{
				Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Invalid JSON for physpreset {}! {}", name, e.what());
				return;
			}

			try
			{
				asset->name = builder->getAllocator()->duplicateString(physPresetJson["name"].get<std::string>());
				asset->type = physPresetJson["type"].get<int>();
				asset->bounce = physPresetJson["bounce"].get<float>();
				asset->mass = physPresetJson["mass"].get<float>();
				asset->friction = physPresetJson["friction"].get<float>();
				asset->bulletForceScale = physPresetJson["bulletForceScale"].get<float>();
				asset->explosiveForceScale = physPresetJson["explosiveForceScale"].get<float>();
				asset->sndAliasPrefix = builder->getAllocator()->duplicateString(physPresetJson["sndAliasPrefix"].get<std::string>());
				asset->piecesSpreadFraction = physPresetJson["piecesSpreadFraction"].get<float>();
				asset->piecesUpwardVelocity = physPresetJson["piecesUpwardVelocity"].get<float>();
				asset->tempDefaultToCylinder = physPresetJson["tempDefaultToCylinder"].get<bool>();
				asset->perSurfaceSndAlias = physPresetJson["perSurfaceSndAlias"].get<bool>();

				assert(asset->mass > std::numeric_limits<float>::epsilon());
			}
			catch (const std::exception& e)
			{
				Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Malformed JSON for physpreset {}! {}", name, e.what());
				return;
			}
		}

		header->physPreset = asset;
	}
}
