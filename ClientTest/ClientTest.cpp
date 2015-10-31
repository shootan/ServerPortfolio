// ClientTest.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "XSocket.h"
#include <atlstr.h>

#include <process.h>

unsigned __stdcall Thread(void*)
{
	srand(GetTickCount());

	char buf[1024];
	int MaxCount = 100;
	int CurrentCount = 0;

	while(1)
	{
		if(MaxCount <= CurrentCount)
		{
			Sleep(1000);
			continue;
		}
		CXSocket s;
		if(!s.Create() || !s.Connect("192.168.0.9", 9000, 5000))
		{
			printf("%d\r\n", GetLastError());
			continue;
		}

		CString strData = "TEST Server!";

		s.Send((PTSTR)(PCTSTR)strData, strData.GetLength());

		/*buf[0] = 0;
		int nRecv = s.Receive(buf, 1024, 1000);
		if(nRecv <= 0)
		{
			printf("%d\r\n", GetLastError());
		}
		else
		{
			buf[nRecv] = 0;
			printf("%d - %s\r\n", nRecv, buf);
		}
*/
		CurrentCount++;
		Sleep(1000);
		//Sleep(1000);

		//s.Close(true);
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA w;
	WSAStartup(MAKEWORD(2, 2), &w);

	for(int i = 0; i < 200; i++)
	{
		_beginthreadex(NULL, 0, Thread, NULL, 0, NULL);
		Sleep(10);
	}

	while(1)
	{
		Sleep(1000);
	}

	return 0;
}

