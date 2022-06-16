#include <STDInclude.hpp>

namespace Components
{
	Dvar::Var MapRotation::SVRandomMapRotation;
	Dvar::Var MapRotation::SVDontRotate;

	Game::dvar_t** MapRotation::SVMapRotation = reinterpret_cast<Game::dvar_t**>(0x62C7C44);
	Game::dvar_t** MapRotation::SVMapname = reinterpret_cast<Game::dvar_t**>(0x2098DDC);

	MapRotation::RotationData MapRotation::DedicatedRotation;

	MapRotation::RotationData::RotationData()
		:index_(0)
	{
	}

	void MapRotation::RotationData::randomize()
	{
		// Code from https://en.cppreference.com/w/cpp/algorithm/random_shuffle
		std::random_device rd;
		std::mt19937 gen(rd());

		std::ranges::shuffle(this->rotationEntries_, gen);
	}

	void MapRotation::RotationData::addEntry(const std::string& key, const std::string& value)
	{
		this->rotationEntries_.emplace_back(std::make_pair(key, value));
	}

	std::size_t MapRotation::RotationData::getEntriesSize() const
	{
		return this->rotationEntries_.size();
	}

	MapRotation::RotationData::rotationEntry& MapRotation::RotationData::getNextEntry()
	{
		const auto index = this->index_;
		++this->index_ %= this->rotationEntries_.size(); // Point index_ to the next entry
		return this->rotationEntries_.at(index);
	}

	void MapRotation::RotationData::parse(const std::string& data)
	{
		const auto tokens = Utils::String::Split(data, ' ');

		for (std::size_t i = 0; !tokens.empty() && i < (tokens.size() - 1); i += 2)
		{
			const auto& key = tokens[i];
			const auto& value = tokens[i + 1];

			if (key == "map" || key == "gametype")
			{
				this->addEntry(key, value);
			}
			else
			{
				throw ParseRotationError();
			}
		}
	}

	json11::Json MapRotation::RotationData::to_json() const
	{
		std::vector<std::string> mapVector;
		std::vector<std::string> gametypeVector;

		for (const auto& [key, val] : this->rotationEntries_)
		{
			if (key == "map")
			{
				mapVector.emplace_back(val);
			}
			else if (key == "gametype")
			{
				gametypeVector.emplace_back(val);
			}
		}


		json11::Json mapRotationJson = json11::Json::object
		{
			{"maps", mapVector},
			{"gametypes", gametypeVector},
		};

		return mapRotationJson;
	}

	void MapRotation::LoadRotation(const std::string& data)
	{
		static auto loaded = false;

		if (loaded)
		{
			// Load the rotation once
			return;
		}

		try
		{
			DedicatedRotation.parse(data);
		}
		catch (const std::exception& ex)
		{
			Logger::Print(Game::CON_CHANNEL_SERVER, "{}: sv_mapRotation contains invalid data!\n", ex.what());
		}

		Logger::DebugInfo("DedicatedRotation size after parsing is '{}'\n", DedicatedRotation.getEntriesSize());

		loaded = true;
	}

	void MapRotation::AddMapRotationCommands()
	{
		Command::Add("addMap", [](Command::Params* params)
		{
			if (params->size() < 2)
			{
				Logger::Print("{} <map name> : add a map to the map rotation\n", params->get(0));
				return;
			}

			DedicatedRotation.addEntry("map", params->get(1));
		});

		Command::Add("addGametype", [](Command::Params* params)
		{
			if (params->size() < 2)
			{
				Logger::Print("{} <gametype> : add a game mode to the map rotation\n", params->get(0));
				return;
			}

			DedicatedRotation.addEntry("gametype", params->get(1));
		});
	}

	bool MapRotation::ShouldRotate()
	{
		if (!Dedicated::IsEnabled() && SVDontRotate.get<bool>())
		{
			Logger::Print(Game::CON_CHANNEL_SERVER, "Not performing map rotation as sv_dontRotate is true\n");
			SVDontRotate.set(false);
			return false;
		}

		if (Dvar::Var("party_enable").get<bool>() && Dvar::Var("party_host").get<bool>())
		{
			Logger::Print(Game::CON_CHANNEL_SERVER, "Not performing map rotation as we are hosting a party!\n");
			return false;
		}

		return true;
	}

