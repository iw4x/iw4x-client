#include "STDInclude.hpp"

namespace Assets
{
	void IFont_s::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::Font_s *asset = header.font;

		if (asset->image)
		{
			builder->loadAsset(Game::ASSET_TYPE_MATERIAL, asset->image);
		}

		if (asset->glowImage)
		{
			builder->loadAsset(Game::ASSET_TYPE_MATERIAL, asset->glowImage);
		}
	}

	void IFont_s::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::Font_s, 24);
		AssertSize(Game::FontEntry, 24);

		Utils::Stream* buffer = builder->getBuffer();
		Game::Font_s* asset = header.font;
		Game::Font_s* dest = buffer->dest<Game::Font_s>();

		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(asset->name);
			Utils::Stream::ClearPointer(&dest->name);
		}

		dest->image = builder->saveSubAsset(Game::ASSET_TYPE_MATERIAL, asset->image).material;
		dest->glowImage = builder->saveSubAsset(Game::ASSET_TYPE_MATERIAL, asset->glowImage).material;

		if (asset->characters)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->characters, asset->entries);
			Utils::Stream::ClearPointer(&dest->characters);
		}

		buffer->popBlock();
	}
}
