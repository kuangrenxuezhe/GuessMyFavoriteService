// UH_DetectMemoryLeak.h

#ifndef _UH_DETECT_MEMORY_LEAK_H_
#define _UH_DETECT_MEMORY_LEAK_H_

#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC

#include <stdlib.h>
#include <crtdbg.h>

inline void* __cdecl operator new(unsigned int a, char* b, int c)
		{ return ::operator new(a, _NORMAL_BLOCK, b, c); }
inline void __cdecl operator delete(void *a, char* b, int c)
		{  ::operator delete(a, _NORMAL_BLOCK, b, c); }
/*
inline void operator delete(void *pUserData)
{
	::operator delete(pUserData);
}
*/

#define new new(__FILE__, __LINE__)
/*
#define delete delete(__FILE__, __LINE__)
*/

#define DETECTMEMORYLEAK() _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);\
	                       _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);\
				           _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);\
				           _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);\
                           _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);\
				           _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);\
				           _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT)

typedef struct _DETECTMEMORYLEAK_STRUCT
{
	_DETECTMEMORYLEAK_STRUCT()
	{
		DETECTMEMORYLEAK();
	}
} DETECTMEMORYLEAK_STRUCT;

static DETECTMEMORYLEAK_STRUCT DETECTMEMORYLEAK_INIT;

#endif

#endif // _UH_DETECT_MEMORY_LEAK_H__
