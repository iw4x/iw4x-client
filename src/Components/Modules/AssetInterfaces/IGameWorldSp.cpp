#include <STDInclude.hpp>
#include "IGameWorldSp.hpp"

namespace Assets
{
	void IGameWorldSp::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::GameWorldSp* asset = header.gameWorldSp;

		if (asset->path.nodes)
		{
			for (unsigned int i = 0; i < asset->path.nodeCount; ++i)
			{
				Game::pathnode_t* node = &asset->path.nodes[i];

				for (char j = 0; j < 5; ++j)
				{
					builder->addScriptString((&node->constant.targetname)[j]);
				}
			}
		}
	}

	void IGameWorldSp::savepathnode_tree_info_t(Game::pathnode_tree_t* nodeTree, Game::pathnode_tree_t* destNodeTree, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::pathnode_tree_info_t, 8);
		Utils::Stream* buffer = builder->getBuffer();

		if (nodeTree->axis < 0)
		{
			AssertSize(Game::pathnode_tree_nodes_t, 8);

			if (nodeTree->u.s.nodes)
			{
				buffer->align(Utils::Stream::ALIGN_2);
				buffer->saveArray(nodeTree->u.s.nodes, nodeTree->u.s.nodeCount);
				Utils::Stream::ClearPointer(&destNodeTree->u.s.nodes);
			}
		}
		else
		{
			for (int i = 0; i < 2; ++i)
			{
				Game::pathnode_tree_t** destChildNodeTreePtr = &destNodeTree->u.child[i];
				Game::pathnode_tree_t** childNodeTreePtr = &nodeTree->u.child[i];

				if (*childNodeTreePtr)
				{
					if (builder->hasPointer(*childNodeTreePtr))
					{
						*destChildNodeTreePtr = builder->getPointer(*childNodeTreePtr);
					}
					else
					{
						buffer->align(Utils::Stream::ALIGN_4);
						builder->storePointer(*childNodeTreePtr);

						Game::pathnode_tree_t* destChildNodeTree = buffer->dest<Game::pathnode_tree_t>();
						buffer->save(*childNodeTreePtr);

						this->savepathnode_tree_info_t(*childNodeTreePtr, destChildNodeTree, builder);
						Utils::Stream::ClearPointer(destChildNodeTreePtr);
					}
				}
			}
		}
	}

	void IGameWorldSp::saveVehicleTrackSegment_ptrArray(Game::VehicleTrackSegment** trackSegmentPtrs, int count, Components::ZoneBuilder::Zone* builder)
	{
		Utils::Stream* buffer = builder->getBuffer();
		if (!trackSegmentPtrs) return;

		Game::VehicleTrackSegment** destTrackSegmentPtrs = buffer->dest<Game::VehicleTrackSegment*>();
		buffer->saveArray(trackSegmentPtrs, count);

		for (int i = 0; i < count; ++i)
		{
			Game::VehicleTrackSegment** destTrackSegmentPtr = &destTrackSegmentPtrs[i];
			Game::VehicleTrackSegment** trackSegmentPtr = &trackSegmentPtrs[i];

			if (*trackSegmentPtr)
			{
				if (builder->hasPointer(*trackSegmentPtr))
				{
					*destTrackSegmentPtr = builder->getPointer(*trackSegmentPtr);
				}
				else
				{
					buffer->align(Utils::Stream::ALIGN_4);
					builder->storePointer(*trackSegmentPtr);

					Game::VehicleTrackSegment* destTrackSegment = buffer->dest<Game::VehicleTrackSegment>();
					buffer->save(*trackSegmentPtr);

					this->saveVehicleTrackSegment(*trackSegmentPtr, destTrackSegment, builder);

					Utils::Stream::ClearPointer(destTrackSegmentPtr);
				}
			}
		}
	}

	void IGameWorldSp::saveVehicleTrackSegment(Game::VehicleTrackSegment* trackSegment, Game::VehicleTrackSegment* destTrackSegment, Components::ZoneBuilder::Zone* builder)
	{
		Utils::Stream* buffer = builder->getBuffer();

		if (trackSegment->targetName)
		{
			buffer->saveString(trackSegment->targetName);
			Utils::Stream::ClearPointer(&destTrackSegment->targetName);
		}

		if (trackSegment->sectors)
		{
			AssertSize(Game::VehicleTrackSector, 60);
			buffer->align(Utils::Stream::ALIGN_4);

			Game::VehicleTrackSector* destTrackSectors = buffer->dest<Game::VehicleTrackSector>();
			buffer->saveArray(trackSegment->sectors, trackSegment->sectorCount);

			for (unsigned int i = 0; i < trackSegment->sectorCount; ++i)
			{
				Game::VehicleTrackSector* destTrackSector = &destTrackSectors[i];
				Game::VehicleTrackSector* trackSector = &trackSegment->sectors[i];

				if (trackSector->obstacles)
				{
					AssertSize(Game::VehicleTrackObstacle, 12);
					buffer->align(Utils::Stream::ALIGN_4);
					buffer->saveArray(trackSector->obstacles, trackSector->obstacleCount);
					Utils::Stream::ClearPointer(&destTrackSector->obstacles);
				}
			}
		}

		if (trackSegment->nextBranches)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			this->saveVehicleTrackSegment_ptrArray(trackSegment->nextBranches, trackSegment->nextBranchesCount, builder);
			Utils::Stream::ClearPointer(&destTrackSegment->nextBranches);
		}

		if (trackSegment->prevBranches)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			this->saveVehicleTrackSegment_ptrArray(trackSegment->prevBranches, trackSegment->prevBranchesCount, builder);
			Utils::Stream::ClearPointer(&destTrackSegment->prevBranches);
		}
	}

	void IGameWorldSp::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::GameWorldSp, 0x38);

		Utils::Stream* buffer = builder->getBuffer();
		auto* asset = header.gameWorldSp;
		auto* dest = buffer->dest<Game::GameWorldSp>();
		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		// Save_PathData
		{
			AssertSize(Game::PathData, 40);

			if (asset->path.nodes)
			{
				AssertSize(Game::pathnode_t, 136);
				buffer->align(Utils::Stream::ALIGN_4);

				Game::pathnode_t* destNodes = buffer->dest<Game::pathnode_t>();
				buffer->saveArray(asset->path.nodes, asset->path.nodeCount);

				for (unsigned int i = 0; i < asset->path.nodeCount; ++i)
				{
					Game::pathnode_t* destNode = &destNodes[i];
					Game::pathnode_t* node = &asset->path.nodes[i];

					AssertSize(Game::pathnode_constant_t, 64);

					for (char j = 0; j < 5; ++j)
					{
						builder->mapScriptString((&node->constant.targetname)[j]);
					}

					if (node->constant.Links)
					{
						AssertSize(Game::pathlink_s, 12);
						buffer->align(Utils::Stream::ALIGN_4);
						buffer->saveArray(node->constant.Links, node->constant.totalLinkCount);
						Utils::Stream::ClearPointer(&destNode->constant.Links);
					}
				}

				Utils::Stream::ClearPointer(&dest->path.nodes);
			}

			buffer->pushBlock(Game::XFILE_BLOCK_RUNTIME);

			if (asset->path.basenodes)
			{
				AssertSize(Game::pathbasenode_t, 16);

				buffer->align(Utils::Stream::ALIGN_16);
				buffer->saveArray(asset->path.basenodes, asset->path.nodeCount);
				Utils::Stream::ClearPointer(&dest->path.basenodes);
			}

			buffer->popBlock();

			if (asset->path.chainNodeForNode)
			{
				buffer->align(Utils::Stream::ALIGN_2);
				buffer->saveArray(asset->path.chainNodeForNode, asset->path.nodeCount);
				Utils::Stream::ClearPointer(&dest->path.chainNodeForNode);
			}

			if (asset->path.nodeForChainNode)
			{
				buffer->align(Utils::Stream::ALIGN_2);
				buffer->saveArray(asset->path.nodeForChainNode, asset->path.nodeCount);
				Utils::Stream::ClearPointer(&dest->path.nodeForChainNode);
			}

			if (asset->path.pathVis)
			{
				buffer->saveArray(asset->path.pathVis, asset->path.visBytes);
				Utils::Stream::ClearPointer(&dest->path.pathVis);
			}

			if (asset->path.nodeTree)
			{
				AssertSize(Game::pathnode_tree_t, 16);
				buffer->align(Utils::Stream::ALIGN_4);

				Game::pathnode_tree_t* destNodeTrees = buffer->dest<Game::pathnode_tree_t>();
				buffer->saveArray(asset->path.nodeTree, asset->path.nodeTreeCount);

				for (int i = 0; i < asset->path.nodeTreeCount; ++i)
				{
					Game::pathnode_tree_t* destNodeTree = &destNodeTrees[i];
					Game::pathnode_tree_t* nodeTree = &asset->path.nodeTree[i];

					this->savepathnode_tree_info_t(nodeTree, destNodeTree, builder);
				}

				Utils::Stream::ClearPointer(&dest->path.nodeTree);
			}
		}

		// Save_VehicleTrack
		{
			AssertSize(Game::VehicleTrack, 8);

			if (asset->vehicleTrack.segments)
			{
				if (builder->hasPointer(asset->vehicleTrack.segments))
				{
					dest->vehicleTrack.segments = builder->getPointer(asset->vehicleTrack.segments);
				}
				else
				{
					AssertSize(Game::VehicleTrackSegment, 44);

					buffer->align(Utils::Stream::ALIGN_4);
					Game::VehicleTrackSegment* destTrackSegments = buffer->dest<Game::VehicleTrackSegment>();

					for (unsigned int i = 0; i < asset->vehicleTrack.segmentCount; ++i)
					{
						builder->storePointer(&asset->vehicleTrack.segments[i]);
						buffer->save(&asset->vehicleTrack.segments[i]);
					}

					for (unsigned int i = 0; i < asset->vehicleTrack.segmentCount; ++i)
					{
						Game::VehicleTrackSegment* destTrackSegment = &destTrackSegments[i];
						Game::VehicleTrackSegment* trackSegment = &asset->vehicleTrack.segments[i];

						this->saveVehicleTrackSegment(trackSegment, destTrackSegment, builder);
					}

					Utils::Stream::ClearPointer(&dest->vehicleTrack.segments);
				}
			}
		}

		if (asset->g_glassData)
		{
			// Save_G_GlassData
			{
				AssertSize(Game::G_GlassData, 128);
				buffer->align(Utils::Stream::ALIGN_4);

				Game::G_GlassData* destGlass = buffer->dest<Game::G_GlassData>();
				buffer->save(asset->g_glassData);

				if (asset->g_glassData->glassPieces)
				{
					AssertSize(Game::G_GlassPiece, 12);
					buffer->align(Utils::Stream::ALIGN_4);
					buffer->saveArray(asset->g_glassData->glassPieces, asset->g_glassData->pieceCount);
					Utils::Stream::ClearPointer(&destGlass->glassPieces);
				}

				if (asset->g_glassData->glassNames)
				{
					AssertSize(Game::G_GlassName, 12);
					buffer->align(Utils::Stream::ALIGN_4);

					Game::G_GlassName* destGlassNames = buffer->dest<Game::G_GlassName>();
					buffer->saveArray(asset->g_glassData->glassNames, asset->g_glassData->glassNameCount);

					for (unsigned int i = 0; i < asset->g_glassData->glassNameCount; ++i)
					{
						Game::G_GlassName* destGlassName = &destGlassNames[i];
						Game::G_GlassName* glassName = &asset->g_glassData->glassNames[i];

						if (glassName->nameStr)
						{
							buffer->saveString(glassName->nameStr);
							Utils::Stream::ClearPointer(&destGlassName->nameStr);
						}

						if (glassName->pieceIndices)
						{
							buffer->align(Utils::Stream::ALIGN_2);
							buffer->saveArray(glassName->pieceIndices, glassName->pieceCount);
							Utils::Stream::ClearPointer(&destGlassName->pieceIndices);
						}
					}

					Utils::Stream::ClearPointer(&destGlass->glassNames);
				}
			}

			Utils::Stream::ClearPointer(&dest->g_glassData);
		}

		buffer->popBlock();
	}
}
