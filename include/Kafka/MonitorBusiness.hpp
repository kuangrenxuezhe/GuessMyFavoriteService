#ifndef		MONITOR_BUSINESS_H83F2BA1B0E6B4a5d8D1B3DDA54E0C70E
#define MONITOR_BUSINESS_H83F2BA1B0E6B4a5d8D1B3DDA54E0C70E
#include "MonitorBaseTcp.hpp"
//#include "qi_tool.h"

typedef struct STRUCT_MONITOR_GROUP
{
	int iMax;//最多个数
	char szDesc[32];
	STRUCT_MONITOR_GROUP()
	{
		iMax=0;
		for(int i=0;i<32;++i)
			szDesc[i]=0;
	}
} STT_MONITOR_GROUP;

class CMonitorItem
{
protected:
	//CP_MutexLock m_Mutex;
	time_t   m_u64Time;          //u32Time: 当前状态所用时间,m_bWork==true  u32Time：起始时间
	int			m_uState;         	 //当前状态
	bool	   m_bWork; 	//	当前状态是否工作 true:工作
	char m_szState[64];
public:
	bool	m_bUsing;
public:
	CMonitorItem()
	{
		m_u64Time=0;		
		m_uState=0;
		m_bWork=false;
		m_bUsing=false;
		memset(m_szState,0,sizeof(m_szState));
	}
	~CMonitorItem()	{	};
	void Clear()
	{			
		//m_Mutex.lock();
		m_u64Time=0;		
		m_uState=0;
		m_bWork=false;
		//m_Mutex.unlock();
	};
	void Out ()
	{
		//m_Mutex.lock();
		m_u64Time =time(NULL)-m_u64Time; // 
		m_bWork =false;	
		//m_Mutex.unlock();
	}
	//检测
	void In()
	{
		//m_Mutex.lock();
		m_u64Time =time(NULL);
		//m_Mutex.unlock();
	};
	//
	void In(const unsigned int uState,const char *pState)
	{
		//m_Mutex.lock();
		m_u64Time =time(NULL);
		m_bWork =true;
		m_uState =uState;
		if(NULL!=pState)
			 strncpy(m_szState,pState,62);
		else
			memset(m_szState,0,2);
		//m_Mutex.unlock();
	}
	//////////////////////////////////////////////////////////////////////
	//函数：     　 GetStateInfo
	//功能：	　　取状态信息
	//入参：
	//              u32OverTime:超时值
	//              u32OutBufSize:lszpOutBuf的SIZE
	//出参：          
	//              lszpOutBuf:输出状态BUFFER
	//              u32OutBufLen:输出数据的Length
	//返回值:
	//              0:状态正常    >0：有超时数据   
	//备注： 
	//              lszpOutBuf==NULL: 只取函数内是否有超时的处理  否则 取函数内是否有超时的处理,并装错误信息添加到
	//////////////////////////////////////////////////////////////////////
	var_4 Check(const var_u4 u32OverTime,char * const lszpOutBuf,const var_u4 u32OutBufSize,var_u4 &u32OutBufLen)
	{
		u32OutBufLen=0;				
		var_4 lRet=0;		

		if(lszpOutBuf && u32OutBufSize>0)
		{
			time_t u64CurTime=time(NULL);
			//m_Mutex.lock();
			if(m_bWork && (u64CurTime-m_u64Time)>u32OverTime)
			{
				lRet=snprintf(lszpOutBuf,u32OutBufSize-1,"ustatus=%d, usetime=%lu,status=%s",m_uState,u64CurTime-m_u64Time,m_szState);
				if(lRet>0)
					u32OutBufLen=lRet;

				lRet=1;			
			}
			//m_Mutex.unlock();		
		}
		else
		{
			//m_Mutex.lock();		
			if(m_bWork && time(NULL)-m_u64Time>u32OverTime)
			{
				lRet=1;					
			}	
			//m_Mutex.unlock();

		}

		return lRet;
	}


};
class CMonitorGroup
{
public:
	int iMax;
	char szDesc[32];
	CMonitorItem   *pItems;
public:
	CMonitorGroup()
	{
		iMax=0;
		pItems=NULL;
		for(int i=0;i<32;++i)
		{
			szDesc[i]=0;
		}
		
	};
	~CMonitorGroup()
	{
		if(pItems)
		{
			delete [] pItems;
			pItems=NULL;
		}
	};
};

