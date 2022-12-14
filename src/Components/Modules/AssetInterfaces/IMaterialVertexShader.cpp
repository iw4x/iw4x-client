#include <STDInclude.hpp>
#include "IMaterialVertexShader.hpp"

#define GFX_RENDERER_SHADER_SM3 0

namespace Assets
{
	void IMaterialVertexShader::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		if (!header->data) this->loadBinary(header, name, builder); // Check if we need to import a new one into the game
		if (!header->data) this->loadNative(header, name, builder); // Check if there is a native one
	}

	void IMaterialVertexShader::loadNative(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* /*builder*/)
	{
		header->vertexShader = Components::AssetHandler::FindOriginalAsset(this->getType(), name.data()).vertexShader;
	}

	void IMaterialVertexShader::loadBinary(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File vsFile(std::format("vs/{}.cso", name));
		if (!vsFile.exists()) return;

		auto buff = vsFile.getBuffer();
		auto programSize = buff.size() / 4;
		Game::MaterialVertexShader* asset = builder->getAllocator()->allocate<Game::MaterialVertexShader>();

		asset->name = builder->getAllocator()->duplicateString(name);
		asset->prog.loadDef.loadForRenderer = GFX_RENDERER_SHADER_SM3;
		asset->prog.loadDef.programSize = static_cast<unsigned short>(programSize);
		asset->prog.loadDef.program = builder->getAllocator()->allocateArray<unsigned int>(programSize);
		memcpy_s(asset->prog.loadDef.program, buff.size(), buff.data(), buff.size());

		header->vertexShader = asset;
	}

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
