// UC_Communication.h

#ifndef _UC_COMMUNICATION_H_
#define _UC_COMMUNICATION_H_

#include <winsock2.h>
#pragma comment(lib, "ws2_32")
#include <mswsock.h>

#include <windows.h>

typedef BOOL (PASCAL FAR *ACCEPTEX)(SOCKET, SOCKET, PVOID, DWORD, DWORD, DWORD, LPDWORD, LPOVERLAPPED);
typedef void (PASCAL FAR *GETACCEPTEXSOCKADDRS)(PVOID, DWORD, DWORD, DWORD, LPSOCKADDR*, LPINT, LPSOCKADDR*, LPINT);

typedef ACCEPTEX				LPFN_ACCEPTEX;
typedef GETACCEPTEXSOCKADDRS	LPFN_GETACCEPTEXSOCKADDRS;

#define IPADDRESSLEN	(sizeof(SOCKADDR_IN) + 16)

typedef struct _OVERLAPPED_PLUS_
{
	OVERLAPPED cOverlapped;
	SOCKET     sSocket;
	char       szaBuffer[IPADDRESSLEN<<1];

	_OVERLAPPED_PLUS_()
	{
		sSocket = INVALID_SOCKET;
	}
} UC_OVERLAPPEDPLUS;

class UC_Communication
{
public:
	/*************************************************************************
	����ͨѶ����
	*************************************************************************/

