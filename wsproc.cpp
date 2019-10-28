#include "stdafx.h"
#include "mongoose.h"
#include "wsproc.h"
#include <string>
#include "notifychange.h"


static std::string g_http_port = "6006";
static HANDLE g_hMutex = INVALID_HANDLE_VALUE;
HANDLE g_hTread = INVALID_HANDLE_VALUE;


struct mg_mgr mgr;
void config_param()
{
	char buffer[MAX_PATH];
	//GetModuleFileName(NULL, buffer, sizeof(buffer));
	std::string portal_path(buffer);
	int pos = portal_path.find_last_of('\\');
	portal_path = portal_path.substr(0, pos);
	portal_path += "\\config.ini";
	//strTempPath = "resafety_render work path=" + portal_path;
	//OutputDebugString(strTempPath.c_str());
}

void start_tcp_server()
{
	struct mg_connection *c;
	mg_mgr_init(&mgr, NULL);
	//c = mg_bind(&mgr, g_http_port.c_str(), ev_handler);
	c = mg_bind(&mgr, g_http_port.c_str(), ev_handler);
	g_hMutex = CreateMutex(NULL, FALSE, "Mutex-Alive");
	if (!g_hMutex)
	{
		printf("Failed to CreateMutex !");
	}
	g_hTread = CreateThread(NULL, 0, ThreadProcFunc, NULL, 0, NULL);
}

void start_websocket()
{
	struct mg_connection *c;
	mg_mgr_init(&mgr, NULL);
	c = mg_bind(&mgr, g_http_port.c_str(), ev_handler);
	mg_set_protocol_http_websocket(c);

	g_hMutex = CreateMutex(NULL, FALSE, "Mutex-Alive");
	if (!g_hMutex)
	{
		printf("Failed to CreateMutex !");
	}
	g_hTread = CreateThread(NULL, 0, ThreadProcFunc, NULL, 0, NULL);
}

void keep_alive_ws()
{
	for (;;)
	{
		mg_mgr_poll(&mgr, 3000);
		Sleep(10);
	}
}

void close_websocket()
{
	mg_mgr_free(&mgr);
	CloseHandle(g_hTread);
}


DWORD WINAPI ThreadProcFunc(LPVOID lpParam)
{
	keep_alive_ws();
	return 0;
}

void ev_handler(struct mg_connection *c, int ev, void *p)
{
	switch (ev)
	{
		case MG_EV_HTTP_REQUEST:
		{
			struct http_message *hm = (struct http_message *)p;
			mg_send_head(c, 200, hm->message.len, "Content-Type: text/plain");
			break;
		}
		case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
		{
			char addr[32];
			mg_sock_addr_to_str(&c->sa, addr, sizeof(addr), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
			std::string temp_addr = addr;

			mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, addr, strlen(addr));
			//struct websocket_message* wm = (struct websocket_message*) p;
			//struct mg_str d = { (char*)wm->data,wm->size };
			CChangeTransmit::GetInstance()->NotifyStartWindow(temp_addr.c_str());
			break;
		}
		case MG_EV_WEBSOCKET_FRAME:
		{
			struct websocket_message* wm = (struct websocket_message*) p;
			struct mg_str d = { (char*)wm->data,wm->size };

			CChangeTransmit::GetInstance()->NotifyChange(d.p);

			break;

		}
		case MG_EV_CLOSE:
		{
			int i = 0;
			break;
		}

	}
}

