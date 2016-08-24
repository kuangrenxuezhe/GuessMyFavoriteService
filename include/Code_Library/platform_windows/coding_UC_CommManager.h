// UC_CommManager.h

#ifndef _UC_COMMMANAGER_H_
#define _UC_COMMMANAGER_H_

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "UC_Communication.h"
#include "UT_Queue_Event.h"
#include "UT_Arithmetic.h"

#define VERIFYHEAD		0xaaaaaaaabbbbbbbb
#define VERIFYTAIL      0x0123456789abcedf

//////////////////////////////////////////////////////////////////////////
struct _OVERLAPPEDPLUS_NODE : public OVERLAPPEDPLUS
{
	long lActFlag;
	WSABUF wsaBuf;

	char* szpCommBuf;
	long lCommBufLen;

	long lMacNo;
};
typedef _OVERLAPPEDPLUS_NODE OVERLAPPEDPLUS_NODE;

typedef struct _SYNC_NODE
{
	HANDLE hEvent;
	long lRecv_Fin;
	long lCycleCount;
	long lRecvLen;
	char* szpRecvBuf;	
	CRITICAL_SECTION cNodeCriSec;	

	_SYNC_NODE()
	{
		lCycleCount = 0;
		szpRecvBuf = NULL;

		char szaEventName[64];
		sprintf(szaEventName, "zhongsou%zhl", this);
		hEvent = CreateEvent(NULL, TRUE, FALSE, szaEventName);

		InitializeCriticalSection(&cNodeCriSec);
	};
	~_SYNC_NODE()
	{		
		CloseHandle(hEvent);

		DeleteCriticalSection(&cNodeCriSec);
	};
} SYNC_NODE;

typedef struct _ASYNC_NODE
{
	char* szpBuffer;
} ASYNC_NODE;

class UC_CommManager
{
public:
	UC_CommManager()
	{
		m_szppIPList = NULL;
		m_uspSendPort = NULL;		
		
		m_hCompletionPort = NULL;
		m_funPos = NULL;		
		
		m_cpRecvLib = NULL;
		m_cpSendLib = NULL;
		
		m_cpSendQue = NULL;		
		
		m_cpSyncLib = NULL;

		m_ui64p_MacIndex = NULL;
		m_lpMacPos = NULL;
	}

	~UC_CommManager()
	{
		if(m_szppIPList)
		{
			for(long i = 0; i < m_lMachineNum; i++)
			{
				if(m_szppIPList[i])
					delete m_szppIPList[i];
			}
			delete m_szppIPList;
		}
		
		if(m_uspSendPort)
			delete m_uspSendPort;
		
		if(m_hCompletionPort)
			CloseHandle(m_hCompletionPort);
		
		if(m_cpRecvLib)
			delete m_cpRecvLib;

		if(m_cpSendLib)
			delete m_cpSendLib;
		
		if(m_cpSendQue)
			delete m_cpSendQue;
		
		if(m_cpSyncLib)
			delete m_cpSyncLib;

		if(m_ui64p_MacIndex)
			delete m_ui64p_MacIndex;

		if(m_lpMacPos)
			delete m_lpMacPos;
	}

	unsigned __int64 GetMacMashValue(char* szpIP, unsigned short usPort)
	{
		char* p = szpIP;
		unsigned __int64 value = 0;
		value += atol(p) * 1000000000;
		p = strchr(p, '.') + 1;
		value += atol(p) * 1000000;
		p = strchr(p, '.') + 1;
		value += atol(p) * 1000;
		p = strchr(p, '.') + 1;
		value += atol(p);
		value <<= 48;
		value += usPort;

		return value;
	}

	long GetMacNo(char* szpIP, unsigned short usPort)
	{
		long lRetVal = m_cArithmetic.BSearch(0, m_lMachineNum - 1, m_ui64p_MacIndex, GetMacMashValue(szpIP, usPort));
		if(lRetVal < 0)
			return -1;
		return m_lpMacPos[lRetVal];
	}

