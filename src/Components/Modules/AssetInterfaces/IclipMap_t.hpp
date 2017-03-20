#pragma once

namespace Assets
{
	class IclipMap_t : public Components::AssetHandler::IAsset
	{
	public:
		virtual Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_CLIPMAP_PVS; };

		virtual void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		virtual void mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		virtual void load(Game::XAssetHeader* header, std::string name, Components::ZoneBuilder::Zone* builder) override;

	private:
		class SModelQuadtree
		{
		public:
			SModelQuadtree(Game::cStaticModel_t* /*modelList*/, int numModels)
			{
				numValues = 0;

				for (int i = 0; i < numModels; ++i)
				{

				}
			}

			void insert(Game::cStaticModel_t* item)
			{
				if (numValues < 4) // add here
				{
					values[numValues++] = item;
				}
				else // add to child
				{
					if (numValues == 4) // split
					{
						numValues++;
						for (int i = 0; i < 4; ++i)
						{
							if (item->origin[0] > x && values[i]->origin[1] > y)
								children[0]->insert(values[i]);
							if (item->origin[0] < x && values[i]->origin[1] > y)
								children[1]->insert(values[i]);
							if (item->origin[0] < x && values[i]->origin[1] < y)
								children[2]->insert(values[i]);
							if (item->origin[0] > x && values[i]->origin[1] < y)
								children[3]->insert(values[i]);

							values[i] = nullptr;
						}

						for (int i = 0; i < 4; ++i)
						{
							children[i]->halfX = halfX / 2;
							children[i]->halfY = halfY / 2;
							children[i]->halfZ = halfZ / 2;
						}
					}

					if (item->origin[0] > x && item->origin[1] > y)
						children[0]->insert(item);
					if (item->origin[0] < x && item->origin[1] > y)
						children[1]->insert(item);
					if (item->origin[0] < x && item->origin[1] < y)
						children[2]->insert(item);
					if (item->origin[0] > x && item->origin[1] < y)
						children[3]->insert(item);
				}
			}

		private:
			SModelQuadtree* children[4];
			Game::cStaticModel_t* values[4];
			int numValues;
			float x, y, z;
			float halfX, halfY, halfZ;
		};
	};
}
