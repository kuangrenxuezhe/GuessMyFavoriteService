#ifndef _ZJS_EVENT_H_
#define _ZJS_EVENT_H_
#include "Sys.h"

namespace nsZJSEvent
{
    class CEvent 
    {
    public:
        // 事件的构造函数
        CEvent()
        {
#ifdef WIN32
            m_event = CreateEvent(0, true, true, "");
#else
            m_signal = false;
            pthread_mutex_init(&m_mutex, 0);
            pthread_cond_init(&m_cond, 0);
#endif
        }

        // 事件的析构函数
        ~CEvent()
        {
#ifdef WIN32
            if(m_event != 0)
            {
                CloseHandle(m_event);
                m_event = 0;
            }
#else
            pthread_mutex_destroy(&m_mutex);
            pthread_cond_destroy(&m_cond);
#endif
        }

        // 事件的等待函数，等待其他线程的唤醒通知
        void Wait()
        {
#ifdef WIN32
            WaitForSingleObject(m_event, INFINITE);
#else
            pthread_mutex_lock(&m_mutex);
            if(m_signal)
            {
                pthread_cond_wait(&m_cond, &m_mutex);
            }
            pthread_mutex_unlock(&m_mutex);
#endif
        }

        // 事件的重置函数，置成初始的无信号状态
        int32_t Reset()
        {
            int32_t flag = 0;

#ifdef WIN32
            if(!ResetEvent(m_event))
            {
                flag = errno;
            }

            return flag;
#else
            pthread_mutex_lock(&m_mutex);
            if(!m_signal)
            {
                m_signal = true;
            }
            pthread_mutex_unlock(&m_mutex);

            return flag;
#endif
        }

        // 事件的通知函数，唤醒等待此事件的线程
        int32_t Signal()
        {
            int32_t flag = 0;

#ifdef WIN32
            if(!SetEvent(m_event))
            {
                flag = errno;
            }

            return flag;
#else
            pthread_mutex_lock(&m_mutex);
            if(m_signal)
            {
                m_signal = false;
                flag = pthread_cond_broadcast(&m_cond);
            }
            pthread_mutex_unlock(&m_mutex);

            return flag;
#endif
        }

    private:
#ifdef WIN32
        HANDLE              m_event;            // windows的事件句柄
#else
        int32_t             m_signal;           // unix的事件信号灯标记，缺省为无信号状态
        pthread_mutex_t     m_mutex;            // posix系统的互斥锁结构
        pthread_cond_t      m_cond;             // posix系统的条件触发器结构
#endif
    };

    // 消息入口的内容数据结构
    struct SMessageEntry{
        struct SMessageEntry    *pPrev;              // 当前消息的前一个消息的结构指针
        struct SMessageEntry    *pNext;              // 当前消息的后一个消息的结构指针
        int32_t                 iCommand;            // 当前消息的命令编号的唯一键值
        void                    *pBuffer;            // 当前消息的命令传递参数的指针
        SMessageEntry(){
            memset(this, 0, sizeof(SMessageEntry));
        }
    };

    class CMessage
    {
    public:
        // 消息的构造函数
        CMessage()
        {
            m_pRoot = 0;

#ifdef WIN32
            InitializeCriticalSection(&m_mutex);
            m_event = CreateEvent(0, true, true, "");
#else
            pthread_mutex_init(&m_mutex, 0);
            pthread_cond_init(&m_cond, 0);
#endif
        }

        // 消息的析构函数
        ~CMessage()
        {
            SMessageEntry *o_pEntry, *n_pEntry;

            n_pEntry = m_pRoot;
            while(n_pEntry != 0)
            {
                o_pEntry = n_pEntry;
                n_pEntry = n_pEntry->pNext;

                delete(o_pEntry);

                if(n_pEntry == m_pRoot)
                {
                    break;
                }
            }

#ifdef WIN32
            DeleteCriticalSection(&m_mutex);
            if(m_event != 0)
            {
                CloseHandle(m_event);
                m_event = 0;
            }
#else
            pthread_mutex_destroy(&m_mutex);
            pthread_cond_destroy(&m_cond);
#endif
        }

        // 从消息循环队列中获取一条消息
        void Get(int32_t &iCommand, void *&pBuffer)
        {
#ifdef WIN32
            WaitForSingleObject(m_event, INFINITE);

            EnterCriticalSection(&m_mutex);

            if(m_pRoot == 0)
            {
                iCommand = 0;
                pBuffer = 0;

                ResetEvent(m_event);
            }
            else
            {
                GetEntry(iCommand, pBuffer);
            }

            LeaveCriticalSection(&m_mutex);
#else
            pthread_mutex_lock(&m_mutex);

            if(m_pRoot == 0)
            {
                iCommand = 0;
                pBuffer = 0;

                pthread_cond_wait(&m_cond, &m_mutex);
            }
            else
            {
                GetEntry(iCommand, pBuffer);
            }

            pthread_mutex_unlock(&m_mutex);
#endif
        }

        // 向消息循环队列中追加一条消息
        void Set(int32_t iCommand, void *pBuffer)
        {
#ifdef WIN32
            EnterCriticalSection(&m_mutex);

            if(!SetEntry(iCommand, pBuffer))
            {
                SetEvent(m_event);
                LeaveCriticalSection(&m_mutex); 
            }
#else
            pthread_mutex_lock(&m_mutex);

            if(!SetEntry(iCommand, pBuffer))
            {
                pthread_cond_signal(&m_cond);
                pthread_mutex_unlock(&m_mutex);
            }
#endif
        }