	long InitCommManager(unsigned short usRecvPort, long lCommThreadNum, long lMaxSendBufLen, long lMaxRecvBufLen, long lMachineNum, char szppIPList[][16], unsigned short* uspSendPort, long lChannelNumPerMac, long lRecvTimeOut, long lSendTimeOut)
	{
		// 初始化系统参数
		m_usRecvPort = usRecvPort;
		
		m_lCommThreadNum = lCommThreadNum;

		m_lMaxSendBufLen = lMaxSendBufLen;
		m_lMaxRecvBufLen = lMaxRecvBufLen;
		m_lMaxBufLen = (lMaxSendBufLen > lMaxRecvBufLen ? lMaxSendBufLen : lMaxRecvBufLen) + 28;
			
		m_lMachineNum = lMachineNum;
		
		m_ui64p_MacIndex = new unsigned __int64[lMachineNum];
		m_lpMacPos = new long[lMachineNum];

		m_szppIPList = new char*[lMachineNum];
		m_uspSendPort = new unsigned short[lMachineNum];
		for(long i = 0; i < lMachineNum; i++)
		{
			m_szppIPList[i] = new char[16];
			strcpy(m_szppIPList[i], szppIPList[i]);
			m_uspSendPort[i] = uspSendPort[i];

			m_ui64p_MacIndex[i] = GetMacMashValue(szppIPList[i], uspSendPort[i]);
			m_lpMacPos[i] = i;
		}		
		m_cArithmetic.QuickSort_U(0, lMachineNum - 1, m_ui64p_MacIndex, m_lpMacPos);

		m_lChannelNumPerMac = lChannelNumPerMac;

		m_lRecvTimeOut = lRecvTimeOut;
		m_lSendTimeOut = lSendTimeOut;

		m_lStartTime = time(NULL);
		
		// 初始化通讯变量
		if(UC::InitSocket())
		{
			printf("UC::InitSocket() failed!\n");
			return -1;
		}
		if(CreateListen_IOCP(m_usRecvPort, m_sListen, m_hCompletionPort))
		{
			printf("UC::CreateListen_IOCP() failed!\n");
			return -1;
		}
		if(GetAcceptEx(m_sListen, m_funPos))
		{
			printf("UC::GetAcceptEx() failed\n");
			return -1;
		}

		// 初始化同步结构	
		m_cSyncQue.InitQueue(m_lChannelNumPerMac * m_lMachineNum);
		m_cpSyncLib = new SYNC_NODE[m_lChannelNumPerMac * m_lMachineNum];
		for(long m = 0; m < m_lChannelNumPerMac * m_lMachineNum; m++)
			m_cSyncQue.PushData(m_cpSyncLib + m);

		// 初始化通信结构
		m_cpSendLib = new OVERLAPPEDPLUS_NODE[m_lChannelNumPerMac * m_lMachineNum];	
		m_cpSendQue = new UT_Queue_Event<OVERLAPPEDPLUS_NODE*>[m_lMachineNum];
		for(long j = 0; j < m_lMachineNum; j++)
		{
			m_cpSendQue[j].InitQueue(m_lChannelNumPerMac);
			for(long k = 0; k < m_lChannelNumPerMac; k++)
			{
				long i = j * m_lChannelNumPerMac + k;
				m_cpSendLib[i].szpCommBuf = new char[m_lMaxBufLen];
				m_cpSendLib[i].lMacNo = j;
				while(UC::ConnectServer_IOCP(m_szppIPList[j], m_uspSendPort[j], m_hCompletionPort, m_cpSendLib[i].sSocket))
				{
					printf("UC::ConnectServer_IOCP() failed! %s:%d\n", m_szppIPList[j], m_uspSendPort[j]);
					Sleep(1000);
				}
				m_cpSendQue[j].PushData(m_cpSendLib + i);
			}
		}
		m_cpRecvLib = new OVERLAPPEDPLUS_NODE[m_lChannelNumPerMac * m_lMachineNum];
		for(long k = 0; k < m_lChannelNumPerMac * m_lMachineNum; k++)
		{
			m_cpRecvLib[k].lActFlag = 0;
			m_cpRecvLib[k].szpCommBuf = new char[m_lMaxBufLen];
			if(UC::RunAcceptEx(m_sListen, m_hCompletionPort, m_funPos, m_cpRecvLib + k))
			{
				printf("UC::RunAcceptEx() failed!\n");
				Sleep(1000);
			}
		}

		// 启动主通讯线程
		for(long n = 0; n < m_lCommThreadNum; n++)
		{
			if(UC::StartThread(Thread_CommManager, this))
				return -1;
		}

		// 打印初始化成功信息
		printf("Congratulation, Init System OK!\n");

		return 0;
	}

