#include <STDInclude.hpp>
#include "MapRotation.hpp"

namespace Components
{
	Dvar::Var MapRotation::SVRandomMapRotation;
	Dvar::Var MapRotation::SVDontRotate;
	Dvar::Var MapRotation::SVNextMap;

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

	std::size_t MapRotation::RotationData::getEntriesSize() const noexcept
	{
		return this->rotationEntries_.size();
	}

	MapRotation::RotationData::rotationEntry& MapRotation::RotationData::getNextEntry()
	{
		const auto index = this->index_;
		++this->index_ %= this->rotationEntries_.size(); // Point index_ to the next entry
		return this->rotationEntries_.at(index);
	}

	MapRotation::RotationData::rotationEntry& MapRotation::RotationData::peekNextEntry()
	{
		return this->rotationEntries_.at(this->index_);
	}

	void MapRotation::RotationData::parse(const std::string& data)
	{
		const auto tokens = Utils::String::Split(data, ' ');

		for (std::size_t i = 0; !tokens.empty() && i < (tokens.size() - 1); i += 2)
		{
			const auto& key = tokens[i];
			const auto& value = tokens[i + 1];

			if (key == "map"s || key == "gametype"s)
			{
				this->addEntry(key, value);
			}
			else
			{
				throw MapRotationParseError();
			}
		}
	}

	bool MapRotation::RotationData::empty() const noexcept
	{
		return this->rotationEntries_.empty();
	}

	bool MapRotation::RotationData::contains(const std::string& key, const std::string& value) const
	{
		return std::ranges::any_of(this->rotationEntries_, [&](const auto& entry)
		{
			return entry.first == key && entry.second == value;
		});
	}

	void MapRotation::LoadRotation(const std::string& data)
	{
		try
		{
			DedicatedRotation.parse(data);
		}
		catch (const std::exception& ex)
		{
			Logger::PrintError(Game::CON_CHANNEL_ERROR, "{}: {} contains invalid data!\n", ex.what(), (*Game::sv_mapRotation)->name);
		}

		Logger::Debug("DedicatedRotation size after parsing is '{}'", DedicatedRotation.getEntriesSize());
	}

	void MapRotation::LoadMapRotation()
	{
		static auto loaded = false;
		if (loaded)
		{
			// Load the rotation once
			return;
		}

		loaded = true;

		const std::string mapRotation = (*Game::sv_mapRotation)->current.string;
		// People may have sv_mapRotation empty because they only use 'addMap' or 'addGametype'
		if (!mapRotation.empty())
		{
			Logger::Debug("sv_mapRotation is not empty. Parsing...");
			LoadRotation(mapRotation);
		}
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

	bool MapRotation::Contains(const std::string& key, const std::string& value)
	{
		return DedicatedRotation.contains(key, value);
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
			Logger::Warning(Game::CON_CHANNEL_SERVER, "Not performing map rotation as we are hosting a party!\n");
			return false;
		}

		return true;
	}

	void MapRotation::ApplyMap(const std::string& map)
	{
		assert(!map.empty());

		if ((*Game::sv_cheats)->current.enabled)
		{
			Command::Execute(std::format("devmap {}", map), true);
		}
		else
		{
			Command::Execute(std::format("map {}", map), true);
		}
	}

	void MapRotation::ApplyGametype(const std::string& gametype)
	{
		assert(!gametype.empty());
		Game::Dvar_SetStringByName("g_gametype", gametype.data());
	}

	void MapRotation::RestartCurrentMap()
	{
		std::string svMapname = (*Game::sv_mapname)->current.string;

		if (svMapname.empty())
		{
			Logger::Print(Game::CON_CHANNEL_SERVER, "mapname dvar is empty! Defaulting to mp_afghan\n");
			svMapname = "mp_afghan"s;
		}

		ApplyMap(svMapname);
	}

	void MapRotation::ApplyRotation(RotationData& rotation)
	{
		assert(!rotation.empty());

		// Continue to apply gametype until a map is found
		std::size_t i = 0;
		while (i < rotation.getEntriesSize())
		{
			const auto& entry = rotation.getNextEntry();
			if (entry.first == "map"s)
			{
				Logger::Print("Loading new map: '{}'", entry.second);
				ApplyMap(entry.second);

				// Map was found so we exit the loop
				break;
			}

			if (entry.first == "gametype"s)
			{
				Logger::Print("Applying new gametype: '{}'", entry.second);
				ApplyGametype(entry.second);
			}

			++i;
		}

		if (i == rotation.getEntriesSize())
		{
			Logger::PrintError(Game::CON_CHANNEL_ERROR, "Map rotation does not contain any map. Restarting\n");
			RestartCurrentMap();
		}
	}

