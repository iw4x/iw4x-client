#pragma once

namespace Components
{
	class Component
	{
	public:
		Component() = default;
		virtual ~Component() = default;

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		virtual std::string getName()
		{
			std::string name = typeid(*this).name();
			Utils::String::Replace(name, "class Components::", "");
			return name;
		};
#endif

		// It's illegal to spawn threads in DLLMain, and apparently it causes problems if they are destroyed there as well.
		// This method is called before DLLMain (if possible) and should to destroy threads.
		// It's not 100% guaranteed that it's called outside DLLMain, as it depends on the game, but it's 100% guaranteed, that it is called at all.
		virtual void preDestroy() {}
		virtual bool unitTest() { return true; } // Unit testing entry
	};

	class Loader
	{
	public:
		static void Initialize();
		static void Uninitialize();
		static void PreDestroy();
		static void PreDestroyNoPostGame();
		static bool PerformUnitTests();
		static bool IsPerformingUnitTests();
		static void Register(Component* component);

		static bool IsPregame();
		static bool IsPostgame();
		static bool IsUninitializing();

		template <typename T>
		static T* GetInstance()
		{
			for (auto& component : Components)
			{
				if (typeid(*component) == typeid(T))
				{
					return reinterpret_cast<T*>(component);
				}
			}

			return nullptr;
		}

	private:
		static bool Pregame;
		static bool Postgame;
		static bool Uninitializing;
		static std::vector<Component*> Components;
	};
}

// Priority
#include "Modules/Command.hpp"
#include "Modules/Dvar.hpp"
#include "Modules/Flags.hpp"
#include "Modules/Network.hpp"
#include "Modules/Logger.hpp"
#include "Modules/Singleton.hpp"
#include "Modules/UIScript.hpp"
#include "Modules/ZoneBuilder.hpp"

#include "Modules/AssetHandler.hpp"
#include "Modules/Dedicated.hpp"
#include "Modules/FileSystem.hpp"
#include "Modules/Localization.hpp"
#include "Modules/Maps.hpp"
#include "Modules/Menus.hpp"
#include "Modules/Renderer.hpp"
#include "Modules/Scheduler.hpp"
#include "Modules/Zones.hpp"

#include "Modules/GSC/GSC.hpp"