	long Client_RecvBuffer(void* vpNode, char*& szprRecvBuf, long& lrRecvLen)
	{
		SYNC_NODE* cpSyncNode = (SYNC_NODE*)vpNode;

		WaitForSingleObject(cpSyncNode->hEvent, m_lRecvTimeOut);

		EnterCriticalSection(&cpSyncNode->cNodeCriSec);
		cpSyncNode->lCycleCount++;
		LeaveCriticalSection(&cpSyncNode->cNodeCriSec);

		long lRetVal = cpSyncNode->lRecv_Fin;
		lrRecvLen = cpSyncNode->lRecvLen;
		szprRecvBuf = cpSyncNode->szpRecvBuf;

		m_cSyncQue.PushData(cpSyncNode);

		return lRetVal;
	}

	long Client_SendBuffer(void*& vprNode, char* szpSendBuf, long lSendLen, char* szpRecvBuf, char* szpIP, unsigned short usPort)
	{
		if(lSendLen + 28 > m_lMaxBufLen)
			return -1;

		long lMacNo = GetMacNo(szpIP, usPort);
		if(lMacNo < 0)
			return -1;

		SYNC_NODE* cpSyncNode = m_cSyncQue.PopData();
		OVERLAPPEDPLUS_NODE* cpSendNode = m_cpSendQue[lMacNo].PopData();

		// verifyhead time struct count buffer verifytail
		char* p = cpSendNode->szpCommBuf;
		*(unsigned __int64*)p = VERIFYHEAD;
		p += 8;
		*(long*)p = m_lStartTime;
		p += 4;
		*(long*)p = (long)cpSyncNode;
		p += 4;
		*(long*)p = cpSyncNode->lCycleCount;
		p += 4;
		memcpy(p, szpSendBuf, lSendLen);
		p += lSendLen;
		*(unsigned __int64*)p = VERIFYTAIL;
		p += 8;

		cpSendNode->lCommBufLen = p - cpSendNode->szpCommBuf;
		cpSendNode->lActFlag = 3;	
		cpSendNode->wsaBuf.buf = cpSendNode->szpCommBuf;
		cpSendNode->wsaBuf.len = cpSendNode->lCommBufLen;

		DWORD dwCommNum = 0;
		if(WSASend(cpSendNode->sSocket, (LPWSABUF)&cpSendNode->wsaBuf, 1, &dwCommNum, 0, (LPWSAOVERLAPPED)cpSendNode, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			UC::CloseSocket(cpSendNode->sSocket);
			while(UC::ConnectServer_IOCP(m_szppIPList[cpSendNode->lMacNo], m_uspSendPort[cpSendNode->lMacNo], m_hCompletionPort, cpSendNode->sSocket))
				Sleep(1);
			m_cpSendQue[cpSendNode->lMacNo].PushData(cpSendNode);

			m_cSyncQue.PushData(cpSyncNode);

			return -1;
		}

		ResetEvent(cpSyncNode->hEvent);
		cpSyncNode->szpRecvBuf = szpRecvBuf;
		cpSyncNode->lRecv_Fin = -1;
		vprNode = cpSyncNode;

		return 0;
	}

	static DWORD WINAPI Thread_CommManager(void* vpArg)
	{
		UC_CommManager* cpClass = (UC_CommManager*)vpArg;
		
		DWORD dwCompKey = 0;
		DWORD dwRetCount = 0;

		OVERLAPPEDPLUS_NODE* cpNode = NULL;

		for(;;)
		{
			if(!GetQueuedCompletionStatus(cpClass->m_hCompletionPort, &dwRetCount, &dwCompKey, (OVERLAPPED**)&cpNode, INFINITE))
			{
				if(cpNode)
				{
					UC::CloseSocket(cpNode->sSocket);
					cpNode->lActFlag = 0;
					while(UC::RunAcceptEx(cpClass->m_sListen, cpClass->m_hCompletionPort, cpClass->m_funPos, cpNode))
						Sleep(1);
				}
				Sleep(1);
				continue;
			}

			if(dwCompKey)
				break;

			DWORD dwFlag = 0, dwCommNum = 0;
			switch(cpNode->lActFlag) 
			{
			case 0: // 连接成功			
				printf("Thread_CommManager, accept %s success\n", inet_ntoa(((struct sockaddr_in*)(cpNode->szaAddress + 38))->sin_addr));

				cpNode->lActFlag = 1;
				cpNode->wsaBuf.buf = cpNode->szpCommBuf;
				cpNode->wsaBuf.len = 4;			
				if(WSARecv(cpNode->sSocket, (LPWSABUF)&cpNode->wsaBuf, 1, &dwCommNum, &dwFlag, (LPWSAOVERLAPPED)cpNode, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
				{
					printf("Thread_CommManager, recv head failed, code = %d\n", WSAGetLastError());

					UC::CloseSocket(cpNode->sSocket);
					cpNode->lActFlag = 0;
					while(UC::RunAcceptEx(cpClass->m_sListen, cpClass->m_hCompletionPort, cpClass->m_funPos, cpNode))
						Sleep(1);
				}			
				break;
				
			case 1: // 接收包长度
				if(dwRetCount == 0)
				{
					printf("Thread_CommManager, recv head length failed, code = %d\n", dwRetCount, WSAGetLastError());

					UC::CloseSocket(cpNode->sSocket);
					cpNode->lActFlag = 0;
					while(UC::RunAcceptEx(cpClass->m_sListen, cpClass->m_hCompletionPort, cpClass->m_funPos, cpNode))
						Sleep(1);
				}
				else if(dwRetCount != cpNode->wsaBuf.len)
				{
					cpNode->lActFlag = 1;
					cpNode->wsaBuf.buf += dwRetCount;
					cpNode->wsaBuf.len -= dwRetCount;

					if(WSARecv(cpNode->sSocket, (LPWSABUF)&cpNode->wsaBuf, 1, &dwCommNum, &dwFlag, (LPWSAOVERLAPPED)cpNode, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
					{
						printf("Thread_CommManager, recv head failed, code = %d\n", WSAGetLastError());

						UC::CloseSocket(cpNode->sSocket);
						cpNode->lActFlag = 0;
						while(UC::RunAcceptEx(cpClass->m_sListen, cpClass->m_hCompletionPort, cpClass->m_funPos, cpNode))
							Sleep(1);
					}
				}
				else
				{
					if(*(long*)cpNode->szpCommBuf > cpClass->m_lMaxBufLen || *(long*)cpNode->szpCommBuf < 28)
					{
						printf("Thread_CommManager, head too length!\n");

						UC::CloseSocket(cpNode->sSocket);
						cpNode->lActFlag = 0;
						while(UC::RunAcceptEx(cpClass->m_sListen, cpClass->m_hCompletionPort, cpClass->m_funPos, cpNode))
							Sleep(1);
					}
					else
					{
						cpNode->lCommBufLen = *(long*)cpNode->szpCommBuf;

						cpNode->lActFlag = 2;
						cpNode->wsaBuf.buf = cpNode->szpCommBuf;
						cpNode->wsaBuf.len = *(long*)cpNode->szpCommBuf;

						if(WSARecv(cpNode->sSocket, (LPWSABUF)&cpNode->wsaBuf, 1, &dwCommNum, &dwFlag, (LPWSAOVERLAPPED)cpNode, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
						{
							printf("Thread_CommManager, recv head failed, code = %d\n", WSAGetLastError());

							UC::CloseSocket(cpNode->sSocket);
							cpNode->lActFlag = 0;
							while(UC::RunAcceptEx(cpClass->m_sListen, cpClass->m_hCompletionPort, cpClass->m_funPos, cpNode))
								Sleep(1);
						}
					}
				}
				break;

			case 2: // 接收包内容
				if(dwRetCount == 0)
				{
					printf("Thread_CommManager, recv body failed, code = %d\n", dwRetCount, WSAGetLastError());

					UC::CloseSocket(cpNode->sSocket);
					cpNode->lActFlag = 0;
					while(UC::RunAcceptEx(cpClass->m_sListen, cpClass->m_hCompletionPort, cpClass->m_funPos, cpNode))
						Sleep(1);
				}
				else if(dwRetCount != cpNode->wsaBuf.len)
				{
					cpNode->lActFlag = 2;
					cpNode->wsaBuf.buf += dwRetCount;
					cpNode->wsaBuf.len -= dwRetCount;

					if(WSARecv(cpNode->sSocket, (LPWSABUF)&cpNode->wsaBuf, 1, &dwCommNum, &dwFlag, (LPWSAOVERLAPPED)cpNode, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
					{
						printf("Thread_CommManager, recv body failed, code = %d\n", WSAGetLastError());
						
						UC::CloseSocket(cpNode->sSocket);
						cpNode->lActFlag = 0;
						while(UC::RunAcceptEx(cpClass->m_sListen, cpClass->m_hCompletionPort, cpClass->m_funPos, cpNode))
							Sleep(1);
					}
				}
				else
				{
					// 处理结果(verifyhead time struct count buffer verifytail)
					char* p = cpNode->szpCommBuf;
					
					// judge time
					if(*(long*)(p + 8) != cpClass->m_lStartTime)
						return -1;
					// judge verify
					if(*(unsigned __int64*)p != VERIFYHEAD || *(unsigned __int64*)(p + cpNode->lCommBufLen - 8) != VERIFYTAIL)
						return -1;
					// judge sync count
					SYNC_NODE* cpSyncNode = (SYNC_NODE*)(p + 12);
					EnterCriticalSection(&cpSyncNode->cNodeCriSec);
					if(*(long*)(p + 16) == cpSyncNode->lCycleCount)
					{
						cpSyncNode->lRecvLen = cpNode->lCommBufLen - 28;
						memcpy(cpSyncNode->szpRecvBuf, p + 20, cpSyncNode->lRecvLen);
						cpSyncNode->lRecv_Fin = 0;
					}
					LeaveCriticalSection(&cpSyncNode->cNodeCriSec);

					SetEvent(cpSyncNode->hEvent);

					// 接收下一个请求
					cpNode->lActFlag = 1;
					cpNode->wsaBuf.buf = cpNode->szpCommBuf;
					cpNode->wsaBuf.len = 4;
					if(WSARecv(cpNode->sSocket, (LPWSABUF)&cpNode->wsaBuf, 1, &dwCommNum, &dwFlag, (LPWSAOVERLAPPED)cpNode, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
					{
						printf("Thread_CommManager, recv head failed, code = %d\n", WSAGetLastError());
						
						UC::CloseSocket(cpNode->sSocket);
						cpNode->lActFlag = 0;
						while(UC::RunAcceptEx(cpClass->m_sListen, cpClass->m_hCompletionPort, cpClass->m_funPos, cpNode))
							Sleep(1);
					}
				}
				break;

			case 3: // 发送
				if(dwRetCount == 0)
				{
					printf("Thread_CommManager, send head failed, code = %d\n", WSAGetLastError());
					
					UC::CloseSocket(cpNode->sSocket);
					while(UC::ConnectServer_IOCP(cpClass->m_szppIPList[cpNode->lMacNo], cpClass->m_uspSendPort[cpNode->lMacNo], cpClass->m_hCompletionPort, cpNode->sSocket))
						Sleep(1);
					cpClass->m_cpSendQue[cpNode->lMacNo].PushData(cpNode);
				}
				else if(dwRetCount != cpNode->wsaBuf.len)
				{
					cpNode->lActFlag = 3;
					cpNode->wsaBuf.buf += dwRetCount;
					cpNode->wsaBuf.len -= dwRetCount;

					if(WSASend(cpNode->sSocket, (LPWSABUF)&cpNode->wsaBuf, 1, &dwCommNum, 0, (LPWSAOVERLAPPED)cpNode, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
					{
						printf("Thread_CommManager, send head failed, code = %d\n", WSAGetLastError());

						UC::CloseSocket(cpNode->sSocket);
						while(UC::ConnectServer_IOCP(cpClass->m_szppIPList[cpNode->lMacNo], cpClass->m_uspSendPort[cpNode->lMacNo], cpClass->m_hCompletionPort, cpNode->sSocket))
							Sleep(1);
						cpClass->m_cpSendQue[cpNode->lMacNo].PushData(cpNode);
					}
				}
				else
				{
					cpClass->m_cpSendQue[cpNode->lMacNo].PushData(cpNode);
				}
				break;
			}
		}

		return 0;
	}

public:
	unsigned short m_usRecvPort;

	long m_lCommThreadNum;
	
	long m_lMaxSendBufLen;
	long m_lMaxRecvBufLen;
	long m_lMaxBufLen;
		
	long m_lMachineNum;
	char** m_szppIPList;
	unsigned short* m_uspSendPort;
	long m_lChannelNumPerMac;

	unsigned __int64* m_ui64p_MacIndex;
	long* m_lpMacPos;
	
	SOCKET m_sListen;
	HANDLE m_hCompletionPort;	
	ACCEPTEX m_funPos;

	long m_lRecvTimeOut;
	long m_lSendTimeOut;

	OVERLAPPEDPLUS_NODE* m_cpRecvLib;
	OVERLAPPEDPLUS_NODE* m_cpSendLib;

	UT_Queue_Event<OVERLAPPEDPLUS_NODE*>* m_cpSendQue;

	long m_lStartTime;

	SYNC_NODE* m_cpSyncLib;
	UT_Queue_Event<SYNC_NODE*> m_cSyncQue;

	UT_Arithmetic<unsigned __int64, long> m_cArithmetic;
};

#endif // _UC_COMMMANAGER_H_
