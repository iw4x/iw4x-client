#pragma once

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

#define MAX_PACKETS_AT_ONCE static_cast<size_t>(8)

namespace Components
{
	class Metrics : public Component
	{

	public:

		class SerializableData
		{
		public:
			virtual rapidjson::Value Serialize(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) = 0;
		};

		class Context : public SerializableData
		{
		public:
			std::string mapName;
			std::string gametype;
			uint32_t serverId;
			uint64_t serverTime;
			std::string fsGame;


		public:
			rapidjson::Value Serialize(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator);

			Context()
			{
				mapName = (*Game::sv_mapname)->current.string;
				gametype = (*Game::sv_gametype)->current.string;
				serverId = *Game::sv_serverId_value;
				serverTime = *Game::svs_time;
				fsGame = (*Game::fs_gameDirVar)->current.string;
			}

		};

		class EventData : public SerializableData
		{
		public:
			Context context;

			virtual std::string GetType() = 0;

			std::string ToString()
			{
				rapidjson::Document output(rapidjson::kObjectType);
				auto& allocator = output.GetAllocator();

				output.AddMember("type", GetType(), allocator);
				output.AddMember("context", context.Serialize(allocator), allocator);
				output.AddMember("data", Serialize(allocator), allocator);

				// Write to disk
				rapidjson::StringBuffer buff;
				rapidjson::PrettyWriter<
					/*typename OutputStream  */ rapidjson::StringBuffer,
					/*typename SourceEncoding*/ rapidjson::UTF8<>,
					/*typename TargetEncoding*/ rapidjson::UTF8<>,
					/*typename StackAllocator*/ rapidjson::CrtAllocator,
					/*unsigned writeFlags*/ rapidjson::kWriteNanAndInfNullFlag | rapidjson::kWriteNanAndInfFlag>
					writer(buff);
				writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);

				output.Accept(writer);

				return buff.GetString();
			}
		};

		struct Player : public SerializableData
		{
			std::string name;
			char playerGuid[17]{};
			uint64_t steamID{};
			int ping{};

			virtual rapidjson::Value Serialize(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) override;;

			Player(Game::gentity_s* entity)
			{
				if (entity->client)
				{
					name = entity->client->sess.cs.name;
					const char* guid = Utils::Hook::Call<const char* (int index)>(0x4477A0)(entity->client->sess.cs.clientIndex);
					size_t length = strnlen(guid, sizeof(playerGuid));
					std::memcpy(playerGuid, guid, length);

					for (int i = 0; i < *Game::svs_clientCount; i++)
					{
						const auto client = &Game::svs_clients[i];
						if (client->gentity == entity)
						{
							steamID = client->steamID;
							ping = client->ping;
							break;
						}
					}
				}
			};
		};

		struct Position : public SerializableData
		{
			Game::vec3_t position;

			rapidjson::Value Serialize(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) override;

			Position(const Game::vec3_t& position)
			{
				std::memcpy(this->position, position, sizeof(position));
			};

			Position() {};
		};

		struct Angles : public SerializableData
		{
			Game::vec3_t angles;

			rapidjson::Value Serialize(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) override;

			Angles(const Game::vec3_t& angles)
			{
				std::memcpy(this->angles, angles, sizeof(angles));
			};

			Angles() {};
		};

		struct PlayerEntity : public Player
		{
			Position position;
			Position forward;
			Angles angles;
			uint32_t health;
			Game::team_t team;
			uint32_t playerCardIcon;
			uint32_t playerCardTitle;
			uint32_t perks[2];

			virtual rapidjson::Value Serialize(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) override;

			PlayerEntity(Game::gentity_s* entity) : Player(entity)
			{
				position = Position(entity->r.currentOrigin);
				angles = Angles(entity->r.currentAngles);

				Game::vec3_t forwardVector{};
				Game::AngleVectors(entity->r.currentOrigin, forwardVector, nullptr, nullptr);

				forward = Position(forwardVector);

				health = static_cast<uint32_t>((entity->health / static_cast<float>(entity->maxHealth)) * 100);

				team = static_cast<Game::team_t>(entity->team);

				if (entity->client)
				{
					playerCardIcon = entity->client->sess.cs.playerCardIcon;
					playerCardTitle = entity->client->sess.cs.playerCardTitle;

					std::memcpy(perks, entity->client->sess.cs.perks, sizeof(entity->client->sess.cs.perks));
				}
			}
		};

		class PlayerKilledEvent : public EventData
		{
			PlayerEntity attackee;
			PlayerEntity attacker;
			Game::meansOfDeath_t mod;
			Game::hitLocation_t hitLocation;
			int weaponIndex;
			int damage;
			int damageFlags;
			Position vector;

			virtual std::string GetType() override { return "PlayerKilled"; };
		};

		class PlayerDamagedEvent : public PlayerKilledEvent
		{
			Position point;

			virtual std::string GetType() override { return "PlayerDamaged"; };
		};


		Metrics();

	private:
		static rapidjson::Value packetsBeingSent[MAX_PACKETS_AT_ONCE];
		static std::vector<rapidjson::Value> packetsToSend;
		static std::mutex threadMutex;
		static size_t awaitingPacketCount;
		static bool busy;

		static void DispatchPackets();
		static void CopyPackets();

		static void Main();

		static bool BG_HasPerk(const unsigned int* perks, const Game::perksEnum perkIndex)
		{
			assert(perks);
			AssertIn(perkIndex, Game::PERK_COUNT);

			const std::size_t arrayIndex = perkIndex >> 5;
			AssertIn(arrayIndex, Game::PERK_ARRAY_COUNT);

			constexpr std::size_t BIT_MASK_SIZE = 31; // 0x1F
			const auto bitPos = perkIndex & BIT_MASK_SIZE;
			const auto bitMask = 1 << bitPos;

			return perks[arrayIndex] & bitMask;
		}

	};
}
