#pragma once

namespace Components
{
	class ScriptExtension : public Component
	{
	public:
		ScriptExtension();

		static void AddEntityField(const char* name, Game::fieldtype_t type, const Game::ScriptCallbackEnt& setter, const Game::ScriptCallbackEnt& getter);
		static void AddClientField(const char* name, Game::fieldtype_t type, const Game::ScriptCallbackClient& setter, const Game::ScriptCallbackClient& getter);

	private:
		static std::unordered_map<std::uint16_t, Game::ent_field_t> CustomEntityFields;
		static std::unordered_map<std::uint16_t, Game::client_fields_s> CustomClientFields;

		static void GScr_AddFieldsForEntityStub();

		// Two hooks because it makes our code cleaner (luckily functions were not inlined)
		static int Scr_SetObjectFieldStub(unsigned int classnum, int entnum, int offset);
		static void Scr_SetClientFieldStub(Game::gclient_s* client, int offset);

		// One hook because functions were inlined
		static void Scr_GetEntityFieldStub(int entnum, int offset);

		static void AddFunctions();
		static void AddMethods();
		static void AddEntityFields();
		static void AddClientFields();
		static void Scr_TableLookupIStringByRow();
	};
}
