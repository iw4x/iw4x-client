#include <STDInclude.hpp>

#include "Metrics.hpp"



namespace Components
{
	rapidjson::Value Metrics::packetsBeingSent[MAX_PACKETS_AT_ONCE]{};
	std::vector<rapidjson::Value> Metrics::packetsToSend{};
	size_t Metrics::awaitingPacketCount = 0;

	std::mutex Metrics::threadMutex{};

	void Metrics::CopyPackets()
	{
		std::lock_guard _(threadMutex);
		if (packetsToSend.size() > 0)
		{
			for (size_t i = awaitingPacketCount; i < std::min(MAX_PACKETS_AT_ONCE, packetsToSend.size()); i++)
			{
				packetsBeingSent[i] = packetsToSend[0];
				packetsToSend.erase(packetsToSend.begin());
				awaitingPacketCount++;
			}
		}
	}

	void Metrics::DispatchPackets()
	{

	}

	void Metrics::Main()
	{
		while (true)
		{
			std::this_thread::sleep_for(100ms);
			CopyPackets();
			DispatchPackets();
		}
	}

	Metrics::Metrics()
	{
		// Hook jmp 4477A0
		// Hook jmp 463350

		auto thread = std::thread(Main);
		thread.detach();
	}

	rapidjson::Value Metrics::PlayerEntity::Serialize(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator)
	{
		rapidjson::Value obj = Player::Serialize(allocator);

		obj.AddMember("position", position.Serialize(allocator), allocator);
		obj.AddMember("forward", forward.Serialize(allocator), allocator);
		obj.AddMember("angles", angles.Serialize(allocator), allocator);

		obj.AddMember("health", health, allocator);

		{
			const std::string& teamName = Game::CG_GetTeamName(team);
			obj.AddMember("team", rapidjson::Value(teamName.c_str(), teamName.size(), allocator), allocator);
		}

		obj.AddMember("playerCardIcon", playerCardIcon, allocator);
		obj.AddMember("playerCardTitle", playerCardTitle, allocator);

		{
			std::vector<std::string> specialties{};
			for (size_t i = 0; i < Game::perksEnum::PERK_COUNT; i++)
			{
				//if (Game::BG_HasPerk(this->perks, static_cast<Game::perksEnum>(i)))
				{
					// 0x795B00 => perk names
					const char* perkName = *reinterpret_cast<const char**>(0x795B00 + i * sizeof(const char*));
					specialties.push_back(perkName);
				}
			}

			rapidjson::Value specialtiesJson = rapidjson::Value(rapidjson::kArrayType);
			for (const auto& specialty : specialties)
			{
				specialtiesJson.PushBack(rapidjson::Value(specialty.c_str(), specialty.size(), allocator), allocator);
			}

			obj.AddMember("perks", specialtiesJson, allocator);
		}

		return obj;
	}

	rapidjson::Value Metrics::Angles::Serialize(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator)
	{
		rapidjson::Value obj(rapidjson::kObjectType);

		obj.AddMember("yaw", angles[0], allocator);
		obj.AddMember("pitch", angles[1], allocator);
		obj.AddMember("roll", angles[2], allocator);

		return obj;
	}

	rapidjson::Value Metrics::Position::Serialize(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator)
	{
		rapidjson::Value obj(rapidjson::kObjectType);

		obj.AddMember("x", position[0], allocator);
		obj.AddMember("y", position[1], allocator);
		obj.AddMember("z", position[2], allocator);

		return obj;
	}

	rapidjson::Value Metrics::Player::Serialize(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator)
	{
		rapidjson::Value obj(rapidjson::kObjectType);

		obj.AddMember("name", rapidjson::Value(name.c_str(), name.size(), allocator), allocator);
		obj.AddMember("playerGuid", rapidjson::Value(playerGuid, ARRAYSIZE(playerGuid), allocator), allocator);

		std::string stringId = std::to_string(steamID);

		obj.AddMember("steamID", rapidjson::Value(stringId.c_str(), stringId.size(), allocator), allocator);

		obj.AddMember("ping", ping, allocator);

		return obj;
	}

	rapidjson::Value Metrics::Context::Serialize(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator)
	{
		rapidjson::Value obj(rapidjson::kObjectType);

		obj.AddMember("mapName", rapidjson::Value(mapName.c_str(), mapName.size(), allocator), allocator);
		obj.AddMember("gametype", rapidjson::Value(gametype.c_str(), gametype.size(), allocator), allocator);
		obj.AddMember("serverId", serverId, allocator);

		{
			std::string serverTimeStr = std::to_string(serverTime);
			obj.AddMember("serverTimeStr", rapidjson::Value(serverTimeStr.c_str(), serverTimeStr.size(), allocator), allocator);
		}

		obj.AddMember("fsGame", rapidjson::Value(fsGame.c_str(), fsGame.size(), allocator), allocator);

		return obj;
	}
}
