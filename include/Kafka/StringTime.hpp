#ifndef STRING_GET_TIME_b7287176ef5542bcbd570672f0f59d53
#define STRING_GET_TIME_b7287176ef5542bcbd570672f0f59d53
#define STRTM_FMT_TIME_T		0x1
#define STRTM_FMT_YYYYMMDD     0x2  //年月日
#define STRTM_FMT_YYYY_MM_DD   0x3 //年月日

class CStringTime
{
public:
	int  init(int iType,bool bRemoveDir,const char *pRefix,const char *pSuffix)
	{
		m_iType=iType;
		bRemoveDir=bRemoveDir;
		strncpy(m_szPrefix,pRefix,62);
		strncpy(m_szSuffix,pSuffix,62);
	};
int GetTime(const char *pStr,time_t *pRtFileTime);
public:
	int m_iType;
	bool m_bRemoveDir;
	char m_szPrefix[64];
	char m_szSuffix[64];
public:
	CStringTime(int iType,bool bRemoveDir,const char *pRefix,const char *pSuffix)
	{
		init(iType,bRemoveDir,pRefix,pSuffix);
	};
	CStringTime()
	{
		m_iType=0;
		m_bRemoveDir=0;
		memset(m_szPrefix,0,sizeof(m_szPrefix));
		memset(m_szSuffix,0,sizeof(m_szSuffix));
	};
};
//去除目录
int GetRemoveDir(const char *pStr,const int icReSize,char *pRetStr)
{
	char *pPos=(char *)pStr;
	char *pTmp=NULL;
	char *pNext=NULL;
	do 
	{
		//去除pathfile中的目录信息(linux)
		pNext=strrchr(pPos,'/');
		if(pNext)
		{
			strncpy(pRetStr,pNext+1,icReSize-2);
			break;
		}
		//去除pathfile中的目录信息(windows)
		pNext=strrchr(pPos,'\\');
		if(pNext)
		{
			strncpy(pRetStr,pNext+1,icReSize-2);
			break;
		}
		strncpy(pRetStr,pStr,icReSize-2);
		break;
	} while (false);
	return 0;
}
int CStringTime::GetTime(const char *pStr,time_t *pRtFileTime)
{
	if(NULL==pStr || '\0'==*pStr)
	{
		return -1;
	}
	char szFileName[256]={0};
	if(m_bRemoveDir)
		GetRemoveDir(pStr,512,szFileName);
	else
		strncpy(szFileName,pStr,254);
	char *pNext=NULL;
	char *pPos=szFileName;
	pPos= strstr(pPos, m_szPrefix);
	if(NULL==pPos)
		return -104;
	pPos+=strlen(m_szPrefix);
	pNext=strstr(pPos,m_szSuffix);
	if(NULL==pNext)
		return -105;
	*pNext='\0';
	if(STRTM_FMT_TIME_T==m_iType)
	{
		if(	('0'>pPos[0]  || pPos[0]>'9' ) || ('0'>pPos[1]  || pPos[1]>'9')	)
			return -903;
		*pRtFileTime=atol(pPos);
	}
	//获取文件名称中的时间(秒)
	if(STRTM_FMT_YYYYMMDD==m_iType)
	{
		char szTime[6];
		struct tm tmTime;
		memset(&tmTime,0,sizeof(struct tm));
		//解析串
		char *pPos=szFileName;
		if(	('0'>pPos[0]  || pPos[0]>'9' ) || ('0'>pPos[1]  || pPos[1]>'9') || ('0'>pPos[2]  || pPos[2]>'9') || ('0'>pPos[3]  || pPos[3]>'9')	)
			return -1001;
		memcpy(szTime,pPos,4);
		szTime[4]='\0';
		pPos+=4;
		if(	('0'>pPos[0]  || pPos[0]>'9' ) || ('0'>pPos[1]  || pPos[1]>'9')	)
			return -1002;
		tmTime.tm_year=atol(szTime);
		memcpy(szTime,pPos,2);
		szTime[2]='\0';
		pPos+=2;
		tmTime.tm_mon=atol(szTime);
		if(	('0'>pPos[0]  || pPos[0]>'9' ) || ('0'>pPos[1]  || pPos[1]>'9')	)
			return -1003;
		memcpy(szTime,pPos,2);
		szTime[2]='\0';
		pPos+=2;
		tmTime.tm_mday=atol(szTime);
		//转为秒
		tmTime.tm_year-=1900;
		tmTime.tm_mon-=1;
		*pRtFileTime=mktime(&tmTime);
			
	}
	if(STRTM_FMT_YYYY_MM_DD==m_iType)
	{
		char szTime[6];
		struct tm tmTime;
		memset(&tmTime,0,sizeof(struct tm));
		//解析串
		char *pPos=szFileName;
		if(	('0'>pPos[0]  || pPos[0]>'9' ) || ('0'>pPos[1]  || pPos[1]>'9') || ('0'>pPos[2]  || pPos[2]>'9') || ('0'>pPos[3]  || pPos[3]>'9')	)
			return -1011;
		memcpy(szTime,pPos,4);
		szTime[4]='\0';
		pPos+=5;
		if(	('0'>pPos[0]  || pPos[0]>'9' ) || ('0'>pPos[1]  || pPos[1]>'9')	)
			return -1012;
		tmTime.tm_year=atol(szTime);
		memcpy(szTime,pPos,2);
		szTime[2]='\0';
		pPos+=3;
		tmTime.tm_mon=atol(szTime);
		if(	('0'>pPos[0]  || pPos[0]>'9' ) || ('0'>pPos[1]  || pPos[1]>'9')	)
			return -1013;
		memcpy(szTime,pPos,2);
		szTime[2]='\0';
		pPos+=2;
		//转为秒
		tmTime.tm_mday=atol(szTime);
		tmTime.tm_year-=1900;
		tmTime.tm_mon-=1;
		*pRtFileTime=mktime(&tmTime);
	}
	return 0;
}
#endif

