#include <STDInclude.hpp>

namespace Assets
{
	void ILocalizedEntry::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_Size(Game::LocalizedEntry, 8);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::LocalizedEntry* asset = header.localize;
		Game::LocalizedEntry* dest = buffer->Dest<Game::LocalizedEntry>();
		buffer->Save(asset, sizeof(Game::LocalizedEntry));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->value)
		{
			buffer->SaveString(asset->value);
			Utils::Stream::ClearPointer(&dest->value);
		}

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		buffer->PopBlock();
	}
}