	void MapRotation::RestartCurrentMap()
	{
		std::string svMapname = (*SVMapname)->current.string;

		if (svMapname.empty())
		{
			Logger::Print(Game::CON_CHANNEL_SERVER, "mapname dvar is empty! Defaulting to mp_afghan\n");
			svMapname = "mp_afghan";
		}

		if (!Dvar::Var("sv_cheats").get<bool>())
		{
			Command::Execute(Utils::String::VA("map %s", svMapname.data()), true);
		}
		else
		{
			Command::Execute(Utils::String::VA("devmap %s", svMapname.data()), true);
		}
	}

	void MapRotation::ApplyMapRotation()
	{
		// Continue to apply gametype until a map is found
		auto foundMap = false;

		std::size_t i = 0;
		while (!foundMap && i < DedicatedRotation.getEntriesSize())
		{
			const auto& entry = DedicatedRotation.getNextEntry();

			if (entry.first == "map")
			{
				Logger::DebugInfo("Loading new map: '{}'\n", entry.second);
				Command::Execute(Utils::String::VA("map %s", entry.second.data()), true);

				// Map was found so we exit the loop
				foundMap = true;
			}
			else if (entry.first == "gametype")
			{
				Logger::DebugInfo("Applying new gametype: '{}'\n", entry.second);
				Dvar::Var("g_gametype").set(entry.second);
			}

			++i;
		}
	}

	void MapRotation::RandomizeMapRotation()
	{
		if (SVRandomMapRotation.get<bool>())
		{
			Logger::Print(Game::CON_CHANNEL_SERVER, "Randomizing the map rotation\n");
			DedicatedRotation.randomize();
		}
		else
		{
			Logger::DebugInfo("Map rotation was not randomized");
		}
	}

	void MapRotation::SV_MapRotate_f()
	{
		if (!ShouldRotate())
		{
			return;
		}

		Logger::Print(Game::CON_CHANNEL_SERVER, "Rotating map...\n");
		const std::string mapRotation = (*SVMapRotation)->current.string;

		// People may have sv_mapRotation empty because they only use 'addMap' or 'addMap'
		if (!mapRotation.empty())
		{
			Logger::DebugInfo("sv_mapRotation is not empty. Parsing...");
			LoadRotation(mapRotation);
		}

		if (DedicatedRotation.getEntriesSize() == 0)
		{
			Logger::Print(Game::CON_CHANNEL_SERVER, "sv_mapRotation is empty or contains invalid data. Restarting map\n");
			RestartCurrentMap();
			return;
		}

		RandomizeMapRotation();

		ApplyMapRotation();
	}

	MapRotation::MapRotation()
	{
		AddMapRotationCommands();
		Utils::Hook::Set<void(*)()>(0x4152E8, SV_MapRotate_f);

		SVRandomMapRotation = Dvar::Register<bool>("sv_randomMapRotation", false,
			Game::dvar_flag::DVAR_ARCHIVE, "Randomize map rotation when true");
		SVDontRotate = Dvar::Register<bool>("sv_dontRotate", false,
			Game::dvar_flag::DVAR_NONE, "Do not perform map rotation");
	}

	bool MapRotation::unitTest()
	{
		RotationData rotation;

		Logger::DebugInfo("Testing map rotation parsing...");

		const auto* normal = "map mp_highrise map mp_terminal map mp_firingrange map mp_trailerpark gametype dm map mp_shipment_long";

		try
		{
			DedicatedRotation.parse(normal);
		}
		catch (const std::exception& ex)
		{
			Logger::PrintError(Game::CON_CHANNEL_ERROR, "{}: parsing of 'normal' failed", ex.what());
			return false;
		}

		const auto* mistake = "spdevmap mp_dome";
		auto success = false;

		try
		{
			DedicatedRotation.parse(mistake);
		}
		catch (const std::exception& ex)
		{
			Logger::Debug("{}: parsing of 'normal' failed as expected", ex.what());
			success = true;
		}

		return success;
	}
}
