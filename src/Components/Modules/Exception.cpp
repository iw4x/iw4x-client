#include "STDInclude.hpp"

// Stuff causes warnings
#pragma warning(push)
#pragma warning(disable: 4091)
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#pragma warning(pop)

namespace Components
{
	// Fileupload to Webhost
	bool Exception::UploadMinidump(std::string filename)
	{
		Utils::WebIO webio("Firefucks", UPLOAD_URL);

		if (Utils::IO::FileExists(filename))
		{
			std::string buffer = Utils::IO::ReadFile(filename);
			std::string result = webio.PostFile(buffer);

			std::string errors;
			json11::Json object = json11::Json::parse(result, errors);

			if (!object.is_object()) return false;

			json11::Json success = object["success"];

			if (!success.is_bool() || !success.bool_value()) return false;

			json11::Json files = object["files"];

			if (!files.is_array()) return false;

			for (auto file : files.array_items())
			{
				json11::Json url = file["url"];
				json11::Json hash = file["hash"];

				if (hash.is_string() && url.is_string())
				{
					if (Utils::String::ToLower(Utils::Cryptography::SHA1::Compute(buffer, true)) == Utils::String::ToLower(hash.string_value()))
					{
						MessageBoxA(0, url.string_value().data(), 0, 0);
						return true;
					}
				}
			}
		}
		return false;
	}
	
	// Fileupload to Bitmessage
	bool Exception::UploadMinidump2BM(std::string filename)
	{
		if (Components::BitMessage::Singleton == nullptr)
		{
			//throw new std::runtime_error("BitMessage was already stopped.");
			Logger::Print("Bitmessage was already stopped.\n");
		}
		else
		{
			BitMessage* Singleton;
			Singleton = Components::BitMessage::Singleton;

			if (Utils::IO::FileExists(filename))
			{
				// TODO: Validate filesize of minidump
				// TODO: Convert to base64
				// TODO: Split if filesize > xxxkB
				std::string buffer = Utils::String::encodeBase64(Utils::IO::ReadFile(filename));
				
				ustring pubAddrString;
				pubAddrString.fromString(BITMESSAGE_UPLOAD_IDENTITY);
				PubAddr pubAddr;
				if (pubAddr.loadAddr(pubAddrString))
				{
					int g;
					ustring msg;
					
					Logger::Print("Uploading Minidump (this may take a while)...\n");
					
					for (size_t i = 0; i < buffer.size(); i=i+BITMESSAGE_SIZE_LIMIT)
					{
						if (buffer.size() > i + BITMESSAGE_SIZE_LIMIT)
							g = buffer.size();
						else
							g = i + BITMESSAGE_SIZE_LIMIT;
						std::string substring = buffer.substr(i, g);
						msg.fromString(substring);
						Singleton->BMClient->sendMessage(msg, pubAddr, Singleton->BMClient->PrivAddresses[0]);
					}

					Logger::Print("Minidump uploaded.\n");
				}
				else
				{
					Logger::Print("Address not correct!\n");
				}
			}
		}
		return false;
	}

	LONG WINAPI Exception::ExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
	{
		char filename[MAX_PATH];
		__time64_t time;
		tm ltime;

		_time64(&time);
		_localtime64_s(&ltime, &time);
		strftime(filename, sizeof(filename) - 1, "iw4x-" VERSION_STR "-%Y%m%d%H%M%S.dmp", &ltime);

		HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile && hFile != INVALID_HANDLE_VALUE)
		{
			MINIDUMP_EXCEPTION_INFORMATION ex = { GetCurrentThreadId(), ExceptionInfo, FALSE };
			MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ex, NULL, NULL);
			CloseHandle(hFile);
		}

		//Exception::UploadMinidump(filename);
		Exception::UploadMinidump2BM(filename);

		if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
		{
			Logger::Error("Termination because of a stack overflow.\n");
			TerminateProcess(GetCurrentProcess(), EXCEPTION_STACK_OVERFLOW);
		}
		else
		{
			Logger::Error("Fatal error (0x%08X) at 0x%08X.", ExceptionInfo->ExceptionRecord->ExceptionCode, ExceptionInfo->ExceptionRecord->ExceptionAddress);
		}

		return EXCEPTION_CONTINUE_SEARCH;
	}

	LPTOP_LEVEL_EXCEPTION_FILTER WINAPI Exception::SetUnhandledExceptionFilterStub(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
	{
		SetUnhandledExceptionFilter(&Exception::ExceptionFilter);
		return lpTopLevelExceptionFilter;
	}

	Exception::Exception()
	{
#ifdef DEBUG
		// Display DEBUG branding, so we know we're on a debug build
		Renderer::OnFrame([] ()
		{
			Game::Font* font = Game::R_RegisterFont("fonts/normalFont");
			float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

			// Change the color when attaching a debugger
			if (IsDebuggerPresent())
			{
				color[0] = 0.6588f;
				color[1] = 1.0000f;
				color[2] = 0.0000f;
			}

			Game::R_AddCmdDrawText("DEBUG-BUILD", 0x7FFFFFFF, font, 15.0f, 10.0f + Game::R_TextHeight(font), 1.0f, 1.0f, 0.0f, color, Game::ITEM_TEXTSTYLE_SHADOWED);
		});
#else
		Utils::Hook::Set(0x6D70AC, Exception::SetUnhandledExceptionFilterStub);
		SetUnhandledExceptionFilter(&Exception::ExceptionFilter);
#endif

		Command::Add("mapTest", [] (Command::Params params)
		{
			std::string command;

			int max = (params.Length() >= 2 ? atoi(params[1]) : 16), current = 0;

			for (int i =0;;)
			{
				char* mapname = Game::mapnames[i];
				if (!*mapname)
				{
					i = 0;
					continue;
				}

				if(!(i % 2)) command.append(fmt::sprintf("wait 250;disconnect;wait 750;", mapname)); // Test a disconnect
				else command.append(fmt::sprintf("wait 500;", mapname));                             // Test direct map switch
				command.append(fmt::sprintf("map %s;", mapname));

				++i, ++current;

				if (current >= max) break;
			}

			Command::Execute(command, false);
		});
	}
}
