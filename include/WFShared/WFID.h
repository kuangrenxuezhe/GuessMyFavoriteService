
#ifndef WF_ID_H
#define WF_ID_H

#include "./md5/md5int.h"
#include "WFInt.h"

#include <stdio.h>
#include <string.h>

namespace nsWFID
{

const int INVALID_ID = 0;
const int MAX_WORD_LEN = 1<<10;
const int MAX_URL_LEN = 2<<10;


//FUNC  �õ�����MD5
static uint64_t GetMD5(const char *pStr, int iStrlen)
{
    if (pStr == NULL || iStrlen <= 0)
    {
        return INVALID_ID;
    }
    uint64_t u64MD5 = INVALID_ID;
    MD5Segment(pStr, iStrlen, (unsigned char*)&u64MD5, 8);
    return u64MD5;
}

//FUNC  �õ�����MD5
static uint64_t GetMD5(const char *pStr)
{
    if (pStr == NULL)
    {
        return INVALID_ID;
    }
    return GetMD5(pStr, strlen(pStr));
}

//FUNC  �õ���ID�������ִ�Сд
static uint64_t GetWordID(const char *pWord, int iWordLen, char *pWordTmp)
{
	if (pWord == NULL || iWordLen <= 0)
	{
		return INVALID_ID;
	}

    const char *pWordEnd = pWord+iWordLen;
    const char *pSrc = pWord;
    char *pDest = pWordTmp;
    while (pSrc < pWordEnd)
    {
        if (*pSrc < 0)
        {//�Ǻ���
            *pDest++ = *pSrc++;
            *pDest++ = *pSrc++;
        }
        else
        {//���ַ�
            if ('A' <= *pSrc && *pSrc <= 'Z')
            {//��дתСд
                *pDest++ = *pSrc++ - 'A' + 'a';
            }
            else
            {
                *pDest++ = *pSrc++;
            }
        }
    }
    return GetMD5(pWordTmp, iWordLen);
}

//FUNC  �õ���ID�������ִ�Сд
static uint64_t GetWordID(const char *pWord, int iWordLen)
{
	if (pWord == NULL || iWordLen <= 0 || iWordLen > MAX_WORD_LEN)
	{
		return INVALID_ID;
	}

    char szWord[MAX_WORD_LEN+4] = {0};
    return GetWordID(pWord, iWordLen, szWord);
}

//FUNC  �õ���ID�������ִ�Сд
static uint64_t GetWordID(const char *pWord)
{
	if (pWord == NULL)
	{
		return INVALID_ID;
	}
    return GetWordID(pWord, strlen(pWord));
}

static uint64_t GetDomainID(const char *pUrl)
{
    if (pUrl == NULL)
    {
        return INVALID_ID;
    }

    //copy domain
    char szUrlCopy[MAX_URL_LEN+16] = "";

    const char *p = strstr(pUrl, "://");
    if (p != NULL)
    {
        p += 3;
    }
    else
    {
        p = pUrl;
        while (*p == ' ')
        {
            ++p;
        }
    }
 
    char *q = szUrlCopy;
    while (*p != '\0' && *p != '/')
    {
        *q++ = *p++;
    }

    *q = '\0';

    //get ID
    return nsWFID::GetWordID(szUrlCopy, q-szUrlCopy);
}

}   // end of namespace

#endif  // end of #ifndef


