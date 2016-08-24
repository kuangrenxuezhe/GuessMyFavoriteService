// UT_Queue_NB.h NB - nonblocking

#ifndef _UT_QUEUE_NB_H_
#define _UT_QUEUE_NB_H_

#include "UH_Define.h"

template <class T_Key>
class UT_Queue_NB
{
public:
	UT_Queue_NB()
	{
		m_lMaxQueueSize = -1;
		m_lCurQueueSize = -1;

		m_tpQueue = NULL;

		m_lBegPos = -1;
		m_lEndPos = -1;
	};

	~UT_Queue_NB()
	{
		if(m_tpQueue)
			delete m_tpQueue;
	};

	var_4 InitQueue(var_4 lMaxQueueSize)
	{
		m_lMaxQueueSize = lMaxQueueSize;
		m_lCurQueueSize = 0;
		
		m_tpQueue = new T_Key[m_lMaxQueueSize];
		if(m_tpQueue == NULL)
			return -1;
		m_lBegPos = 0;
		m_lEndPos = 0;

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

		m_lMaxQueueSize = -1;
		m_lCurQueueSize = -1;

		m_lBegPos = -1;
		m_lEndPos = -1;
	};

	var_4 IsQueueFull()
	{
		if(m_lCurQueueSize == m_lMaxQueueSize)
			return 1;

		return 0;
	};

	var_4 IsQueueEmpty()
	{
		if(m_lCurQueueSize == 0)
			return 1;

		return 0;
	};

	var_4 PushData(T_Key tKey)
	{
		m_cQueueCriSec.lock();
		if(m_lCurQueueSize >= m_lMaxQueueSize)
		{
			m_cQueueCriSec.unlock();
			return -1;
		}
		m_tpQueue[m_lEndPos++] = tKey;
		m_lEndPos %= m_lMaxQueueSize;
		m_lCurQueueSize++;			
		m_cQueueCriSec.unlock();

		return 0;
	};

	var_4 PopData(T_Key& tKey)
	{
		m_cQueueCriSec.lock();
		if(m_lCurQueueSize <= 0)
		{
			m_cQueueCriSec.unlock();
			return -1;
		}
		tKey = m_tpQueue[m_lBegPos++];
		m_lBegPos %= m_lMaxQueueSize;
		m_lCurQueueSize--;
		m_cQueueCriSec.unlock();

		return 0;
	};

	// 得到当前版本号
	var_1* GetVersion()
	{
		// v1.000 - 2008.08.26 - 初始版本
		// v1.001 - 2008.12.04 - 增加函数 IsQueueFull(), IsQueueEmpty()
		// v1.100 - 2009.04.10 - 增加跨平台支持
		return "v1.100";
	}

private:
	var_4 m_lMaxQueueSize;
	var_4 m_lCurQueueSize;

	T_Key* m_tpQueue;

	var_4 m_lBegPos;
	var_4 m_lEndPos;

	CP_MUTEXLOCK m_cQueueCriSec;
	CP_MUTEXLOCK m_cEventCriSec;
};

#endif // _UT_QUEUE_NB_H_
