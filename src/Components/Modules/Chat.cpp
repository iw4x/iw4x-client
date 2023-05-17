#include <STDInclude.hpp>
#include "Chat.hpp"
#include "Events.hpp"
#include "PlayerName.hpp"
#include "TextRenderer.hpp"
#include "Voice.hpp"

#include "GSC/Script.hpp"
#include "GSC/ScriptExtension.hpp"

namespace Components
{
	Dvar::Var Chat::cg_chatWidth;
	Dvar::Var Chat::sv_disableChat;
	Dvar::Var Chat::sv_sayName;

	bool Chat::SendChat;

	Utils::Concurrency::Container<Chat::muteList> Chat::MutedList;
	const char* Chat::MutedListFile = "userraw/muted-users.json";

	bool Chat::CanAddCallback = true;
	std::vector<Scripting::Function> Chat::SayCallbacks;

	// Have only one instance of IW4x read/write the file
	std::unique_lock<Utils::NamedMutex> Chat::Lock()
	{
		static Utils::NamedMutex mutex{ "iw4x-mute-list-lock" };
		std::unique_lock lock{mutex};
		return lock;
	}

	const char* Chat::EvaluateSay(char* text, Game::gentity_s* player, int mode)
	{
		SendChat = true;

		const auto _0 = gsl::finally([]
		{
			CanAddCallback = true;
		});

		// Prevent callbacks from adding a new callback (would make the vector iterator invalid)
		CanAddCallback = false;

		// Chat messages sent through the console do not begin with \x15. In some cases it contains \x14
		auto msgIndex = 0;
		while (text[msgIndex] == '\x15' || text[msgIndex] == '\x14')
		{
			++msgIndex;
		}

		if (text[msgIndex] == '/')
		{
			SendChat = false;
			++msgIndex;
		}

		if (IsMuted(player))
		{
			SendChat = false;
			Game::SV_GameSendServerCommand(player - Game::g_entities, Game::SV_CMD_CAN_IGNORE, Utils::String::VA("%c \"You are muted\"", 0x65));
		}

		if (sv_disableChat.get<bool>())
		{
			SendChat = false;
			Game::SV_GameSendServerCommand(player - Game::g_entities, Game::SV_CMD_CAN_IGNORE, Utils::String::VA("%c \"Chat is disabled\"", 0x65));
		}

		// Message might be empty after the special characters or '/'
		if (text[msgIndex] == '\0')
		{
			SendChat = false;
			return text;
		}

		TextRenderer::StripMaterialTextIcons(text, text, std::strlen(text) + 1);
		Logger::Print("{}: {}\n", Game::svs_clients[player - Game::g_entities].name, (text + msgIndex));

		for (const auto& callback : SayCallbacks)
		{
			if (!ChatCallback(player, callback.getPos(), (text + msgIndex), mode))
			{
				SendChat = false;
			}
		}

		Game::Scr_AddEntity(player);
		Game::Scr_AddString(text + msgIndex);
		Game::Scr_NotifyLevel(static_cast<std::uint16_t>(Game::SL_GetString("say", 0)), 2);

		return text;
	}

	__declspec(naked) void Chat::PreSayStub()
	{
		__asm
		{
			mov eax, [esp + 0x100 + 0x10]

			push eax
			pushad

			push [esp + 0x100 + 0x30] // mode
			push [esp + 0x100 + 0x2C] // player
			push eax // text
			call EvaluateSay
			add esp, 0xC

			mov [esp + 0x20], eax
			popad
			pop eax

			mov [esp + 0x100 + 0x10], eax

			jmp PlayerName::CleanStrStub
		}
	}

	__declspec(naked) void Chat::PostSayStub()
	{
		__asm
		{
			// eax is used by the callee
			push eax

			xor eax, eax
			mov al, SendChat

			test al, al
			jnz return

			// Don't send the chat
			pop eax
			retn

		return:
			pop eax

			// Jump to the target
			push 5DF620h
			retn
		}
	}

