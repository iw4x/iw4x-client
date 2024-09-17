#include <STDInclude.hpp>
#include "ILocalizeEntry.hpp"

namespace Assets
{
	void ILocalizeEntry::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		const auto path = "localizedstrings/" + name;

		Components::FileSystem::File rawFile(path);
		if (!rawFile.exists())
		{
			return;
		}

		Components::Logger::Debug("Parsing localized string \"{}\"...", path);

		auto* asset = builder->getAllocator()->allocate<Game::LocalizeEntry>();
		if (!asset)
		{
			return;
		}

		asset->name = builder->getAllocator()->duplicateString(name);
		asset->value = builder->getAllocator()->duplicateString(rawFile.getBuffer());

		header->localize = asset;
	}

	void ILocalizeEntry::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::LocalizeEntry, 8);

		auto* buffer = builder->getBuffer();
		auto* asset = header.localize;
		auto* dest = buffer->dest<Game::LocalizeEntry>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->value)
		{
			buffer->saveString(asset->value);
			Utils::Stream::ClearPointer(&dest->value);
		}

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		buffer->popBlock();
	}

	void ILocalizeEntry::ParseLocalizedStringsFile(Components::ZoneBuilder::Zone* builder, const std::string& name, const std::string& filename)
	{
		std::vector<Game::LocalizeEntry*> assets;
		const auto _0 = gsl::finally([]
		{
			Components::Localization::ParseOutput(nullptr);
			Components::Localization::PrefixOverride = {};
		});

		Components::Localization::PrefixOverride = Utils::String::ToUpper(name) + "_";
		Components::Localization::ParseOutput([&assets](Game::LocalizeEntry* asset)
		{
			assets.push_back(asset);
		});
		
		const auto* psLoadedFile = Game::SE_Load(filename.data(), false);
		if (psLoadedFile)
		{
			Game::Com_PrintError(Game::CON_CHANNEL_SYSTEM, "^1Localization ERROR: %s\n", psLoadedFile);
			return;
		}		

		auto type = Game::DB_GetXAssetNameType("localize");
		for (const auto& entry : assets)
		{
			builder->addRawAsset(type, entry);
		}
	}

	void ILocalizeEntry::ParseLocalizedStringsJSON(Components::ZoneBuilder::Zone* builder, Components::FileSystem::File& file)
	{
		nlohmann::json localize;
		try
		{
			Components::Logger::Debug("Parsing localized string \"{}\"...", file.getName());
			localize = nlohmann::json::parse(file.getBuffer());
		}
		catch (const std::exception& ex)
		{
			Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "{}\n", ex.what());
			return;
		}

		if (!localize.is_object())
		{
			Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "Localized strings json file '{}' should be an object!", file.getName());
			return;
		}

		std::vector<Game::LocalizeEntry*> assets;

		try
		{
			for (const auto& [key, value] : localize.items())
			{
				const auto valueStr = value.get<std::string>();

				auto* entry = builder->getAllocator()->allocate<Game::LocalizeEntry>();
				entry->name = builder->getAllocator()->duplicateString(key);
				entry->value = builder->getAllocator()->duplicateString(valueStr);

				assets.emplace_back(entry);
			}
		}
		catch (const std::exception& ex)
		{
			Components::Logger::PrintError(Game::CON_CHANNEL_ERROR, "{}: Localized strings json file '{}' contains invalid data!", ex.what(), file.getName());
		}

		auto type = Game::DB_GetXAssetNameType("localize");
		for (const auto& entry : assets)
		{
			builder->addRawAsset(type, entry);
		}
	}
}
