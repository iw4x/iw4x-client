#include "STDInclude.hpp"

namespace Assets
{
	void IFont_s::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::Font_s *asset = header.font;

		if (asset->material)
		{
			builder->loadAsset(Game::ASSET_TYPE_MATERIAL, asset->material);
		}

		if (asset->glowMaterial)
		{
			builder->loadAsset(Game::ASSET_TYPE_MATERIAL, asset->glowMaterial);
		}
	}

	void IFont_s::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::Font_s, 24);
		AssertSize(Game::Glyph, 24);

		Utils::Stream* buffer = builder->getBuffer();
		Game::Font_s* asset = header.font;
		Game::Font_s* dest = buffer->dest<Game::Font_s>();

		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->fontName)
		{
			buffer->saveString(asset->fontName);
			Utils::Stream::ClearPointer(&dest->fontName);
		}

		dest->material = builder->saveSubAsset(Game::ASSET_TYPE_MATERIAL, asset->material).material;
		dest->glowMaterial = builder->saveSubAsset(Game::ASSET_TYPE_MATERIAL, asset->glowMaterial).material;

		if (asset->glyphs)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->glyphs, asset->glyphCount);
			Utils::Stream::ClearPointer(&dest->glyphs);
		}

		buffer->popBlock();
	}
}
