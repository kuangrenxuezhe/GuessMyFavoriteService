/********************************************************************
	created:	2015/02/26
	created:	26:2:2015   14:01
	filename: 	LogData.h
	author:		Ding Jianmin
note:

   ԭʼ��������ܣ�����־������ܵ���־�ļ��Ͻ��е���΢��
   1.�ֿ� ��Ļ���ݺ�Ӳ����ر�ǡ� �����Զ�̬���ø������
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
	//������     �� InitLog
	//���ܣ�	������־��ʼ��
	//��Σ�
	//��������������u32MsgLen:��־��ϢBuffer����
	//              lpszPath:�����־Ŀ¼
	//              lpszPrefixion:��־�ļ���ǰ׺
	//              bNeedFlush:ÿ��LOG֮���ʵʱд���ļ���,Ĭ��Ϊʵʱд��
	//���Σ�
	//����ֵ:
	//              <0:ʧ��
	//				0: ����
	//��ע��
	//////////////////////////////////////////////////////////////////////
		
	//////////////////////////////////////////////////////////////////////
	//������     �� Log
	//���ܣ�	������¼��־
	//��Σ�
	//��������������bPrintScree:�Ƿ��ӡ����Ļ
	//              format:�ɱ����
	//���Σ�
	//����ֵ:
	//              <0:ʧ��
	//				0: ����
	//��ע��        ���÷���ͬprintf
	//////////////////////////////////////////////////////////////////////
inline var_2 init(var_1 *const lpszPath,var_1 *const lpszPrefixion, var_4 reserve_days = 100, const bool bNeedFlush=true, const var_u4 u32MsgLen = BUFSIZE_1M);
inline var_4 Log(const var_4 uLevenl,const var_1 * format, ...);
inline var_4 Log(const var_4 uForceMask/*ǿ������*/,const var_4 uLevenl,const var_1 * format, ...);
inline var_4 AddLog(const bool bPrintScree,const var_1 * format, ...);
	var_vd delLog(time_t curtime)
	{
		struct tm m_tmOwn;
		struct tm *m_tm;

		// ɾ����ǰ������
		var_1 delfilename[BUFSIZE_1K];
		m_tm = cp_localtime(curtime-m_dellaststep, &m_tmOwn);
		sprintf((var_1*)delfilename, "%s%04d-%02d-%02d.log", m_szLogPath,m_tm->tm_year+1900, m_tm->tm_mon+1, m_tm->tm_mday);			
		if(lw_file_exist(delfilename))
			remove(delfilename);
	}
private:
	var_1   * m_lpszMsg;						//log����BUF
	FILE   * m_fpLog;							//��־�ļ���ָ��
	var_4  m_iCurDay;					//�����Ƿ��Ѿ��ı���
	bool     m_bFlush;				    //�Ƿ�ʵʱˢ��

	var_1    m_szFileName[BUFSIZE_1K];        //��ǰ��¼�ļ����ļ���,��ʽΪyyyy-mm-dd.log
	var_1    m_szLogPath[BUFSIZE_1K];         //��־Ŀ¼

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
	//ȡ�䳤�������γ�Ҫ��ӡ��BUF		
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

	//��ӡ��STDOUT
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


	//��������
	m_dellaststep = reserve_days * 24 * 3600;

	time_t m_time_t;					//���ڴ洢��ǰʱ�����ʱ����
	struct tm* m_tm;					//���ڴ洢��ǰʱ�����ʱ����
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
//������     �� Log
//���ܣ�	������¼��־
//��Σ�
//��������������bPrintScree:�Ƿ��ӡ����Ļ
//              format:�ɱ����
//���Σ�
//����ֵ:
//              <0:ʧ��
//				0: ����
//��ע��        ���÷���ͬprintf
//////////////////////////////////////////////////////////////////////
var_4 CQLog::Log(const var_4 uLevenl,const var_1 * format, ...)
{	




	if(m_lpszMsg==NULL || m_uMsgSize<=0)
		return -1;

	//ȡ�䳤�������γ�Ҫ��ӡ��BUF		
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

	//��ӡ��STDOUT
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

				if(NULL==m_fpLog)       //˵�����ǵ�һ��LOG	
				{				
					sprintf(m_szFileName, "%s%04d-%02d-%02d.log", m_szLogPath,pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday);
					m_fpLog = fopen(m_szFileName, "a");	
					break;
				}
				else if(m_iCurDay == pTm->tm_mday)//�ж��Ƿ����µ�һ��
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

			if(m_bFlush)	//ʵʱˢ��
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

var_4 CQLog::Log(const var_4 uForceMask/*ǿ������*/,const var_4 uLevenl,const var_1 * format, ...)
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
	//ȡ�䳤�������γ�Ҫ��ӡ��BUF		
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

	//��ӡ��STDOUT
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

	if(NULL==m_fpLog)       //˵�����ǵ�һ��LOG	
	{				
	sprintf(m_szFileName, "%s%04d-%02d-%02d.log", m_szLogPath,m_tm->tm_year+1900, m_tm->tm_mon+1, m_tm->tm_mday);
	m_fpLog = fopen(m_szFileName, "a");	
	break;
	}
	else if(m_iCurDay == m_tm->tm_mday)//�ж��Ƿ����µ�һ��
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

	if(m_bFlush)	//ʵʱˢ��
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
//������     �� AddLog
//���ܣ�	������¼��־,�Ҽ�ʱ�ر�
//��Σ�
//��������������bPrintScree:�Ƿ��ӡ����Ļ
//              format:�ɱ����
//���Σ�
//����ֵ:
//              <0:ʧ��
//				0: ����
//��ע��        ���÷���ͬprintf
//////////////////////////////////////////////////////////////////////
var_4 CQLog::AddLog(const bool bPrintScree,const var_1 * format, ...)
{
	if(m_lpszMsg==NULL || m_uMsgSize<=0)
		return -1;
	//ȡ�䳤�������γ�Ҫ��ӡ��BUF

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

	if(m_fpLog == NULL)       //˵�����ǵ�һ��LOG	
	{				
		sprintf(m_szFileName, "%s%04d-%02d-%02d.log", m_szLogPath,m_tm->tm_year+1900, m_tm->tm_mon+1, m_tm->tm_mday);
		m_fpLog = fopen(m_szFileName, "a");			
	}
	else if(m_iCurDay != m_tm->tm_mday)//�ж��Ƿ����µ�һ��
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
	//��ӡ��STDOUT
	if(bPrintScree)
	{
		printf("%02d:%02d:%02d  %s", m_tm->tm_hour, m_tm->tm_min, m_tm->tm_sec, m_lpszMsg);
	}

	m_Mutex.unlock();			

	return lRet;
}


#endif //END LOG_DATA_H_be6f405d68f946818f150b7922e9709e

