#ifndef _ZJS_MUTEX_H_
#define _ZJS_MUTEX_H_
#include "Sys.h"

namespace nsZJSMutex
{
    //线程锁
    class CMutex
    {
    public:
        // 互斥锁的构造函数
        CMutex()
        {
#ifdef WIN32
            InitializeCriticalSection(&m_mutex);		
#else
            pthread_mutex_init(&m_mutex, NULL);	
#endif
        }

        // 互斥锁的析构函数
        ~CMutex()
        {
#ifdef	WIN32
            DeleteCriticalSection(&m_mutex);		
#else
            pthread_mutex_destroy(&m_mutex);	
#endif
        }

        // 互斥锁的尝试加锁函数，返回是否加锁成功
        inline int32_t TryLock()
        {
#ifdef WIN32
            return TryEnterCriticalSection(&m_mutex);
#else
            return !pthread_mutex_trylock(&m_mutex);
#endif
        }

        // 互斥锁的加锁函数
        inline void Lock()
        {
#ifdef WIN32
            EnterCriticalSection(&m_mutex);		
#else
            pthread_mutex_lock(&m_mutex);	
#endif
        }

        // 互斥锁的解锁函数
        inline void UnLock()
        {
#ifdef WIN32
            LeaveCriticalSection(&m_mutex);		
#else
            pthread_mutex_unlock(&m_mutex);	
#endif
        }

    private:
#ifdef WIN32
        CRITICAL_SECTION    m_mutex;              // windows的系统临界区结构
#else
        pthread_mutex_t     m_mutex;              // posix系统的互斥锁结构
#endif
    };

    class CRWLock
    {
    public:    
        // 读写锁的构造函数
        CRWLock()
        {
#ifdef WIN32
            InitializeCriticalSection(&m_mutex);
            m_writer = CreateSemaphore(0, 1, 1, 0);
            m_reader = 0;
#else
            pthread_rwlock_init(&m_rwlock, 0);
#endif
        }

        // 读写锁的析构函数
        ~CRWLock()
        {
#ifdef WIN32
            CloseHandle(m_writer);
            DeleteCriticalSection(&m_mutex);
#else
            pthread_rwlock_destroy(&m_rwlock);
#endif
        }

        // 读写锁的尝试加读锁函数，返回是否加读锁成功
        int32_t TryRLock()
        {
#ifdef WIN32
            if(!TryEnterCriticalSection(&m_mutex))
            {
                return false;
            }

            if(m_reader <= 0)
            {
                m_reader = 0;
                if(WaitForSingleObject(m_writer, 0) != WAIT_OBJECT_0)
                {
                    LeaveCriticalSection(&m_mutex);
                    return false;
                }
            }
            m_reader ++;

            LeaveCriticalSection(&m_mutex);
            return true;
#else
            return !pthread_rwlock_tryrdlock(&m_rwlock);
#endif
        }

        // 读写锁的加读锁函数
        void Rlock()
        {
#ifdef WIN32
            EnterCriticalSection(&m_mutex);

            if(m_reader <= 0)
            {
                m_reader = 0;
                WaitForSingleObject(m_writer, INFINITE);
            }
            m_reader ++;

            LeaveCriticalSection(&m_mutex);
#else
            pthread_rwlock_rdlock(&m_rwlock);
#endif
        }

        // 读写锁的解读锁函数
        void UnRlock()
        {
#ifdef WIN32
            EnterCriticalSection(&m_mutex);

            m_reader --;
            if(m_reader <= 0)
            {
                m_reader = 0;
                ReleaseSemaphore(m_writer, 1, 0);
            }

            LeaveCriticalSection(&m_mutex);
#else
            pthread_rwlock_unlock(&m_rwlock);
#endif
        }

        // 读写锁的尝试加写锁函数，返回是否加写锁成功
        int32_t TryWLock()
        {
#ifdef WIN32
            if(WaitForSingleObject(m_writer, 0) != WAIT_OBJECT_0)
            {
                return false;
            }

            return true;
#else
            return !pthread_rwlock_trywrlock(&m_rwlock);
#endif
        }

        // 读写锁的加写锁函数
        void WLock()
        {
#ifdef WIN32
            WaitForSingleObject(m_writer, INFINITE);
#else
            pthread_rwlock_wrlock(&m_rwlock);
#endif
        }

        // 读写锁的解写锁函数
        void UnWLock()
        {
#ifdef WIN32
            ReleaseSemaphore(m_writer, 1, 0);
#else
            pthread_rwlock_unlock(&m_rwlock);
#endif
        }

    private:    
#ifdef WIN32
        CRITICAL_SECTION    m_mutex;              // windows的系统临界区结构
        long                m_reader;             // windows的读取计数器
        HANDLE              m_writer;             // windows的写入信号灯
#else
        pthread_rwlock_t    m_rwlock;             // posix系统的读写锁结构
#endif
    };

}   // end of namespace
#endif //_ZJS_MUTEX_H_
