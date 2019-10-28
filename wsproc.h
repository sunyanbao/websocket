#pragma once

//websocket ²Ù×÷
//

void config_param();
void start_tcp_server();
void start_websocket();
void keep_alive_ws();
void close_websocket();
static void ev_handler(struct mg_connection *c, int ev, void *p);
DWORD WINAPI ThreadProcFunc(LPVOID lpParam);