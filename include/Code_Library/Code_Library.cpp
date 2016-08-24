// CodeLibrary.cpp

/************************************************************************/
// for UC_FileTransfers_Client.h and UC_FileTransfers_Server.h                                                                   
/************************************************************************/
/*
#include "./unstable/UC_FileTransfers_Client.h"
#include "./unstable/UC_FileTransfers_Server.h"

long test_uc_file_transfers()
{
	UC_FileTransfers_Server cServer;
	if(cServer.InitSystem(5555, 10))
		return -1;

	UC_FileTransfers_Client cClient;
	if(cClient.InitSystem())
		return -1;

	char* p = new char[10<<20];
	for(long i = 0; i < 1048576; i++)
		memcpy(p+i*10, "0123456789", 10);

//	FH* fh_w = cClient.OpenFile("\\\\192.168.15.62\\e$\\src.txt", 'w', 1029, 5555);
//	FH* fh_w = cClient.OpenFile("e:\\src.txt", 'w', 1029, 5555);
//	FH* fh_w = cClient.OpenFile("192.168.15.62", "192.168.15.62", "e:\\src.txt", 'w', 1029, 5555);
	FH* fh_w = cClient.OpenFile("192.168.15.62", "192.168.15.52", "e:\\src.txt", 'w', 1029, 5555);
	cClient.WriteFile(p, 10, fh_w);
	cClient.WriteFile(p + 10, (10<<20) - 30, fh_w);
	cClient.WriteFile(p + (10<<20) - 20, 20, fh_w);
	cClient.CloseFile(fh_w);

	memset(p, 0, 10<<20);

//	FH* fh_r = cClient.OpenFile("\\\\192.168.15.62\\e$\\src.txt", 'r', 1020, 5555);
//	FH* fh_r = cClient.OpenFile("e:\\src.txt", 'r', 1020, 5555);
//	FH* fh_r = cClient.OpenFile("192.168.15.62", "192.168.15.62", "e:\\src.txt", 'r', 1020, 5555);
	FH* fh_r = cClient.OpenFile("192.168.15.62", "192.168.15.52", "e:\\src.txt", 'r', 1020, 5555);
	cClient.ReadFile(p, 10, fh_r);
	cClient.ReadFile(p + 10, (10<<20) - 30, fh_r);
	cClient.ReadFile(p + (10<<20) - 20, 10, fh_r);
	cClient.ReadFile(p + (10<<20) - 10, 10, fh_r);
	cClient.CloseFile(fh_r);

	FILE* fp = fopen("e:\\des.txt", "wb");
	fwrite(p, 10<<20, 1, fp);
	fclose(fp);	

	delete p;

	return 0;
}
*/