class CMonitorStatus:public CMonitorBaseTcp
{
protected:

public:
	static CMonitorStatus * GetInstance()
	{
		static CMonitorStatus *pThis=NULL;
		if(NULL==pThis)
		{
			pThis=new CMonitorStatus;
		}
		return pThis;
	}
public:
	CMonitorGroup  *m_pstGroup;
	int m_iCounterGroup;
	int m_iOverTime;//超时时间
	CMonitorStatus()
	{
		m_iCounterGroup=0;
		m_pstGroup=NULL;
	};
	~CMonitorStatus()
	{
		
	};
	/*
	desc 初始化 
	STT_MONITOR_GROUP *pStGroup	[in] 监控组
	int iCounterGroup 	[in] 监控组个数
	const char *pIpAddr [in]  Ip地址  默认为空
	int iPort    [in]		端口号
	int iOverTime [in]   超时时间(秒)
	*/
	int Init(STT_MONITOR_GROUP *pStGroup,int iCounterGroup,const char *pIpAddr,int iPort,int iOverTime)
	{
		if(NULL==pStGroup || iCounterGroup<=0)
		{
			return -1;
		}
		if(iOverTime<0)
			return -2;
		int ix=0;
		for(ix=0;ix<iCounterGroup;++ix)
		{
			if(pStGroup[ix].iMax<=0)
				return -100-ix;
		}
		m_iOverTime=iOverTime;
		 //初始化类
		if(0!=CMonitorBaseTcp::Init(pIpAddr,iPort))
		{
			return -2;
		}
		m_pstGroup=new CMonitorGroup[iCounterGroup];
		m_iCounterGroup=iCounterGroup;
		
		for(ix=0;ix<iCounterGroup;++ix)
		{
			CMonitorGroup &stMnGrp=m_pstGroup[ix];
			stMnGrp.iMax=pStGroup[ix].iMax;//记录每个监控的开始位置
			strcpy(stMnGrp.szDesc,pStGroup[ix].szDesc);
			stMnGrp.pItems=new CMonitorItem[stMnGrp.iMax];
			if(NULL==stMnGrp.pItems)
			{
				for(int ii=0;ii<ix;++ii)
				{
					if(m_pstGroup[ii].pItems)
					{
						delete [] m_pstGroup[ii].pItems;
						m_pstGroup[ii].pItems=NULL;
					}
				}
				return -200-ix;
			}
		}
		return 0;
	};
	//
	int GetItem(int iGroupId,CMonitorItem **ppItem)
	{
		int iError=-1;
		if(iGroupId<0 || iGroupId>m_iCounterGroup)
			 return -1;

		CMonitorGroup &stGroup=m_pstGroup[iGroupId];
		for (int iIter=0;iIter<stGroup.iMax;++iIter)
		{
			if(false==stGroup.pItems[iIter].m_bUsing)
			{
				stGroup.pItems[iIter].m_bUsing=true;
				*ppItem=&(stGroup.pItems[iIter]);
				iError=0;
				break;
			}
		}
		return iError;
	};
	int ReleaseItem(int iGroupId,CMonitorItem *pItem)
	{
		int iError=-1;
		if(iGroupId<0 || iGroupId>m_iCounterGroup)
			return -1;

		CMonitorGroup &stGroup=m_pstGroup[iGroupId];
		for (int iIter=0;iIter<stGroup.iMax;++iIter)
		{
			if(pItem==&(stGroup.pItems[iIter]))
			{
				stGroup.pItems[iIter].m_bUsing=false;
				iError=0;
				break;
			}
		}
		return iError;
	}
protected:
	int CheckStatus(void *pPara,int iSize,char *pError,int *piLen)
	{
		if(NULL==pError || iSize<0)
			return -1;

		CMonitorStatus  *pData=CMonitorStatus::GetInstance();
		int iTypIter=0;
		int iIter=0;
		int iCounterError=0;
		char szDesc[256]={0};
		char szTmp[1024]={0};
		char *pPosErr=pError;
		const char *pEndError=pError+iSize-3;
		for (iTypIter=0;iTypIter<pData->m_iCounterGroup;++iTypIter)
		{
			CMonitorGroup &stMnGrp=pData->m_pstGroup[iTypIter];
			for (iIter=0;iIter<stMnGrp.iMax;++iIter)
			{
				var_u4 uRetLen=0;
				if(false==stMnGrp.pItems[iIter].m_bUsing)
					continue;
				if(stMnGrp.pItems[iIter].Check(m_iOverTime, szDesc, 250, uRetLen))
				{
					int iTmp=sprintf(szTmp,"group=%s, item={%s}",stMnGrp.szDesc,szDesc);
					printf("--Error-- monitor warning{ %s } [%s:%d] \n",
						szTmp,__FILE__,__LINE__);
					if((pEndError-pPosErr)>iTmp)
					{
							iTmp=sprintf(pPosErr,";%s",szTmp);
							pPosErr+=iTmp;
					}
					iCounterError++;
				}
			}
		}
		*piLen=pPosErr-pError;
		return iCounterError;
	};

};
// int CMonitorBusiness::GetMonitorData(void *pPara)
// {
// 	
// }

#endif