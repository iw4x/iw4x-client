#include <STDInclude.hpp>

namespace Assets
{
	void IGameWorldSp::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::GameWorldSp* asset = header.gameWorldSp;

		if (asset->pathData.nodes)
		{
			for (unsigned int i = 0; i < asset->pathData.nodeCount; ++i)
			{
				Game::pathnode_t* node = &asset->pathData.nodes[i];

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

		if (trackSegment->name)
		{
			buffer->saveString(trackSegment->name);
			Utils::Stream::ClearPointer(&destTrackSegment->name);
		}

		if (trackSegment->trackSectors)
		{
			AssertSize(Game::VehicleTrackSector, 60);
			buffer->align(Utils::Stream::ALIGN_4);

			Game::VehicleTrackSector* destTrackSectors = buffer->dest<Game::VehicleTrackSector>();
			buffer->saveArray(trackSegment->trackSectors, trackSegment->trackSectorCount);

			for (int i = 0; i < trackSegment->trackSectorCount; ++i)
			{
				Game::VehicleTrackSector* destTrackSector = &destTrackSectors[i];
				Game::VehicleTrackSector* trackSector = &trackSegment->trackSectors[i];

				if (trackSector->trackObstacles)
				{
					AssertSize(Game::VehicleTrackObstacle, 12);
					buffer->align(Utils::Stream::ALIGN_4);
					buffer->saveArray(trackSector->trackObstacles, trackSector->trackObstacleCount);
					Utils::Stream::ClearPointer(&destTrackSector->trackObstacles);
				}
			}
		}

		if (trackSegment->trackSegments1)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			this->saveVehicleTrackSegment_ptrArray(trackSegment->trackSegments1, trackSegment->trackSegmentCount1, builder);
			Utils::Stream::ClearPointer(&destTrackSegment->trackSegments1);
		}

		if (trackSegment->trackSegments2)
		{
			buffer->align(Utils::Stream::ALIGN_4);
			this->saveVehicleTrackSegment_ptrArray(trackSegment->trackSegments2, trackSegment->trackSegmentCount2, builder);
			Utils::Stream::ClearPointer(&destTrackSegment->trackSegments2);
		}
	}