	// ����socket��̬��
	static long InitSocket()
	{
		WSAData cWSAData;
		if(WSAStartup(MAKEWORD(2, 2), &cWSAData))
			return -1;

		return 0;
	}
	// ж��socket��̬��
	static long ClearSocket()
	{
		WSACleanup();

		return 0;
	}
	// ��socket
	static long OpenSocket(SOCKET& out_sSocket)
	{
		if(out_sSocket != INVALID_SOCKET)
			closesocket(out_sSocket);
		
		out_sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if(out_sSocket == INVALID_SOCKET)
			return -1;

		return 0;
	}
	// �ر�socket
	static long CloseSocket(SOCKET& sSocket)
	{
		closesocket(sSocket);
		sSocket = INVALID_SOCKET;
		
		return 0;
	}
	// �������󶨼���
	static long BindSocket(SOCKET in_sSocket, unsigned short in_usPort)
	{
		SOCKADDR_IN cSocketAddr;
		cSocketAddr.sin_family = AF_INET;
		cSocketAddr.sin_port = htons(in_usPort);
		cSocketAddr.sin_addr.s_addr = htonl(INADDR_ANY); // or Zero

		if(bind(in_sSocket, (struct sockaddr*)&cSocketAddr, sizeof(cSocketAddr)) == SOCKET_ERROR)
			return -1;

		return 0;
	}
	// ����socket���շ��ͳ�ʱ
	static void SetSocketOverTime(SOCKET sSocket, long lOverTime)
	{
		setsockopt(sSocket, SOL_SOCKET, SO_RCVTIMEO,(char*)&lOverTime, sizeof(lOverTime));
		setsockopt(sSocket, SOL_SOCKET, SO_SNDTIMEO,(char*)&lOverTime, sizeof(lOverTime));
	}
	static void SetSocketOverTime(SOCKET sSocket, long lRecvOverTime, long lSendOverTime)
	{
		setsockopt(sSocket, SOL_SOCKET, SO_RCVTIMEO,(char*)&lRecvOverTime, sizeof(lRecvOverTime));
		setsockopt(sSocket, SOL_SOCKET, SO_SNDTIMEO,(char*)&lSendOverTime, sizeof(lSendOverTime));
	}
	// ���տͻ�������
	static long AcceptSocket(SOCKET in_sListen, SOCKET& out_sClient)
	{
		struct sockaddr in_cClientAddress;
		int iAddressLen = sizeof(struct sockaddr);

		out_sClient = accept(in_sListen, &in_cClientAddress, &iAddressLen);
		if(out_sClient == SOCKET_ERROR)
			return -1;

		return 0;
	}
	static long AcceptSocket(SOCKET in_sListen, SOCKET& out_sClient, struct sockaddr& in_cClientAddress)
	{
		int iAddressLen = sizeof(struct sockaddr);

		out_sClient = accept(in_sListen, &in_cClientAddress, &iAddressLen);
		if(out_sClient == SOCKET_ERROR)
			return -1;

		return 0;
	}
	// ��������, ����
	static long SendBuffer(SOCKET in_sSocket, char* in_szpSendBuf, long in_lSendLen)
	{
		long lRetLen = 0, lFinishLen = 0;
		do{
			lRetLen = send(in_sSocket, in_szpSendBuf + lFinishLen, in_lSendLen - lFinishLen, 0);
			if(lRetLen > 0)
				lFinishLen += lRetLen;
		}while(lRetLen > 0 && lFinishLen < in_lSendLen);
		if(lRetLen < 0 || lFinishLen < in_lSendLen)
			return -1;

		return 0;
	}
	static long RecvBuffer(SOCKET in_sSocket, char* in_szpRecvBuf, long in_lRecvLen)
	{
		long lRetLen = 0, lFinishLen = 0;
		do{
			lRetLen = recv(in_sSocket, in_szpRecvBuf + lFinishLen, in_lRecvLen - lFinishLen, 0);
			if(lRetLen > 0)
				lFinishLen += lRetLen;
		}while(lRetLen > 0 && lFinishLen < in_lRecvLen);
		if(lRetLen < 0 || lFinishLen < in_lRecvLen)
			return -1;

		return 0;
	}
	// �������󶨼���
	static long CreateListen(unsigned short in_usPort, SOCKET& out_sListen)
	{
		SOCKADDR_IN cListenAddr;
		cListenAddr.sin_family = AF_INET;
		cListenAddr.sin_port = htons(in_usPort);
		cListenAddr.sin_addr.s_addr = htonl(INADDR_ANY); // or Zero

		out_sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if(out_sListen == INVALID_SOCKET)
			return -1;
		if(bind(out_sListen, (struct sockaddr*)&cListenAddr, sizeof(cListenAddr)) == SOCKET_ERROR)
		{
			closesocket(out_sListen);
			return -1;
		}
		if(listen(out_sListen, SOMAXCONN) == SOCKET_ERROR)
		{
			closesocket(out_sListen);
			return -1;
		}

		return 0;
	}
	// ���ӵ�������
	static long ConnectServer(char* in_szpIP, unsigned short in_usPort, SOCKET& out_sConnect)
	{
		SOCKADDR_IN cClientAddress;
		cClientAddress.sin_family = AF_INET;
		cClientAddress.sin_addr.s_addr = inet_addr(in_szpIP);
		cClientAddress.sin_port = htons(in_usPort);

		out_sConnect = socket(AF_INET, SOCK_STREAM, 0);
		if(out_sConnect == INVALID_SOCKET)
			return -1;

		if(connect(out_sConnect, (struct sockaddr*)&cClientAddress, sizeof(cClientAddress)) == SOCKET_ERROR)
		{
			closesocket(out_sConnect);
			return -1;
		}

		return 0;
	}
	static long ConnectServer_NoWait(char* in_szpIP, unsigned short in_usPort, SOCKET& out_sConnect)
	{
		SOCKADDR_IN cClientAddress;
		cClientAddress.sin_family = AF_INET;
		cClientAddress.sin_addr.s_addr = inet_addr(in_szpIP);
		cClientAddress.sin_port = htons(in_usPort);

		out_sConnect = socket(AF_INET, SOCK_STREAM, 0);
		if(out_sConnect == INVALID_SOCKET)
			return -1;
		
		//////////////////////////////////////////////////////////////////////////
		// �˶δ��뼤�� - �ᵼ��û����ȫ�շ������͹ر�SOCKET���������
		// �˶δ������� - �ᵼ��SOCKET���Ѿ��رպ�,��ʼ����SOCKET�������ӷ�������������
		//////////////////////////////////////////////////////////////////////////
		BOOL on = TRUE;
		if(setsockopt(out_sConnect, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) == -1)
		{
			closesocket(out_sConnect);
			return -1;
		}
		struct linger ling;
		ling.l_onoff = 1;
		ling.l_linger = 0;
		if(setsockopt(out_sConnect, SOL_SOCKET, SO_LINGER, (const char*)&ling, sizeof(ling)) == -1)
		{
			closesocket(out_sConnect);
			return -1;
		}
		//////////////////////////////////////////////////////////////////////////

		if(connect(out_sConnect, (struct sockaddr*)&cClientAddress, sizeof(cClientAddress)) == SOCKET_ERROR)
		{
			closesocket(out_sConnect);
			return -1;
		}

		return 0;
	}
	// ȡ�����Ӷ�IP��ַ
	static char* GetIpInfo(SOCKET in_sSocket)
	{
		struct sockaddr cIP;
		int iIPLen = sizeof(struct sockaddr);

		if(getpeername(in_sSocket, &cIP, &iIPLen))
			return NULL;

		return inet_ntoa(((struct sockaddr_in*)&cIP)->sin_addr);	
	}