        // 唤醒消息
        void Signal()
        {
#ifdef WIN32
            SetEvent(m_event);
#else
            pthread_cond_signal(&m_cond);
#endif
        }
    private:    
        void GetEntry(int32_t &iCommand, void *&pBuffer)
        {
            SMessageEntry *pEntry;

            iCommand = 0;
            pBuffer = 0;

            if(m_pRoot == m_pRoot->pNext)
            {
                iCommand = m_pRoot->iCommand;
                pBuffer = m_pRoot->pBuffer;

                delete(m_pRoot);
                m_pRoot = 0;
            }
            else
            {
                m_pRoot->pNext->pPrev = m_pRoot->pPrev;
                m_pRoot->pPrev->pNext = m_pRoot->pNext;
                pEntry = m_pRoot;

                iCommand = pEntry->iCommand;
                pBuffer = pEntry->pBuffer;

                m_pRoot = m_pRoot->pNext;
                delete(pEntry);
            }
        }

        int32_t SetEntry(int32_t iCommand, void *pBuffer)
        {
            SMessageEntry *pEntry, *n_pEntry;

            n_pEntry = new SMessageEntry();
            if(n_pEntry == 0)
                return -1;
            
            n_pEntry->pPrev = 0;
            n_pEntry->pNext = 0;
            n_pEntry->iCommand = iCommand;
            n_pEntry->pBuffer = pBuffer;
        
            if(m_pRoot == 0)
            {
                m_pRoot = n_pEntry;
                n_pEntry->pPrev = n_pEntry;
                n_pEntry->pNext = n_pEntry;
            }
            else
            {
                pEntry = m_pRoot->pPrev;
                pEntry->pNext = n_pEntry;
                n_pEntry->pNext = m_pRoot;
                m_pRoot->pPrev = n_pEntry;
                n_pEntry->pPrev = pEntry;
            }
            return 0;
        }
    private:
        SMessageEntry       *m_pRoot;             // 消息循环队列中的首消息的结构指针
#ifdef WIN32
        CRITICAL_SECTION    m_mutex;              // windows的系统临界区结构
        HANDLE              m_event;              // windows的事件句柄
#else
        pthread_mutex_t     m_mutex;              // posix系统的互斥锁结构
        pthread_cond_t      m_cond;               // posix系统的条件触发器结构
#endif 
    };

    typedef void (*MessageThreadProcess)
        (
        long                iCommand,            // 待处理的消息编号
        void                *pBuffer             // 待处理的消息内容
        );
    // 跨平台的消息驱动的线程的数据结构
    class CMessageThread
    {
    public:
        // 线程的构造函数
        CMessageThread()
        {
            m_thread = 0;
            m_iClosed = false;
            m_process = 0;
            m_pParam = 0;
        }

        // 线程的析构函数
        ~CMessageThread()
        {
            Drop();
        }

        // 处理函数调用

        static void* MessageThreadLoop(void* m_pParam)
        {
            void *pBuffer;
            int32_t iCommand;

            CMessageThread *m_thread = (CMessageThread *)m_pParam;

            while(!m_thread->m_iClosed)
            {
                m_thread->m_message.Get(iCommand, pBuffer);

                if(iCommand != 0)
                {
                    m_thread->m_process(iCommand, pBuffer);
                    if(iCommand < 0)
                    {
                        m_thread->m_iClosed = true;
                        break;
                    }
                }
            }

            return 0;
        }
        // 线程创建的函数
        int32_t Create(MessageThreadProcess process)
        {          
#ifdef WIN32
            m_thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CMessageThread::MessageThreadLoop, (void*)this, 0, 0);
            if(m_thread == 0)
            {
                return errno;
            }
#else
            if(pthread_create(&m_thread, 0, CMessageThread::MessageThreadLoop, this) != 0)
            {
                return errno;
            }
#endif
            m_process = process;
            return 0;
        }

        // 给线程发送命令消息
        void Message(long iCommand, void *pBuffer)
        {
            if(!m_iClosed)
            {
                m_message.Set(iCommand, pBuffer);
            }
        }

    private:
        // 线程等待同步关闭的函数
        void Drop()
        {
            m_iClosed = true;
            m_message.Signal();
            Join();
            Close();
        }

        // 线程等待同步的函数
        void Join()
        {
#ifdef WIN32
            if(m_thread != 0)
            {
                WaitForSingleObject(m_thread, INFINITE);
            }
#else
            if(m_thread != 0)
            {
                pthread_join(m_thread, 0);
            }
#endif
        }

        // 线程强制关闭的函数
        int32_t Close()
        {
#ifdef WIN32
            int32_t retval;

            if(m_thread == 0)
            {
                return 0;
            }

            retval = CloseHandle(m_thread);
            m_thread = 0;

            if(retval == 0)
            {
                return errno;
            }
#endif

            return 0;
        }

    private:
#ifdef WIN32
        HANDLE              m_thread;             // windows的线程句柄
#else
        pthread_t           m_thread;             // posix的线程结构
#endif
        int32_t             m_iClosed;            // 线程关闭状态标记
        CMessage            m_message;            // 消息队列的结构
        MessageThreadProcess m_process;           // 消息处理的主函数地址
        void                *m_pParam;            // 线程的可扩展参数指针
    };

}   // end of namespace
#endif //_ZJS_EVENT_H_
