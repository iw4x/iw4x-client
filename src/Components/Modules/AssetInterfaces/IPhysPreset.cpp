#include <STDInclude.hpp>

namespace Assets
{
	void IPhysPreset::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_Size(Game::PhysPreset, 44);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::PhysPreset* asset = header.physPreset;
		Game::PhysPreset* dest = buffer->Dest<Game::PhysPreset>();
		buffer->Save(asset, sizeof(Game::PhysPreset));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		// TODO: I think we have to write them, even if they are NULL

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->sndAliasPrefix)
		{
			buffer->SaveString(asset->sndAliasPrefix);
			Utils::Stream::ClearPointer(&dest->sndAliasPrefix);
		}

		buffer->PopBlock();
	}
}
