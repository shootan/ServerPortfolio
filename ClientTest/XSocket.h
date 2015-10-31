#pragma once

#include <WinSock2.h>
#include <Windows.h>

class CXSocket
{
public:

	SOCKET m_hSocket;
	bool m_bUDP;

	CXSocket(void);
	~CXSocket(void);
	bool Create(int nPort = -1, bool bUDP = false);
	SOCKET Accept(DWORD dwTimeout = INFINITE);
	bool Connect(LPCTSTR szAddr, int nPort, DWORD nTimeout);
	bool Connect(in_addr ip, int nPort, DWORD nTimeout);
	void Close(bool bHardful = false);
	int Receive(void* buf, int len, DWORD nTimeout = INFINITE);
	int ReceiveByLen(void* buf, int len, DWORD dwTimeout = INFINITE);

	int ReceiveFrom(void* buf, int len, in_addr& ip, int& nPort, DWORD dwTimeout = INFINITE);
	int Send(void* buf, int len);
	int SendTo(void* buf, int len, LPCTSTR szIP, int nPort);
	int SendTo(void* buf, int len, in_addr ip, int nPort);

	int SetBlocking(bool bBlock);
	bool IsUDP(void);
	int Listen(int nBacklog=5);
	int ShutDown(int nHow);
};
