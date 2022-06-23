#include <STDInclude.hpp>
#include "ISndCurve.hpp"

namespace Assets
{
	void ISndCurve::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::SndCurve, 136);

		Utils::Stream* buffer = builder->getBuffer();
		Game::SndCurve* asset = header.sndCurve;
		Game::SndCurve* dest = buffer->dest<Game::SndCurve>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->filename)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->filename));
			Utils::Stream::ClearPointer(&dest->filename);
		}

		buffer->popBlock();
	}
}
