#include <STDInclude.hpp>
#include "Field.hpp"

namespace Components::GSC
{
	std::unordered_map<std::uint16_t, Field::EntField> Field::CustomEntityFields;
	std::unordered_map<std::uint16_t, Field::ClientFields> Field::CustomClientFields;

	void Field::AddEntityField(const char* name, const ScriptCallbackEnt& setter, const ScriptCallbackEnt& getter)
	{
		static std::uint16_t fieldOffsetStart = 15; // fields count
		assert((fieldOffsetStart & Game::ENTFIELD_MASK) == Game::ENTFIELD_ENTITY);

		CustomEntityFields[fieldOffsetStart] = { name, fieldOffsetStart, setter, getter };
		++fieldOffsetStart;
	}

	void Field::AddClientField(const char* name, const ScriptCallbackClient& setter, const ScriptCallbackClient& getter)
	{
		static std::uint16_t fieldOffsetStart = 21; // fields count
		assert((fieldOffsetStart & Game::ENTFIELD_MASK) == Game::ENTFIELD_ENTITY);

		const auto offset = fieldOffsetStart | Game::ENTFIELD_CLIENT; // This is how client field's offset is calculated

		// Use 'index' in 'array' as map key. It will be used later in Scr_SetObjectFieldStub
		CustomClientFields[fieldOffsetStart] = { name, offset, setter, getter };
		++fieldOffsetStart;
	}

	void Field::GScr_AddFieldsForEntityStub()
	{
		for (const auto& field : CustomEntityFields | std::views::values)
		{
			Game::Scr_AddClassField(Game::ClassNum::CLASS_NUM_ENTITY, field.name, field.ofs);
		}

		Utils::Hook::Call<void()>(0x4A7CF0)(); // GScr_AddFieldsForClient

		for (const auto& field : CustomClientFields | std::views::values)
		{
			Game::Scr_AddClassField(Game::ClassNum::CLASS_NUM_ENTITY, field.name, field.ofs);
		}
	}

	// Because some functions are inlined we have to hook this function instead of Scr_SetEntityField
	int Field::Scr_SetObjectFieldStub(const unsigned int classnum, const int entnum, const int offset)
	{
		if (classnum == Game::ClassNum::CLASS_NUM_ENTITY)
		{
			const auto entity_offset = static_cast<std::uint16_t>(offset);
			if (const auto itr = CustomEntityFields.find(entity_offset); itr != CustomEntityFields.end())
			{
				itr->second.setter(&Game::g_entities[entnum], offset);
				return 1;
			}
		}

		// No custom generic field was found, let the game handle it
		return Game::Scr_SetObjectField(classnum, entnum, offset);
	}

	// Offset was already converted to array 'index' following binop offset & ~Game::ENTFIELD_MASK
	void Field::Scr_SetClientFieldStub(Game::gclient_s* client, const int offset)
	{
		const auto client_offset = static_cast<std::uint16_t>(offset);
		if (const auto itr = CustomClientFields.find(client_offset); itr != CustomClientFields.end())
		{
			itr->second.setter(nullptr, client, &itr->second);
			return;
		}

		// No custom field client was found, let the game handle it
		Game::Scr_SetClientField(client, offset);
	}

	void Field::Scr_GetEntityFieldStub(const int entnum, int offset)
	{
		if ((offset & Game::ENTFIELD_MASK) == Game::ENTFIELD_CLIENT)
		{
			// If we have a ENTFIELD_CLIENT offset we need to check g_entity is actually a fully connected client
			if (Game::g_entities[entnum].client != nullptr)
			{
				const auto client_offset = static_cast<std::uint16_t>(offset & ~Game::ENTFIELD_MASK);
				if (const auto itr = CustomClientFields.find(client_offset); itr != CustomClientFields.end())
				{
					// Game functions probably don't ever need to use the reference to client_fields_s...
					itr->second.getter(&Game::svs_clients[entnum], Game::g_entities[entnum].client, &itr->second);
					return;
				}
			}
		}

		// Regular entity offsets can be searched directly in our custom handler
		const auto entity_offset = static_cast<std::uint16_t>(offset);

		if (const auto itr = CustomEntityFields.find(entity_offset); itr != CustomEntityFields.end())
		{
			itr->second.getter(&Game::g_entities[entnum], offset);
			return;
		}

		// No custom generic field was found, let the game handle it
		Game::Scr_GetEntityField(entnum, offset);
	}


	void Field::AddEntityFields()
	{
		AddEntityField("entityflags",
			[](Game::gentity_s* ent, [[maybe_unused]] int offset)
			{
				ent->flags = Game::Scr_GetInt(0);
			},
			[](Game::gentity_s* ent, [[maybe_unused]] int offset)
			{
				Game::Scr_AddInt(ent->flags);
			}
		);
	}

	void Field::AddClientFields()
	{
		AddClientField("clientflags",
			[]([[maybe_unused]] Game::client_s* client, Game::gclient_s* pSelf, [[maybe_unused]] const ClientFields* pField)
			{
				pSelf->flags = Game::Scr_GetInt(0);
			},
			[]([[maybe_unused]] Game::client_s* client, Game::gclient_s* pSelf, [[maybe_unused]] const ClientFields* pField)
			{
				Game::Scr_AddInt(pSelf->flags);
			}
		);

		AddClientField("ping",
			[]([[maybe_unused]] Game::client_s* client, [[maybe_unused]] Game::gclient_s* pSelf, [[maybe_unused]] const ClientFields* pField)
			{
			},
			[]([[maybe_unused]] Game::client_s* client, [[maybe_unused]] Game::gclient_s* pSelf, [[maybe_unused]] const ClientFields* pField)
			{
				Game::Scr_AddInt(client->ping);
			}
		);

		AddClientField("address",
			[]([[maybe_unused]] Game::client_s* client, [[maybe_unused]] Game::gclient_s* pSelf, [[maybe_unused]] const ClientFields* pField)
			{
			},
			[]([[maybe_unused]] Game::client_s* client, [[maybe_unused]] Game::gclient_s* pSelf, [[maybe_unused]] const ClientFields* pField)
			{
				Game::Scr_AddString(Game::NET_AdrToString(client->header.netchan.remoteAddress));
			}
		);
	}

	Field::Field()
	{
		AddEntityFields();
		AddClientFields();

		Utils::Hook(0x4EC721, GScr_AddFieldsForEntityStub, HOOK_CALL).install()->quick(); // GScr_AddFieldsForEntity

		Utils::Hook(0x41BED2, Scr_SetObjectFieldStub, HOOK_CALL).install()->quick(); // SetEntityFieldValue
		Utils::Hook(0x5FBF01, Scr_SetClientFieldStub, HOOK_CALL).install()->quick(); // Scr_SetObjectField
		Utils::Hook(0x4FF413, Scr_GetEntityFieldStub, HOOK_CALL).install()->quick(); // Scr_GetObjectField
	}
}
