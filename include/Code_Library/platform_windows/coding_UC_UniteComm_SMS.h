// UC_UniteComm_SMS.h

#ifndef _UC_UNITE_COMM_SMS_H_
#define _UC_UNITE_COMM_SMS_H_

#include "UC_Communication.h"
#include "UC_ReadConfigFile.h"
#include "UT_Queue_Event.h"

#define MAXMACHINENUM	64
#define SENDQUEUESIZE	60

typedef struct _Unite_Comm_SMS_Config_Info_
{
	long lOverLappedNum;       // 重叠结构个数

	long lSendBufSize;         // 发送缓冲区大小
	long lRecvBufSize;         // 接收缓冲区大小
	long lRecvThreadNum;       // 接收线程数
	unsigned short usRecvPort; // 接收端口号

	long lServerGroupNum;	
	long* lpSendMacNum;          // 发送机器数量
	char*** szpppSendMacIP;      // 发送机器IP
	long* lpSendBufSize;         // 发送缓冲区大小
	long* lpRecvBufSize;         // 接收缓冲区大小
	long* lpSendThreadNum;       // 发送线程数(Pre IP)
	unsigned short* uspSendPort; // 发送端口号

	long InitConfigInfo(char* configfile)
	{
		UC_ReadConfigFile cConfig;
		if(cConfig.InitConfigFile(configfile))
			return -1;

		if(cConfig.GetFieldValue("UNIT_COMM_OVER_LAPPED_NUM", lOverLappedNum))
			return -1;

		if(cConfig.GetFieldValue("UNIT_COMM_SEND_BUF_SIZE", lSendBufSize))
			return -1;
		if(cConfig.GetFieldValue("UNIT_COMM_RECV_BUF_SIZE", lRecvBufSize))
			return -1;
		if(cConfig.GetFieldValue("UNIT_COMM_RECV_THREAD_NUM", lRecvThreadNum))
			return -1;
		if(cConfig.GetFieldValue("UNIT_COMM_RECV_PORT", usRecvPort))
			return -1;

		if(cConfig.GetFieldValue("UNIT_COMM_SERVER_GROUP_NUM", lServerGroupNum))
			return -1;
		lpSendMacNum = new long[lServerGroupNum];
		lpSendBufSize = new long[lServerGroupNum];
		lpRecvBufSize = new long[lServerGroupNum];
		lpSendThreadNum = new long[lServerGroupNum];
		uspSendPort = new unsigned short[lServerGroupNum];
		szpppSendMacIP = new char**[lServerGroupNum];
		if(lpSendMacNum == NULL || lpSendBufSize == NULL || lpRecvBufSize == NULL || 
		   lpSendThreadNum == NULL || uspSendPort == NULL || szpppSendMacIP == NULL)
		{
			if(lpSendMacNum) delete lpSendMacNum;
			if(lpSendBufSize) delete lpSendBufSize;
			if(lpRecvBufSize) delete lpRecvBufSize;
			if(lpSendThreadNum) delete lpSendThreadNum;
			if(uspSendPort) delete uspSendPort;
			if(szpppSendMacIP) delete szpppSendMacIP;
			
			return -1;
		}
		char configname[128];
		for(long i = 0; i < lServerGroupNum; i++)
		{
			sprintf(configname, "UNIT_COMM_GROUP_%.2d_SEND_MAC_NUM", i);
			if(cConfig.GetFieldValue(configname, lpSendMacNum[i]))
				return -1;
			if(lpSendMacNum[i] > MAXMACHINENUM)
				return -1;
			sprintf(configname, "UNIT_COMM_GROUP_%.2d_SEND_BUF_SIZE", i);
			if(cConfig.GetFieldValue(configname, lpSendBufSize[i]))
				return -1;
			sprintf(configname, "UNIT_COMM_GROUP_%.2d_RECV_BUF_SIZE", i);
			if(cConfig.GetFieldValue(configname, lpRecvBufSize[i]))
				return -1;
			sprintf(configname, "UNIT_COMM_GROUP_%.2d_SEND_PORT", i);
			if(cConfig.GetFieldValue(configname, uspSendPort[i]))
				return -1;
			sprintf(configname, "UNIT_COMM_GROUP_%.2d_SEND_THREAD_NUM", i);
			if(cConfig.GetFieldValue(configname, lpSendThreadNum[i]))
				return -1;
			szpppSendMacIP[i] = new char*[lpSendMacNum[i]];
			if(szpppSendMacIP[i] == NULL)
				return -1;
			for(long j = 0; j < lpSendMacNum[i]; j++)
			{
				szpppSendMacIP[i][j] = new char[16];
				sprintf(configname, "UNIT_COMM_GROUP_%.2d_MAC_IP_%.2d", i, j);
				if(cConfig.GetFieldValue(configname, szpppSendMacIP[i][j]))
					return -1;
			}
		}

		return 0;
	}
} UNITE_COMM_SMS_CONFIG_INFO;