	void Chat::CheckChatLineEnd(const char*& inputBuffer, char*& lineBuffer, float& len, const int chatHeight, const float chatWidth, char*& lastSpacePos, char*& lastFontIconPos, const int lastColor)
	{
		if (len > chatWidth)
		{
			if (lastSpacePos && lastSpacePos > lastFontIconPos)
			{
				inputBuffer += lastSpacePos - lineBuffer + 1;
				lineBuffer = lastSpacePos;
			}
			else if (lastFontIconPos)
			{
				inputBuffer += lastFontIconPos - lineBuffer;
				lineBuffer = lastFontIconPos;
			}

			*lineBuffer = 0;
			len = 0.0f;
			Game::cgsArray[0].teamChatMsgTimes[Game::cgsArray[0].teamChatPos % chatHeight] = Game::cgArray[0].time;

			Game::cgsArray[0].teamChatPos++;
			lineBuffer = Game::cgsArray[0].teamChatMsgs[Game::cgsArray[0].teamChatPos % chatHeight];
			lineBuffer[0] = '^';
			lineBuffer[1] = CharForColorIndex(lastColor);
			lineBuffer += 2;
			lastSpacePos = nullptr;
			lastFontIconPos = nullptr;
		}
	}

	void Chat::CG_AddToTeamChat(const char* text)
	{
		// Text can only be 150 characters maximum. This is bigger than the teamChatMsgs buffers with 160 characters
		// Therefore it is not needed to check for buffer lengths

		const auto chatHeight = (*Game::cg_chatHeight)->current.integer;
		const auto chatWidth = static_cast<float>(cg_chatWidth.get<int>());
		const auto chatTime = (*Game::cg_chatTime)->current.integer;
		if (chatHeight <= 0 || static_cast<unsigned>(chatHeight) > std::extent_v<decltype(Game::cgs_t::teamChatMsgs)> || chatWidth <= 0 || chatTime <= 0)
		{
			Game::cgsArray[0].teamLastChatPos = 0;
			Game::cgsArray[0].teamChatPos = 0;
			return;
		}

		TextRenderer::FontIconInfo fontIconInfo{};
		auto len = 0.0f;
		auto lastColor = static_cast<std::underlying_type_t<TextColor>>(TextColor::TEXT_COLOR_DEFAULT);
		char* lastSpace = nullptr;
		char* lastFontIcon = nullptr;
		char* p = Game::cgsArray[0].teamChatMsgs[Game::cgsArray[0].teamChatPos % chatHeight];
		p[0] = '\0';

		while (*text)
		{
			CheckChatLineEnd(text, p, len, chatHeight, chatWidth, lastSpace, lastFontIcon, lastColor);

			const char* fontIconEndPos = &text[1];
			if (text[0] == TextRenderer::FONT_ICON_SEPARATOR_CHARACTER && TextRenderer::IsFontIcon(fontIconEndPos, fontIconInfo))
			{
				// The game calculates width on a per character base. Since the width of a font icon is calculated based on the height of the font
				// which is roughly double as much as the average width of a character without an additional multiplier the calculated len of the font icon
				// would be less than it most likely would be rendered. Therefore apply a guessed 2.0f multiplier at this location which makes
				// the calculated width of a font icon roughly comparable to the width of an average character of the font.
				const auto normalizedFontIconWidth = TextRenderer::GetNormalizedFontIconWidth(fontIconInfo);
				const auto fontIconWidth = normalizedFontIconWidth * FONT_ICON_CHAT_WIDTH_CALCULATION_MULTIPLIER;
				len += fontIconWidth;

				lastFontIcon = p;
				for(; text < fontIconEndPos; text++)
				{
					p[0] = text[0];
					p++;
				}

				CheckChatLineEnd(text, p, len, chatHeight, chatWidth, lastSpace, lastFontIcon, lastColor);
			}
			else if (text[0] == '^' && text[1] != 0 && text[1] >= TextRenderer::COLOR_FIRST_CHAR && text[1] <= TextRenderer::COLOR_LAST_CHAR)
			{
				p[0] = '^';
				p[1] = text[1];
				lastColor = ColorIndexForChar(text[1]);
				p += 2;
				text += 2;
			}
			else
			{
				if (text[0] == ' ')
					lastSpace = p;
				*p++ = *text++;
				len += 1.0f;
			}
		}

		*p = 0;

		Game::cgsArray[0].teamChatMsgTimes[Game::cgsArray[0].teamChatPos % chatHeight] = Game::cgArray[0].time;

		Game::cgsArray[0].teamChatPos++;
		if (Game::cgsArray[0].teamChatPos - Game::cgsArray[0].teamLastChatPos > chatHeight)
		{
			Game::cgsArray[0].teamLastChatPos = Game::cgsArray[0].teamChatPos + 1 - chatHeight;
		}
	}

