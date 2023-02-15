#pragma once

namespace Assets
{
	class IclipMap_t : public Components::AssetHandler::IAsset
	{
	public:
		Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_CLIPMAP_MP; }

		void save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		void mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder) override;
		void load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* builder) override;
		static void dump(Game::XAssetHeader /*header*/);

	private:
		class SModelQuadtree
		{
		public:
			SModelQuadtree()
			{
			}

			SModelQuadtree(Game::cStaticModel_s* modelList, int numModels)
			{
				numValues = 0;

				for (int i = 0; i < numModels; ++i)
				{
					insert(&modelList[i]);
				}
			}

			void insert(Game::cStaticModel_s* item)
			{
				if (numValues < 4) // add here
				{
					values[numValues++] = item;
				}
				else // add to child
				{
					if (numValues == 4) // split
					{
						// create children objects
						for (int i = 0; i < 4; ++i)
						{
							children[i] = new SModelQuadtree();
						}

						for (int i = 0; i < numValues; ++i)
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
							children[i]->halfZ = halfZ;
						}

						// update origins here

						numValues++;
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
			Game::cStaticModel_s* values[4];
			int numValues;
			float x, y, z;
			float halfX, halfY, halfZ;
		};
	};
}
