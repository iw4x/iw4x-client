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

		if (asset->prog.loadDef.program)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->prog.loadDef.program, asset->prog.loadDef.programSize);
			Utils::Stream::ClearPointer(&dest->prog.loadDef.program);
		}

		buffer->popBlock();
	}
}
