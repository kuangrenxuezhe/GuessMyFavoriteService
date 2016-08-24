
#ifndef WF_COMPRESS_H
#define WF_COMPRESS_H

#include "./zlib/zlib.h"
#include "WFLock.h"

namespace nsWFCompress
{

class CZlib
{
public:
    CZlib()
    {
        m_ulMaxDataLen = 0;
        m_pTempBuf = NULL;
    }
    ~CZlib()
    {
        if (m_pTempBuf != NULL)
        {
            delete m_pTempBuf;
            m_pTempBuf = NULL;
        }
    }
public:
    int Init(unsigned long ulMaxDataLen)
    {
        if (m_pTempBuf != NULL)
        {
            delete m_pTempBuf;
            m_pTempBuf = NULL;
        }

        m_ulMaxDataLen = ulMaxDataLen;
        m_ulTempBufLen = (unsigned long)((m_ulMaxDataLen+12)*1.1+1024);
        m_pTempBuf = nsWFMem::New<unsigned char>(m_ulTempBufLen);
        if (m_pTempBuf == NULL)
        {
            return -10;
        }
        return 0;
    }
    int Compress(unsigned char *pZipBuf, unsigned long &ulZipLen,
        unsigned char *pSrcBuf, unsigned long ulSrcLen)
    {
        unsigned long ulTempLen = m_ulTempBufLen;

        nsWFLock::CAutolock autolock(&m_Lock);
        int iRet = compress(m_pTempBuf, &ulTempLen, pSrcBuf, ulSrcLen);
        if (iRet != Z_OK)
        {
            //printf("compress ERROR, ret=%d!\n", iRet);
            return iRet;
        }
        if (ulZipLen < ulTempLen)
        {
            return -100;
        }
        memcpy(pZipBuf, m_pTempBuf, ulTempLen);
        ulZipLen = ulTempLen;
        //printf("compress OK, ZipLen=%u\n", ulZipLen);
        return 0;
    }
    int UnCompress(unsigned char *pUnZipBuf, unsigned long &ulUnZipLen,
        const unsigned char *pZipBuf, unsigned long ulZipLen)
    {
        unsigned long ulTempLen = m_ulTempBufLen;

        nsWFLock::CAutolock autolock(&m_Lock);

        int iRet = uncompress(m_pTempBuf, &ulTempLen, pZipBuf, ulZipLen);
        if (iRet != Z_OK)
        {
            //printf("uncompress ERROR, ret=%d!\n", iRet);
            return iRet;
        }
        if (ulUnZipLen < ulTempLen)
        {
            return -100;
        }
        memcpy(pUnZipBuf, m_pTempBuf, ulTempLen);
        ulUnZipLen = ulTempLen;
        //printf("uncompress OK, UnZipLen=%u\n", ulUnZipLen);
        return 0;
    }

protected:
private:
    nsWFLock::CLock m_Lock;
    unsigned long m_ulMaxDataLen;
    unsigned long m_ulTempBufLen;
    unsigned char *m_pTempBuf;
};


}   // end of namespace

#endif  // end of #ifndef
