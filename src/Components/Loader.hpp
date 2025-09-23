#pragma once

namespace Components
{
	class Loader
	{
	public:
		static void Initialize();
	};
}

// Priority
//
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
