#include <STDInclude.hpp>
#include "IMaterialVertexDeclaration.hpp"

#define IW4X_TECHSET_VERSION 1

namespace Assets
{
	void IMaterialVertexDeclaration::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		if (!header->data) this->loadBinary(header, name, builder); // Check if we need to import a new one into the game
		if (!header->data) this->loadNative(header, name, builder); // Check if there is a native one
	}

	void IMaterialVertexDeclaration::loadNative(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* /*builder*/)
	{
		header->vertexDecl = Components::AssetHandler::FindOriginalAsset(this->getType(), name.data()).vertexDecl;
	}

	void IMaterialVertexDeclaration::loadBinary(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File declFile(std::format("decl/{}.iw4xDECL", name));
		if (!declFile.exists()) return;

		Utils::Stream::Reader reader(builder->getAllocator(), declFile.getBuffer());

		char* magic = reader.readArray<char>(8);
		if (std::memcmp(magic, "IW4xDECL", 8))
		{
			Components::Logger::Error(Game::ERR_FATAL, "Reading vertex declaration '{}' failed, header is invalid!", name);
		}

		auto version = reader.read<char>();
		if (version != IW4X_TECHSET_VERSION)
		{
			Components::Logger::Error(Game::ERR_FATAL, "Reading vertex declaration '{}' failed, expected version is {}, but it was {:d}!",
				name, IW4X_TECHSET_VERSION, version);
		}

		Game::MaterialVertexDeclaration* asset = reader.readObject<Game::MaterialVertexDeclaration>();

		if (asset->name)
		{
			asset->name = reader.readCString();
		}

		header->vertexDecl = asset;
	}

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

		AssertSize(Game::MaterialVertexStreamRouting, 92);

		buffer->popBlock();
	}
}
