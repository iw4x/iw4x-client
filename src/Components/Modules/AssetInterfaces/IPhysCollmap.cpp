#include <STDInclude.hpp>

namespace Assets
{
	void IPhysCollmap::Save_BrushWrapper(Components::ZoneBuilder::Zone* builder, Game::BrushWrapper* brush)
	{
		Assert_Size(Game::BrushWrapper, 68);

		Utils::Stream* buffer = builder->GetBuffer();

		Game::BrushWrapper* destBrush = buffer->Dest<Game::BrushWrapper>();
		buffer->Save(brush, sizeof(Game::BrushWrapper));

		// Save_cbrushWrapper_t
		{
			Assert_Size(Game::cbrushWrapper_t, 36);

			if (brush->brush.brushSide)
			{
				Assert_Size(Game::cbrushside_t, 8);

				buffer->Align(Utils::Stream::ALIGN_4);

				Game::cbrushside_t* destBrushSide = buffer->Dest<Game::cbrushside_t>();
				buffer->SaveArray(brush->brush.brushSide, brush->brush.count);

				// Save_cbrushside_tArray
				for (short i = 0; i < brush->brush.count; ++i)
				{
					Game::cbrushside_t* destSide = &destBrushSide[i];
					Game::cbrushside_t* side = &brush->brush.brushSide[i];

					if (side->side)
					{
						if (builder->HasPointer(side->side))
						{
							destSide->side = builder->GetPointer(side->side);
						}
						else
						{
							buffer->Align(Utils::Stream::ALIGN_4);
							builder->StorePointer(side->side);

							buffer->Save(side->side, sizeof(Game::cplane_t));
							destSide->side = reinterpret_cast<Game::cplane_t*>(-1);
						}
					}
				}

				destBrush->brush.brushSide = reinterpret_cast<Game::cbrushside_t*>(-1);
			}

			if (brush->brush.brushEdge)
			{
				buffer->Save(brush->brush.brushEdge, brush->totalEdgeCount);
				destBrush->brush.brushEdge = reinterpret_cast<char*>(-1);
			}
		}

		if (brush->planes)
		{
			Assert_Size(Game::cplane_t, 20);

			if (builder->HasPointer(brush->planes))
			{
				destBrush->planes = builder->GetPointer(brush->planes);
			}
			else
			{
				buffer->Align(Utils::Stream::ALIGN_4);
				builder->StorePointer(brush->planes);

				buffer->Save(brush->planes, sizeof(Game::cplane_t));
				destBrush->planes = reinterpret_cast<Game::cplane_t*>(-1);
			}
		}
	}

	void IPhysCollmap::Save_PhysGeomInfoArray(Components::ZoneBuilder::Zone* builder, Game::PhysGeomInfo* geoms, unsigned int count)
	{
		Assert_Size(Game::PhysGeomInfo, 68);

		Utils::Stream* buffer = builder->GetBuffer();

		Game::PhysGeomInfo* destGeoms = buffer->Dest<Game::PhysGeomInfo>();
		buffer->SaveArray(geoms, count);

		for (unsigned int i = 0; i < count; ++i)
		{
			Game::PhysGeomInfo* destGeom = &destGeoms[i];
			Game::PhysGeomInfo* geom = &geoms[i];

			if (geom->brush)
			{
				buffer->Align(Utils::Stream::ALIGN_4);

				IPhysCollmap::Save_BrushWrapper(builder, geom->brush);
				destGeom->brush = reinterpret_cast<Game::BrushWrapper*>(-1);
			}
		}
	}

	void IPhysCollmap::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_Size(Game::XModel, 304);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::PhysCollmap* asset = header.physCollmap;
		Game::PhysCollmap* dest = buffer->Dest<Game::PhysCollmap>();
		buffer->Save(asset, sizeof(Game::PhysCollmap));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			dest->name = reinterpret_cast<char*>(-1);
		}

		if (asset->geoms)
		{
			buffer->Align(Utils::Stream::ALIGN_4);

			IPhysCollmap::Save_PhysGeomInfoArray(builder, asset->geoms, asset->count);
			dest->geoms = reinterpret_cast<Game::PhysGeomInfo*>(-1);
		}

		buffer->PopBlock();
	}
}
