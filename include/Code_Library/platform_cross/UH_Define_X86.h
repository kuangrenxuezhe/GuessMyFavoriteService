// UH_Define.h

#ifndef _UH_DEFINE_H_
#define _UH_DEFINE_H_

// 系统类型定义
#ifdef WIN32

#define _WIN32_ENV_
#define _SYSTEM_X86_

#define gettimeofday(x,y)

#else

#define _LINUX_ENV_
#define _SYSTEM_X86_

#endif

// 头文件包含定义
#ifdef _WIN32_ENV_
#pragma warning(disable:4996)
#pragma warning(disable:4244)

#include <winsock2.h>
#pragma comment(lib, "ws2_32")

#include <mswsock.h>
#include <windows.h>

#include <io.h>
#include <time.h>
#include <share.h>
#include <sys/stat.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <stdarg.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <signal.h>

#include <netdb.h>

#include <net/if.h>
#include <errno.h>
#include <unistd.h>
#endif

#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


// 变量类型定义
#ifdef _SYSTEM_X86_
typedef               char var_1;
typedef      unsigned char var_u1;
typedef              short var_2;
typedef     unsigned short var_u2;
typedef                int var_4;
typedef       unsigned int var_u4;
typedef              float var_f4;
typedef             double var_d8;
typedef				  void var_vd;

#ifdef _WIN32_ENV_
typedef            __int64 var_8;
typedef   unsigned __int64 var_u8;
#else
typedef          long long var_8;
typedef unsigned long long var_u8;
#endif // _WIN32_ENV_

#else

typedef               char var_1;
typedef      unsigned char var_u1;
typedef              short var_2;
typedef     unsigned short var_u2;
typedef                int var_4;
typedef       unsigned int var_u4;
typedef              float var_f4;
typedef             double var_d8;
typedef				  void var_vd;

#ifdef _WIN32_ENV_
typedef            __int64 var_8;
typedef   unsigned __int64 var_u8;
#else
typedef               long var_8;
typedef      unsigned long var_u8;
#endif // _WIN32_ENV_

#endif // _SYSTEM_X86_

// 自加减
__inline void lock_inc(var_u4* val) 
{
#ifdef _WIN32_ENV_

#ifdef _SYSTEM_X86_
	InterlockedIncrement((long*)val);
#else
//	InterlockedIncrement(val);
#endif

#else

#ifdef _SYSTEM_X86_
	asm("movl %0,%%eax" :"=m"(val) :"m"(val) :"%eax");
	asm("lock; incl (%eax)");
#else
	asm("movq %0,%%rax" :"=m"(val) :"m"(val) :"%rax");
	asm("lock; incq (%rax)");
#endif

#endif
}

__inline void lock_dec(var_u4* val)
{
#ifdef _WIN32_ENV_

#ifdef _SYSTEM_X86_
	InterlockedDecrement((long*)val);
#else
//	InterlockedDecrement(val);
#endif

#else

#ifdef _SYSTEM_X86_
	asm("movl %0,%%eax" :"=m"(val) :"m"(val) :"%eax");
	asm("lock; decl (%eax)");
#else
	asm("movq %0,%%rax" :"=m"(val) :"m"(val) :"%rax");
	asm("lock; decq (%rax)");
#endif

#endif	
}

// 休眠
#ifdef _WIN32_ENV_
#define cp_sleep(x)			Sleep(x)
#else
#define cp_sleep(x)			usleep(x*1000)
#endif

static var_vd cp_wait_dispose()
{
	for(;;) cp_sleep(5000);
}

// 互斥锁 读写锁
#ifdef _WIN32_ENV_
#include <windows.h>
typedef struct CP_MutexLock
{
	CRITICAL_SECTION mutex;

	CP_MutexLock()
	{
		InitializeCriticalSection(&mutex);
	}

	~CP_MutexLock()
	{
		DeleteCriticalSection(&mutex);
	}

	inline void lock()
	{
		EnterCriticalSection(&mutex);
	}

	inline void unlock()
	{
		LeaveCriticalSection(&mutex);
	}
} CP_MUTEXLOCK;

