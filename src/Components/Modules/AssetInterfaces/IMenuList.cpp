#include "STDInclude.hpp"

namespace Assets
{
    void IMenuList::load(Game::XAssetHeader* header, const std::string& name, Components::ZoneBuilder::Zone* /*builder*/)
    {
        header->menuList = Components::Menus::LoadMenuList(name);

        for (int i = 0; i < header->menuList->menuCount; i++)
        {
            ImenuDef_t::LoadedMenus[header->menuList->menus[i]->window.name] = header->menuList->menus[i];
        }

    }
	void IMenuList::mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::MenuList *asset = header.menuList;

		for (int i = 0; i < asset->menuCount; ++i)
		{
			if (asset->menus[i])
			{
				builder->loadAsset(Game::XAssetType::ASSET_TYPE_MENU, asset->menus[i]);
			}
		}
	}
	void IMenuList::save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		AssertSize(Game::MenuList, 12);

		Utils::Stream* buffer = builder->getBuffer();
		Game::MenuList* asset = header.menuList;
		Game::MenuList* dest = buffer->dest<Game::MenuList>();

		buffer->save(asset);

		buffer->pushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->saveString(builder->getAssetName(this->getType(), asset->name));
			Utils::Stream::ClearPointer(&dest->name);
		}

		if (asset->menus)
		{
			buffer->align(Utils::Stream::ALIGN_4);

			Game::menuDef_t **destMenus = buffer->dest<Game::menuDef_t*>();
			buffer->saveArray(asset->menus, asset->menuCount);

			for (int i = 0; i < asset->menuCount; ++i)
			{
				destMenus[i] = builder->saveSubAsset(Game::XAssetType::ASSET_TYPE_MENU, asset->menus[i]).menu;
			}
		}

		buffer->popBlock();
	}
}
