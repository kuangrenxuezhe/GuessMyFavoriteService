// UC_Tools.h

#ifndef _UC_TOOLS_H_
#define _UC_TOOLS_H_
#include <Windows.h>
class UC_Tools
{
public:
	// 高性能计数器
	static LONGLONG GetTimeCount()
	{
		LARGE_INTEGER cLI;
		QueryPerformanceCounter(&cLI);//获得初始值

		return cLI.QuadPart;
	}
	static long GetTimeResult(LONGLONG time_1, LONGLONG time_2)
	{
		LARGE_INTEGER cLI;
		QueryPerformanceFrequency(&cLI);//获得时钟频率
		double base = (double)cLI.QuadPart;
		
		return (long)((double)(time_2 - time_1)/base*1000);//多少毫秒
	}
	static double GetTimeResult_High(LONGLONG time_1, LONGLONG time_2)
	{
		LARGE_INTEGER cLI;
		QueryPerformanceFrequency(&cLI);//获得时钟频率
		double base = (double)cLI.QuadPart;
		
		return (double)(time_2 - time_1)/base;
	}
	// 启动线程_带返回结果
	static long StartThread(DWORD (WINAPI in_function)(void*), void* in_vpArg, HANDLE& out_hThreadHandle, DWORD& out_dwThreadID)
	{
		out_hThreadHandle = CreateThread(NULL, 0, in_function, in_vpArg, 0, &out_dwThreadID);
		if(out_hThreadHandle == NULL)
			return -1;

		return 0;
	}
	// 启动线程_无返回结果
	static long StartThread(DWORD (WINAPI in_function)(void*), void* in_vpArg, long in_thread_num = 1)
	{
		DWORD dwThreadID;

		for(long i = 0; i < in_thread_num; i++)
		{			
			HANDLE hThreadHandle = CreateThread(NULL, 0, in_function, in_vpArg, 0, &dwThreadID);
			if(hThreadHandle == NULL)
				return -1;
			CloseHandle(hThreadHandle);
		}

		return 0;
	}
	// 得到当前版本号
	static char* GetVersion()
	{
		// 初始版本 - v1.000 - 2008.08.20 
		return "v1.000";
	}
};

typedef UC_Tools UT;

#endif // _UC_TOOLS_H_
