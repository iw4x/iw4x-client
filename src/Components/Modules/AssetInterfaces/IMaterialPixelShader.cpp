#include <STDInclude.hpp>
#include "IMaterialPixelShader.hpp"

namespace Assets
{

	void IMaterialPixelShader::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		if (!header->data) this->loadBinary(header, name, builder); // Check if we need to import a new one into the game
		if (!header->data) this->loadNative(header, name, builder); // Check if there is a native one
	}

	void IMaterialPixelShader::loadNative(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* /*builder*/)
	{
		header->pixelShader = Components::AssetHandler::FindOriginalAsset(this->getType(), name.data()).pixelShader;
	}

	void IMaterialPixelShader::loadBinary(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		header->pixelShader = builder->getIW4OfApi()->read<Game::MaterialPixelShader>(Game::XAssetType::ASSET_TYPE_PIXELSHADER, name);
	}

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

		if (asset->prog.loadDef.program)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			buffer->saveArray(asset->prog.loadDef.program, asset->prog.loadDef.programSize);
			Utils::Stream::ClearPointer(&dest->prog.loadDef.program);
		}

		buffer->popBlock();
	}
}
