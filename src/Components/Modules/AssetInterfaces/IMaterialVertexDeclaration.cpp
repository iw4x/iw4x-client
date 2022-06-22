#include <STDInclude.hpp>
#include "IMaterialVertexDeclaration.hpp"

#define IW4X_TECHSET_VERSION "0"

namespace Assets
{
	void IMaterialVertexDeclaration::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		if (!header->data) this->loadNative(header, name, builder); // Check if there is a native one
		if (!header->data) this->loadBinary(header, name, builder); // Check if we need to import a new one into the game
	}

	void IMaterialVertexDeclaration::loadNative(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* /*builder*/)
	{
		header->vertexDecl = Components::AssetHandler::FindOriginalAsset(this->getType(), name.data()).vertexDecl;
	}

	void IMaterialVertexDeclaration::loadBinary(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder)
	{
		Components::FileSystem::File declFile(Utils::String::VA("decl/%s.iw4xDECL", name.data()));
		if (!declFile.exists()) return;

		Utils::Stream::Reader reader(builder->getAllocator(), declFile.getBuffer());

		char* magic = reader.readArray<char>(8);
		if (std::memcmp(magic, "IW4xDECL", 8))
		{
			Components::Logger::Error(Game::ERR_FATAL, "Reading vertex declaration '{}' failed, header is invalid!", name);
		}

		std::string version;
		version.push_back(reader.read<char>());
		if (version != IW4X_TECHSET_VERSION)
		{
			Components::Logger::Error(Game::ERR_FATAL, "Reading vertex declaration '{}' failed, expected version is {}, but it was {}!",
				name, IW4X_TECHSET_VERSION, version.data());
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
