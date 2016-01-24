#include <STDInclude.hpp>

namespace Assets
{
	void IMaterialPixelShader::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_Size(Game::MaterialPixelShader, 16);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::MaterialPixelShader* asset = header.pixelShader;
		Game::MaterialPixelShader* dest = buffer->Dest<Game::MaterialPixelShader>();
		buffer->Save(asset, sizeof(Game::MaterialPixelShader));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			dest->name = reinterpret_cast<char*>(-1);
		}

		if (asset->loadDef.physicalPart)
		{
			buffer->Align(Utils::Stream::ALIGN_4);
			buffer->Save(asset->loadDef.physicalPart, 4, asset->loadDef.cachedPartSize & 0xFFFF);
			dest->loadDef.physicalPart = reinterpret_cast<char*>(-1);
		}

		buffer->PopBlock();
	}
}
