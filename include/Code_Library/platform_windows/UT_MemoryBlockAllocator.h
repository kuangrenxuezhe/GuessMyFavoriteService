// UT_MemoryBlockAllocator.h

#ifndef _UT_MEMORY_BLOCK_ALLOCATOR_H_
#define _UT_MEMORY_BLOCK_ALLOCATOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define DEFAULTBLOCKNUM		1024
#define DEFAULTBLOCKSIZE    1048576

struct _Node_MB_
{
	_Node_MB_* cpNext;
	void* vpHandle;
	_Node_MB_()
	{
		cpNext = NULL;
		vpHandle = NULL;
	}
};

template <class T>
class UT_MemoryBlockAllocator
{
public:
	UT_MemoryBlockAllocator()
	{
		m_cpNodeLibrary = NULL;
		m_cpAllocHead = NULL;
		m_cpFreeHead = NULL;
		m_cpWaitHead = NULL;

		m_lBlockSize = DEFAULTBLOCKSIZE;
		m_lBlockNum = DEFAULTBLOCKNUM;

		m_tBlockLibrary = NULL;

		m_lFreeMode = 0;

#ifdef _MT
		InitializeCriticalSection(&m_cAllocCriSec);
		InitializeCriticalSection(&m_cFreeCriSec);
		InitializeCriticalSection(&m_cWaitCriSec);
#endif
	};

	~UT_MemoryBlockAllocator()
	{
		if(m_tBlockLibrary)
			delete m_tBlockLibrary;
		if(m_cpNodeLibrary)
			delete m_cpNodeLibrary;

#ifdef _MT
		DeleteCriticalSection(&m_cAllocCriSec);
		DeleteCriticalSection(&m_cFreeCriSec);
		DeleteCriticalSection(&m_cWaitCriSec);
#endif
	};

	long InitMemoryBlockAllocator(long lBlockSize = DEFAULTBLOCKSIZE, long lBlockNum = DEFAULTBLOCKNUM, long lFreeMode = 0)
	{
		if(lBlockSize <= 0 || lBlockNum <= 0)
		{
			m_lBlockNum = DEFAULTBLOCKNUM;
			m_lBlockSize = DEFAULTBLOCKSIZE;
		}
		else
		{
			m_lBlockNum = lBlockNum;
			m_lBlockSize = lBlockSize;
		}

		m_tBlockLibrary = new T[m_lBlockNum * m_lBlockSize];
		if(m_tBlockLibrary == NULL)
			return -1;
		m_cpNodeLibrary = new _Node_MB_[m_lBlockNum];
		if(m_cpNodeLibrary == NULL)
		{
			delete m_tBlockLibrary;
			m_tBlockLibrary = NULL;
			return -1;
		}

		for(long i = 0; i < m_lBlockNum; i++)
		{
			m_cpNodeLibrary[i].vpHandle = (void*)(m_tBlockLibrary + i * m_lBlockSize);
			*(char*)m_cpNodeLibrary[i].vpHandle = 0;
			m_cpNodeLibrary[i].cpNext = m_cpFreeHead;//第二个指向第一个，第三个指向第二个
			m_cpFreeHead = m_cpNodeLibrary + i;
		}

		m_lFreeMode = lFreeMode;

		if(m_lFreeMode)
		{
			DWORD dwThreadID;
			HANDLE hThreadHD = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_FreeMemBlock, (void*)this, 0, &dwThreadID);
			if(hThreadHD == NULL)
				return -1;
			CloseHandle(hThreadHD);
		}

