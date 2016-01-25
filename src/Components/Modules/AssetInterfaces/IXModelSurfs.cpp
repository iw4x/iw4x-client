#include <STDInclude.hpp>

namespace Assets
{
	void IXModelSurfs::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_Size(Game::XModelSurfs, 36);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::XModelSurfs* asset = header.surfaces;
		Game::XModelSurfs* dest = buffer->Dest<Game::XModelSurfs>();
		buffer->Save(asset, sizeof(Game::PhysPreset));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			dest->name = reinterpret_cast<char*>(-1);
		}

		if (asset->surfaces)
		{
			Assert_Size(Game::XSurface, 64);

			buffer->Align(Utils::Stream::ALIGN_4);
			buffer->SaveArray(asset->surfaces, asset->numSurfaces);

			for (int i = 0; i < asset->numSurfaces; ++i)
			{

			}

			dest->surfaces = reinterpret_cast<Game::XSurface*>(-1);
		}

		buffer->PopBlock();
	}
}
