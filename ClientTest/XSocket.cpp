#include "StdAfx.h"
#include "xsocket.h"
#include ".\xsocket.h"

#pragma comment(lib, "WS2_32.LIB")

CXSocket::CXSocket(void)
{
	m_bUDP = false;
}

CXSocket::~CXSocket(void)
{
	//if(m_hSocket != INVALID_SOCKET)	Close();
}

SOCKET CXSocket::Accept(DWORD dwTimeout)
{
	int nAddrSize = sizeof(SOCKADDR_IN);
	SOCKADDR_IN SockAddr;

	if(dwTimeout == INFINITE)	return accept(m_hSocket,(SOCKADDR*)&SockAddr, &nAddrSize);

	struct  timeval     timevalue;
	fd_set fdset;
	FD_ZERO( &fdset );
	FD_SET( m_hSocket, &fdset );
	timevalue.tv_sec = dwTimeout/1000;
	timevalue.tv_usec = 0;

	if(select(0, &fdset, NULL, NULL, &timevalue ) <= 0)	return SOCKET_ERROR;

	return accept(m_hSocket,(SOCKADDR*)&SockAddr, &nAddrSize);
}

bool CXSocket::Connect(LPCTSTR szAddr, int nPort, DWORD nTimeout)
{
	SOCKADDR_IN addr;

	addr.sin_addr.s_addr = inet_addr(szAddr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons((short)nPort);

	ULONG nonBlk = TRUE;
	struct  timeval     timevalue;

	if(ioctlsocket(m_hSocket, FIONBIO, &nonBlk ) == SOCKET_ERROR)	return false;
	
	int ret;
	ret = connect(m_hSocket, (LPSOCKADDR)&addr, sizeof(addr));

	if(ret == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)	return false;

	fd_set fdset;
	FD_ZERO( &fdset );
	FD_SET( m_hSocket, &fdset );
	timevalue.tv_sec = nTimeout/1000;
	timevalue.tv_usec = 0;

	if(select(0, NULL, &fdset, NULL, &timevalue ) == SOCKET_ERROR)	return false;
	if ( !FD_ISSET( m_hSocket, &fdset ) )
	{		
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
		return false;
	}

	nonBlk = FALSE;
	if(ioctlsocket(m_hSocket, FIONBIO, &nonBlk ) == SOCKET_ERROR)	return false;

	return true;
}

bool CXSocket::Connect(in_addr ip, int nPort, DWORD nTimeout)
{
	SOCKADDR_IN addr;

	addr.sin_addr = ip;
	addr.sin_family = AF_INET;
	addr.sin_port = htons((short)nPort);

	ULONG nonBlk = TRUE;
	struct  timeval     timevalue;

	if(ioctlsocket(m_hSocket, FIONBIO, &nonBlk ) == SOCKET_ERROR)	return false;

	int ret;
	ret = connect(m_hSocket, (LPSOCKADDR)&addr, sizeof(addr));

	if(ret == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)	return false;

	fd_set fdset;
	FD_ZERO( &fdset );
	FD_SET( m_hSocket, &fdset );
	timevalue.tv_sec = nTimeout/1000;
	timevalue.tv_usec = 0;

	if(select(0, NULL, &fdset, NULL, &timevalue ) == SOCKET_ERROR)	return false;
	if ( !FD_ISSET( m_hSocket, &fdset ) )
	{		
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;
		return false;
	}

	nonBlk = FALSE;
	if(ioctlsocket(m_hSocket, FIONBIO, &nonBlk ) == SOCKET_ERROR)	return false;

	return true;
}

bool CXSocket::Create(int nPort, bool bUDP)
{
	if(!bUDP)	m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	else		m_hSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(m_hSocket == INVALID_SOCKET)	return false;

	m_bUDP = bUDP;

	if(nPort != -1)
	{
		SOCKADDR_IN ra;
		memset(&ra, 0, sizeof(ra));
		ra.sin_family = AF_INET;
		ra.sin_port = htons(nPort);
		ra.sin_addr.s_addr = htonl(INADDR_ANY);

		if(bind(m_hSocket, (SOCKADDR*)&ra, sizeof(ra)) == SOCKET_ERROR)
		{
			return false;
		}
	}

	return true;
}

void CXSocket::Close(bool bHardful)
{
	if(!m_bUDP)
	{
		if(bHardful)
		{
			struct linger   ling;
			ling.l_onoff = 1;
			ling.l_linger = 0;
			setsockopt(m_hSocket, SOL_SOCKET, SO_LINGER, (char*)&ling, sizeof(ling));
		}
		else
		{
			ShutDown(SD_BOTH);
			//char buf[1024];
			//Receive(buf, 1024, 1000*1);
		}
	}

	closesocket(m_hSocket);
	m_hSocket = INVALID_SOCKET;
}

int CXSocket::Receive(void* buf, int len, DWORD nTimeout)
{
	if(nTimeout == INFINITE)	return recv(m_hSocket, (char*)buf, len, 0);

	struct  timeval     timevalue;
	fd_set fdset;
	FD_ZERO( &fdset );
	FD_SET( m_hSocket, &fdset );
	timevalue.tv_sec = nTimeout/1000;
	timevalue.tv_usec = 0;

	if(select(0, &fdset, NULL, NULL, &timevalue ) <= 0)	return SOCKET_ERROR;

    return recv(m_hSocket, (char*)buf, len, 0);
}

int CXSocket::ReceiveByLen(void* buf, int len, DWORD dwTimeout)
{
	char* buf2 = (char*)buf;
	int nPos = 0;
	while(true)
	{
		int nRemain = len - nPos;
		if(nRemain <= 0)	break;
		int nRecv = Receive(&buf2[nPos], nRemain, dwTimeout);
		if(nRecv <= 0)	return SOCKET_ERROR;

		nPos += nRecv;
	}

	return len;
}

int CXSocket::ReceiveFrom(void* buf, int len, in_addr& ip, int& nPort, DWORD dwTimeout)
{
	SOCKADDR_IN sa;
	int sa_len = sizeof(sa);

	if(dwTimeout != INFINITE)
	{
		struct  timeval     timevalue;
		fd_set fdset;
		FD_ZERO( &fdset );
		FD_SET( m_hSocket, &fdset );
		timevalue.tv_sec = dwTimeout/1000;
		timevalue.tv_usec = 0;

		if(select(0, &fdset, NULL, NULL, &timevalue ) <= 0)	return SOCKET_ERROR;
	}

	int nRecv = recvfrom(m_hSocket, (char*)buf, len, 0, (SOCKADDR*)&sa, &sa_len);

	if(nRecv <= 0)	return nRecv;

	memcpy(&ip, &sa.sin_addr, sizeof(in_addr));
	nPort = (int)sa.sin_port;

	return nRecv;
}

int CXSocket::Send(void* buf, int len)
{
	return send(m_hSocket, (const char*)buf, len, 0);
}

int CXSocket::SendTo(void* buf, int len, LPCTSTR szIP, int nPort)
{
	SOCKADDR_IN sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(nPort);
	sa.sin_addr.s_addr = inet_addr(szIP);

	return sendto(m_hSocket, (const char*)buf, len, 0, (SOCKADDR*)&sa, sizeof(SOCKADDR));
}

int CXSocket::SendTo(void* buf, int len, in_addr ip, int nPort)
{
	SOCKADDR_IN sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(nPort);
	sa.sin_addr = ip;

	return sendto(m_hSocket, (const char*)buf, len, 0, (SOCKADDR*)&sa, sizeof(SOCKADDR));
}

int CXSocket::SetBlocking(bool bBlock)
{
	ULONG nonBlk = !bBlock;
	return ioctlsocket(m_hSocket, FIONBIO, &nonBlk );
}


bool CXSocket::IsUDP(void)
{
	return m_bUDP;
}

int CXSocket::Listen(int nBacklog)
{
	return listen(m_hSocket, nBacklog) != SOCKET_ERROR;
}

int CXSocket::ShutDown(int nHow)
{
	return shutdown(m_hSocket, nHow);
}
