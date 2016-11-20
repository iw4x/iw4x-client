#include <STDInclude.hpp>

namespace Assets
{
	void IXModelSurfs::saveXSurfaceCollisionTree(Game::XSurfaceCollisionTree* entry, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::XSurfaceCollisionTree, 40);

		Utils::Stream* buffer = builder->getBuffer();

		Game::XSurfaceCollisionTree* destEntry = buffer->dest<Game::XSurfaceCollisionTree>();
		buffer->save(entry);

		if (entry->node)
		{
			buffer->align(Utils::Stream::ALIGN_16);
			buffer->save(entry->node, 16, entry->numNode);
			Utils::Stream::ClearPointer(&destEntry->node);
		}

		if (entry->leaf)
		{
			buffer->align(Utils::Stream::ALIGN_2);
			buffer->saveArray(entry->leaf, entry->numLeaf);
			Utils::Stream::ClearPointer(&destEntry->leaf);
		}
	}

	void IXModelSurfs::saveXSurface(Game::XSurface* surf, Game::XSurface* destSurf, Components::ZoneBuilder::Zone* builder)
	{
		Utils::Stream* buffer = builder->getBuffer();

		if (surf->blendInfo)
		{
			buffer->align(Utils::Stream::ALIGN_2);
			buffer->save(surf->blendInfo, sizeof(short), surf->blendNum1 + (surf->blendNum2 * 3) + (surf->blendNum3 * 5) + (surf->blendNum4 * 7));
			Utils::Stream::ClearPointer(&destSurf->blendInfo);
		}

		// Access vertex block
		buffer->pushBlock(Game::XFILE_BLOCK_VERTEX);
		if (surf->vertexBuffer)
		{
			AssertSize(Game::GfxPackedVertex, 32);
			
			buffer->align(Utils::Stream::ALIGN_16);
			buffer->saveArray(surf->vertexBuffer, surf->numVertices);
			Utils::Stream::ClearPointer(&destSurf->vertexBuffer);
		}
		buffer->popBlock();

		// Save_XRigidVertListArray
		if (surf->ct)
		{
			AssertSize(Game::XRigidVertList, 12);

			buffer->align(Utils::Stream::ALIGN_4);

			Game::XRigidVertList* destCt = buffer->dest<Game::XRigidVertList>();
			buffer->saveArray(surf->ct, surf->numCT);

			for (int i = 0; i < surf->numCT; ++i)
			{
				Game::XRigidVertList* destRigidVertList = &destCt[i];
				Game::XRigidVertList* rigidVertList = &surf->ct[i];

				if (rigidVertList->entry)
				{
					buffer->align(Utils::Stream::ALIGN_4);
					this->saveXSurfaceCollisionTree(rigidVertList->entry, builder);
					Utils::Stream::ClearPointer(&destRigidVertList->entry);
				}
			}

			Utils::Stream::ClearPointer(&destSurf->ct);
		}

		// Access index block
		buffer->pushBlock(Game::XFILE_BLOCK_INDEX);
		if (surf->indexBuffer)
		{
			AssertSize(Game::Face, 6);

			buffer->align(Utils::Stream::ALIGN_16);
			buffer->saveArray(surf->indexBuffer, surf->numPrimitives);
			Utils::Stream::ClearPointer(&destSurf->indexBuffer);
		}
		buffer->popBlock();
	}

	void IXModelSurfs::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::XModelSurfs, 36);

		Utils::Stream* buffer = builder->getBuffer();
		Game::XModelSurfs* asset = header.surfaces;
		Game::XModelSurfs* dest = buffer->dest<Game::XModelSurfs>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->surfaces)
		{
			AssertSize(Game::XSurface, 64);

			buffer->align(Utils::Stream::ALIGN_4);

			Game::XSurface* destSurfaces = buffer->dest<Game::XSurface>();
			buffer->saveArray(asset->surfaces, asset->numSurfaces);

			for (int i = 0; i < asset->numSurfaces; ++i)
			{
				this->saveXSurface(&asset->surfaces[i], &destSurfaces[i], builder);
			}

			Utils::Stream::ClearPointer(&dest->surfaces);
		}

		buffer->popBlock();
	}
}