/************************************************************************/
// quick sort
/************************************************************************/
/*
typedef struct aaa
{
	long a;
	long b;

	inline long operator > (aaa& b)
	{
		return a > b.a;
	};
	inline long operator < (aaa& b)
	{
		return a < b.a;
	};
} AAA;

long Comp(AAA* a, AAA* b)
{
	if(a->a > b->a)
		return 1;
	if(a->a < b->a)
		return -1;
	return 0;
}

long QuickSort(long lBegin, long lEnd, AAA** tKey, long (*_CompareKey)(AAA*, AAA*))
{
	if(lBegin >= lEnd)
		return 0;

	AAA* tK_TmpVal;
	
	if(lBegin + 1 == lEnd)
	{
		if(_CompareKey(tKey[lBegin], tKey[lEnd]) > 0)
		{
			tK_TmpVal = tKey[lBegin];
			tKey[lBegin] = tKey[lEnd];
			tKey[lEnd] = tK_TmpVal;
		}
		return 0;
	}
	
	long lMid = (lBegin + lEnd)>>1;
	long m = lBegin, n = lEnd;

	AAA* tK_MidVal = tKey[lMid];

	while(lBegin < lEnd)
	{
		while(lBegin < lEnd && _CompareKey(tKey[lBegin], tK_MidVal) < 0) lBegin++;
		while(lBegin < lEnd && _CompareKey(tKey[lEnd], tK_MidVal) > 0) lEnd--;
		if(lBegin < lEnd)
		{
			tK_TmpVal = tKey[lBegin];
			tKey[lBegin] = tKey[lEnd];
			tKey[lEnd] = tK_TmpVal;
			
			if(++lBegin < lEnd)
				lEnd--;
		}
	}

	if(_CompareKey(tKey[lBegin], tK_MidVal) < 0)
		lBegin++;

	if(lBegin > m)
		QuickSort(m, lBegin - 1, tKey, _CompareKey);
	if(lEnd < n)
		QuickSort(lEnd, n, tKey, _CompareKey);

	return 0;
};	

long QuickSort_TTT(long lBegin, long lEnd, AAA** tKey)
{
	if(lBegin >= lEnd)
		return 0;

	AAA* tK_TmpVal;
	
	if(lBegin + 1 == lEnd)
	{
		if(*tKey[lBegin] > *tKey[lEnd])
		{
			tK_TmpVal = tKey[lBegin];
			tKey[lBegin] = tKey[lEnd];
			tKey[lEnd] = tK_TmpVal;
		}
		return 0;
	}
	
	long lMid = (lBegin + lEnd)>>1;
	long m = lBegin, n = lEnd;

	AAA* tK_MidVal = tKey[lMid];

	while(lBegin < lEnd)
	{
		while(lBegin < lEnd && *tKey[lBegin] < *tK_MidVal) lBegin++;
		while(lBegin < lEnd && *tKey[lEnd] > *tK_MidVal) lEnd--;
		if(lBegin < lEnd)
		{
			tK_TmpVal = tKey[lBegin];
			tKey[lBegin] = tKey[lEnd];
			tKey[lEnd] = tK_TmpVal;
			
			if(++lBegin < lEnd)
				lEnd--;
		}
	}

	if(*tKey[lBegin] < *tK_MidVal)
		lBegin++;

	if(lBegin > m)
		QuickSort_TTT(m, lBegin - 1, tKey);
	if(lEnd < n)
		QuickSort_TTT(lEnd, n, tKey);

	return 0;
};	

long test_quicksort()
{
	srand(time(NULL));

	AAA* buf = new AAA[10000000];
	AAA** arr = new AAA*[10000000];
	for(long i = 0; i < 10000000; i++)
	{
		long j = 0;
		char* p = (char*)&j;
		*(unsigned short*)p = rand();
		*(unsigned short*)(p + 2) = rand();
		
		arr[i] = &buf[i];
		arr[i]->a = j;
	}

	LONGLONG tm1, tm2;

	printf("start...\n");

	UT_Arithmetic<long> ccc;
	tm1 = ccc.GetTimeCount();
	QuickSort_TTT(0, 10000000 - 1, arr);
//	QuickSort(0, 10000000 - 1, arr, Comp);
	tm2 = ccc.GetTimeCount();

	long tm = ccc.GetTimeResult(tm1, tm2);

	printf("%d ms\n", tm);

	return 0;
}

// 整理下面的排序算法，基数排序，分析适用情况
var_4 QuickSort(var_4 lBegin, var_4 lEnd, T_Key* tKey, unsigned var_4 k, var_4 (*_CompareKey)(T_Key, T_Key))
{
var_4 i = lBegin;
var_4 j = lEnd;

T_Key tK_TmpVal;

while(i < j)
{
while((tKey[j]&k) && (i < j)) j--;     // 自右端进行比较, 若tKey[j]&k值为1, 则j向左扫描
while((tKey[i]&k) == 0 && (i < j))i++; // 自左端进行比较，若tKey[i]&k值为0, 则i向右扫描

if(i < j) //交换
{       
tK_TmpVal = tKey[j];
tKey[j] = tKey[i];
tKey[i] = tK_TmpVal;
}
else // 将序列划为两个子序列
{
if(tKey[j]&k) 
i--;  
else 
j++;

break;
}
}

if(k > 1)
{
if(lBegin < i) QuickSort(tKey, lBegin, i, k/2);
if(j < lEnd) QuickSort(tKey, j, lEnd, k/2);
}

return 0;
}
*/

/************************************************************************/
//                                                                      
/************************************************************************/
/*
#include "complete/UC_CodeChange.h"
long test_code_change()
{
	char* src = "北京时间3月16日消息";
	long  len = strlen(src);

	char des[1024];
	char tmp[1024];

	UC_CodeChange cClass;

	cClass.GB2312ToUTF_8(src, len, des);
	cClass.UTF_8ToGB2312(des, strlen(des), tmp);

	return 0;
}
*/

