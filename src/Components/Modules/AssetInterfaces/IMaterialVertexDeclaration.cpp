#include <STDInclude.hpp>

namespace Assets
{
	void IMaterialVertexDeclaration::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_AssetStruct(Game::MaterialVertexDeclaration, 100);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::MaterialVertexDeclaration* asset = header.vertexDecl;
		Game::MaterialVertexDeclaration* dest = (Game::MaterialVertexDeclaration*)buffer->At();
		buffer->Save(asset, sizeof(Game::MaterialVertexDeclaration));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			dest->name = (char *)-1;
		}

		buffer->PopBlock();
	}
}
