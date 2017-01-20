#include "STDInclude.hpp"

namespace Assets
{
	void IMaterialVertexShader::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::MaterialVertexShader, 16);

		Utils::Stream* buffer = builder->getBuffer();
		Game::MaterialVertexShader* asset = header.vertexShader;
		Game::MaterialVertexShader* dest = buffer->dest<Game::MaterialVertexShader>();
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
