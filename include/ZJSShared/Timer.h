#ifndef _ZJS_TIMER_H_
#define _ZJS_TIMER_H_
#include "Sys.h"
#include "Thread.h"
#include "Mutex.h"

using nsZJSMutex::CMutex;
using nsZJSThread::CThread;

namespace nsZJSTimer
{
#ifdef WINDOWS_PLATFORM
    template <typename TYPE, void (TYPE::*_RunThread)() >
        uint32_t __stdcall thread_rounter(void * m_pParam)
        {
            TYPE *p=(TYPE*)m_pParam;
            p->_RunThread();
            return NULL;
        }


#else
    template <typename TYPE, void (TYPE::*_RunThread)() >
        void *thread_rounter(void * m_pParam)
        {
            TYPE *p=(TYPE*)m_pParam;
            p->_RunThread();
            return NULL;
        }
#endif
    typedef void (*TimerProcessFunc)(
            void                *pParam         // 回调函数使用的参数
            );

    struct STimerEntry
    {
        uint32_t            interVal;           // 定时器的时间间隔
        uint32_t            nextVal;            // 下次运行的时间间隔
        void                *pParam;            // 设定的可扩展参数指针
        TimerProcessFunc    cbFunc;             // 回调函数
        nsZJSThread::CThread *thread;           // 任务线程
        nsZJSMutex::CMutex  *mutex;             // 任务互斥锁
        STimerEntry()
        {
            memset(this, 0, sizeof(STimerEntry));
        }
    };

#ifdef WINDOWS_PLATFORM
    uint32_t __stdcall thread_task(void * m_pParam)
    {
        STimerEntry *entry = (STimerEntry*)m_pParam;
        while(true)
        {
            entry->mutex->Lock();
            entry->cbFunc(entry->pParam);
            entry->mutex->UnLock();
            entry->mutex->Lock();
        }
    }
#else
    void *thread_task(void * m_pParam)
    {
        STimerEntry *entry = (STimerEntry*)m_pParam;
        while(true)
        {
            entry->mutex->Lock();
            entry->cbFunc(entry->pParam);
            entry->mutex->UnLock();
            entry->mutex->Lock();
        }
    }
#endif

    static const int32_t MAX_TIMER = 8;
    class CTimer
    {
        public:
            CTimer(){
                m_closed = false;
                m_interval = 0;
                m_thread.Create(thread_rounter<CTimer, &CTimer::_RunThread>, (void *)this);
            }

            ~CTimer(){
                m_closed = true;
            }

            void SetTimer(int32_t timeid, int32_t interval, void *pParam, TimerProcessFunc callback){
                m_mutex.Lock();
                if(timeid > 0 && timeid <= MAX_TIMER)
                {
                    m_timers[timeid-1].interVal = interval;
                    m_timers[timeid-1].nextVal = interval;
                    m_timers[timeid-1].pParam = pParam;
                    m_timers[timeid-1].cbFunc = callback;
                    m_timers[timeid-1].thread = new CThread();
                    m_timers[timeid-1].thread->Create(thread_task, (void *)(&m_timers[timeid-1]));
                    m_timers[timeid-1].mutex = new CMutex();
                    m_timers[timeid-1].mutex->Lock();
                }
                m_mutex.UnLock();
            }

            void KillTimer(int32_t timeid){
                m_mutex.Lock();
                if(timeid > 0 && timeid <= MAX_TIMER)
                {
                    m_timers[timeid-1].interVal = 0;
                    m_timers[timeid-1].nextVal = 0;
                    m_timers[timeid-1].pParam = 0;
                }
                m_mutex.UnLock();
            }

            void CloseTimer(){
                m_closed = true;
            }

            void _RunThread(){  
                long x[MAX_TIMER];
                long i, num;

                while(!m_closed)
                {
                    m_mutex.Lock();
                    num = 0;
                    // 初始化各定时器下一次触发的值
                    for(i=0; i<MAX_TIMER; ++i)
                    {
                        if(m_timers[i].interVal > 0)
                        {
                            if(m_timers[i].nextVal <= m_interval)
                            {
                                x[num++] = i;
                                m_timers[i].nextVal = m_timers[i].interVal;
                            }
                            else
                            {
                                m_timers[i].nextVal -= m_interval;
                            }
                        }
                    }

                    m_interval = 0;
                    // 获得下次要触发的时间间隔
                    for(i=0; i<MAX_TIMER; i++)
                    {
                        if(m_timers[i].interVal > 0)
                        {
                            if(m_interval == 0)
                            {
                                m_interval = m_timers[i].nextVal;
                            }
                            else if(m_interval > m_timers[i].nextVal)
                            {
                                m_interval = m_timers[i].nextVal;
                            }
                        }
                    }
                    m_mutex.UnLock();

                    if(num > 0)
                    {
                        for(i=0; i<num; i++)
                        {
                            m_timers[x[i]].mutex->UnLock();
                        }
                    }

                    if(m_interval > 0)
                    {
                        num = (m_interval-1)/100 + 1;
                        for(i=0; i<num; i++)
                        {
                            if(m_closed)
                            {
                                break;
                            }
                            Sleep(100);
                        }
                    }
                    else
                    {
                        Sleep(100);
                    }
                }
            }
        private:
            int32_t             m_closed;             // 定时器是否关闭的标记
            uint32_t            m_interval;           // 定时器的时间间隔
            nsZJSThread::CThread  m_thread;           // 定时器运行的定时探测线程
            STimerEntry         m_timers[MAX_TIMER];  // 定时器检测的入口数组，最多8个
            nsZJSMutex::CMutex  m_mutex;              // 定时器运行的互斥锁
    };
}   // end of namespace

#endif //_ZJS_TIMER_H_
