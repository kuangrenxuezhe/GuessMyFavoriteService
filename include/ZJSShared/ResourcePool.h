#ifndef _ZJS_RESOURCE_POOL_H_
#define _ZJS_RESOURCE_POOL_H_

#include "Sys.h"
#include "Event.h"
#include "Mutex.h"
#include "Queue.h"

namespace nsZJSResourcePool 
{
    typedef void *(*ResourceNewFunc)();
    class CResourcePool 
    {
    public:
        // 事件的构造函数
        CResourcePool()
        {
            m_minPoolSize = 0;
            m_maxPoolSize = 100;
            m_size = 0;
        }

        // 事件的析构函数
        ~CResourcePool()
        {
        }

        uint32_t Init(uint32_t minPoolSize, uint32_t maxPoolSize, void* (in_function)())
        {
            m_minPoolSize = minPoolSize;    
            m_maxPoolSize = maxPoolSize;
            m_resourceNewFunc = in_function;
        }

        void *Pop()
        {
            void *data;
            m_mutex.Lock();
            again:
            if (!(data = m_queue.PopHead())) {
                if (m_size < m_maxPoolSize) {
                    data = m_resourceNewFunc();
                    m_size++;
                } else {
                    m_event.Wait();
                    goto again;
                }
            }

            m_mutex.UnLock();
            return data;
        }

        void *TryPop()
        {
            void *data = 0;
            m_mutex.Lock();

            if (!(data = m_queue.PopHead())) {
                if (m_size < m_maxPoolSize) {
                    data = m_resourceNewFunc();
                    m_size++;
                }
            }
            m_mutex.UnLock();
            return data;
        }

        void Push(void *data)
        {
            m_mutex.Lock();
            m_queue.PushHead(data);
            m_event.Signal();
            m_mutex.UnLock();
        }

    private:
        nsZJSEvent::CEvent          m_event;
        nsZJSMutex::CMutex          m_mutex;
        nsZJSQueue::CQueue          m_queue;
        uint32_t        m_minPoolSize;
        uint32_t        m_maxPoolSize;
        uint32_t        m_size;
        ResourceNewFunc m_resourceNewFunc;
    };
}   // end of namespace
#endif //_ZJS_RESOURCE_POOL_H_