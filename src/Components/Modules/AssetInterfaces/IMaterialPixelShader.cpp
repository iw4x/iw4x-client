#include "STDInclude.hpp"

namespace Assets
{
	void IMaterialPixelShader::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::MaterialPixelShader, 16);

		Utils::Stream* buffer = builder->getBuffer();
		Game::MaterialPixelShader* asset = header.pixelShader;
		Game::MaterialPixelShader* dest = buffer->dest<Game::MaterialPixelShader>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->loadDef.physicalPart)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->save(asset->loadDef.physicalPart, 4, asset->loadDef.cachedPartSize & 0xFFFF);
			Utils::Stream::ClearPointer(&dest->loadDef.physicalPart);
		}

		buffer->popBlock();
	}
}