typedef struct CP_MutexLock_RW
{
	CRITICAL_SECTION mutex;
	
	var_u4 rd_c; // read count
	var_u4 wr_f; // write flag

	CP_MutexLock_RW()
	{
		InitializeCriticalSection(&mutex);
		
		rd_c = 0;
		wr_f = 0;
	}

	~CP_MutexLock_RW()
	{
		DeleteCriticalSection(&mutex);
	}

	inline void lock_r()
	{
		for(;;)
		{
			EnterCriticalSection(&mutex);
			if(wr_f != 0)
			{
				LeaveCriticalSection(&mutex);
				cp_sleep(1);

				continue;
			}	
			rd_c++;
			LeaveCriticalSection(&mutex);

			break;
		}
	}

	inline void lock_w()
	{
		for(;;)
		{
			EnterCriticalSection(&mutex);
			if(wr_f != 0 || rd_c != 0)
			{
				LeaveCriticalSection(&mutex);
				cp_sleep(1);

				continue;
			}
			wr_f = 1;
			LeaveCriticalSection(&mutex);

			break;
		}
	}

	inline void unlock()
	{
		EnterCriticalSection(&mutex);
		if(wr_f != 0)
			wr_f = 0;
		else
		{
			if(rd_c <= 0)
				printf("RW_MUTEXLOCK error\n");
			else
				rd_c--;
		}
		LeaveCriticalSection(&mutex);
	}
} CP_MUTEXLOCK_RW;
#else
#include <pthread.h>
typedef struct CP_MutexLock
{
	pthread_mutex_t mutex;

	CP_MutexLock()
	{
		pthread_mutex_init(&mutex, NULL);
	}

	~CP_MutexLock()
	{
		pthread_mutex_destroy(&mutex);
	}

	inline void lock()
	{
		pthread_mutex_lock(&mutex);
	}

	inline void unlock()
	{
		pthread_mutex_unlock(&mutex);
	}
} CP_MUTEXLOCK;

typedef struct CP_MutexLock_RW
{
	pthread_rwlock_t mutex;

	CP_MutexLock_RW()
	{
		pthread_rwlock_init(&mutex, NULL);
	}

	~CP_MutexLock_RW()
	{
		pthread_rwlock_destroy(&mutex);
	}

	inline void lock_r()
	{
		pthread_rwlock_rdlock(&mutex);
	}

	inline void lock_w()
	{
		pthread_rwlock_wrlock(&mutex);
	}

	inline void unlock()
	{
		pthread_rwlock_unlock(&mutex);
	}
} CP_MUTEXLOCK_RW;
#endif

#ifdef _WIN32_ENV_
#define CP_P64		"%I64"
#define CP_PU64		"%I64u"
#else
#define CP_P64		"%l"
#define CP_PU64		"%lu"
#endif

// 64bit string to value
#ifdef _WIN32_ENV_
#define cp_strtoval_u64(x)		_strtoui64(x, NULL, 10)
#define cp_strtoval_64(x)		_strtoi64(x, NULL, 10)
#else
#define cp_strtoval_u64(x)		strtoul(x, NULL, 10)
#define cp_strtoval_64(x)		strtol(x, NULL, 10)
#endif

/************************************************************************/
// 通讯相关
/************************************************************************/
#ifdef _WIN32_ENV_
#define CP_SOCKET_T		SOCKET
#else
#define CP_SOCKET_T		int
#endif

#ifdef _WIN32_ENV_
#define cp_close_socket(x)		closesocket(x)
#else
#define cp_close_socket(x)		close(x)
#endif

static var_4 cp_set_overtime(CP_SOCKET_T in_socket, var_4 in_time)
{
#ifdef _WIN32_ENV_
	setsockopt(in_socket, SOL_SOCKET, SO_RCVTIMEO,(var_1*)&in_time, sizeof(in_time));
	setsockopt(in_socket, SOL_SOCKET, SO_SNDTIMEO,(var_1*)&in_time, sizeof(in_time));
#else
	struct timeval over_time;
	over_time.tv_sec = in_time / 1000;
	over_time.tv_usec = (in_time % 1000) * 1000;

	setsockopt(in_socket, SOL_SOCKET, SO_RCVTIMEO,(var_1*)&over_time, sizeof(over_time));
	setsockopt(in_socket, SOL_SOCKET, SO_SNDTIMEO,(var_1*)&over_time, sizeof(over_time));
#endif
	return 0;
}


static var_4 cp_init_socket()
{
#ifdef _WIN32_ENV_
	WSAData cWSAData;
	if(WSAStartup(MAKEWORD(2, 2), &cWSAData))
	{
		printf("cp_init_socket: init socket failure\n");
		return -1;
	}
#else
	sigset_t signal_mask;
	sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGPIPE);
	if(pthread_sigmask(SIG_BLOCK, &signal_mask, NULL))
	{
		printf("cp_init_socket: block sigpipe failure\n");
		return -1;
	}
