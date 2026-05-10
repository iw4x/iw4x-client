#pragma once

namespace Components
{
	class Component
	{
	public:
		Component() = default;
		virtual ~Component() = default;

#if defined(DEBUG)
		virtual std::string getName()
		{
			std::string name = typeid(*this).name();
			Utils::String::Replace(name, "class Components::", "");
			return name;
		};
#endif
	};

	class Loader
	{
	public:
		static void Initialize();
		static void Register(Component* component);

		static bool IsPregame();

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
#include "Modules/Rumble.hpp"

#include "Modules/GSC/GSC.hpp"
#include "Modules/ViewModelFxSetup.hpp"
