// UT_Queue_Event.h
//一个采用event实现的线程安全的队列
#ifndef _UT_QUEUE_EVENT_H_
#define _UT_QUEUE_EVENT_H_

#include <windows.h>

template <class T_Key>
class UT_Queue_Event
{
public:
	UT_Queue_Event()
	{
		InitializeCriticalSection(&m_cQueueCriSec);
		InitializeCriticalSection(&m_cEventCriSec);

		m_lMaxQueueSize = -1;
		m_lCurQueueSize = -1;

		m_tpQueue = NULL;
		m_hEvent = NULL;

		m_lBegPos = -1;
		m_lEndPos = -1;
	};

	~UT_Queue_Event()
	{
		DeleteCriticalSection(&m_cQueueCriSec);
		DeleteCriticalSection(&m_cEventCriSec);

		if(m_tpQueue)
			delete m_tpQueue;
		if(m_hEvent)
			CloseHandle(m_hEvent);
	};

	long InitQueue(long lMaxQueueSize)
	{
		m_lMaxQueueSize = lMaxQueueSize;
		m_lCurQueueSize = 0;
		
		m_tpQueue = new T_Key[m_lMaxQueueSize];
		if(m_tpQueue == NULL)
			return -1;
		m_lBegPos = 0;
		m_lEndPos = 0;

		long lNum = 0;
		char szaEventName[32];
		do{
			sprintf(szaEventName, "_UT_QUEUE_H_%x_%.2d", this, lNum++);
			m_hEvent = CreateEvent(NULL, TRUE, FALSE, szaEventName);
		} while(GetLastError() == ERROR_ALREADY_EXISTS);
		if(m_hEvent == NULL)
		{
			delete m_tpQueue;
			m_tpQueue = NULL;
			return -1;
		}

		return 0;
	};

	void ResetQueue()
	{		
		m_lCurQueueSize = 0;

		m_lBegPos = 0;
		m_lEndPos = 0;	
	};

	void ClearQueue()
	{
		if(m_tpQueue)
		{
			delete m_tpQueue;
			m_tpQueue = NULL;
		}
		if(m_hEvent)
		{
			CloseHandle(m_hEvent);
			m_hEvent = NULL;
		}

		m_lMaxQueueSize = -1;
		m_lCurQueueSize = -1;

		m_lBegPos = -1;
		m_lEndPos = -1;
	};

	void PushData(T_Key tKey)
	{
		for(;;)
		{
			EnterCriticalSection(&m_cQueueCriSec);
			if(m_lCurQueueSize >= m_lMaxQueueSize)
			{
				LeaveCriticalSection(&m_cQueueCriSec);
				Sleep(1);
				continue;
			}
			m_tpQueue[m_lEndPos++] = tKey;
			m_lEndPos %= m_lMaxQueueSize;//这么写要比判断的方式简单的多
			if(++m_lCurQueueSize == 1)//处于队列头
				SetEvent(m_hEvent);
			LeaveCriticalSection(&m_cQueueCriSec);

			break;
		}
	};

	T_Key PopData()
	{
		EnterCriticalSection(&m_cEventCriSec);
		WaitForSingleObject(m_hEvent, INFINITE);

		EnterCriticalSection(&m_cQueueCriSec);
		T_Key tResult = m_tpQueue[m_lBegPos++];
		m_lBegPos %= m_lMaxQueueSize;
		if(--m_lCurQueueSize <= 0)
			ResetEvent(m_hEvent);
		LeaveCriticalSection(&m_cQueueCriSec);

		LeaveCriticalSection(&m_cEventCriSec);

		return tResult;
	};

	long IsQueueFull()
	{
		if(m_lCurQueueSize == m_lMaxQueueSize)
			return 1;

		return 0;
	};

	long IsQueueEmpty()
	{
		if(m_lCurQueueSize == 0)
			return 1;

		return 0;
	};

	// 得到当前版本号
	char* GetVersion()
	{
		// 初始版本 - v1.000 - 2008.08.26
		//          - v1.001 - 2008.12.04 - 增加函数 IsQueueFull(), IsQueueEmpty()
		return "v1.000";
	}

private:
	long m_lMaxQueueSize;
	long m_lCurQueueSize;

	T_Key* m_tpQueue;
	HANDLE m_hEvent;

	long m_lBegPos;
	long m_lEndPos;

	CRITICAL_SECTION m_cQueueCriSec;
	CRITICAL_SECTION m_cEventCriSec;
};

#endif // _UT_QUEUE_EVENT_H_