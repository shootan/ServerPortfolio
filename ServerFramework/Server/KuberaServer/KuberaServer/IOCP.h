#pragma once
#include "Network.h"


typedef enum OPCODE
{
	OP_INIT,
	OP_RECV,
	OP_RECV_DONE
} OPCODE;

struct IOBuffer{
	SOCKET		m_ClientSock;
	int			m_Id;
	OVERLAPPED	m_Overlapped; 
	char		m_Buf[BUFSIZE+1]; 
	int			m_iRecvbytes; 
	int			m_iSendbytes; 
	WSABUF		m_Wsabuf;
	OPCODE		m_Opcode;
	IOBuffer*	m_pNext;
};

class IOCPServer : public Network
{
public:
	//버퍼를 리스트로 관리해줄 포인터
	IOBuffer* m_pNextBufferList;

	//핸들
	HANDLE m_hIO;
	HANDLE m_hListenThread;
	HANDLE m_hWorkerThread;

	//
	CRITICAL_SECTION m_BufferListLock;
	SOCKADDR_IN m_ClinetAddr;

	//
	BOOL	m_bServerStart;
	BOOL	m_bServerShutDown;

	int		m_iClientCount;	

	
	
	//함수
	IOCPServer();
	~IOCPServer();

	BOOL StartServer(int port);
	BOOL InitThread();
	void CreateBuffer(IOBuffer* buffer);

	//OpCode
	void OnInit(IOBuffer* _buff);
	void OnRecv(IOBuffer* _buff, char* _recvBuff, int _size);
	void OnRecvFinish(IOBuffer* _buff, DWORD _size);

	void SetOpCode(IOBuffer* _buff, OPCODE _opCode);

	//스레드
	static UINT WINAPI ListenThread(LPVOID arg);
	static UINT WINAPI WorkerThread(LPVOID arg);
};