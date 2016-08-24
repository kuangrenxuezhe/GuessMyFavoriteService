#ifndef _ZJS_QUEUE_H_
#define _ZJS_QUEUE_H_
#include "Sys.h"

namespace nsZJSQueue
{
    struct SQueueItem
    {
        SQueueItem *next;
        void       *data;
        SQueueItem()
        {
            memset(this, 0, sizeof (SQueueItem));
        }
    };

    class CQueue 
    {
    public:
        // 队列构造函数
        CQueue()
        {
            m_head = 0;
            m_tail = 0;
        }

        // 队列析构函数
        ~CQueue()
        {
            SQueueItem *item = m_head;
            while (item) 
            {
                SQueueItem *nitem = item->next;
                delete item;
                item = nitem;
            }
        }
        // 从队列头插入
        void PushHead(void *data)
        {
            SQueueItem *item = new SQueueItem;

            item->next = m_head;
            item->data = data;

            m_head = item;

            if (!m_tail) {
                m_tail = item;
            }
        }
        // 从队列尾插入
        void PushTail (void *data)
        {
            SQueueItem *item = new SQueueItem;
            item->data = data;

            if (m_tail) {
                m_tail->next = item;
            } else {
                m_head = item;
            }

            m_tail = item;
        }

        //从队列头部弹出
        void *PopHead()
        {
            SQueueItem *item;
            void *data = NULL;
            if ((item = m_head)) {
                if (!item->next) {
                    m_tail = NULL;
                }
                m_head = item->next;
                data = item->data;
                delete item;
            }

            return data;
        }

        //获得队列长度
        uint32_t GetLength()
        {
            SQueueItem *item;
            uint32_t count = 0;
            for (item = m_head; item; item = item->next) {
                count++;
            }

            return count;
        }
    private:
        SQueueItem *m_head;
        SQueueItem *m_tail;
    };

}   // end of namespace
#endif //_ZJS_QUEUE_H_
