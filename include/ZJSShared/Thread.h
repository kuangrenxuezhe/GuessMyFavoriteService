#ifndef _ZJS_THEARD_H_
#define _ZJS_THEARD_H_
#include "Sys.h"

#ifdef WIN32
typedef uint32_t WINAPI TFunHead;
#else
typedef void* TFunHead;
#endif
namespace nsZJSThread
{
    class CThread{
    public:
        CThread(int thread_id = 0){
            m_thread = 0;
            m_threadid = thread_id;
        }

        ~CThread(){
            Close();
        }

#ifdef WINDOWS_PLATFORM  
        int32_t Create(uint32_t (WINAPI process)(void*), void *pParam)
        {
            m_thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)process, pParam, 0, 0);
            if(m_thread == 0){
                return -100;
            }

            return 0;
        }
#else
        int32_t Create(void* (process)(void*), void *pParam)
        {
            if(pthread_create(&m_thread, 0, process, pParam) != 0)
            {
                return -100;
            }

            return 0;
        }
#endif      

    private:
		int32_t Close(){
            if(m_thread == 0){
                return 0;
            }
#ifdef WIN32
            WaitForSingleObject(m_thread, INFINITE);
            if(CloseHandle(m_thread)){
                return -100;
            }
            m_thread = 0;
#else
            pthread_join(m_thread, 0);
			return 0;
#endif
        }

    public:
        long                m_threadid;             // 线程的唯一编号
    private:
#ifdef WIN32
        HANDLE              m_thread;               // windows的线程句柄
#else
        pthread_t           m_thread;               // posix的线程结构
#endif
    };

    class CThreadManage
    {
    public:
        CThreadManage(){
        }

        ~CThreadManage(){
        }

        void Init(const uint32_t count = 1)
        {
            m_iMax = count;
        }
        // 启动线程_带返回结果
#ifdef WIN32
        const int32_t StartupThread(uint32_t (WINAPI in_function)(void*), void* in_vpArg)
        {
            uint32_t dwThreadID=0;
            for(uint32_t i=0; i<m_iMax; i++)
            {
                HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)in_function, in_vpArg, 0, (LPDWORD)&dwThreadID);
                if(hThread == NULL)
                    return -1;
                CloseHandle(hThread);
                Sleep(10);
            }

            return 0;
        }
#else  
        const int32_t StartupThread(void* (in_function)(void*), void* in_vpArg)
        {
            pthread_t pid = 0;
            for(uint32_t i=0; i<m_iMax; i++)
            {
                pthread_create(&pid, 0, in_function, in_vpArg);
                if(0 == pid)
                    return -1;
                Sleep(10);
            }
            return 0;
        }
#endif	
    private:
        uint32_t m_iMax;
    };  
}   // end of namespace

#endif  // end of _ZJS_THEARD_H_