	__declspec(naked) void Chat::CG_AddToTeamChat_Stub()
	{
		__asm
		{
			pushad

			push ecx
			call CG_AddToTeamChat
			add esp, 4h

			popad
			ret
		}
	}

	bool Chat::IsMuted(const Game::gentity_s* ent)
	{
		const auto clientNum = ent - Game::g_entities;
		const auto xuid = Game::svs_clients[clientNum].steamID;

		const auto result = MutedList.access<bool>([&](const muteList& clients)
		{
			return clients.contains(xuid);
		});

		return result;
	}

	bool Chat::IsMuted(const Game::client_s* cl)
	{
		const auto clientNum = cl - Game::svs_clients;
		const auto xuid = Game::svs_clients[clientNum].steamID;

		const auto result = MutedList.access<bool>([&](const muteList& clients)
		{
			return clients.contains(xuid);
		});

		return result;
	}

	void Chat::MuteClient(const Game::client_s* client)
	{
		const auto xuid = client->steamID;
		MutedList.access([&](muteList& clients)
		{
			clients.insert(xuid);
			SaveMutedList(clients);
		});

		Logger::Print("{} was muted\n", client->name);
		Game::SV_GameSendServerCommand(client - Game::svs_clients, Game::SV_CMD_CAN_IGNORE, Utils::String::VA("%c \"You were muted\"", 0x65));
	}

	void Chat::UnmuteClient(const Game::client_s* client)
	{
		UnmuteInternal(client->steamID);

		Logger::Print("{} was unmuted\n", client->name);
		Game::SV_GameSendServerCommand(client - Game::svs_clients, Game::SV_CMD_CAN_IGNORE, Utils::String::VA("%c \"You were unmuted\"", 0x65));
	}

	void Chat::UnmuteInternal(const std::uint64_t id, bool everyone)
	{
		MutedList.access([&](muteList& clients)
		{
			if (everyone)
				clients.clear();
			else
				clients.erase(id);

			SaveMutedList(clients);
		});
	}

	void Chat::SaveMutedList(const muteList& list)
	{
		const auto _ = Lock();

		const nlohmann::json mutedUsers = nlohmann::json
		{
			{ "SteamID", list },
		};

		Utils::IO::WriteFile(MutedListFile, mutedUsers.dump());
	}

	void Chat::LoadMutedList()
	{
		const auto _ = Lock();

		const auto mutedUsers = Utils::IO::ReadFile(MutedListFile);
		if (mutedUsers.empty())
		{
			Logger::Debug("muted-users.json does not exist");
			return;
		}

		nlohmann::json mutedUsersData;
		try
		{
			mutedUsersData = nlohmann::json::parse(mutedUsers);
		}
		catch (const std::exception& ex)
		{
			Logger::PrintError(Game::CON_CHANNEL_ERROR, "JSON Parse Error: {}\n", ex.what());
			return;
		}

		if (!mutedUsersData.contains("SteamID"))
		{
			Logger::PrintError(Game::CON_CHANNEL_ERROR, "muted-users.json contains invalid data\n");
			return;
		}

		const auto& list = mutedUsersData["SteamID"];
		if (!list.is_array())
		{
			return;
		}

		MutedList.access([&](muteList& clients)
		{
			const nlohmann::json::array_t arr = list;
			for (auto& entry : arr)
			{
				if (entry.is_number_unsigned())
				{
					clients.insert(entry.get<std::uint64_t>());
				}
			}
		});
	}