typedef struct _Unite_Comm_SMS_Overlapped_
{
	// 标准结构
	OVERLAPPED cOverlapped;
	SOCKET sClient;	
	char szaBuffer[IPADDRESSLEN<<1];
	char szDealType;

	WSABUF wsaBuf;

	// 服务端接收发送缓冲区
	char* szpRecvBuf;
	long  lRecvFinLen;
	long  lRecvRemLen;

	char* szpSendBuf;
	long  lSendFinLen;
	long  lSendRemLen;

	// 机器数量计数
	long lRetNum;
	long lFinNum;
	CRITICAL_SECTION cCriSec;
	
	// 客户端发送缓冲区
	char* szpSendBuf_Server;
	long  lSendLen_Server;

	// 客户端接收缓冲区
	long lAllRecvLen;
	char* szpaRecvBuf[MAXMACHINENUM];
	long laRecvBufLen[MAXMACHINENUM];
	long laAllocFlag[MAXMACHINENUM];

	_Unite_Comm_SMS_Overlapped_()
	{
		sClient = -1;

		InitializeCriticalSection(&cCriSec);
	}
	~_Unite_Comm_SMS_Overlapped_()
	{
		DeleteCriticalSection(&cCriSec);
	}
} UNITE_COMM_SMS_OVERLAPPED;