	void IGameWorldSp::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::GameWorldMp, 8);

		Utils::Stream* buffer = builder->getBuffer();
		Game::GameWorldSp* asset = header.gameWorldSp;
		Game::GameWorldSp* dest = buffer->dest<Game::GameWorldSp>();
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

			if (asset->pathData.nodes)
			{
				AssertSize(Game::pathnode_t, 136);
				buffer->align(Utils::Stream::ALIGN_4);

				Game::pathnode_t* destNodes = buffer->dest<Game::pathnode_t>();
				buffer->saveArray(asset->pathData.nodes, asset->pathData.nodeCount);

				for (unsigned int i = 0; i < asset->pathData.nodeCount; ++i)
				{
					Game::pathnode_t* destNode = &destNodes[i];
					Game::pathnode_t* node = &asset->pathData.nodes[i];

					AssertSize(Game::pathnode_constant_t, 64);

					for (char j = 0; j < 5; ++j)
					{
						builder->mapScriptString(&(&node->constant.targetname)[j]);
					}

					if (node->constant.Links)
					{
						AssertSize(Game::pathlink_s, 12);
						buffer->align(Utils::Stream::ALIGN_4);
						buffer->saveArray(node->constant.Links, node->constant.totalLinkCount);
						Utils::Stream::ClearPointer(&destNode->constant.Links);
					}
				}

				Utils::Stream::ClearPointer(&dest->pathData.nodes);
			}

			buffer->pushBlock(Game::XFILE_BLOCK_RUNTIME);

			if (asset->pathData.basenodes)
			{
				AssertSize(Game::pathbasenode_t, 16);

				buffer->align(Utils::Stream::ALIGN_16);
				buffer->saveArray(asset->pathData.basenodes, asset->pathData.nodeCount);
				Utils::Stream::ClearPointer(&dest->pathData.basenodes);
			}

			buffer->popBlock();

			if (asset->pathData.chainNodeForNode)
			{
				buffer->align(Utils::Stream::ALIGN_2);
				buffer->saveArray(asset->pathData.chainNodeForNode, asset->pathData.nodeCount);
				Utils::Stream::ClearPointer(&dest->pathData.chainNodeForNode);
			}

			if (asset->pathData.nodeForChainNode)
			{
				buffer->align(Utils::Stream::ALIGN_2);
				buffer->saveArray(asset->pathData.nodeForChainNode, asset->pathData.nodeCount);
				Utils::Stream::ClearPointer(&dest->pathData.nodeForChainNode);
			}

			if (asset->pathData.pathVis)
			{
				buffer->saveArray(asset->pathData.pathVis, asset->pathData.visBytes);
				Utils::Stream::ClearPointer(&dest->pathData.pathVis);
			}

			if (asset->pathData.nodeTree)
			{
				AssertSize(Game::pathnode_tree_t, 16);
				buffer->align(Utils::Stream::ALIGN_4);

				Game::pathnode_tree_t* destNodeTrees = buffer->dest<Game::pathnode_tree_t>();
				buffer->saveArray(asset->pathData.nodeTree, asset->pathData.nodeTreeCount);

				for (int i = 0; i < asset->pathData.nodeTreeCount; ++i)
				{
					Game::pathnode_tree_t* destNodeTree = &destNodeTrees[i];
					Game::pathnode_tree_t* nodeTree = &asset->pathData.nodeTree[i];

					this->savepathnode_tree_info_t(nodeTree, destNodeTree, builder);
				}

				Utils::Stream::ClearPointer(&dest->pathData.nodeTree);
			}
		}

		// Save_VehicleTrack
		{
			AssertSize(Game::VehicleTrack, 8);

			if (asset->vehicleTrack.trackSegments)
			{
				if (builder->hasPointer(asset->vehicleTrack.trackSegments))
				{
					dest->vehicleTrack.trackSegments = builder->getPointer(asset->vehicleTrack.trackSegments);
				}
				else
				{
					AssertSize(Game::VehicleTrackSegment, 44);

					buffer->align(Utils::Stream::ALIGN_4);
					Game::VehicleTrackSegment* destTrackSegments = buffer->dest<Game::VehicleTrackSegment>();

					for (int i = 0; i < asset->vehicleTrack.trackSegmentCount; ++i)
					{
						builder->storePointer(&asset->vehicleTrack.trackSegments[i]);
						buffer->save(&asset->vehicleTrack.trackSegments[i]);
					}

					for (int i = 0; i < asset->vehicleTrack.trackSegmentCount; ++i)
					{
						Game::VehicleTrackSegment* destTrackSegment = &destTrackSegments[i];
						Game::VehicleTrackSegment* trackSegment = &asset->vehicleTrack.trackSegments[i];

						this->saveVehicleTrackSegment(trackSegment, destTrackSegment, builder);
					}

					Utils::Stream::ClearPointer(&dest->vehicleTrack.trackSegments);
				}
			}
		}

		if (asset->data)
		{
			// Save_G_GlassData
			{
				AssertSize(Game::G_GlassData, 128);
				buffer->align(Utils::Stream::ALIGN_4);

				Game::G_GlassData* destGlass = buffer->dest<Game::G_GlassData>();
				buffer->save(asset->data);

				if (asset->data->glassPieces)
				{
					AssertSize(Game::G_GlassPiece, 12);
					buffer->align(Utils::Stream::ALIGN_4);
					buffer->saveArray(asset->data->glassPieces, asset->data->pieceCount);
					Utils::Stream::ClearPointer(&destGlass->glassPieces);
				}

				if (asset->data->glassNames)
				{
					AssertSize(Game::G_GlassName, 12);
					buffer->align(Utils::Stream::ALIGN_4);

					Game::G_GlassName* destGlassNames = buffer->dest<Game::G_GlassName>();
					buffer->saveArray(asset->data->glassNames, asset->data->glassNameCount);

					for (unsigned int i = 0; i < asset->data->glassNameCount; ++i)
					{
						Game::G_GlassName* destGlassName = &destGlassNames[i];
						Game::G_GlassName* glassName = &asset->data->glassNames[i];

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

			Utils::Stream::ClearPointer(&dest->data);
		}

		buffer->popBlock();
	}
}