	void Chat::AddServerCommands()
	{
		Command::AddSV("muteClient", [](const Command::Params* params)
		{
			if (!Dedicated::IsRunning())
			{
				Logger::Print("Server is not running.\n");
				return;
			}

			const auto* cmd = params->get(0);
			if (params->size() < 2)
			{
				Logger::Print("Usage: {} <client number> : prevent the player from using the chat\n", cmd);
				return;
			}

			const auto* client = Game::SV_GetPlayerByNum();
			if (client && !client->bIsTestClient)
			{
				Voice::SV_MuteClient(client - Game::svs_clients);
				MuteClient(client);
			}
		});

		Command::AddSV("unmute", [](const Command::Params* params)
		{
			if (!Dedicated::IsRunning())
			{
				Logger::Print("Server is not running.\n");
				return;
			}

			const auto* cmd = params->get(0);
			if (params->size() < 2)
			{
				Logger::Print("Usage: {} <client number or guid>\n{} all = unmute everyone\n", cmd, cmd);
				return;
			}

			const auto* client = Game::SV_GetPlayerByNum();
			if (client->bIsTestClient)
			{
				return;
			}

			if (client)
			{
				UnmuteClient(client);
				Voice::SV_UnmuteClient(client - Game::svs_clients);
				return;
			}

			if (std::strcmp(params->get(1), "all") == 0)
			{
				Logger::Print("All players were unmuted\n");
				UnmuteInternal(0, true);
				Voice::SV_ClearMutedList();
			}
			else
			{
				const auto steamId = std::strtoull(params->get(1), nullptr, 16);
				UnmuteInternal(steamId);
			}
		});

		Command::AddSV("say", [](const Command::Params* params)
		{
			if (!Dedicated::IsRunning())
			{
				Logger::Print("Server is not running.\n");
				return;
			}

			if (params->size() < 2) return;

			const auto message = params->join(1);
			const auto name = sv_sayName.get<std::string>();

			if (!name.empty())
			{
				Game::SV_GameSendServerCommand(-1, Game::SV_CMD_CAN_IGNORE, Utils::String::Format("{:c} \"{}: {}\"", 0x68, name, message));
				Logger::Print("{}: {}\n", name, message);
			}
			else
			{
				Game::SV_GameSendServerCommand(-1, Game::SV_CMD_CAN_IGNORE, Utils::String::Format("{:c} \"Console: {}\"", 0x68, message));
				Logger::Print("Console: {}\n", message);
			}
		});

		Command::AddSV("tell", [](const Command::Params* params)
		{
			if (!Dedicated::IsRunning())
			{
				Logger::Print("Server is not running.\n");
				return;
			}

			if (params->size() < 3) return;

			const auto parsedInput = std::strtoul(params->get(1), nullptr, 10);
			const auto clientNum = static_cast<int>(std::min<std::size_t>(parsedInput, Game::MAX_CLIENTS));

			const auto message = params->join(2);
			const auto name = sv_sayName.get<std::string>();

			if (!name.empty())
			{
				Game::SV_GameSendServerCommand(clientNum, Game::SV_CMD_CAN_IGNORE, Utils::String::Format("{:c} \"{}: {}\"", 0x68, name.data(), message));
				Logger::Print("{} -> {}: {}\n", name, clientNum, message);
			}
			else
			{
				Game::SV_GameSendServerCommand(clientNum, Game::SV_CMD_CAN_IGNORE, Utils::String::Format("{:c} \"Console: {}\"", 0x68, message));
				Logger::Print("Console -> {}: {}\n", clientNum, message);
			}
		});

		Command::AddSV("sayraw", [](const Command::Params* params)
		{
			if (!Dedicated::IsRunning())
			{
				Logger::Print("Server is not running.\n");
				return;
			}

			if (params->size() < 2) return;

			const auto message = params->join(1);
			Game::SV_GameSendServerCommand(-1, Game::SV_CMD_CAN_IGNORE, Utils::String::Format("{:c} \"{}\"", 0x68, message));
			Logger::Print("Raw: {}\n", message);
		});

		Command::AddSV("tellraw", [](const Command::Params* params)
		{
			if (!Dedicated::IsRunning())
			{
				Logger::Print("Server is not running.\n");
				return;
			}

			if (params->size() < 3) return;

			const auto parsedInput = std::strtoul(params->get(1), nullptr, 10);
			const auto clientNum = static_cast<int>(std::min<std::size_t>(parsedInput, Game::MAX_CLIENTS));

			const auto message = params->join(2);
			Game::SV_GameSendServerCommand(clientNum, Game::SV_CMD_CAN_IGNORE, Utils::String::Format("{:c} \"{}\"", 0x68, message));
			Logger::Print("Raw -> {}: {}\n", clientNum, message);
		});

		sv_sayName = Dvar::Register<const char*>("sv_sayName", "^7Console", Game::DVAR_NONE, "The alias of the server when broadcasting a chat message");
	}