#endif

	return 0;
}

static var_4 cp_listen_socket(CP_SOCKET_T& in_listen, var_u2 in_port)
{
	struct sockaddr_in my_server_addr;
	my_server_addr.sin_family = AF_INET;
	my_server_addr.sin_port = htons(in_port);
	my_server_addr.sin_addr.s_addr = htons(INADDR_ANY);

	in_listen = socket(AF_INET, SOCK_STREAM, 0);
	if(in_listen < 0)
	{
		printf("cp_listen_socket: create socket failure\n");
		return -1;
	}
	if(bind(in_listen, (struct sockaddr*)&my_server_addr, sizeof(my_server_addr)) < 0)
	{
		cp_close_socket(in_listen);

		printf("cp_listen_socket: bind to port %d failure!\n", in_port);
		return -1;
	}
	if(listen(in_listen, SOMAXCONN) < 0)
	{
		cp_close_socket(in_listen);

		printf("cp_listen_socket: listen to port %d failure\n", in_port);
		return -1;
	}

	return 0;
}

static var_4 cp_connect_socket(CP_SOCKET_T& in_socket, var_1* in_ip, var_u2 in_port)
{
	struct sockaddr_in my_server_addr;
	my_server_addr.sin_family = AF_INET;
	my_server_addr.sin_port = htons(in_port);
	my_server_addr.sin_addr.s_addr = inet_addr(in_ip);

	in_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(in_socket < 0)
	{
		printf("cp_connect_socket: create socket failure\n");
		return -1;
	}
	if(connect(in_socket, (struct sockaddr*)&my_server_addr, sizeof(my_server_addr)) < 0)
	{
		cp_close_socket(in_socket);

		printf("cp_connect_socket: connect %s:%d failure\n", in_ip, in_port);
		return -1;
	}

	return 0;
}

static var_4 cp_sendbuf(CP_SOCKET_T in_socket, var_1* in_buffer, var_4 in_buflen)
{
	var_4 retval = 0, finlen = 0;
	do{
		retval = send(in_socket, in_buffer + finlen, in_buflen - finlen, 0);
		if(retval > 0)
			finlen += retval;
	}while(retval > 0 && finlen < in_buflen);
	if(retval < 0 || finlen < in_buflen)
		return -1;

	return 0;
}

static var_4 cp_recvbuf(CP_SOCKET_T in_socket, var_1* in_buffer, var_4 in_buflen)
{
	var_4 retval = 0, finlen = 0;
	do{
		retval = recv(in_socket, in_buffer + finlen, in_buflen - finlen, 0);
		if(retval > 0)
			finlen += retval;
	}while(retval > 0 && finlen < in_buflen);
	if(retval < 0 || finlen < in_buflen)
		return -1;

	return 0;
}

static var_4 cp_recvbuf_onebyte(CP_SOCKET_T in_socket, var_1 end_char, var_1* in_buffer, var_4& in_buflen)
{
	in_buflen = 0;

	var_4 once_len = 0;

	for(;;)
	{		
		once_len = recv(in_socket, in_buffer + in_buflen, 1, 0);
		if(once_len <= 0)
			break;
		in_buflen += once_len;
		if(in_buffer[in_buflen - 1] == end_char)
			break;
	}
	if(once_len <= 0)
		return -1;

	in_buflen--;

	return 0;
}

static var_4 cp_recvbuf_twobyte(CP_SOCKET_T in_socket, var_1 end_one, var_1 end_two, var_1* in_buffer, var_4& in_buflen)
{
	in_buflen = 0;

	var_4 once_len = 0;

	for(;;)
	{		
		once_len = recv(in_socket, in_buffer + in_buflen, 1, 0);
		if(once_len <= 0)
			break;
		in_buflen += once_len;
		if(in_buflen >= 2 && in_buffer[in_buflen - 2] == end_one && in_buffer[in_buflen - 1] == end_two)
			break;
	}
	if(once_len <= 0)
		return -1;

	in_buflen -= 2;

	return 0;
}

static var_8 cp_get_file_size(var_1* in_file)
{
	struct stat file_info;
	if(stat(in_file, &file_info))
		return -1;
	return file_info.st_size;
}

