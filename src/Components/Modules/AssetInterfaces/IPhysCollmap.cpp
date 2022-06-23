#include <STDInclude.hpp>
#include "IPhysCollmap.hpp"

namespace Assets
{
	void IPhysCollmap::saveBrushWrapper(Components::ZoneBuilder::Zone* builder, Game::BrushWrapper* brush)
	{
		AssertSize(Game::BrushWrapper, 68);

		Utils::Stream* buffer = builder->getBuffer();

		Game::BrushWrapper* destBrush = buffer->dest<Game::BrushWrapper>();
		buffer->save(brush);

		// Save_cbrushWrapper_t
		{
			AssertSize(Game::cbrush_t, 36);

			if (brush->brush.sides)
			{
				AssertSize(Game::cbrushside_t, 8);

				buffer->align(Utils::Stream::ALIGN_4);

				Game::cbrushside_t* destBrushSide = buffer->dest<Game::cbrushside_t>();
				buffer->saveArray(brush->brush.sides, brush->brush.numsides);

				// Save_cbrushside_tArray
				for (unsigned short i = 0; i < brush->brush.numsides; ++i)
				{
					Game::cbrushside_t* destSide = &destBrushSide[i];
					Game::cbrushside_t* side = &brush->brush.sides[i];

					if (side->plane)
					{
						if (builder->hasPointer(side->plane))
						{
							destSide->plane = builder->getPointer(side->plane);
						}
						else
						{
							buffer->align(Utils::Stream::ALIGN_4);
							builder->storePointer(side->plane);

							buffer->save(side->plane, sizeof(Game::cplane_s));
							Utils::Stream::ClearPointer(&destSide->plane);
						}
					}
				}

				Utils::Stream::ClearPointer(&destBrush->brush.sides);
			}

			if (brush->brush.baseAdjacentSide)
			{
				buffer->save(brush->brush.baseAdjacentSide, brush->totalEdgeCount);
				Utils::Stream::ClearPointer(&destBrush->brush.baseAdjacentSide);
			}
		}

		if (brush->planes)
		{
			AssertSize(Game::cplane_s, 20);

			if (builder->hasPointer(brush->planes))
			{
				Components::Logger::Print("Loading cplane pointer before the array has been written. Not sure if this is correct!\n");
				destBrush->planes = builder->getPointer(brush->planes);
			}
			else
			{
				buffer->align(Utils::Stream::ALIGN_4);

				for (unsigned short j = 0; j < brush->brush.numsides; ++j)
				{
					builder->storePointer(&brush->planes[j]);
					buffer->save(&brush->planes[j]);
				}

				Utils::Stream::ClearPointer(&destBrush->planes);
			}
		}
	}

	void IPhysCollmap::savePhysGeomInfoArray(Components::ZoneBuilder::Zone* builder, Game::PhysGeomInfo* geoms, unsigned int count)
	{
		AssertSize(Game::PhysGeomInfo, 68);

		Utils::Stream* buffer = builder->getBuffer();

		Game::PhysGeomInfo* destGeoms = buffer->dest<Game::PhysGeomInfo>();
		buffer->saveArray(geoms, count);

		for (unsigned int i = 0; i < count; ++i)
		{
			Game::PhysGeomInfo* destGeom = &destGeoms[i];
			Game::PhysGeomInfo* geom = &geoms[i];

			if (geom->brushWrapper)
			{
				buffer->align(Utils::Stream::ALIGN_4);

				this->saveBrushWrapper(builder, geom->brushWrapper);
				Utils::Stream::ClearPointer(&destGeom->brushWrapper);
			}
		}
	}

	void IPhysCollmap::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::XModel, 304);

		Utils::Stream* buffer = builder->getBuffer();
		Game::PhysCollmap* asset = header.physCollmap;
		Game::PhysCollmap* dest = buffer->dest<Game::PhysCollmap>();
		buffer->save(asset, sizeof(Game::PhysCollmap));

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->geoms)
		{
			buffer->align(Utils::Stream::ALIGN_4);

			this->savePhysGeomInfoArray(builder, asset->geoms, asset->count);
			Utils::Stream::ClearPointer(&dest->geoms);
		}

		buffer->popBlock();
	}
}
