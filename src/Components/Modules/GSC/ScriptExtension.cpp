#include <STDInclude.hpp>
#include "ScriptExtension.hpp"
#include "Script.hpp"

namespace Components
{
	std::unordered_map<std::uint16_t, Game::ent_field_t> ScriptExtension::CustomEntityFields;
	std::unordered_map<std::uint16_t, Game::client_fields_s> ScriptExtension::CustomClientFields;

	std::unordered_map<const char*, const char*> ScriptExtension::ReplacedFunctions;
	const char* ScriptExtension::ReplacedPos = nullptr;

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
	void ScriptExtension::Scr_SetClientFieldStub(Game::gclient_s* client, int offset)
	{
		const auto client_offset = static_cast<std::uint16_t>(offset);
		if (const auto itr = CustomClientFields.find(client_offset); itr != CustomClientFields.end())
		{
			itr->second.setter(client, &itr->second);
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
				if (const auto itr = CustomClientFields.find(client_offset); itr != CustomClientFields.end())
				{
					// Game functions probably don't ever need to use the reference to client_fields_s...
					itr->second.getter(Game::g_entities[entnum].client, &itr->second);
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

	const char* ScriptExtension::GetCodePosForParam(int index)
	{
		if (static_cast<unsigned int>(index) >= Game::scrVmPub->outparamcount)
		{
			Game::Scr_ParamError(static_cast<unsigned int>(index), "^1GetCodePosForParam: Index is out of range!\n");
			return "";
		}

		const auto* value = &Game::scrVmPub->top[-index];

		if (value->type != Game::VAR_FUNCTION)
		{
			Game::Scr_ParamError(static_cast<unsigned int>(index), "^1GetCodePosForParam: Expects a function as parameter!\n");
			return "";
		}

		return value->u.codePosValue;
	}

	void ScriptExtension::GetReplacedPos(const char* pos)
	{
		if (ReplacedFunctions.contains(pos))
		{
			ReplacedPos = ReplacedFunctions[pos];
		}
	}

	void ScriptExtension::SetReplacedPos(const char* what, const char* with)
	{
		if (!*what || !*with)
		{
			Logger::Warning(Game::CON_CHANNEL_SCRIPT, "Invalid parameters passed to ReplacedFunctions\n");
			return;
		}

		if (ReplacedFunctions.contains(what))
		{
			Logger::Warning(Game::CON_CHANNEL_SCRIPT, "ReplacedFunctions already contains codePosValue for a function\n");
		}

		ReplacedFunctions[what] = with;
	}

	__declspec(naked) void ScriptExtension::VMExecuteInternalStub()
	{
		__asm
		{
			pushad

			push edx
			call GetReplacedPos

			pop edx
			popad

			cmp ReplacedPos, 0
			jne SetPos

			movzx eax, byte ptr [edx]
			inc edx

		Loc1:
			cmp eax, 0x8B

			push ecx

			mov ecx, 0x2045094
			mov [ecx], eax

			mov ecx, 0x2040CD4
			mov [ecx], edx

			pop ecx

			push 0x61E944
			ret

		SetPos:
			mov edx, ReplacedPos
			mov ReplacedPos, 0

			movzx eax, byte ptr [edx]
			inc edx

			jmp Loc1
		}
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
			const auto* str = Game::Scr_GetString(0);
			const auto* suffix = Game::Scr_GetString(1);

			if (str == nullptr || suffix == nullptr)
			{
				Game::Scr_Error("^1IsEndStr: Illegal parameters!\n");
				return;
			}

			Game::Scr_AddBool(Utils::String::EndsWith(str, suffix));
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

		// Func present on IW5
		Script::AddFunction("CastFloat", [] // gsc: CastFloat()
		{
			switch (Game::Scr_GetType(0))
			{
			case Game::VAR_STRING:
				Game::Scr_AddFloat(static_cast<float>(std::atof(Game::Scr_GetString(0))));
				break;
			case Game::VAR_FLOAT:
				Game::Scr_AddFloat(Game::Scr_GetFloat(0));
				break;
			case Game::VAR_INTEGER:
				Game::Scr_AddFloat(static_cast<float>(Game::Scr_GetInt(0)));
				break;
			default:
				Game::Scr_ParamError(0, Utils::String::VA("cannot cast %s to float", Game::Scr_GetTypeName(0)));
				break;
			}
		});

		Script::AddFunction("Strtol", [] // gsc: Strtol(<string>, <int>)
		{
			const auto* input = Game::Scr_GetString(0);
			const auto base = Game::Scr_GetInt(1);

			char* end;
			const auto result = std::strtol(input, &end, base);
			if (input == end)
			{
				Game::Scr_ParamError(0, "cannot cast string to int");
			}

			Game::Scr_AddInt(result);
		});

		Script::AddFunction("ReplaceFunc", [] // gsc: ReplaceFunc(<function>, <function>)
		{
			if (Game::Scr_GetNumParam() != 2)
			{
				Game::Scr_Error("^1ReplaceFunc: Needs two parameters!\n");
				return;
			}

			const auto what = GetCodePosForParam(0);
			const auto with = GetCodePosForParam(1);

			SetReplacedPos(what, with);
		});


		Script::AddFunction("GetSystemMilliseconds", [] // gsc: GetSystemMilliseconds()
		{
			SYSTEMTIME time;
			GetSystemTime(&time);

			Game::Scr_AddInt(time.wMilliseconds);
		});

		Script::AddFunction("Exec", [] // gsc: Exec(<string>)
		{
			const auto* str = Game::Scr_GetString(0);
			if (!str)
			{
				Game::Scr_ParamError(0, "^1Exec: Illegal parameter!\n");
				return;
			}

			Command::Execute(str, false);
		});

		// Allow printing to the console even when developer is 0
		Script::AddFunction("PrintConsole", [] // gsc: PrintConsole(<string>)
		{
			for (std::size_t i = 0; i < Game::Scr_GetNumParam(); ++i)
			{
				const auto* str = Game::Scr_GetString(i);
				if (!str)
				{
					Game::Scr_ParamError(i, "^1PrintConsole: Illegal parameter!\n");
					return;
				}

				Logger::Print(Game::level->scriptPrintChannel, "{}", str);
			}
		});		
	}

	void ScriptExtension::AddMethods()
	{
		// ScriptExtension methods
		Script::AddMethod("GetIp", [](const Game::scr_entref_t entref) // gsc: self GetIp()
		{
			const auto* ent = Game::GetPlayerEntity(entref);
			const auto* client = Script::GetClient(ent);

			std::string ip = Game::NET_AdrToString(client->header.netchan.remoteAddress);

			if (const auto pos = ip.find_first_of(":"); pos != std::string::npos)
				ip.erase(ip.begin() + pos, ip.end()); // Erase port

			Game::Scr_AddString(ip.data());
		});

		Script::AddMethod("GetPing", [](const Game::scr_entref_t entref) // gsc: self GetPing()
		{
			const auto* ent = Game::GetPlayerEntity(entref);
			const auto* client = Script::GetClient(ent);

			Game::Scr_AddInt(client->ping);
		});

		Script::AddMethod("SetPing", [](const Game::scr_entref_t entref) // gsc: self SetPing(<int>)
		{
			auto ping = Game::Scr_GetInt(0);

			ping = std::clamp(ping, 0, 999);

			const auto* ent = Game::GetPlayerEntity(entref);
			auto* client = Script::GetClient(ent);

			client->ping = ping;
		});

		// PlayerCmd_AreControlsFrozen GSC function from Black Ops 2
		Script::AddMethod("AreControlsFrozen", [](Game::scr_entref_t entref) // Usage: self AreControlsFrozen();
		{
			const auto* ent = Script::Scr_GetPlayerEntity(entref);
			Game::Scr_AddBool((ent->client->flags & Game::PF_FROZEN) != 0);
		});
	}

	void ScriptExtension::AddEntityFields()
	{
		AddEntityField("entityflags", Game::F_INT,
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
		AddClientField("clientflags", Game::F_INT,
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

		Utils::Hook(0x4EC721, GScr_AddFieldsForEntityStub, HOOK_CALL).install()->quick(); // GScr_AddFieldsForEntity

		Utils::Hook(0x41BED2, Scr_SetObjectFieldStub, HOOK_CALL).install()->quick(); // SetEntityFieldValue
		Utils::Hook(0x5FBF01, Scr_SetClientFieldStub, HOOK_CALL).install()->quick(); // Scr_SetObjectField
		Utils::Hook(0x4FF413, Scr_GetEntityFieldStub, HOOK_CALL).install()->quick(); // Scr_GetObjectField

		Utils::Hook(0x61E92E, VMExecuteInternalStub, HOOK_JUMP).install()->quick();
		Utils::Hook::Nop(0x61E933, 1);

		Events::OnVMShutdown([]
		{
			ReplacedFunctions.clear();
		});
	}
}
