#ifndef MONITORBASETCPHD2735FB2B6D44ba69E975ECEBF51DA02
#define MONITORBASETCPHD2735FB2B6D44ba69E975ECEBF51DA02
#include "UH_Define.h"
// #define  MAX_NUM_MONITOR  20 /*最大个监控个数*/  
// #define  MNT_STAT_RUNING	0x22
// #define  MNT_STAT_CLOSING	0x00

class CMonitorBaseTcp
{
public:
	CMonitorBaseTcp()
	{
		m_bRun=0;
		m_iPort=0;
	}
	~CMonitorBaseTcp()
	{
		m_bRun=0;
	}
	/*
	desc：初始化监控数据
	int iNumber		[in]	设置监控线程个数
	int iPort		[in]	设置监控端口号
	int iOutTime	[in]	设置超时时间(秒)
	*/
	int Init(const char *pIpAddr,int iPort)//,CTemplateLog *pLog 
	{
		
		m_iPort=iPort;
		if(NULL!=pIpAddr && '\0'!=*pIpAddr)
			strncpy(m_szIpAddr,pIpAddr,30);
		return 0;
	};
private:
	int m_iPort;//监控端口号
	char m_szIpAddr[32];
	int m_bRun;
	//CTemplateLog *m_pLog;
public:
	int StartMonitor()
	{
		m_bRun=1;
		int iRet=0;
		//for(iIter=0;iIter<1;++iIter)
		//{
			iRet = cp_create_thread(ThrdMonitorData, this);
			//if(0===iRet)
				//break;
		//}
	   if(iRet)
	   {
		  printf("--Error-- Failure: Create Thread =ThrdMonitorData\r\n");
		  iRet=-234;
	   }
	   else
	   {
		 printf("--Debug-- Success: Create Thread =ThrdMonitorData\r\n");
	   }
	   return iRet;
		
	};
	inline static void * ThrdMonitorData(void *pPara);
	//检测 各种状态
	virtual int CheckStatus(void *pPara,int iSize,char *pError,int *piLen)=0;
};

//错级类型   
enum ErrorType {
	TYPE_OK = 0,		//运行正常
	TYPE_MONITOR,		//monitor错误
	TYPE_NETWORK,		//网络故障
	TYPE_SERVICE,		//服务错误
	TYPE_OTHER,			//其它错误
};
//错误级别
enum ErrorLevel {
	LEVEL_A = 1,		//严重
	LEVEL_B,			//重要
	LEVEL_C,			//一般
	LEVEL_D,			//调试
	LEVEL_E				//可忽略
};
void * CMonitorBaseTcp::ThrdMonitorData(void *pPara)
{
	if (NULL==pPara)
	{
		return (void *)0;
	}
	var_4 ret=0;
	CMonitorBaseTcp *pApp=(CMonitorBaseTcp *)pPara;
	CP_SOCKET_T sock_mon=0;
	CP_SOCKET_T sock_client=0;
	int iTimes=0;
	while(1)//for(iTimes=0;iTimes<3;++iTimes)
	{
		ret = cp_listen_socket(sock_mon, pApp->m_iPort);
		if(ret<0)
		{
			printf("---Failure-- service_monitor listen(port=%d) times=%d \r\n",pApp->m_iPort,iTimes);
			//return -3;
			sleep(2);
			continue;
		}
		printf("--Success-- service_monitor listen(port=%d) \r\n",pApp->m_iPort);
		break;
	} 
	if(ret<0)
	{
		pApp->m_bRun=-100;
		return 0;
	}

	const int icSizeError=1024;
	char szError[icSizeError]={0};
	int  iLenError=0;
	char str_info[1024]={0};
	char str_client_ip[30];
	var_4 client_port = 0;
	memcpy(str_info, "MonitorP1 ", 10);
	pApp->m_bRun=100;
	int iErrorCounter=0;//出错的数据个数
	while(pApp->m_bRun>0)
	{
		
		if(cp_accept_socket(sock_mon, sock_client) != 0)
		{
			sleep(1);
			continue;
		}
		cp_set_overtime(sock_client, 3000);
		char *ptr= str_info + 10;
		ret = cp_recvbuf(sock_client, ptr, 4);
		if(ret !=0 )
		{
			*(var_4*)ptr = 4;
			ptr += 4;
			*(var_4*)ptr=TYPE_NETWORK;
			ptr += 4;
		}
		else
		{
			ptr += 4;
			char *ptr_type = ptr;//报警类型
			ptr += 4;
			char *ptr_level = ptr;//报警级别
			ptr += 4;
			iLenError=0;
			iErrorCounter=pApp->CheckStatus(NULL,icSizeError,szError,&iLenError);
			if(0!=iErrorCounter)
			{
				*(var_4*)ptr_type=TYPE_SERVICE;
				*(var_4*)ptr_level=LEVEL_A;
			}
			else
			{
				*(var_4*)ptr_type=TYPE_OK;
				*(var_4*)ptr_level=LEVEL_B;
			}
			//数据长度
			*(var_4*)(str_info + 10) = (var_4)(ptr - str_info) - 14;
		}
		//
		ret = cp_sendbuf(sock_client, str_info, (var_4)(ptr - str_info));
		cp_close_socket(sock_client);
		if(0!=ret)
		{
			printf("--Error- Send Monitor {ret=%d len=%d} \n", ret, (var_4)(ptr - str_info));
		}
		/*else
		{
		printf("monitor   is  Ok{ret=%d len=%d} \n", ret, (var_4)(ptr - str_info));
		}*/
		if(iErrorCounter>0)
		{
			szError[iLenError]='\0';
			printf("--Error--Status {Counter=%d, desc=%s } \n", iErrorCounter, szError);
		}
		
	}

	return 0;
}

#endif //END MONITORBASETCPHD2735FB2B6D44ba69E975ECEBF51DA02


