#pragma comment (lib, "ws2_32.lib")
#include <stdio.h>
#include <WinSock2.h>

// BUFSIZE
#define BUF_SIZE 1024*2
#define MY_SERVER_PORT 9000

// STATE
struct UserState{
#define CONNECT		0
#define WAIT			1
#define PLAYGAME		2
};

SOCKET g_mysocket;

int main()
{
	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2,2), &wsadata);

	g_mysocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family		= AF_INET;
	ServerAddr.sin_port			= htons(MY_SERVER_PORT);
	ServerAddr.sin_addr.s_addr	= inet_addr("192.168.0.9");

	int Result = WSAConnect(g_mysocket, (sockaddr *) &ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);

	if(Result == SOCKET_ERROR)
	{
		return -1;
	}

	while(1)
	{
		Sleep(1000);
	}

	return 0;
}