	/*************************************************************************
	IOCPϵ�к����б� - IOCP_CreateIOCP                  - ����IOCP
	                 - IOCP_BindIOCP                    - ��һ��SOCKET�󶨵�һ��IOCP
					 - IOCP_GetConnectEx                - ��ȡConnectEx������ַ
					 - IOCP_GetAcceptEx                 - ��ȡAcceptEx������ַ
					 - IOCP_GetGetAcceptExSockaddrs     - ��ȡGetAcceptExSockaddrs������ַ
					 - IOCP_SendData                    - ʹ��IOCP�첽��������
					 - IOCP_RecvData                    - ʹ��IOCP�첽��������
					 - IOCP_ConnectEx                   - ʹ��IOCP�첽���ӵ������(��ѡ: ��������)
					 - IOCP_CreateListen                - ����������ɶ˿ڰ󶨼���
					 - IOCP_PostOverlapped				- Ͷ���ص��ṹ
					 - IOCP_GetIpInfo					- ȡ�����Ӷ�IP��ַ(��ɶ˿�ģ��)	
	*************************************************************************/

	// ����IOCP
	static long IOCP_CreateIOCP(HANDLE& out_hCompletionPort)
	{
		out_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if(out_hCompletionPort == NULL)
			return -1;

		return 0;
	}
	// ��һ��SOCKET�󶨵�һ��IOCP
	static long IOCP_BindIOCP(SOCKET in_sSocket, HANDLE in_hCompletionPort)
	{
		if(CreateIoCompletionPort((HANDLE)in_sSocket, in_hCompletionPort, 0, 0) == NULL)
			return -1;

		return 0;
	}
	// ��ȡConnectEx������ַ
	static long IOCP_GetConnectEx(LPFN_CONNECTEX& out_fpFunPoint, SOCKET in_sSocket = INVALID_SOCKET)
	{
		DWORD dwRetVal = 0;
		GUID GUIDName = WSAID_CONNECTEX;

		if(in_sSocket == INVALID_SOCKET)
		{
			in_sSocket = socket(AF_INET, SOCK_STREAM, 0);
			if(in_sSocket == INVALID_SOCKET)
				return -1;
			if(WSAIoctl(in_sSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GUIDName, sizeof(GUIDName), &out_fpFunPoint, sizeof(out_fpFunPoint), &dwRetVal, NULL, NULL))
			{
				closesocket(in_sSocket);
				return -1;
			}
			closesocket(in_sSocket);
		}
		else
		{
			if(WSAIoctl(in_sSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GUIDName, sizeof(GUIDName), &out_fpFunPoint, sizeof(out_fpFunPoint), &dwRetVal, NULL, NULL))
				return -1;
		}

		return 0;
	}
	// ��ȡAcceptEx������ַ
	static long IOCP_GetAcceptEx(LPFN_ACCEPTEX& out_fpFunPoint, SOCKET in_sSocket = INVALID_SOCKET)
	{
		DWORD dwRetVal = 0;
		GUID GUIDName = WSAID_ACCEPTEX;

		if(in_sSocket == INVALID_SOCKET)
		{
			in_sSocket = socket(AF_INET, SOCK_STREAM, 0);
			if(in_sSocket == INVALID_SOCKET)
				return -1;
			if(WSAIoctl(in_sSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GUIDName, sizeof(GUIDName), &out_fpFunPoint, sizeof(out_fpFunPoint), &dwRetVal, NULL, NULL))
			{
				closesocket(in_sSocket);
				return -1;
			}
			closesocket(in_sSocket);
		}
		else
		{
			if(WSAIoctl(in_sSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GUIDName, sizeof(GUIDName), &out_fpFunPoint, sizeof(out_fpFunPoint), &dwRetVal, NULL, NULL))
				return -1;
		}

		return 0;
	}
	// ��ȡGetAcceptExSockaddrs������ַ
	static long IOCP_GetGetAcceptExSockaddrs(LPFN_GETACCEPTEXSOCKADDRS& out_fpFunPoint, SOCKET in_sSocket = INVALID_SOCKET)
	{
		DWORD dwRetVal = 0;
		GUID GUIDName = WSAID_GETACCEPTEXSOCKADDRS;

		if(in_sSocket == INVALID_SOCKET)
		{
			in_sSocket = socket(AF_INET, SOCK_STREAM, 0);
			if(in_sSocket == INVALID_SOCKET)
				return -1;
			if(WSAIoctl(in_sSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GUIDName, sizeof(GUIDName), &out_fpFunPoint, sizeof(out_fpFunPoint), &dwRetVal, NULL, NULL))
			{
				closesocket(in_sSocket);
				return -1;
			}
			closesocket(in_sSocket);
		}
		else
		{
			if(WSAIoctl(in_sSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GUIDName, sizeof(GUIDName), &out_fpFunPoint, sizeof(out_fpFunPoint), &dwRetVal, NULL, NULL))
				return -1;
		}

		return 0;
	}
	// ʹ��IOCP�첽��������
	static long IOCP_SendData(UC_OVERLAPPEDPLUS* in_cpNode)
	{
		/*
		cpNode->szDealType = 4;
		cpNode->lSendFinLen += dwRetCount;
		cpNode->lSendRemLen = cpNode->wsaBuf.len;

		cpNode->wsaBuf.buf = cpNode->szpSendBuf + cpNode->lSendFinLen;
		cpNode->wsaBuf.len = cpNode->lSendRemLen - dwRetCount;
		
		if(WSASend(cpNode->sClient, (LPWSABUF)&cpNode->wsaBuf, 1, &dwSendNum, 0, (LPWSAOVERLAPPED)cpNode, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			cpClass->m_cSendBufQue.PushData(cpNode->szpSendBuf);
			printf("%d-2.WSASend() failed, code = %u\n", cpNode->szDealType, GetLastError());
			while(cpClass->_AcceptEx(cpClass->m_sListen, cpNode, cpClass->m_funPos))
				Sleep(1);
		}
		*/

		return 0;
	}
	// ʹ��IOCP�첽��������
	static long IOCP_RecvData(UC_OVERLAPPEDPLUS* in_cpNode)
	{
		/*
		cpNode->szDealType = 1;
		cpNode->lRecvFinLen = 0;			
		cpNode->lRecvRemLen = cpNode->wsaBuf.len;

		cpNode->wsaBuf.buf = cpNode->szpRecvBuf;
		cpNode->wsaBuf.len = 98;
		
		if(WSARecv(cpNode->sClient, (LPWSABUF)&cpNode->wsaBuf, 1, &dwRecvNum, &dwFlag, (LPWSAOVERLAPPED)cpNode, NULL) == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			cpClass->m_cRecvBufQue.PushData(cpNode->szpRecvBuf);
			printf("%d-1.WSARecv() failed, code = %u\n", cpNode->szDealType, GetLastError());
			while(cpClass->_AcceptEx(cpClass->m_sListen, cpNode, cpClass->m_funPos))
				Sleep(1);
		}
		*/
		
		return 0;
	}
	// ʹ��IOCP�첽���ӵ������(��ѡ: ��������)
	static long IOCP_ConnectServer(char* in_szpIP, unsigned short in_usPort, HANDLE in_hCompletionPort,  LPFN_CONNECTEX in_fpFunPoint, UC_OVERLAPPEDPLUS* in_cpOverlapped, void* in_vpSendBuf = NULL, unsigned long in_ulSendBufLen = 0, unsigned long* out_ulpSentNum = NULL)
	{
		if(in_cpOverlapped->sSocket != INVALID_SOCKET)
			closesocket(in_cpOverlapped->sSocket);

		in_cpOverlapped->sSocket = socket(AF_INET, SOCK_STREAM, 0);
		if(in_cpOverlapped->sSocket == INVALID_SOCKET)
			return -1;
		
		SOCKADDR_IN cLocalAddr;
		cLocalAddr.sin_family = AF_INET;
		cLocalAddr.sin_port = htons(0);
		cLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		if(bind(in_cpOverlapped->sSocket, (struct sockaddr*)&cLocalAddr, sizeof(cLocalAddr)) == SOCKET_ERROR)
		{
			closesocket(in_cpOverlapped->sSocket);
			return -1;
		}

		if(IOCP_BindIOCP(in_cpOverlapped->sSocket, in_hCompletionPort))
		{
			closesocket(in_cpOverlapped->sSocket);
			return -1;
		}

		memset(in_cpOverlapped->szaBuffer, 0, IPADDRESSLEN<<1);
		memset(&in_cpOverlapped->cOverlapped, 0, sizeof(OVERLAPPED)); // ���������

		SOCKADDR_IN cRemoteAddr;
		cRemoteAddr.sin_family = AF_INET;
		cRemoteAddr.sin_port = htons(in_usPort);
		cRemoteAddr.sin_addr.s_addr = inet_addr(in_szpIP);

		if(in_fpFunPoint(in_cpOverlapped->sSocket, (struct sockaddr*)&cRemoteAddr, sizeof(cRemoteAddr), NULL, 0, NULL, (OVERLAPPED*)in_cpOverlapped) == FALSE && WSAGetLastError() != ERROR_IO_PENDING)
		{
			closesocket(in_cpOverlapped->sSocket);
			return -1;
		}

		return 0;
	}
	// ����������ɶ˿ڰ󶨼���
	static long IOCP_CreateListen(unsigned short in_usPort, SOCKET& out_sListen, HANDLE& out_hCompletionPort)
	{
		SOCKADDR_IN cListenAddr;
		cListenAddr.sin_family = AF_INET;
		cListenAddr.sin_port = htons(in_usPort);
		cListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		out_sListen = socket(AF_INET, SOCK_STREAM, 0);
		if(out_sListen == INVALID_SOCKET)
			return -1;
		if(bind(out_sListen, (struct sockaddr*)&cListenAddr, sizeof(cListenAddr)) == SOCKET_ERROR)
		{
			closesocket(out_sListen);
			return -1;
		}
		if(listen(out_sListen, SOMAXCONN) == SOCKET_ERROR)
		{
			closesocket(out_sListen);
			return -1;
		}

		if(IOCP_CreateIOCP(out_hCompletionPort))
		{
			closesocket(out_sListen);
			return -1;
		}
		if(IOCP_BindIOCP(out_sListen, out_hCompletionPort))
		{
			CloseHandle(out_hCompletionPort);
			closesocket(out_sListen);
			return -1;
		}

		return 0;
	}
	// Ͷ���ص��ṹ
	static long IOCP_PostOverlapped(SOCKET in_sListen, HANDLE in_hCompletionPort, LPFN_ACCEPTEX in_fun, UC_OVERLAPPEDPLUS* in_cpNode)
	{
		if(in_cpNode->sSocket != INVALID_SOCKET)
			closesocket(in_cpNode->sSocket);

		in_cpNode->sSocket = socket(AF_INET, SOCK_STREAM, 0);
		if(in_cpNode->sSocket == INVALID_SOCKET)
			return -1;		
		
		memset(in_cpNode->szaBuffer, 0, IPADDRESSLEN<<1);
		memset(&in_cpNode->cOverlapped, 0, sizeof(OVERLAPPED));
		
		if(IOCP_BindIOCP(in_cpNode->sSocket, in_hCompletionPort))
		{
			closesocket(in_cpNode->sSocket);
			return -1;
		}
		
		DWORD dwRetVal = 0;
		if(in_fun(in_sListen, in_cpNode->sSocket, in_cpNode->szaBuffer, 0, IPADDRESSLEN, IPADDRESSLEN, &dwRetVal, &(in_cpNode->cOverlapped)) == NULL && WSAGetLastError() != ERROR_IO_PENDING)
		{
			closesocket(in_cpNode->sSocket);
			return -1;
		}
		
		return 0;
	}
	// ȡ�����Ӷ�IP��ַ(��ɶ˿�ģ��)
	static char* IOCP_GetIpInfo(char* buffer, LPFN_GETACCEPTEXSOCKADDRS in_fun = NULL, SOCKET in_sSocket = INVALID_SOCKET)
	{
		int locallen, remotelen;
		SOCKADDR *pLocal = 0, *pRemote = 0;
		
		if(in_fun == NULL)
			IOCP_GetGetAcceptExSockaddrs(in_fun, in_sSocket);

		in_fun(buffer, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &pLocal, &locallen, &pRemote, &remotelen);
		
		SOCKADDR_IN addr;
		memcpy(&addr, pRemote, sizeof(sockaddr_in));
		
		return inet_ntoa(addr.sin_addr);
	}

	/*************************************************************************
	�汾˵��                                                                 
	*************************************************************************/

	// �õ���ǰ�汾��
	static char* GetVersion()
	{
		/* ��ʼ�汾 - v1.000 - 2008.08.20 
		              v1.100 - 2008.11.05 - ������ɶ˿ں���, �仯�Ƚϴ�, ����ǰ�汾�в��ֲ���������
		*/
		return "v1.100";
	}
};

typedef UC_Communication UC;

#endif // _UC_COMMUNICATION_H_
