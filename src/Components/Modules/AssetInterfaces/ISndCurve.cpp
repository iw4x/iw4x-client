#include <STDInclude.hpp>
#include "ISndCurve.hpp"

namespace Assets
{
	void ISndCurve::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::SndCurve, 136);

		auto* buffer = builder->getBuffer();
		auto* asset = header.sndCurve;
		auto* dest = buffer->dest<Game::SndCurve>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->filename)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->filename));
			Utils::Stream::ClearPointer(&dest->filename);
		}

		buffer->popBlock();
	}

	void ISndCurve::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		header->sndCurve = builder->getIW4OfApi()->read<Game::SndCurve>(Game::XAssetType::ASSET_TYPE_SOUND_CURVE, name);

		if (!header->sndCurve)
		{
			header->sndCurve = Components::AssetHandler::FindOriginalAsset(Game::XAssetType::ASSET_TYPE_SOUND_CURVE, name.data()).sndCurve;
		}
	}
}
