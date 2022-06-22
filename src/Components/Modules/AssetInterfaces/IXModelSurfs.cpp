#include <STDInclude.hpp>
#include "IXModelSurfs.hpp"

namespace Assets
{
	void IXModelSurfs::saveXSurfaceCollisionTree(Game::XSurfaceCollisionTree* entry, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::XSurfaceCollisionTree, 40);

		Utils::Stream* buffer = builder->getBuffer();

		Game::XSurfaceCollisionTree* destEntry = buffer->dest<Game::XSurfaceCollisionTree>();
		buffer->save(entry);

		if (entry->nodes)
		{
			AssertSize(Game::XSurfaceCollisionNode, 16);

			buffer->align(Utils::Stream::ALIGN_16);
			buffer->saveArray(entry->nodes, entry->nodeCount);
			Utils::Stream::ClearPointer(&destEntry->nodes);
		}

		if (entry->leafs)
		{
			AssertSize(Game::XSurfaceCollisionLeaf, 2);

			buffer->align(Utils::Stream::ALIGN_2);
			buffer->saveArray(entry->leafs, entry->leafCount);
			Utils::Stream::ClearPointer(&destEntry->leafs);
		}
	}

	void IXModelSurfs::saveXSurface(Game::XSurface* surf, Game::XSurface* destSurf, Components::ZoneBuilder::Zone* builder)
	{
		Utils::Stream* buffer = builder->getBuffer();

		if (surf->vertInfo.vertsBlend)
		{
			buffer->align(Utils::Stream::ALIGN_2);
			buffer->saveArray(surf->vertInfo.vertsBlend, surf->vertInfo.vertCount[0] + (surf->vertInfo.vertCount[1] * 3) + (surf->vertInfo.vertCount[2] * 5) + (surf->vertInfo.vertCount[3] * 7));
			Utils::Stream::ClearPointer(&destSurf->vertInfo.vertsBlend);
		}

		// Access vertex block
		buffer->pushBlock(Game::XFILE_BLOCK_VERTEX);
		if (surf->verts0)
		{
			AssertSize(Game::GfxPackedVertex, 32);

			buffer->align(Utils::Stream::ALIGN_16);
			buffer->saveArray(surf->verts0, surf->vertCount);
			Utils::Stream::ClearPointer(&destSurf->verts0);
		}
		buffer->popBlock();

		// Save_XRigidVertListArray
		if (surf->vertList)
		{
			AssertSize(Game::XRigidVertList, 12);

			buffer->align(Utils::Stream::ALIGN_4);

			Game::XRigidVertList* destCt = buffer->dest<Game::XRigidVertList>();
			buffer->saveArray(surf->vertList, surf->vertListCount);

			for (unsigned int i = 0; i < surf->vertListCount; ++i)
			{
				Game::XRigidVertList* destRigidVertList = &destCt[i];
				Game::XRigidVertList* rigidVertList = &surf->vertList[i];

				if (rigidVertList->collisionTree)
				{
					buffer->align(Utils::Stream::ALIGN_4);
					this->saveXSurfaceCollisionTree(rigidVertList->collisionTree, builder);
					Utils::Stream::ClearPointer(&destRigidVertList->collisionTree);
				}
			}

			Utils::Stream::ClearPointer(&destSurf->vertList);
		}

		// Access index block
		buffer->pushBlock(Game::XFILE_BLOCK_INDEX);
		if (builder->hasPointer(surf->triIndices))
		{
			destSurf->triIndices = builder->getPointer(surf->triIndices);
		}
		else
		{
			buffer->align(Utils::Stream::ALIGN_16);
			buffer->saveArray(surf->triIndices, surf->triCount * 3);
			Utils::Stream::ClearPointer(&destSurf->triIndices);
		}
		buffer->popBlock();
	}

	void IXModelSurfs::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::XModelSurfs, 36);

		Utils::Stream* buffer = builder->getBuffer();
		Game::XModelSurfs* asset = header.modelSurfs;
		Game::XModelSurfs* dest = buffer->dest<Game::XModelSurfs>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->surfs)
		{
			AssertSize(Game::XSurface, 64);

			buffer->align(Utils::Stream::ALIGN_4);

			Game::XSurface* destSurfaces = buffer->dest<Game::XSurface>();
			buffer->saveArray(asset->surfs, asset->numsurfs);

			for (int i = 0; i < asset->numsurfs; ++i)
			{
				this->saveXSurface(&asset->surfs[i], &destSurfaces[i], builder);
			}

			Utils::Stream::ClearPointer(&dest->surfs);
		}

		buffer->popBlock();
	}
}
