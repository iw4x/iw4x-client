#include <STDInclude.hpp>

namespace Assets
{
	void IMaterialVertexDeclaration::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::MaterialVertexDeclaration, 100);

		Utils::Stream* buffer = builder->getBuffer();
		Game::MaterialVertexDeclaration* asset = header.vertexDecl;
		Game::MaterialVertexDeclaration* dest = buffer->dest<Game::MaterialVertexDeclaration>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		buffer->popBlock();
	}
}