static var_4 cp_sendfile(CP_SOCKET_T in_socket, var_1* in_file)
{
	var_4 file_len = cp_get_file_size(in_file);
	if(file_len < 0)
	{
		printf("cp_sendfile: get file size failure\n");
		return -1;
	}

	FILE* fp = fopen(in_file, "rb");
	if(fp == NULL)
	{
		printf("cp_sendfile: open file %s failure\n", in_file);
		return -1;
	}

	if(cp_sendbuf(in_socket, (var_1*)&file_len, 4))
	{
		printf("cp_sendfile: send file size failure\n");
		fclose(fp);
		return -1;
	}	

	var_1 send_buf[4096];
	var_4 send_len = 0;

	while(file_len > 0)
	{
		send_len = 4096;
		if(file_len < 4096)
			send_len = file_len;
		
		if(fread(send_buf, send_len, 1, fp) != 1)
		{
			printf("cp_sendfile: read file %s failure\n", in_file);
			fclose(fp);
			return -1;
		}
		if(cp_sendbuf(in_socket, send_buf, send_len))
		{
			printf("cp_sendfile: send file content failure\n");
			fclose(fp);
			return -1;
		}

		file_len -= send_len;
	}

	fclose(fp);

	return 0;
}

/************************************************************************/
// 线程相关
/************************************************************************/
#ifdef _WIN32_ENV_
#define CP_THREAD_T		DWORD WINAPI
#else
#define CP_THREAD_T		void*
#endif

#ifdef _WIN32_ENV_
static var_4 cp_create_thread(LPTHREAD_START_ROUTINE in_function, void* in_argv)
{
	DWORD dwThreadID;
	HANDLE hThreadHandle = CreateThread(NULL, 0, in_function, in_argv, 0, &dwThreadID);
	if(hThreadHandle == NULL)
		return -1;
	CloseHandle(hThreadHandle);
	return 0;
}
#else
static var_4 cp_create_thread(void* (in_function)(void*), void* in_argv)
{
	pthread_t pid;
	if(pthread_create(&pid, 0, in_function, in_argv))
		return -1;
	return 0;
}
#endif

// 改变文件大小
#ifdef _WIN32_ENV_
static var_4 cp_change_file_size(FILE* fp, var_8 size)
{
	if(chsize(fileno(fp), size))
		return -1;	
	return 0;
}
#else
static var_4 cp_change_file_size(FILE* fp, off_t size)
{
	if(ftruncate(fileno(fp), size))
		return -1;
	return 0;
}
#endif

static var_u1 BM_RULE_BIT[] = {0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1};
#define BM_SET_BIT(bm, val)	(((var_u1*)(bm))[(val)>>3] |= BM_RULE_BIT[(val)&0x7])
#define BM_GET_BIT(bm, val)	(((var_u1*)(bm))[(val)>>3] & BM_RULE_BIT[(val)&0x7])

#define BM_LEN_1BYTE(num)	(((num) + 7) / 8)
#define BM_LEN_2BYTE(num)	(((num) + 15) / 16)
#define BM_LEN_4BYTE(num)	(((num) + 31) / 32)
#define BM_LEN_8BYTE(num)	(((num) + 63) / 64)

/*
static var_u1 BM_RULE_1BIT[] = {0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1};
#define BM_SET_1BYTE(bm, val)	(((var_u1*)(bm))[(val)>>3] |= BM_RULE_1BIT[(val)&0x7])
#define BM_GET_1BYTE(bm, val)	(((var_u1*)(bm))[(val)>>3] & BM_RULE_1BIT[(val)&0x7])

static var_u2 BM_RULE_2BIT[] = {0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100, 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1};
#define BM_SET_2BYTE(bm, val)	(((var_u2*)(bm))[(val)>>4] |= BM_RULE_2BIT[(val)&0xF])
#define BM_GET_2BYTE(bm, val)	(((var_u2*)(bm))[(val)>>4] & BM_RULE_2BIT[(val)&0xF])

static var_u4 BM_RULE_4BIT[] = {0x80000000, 0x40000000, 0x20000000, 0x10000000, 0x8000000, 0x4000000, 0x2000000, 0x1000000, 0x800000, 0x400000, 0x200000, 0x100000, 0x80000, 0x40000, 0x20000, 0x10000, 0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100, 0x80, 0x40,	0x20, 0x10, 0x8, 0x4, 0x2, 0x1};
#define BM_SET_4BYTE(bm, val)	(((var_u4*)(bm))[(val)>>5] |= BM_RULE_4BIT[(val)&0x1F])
#define BM_GET_4BYTE(bm, val)	(((var_u4*)(bm))[(val)>>5] & BM_RULE_4BIT[(val)&0x1F])

static var_u8 BM_RULE_8BIT[] = {0x8000000000000000, 0x4000000000000000, 0x2000000000000000, 0x1000000000000000,	0x800000000000000, 0x400000000000000, 0x200000000000000, 0x100000000000000, 0x80000000000000, 0x40000000000000, 0x20000000000000, 0x10000000000000, 0x8000000000000, 0x4000000000000, 0x2000000000000, 0x1000000000000, 0x800000000000, 0x400000000000, 0x200000000000, 0x100000000000, 0x80000000000, 0x40000000000, 0x20000000000, 0x10000000000, 0x8000000000, 0x4000000000, 0x2000000000, 0x1000000000, 0x800000000, 0x400000000, 0x200000000, 0x100000000, 0x80000000, 0x40000000, 0x20000000, 0x10000000, 0x8000000, 0x4000000, 0x2000000, 0x1000000, 0x800000, 0x400000,	0x200000, 0x100000, 0x80000, 0x40000, 0x20000, 0x10000, 0x8000, 0x4000, 0x2000,	0x1000, 0x800, 0x400, 0x200, 0x100, 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1};
#define BM_SET_8BYTE(bm, val)	(((var_u8*)(bm))[(val)>>6] |= BM_RULE_8BIT[(val)&0x3F])
#define BM_GET_8BYTE(bm, val)	(((var_u8*)(bm))[(val)>>6] & BM_RULE_8BIT[(val)&0x3F])
*/