	int Chat::GetCallbackReturn()
	{
		if (Game::scrVmPub->inparamcount == 0)
		{
			// Nothing. Let's not mute the player
			return 1;
		}

		Game::Scr_ClearOutParams();
		Game::scrVmPub->outparamcount = Game::scrVmPub->inparamcount;
		Game::scrVmPub->inparamcount = 0;

		const auto* result = &Game::scrVmPub->top[1 - Game::scrVmPub->outparamcount];

		if (result->type != Game::VAR_INTEGER)
		{
			// Garbage was returned
			return 1;
		}

		return result->u.intValue;
	}

	int Chat::ChatCallback(Game::gentity_s* self, const char* codePos, const char* message, int mode)
	{
		constexpr auto paramcount = 2;

		Scripting::StackIsolation _;
		Game::Scr_AddInt(mode);
		Game::Scr_AddString(message);

		const auto objId = Game::Scr_GetEntityId(self - Game::g_entities, 0);
		Game::AddRefToObject(objId);
		const auto id = Game::VM_Execute_0(Game::AllocThread(objId), codePos, paramcount);

		const auto result = GetCallbackReturn();

		Game::RemoveRefToValue(Game::scrVmPub->top->type, Game::scrVmPub->top->u);

		Game::scrVmPub->top->type = Game::VAR_UNDEFINED;
		--Game::scrVmPub->top;
		--Game::scrVmPub->inparamcount;

		Game::Scr_FreeThread(static_cast<std::uint16_t>(id));

		return result;
	}

	void Chat::AddScriptFunctions()
	{
		GSC::Script::AddFunction("OnPlayerSay", [] // gsc: OnPlayerSay(<function>)
		{
			if (Game::Scr_GetNumParam() != 1)
			{
				Game::Scr_Error("OnPlayerSay: Needs one function pointer!");
				return;
			}

			if (!CanAddCallback)
			{
				Game::Scr_Error("OnPlayerSay: Cannot add a callback in this context");
				return;
			}

			const auto* func = GSC::ScriptExtension::GetCodePosForParam(0);
			SayCallbacks.emplace_back(func);
		});
	}

	Chat::Chat()
	{
		AssertOffset(Game::client_s, steamID, 0x43F00);

		cg_chatWidth = Dvar::Register<int>("cg_chatWidth", 52, 1, std::numeric_limits<int>::max(), Game::DVAR_ARCHIVE, "The normalized maximum width of a chat message");
		sv_disableChat = Dvar::Register<bool>("sv_disableChat", false, Game::DVAR_NONE, "Disable chat messages from clients");
		Events::OnSVInit(AddServerCommands);

		LoadMutedList();

		// Intercept chat sending
		Utils::Hook(0x4D000B, PreSayStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x4D00D4, PostSayStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x4D0110, PostSayStub, HOOK_CALL).install()->quick();

		// Change logic that does word splitting with new lines for chat messages to support fonticons
		Utils::Hook(0x592E10, CG_AddToTeamChat_Stub, HOOK_JUMP).install()->quick();

		// Add back removed command from CoD4
		Command::Add("mp_QuickMessage", []() -> void
		{
			Command::Execute("openmenu quickmessage");
		});

		AddScriptFunctions();

		// Avoid duplicates
		Events::OnVMShutdown([]
		{
			SayCallbacks.clear();
		});
	}
}