	void MapRotation::ApplyMapRotationCurrent(const std::string& data)
	{
		assert(!data.empty());

		// Ook, ook, eek
		Logger::Warning(Game::CON_CHANNEL_SERVER, "You are using deprecated {}", (*Game::sv_mapRotationCurrent)->name);

		RotationData rotationCurrent;
		try
		{
			Logger::Debug("Parsing {}", (*Game::sv_mapRotationCurrent)->name);
			rotationCurrent.parse(data);
		}
		catch (const std::exception& ex)
		{
			Logger::PrintError(Game::CON_CHANNEL_ERROR, "{}: {} contains invalid data!\n", ex.what(), (*Game::sv_mapRotationCurrent)->name);
		}

		Game::Dvar_SetString(*Game::sv_mapRotationCurrent, "");

		if (rotationCurrent.empty())
		{
			Logger::Print(Game::CON_CHANNEL_SERVER, "{} is empty or contains invalid data. Restarting map\n", (*Game::sv_mapRotationCurrent)->name);
			RestartCurrentMap();
			return;
		}

		ApplyRotation(rotationCurrent);
	}

	void MapRotation::SetNextMap(RotationData& rotation)
	{
		assert(!rotation.empty());

		const auto& entry = rotation.peekNextEntry();
		if (entry.first == "map"s)
		{
			SVNextMap.set(entry.second);
		}
		else
		{
			ClearNextMap();
		}
	}

	void MapRotation::SetNextMap(const char* value)
	{
		assert(value);
		SVNextMap.set(value);
	}

	void MapRotation::ClearNextMap()
	{
		SVNextMap.set("");
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
			Logger::Debug("Map rotation was not randomized");
		}
	}

	void MapRotation::SV_MapRotate_f()
	{
		if (!ShouldRotate())
		{
			return;
		}

		Logger::Print(Game::CON_CHANNEL_SERVER, "Rotating map...\n");

		// This takes priority because of backwards compatibility
		const std::string mapRotationCurrent = (*Game::sv_mapRotationCurrent)->current.string;
		if (!mapRotationCurrent.empty())
		{
			Logger::Debug("Applying {}", (*Game::sv_mapRotationCurrent)->name);
			ApplyMapRotationCurrent(mapRotationCurrent);
			ClearNextMap();
			return;
		}

		LoadMapRotation();
		if (DedicatedRotation.empty())
		{
			Logger::Print(Game::CON_CHANNEL_SERVER, "{} is empty or contains invalid data. Restarting map\n", (*Game::sv_mapRotation)->name);
			RestartCurrentMap();
			SetNextMap("map_restart");
			return;
		}

		RandomizeMapRotation();

		ApplyRotation(DedicatedRotation);
		SetNextMap(DedicatedRotation);
	}

	void MapRotation::RegisterMapRotationDvars()
	{
		SVRandomMapRotation = Dvar::Register<bool>("sv_randomMapRotation", false, Game::DVAR_ARCHIVE, "Randomize map rotation when true");
		SVDontRotate = Dvar::Register<bool>("sv_dontRotate", false, Game::DVAR_NONE, "Do not perform map rotation");
		SVNextMap = Dvar::Register<const char*>("sv_nextMap", "", Game::DVAR_SERVERINFO, "");
	}

	MapRotation::MapRotation()
	{
		AddMapRotationCommands();
		Utils::Hook::Set<void(*)()>(0x4152E8, SV_MapRotate_f);

		Events::OnDvarInit(RegisterMapRotationDvars);
	}

	bool MapRotation::unitTest()
	{
		RotationData rotation;

		Logger::Debug("Testing map rotation parsing...");

		const auto* normal = "map mp_highrise map mp_terminal map mp_firingrange map mp_trailerpark gametype dm map mp_shipment_long";

		try
		{
			DedicatedRotation.parse(normal);
		}
		catch (const std::exception& ex)
		{
			Logger::PrintError(Game::CON_CHANNEL_ERROR, "{}: parsing of 'normal' failed\n", ex.what());
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
