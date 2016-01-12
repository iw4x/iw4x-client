#include <STDInclude.hpp>

namespace Assets
{
	void ILocalizedEntry::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Utils::Stream* buffer = builder->GetBuffer();
		Game::LocalizedEntry* asset = header.localize;
		Game::LocalizedEntry* dest = (Game::LocalizedEntry*)buffer->At();
		buffer->Save(asset, sizeof(Game::LocalizedEntry));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->value)
		{
			buffer->SaveString(asset->value);
			dest->value = (char *)-1;
		}

		if (asset->name)
		{
			buffer->SaveString(asset->name);
			dest->name = (char *)-1;
		}

		buffer->PopBlock();
	}
}
