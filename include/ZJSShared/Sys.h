#ifndef SYS_H_
#define SYS_H_
#include <cstdio>
#include <cstring>
#include <cstdlib>
#ifdef WIN32
#pragma warning(disable:4996)

#include <io.h>
#include <sys/locking.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/locking.h>
#include <share.h>
#include <fcntl.h>
#include <direct.h>
#include <errno.h>

#include <Winsock2.h>
#include <Windows.h>

#pragma comment(lib,"Ws2_32.lib")

#else
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>  
#include <sys/ioctl.h>  
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/file.h>
#include <sys/select.h>
#include <sys/time.h>
#include <net/if.h>  
#include <net/if_arp.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include <pthread.h>
#include <signal.h>
#endif

#if defined (_WIN32) || defined (WIN64)
#define WINDOWS_PLATFORM
#endif

#ifdef WINDOWS_PLATFORM //是WINDOWS平台
typedef __int8              int8_t;
typedef __int16             int16_t;
typedef __int32             int32_t;
typedef __int64             int64_t;
typedef unsigned __int8     uint8_t;
typedef unsigned __int16    uint16_t;
typedef unsigned __int32    uint32_t;
typedef unsigned __int64    uint64_t;
#else
typedef u_int8_t            uint8_t;
typedef u_int16_t           uint16_t;
typedef u_int32_t           uint32_t;
typedef u_int64_t           uint64_t;
#endif

#ifdef WINDOWS_PLATFORM
#define strncasecmp         _memicmp
#define strcasecmp          strcmpi

#define usleep              Sleep
#define THREADFUNTYPE       var_u4 WINAPI

#include <winsock2.h>
#define snprintf            _snprintf
#define strtoll             _strtoi64
#define strtoull            _strtoui64

#else
#define GetLastError()      errno

#define Sleep(x)            usleep(x*1000)
#define THREADFUNTYPE       void*

typedef pthread_mutex_t     CRITICAL_SECTION;

#define _snprintf           snprintf
#define _vsnprintf          vsnprintf

#define _atoi64(val)        strtoll(val, NULL, 10)
#define _fseeki64           fseek

#endif

#endif //SYS_H_

