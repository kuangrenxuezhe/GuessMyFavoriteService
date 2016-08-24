#ifndef PUBLIC_DEFINE_H
#define PUBLIC_DEFINE_H

#include <UH_Define.h>
#include "WFLog.h"
#include "WFMonitor.h"
#include "WFLock.h"
#include "Timer.h"
#include "PublicDefine.h"
#include <time.h>
#ifndef _WIN32
#include <iconv.h>
#endif

using nsWFLog::CDailyLog;
using nsWFLock::CLock;
using nsWFMonitor::CMonitor;
using nsZJSTimer::CTimer;

#if _WIN32
#include <winsock2.h>
#include <windows.h>
#define strcasecmp			strcmpi
#define strncasecmp			_strnicmp
#define snprintf			_snprintf
#define strtoll				_strtoi64
#define strtoull			_strtoui64
#endif

#define CONFIG_MAX_ITEM_LEN (1024)
typedef struct 
{
	char ip[16];
	unsigned short port;
	char dbName[CONFIG_MAX_ITEM_LEN];
} kafka_info;

typedef struct 
{
	char ip[16];
	unsigned short port;
} anchor_info;

#define LOG_LEVEL_ERROR (1)
#define LOG_LEVEL_INFO (2)
#define LOG_LEVEL_DEBUG (3)

#define free_object(o) if(o != NULL) delete o;
#define free_object_array(o) if(o != NULL) delete [] o;

static void die(int died ,const char *fmt,...){
	if(died){
		va_list ap;
		va_start(ap,fmt);
		vprintf( fmt,ap);
		va_end(ap);
		
		exit(EXIT_FAILURE);
    }
}

//FUNC  得到线程ID
static unsigned long get_thread_id()
{
#ifdef _WIN32_ENV_
    return GetCurrentThreadId();
#else
    return pthread_self();
#endif
}

#ifdef _WIN32_ENV_
int gettimeofday(struct timeval *tp, void *tzp)
{
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;

    GetLocalTime(&wtm);
    tm.tm_year		= wtm.wYear - 1900;
    tm.tm_mon		= wtm.wMonth - 1;
    tm.tm_mday		= wtm.wDay;
    tm.tm_hour		= wtm.wHour;
    tm.tm_min		= wtm.wMinute;
    tm.tm_sec		= wtm.wSecond;
    tm.tm_isdst    = -1;
    clock = mktime(&tm);
    tp->tv_sec = clock;
    tp->tv_usec = wtm.wMilliseconds * 1000;

    return (0);
}
#endif

#ifndef _WIN32_ENV_
int code_convert( char *pSrcCharset,  char *pDesCharset, char *pSource, size_t lenSource, char *Desbuf, size_t lenMaxSource)
{
	if (NULL == pSrcCharset || NULL == pDesCharset || NULL == pSource || lenSource == 0 || lenSource == (size_t)(-1))
	{
		return -1;
	}
	iconv_t ivt = iconv_open(pDesCharset, pSrcCharset);
	if(ivt == (iconv_t)(-1))
	{
		return -2;
	}

	char *pInBuf = pSource;
	char *pOutbuf = Desbuf;
	size_t lenOutBuf = 0;
	size_t lenInLeft = lenSource;
	size_t lenOutLeft = lenMaxSource;

	bool bLoop = true;
	int iRetCode = 0;
	while ((int)lenInLeft > 0 && bLoop)
	{
		size_t ret = iconv(ivt, &pInBuf, &lenInLeft, &pOutbuf, &lenOutLeft);
		size_t ierr = errno;
		if(ret != (size_t)(-1))
		{
			iRetCode = lenMaxSource - lenOutLeft;
			break;
		}
		switch (ierr)
		{
		case E2BIG:
			// 没有足够的空间以进行下一次的字符转换
			bLoop = false;
			iRetCode =-3;// NO_MORE_ROOM;
			printf("没有足够的空间\n");
			break;
		case EILSEQ:
			// 碰到非法的多字节序，跳过该非法字节，继续尝试转码
			pInBuf++;
			lenInLeft = lenSource - (pInBuf - pSource);
			//				printf("非法的多字节序\n");
			break;
		case EINVAL:
			// 碰到不完整的多字节序
			bLoop = false;
			iRetCode =-4;//INCOMPLETE_MULTIBYTE_SEQUENCE;
			//printf("碰到不完整的多字节序\n");
			break;
		default:
			bLoop = false;
			iRetCode =3;
			break;
		}
		if (iRetCode==-4)
		{
			iconv_close(ivt);
			return iRetCode;
		}
	}
	
	iconv_close(ivt);
	return iRetCode;
}
#endif

#endif

