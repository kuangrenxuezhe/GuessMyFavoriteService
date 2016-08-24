/********************************************************************
	created:	2015/02/26
	created:	26:2:2015   14:01
	filename: 	LogData.h
	author:		Ding Jianmin
note:

   原始作者齐彦杰，本日志在齐彦杰的日志文件上进行的了微调
   1.分开 屏幕数据和硬盘落地标记。 并可以动态设置各个标记
   2.
	purpose:	
*********************************************************************/
#ifndef LOG_DATA_H_be6f405d68f946818f150b7922e9709e
#define LOG_DATA_H_be6f405d68f946818f150b7922e9709e
#include "MacroDefine.h"
#include "UH_Define.h" 
#include "lindow_api.hpp"
//#include "qi_tool.h"
#define  BUFSIZE_1K 1024
#define  BUFSIZE_1M (1024*BUFSIZE_1K)

#define FMASK_SCREEN 0x01
#define FMASK_FILE	0x02
enum VS_LOG_TYPE{INFO =1,DEBUG=2,WARN=4,ERROR=8,FATAL=0x10};

var_4 ScreenLog(const var_4 uLevenl,const var_1 * format, ...);

class CQLog
{
public:
	CQLog()
	{
		m_lpszMsg=NULL;
		m_fpLog=NULL;
		m_iCurDay=0;
		initMask();
	}
public:
	void initMask()
	{
		m_uMaskScreen=0xFF;
		m_uMaskFile=0xFF;
		strcpy(m_szLevel[INFO],"INFO");
		strcpy(m_szLevel[DEBUG],"DEBUG");
		strcpy(m_szLevel[WARN],"WARN");
		strcpy(m_szLevel[ERROR],"ERROR");
		strcpy(m_szLevel[FATAL],"FATAL");
	};
	void SetMask(long uMaskFile,long uMaskScreen)
	{
		m_uMaskFile=uMaskFile;
		m_uMaskScreen=uMaskScreen;

	};
private:
	char m_szLevel[32][32];
	long m_uMaskScreen;
	long m_uMaskFile;
public:
	~CQLog()
	{
		if(m_lpszMsg!=NULL)
		{
			delete []m_lpszMsg;
			m_lpszMsg=NULL;
		}

		if(m_fpLog != NULL)
		{
			fclose(m_fpLog);
			m_fpLog=NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////
	//函数：     　 InitLog
	//功能：	　　日志初始化
	//入参：
	//　　　　　　　u32MsgLen:日志信息Buffer长度
	//              lpszPath:存放日志目录
	//              lpszPrefixion:日志文件名前缀
	//              bNeedFlush:每次LOG之后就实时写入文件中,默认为实时写入
	//出参：
	//返回值:
	//              <0:失败
	//				0: 正常
	//备注：
	//////////////////////////////////////////////////////////////////////
		
	//////////////////////////////////////////////////////////////////////
	//函数：     　 Log
	//功能：	　　记录日志
	//入参：
	//　　　　　　　bPrintScree:是否打印到屏幕
	//              format:可变参数
	//出参：
	//返回值:
	//              <0:失败
	//				0: 正常
	//备注：        调用方法同printf
	//////////////////////////////////////////////////////////////////////
inline var_2 init(var_1 *const lpszPath,var_1 *const lpszPrefixion, var_4 reserve_days = 100, const bool bNeedFlush=true, const var_u4 u32MsgLen = BUFSIZE_1M);
inline var_4 Log(const var_4 uLevenl,const var_1 * format, ...);
inline var_4 Log(const var_4 uForceMask/*强制掩码*/,const var_4 uLevenl,const var_1 * format, ...);
inline var_4 AddLog(const bool bPrintScree,const var_1 * format, ...);
	var_vd delLog(time_t curtime)
	{
		struct tm m_tmOwn;
		struct tm *m_tm;

		// 删除从前的数据
		var_1 delfilename[BUFSIZE_1K];
		m_tm = cp_localtime(curtime-m_dellaststep, &m_tmOwn);
		sprintf((var_1*)delfilename, "%s%04d-%02d-%02d.log", m_szLogPath,m_tm->tm_year+1900, m_tm->tm_mon+1, m_tm->tm_mday);			
		if(lw_file_exist(delfilename))
			remove(delfilename);
	}
private:
	var_1   * m_lpszMsg;						//log串的BUF
	FILE   * m_fpLog;							//日志文件的指针
	var_4  m_iCurDay;					//日期是否已经改变了
	bool     m_bFlush;				    //是否实时刷新

	var_1    m_szFileName[BUFSIZE_1K];        //当前记录文件的文件名,格式为yyyy-mm-dd.log
	var_1    m_szLogPath[BUFSIZE_1K];         //日志目录

	var_u4      m_uMsgSize;

	CP_MutexLock m_Mutex;
	time_t m_dellaststep;

};
inline var_4 ScreenLog(const var_4 uLevenl,const var_1 * format, ...)
{	
	static bool bInitLevel=false;
	static char szLevel[32][32];
	if(false==bInitLevel)
	{
		bInitLevel=true;
		strcpy(szLevel[INFO],"S_INFO");
		strcpy(szLevel[DEBUG],"S_DEBUG");
		strcpy(szLevel[WARN],"S_WARN");
		strcpy(szLevel[ERROR],"S_ERROR");
		strcpy(szLevel[FATAL],"S_FATAL");
	}
	char szLog[1024]={0};
	//取变长参数表，形成要打印的BUF		
	var_4 lRet=-1;

	va_list arg;
	va_start(arg, format);	
	lRet=snprintf(szLog, 1024,format,arg);	
	va_end(arg);


	struct tm m_tmOwn;
	struct tm *pTm=NULL;
	time_t tTime;
	time(&tTime);
	pTm = cp_localtime(tTime, &m_tmOwn);

	//打印到STDOUT
	if(1)//uLevenl&m_uMaskScreen)
	{
		printf("%d-%d-%d %02d:%02d:%02d [%s]  %s\r\n", pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday,  pTm->tm_hour, pTm->tm_min, pTm->tm_sec,szLevel[uLevenl], szLog);
	}
	return lRet;
}




var_2 CQLog::init(var_1 *const lpszPath,var_1 *const lpszPrefixion, var_4 reserve_days,
				  const bool bNeedFlush, const var_u4 u32MsgLen)
{
	if(u32MsgLen<=0 )
		return -1;

	m_uMsgSize=u32MsgLen;

	if(NULL==m_lpszMsg)
	{
		m_lpszMsg = new var_1[u32MsgLen];
	}
	if(m_lpszMsg==NULL)
		return -2;


	//保存天数
	m_dellaststep = reserve_days * 24 * 3600;

	time_t m_time_t;					//用于存储当前时间的临时变量
	struct tm* m_tm;					//用于存储当前时间的临时变量
	time(&m_time_t);
	m_tm = localtime(&m_time_t);
	m_iCurDay = m_tm->tm_mday;

	m_bFlush = bNeedFlush;

	if(lpszPath==NULL || '\0'==*lpszPath)
		return -3;

	var_u4 u32Len=strlen(lpszPath);

	if(lpszPrefixion && lpszPrefixion[0]!=0)
	{
		if('/'!=lpszPath[u32Len-1])
		{
			if(_snprintf(m_szLogPath,BUFSIZE_1K, "%s/%s", lpszPath,lpszPrefixion)<=0)
				return -3;
		}
		else
		{
			if(_snprintf(m_szLogPath,BUFSIZE_1K, "%s%s", lpszPath,lpszPrefixion)<=0)
				return -4;
		}	
	}
	else
	{
		if('/'!=lpszPath[u32Len-1])
		{
			if(_snprintf(m_szLogPath,BUFSIZE_1K, "%s/", lpszPath)<=0)
				return -5;
		}
		else
		{
			if(_snprintf(m_szLogPath,BUFSIZE_1K, "%s", lpszPath)<=0)
				return -6;
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////
//函数：     　 Log
//功能：	　　记录日志
//入参：
//　　　　　　　bPrintScree:是否打印到屏幕
//              format:可变参数
//出参：
//返回值:
//              <0:失败
//				0: 正常
//备注：        调用方法同printf
//////////////////////////////////////////////////////////////////////
var_4 CQLog::Log(const var_4 uLevenl,const var_1 * format, ...)
{	




	if(m_lpszMsg==NULL || m_uMsgSize<=0)
		return -1;

	//取变长参数表，形成要打印的BUF		
	var_4 lRet=-1;

	m_Mutex.lock();

	va_list arg;
	va_start(arg, format);	
	lRet=snprintf(m_lpszMsg, m_uMsgSize,format,arg);	
	va_end(arg);

	if(lRet<=0)
	{
		m_Mutex.unlock();
		return -1;			
	}

	struct tm m_tmOwn;
	struct tm *pTm=NULL;
	time_t tTime=0;
	time(&tTime);
	pTm = cp_localtime(tTime, &m_tmOwn);

	//打印到STDOUT
	if(uLevenl&m_uMaskScreen)
	{
		printf("%d-%d-%d %02d:%02d:%02d [%s]  %s\r\n", pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday,  pTm->tm_hour, pTm->tm_min, pTm->tm_sec,m_szLevel[uLevenl], m_lpszMsg);
	}
	if ( uLevenl&m_uMaskFile )
	{
		FILE * hlCloseFailure=NULL;
		for (int ix=0;ix<2;++ix)
		{
			try
			{

				if(NULL==m_fpLog)       //说明这是第一次LOG	
				{				
					sprintf(m_szFileName, "%s%04d-%02d-%02d.log", m_szLogPath,pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday);
					m_fpLog = fopen(m_szFileName, "a");	
					break;
				}
				else if(m_iCurDay == pTm->tm_mday)//判断是否到了新的一天
				{
					break;
				}
				else 
				{
					if(m_fpLog)
					{
						fclose(m_fpLog);
						m_fpLog = NULL;
					}
					sprintf(m_szFileName, "%s%04d-%02d-%02d.log", m_szLogPath,pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday);								m_fpLog = fopen(m_szFileName, "a");	
					m_iCurDay = pTm->tm_mday;
					delLog(tTime);
					break;
				}


			}
			catch (...)
			{
				int ixx=0;
				++ixx;
				++ixx;
				hlCloseFailure=m_fpLog;
				m_fpLog = NULL;

			}
		}//end for (int ix=0;ix<2;++ix)
		if(m_fpLog != NULL)
		{
			if(NULL!=hlCloseFailure)
			{
				fprintf(m_fpLog,"%d-%d-%d %02d:%02d:%02d [%s] --failure-- close file hlCloseFailure=0x%08X \r\n", pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday,pTm->tm_hour, pTm->tm_min, pTm->tm_sec,m_szLevel[uLevenl], hlCloseFailure);
				hlCloseFailure=NULL;
			}
			fprintf(m_fpLog,"%d-%d-%d %02d:%02d:%02d [%s] %s\r\n", pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday,pTm->tm_hour, pTm->tm_min, pTm->tm_sec,m_szLevel[uLevenl], m_lpszMsg);

			if(m_bFlush)	//实时刷新
			{
				fflush(m_fpLog);
			}
			lRet=0;
		}
		else
		{
			lRet=-1;
		}		
	}
	m_Mutex.unlock();			
	return lRet;
}

var_4 CQLog::Log(const var_4 uForceMask/*强制掩码*/,const var_4 uLevenl,const var_1 * format, ...)
{	
	static bool bInitLevel=false;
	static char szLevel[32][32];
	if(false==bInitLevel)
	{
		bInitLevel=true;
		strcpy(szLevel[INFO],"S_INFO");
		strcpy(szLevel[DEBUG],"S_DEBUG");
		strcpy(szLevel[WARN],"S_WARN");
		strcpy(szLevel[ERROR],"S_ERROR");
		strcpy(szLevel[FATAL],"S_FATAL");
	}
	char szLog[1024]={0};
	//取变长参数表，形成要打印的BUF		
	var_4 lRet=-1;

	va_list arg;
	va_start(arg, format);	
	lRet=snprintf(szLog, 1024,format,arg);	
	va_end(arg);


	struct tm m_tmOwn;
	struct tm *pTm=NULL;
	time_t tTime;
	time(&tTime);
	pTm = cp_localtime(tTime, &m_tmOwn);

	//打印到STDOUT
	if(1)//uLevenl&m_uMaskScreen)
	{
		printf("%d-%d-%d %02d:%02d:%02d [%s]  %s\r\n", pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday,  pTm->tm_hour, pTm->tm_min, pTm->tm_sec,szLevel[uLevenl], szLog);
	}
	/*
	if (0)//( uLevenl&m_uMaskFile )
	{
	FILE * hlCloseFailure=NULL;
	for (int ix=0;ix<2;++ix)
	{
	try
	{

	if(NULL==m_fpLog)       //说明这是第一次LOG	
	{				
	sprintf(m_szFileName, "%s%04d-%02d-%02d.log", m_szLogPath,m_tm->tm_year+1900, m_tm->tm_mon+1, m_tm->tm_mday);
	m_fpLog = fopen(m_szFileName, "a");	
	break;
	}
	else if(m_iCurDay == m_tm->tm_mday)//判断是否到了新的一天
	{
	break;
	}
	else 
	{
	if(m_fpLog)
	{
	fclose(m_fpLog);
	m_fpLog = NULL;
	}
	sprintf(m_szFileName, "%s%04d-%02d-%02d.log", m_szLogPath,m_tm->tm_year+1900, m_tm->tm_mon+1, m_tm->tm_mday);								m_fpLog = fopen(m_szFileName, "a");	
	m_iCurDay = m_tm->tm_mday;
	delLog(m_time_t);
	break;
	}


	}
	catch (...)
	{
	int ixx=0;
	++ixx;
	++ixx;
	hlCloseFailure=m_fpLog;
	m_fpLog = NULL;

	}
	}//end for (int ix=0;ix<2;++ix)
	if(m_fpLog != NULL)
	{
	if(NULL!=hlCloseFailure)
	{
	fprintf(m_fpLog,"%d-%d-%d %02d:%02d:%02d [%s] --failure-- close file hlCloseFailure=0x%08X \r\n", m_tm->tm_year+1900, m_tm->tm_mon+1, m_tm->tm_mday,m_tm->tm_hour, m_tm->tm_min, m_tm->tm_sec,m_szLevel[uLevenl], hlCloseFailure);
	hlCloseFailure=NULL;
	}
	fprintf(m_fpLog,"%d-%d-%d %02d:%02d:%02d [%s] %s\r\n", m_tm->tm_year+1900, m_tm->tm_mon+1, m_tm->tm_mday,m_tm->tm_hour, m_tm->tm_min, m_tm->tm_sec,m_szLevel[uLevenl], m_lpszMsg);

	if(m_bFlush)	//实时刷新
	{
	fflush(m_fpLog);
	}
	lRet=0;
	}
	else
	{
	lRet=-1;
	}		
	}
	 */
	return lRet;
}
//////////////////////////////////////////////////////////////////////
//函数：     　 AddLog
//功能：	　　记录日志,且即时关闭
//入参：
//　　　　　　　bPrintScree:是否打印到屏幕
//              format:可变参数
//出参：
//返回值:
//              <0:失败
//				0: 正常
//备注：        调用方法同printf
//////////////////////////////////////////////////////////////////////
var_4 CQLog::AddLog(const bool bPrintScree,const var_1 * format, ...)
{
	if(m_lpszMsg==NULL || m_uMsgSize<=0)
		return -1;
	//取变长参数表，形成要打印的BUF

	var_4 lRet=-1;

	m_Mutex.lock();	
	va_list arg;
	va_start(arg, format);
	lRet=snprintf(m_lpszMsg, m_uMsgSize,format,arg);
	va_end(arg);

	if(lRet<=0)
	{
		m_Mutex.unlock();
		return -1;
	}			

	struct tm * m_tm;
	time_t m_time_t;
	time(&m_time_t);
	m_tm = localtime(&m_time_t);

	if(m_fpLog == NULL)       //说明这是第一次LOG	
	{				
		sprintf(m_szFileName, "%s%04d-%02d-%02d.log", m_szLogPath,m_tm->tm_year+1900, m_tm->tm_mon+1, m_tm->tm_mday);
		m_fpLog = fopen(m_szFileName, "a");			
	}
	else if(m_iCurDay != m_tm->tm_mday)//判断是否到了新的一天
	{
		fclose(m_fpLog);
		m_fpLog = NULL;
		sprintf(m_szFileName, "%s%04d-%02d-%02d.log", m_szLogPath,m_tm->tm_year+1900, m_tm->tm_mon+1, m_tm->tm_mday);
		m_fpLog = fopen(m_szFileName, "a");
		m_iCurDay = m_tm->tm_mday;
	}

	if(m_fpLog != NULL)
	{
		fprintf(m_fpLog,"%02d:%02d:%02d  %s", m_tm->tm_hour, m_tm->tm_min, m_tm->tm_sec, m_lpszMsg);

		fclose(m_fpLog);
		m_fpLog = NULL;

		lRet=0;
	}
	else
	{
		lRet=-1;
	}
	//打印到STDOUT
	if(bPrintScree)
	{
		printf("%02d:%02d:%02d  %s", m_tm->tm_hour, m_tm->tm_min, m_tm->tm_sec, m_lpszMsg);
	}

	m_Mutex.unlock();			

	return lRet;
}


#endif //END LOG_DATA_H_be6f405d68f946818f150b7922e9709e