#ifdef _WIN32_ENV_
typedef struct _cp_memory_map_file_
{
	var_vd* pointer;

	_cp_memory_map_file_()
	{
		in_file = NULL;
		pointer = NULL;
	}
	~_cp_memory_map_file_()
	{
		close_MMF();
	}

	var_4 open_MMF(var_1* file,var_1 iMode=0)
	{
		if(file==NULL)
			return -1;

		DWORD dwAccess=GENERIC_READ;
		DWORD dwShare=FILE_SHARE_READ;
		DWORD dwCreat=OPEN_EXISTING;
		DWORD dwAttr=FILE_ATTRIBUTE_READONLY|FILE_FLAG_RANDOM_ACCESS;

		DWORD  dwProtect=PAGE_READONLY;

		DWORD dwMapAccess=FILE_MAP_READ;

		if(iMode==1)
		{
			dwAccess=GENERIC_WRITE;
			dwShare=FILE_SHARE_WRITE;
			dwCreat=TRUNCATE_EXISTING|CREATE_NEW;
			dwAttr= FILE_FLAG_SEQUENTIAL_SCAN;

			dwProtect=PAGE_WRITECOPY;

			dwMapAccess=FILE_MAP_ALL_ACCESS ;
		}
		else if(iMode==2)
		{
			dwAccess=GENERIC_WRITE | GENERIC_READ;

			dwProtect=PAGE_READWRITE;

			dwMapAccess=FILE_MAP_ALL_ACCESS ;
		}

		var_8 ulFileSize=cp_get_file_size(file);
		if(ulFileSize<=0)
		{
			printf("file: %s empty!\n",file);
//			return -2;
			return 0;
		}


		HANDLE hFile = CreateFile(file,dwAccess,dwShare,NULL,dwCreat, dwAttr,NULL);
		if(hFile==NULL)
		{
			printf("createfile:%s failed!,ret=%d\n",file,WSAGetLastError());
			return -3;
		}

		in_file=CreateFileMapping(hFile,NULL,dwProtect,(DWORD)(ulFileSize>>32),(DWORD)(ulFileSize&0xFFFFFFFF),NULL);
		if(in_file==NULL)
		{	
			printf("CreateFileMapping failed! ret=%d\n",WSAGetLastError());
			CloseHandle(hFile);	
			return -4;
		}
		CloseHandle(hFile);

		SYSTEM_INFO sinf;
		GetSystemInfo(&sinf);
		DWORD dwAllocSize=(ulFileSize/sinf.dwAllocationGranularity);
		dwAllocSize+=1;

		pointer =MapViewOfFile(in_file,dwMapAccess,0,0,dwAllocSize);

		if(pointer==NULL)
		{	
			printf("MapViewOfFile failed! ret=%d\n",WSAGetLastError());

			CloseHandle(hFile);
			CloseHandle(in_file);	
			return -5;
		}

		return 0;
	}

	var_4 close_MMF()
	{
		if(pointer)// 从进程的地址空间撤消文件数据映像
		{
			UnmapViewOfFile(pointer);
			pointer=NULL;
		}
		if(in_file)
		{
			CloseHandle(in_file);
			in_file=NULL;
		}

		return 0;
	}
private:
	HANDLE  in_file;

} CP_MMF;
#else
typedef struct _cp_memory_map_file_
{
	var_vd* pointer;

	_cp_memory_map_file_()
	{
		pointer = NULL;
		siFileSize=0;
		m_cOpenFlag=-1;
	}

	~_cp_memory_map_file_()
	{
		close_MMF();
	}

	var_4 open_MMF(var_1* file,var_1 iMode=0)
	{
		if(file==NULL)
			return -1;

		m_cOpenFlag=iMode;

		struct stat stFile;
		if(stat(file,&stFile)<0)
		{
			printf("error to stat %s\n",file);
			return -2;
		}
		siFileSize=stFile.st_size;
		if(siFileSize<=0)
		{
			printf("file %s len:%d\n",file,stFile.st_size);
			return 0;
		}
		int iFlags=O_RDONLY;

		int iPro=PROT_READ;
		if(iMode==1)
		{
			iFlags=O_WRONLY|O_CREAT;

			iPro=PROT_WRITE;
		}
		else if(iMode==2)
		{
			iFlags=O_RDWR;
		}

		int fd=open(file, iFlags);
		if(fd<0)
		{
			printf("error to open %s\n",file);
			return -4;
		}

		pointer = mmap(NULL,siFileSize, iPro,MAP_SHARED,fd,0);
		if(MAP_FAILED == pointer)
		{
			printf("error to mmap,ret:%d\n",errno);
			close(fd);
			return -5;
		}
		close(fd);

		//void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offsize);
		return 0;
	}

	var_4 close_MMF()
	{
		if(pointer)
		{
			if(m_cOpenFlag==1)
				msync(pointer,siFileSize,MS_SYNC);

			munmap(pointer, siFileSize);
			pointer=NULL;
		}		
	}

private:
	size_t  siFileSize;
	var_1   m_cOpenFlag;
} CP_MMF;
#endif

