#include "STDInclude.hpp"

namespace Components
{
	mg_mgr Download::Mgr;

	void Download::EventHandler(mg_connection *nc, int ev, void *ev_data)
	{
		// Only handle http requests
		if (ev != MG_EV_HTTP_REQUEST) return;

		http_message* message = reinterpret_cast<http_message*>(ev_data);

		if (std::string(message->uri.p, message->uri.len) == "/")
		{
			mg_printf(nc, "%s",
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/html\r\n"
				"Connection: close\r\n"
				"\r\n"
				"Hi fella!");
		}
		else
		{
			mg_printf(nc, "%s",
				"HTTP/1.1 404 Not Found\r\n"
				"Content-Type: text/html\r\n"
				"Connection: close\r\n"
				"\r\n"
				"<h1>404 - Not Found</h1>");
		}

		nc->flags |= MG_F_SEND_AND_CLOSE;
	}

	Download::Download()
	{
		if (Dedicated::IsDedicated())
		{
			mg_mgr_init(&Download::Mgr, NULL);

			Network::OnStart([] ()
			{
				mg_connection* nc = mg_bind(&Download::Mgr, Utils::VA("%hu", (Dvar::Var("net_port").Get<int>() & 0xFFFF)), Download::EventHandler);
				mg_set_protocol_http_websocket(nc);
			});

			QuickPatch::OnFrame([]
			{
				mg_mgr_poll(&Download::Mgr, 0);
			});
		}
		else
		{
			Utils::Hook(0x5AC6E9, [] ()
			{
				// TODO: Perform moddownload here

				Game::CL_DownloadsComplete(0);
			}, HOOK_CALL).Install()->Quick();
		}
	}

	Download::~Download()
	{
		if (Dedicated::IsDedicated())
		{
			mg_mgr_free(&Download::Mgr);
		}
		else
		{

		}
	}
}
