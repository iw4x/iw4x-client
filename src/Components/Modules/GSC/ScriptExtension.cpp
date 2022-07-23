#include <STDInclude.hpp>
#include "ScriptExtension.hpp"
#include "Script.hpp"

namespace Components
{
	std::unordered_map<std::uint16_t, Game::ent_field_t> ScriptExtension::CustomEntityFields;
	std::unordered_map<std::uint16_t, Game::client_fields_s> ScriptExtension::CustomClientFields;

	void ScriptExtension::AddEntityField(const char* name, Game::fieldtype_t type,
		const Game::ScriptCallbackEnt& setter, const Game::ScriptCallbackEnt& getter)
	{
		static std::uint16_t fieldOffsetStart = 15; // fields count
		assert((fieldOffsetStart & Game::ENTFIELD_MASK) == Game::ENTFIELD_ENTITY);

		CustomEntityFields[fieldOffsetStart] = {name, fieldOffsetStart, type, setter, getter};
		++fieldOffsetStart;
	}

	void ScriptExtension::AddClientField(const char* name, Game::fieldtype_t type,
		const Game::ScriptCallbackClient& setter, const Game::ScriptCallbackClient& getter)
	{
		static std::uint16_t fieldOffsetStart = 21; // fields count
		assert((fieldOffsetStart & Game::ENTFIELD_MASK) == Game::ENTFIELD_ENTITY);

		const auto offset = fieldOffsetStart | Game::ENTFIELD_CLIENT; // This is how client field's offset is calculated

		// Use 'index' in 'array' as map key. It will be used later in Scr_SetObjectFieldStub
		CustomClientFields[fieldOffsetStart] = {name, offset, type, setter, getter};
		++fieldOffsetStart;
	}

	void ScriptExtension::GScr_AddFieldsForEntityStub()
	{
		for (const auto& [offset, field] : CustomEntityFields)
		{
			Game::Scr_AddClassField(Game::ClassNum::CLASS_NUM_ENTITY, field.name, field.ofs);
		}

		Utils::Hook::Call<void()>(0x4A7CF0)(); // GScr_AddFieldsForClient

		for (const auto& [offset, field] : CustomClientFields)
		{
			Game::Scr_AddClassField(Game::ClassNum::CLASS_NUM_ENTITY, field.name, field.ofs);
		}
	}

	// Because some functions are inlined we have to hook this function instead of Scr_SetEntityField
	int ScriptExtension::Scr_SetObjectFieldStub(unsigned int classnum, int entnum, int offset)
	{
		if (classnum == Game::ClassNum::CLASS_NUM_ENTITY)
		{
			const auto entity_offset = static_cast<std::uint16_t>(offset);

			const auto got =CustomEntityFields.find(entity_offset);
			if (got != CustomEntityFields.end())
			{
				got->second.setter(&Game::g_entities[entnum], offset);
				return 1;
			}
		}

		// No custom generic field was found, let the game handle it
		return Game::Scr_SetObjectField(classnum, entnum, offset);
	}

	// Offset was already converted to array 'index' following binop offset & ~Game::ENTFIELD_MASK
	void ScriptExtension::Scr_SetClientFieldStub(Game::gclient_s* client, int offset)
	{
		const auto client_offset = static_cast<std::uint16_t>(offset);

		const auto got = CustomClientFields.find(client_offset);
		if (got != CustomClientFields.end())
		{
			got->second.setter(client, &got->second);
			return;
		}

		// No custom field client was found, let the game handle it
		Game::Scr_SetClientField(client, offset);
	}

	void ScriptExtension::Scr_GetEntityFieldStub(int entnum, int offset)
	{
		if ((offset & Game::ENTFIELD_MASK) == Game::ENTFIELD_CLIENT)
		{
			// If we have a ENTFIELD_CLIENT offset we need to check g_entity is actually a fully connected client
			if (Game::g_entities[entnum].client != nullptr)
			{
				const auto client_offset = static_cast<std::uint16_t>(offset & ~Game::ENTFIELD_MASK);

				const auto got =CustomClientFields.find(client_offset);
				if (got != CustomClientFields.end())
				{
					// Game functions probably don't ever need to use the reference to client_fields_s...
					got->second.getter(Game::g_entities[entnum].client, &got->second);
					return;
				}
			}
		}

		// Regular entity offsets can be searched directly in our custom handler
		const auto entity_offset = static_cast<std::uint16_t>(offset);

		const auto got = CustomEntityFields.find(entity_offset);
		if (got != CustomEntityFields.end())
		{
			got->second.getter(&Game::g_entities[entnum], offset);
			return;
		}

		// No custom generic field was found, let the game handle it
		Game::Scr_GetEntityField(entnum, offset);
	}