static var_4 cp_drop_useless_char(var_1* str_ptr)
{
	var_4 len = strlen(str_ptr) - 1;
	while(len >= 0 && (str_ptr[len] == '\t' || str_ptr[len] == '\n' || str_ptr[len] == '\r' || str_ptr[len] == ' '))
		len--;	
	str_ptr[++len] = 0;

	return len;
}

#ifdef _WIN32_ENV_
#define U64		"%I64u"
#else
#define U64		"%ul"
#endif

// 时间统计
#ifdef _WIN32_ENV_
typedef struct _cp_stat_time_
{
	DWORD dwTime_1, dwTime_2;

	inline void set_time_begin()
	{
		dwTime_1 = GetTickCount();
	}

	inline void set_time_end()
	{
		dwTime_2 = GetTickCount();
	}

	inline var_4 get_time_s()
	{
		return (dwTime_2 - dwTime_1) / 1000;
	}

	inline var_4 get_time_ms()
	{
		return dwTime_2 - dwTime_1;
	}

	inline var_4 get_time_us()
	{
		return dwTime_2 - dwTime_1;
	}

	static var_4 get_time()
	{
		return GetTickCount();
	}

	inline void clear()
	{
		dwTime_1 = 0;
		dwTime_2 = 0;
	}

} CP_STAT_TIME;
#else
typedef struct _cp_stat_time_
{
	struct timeval tv_1, tv_2;
	
	inline void set_time_begin()
	{
		gettimeofday(&tv_1, 0);
	}

	inline void set_time_end()
	{
		gettimeofday(&tv_2, 0);
	}

	inline var_4 get_time_s()
	{
		return (tv_2.tv_sec - tv_1.tv_sec) + (tv_2.tv_usec - tv_1.tv_usec)/1000000;
	}

	inline var_4 get_time_ms()
	{
		return (tv_2.tv_sec - tv_1.tv_sec)*1000 + (tv_2.tv_usec - tv_1.tv_usec)/1000;
	}

	inline var_4 get_time_us()
	{
		return (tv_2.tv_sec - tv_1.tv_sec)*1000000 + (tv_2.tv_usec - tv_1.tv_usec);
	}

	static var_4 get_time()
	{
		struct timeval tv_T;
		gettimeofday(&tv_T, 0);

		return tv_T.tv_sec*1000000+tv_T.tv_usec;
	}
	
	inline void clear()
	{
		memset(&tv_1, 0, sizeof(struct timeval));
		memset(&tv_2, 0, sizeof(struct timeval));
	}

} CP_STAT_TIME;
#endif

#endif // _UH_DEFINE_H_
