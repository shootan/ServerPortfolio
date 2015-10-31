#include "IOCP.h"

IOCPServer::IOCPServer()
{
	InitializeCriticalSection(&m_BufferListLock);
	m_pNextBufferList = NULL;
	m_hIO = NULL;
	m_hListenThread = NULL;
	m_hWorkerThread = NULL;

	m_bServerShutDown = TRUE;
	m_bServerStart = FALSE;

	m_iClientCount = 0;

	ZeroMemory(&m_ClinetAddr, sizeof(SOCKADDR));
}

IOCPServer::~IOCPServer()
{
	DeleteCriticalSection(&m_BufferListLock);
}

BOOL IOCPServer::StartServer(int port)
{
	m_hIO = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0); 
	m_bServerStart = TRUE;
	m_bServerShutDown = FALSE;

	if(!this->InitServer(port))
		return FALSE;
	
	if(!this->InitThread())
		return FALSE;

	return TRUE;
}

BOOL IOCPServer::InitThread()
{
	unsigned int ThreadID;
	
	m_hListenThread = (HANDLE)_beginthreadex(NULL, 0, ListenThread, this, 0, &ThreadID);
	
	if(m_hListenThread == NULL)
		return FALSE;

	// CPU 갯수확인 
	SYSTEM_INFO si; 
	GetSystemInfo(&si); 

	// (CPU 개수* 2)개의작업자스레드생성 
	for(int i=0; i < (int)si.dwNumberOfProcessors*2; ++i) 
	{ 
		m_hWorkerThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, this, 0, &ThreadID); 
		if(m_hWorkerThread == NULL) 
			return FALSE;
	} 
	return TRUE;
}

UINT WINAPI IOCPServer::ListenThread(LPVOID arg)
{
	IOCPServer* pThis = (IOCPServer*)arg;
	
	while (1)
	{
		if(pThis->m_bServerShutDown) break;
		ZeroMemory(&pThis->m_ClinetAddr, sizeof(SOCKADDR));
		int addr_len = sizeof(pThis->m_ClinetAddr);

		SOCKET client_sock = WSAAccept(pThis->m_ListenSock, (sockaddr *)&pThis->m_ClinetAddr, &addr_len, NULL, NULL);
		if (client_sock == INVALID_SOCKET) {
			exit(-1);
		}
		
		pThis->m_iClientCount++;
		printf("IP 주소= %s, 포트번호= %d , Count = %d\n", 
			inet_ntoa(pThis->m_ClinetAddr.sin_addr), 
			ntohs(pThis->m_ClinetAddr.sin_port), pThis->m_iClientCount); 

		//버퍼 생성
		IOBuffer* buffer = new IOBuffer;
		ZeroMemory(buffer, sizeof(IOBuffer));
		buffer->m_pNext = pThis->m_pNextBufferList;
		pThis->m_pNextBufferList = buffer;
		buffer->m_iRecvbytes = 0;
		buffer->m_iRecvbytes = 0;
		buffer->m_Wsabuf.buf = buffer->m_Buf;
		buffer->m_Wsabuf.len = BUFSIZE;
		buffer->m_ClientSock = client_sock;
		buffer->m_Opcode = OP_INIT;

		//비동기 입출력 시작
		DWORD recvbytes;
		DWORD flags = 0;
		CreateIoCompletionPort((HANDLE)client_sock, pThis->m_hIO, (DWORD)buffer, 0);

		PostQueuedCompletionStatus(pThis->m_hIO, 0, (ULONG_PTR)buffer, &buffer->m_Overlapped);
	}
}

UINT WINAPI IOCPServer::WorkerThread(LPVOID arg)
{
	int retval = 0;
	DWORD dwSize;
	IOBuffer* buff;
	unsigned long key;
	LPOVERLAPPED over;
	IOCPServer* server = (IOCPServer*)arg;

	while(!server->m_bServerShutDown)
	{
		GetQueuedCompletionStatus(server->m_hIO, &dwSize, (PULONG_PTR)&buff, (LPOVERLAPPED*)&over, INFINITE);

		// 클라이언트정보얻기 
		SOCKADDR_IN clientaddr; 
		int addrlen = sizeof(clientaddr); 
		getpeername(buff->m_ClientSock, (SOCKADDR*)&clientaddr, &addrlen);

		switch ( buff->m_Opcode )
		{
		case OP_INIT:
			server->OnInit(buff);
			break;
		case OP_RECV:
			server->OnRecv(buff, buff->m_Buf, BUFSIZE);
			break;
		case OP_RECV_DONE:
			server->OnRecvFinish(buff, dwSize);
			break;
		}
	}
	return 0;
}

void IOCPServer::SetOpCode(IOBuffer* _buff, OPCODE _opCode)
{
	ZeroMemory(&_buff->m_Overlapped, sizeof(OVERLAPPED));
	_buff->m_Opcode = _opCode;
}

void IOCPServer::OnInit(IOBuffer* _buff)
{
	BOOL bSuccess;
	this->SetOpCode(_buff, OP_RECV);
	bSuccess = PostQueuedCompletionStatus(m_hIO, 0, (ULONG_PTR)_buff, &(_buff->m_Overlapped));

	if ( !bSuccess && WSAGetLastError() != ERROR_IO_PENDING )
	{
		// error
	}
}

void IOCPServer::OnRecv(IOBuffer* _buff, char* _recvBuff, int _size)
{
	DWORD dwRecv = 0, dwFlags = 0;
	WSABUF buffRecv;

	this->SetOpCode(_buff, OP_RECV_DONE);

	buffRecv.buf = _recvBuff;
	buffRecv.len = _size;

	int retval = WSARecv(_buff->m_ClientSock, &buffRecv, 1, &dwRecv, &dwFlags, &(_buff->m_Overlapped), NULL);

	if ( retval == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError())) 
	{
		// error
	}
}

void IOCPServer::OnRecvFinish(IOBuffer* _buff, DWORD _size)
{
	if ( _size == 0 )
	{
		m_iClientCount--;
		return;
	}

	_buff->m_iRecvbytes = _size;
	_buff->m_Buf[_buff->m_iRecvbytes] = 0;
	printf("[RECV] %s\n", _buff->m_Buf);
	
	SetOpCode(_buff, OP_RECV);
	BOOL bSuccess = PostQueuedCompletionStatus(m_hIO, 0, (ULONG_PTR)_buff, &(_buff->m_Overlapped));
	_buff->m_iRecvbytes = 0;

	if ( !bSuccess && WSAGetLastError() != ERROR_IO_PENDING )
	{
		// error
	}
}