class UC_UnitComm
{
public:
	long _AcceptEx(SOCKET sListen, UNITE_COMM_SMS_OVERLAPPED* cpNode, ACCEPTEX funPos)
	{
		if(cpNode->sClient != -1)
			closesocket(cpNode->sClient);

		cpNode->sClient = socket(AF_INET, SOCK_STREAM, 0);
		if(cpNode->sClient == INVALID_SOCKET)
			return -1;
		memset(&(cpNode->cOverlapped), 0, sizeof(OVERLAPPED));
		memset(cpNode->szaBuffer, 0, (sizeof(SOCKADDR_IN) + 16)<<1);	
		cpNode->szDealType = 0;

		if(CreateIoCompletionPort((HANDLE)cpNode->sClient, m_hCompletionPort , 0, 0) == NULL)
		{
			printf("CreateIoCompletionPort() failed\n");
			return -1;
		}

		DWORD dwRetVal = 0;
		if(!(funPos(sListen, cpNode->sClient, cpNode->szaBuffer, 0, IPADDRESSLEN, IPADDRESSLEN, &dwRetVal, &(cpNode->cOverlapped))))
		{
			if(GetLastError() != ERROR_IO_PENDING)
				return -1;
		}

		return 0;
	}
	UC_UnitComm()
	{
		m_hCompletionPort = NULL;
		m_sListen = -1;
		m_cpOverLapped = NULL;
	}
	~UC_UnitComm()
	{
		if(m_hCompletionPort)
			CloseHandle(m_hCompletionPort);
		if(m_sListen != -1)
			UC::CloseSocket(m_sListen);
		if(m_cpOverLapped)
			delete m_cpOverLapped;
	}
	long InitUniteComm(char* configfile)
	{
		if(m_cConfigInfo.InitConfigInfo(configfile))
			return -1;
		if(InitClientComm())
			return -1;
		if(InitServerComm())
			return -1;
		printf("Init Unite Comm OK!\n");

		return 0;
	}
	long InitServerComm()
	{
		// 创建监听端口
		if(UC::InitSocket())
			return -1;
		// 创建完成端口并绑定
		if(UC::CreateListen_IOCP(m_cConfigInfo.usRecvPort, m_sListen, m_hCompletionPort))
			return -1;
		// 取AcceptEx函数指针
		if(GetAcceptEx(m_sListen, m_funPos))
		{
			CloseHandle(m_hCompletionPort);
			UC::CloseSocket(m_sListen);		
			return -1;
		}
		// 初始化接收完成队列
		if(m_cRecvFinQue.InitQueue(m_cConfigInfo.lOverLappedNum))
		{
			CloseHandle(m_hCompletionPort);
			UC::CloseSocket(m_sListen);		
			return -1;
		}
		// 初始化接收及发送缓冲区
		char* tmpbuf = NULL;

		if(m_cRecvBufQue.InitQueue(m_cConfigInfo.lOverLappedNum))
		{
			CloseHandle(m_hCompletionPort);
			UC::CloseSocket(m_sListen);		
			return -1;
		}
		tmpbuf = new char[m_cConfigInfo.lRecvBufSize * m_cConfigInfo.lOverLappedNum];
		if(tmpbuf == NULL)
		{
			CloseHandle(m_hCompletionPort);
			UC::CloseSocket(m_sListen);		
			return -1;
		}
		for(long m = 0; m < m_cConfigInfo.lOverLappedNum; m++)
			m_cRecvBufQue.PushData(tmpbuf + m_cConfigInfo.lRecvBufSize * m);
		
		if(m_cSendBufQue.InitQueue(m_cConfigInfo.lOverLappedNum))
		{
			CloseHandle(m_hCompletionPort);
			UC::CloseSocket(m_sListen);		
			return -1;
		}
		tmpbuf = new char[m_cConfigInfo.lSendBufSize * m_cConfigInfo.lOverLappedNum];
		if(tmpbuf == NULL)
		{
			CloseHandle(m_hCompletionPort);
			UC::CloseSocket(m_sListen);		
			return -1;
		}
		for(long n = 0; n < m_cConfigInfo.lOverLappedNum; n++)
			m_cSendBufQue.PushData(tmpbuf + m_cConfigInfo.lSendBufSize * n);
		// 投递完成结构
		m_cpOverLapped = new UNITE_COMM_SMS_OVERLAPPED[m_cConfigInfo.lOverLappedNum];
		if(m_cpOverLapped == NULL)
		{
			CloseHandle(m_hCompletionPort);
			UC::CloseSocket(m_sListen);	
			return -1;
		}
		for(long i = 0; i < m_cConfigInfo.lOverLappedNum; i++)
		{
			if(_AcceptEx(m_sListen, m_cpOverLapped + i, m_funPos))
			{
				CloseHandle(m_hCompletionPort);
				UC::CloseSocket(m_sListen);
				return -1;
			}
		}
		// 启动工作线程
		for(long j = 0; j < m_cConfigInfo.lRecvThreadNum; j++)
		{
			if(UC::StartThread(thread_ServerComm？。mnbvcxz, this) == 0)
				continue;
			CloseHandle(m_hCompletionPort);
			UC::CloseSocket(m_sListen);
			return -1;
		}	
		// 初始化成功
		printf("Init Server Comm OK!\n");

		return 0;
	}
	long InitClientComm()
	{
		// 初始化发送及接收完成队列, 启动线程
		m_cpRecvFinQue = new UT_Queue_Event<UNITE_COMM_SMS_OVERLAPPED*>[m_cConfigInfo.lServerGroupNum];
		if(m_cpRecvFinQue == NULL)
			return -1;
		m_cppSendQue = new UT_Queue_Event<UNITE_COMM_SMS_OVERLAPPED*>*[m_cConfigInfo.lServerGroupNum];
		if(m_cppSendQue == NULL)
			return -1;		
		for(long i = 0; i < m_cConfigInfo.lServerGroupNum; i++)
		{
			if(m_cpRecvFinQue[i].InitQueue(m_cConfigInfo.lOverLappedNum))
				return -1;

			m_cppSendQue[i] = new UT_Queue_Event<UNITE_COMM_SMS_OVERLAPPED*>[m_cConfigInfo.lServerGroupNum];
			if(m_cppSendQue[i] == NULL)
				return -1;
			for(long j = 0; j < m_cConfigInfo.lpSendMacNum[i]; j++)
			{
				if(m_cppSendQue[i][j].InitQueue(m_cConfigInfo.lOverLappedNum))
					return -1;
				
				m_lTmpStartFin = 0; m_lTmpGroupNo = i; m_lTmpMachineNo = j;
				if(UC::StartThread(thread_ClientComm, this))
					return -1;
				while(m_lTmpStartFin == 0)
					Sleep(10);
			}
		}
		// 初始化接收发送缓冲区
		char* tmpbuf = NULL;

		m_cpSendBufQue = new UT_Queue_Event<char*>[m_cConfigInfo.lServerGroupNum];
		if(m_cpSendBufQue == NULL)
			return -1;
		m_cpRecvBufQue = new UT_Queue_Event<char*>[m_cConfigInfo.lServerGroupNum];
		if(m_cpRecvBufQue == NULL)
			return -1;
		for(long x = 0; x < m_cConfigInfo.lServerGroupNum; x++)
		{
			tmpbuf = new char[m_cConfigInfo.lpSendBufSize[x] * m_cConfigInfo.lOverLappedNum];
			if(tmpbuf == NULL)
				return -1;
			if(m_cpSendBufQue[x].InitQueue(m_cConfigInfo.lOverLappedNum))
				return -1;
			for(long m = 0; m < m_cConfigInfo.lOverLappedNum; m++)
				m_cpSendBufQue[x].PushData(tmpbuf + m_cConfigInfo.lpSendBufSize[x] * m);

			tmpbuf = new char[m_cConfigInfo.lpRecvBufSize[x] * m_cConfigInfo.lOverLappedNum * m_cConfigInfo.lpSendMacNum[x]];
			if(tmpbuf == NULL)
				return -1;
			if(m_cpRecvBufQue[x].InitQueue(m_cConfigInfo.lOverLappedNum * m_cConfigInfo.lpSendMacNum[x]))
				return -1;
			for(long n = 0; n < m_cConfigInfo.lOverLappedNum * m_cConfigInfo.lpSendMacNum[x]; n++)
				m_cpRecvBufQue[x].PushData(tmpbuf + m_cConfigInfo.lpRecvBufSize[x] * n);
		}		
		// 初始化成功
		printf("Init Client Comm OK!\n");

		return 0;
	}
	static DWORD WINAPI thread_ServerComm(void* vpArg)
	{
		UC_UnitComm* cpClass = (UC_UnitComm*)vpArg;

		DWORD dwCompKey = 0;
		DWORD dwRecvNum = 0;
		DWORD dwSendNum = 0;
		DWORD dwRetCount = 0;
		UNITE_COMM_SMS_OVERLAPPED* cpNode = NULL;

		for(;;)
		{
			if(!GetQueuedCompletionStatus(cpClass->m_hCompletionPort, &dwRetCount, &dwCompKey, (OVERLAPPED**)&cpNode, INFINITE))
			{
				if(cpNode)
				{
					while(cpClass->_AcceptEx(cpClass->m_sListen, cpNode, cpClass->m_funPos))
						Sleep(1);
				}
				Sleep(1);
				continue;
			}

			if(dwCompKey)
				break;

			DWORD dwFlag = 0;
			switch(cpNode->szDealType) 
			{
			case 0: // 连接成功, 进行第一次接收
				cpNode->szpRecvBuf = cpClass->m_cRecvBufQue.PopData();
				cpNode->szDealType = 1;
				cpNode->lRecvFinLen = 0;			
				cpNode->wsaBuf.buf = cpNode->szpRecvBuf;
				cpNode->wsaBuf.len = 4;
				cpNode->lRecvRemLen = cpNode->wsaBuf.len;
				if(WSARecv(cpNode->sClient, (LPWSABUF)&cpNode->wsaBuf, 1, &dwRecvNum, &dwFlag, (LPWSAOVERLAPPED)cpNode, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
				{
					cpClass->m_cRecvBufQue.PushData(cpNode->szpRecvBuf);
					printf("%d-1.WSARecv() failed, code = %u\n", cpNode->szDealType, GetLastError());
					while(cpClass->_AcceptEx(cpClass->m_sListen, cpNode, cpClass->m_funPos))
						Sleep(1);
				}
				break;

			case 1: // 进行第二次接收
				if(dwRetCount == 0)
				{
					cpClass->m_cRecvBufQue.PushData(cpNode->szpRecvBuf);
					printf("%d-1: WSARecv() failed, code = %u\n", cpNode->szDealType, GetLastError());
					while(cpClass->_AcceptEx(cpClass->m_sListen, cpNode, cpClass->m_funPos))
						Sleep(1);
				}
				else if(dwRetCount != (unsigned)cpNode->lRecvRemLen)
				{
					cpNode->szDealType = 1;
					cpNode->lRecvFinLen += dwRetCount;				
					cpNode->wsaBuf.buf = cpNode->szpRecvBuf + cpNode->lRecvFinLen;
					cpNode->wsaBuf.len = cpNode->lRecvRemLen - dwRetCount;
					cpNode->lRecvRemLen = cpNode->wsaBuf.len;
					if(WSARecv(cpNode->sClient, (LPWSABUF)&cpNode->wsaBuf, 1, &dwRecvNum, &dwFlag, (LPWSAOVERLAPPED)cpNode, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
					{
						cpClass->m_cRecvBufQue.PushData(cpNode->szpRecvBuf);
						printf("%d-2.WSARecv() failed, code = %u\n", cpNode->szDealType, GetLastError());
						while(cpClass->_AcceptEx(cpClass->m_sListen, cpNode, cpClass->m_funPos))
							Sleep(1);
					}
				}
				else
				{
					cpNode->szDealType = 2;
					cpNode->lRecvFinLen += dwRetCount;
					cpNode->wsaBuf.buf = cpNode->szpRecvBuf + cpNode->lRecvFinLen;
					cpNode->wsaBuf.len = *(long*)cpNode->szpRecvBuf;
					cpNode->lRecvRemLen = cpNode->wsaBuf.len;
					if(WSARecv(cpNode->sClient, (LPWSABUF)&cpNode->wsaBuf, 1, &dwRecvNum, &dwFlag, (LPWSAOVERLAPPED)cpNode, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
					{
						cpClass->m_cRecvBufQue.PushData(cpNode->szpRecvBuf);
						printf("%d-3.WSARecv() failed, code = %u\n", cpNode->szDealType, GetLastError());
						while(cpClass->_AcceptEx(cpClass->m_sListen, cpNode, cpClass->m_funPos))
							Sleep(1);
					}
				}
				break;

			case 2:
				if(dwRetCount == 0)
				{
					cpClass->m_cRecvBufQue.PushData(cpNode->szpRecvBuf);
					printf("%d-1: WSARecv() failed, code = %u\n", cpNode->szDealType, GetLastError());
					while(cpClass->_AcceptEx(cpClass->m_sListen, cpNode, cpClass->m_funPos))
						Sleep(1);
				}
				if(dwRetCount != (unsigned)cpNode->lRecvRemLen)
				{
					cpNode->szDealType = 2;
					cpNode->lRecvFinLen += dwRetCount;				
					cpNode->wsaBuf.buf = cpNode->szpRecvBuf + cpNode->lRecvFinLen;
					cpNode->wsaBuf.len = cpNode->lRecvRemLen - dwRetCount;
					cpNode->lRecvRemLen = cpNode->wsaBuf.len;
					if(WSARecv(cpNode->sClient, (LPWSABUF)&cpNode->wsaBuf, 1, &dwRecvNum, &dwFlag, (LPWSAOVERLAPPED)cpNode, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
					{
						cpClass->m_cRecvBufQue.PushData(cpNode->szpRecvBuf);
						printf("%d-2.WSARecv() failed, code = %u\n", cpNode->szDealType, GetLastError());
						while(cpClass->_AcceptEx(cpClass->m_sListen, cpNode, cpClass->m_funPos))
							Sleep(1);
					}
				}
				else
				{
					cpNode->szDealType = -1;
					cpNode->lRecvFinLen += dwRetCount;
					cpClass->m_cRecvFinQue.PushData(cpNode);
				}
				break;

			case 4:
				if(dwRetCount == 0)
				{
					cpClass->m_cSendBufQue.PushData(cpNode->szpSendBuf);
					printf("%d-1: WSASend() failed, code = %u\n", cpNode->szDealType, GetLastError());
					while(cpClass->_AcceptEx(cpClass->m_sListen, cpNode, cpClass->m_funPos))
						Sleep(1);
				}
				if(dwRetCount != (unsigned)cpNode->lSendRemLen)
				{
					cpNode->szDealType = 4;
					cpNode->lSendFinLen += dwRetCount;
					cpNode->wsaBuf.buf = cpNode->szpSendBuf + cpNode->lSendFinLen;
					cpNode->wsaBuf.len = cpNode->lSendRemLen - dwRetCount;
					cpNode->lSendRemLen = cpNode->wsaBuf.len;
					if(WSASend(cpNode->sClient, (LPWSABUF)&cpNode->wsaBuf, 1, &dwSendNum, 0, (LPWSAOVERLAPPED)cpNode, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
					{
						cpClass->m_cSendBufQue.PushData(cpNode->szpSendBuf);
						printf("%d-2.WSASend() failed, code = %u\n", cpNode->szDealType, GetLastError());
						while(cpClass->_AcceptEx(cpClass->m_sListen, cpNode, cpClass->m_funPos))
							Sleep(1);
					}
				}
				else
				{
					cpClass->m_cSendBufQue.PushData(cpNode->szpSendBuf);
					while(cpClass->_AcceptEx(cpClass->m_sListen, cpNode, cpClass->m_funPos))
						Sleep(1);
				}
				break;
			}
		}

		return 0;
	}
	static DWORD WINAPI thread_ClientComm(void* vpArg)
	{
		UC_UnitComm* cpClass = (UC_UnitComm*)vpArg;

		long lGrpNo = cpClass->m_lTmpGroupNo;
		long lMacNo = cpClass->m_lTmpMachineNo;
		cpClass->m_lTmpStartFin = 1;
		
		char* szpMacIP = cpClass->m_cConfigInfo.szpppSendMacIP[lGrpNo][lMacNo];
		unsigned short usMacPort = cpClass->m_cConfigInfo.uspSendPort[lGrpNo];
		UT_Queue_Event<UNITE_COMM_SMS_OVERLAPPED*>* cpQueue = &cpClass->m_cppSendQue[lGrpNo][lMacNo];

		SOCKET sClient;
		SOCKADDR_IN cClientAddress;
		cClientAddress.sin_family = AF_INET;
		cClientAddress.sin_addr.s_addr = inet_addr(szpMacIP);
		cClientAddress.sin_port = htons(usMacPort);
		
		UNITE_COMM_SMS_OVERLAPPED* cpNode = NULL;

		for(;;)
		{
			cpNode = cpQueue->PopData();

			long lErrorFlag = 0;

			if(UC::ConnectServer(szpMacIP, usMacPort, sClient))
			{
				lErrorFlag = 1;
				printf("connect %s:%d error, code = %d\n", szpMacIP, usMacPort, GetLastError());				
			}
			UC::SetSocketOverTime(sClient, 5000);
			if(lErrorFlag == 0 && UC::SendBuffer(sClient, cpNode->szpSendBuf_Server, cpNode->lSendLen_Server))
			{
				lErrorFlag = 2;
				printf("send buffer to %s:%d error, code = %d\n", szpMacIP, usMacPort, GetLastError());
			}
			if(lErrorFlag == 0 && UC::RecvBuffer(sClient, (char*)&cpNode->laRecvBufLen[lMacNo], 4))
			{
				lErrorFlag = 3;
				printf("recv buffer from %s:%d error, code = %d\n", szpMacIP, usMacPort, GetLastError());
			}
			if(lErrorFlag != 0) // 通讯错误
			{
				cpNode->laRecvBufLen[lMacNo] = 0;
				cpNode->szpaRecvBuf[lMacNo] = NULL;
			}
			else if(cpNode->laRecvBufLen[lMacNo] == 0 || cpNode->laRecvBufLen[lMacNo] > cpClass->m_cConfigInfo.lpRecvBufSize[lGrpNo]) // 
			{
				lErrorFlag = -1;

				cpNode->laRecvBufLen[lMacNo] = 0;
				cpNode->szpaRecvBuf[lMacNo] = NULL;
			}
			else
			{
				cpNode->szpaRecvBuf[lMacNo] = cpClass->m_cpRecvBufQue[lGrpNo].PopData();
			}
			if(lErrorFlag == 0 && cpNode->laRecvBufLen[lMacNo] && UC::RecvBuffer(sClient, cpNode->szpaRecvBuf[lMacNo], cpNode->laRecvBufLen[lMacNo]))
			{
				lErrorFlag = 4;
				
				cpClass->m_cpRecvBufQue[lGrpNo].PushData(cpNode->szpaRecvBuf[lMacNo]);

				cpNode->laRecvBufLen[lMacNo] = 0;
				cpNode->szpaRecvBuf[lMacNo] = NULL;

				printf("recv buffer from %s:%d error\n", szpMacIP, usMacPort);
			}
			if(lErrorFlag != 1)
				UC::CloseSocket(sClient);

			long lActiveFlg = 0;
			EnterCriticalSection(&cpNode->cCriSec);
			cpNode->lAllRecvLen += cpNode->laRecvBufLen[lMacNo];
			cpNode->lRetNum++;
			if(cpNode->lRetNum == cpNode->lFinNum)
				lActiveFlg = 1;
			LeaveCriticalSection(&cpNode->cCriSec);		
			
			if(lActiveFlg == 1)
				cpClass->m_cpRecvFinQue[lGrpNo].PushData(cpNode);
		}

		return 0;
	}
/*
	long RecvData_Server()
	{
		UNITE_COMM_SMS_OVERLAPPED* cpNode = m_cRecvFinQue.PopData();		
		return 0;
	}
	long SendData_Server()
	{
		return 0;
	}
	long RecvData_Client(long lGroupNo)
	{
		return 0;
	}	
	long SendData_Client(long lGroupNo)
	{
		return 0;
	}

	long DisposeData(UNITE_COMM_SMS_OVERLAPPED* cpNode)
	{
		if(cpNode->lAllRecvLen > m_cConfigInfo.lMaxBufferSize)
		{
			cpNode->lSendBufFlg = 1;
			cpNode->szpSendBuf = new char[cpNode->lAllRecvLen];
		}
		else
		{
			cpNode->lSendBufFlg = 0;
			cpNode->szpSendBuf = (char *)(m_cpRecvBufQueue->PopData());
		}

		if(cpNode->lSearchType == 1)
		{
			long lRetNum = 0, lRetLen = 0;
			char* p = cpNode->szpSendBuf + 24;
			for(long i = 0; i < m_cConfigInfo.lMachineNum; i++)
			{
				if(cpNode->laAllocFlag[i] == -1)
				{
					printf("%s:%d, No Return\n", m_cConfigInfo.szppMachineIP[i], m_cConfigInfo.usSendPort);
					continue;
				}
				else
					printf("%s:%d, Return %d\n", m_cConfigInfo.szppMachineIP[i], m_cConfigInfo.usSendPort, atol(cpNode->szpaRecvBuf[i] + 8));
				
				lRetNum += atol(cpNode->szpaRecvBuf[i] + 8); 
				lRetLen = atol(cpNode->szpaRecvBuf[i] + 16);
				memcpy(p, cpNode->szpaRecvBuf[i] + 24, lRetLen);
				p += lRetLen;

				if(cpNode->laAllocFlag[i] == 0)
					m_cpRecvBufQueue->PushData(cpNode->szpaRecvBuf[i]);
				else
					delete cpNode->szpaRecvBuf[i];
			}
			lRetLen = p - cpNode->szpSendBuf - 24;
			*(unsigned __int64*)cpNode->szpSendBuf = cpNode->ui64_mask;
			if(lRetLen == 0)
			{
				memcpy(cpNode->szpSendBuf + 8, "0       0       ", 16);
				cpNode->lSendBufLen = 24;
			}
			else
			{			
				char buffer[32];
				sprintf(buffer, "%d       ", lRetNum);
				sprintf(buffer + 8, "%d       ", lRetLen);
				memcpy(cpNode->szpSendBuf + 8, buffer, 16);
				cpNode->lSendBufLen = lRetLen + 24;
			}
		}
		else
		{
			for(long i = 0; i < m_cConfigInfo.lMachineNum; i++)
			{
				if(cpNode->laAllocFlag[i] == -1)
					continue;
				break;
			}
			if(i == m_cConfigInfo.lMachineNum)
			{
				*(unsigned __int64*)cpNode->szpSendBuf = cpNode->ui64_mask;
				if(cpNode->lSearchType == 2 || cpNode->lSearchType == 3)
				{
					memcpy(cpNode->szpSendBuf + 8, "0       0       0       0       ", 32);
					cpNode->lSendBufLen = 40;
				}
				else
				{
					*(unsigned __int64*)(cpNode->szpSendBuf + 8) = *(unsigned __int64*)"0       ";
					cpNode->lSendBufLen = 16;
				}
			}
			else
			{
				memcpy(cpNode->szpSendBuf, cpNode->szpaRecvBuf[i], cpNode->laRecvBufLen[i]);
				cpNode->lSendBufLen = cpNode->laRecvBufLen[i];
				if(cpNode->laAllocFlag[i] == 0)
					m_cpRecvBufQueue->PushData(cpNode->szpaRecvBuf[i]);
				else
					delete cpNode->szpaRecvBuf[i];
			}
		}

		cpNode->szDealType = 4;
		cpNode->lFinishLen = 0;
		cpNode->wsaBuf.buf = cpNode->szpSendBuf;
		cpNode->wsaBuf.len = cpNode->lSendBufLen;
		cpNode->lCommLen = cpNode->wsaBuf.len;
		
		DWORD dwSendNum = 0;
		if(WSASend(cpNode->sClient, (LPWSABUF)&cpNode->wsaBuf, 1, &dwSendNum, 0, (LPWSAOVERLAPPED)cpNode, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			if(cpNode->lSendBufFlg == 0)
				m_cpRecvBufQueue->PushData(cpNode->szpSendBuf);
			else
				delete cpNode->szpSendBuf;

			printf("WSASend() failed, code = %u\n", GetLastError());
			while(_AcceptEx(m_sListen, cpNode, m_funPos))
				Sleep(1);
		}

		return 0;
	}
*/
public:
	// Config
	UNITE_COMM_SMS_CONFIG_INFO m_cConfigInfo;

	// Server
	HANDLE m_hCompletionPort;
	SOCKET m_sListen;
	ACCEPTEX m_funPos;
	UNITE_COMM_SMS_OVERLAPPED* m_cpOverLapped;

	UT_Queue_Event<char*> m_cRecvBufQue;
	UT_Queue_Event<char*> m_cSendBufQue;
	UT_Queue_Event<UNITE_COMM_SMS_OVERLAPPED*> m_cRecvFinQue;

	// Client
	long m_lTmpStartFin;
	long m_lTmpGroupNo;
	long m_lTmpMachineNo;

	UT_Queue_Event<UNITE_COMM_SMS_OVERLAPPED*>** m_cppSendQue;

	UT_Queue_Event<char*>* m_cpSendBufQue;
	UT_Queue_Event<char*>* m_cpRecvBufQue;
	UT_Queue_Event<UNITE_COMM_SMS_OVERLAPPED*>* m_cpRecvFinQue;
};

#endif // _UC_UNITE_COMM_SMS_H_