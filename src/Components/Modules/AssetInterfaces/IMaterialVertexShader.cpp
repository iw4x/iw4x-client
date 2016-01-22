#include <STDInclude.hpp>

namespace Assets
{
	void IMaterialVertexShader::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_AssetStruct(Game::MaterialVertexShader, 16);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::MaterialVertexShader* asset = header.vertexShader;
		Game::MaterialVertexShader* dest = (Game::MaterialVertexShader*)buffer->At();
		buffer->Save(asset, sizeof(Game::MaterialVertexShader));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			dest->name = (char *)-1;
		}

		if (asset->loadDef.physicalPart)
		{
			buffer->Align(Utils::Stream::ALIGN_4);
			buffer->Save(asset->loadDef.physicalPart, 4, asset->loadDef.cachedPartSize & 0xFFFF);
			dest->loadDef.physicalPart = (char*)-1;
		}

		buffer->PopBlock();
	}
}