/************************************************************************/
// or
/************************************************************************/
/*
typedef          int var_4;
typedef unsigned int var_u4;

var_4 GetResult(var_u4* tag_pagebuf[64], var_u4 tag_pagenum[64], var_4 tag_num, var_u4* res_buf, var_4& res_num)
{
	res_num = 0;

	var_4  cur_pos = 0;
	var_u4 cur_val = 0xFFFFFFFF;
	for(var_4 i = 0; i < tag_num; i++)
	{
		if(cur_val > *tag_pagebuf[i])
		{
			cur_val = *tag_pagebuf[i];
			cur_pos = i;
		}
	}

	for(;;)
	{
		var_u4 hit_id = *tag_pagebuf[cur_pos];

		res_buf[res_num++] = hit_id;
		tag_pagebuf[cur_pos]++;
		tag_pagenum[cur_pos]--;

		cur_pos = -1;
		cur_val = 0xFFFFFFFF;
		for(var_4 i = 0; i < tag_num; i++)
		{
			while(tag_pagenum[i] > 0 && *tag_pagebuf[i] == hit_id)
			{
				tag_pagebuf[i]++;
				tag_pagenum[i]--;
				continue;
			}
			if(tag_pagenum[i] <= 0)
				continue;

			if(cur_val > *tag_pagebuf[i])
			{
				cur_val = *tag_pagebuf[i];
				cur_pos = i;
			}
		}

		if(cur_pos < 0)
			break;
	}

	return 0;
}

#include "UT_Heap.h"

var_4 GetResult_Heap(var_u4* tag_pagebuf[64], var_u4 tag_pagenum[64], var_4 tag_num, var_u4* res_buf, var_4& res_num)
{
	res_num = 0;

	UT_Heap<var_u4> heap;
	heap.init_heap(tag_num);

	var_4  cur_pos = 0;
	var_u4 cur_val = 0xFFFFFFFF;
	for(var_4 i = 0; i < tag_num; i++)
	{
		if(cur_val > *tag_pagebuf[i])
		{
			cur_val = *tag_pagebuf[i];
			cur_pos = i;
		}
	}

	for(;;)
	{
		var_u4 hit_id = *tag_pagebuf[cur_pos];

		res_buf[res_num++] = hit_id;
		tag_pagebuf[cur_pos]++;
		tag_pagenum[cur_pos]--;

		cur_pos = -1;
		cur_val = 0xFFFFFFFF;
		for(var_4 i = 0; i < tag_num; i++)
		{
			while(tag_pagenum[i] > 0 && *tag_pagebuf[i] == hit_id)
			{
				tag_pagebuf[i]++;
				tag_pagenum[i]--;
				continue;
			}
			if(tag_pagenum[i] <= 0)
				continue;

			if(cur_val > *tag_pagebuf[i])
			{
				cur_val = *tag_pagebuf[i];
				cur_pos = i;
			}
		}

		if(cur_pos < 0)
			break;
	}

	return 0;
}

long test_or()
{
	var_u4* tag_pagebuf[64];
	var_u4  tag_pagenum[64];
	var_4   tag_num = 2;

	tag_pagebuf[0] = new var_u4[8];
	tag_pagebuf[1] = new var_u4[5];
	tag_pagenum[0] = 8;
	tag_pagenum[1] = 5;

	tag_pagebuf[0][0] = 1;
	tag_pagebuf[0][1] = 3;
	tag_pagebuf[0][2] = 5;
	tag_pagebuf[0][3] = 7;
	tag_pagebuf[0][4] = 8;
	tag_pagebuf[0][5] = 9;
	tag_pagebuf[0][6] = 10;
	tag_pagebuf[0][7] = 14;
	tag_pagebuf[1][0] = 1;
	tag_pagebuf[1][1] = 2;
	tag_pagebuf[1][2] = 3;
	tag_pagebuf[1][3] = 6;
	tag_pagebuf[1][4] = 9;

	var_u4* res_buf = new var_u4[1000];
	var_4   res_num = 0;

	GetResult(tag_pagebuf, tag_pagenum, tag_num, res_buf, res_num);
	for(var_4 i = 0; i < res_num; i++)
		printf("%d\n", res_buf[i]);

	return 0;
}
*/
/************************************************************************/
// union
/************************************************************************/
/*
union b
{
	char*  p1;
	long* p2;
};

int union()
{
	char* m1 = new char[1024];
	char* m2 = new char[1024];
	*(long*)m1 = (long)m2;

	m1 = (char*)(*(long*)m1);

	m1 = *(char**)m1;

	b* e = (b*)m1;
	m1 = e->p1;

	return 0;
}
*/
//////////////////////////////////////////////////////////////////////////
#include "./platform_cross/UH_Define.h"

#include "./platform_cross/UC_MD5.h"
#include "./platform_cross/UC_IOManager.h"
#include "./platform_cross/UC_ReadConfigFile.h"
#include "./platform_cross/UC_Allocator_Recycle.h"

#include "./platform_cross/UT_Heap.h"
#include "./platform_cross/UT_Queue.h"
#include "./platform_cross/UT_Allocator.h"
#include "./platform_cross/UT_Arithmetic.h"
#include "./platform_cross/UT_HashSearch.h"

//////////////////////////////////////////////////////////////////////////
#include "./platform_windows/UH_DetectMemoryLeak.h"

#include "./platform_windows/UC_Tools.h"
#include "./platform_windows/UC_CodeChange.h"
#include "./platform_windows/UC_Communication.h"

#include "./platform_windows/UT_Queue_Event.h"
#include "./platform_windows/UT_MemoryBlockAllocator.h"


int main(int argc, char* argv[])
{
	return 0;
}