	void ScriptExtension::AddFunctions()
	{
		// Misc functions
		Script::AddFunction("ToUpper", [] // gsc: ToUpper(<string>)
		{
			const auto scriptValue = Game::Scr_GetConstString(0);
			const auto* string = Game::SL_ConvertToString(scriptValue);

			char out[1024] = {0}; // 1024 is the max for a string in this SL system
			bool changed = false;

			size_t i = 0;
			while (i < sizeof(out))
			{
				const auto value = *string;
				const auto result = static_cast<char>(std::toupper(static_cast<unsigned char>(value)));
				out[i] = result;

				if (value != result)
					changed = true;

				if (result == '\0') // Finished converting string
					break;

				++string;
				++i;
			}

			// Null terminating character was overwritten 
			if (i >= sizeof(out))
			{
				Game::Scr_Error("string too long");
				return;
			}

			if (changed)
			{
				Game::Scr_AddString(out);
			}
			else
			{
				Game::SL_AddRefToString(scriptValue);
				Game::Scr_AddConstString(scriptValue);
				Game::SL_RemoveRefToString(scriptValue);
			}
		});

		// Func present on IW5
		Script::AddFunction("StrICmp", [] // gsc: StrICmp(<string>, <string>)
		{
			const auto value1 = Game::Scr_GetConstString(0);
			const auto value2 = Game::Scr_GetConstString(1);

			const auto result = _stricmp(Game::SL_ConvertToString(value1),
				Game::SL_ConvertToString(value2));

			Game::Scr_AddInt(result);
		});

		// Func present on IW5
		Script::AddFunction("IsEndStr", [] // gsc: IsEndStr(<string>, <string>)
		{
			const auto* s1 = Game::Scr_GetString(0);
			const auto* s2 = Game::Scr_GetString(1);

			if (s1 == nullptr || s2 == nullptr)
			{
				Game::Scr_Error("^1IsEndStr: Illegal parameters!\n");
				return;
			}

			Game::Scr_AddBool(Utils::String::EndsWith(s1, s2));
		});

		Script::AddFunction("IsArray", [] // gsc: IsArray(<object>)
		{
			auto type = Game::Scr_GetType(0);

			bool result;
			if (type == Game::VAR_POINTER)
			{
				type = Game::Scr_GetPointerType(0);
				assert(type >= Game::FIRST_OBJECT);
				result = (type == Game::VAR_ARRAY);
			}
			else
			{
				assert(type < Game::FIRST_OBJECT);
				result = false;
			}

			Game::Scr_AddBool(result);
		});
	}

	void ScriptExtension::AddMethods()
	{
		// ScriptExtension methods
		Script::AddMethod("GetIp", [](Game::scr_entref_t entref) // gsc: self GetIp()
		{
			const auto* ent = Game::GetPlayerEntity(entref);
			const auto* client = Script::GetClient(ent);

			std::string ip = Game::NET_AdrToString(client->netchan.remoteAddress);

			if (const auto pos = ip.find_first_of(":"); pos != std::string::npos)
				ip.erase(ip.begin() + pos, ip.end()); // Erase port

			Game::Scr_AddString(ip.data());
		});

		Script::AddMethod("GetPing", [](Game::scr_entref_t entref) // gsc: self GetPing()
		{
			const auto* ent = Game::GetPlayerEntity(entref);
			const auto* client = Script::GetClient(ent);

			Game::Scr_AddInt(client->ping);
		});

		Script::AddMethod("SetPing", [](Game::scr_entref_t entref) // gsc: self SetPing(<int>)
		{
			auto ping = Game::Scr_GetInt(0);

			ping = std::clamp(ping, 0, 999);

			const auto* ent = Game::GetPlayerEntity(entref);
			auto* client = Script::GetClient(ent);

			client->ping = static_cast<int16_t>(ping);
		});
	}

	void ScriptExtension::Scr_TableLookupIStringByRow()
	{
		if (Game::Scr_GetNumParam() < 3)
		{
			Game::Scr_Error("USAGE: tableLookupIStringByRow( filename, rowNum, returnValueColumnNum )\n");
			return;
		}

		const auto* fileName = Game::Scr_GetString(0);
		const auto rowNum = Game::Scr_GetInt(1);
		const auto returnValueColumnNum = Game::Scr_GetInt(2);

		const auto* table = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_STRINGTABLE, fileName).stringTable;

		if (table == nullptr)
		{
			Game::Scr_ParamError(0, Utils::String::VA("%s does not exist\n", fileName));
			return;
		}

		const auto* value = Game::StringTable_GetColumnValueForRow(table, rowNum, returnValueColumnNum);
		Game::Scr_AddIString(value);
	}

	void ScriptExtension::AddEntityFields()
	{
		AddEntityField("entityflags", Game::fieldtype_t::F_INT,
			[](Game::gentity_s* ent, [[maybe_unused]] int offset)
			{
				ent->flags = Game::Scr_GetInt(0);
			},
			[](Game::gentity_s* ent, [[maybe_unused]] int offset)
			{
				Game::Scr_AddInt(ent->flags);
			});
	}

	void ScriptExtension::AddClientFields()
	{
		AddClientField("clientflags", Game::fieldtype_t::F_INT,
			[](Game::gclient_s* pSelf, [[maybe_unused]] const Game::client_fields_s* pField)
			{
				pSelf->flags = Game::Scr_GetInt(0);
			},
			[](Game::gclient_s* pSelf, [[maybe_unused]] const Game::client_fields_s* pField)
			{
				Game::Scr_AddInt(pSelf->flags);
			});
	}

	ScriptExtension::ScriptExtension()
	{
		AddFunctions();
		AddMethods();
		AddEntityFields();
		AddClientFields();

		// Correct builtin function pointer
		Utils::Hook::Set<Game::BuiltinFunction>(0x79A90C, Scr_TableLookupIStringByRow);

		Utils::Hook(0x4EC721, GScr_AddFieldsForEntityStub, HOOK_CALL).install()->quick(); // GScr_AddFieldsForEntity

		Utils::Hook(0x41BED2, Scr_SetObjectFieldStub, HOOK_CALL).install()->quick(); // SetEntityFieldValue
		Utils::Hook(0x5FBF01, Scr_SetClientFieldStub, HOOK_CALL).install()->quick(); // Scr_SetObjectField
		Utils::Hook(0x4FF413, Scr_GetEntityFieldStub, HOOK_CALL).install()->quick(); // Scr_GetObjectField

		// Fix format string in Scr_RandomFloatRange
		Utils::Hook::Set<const char*>(0x5F10C6, "Scr_RandomFloatRange parms: %f %f ");
	}
}
