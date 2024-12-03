#pragma once

#undef LoadMenuByName_Recursive

namespace Components
{
	class Menus : public Component
	{
	public:
		Menus();

		void preDestroy() override;

		static void Add(const std::string& menu);

		static std::vector<Game::menuDef_t*> LoadMenuByName_Recursive(const std::string& menu);


	private:
		static std::unordered_map<std::string, Game::menuDef_t*> MenusFromDisk;
		static std::unordered_map<std::string, Game::MenuList*> MenuListsFromDisk;
		static Game::ExpressionSupportingData* SupportingData;

		static Game::UiContext* GameUiContexts[];

		static Dvar::Var PrintMenuDebug;

		// Those two point to the ORIGINAL reference of the menu or menu list that was overriden
		static std::unordered_map<std::string, Game::menuDef_t*> OverridenMenus;

		static std::vector<std::string> CustomIW4xMenus;

		static Utils::Memory::Allocator Allocator;

		static bool MenuAlreadyExists(const std::string& name);

		static void FreeZAllocatedMemory(const void* ptr, bool fromTheGame = false);
		static void FreeAllocatedString(const void* ptr, bool fromTheGame = false);
		static void FreeHunkAllocatedMemory(const void* ptr, bool fromTheGame = false);

		template <typename T> static T* Reallocate(const T* ptr, size_t size)
		{
			const auto newData = Allocator.allocate(size);
			std::memcpy(newData, ptr, size);

			return reinterpret_cast<T*>(newData);
		}

		static void PrepareToUnloadMenu(Game::menuDef_t * menu);
		static void AfterLoadedMenuFromDisk(Game::menuDef_t * menu);

		static Game::Statement_s* ReallocateExpressionLocally(Game::Statement_s* statement, bool andFree = false);
		static Game::StaticDvar* ReallocateStaticDvarLocally(Game::StaticDvar* dvar);
		static Game::itemDef_s* ReallocateItemLocally(Game::itemDef_s* item, bool andFree = false);
		static Game::MenuEventHandlerSet* ReallocateEventHandlerSetLocally(const Game::MenuEventHandlerSet* handlerSet, bool andFree= false);
		static Game::ItemKeyHandler* ReallocateItemKeyHandler(const Game::ItemKeyHandler* handlerSet, bool andFree= false);

		static void FreeMenuListOnly(Game::MenuList* menuList);
		static void FreeMenuOnly(Game::menuDef_t * menu);
		static void FreeExpression(Game::Statement_s* statement, bool fromTheGame = false);
		static void FreeItem(Game::itemDef_s* item, bool fromTheGame = false);
		static void FreeEventHandlerSet(Game::MenuEventHandlerSet* handlerSet, bool fromTheGame = false);
		static void FreeItemKeyHandler(Game::ItemKeyHandler* handlerSet, bool fromTheGame = false);

		static void UpdateSupportingDataContents();
		static void FreeLocalSupportingDataContents();
		static void InitializeSupportingData();

		static void UnloadMenuFromDisk(const std::string & menuName);

		static void ReloadDiskMenus();

		static void LoadScriptMenu(const char* menu, bool allowNewMenus);

		static Game::script_s* LoadMenuScript(const std::string& name, const std::string& buffer);
		static int LoadMenuSource(const std::string& name, const std::string& buffer);

		static int ReserveSourceHandle();
		static bool IsValidSourceHandle(int handle);

		static Game::menuDef_t* ParseMenu(int handle);

		static void FreeScript(Game::script_s* script);
		static void FreeMenuSource(int handle);


		static void ReloadDiskMenus_OnCGameStart();
		static void ReloadDiskMenus_OnUIInitialization();

		static void CheckMenus();


		template <typename... Args>
		static void DebugPrint(const std::string_view& fmt, Args&&... args)
		{
			if (PrintMenuDebug.get<bool>())
			{
				const std::string msg = std::vformat(fmt, std::make_format_args(args...));
				const std::string preformatted = std::format("[MENUS] {:X} {}\n", std::hash<std::thread::id>{}(std::this_thread::get_id()), msg);
				Logger::Print(preformatted);
			}
		}

	};
}
