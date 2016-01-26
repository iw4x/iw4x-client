#include <STDInclude.hpp>

namespace Assets
{
	void IXModelSurfs::Save_XSurfaceCollisionTree(Game::XSurfaceCollisionTree* entry, Components::ZoneBuilder::Zone* builder)
	{
		Assert_Size(Game::XSurfaceCollisionTree, 40);

		Utils::Stream* buffer = builder->GetBuffer();

		Game::XSurfaceCollisionTree* destEntry = buffer->Dest<Game::XSurfaceCollisionTree>();
		buffer->Save(entry, sizeof(Game::XSurfaceCollisionTree));

		if (entry->node)
		{
			buffer->Align(Utils::Stream::ALIGN_16);
			buffer->Save(entry->node, 16, entry->numNode);
			destEntry->node = reinterpret_cast<char*>(-1);
		}

		if (entry->leaf)
		{
			buffer->Align(Utils::Stream::ALIGN_2);
			buffer->SaveArray(entry->leaf, entry->numLeaf);
			destEntry->leaf = reinterpret_cast<short*>(-1);
		}
	}

	void IXModelSurfs::Save_XSurface(Game::XSurface* surf, Game::XSurface* destSurf, Components::ZoneBuilder::Zone* builder)
	{
		Utils::Stream* buffer = builder->GetBuffer();

		if (surf->blendInfo)
		{
			buffer->Align(Utils::Stream::ALIGN_2);

			buffer->Save(surf->blendInfo, sizeof(short), surf->blendNum1 + (surf->blendNum2 * 3) + (surf->blendNum3 * 5) + (surf->blendNum4 * 7));
			destSurf->blendInfo = reinterpret_cast<char*>(-1);
		}

		// Access vertex block
		buffer->PushBlock(Game::XFILE_BLOCK_VERTEX);
		if (surf->vertexBuffer)
		{
			Assert_Size(Game::GfxPackedVertex, 32);
			
			buffer->Align(Utils::Stream::ALIGN_16);
			buffer->SaveArray(surf->vertexBuffer, surf->numVertices);
			destSurf->vertexBuffer = reinterpret_cast<Game::GfxPackedVertex*>(-1);
		}
		buffer->PopBlock();

		// Save_XRigidVertListArray
		if (surf->ct)
		{
			Assert_Size(Game::XRigidVertList, 12);

			buffer->Align(Utils::Stream::ALIGN_4);

			Game::XRigidVertList* destCt = buffer->Dest<Game::XRigidVertList>();
			buffer->SaveArray(surf->ct, surf->numCT);

			for (int i = 0; i < surf->numCT; ++i)
			{
				Game::XRigidVertList* destRigidVertList = &destCt[i];
				Game::XRigidVertList* rigidVertList = &surf->ct[i];

				if (rigidVertList->entry)
				{
					buffer->Align(Utils::Stream::ALIGN_4);
					IXModelSurfs::Save_XSurfaceCollisionTree(rigidVertList->entry, builder);
					destRigidVertList->entry = reinterpret_cast<Game::XSurfaceCollisionTree*>(-1);
				}
			}

			destSurf->ct = reinterpret_cast<Game::XRigidVertList*>(-1);
		}

		// Access index block
		buffer->PushBlock(Game::XFILE_BLOCK_INDEX);
		if (surf->indexBuffer)
		{
			Assert_Size(Game::Face, 6);

			buffer->Align(Utils::Stream::ALIGN_16);
			buffer->SaveArray(surf->indexBuffer, surf->numPrimitives);
			destSurf->indexBuffer = reinterpret_cast<Game::Face*>(-1);
		}
		buffer->PopBlock();
	}

	void IXModelSurfs::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_Size(Game::XModelSurfs, 36);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::XModelSurfs* asset = header.surfaces;
		Game::XModelSurfs* dest = buffer->Dest<Game::XModelSurfs>();
		buffer->Save(asset, sizeof(Game::XModelSurfs));

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

			Game::XSurface* destSurfaces = buffer->Dest<Game::XSurface>();
			buffer->SaveArray(asset->surfaces, asset->numSurfaces);

			for (int i = 0; i < asset->numSurfaces; ++i)
			{
				IXModelSurfs::Save_XSurface(&asset->surfaces[i], &destSurfaces[i], builder);
			}

			dest->surfaces = reinterpret_cast<Game::XSurface*>(-1);
		}

		buffer->PopBlock();
	}
}