		return 0;
	};

	long AllocMemBlock(T*& tBuf)
	{
#ifdef _MT
		EnterCriticalSection(&m_cFreeCriSec);
#endif
		if(m_cpFreeHead == NULL)
		{
#ifdef _MT
			LeaveCriticalSection(&m_cFreeCriSec);
#endif
			tBuf = NULL;
			return -1;
		}
		_Node_MB_* cpNode = m_cpFreeHead;
		m_cpFreeHead = m_cpFreeHead->cpNext;
#ifdef _MT
		LeaveCriticalSection(&m_cFreeCriSec);
#endif

		tBuf = (T*)cpNode->vpHandle;
		cpNode->vpHandle = NULL;
		if(m_lFreeMode == 1)
			*tBuf = 1;

#ifdef _MT
		EnterCriticalSection(&m_cAllocCriSec);
#endif
		cpNode->cpNext = m_cpAllocHead;//构建已分配内存队列
		m_cpAllocHead = cpNode;
#ifdef _MT
		LeaveCriticalSection(&m_cAllocCriSec);
#endif

		return 0;
	};

	long FreeMemBlock(T*& tBuf)
	{
		_Node_MB_* cpNode = NULL;

#ifdef _MT
		EnterCriticalSection(&m_cAllocCriSec);
#endif
		if(m_cpAllocHead == NULL)
		{
#ifdef _MT
			LeaveCriticalSection(&m_cAllocCriSec);
#endif
			return -1;
		}
		cpNode = m_cpAllocHead;
		m_cpAllocHead = m_cpAllocHead->cpNext;
#ifdef _MT
		LeaveCriticalSection(&m_cAllocCriSec);
#endif

		cpNode->vpHandle = (void*)tBuf;
		tBuf = NULL;

		if(m_lFreeMode == 0)
		{
#ifdef _MT
			EnterCriticalSection(&m_cFreeCriSec);
#endif
			cpNode->cpNext = m_cpFreeHead;
			m_cpFreeHead = cpNode;
#ifdef _MT
			LeaveCriticalSection(&m_cFreeCriSec);
#endif
		}
		else
		{
#ifdef _MT
			EnterCriticalSection(&m_cWaitCriSec);
#endif
			cpNode->cpNext = m_cpWaitHead;
			m_cpWaitHead = cpNode;
#ifdef _MT
			LeaveCriticalSection(&m_cWaitCriSec);
#endif
		}

		return 0;
	};

	static long Thread_FreeMemBlock(void* vpArg)
	{
		UT_MemoryBlockAllocator<T>* cpClass = (UT_MemoryBlockAllocator<T>*)vpArg;

		_Node_MB_* cpNewHead = NULL;
		_Node_MB_* cpOldHead = NULL;
		_Node_MB_* cpFreeNode = NULL;

		for(;;)
		{
#ifdef _MT
			EnterCriticalSection(&cpClass->m_cWaitCriSec);
#endif		
			if(cpClass->m_cpWaitHead == NULL && cpNewHead == NULL)
			{
#ifdef _MT
				LeaveCriticalSection(&cpClass->m_cWaitCriSec);
#endif
				Sleep(1);
				continue;
			}

			if(cpClass->m_cpWaitHead == NULL)
				cpOldHead = cpNewHead;
			else
			{
				cpOldHead = cpClass->m_cpWaitHead;
				cpClass->m_cpWaitHead = cpNewHead;
			}
#ifdef _MT
			LeaveCriticalSection(&cpClass->m_cWaitCriSec);
#endif
			cpNewHead = NULL;

			while(cpOldHead)
			{
				cpFreeNode = cpOldHead;
				cpOldHead = cpOldHead->cpNext;
				if(*((char*)cpFreeNode->vpHandle))
				{
#ifdef _MT
					EnterCriticalSection(&cpClass->m_cFreeCriSec);
#endif
					cpFreeNode->cpNext = cpClass->m_cpFreeHead;
					cpClass->m_cpFreeHead = cpFreeNode;
#ifdef _MT
					LeaveCriticalSection(&cpClass->m_cFreeCriSec);
#endif
				}
				else
				{
					cpFreeNode->cpNext = cpNewHead;
					cpNewHead = cpFreeNode;					
				}
			}

			Sleep(1);
		}

		return 0;
	}

	// 得到当前版本号
	char* GetVersion()
	{
		// 初始版本 - v1.000 - 2008.08.26
		return "v1.000";
	}

public:
	_Node_MB_* m_cpNodeLibrary;
	_Node_MB_* m_cpAllocHead;
	_Node_MB_* m_cpFreeHead;
	_Node_MB_* m_cpWaitHead;	

	long m_lBlockSize;
	long m_lBlockNum;

	T* m_tBlockLibrary;

	long m_lFreeMode;

#ifdef _MT
	CRITICAL_SECTION m_cAllocCriSec;
	CRITICAL_SECTION m_cFreeCriSec;
	CRITICAL_SECTION m_cWaitCriSec;
#endif
};

#endif // _UT_MEMORY_BLOCK_ALLOCATOR_H_
