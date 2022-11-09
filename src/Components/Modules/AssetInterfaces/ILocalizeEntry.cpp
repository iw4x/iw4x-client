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
}
