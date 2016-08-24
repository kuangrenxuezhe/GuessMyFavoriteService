
#ifndef WF_CONFIG_H
#define WF_CONFIG_H

#include <stdio.h>
#include <stdlib.h>

namespace nsWFConfig
{

//////////////////////////////////////////////////////////////////////////
//读取配置文件的类
//配置信息应按以下格式组织
//
//[SetName]
//CfgName=Value
//CfgName=Value
//...
class CCfgReader
{
    enum
    {
        DEFAULT_LINE_LEN = 1024,
    };
public:
    CCfgReader()
    {
        m_pCfgBuf = NULL;
        m_iOffset = 0;
        memset(m_DefaultLine, 0, sizeof(m_DefaultLine));
        m_bCaseSensitive = false;
    }
    ~CCfgReader()
    {
        if (m_pCfgBuf != NULL)
        {
            delete []m_pCfgBuf;
        }
    }
    //FUNC  初始化
    //PARAM pCfgFile：配置文件名
    //      bCaseSensitive：是否大小写敏感
    //RET   0：初始化成功
    //      其他：初始化失败，返回错误码
    int Init(const char *pCfgFile, bool bCaseSensitive = false)
    {
        if (pCfgFile == NULL)
        {
            return -1;
        }

        //打开配置文件
        FILE *fpRB = fopen(pCfgFile, "rb");
        if (fpRB == NULL)
        {
            return -10;
        }

        fseek(fpRB, 0, SEEK_END);
        int iFileSize = ftell(fpRB);
        rewind(fpRB);

        int iBufSize = ((iFileSize+2)/1024)*1024+1024;  //align to KB
        if (m_pCfgBuf != NULL)
        {
            delete []m_pCfgBuf;
        }
        m_pCfgBuf = new char[iBufSize];
        if (m_pCfgBuf == NULL)
        {
            fclose(fpRB);
            return -20;
        }
        int iReadLen = fread(m_pCfgBuf, iFileSize, 1, fpRB);
        fclose(fpRB);
        if (iReadLen != 1)
        {
            return -30;
        }
        m_pCfgBuf[iFileSize] = m_pCfgBuf[iFileSize+1] = '\0';
        m_iOffset = 0;

        m_bCaseSensitive = bCaseSensitive;
        return 0;
    }
    //FUNC  初始化
    //PARAM pCfgFile：配置文件名
    //      bCaseSensitive：是否大小写敏感
    //RET   0：初始化成功
    //      其他：初始化失败，返回错误码
    int Init(const char *pCfgBuf, int32_t iCfgBufLen, bool bCaseSensitive = false)
    {
        if (pCfgBuf == NULL || iCfgBufLen <= 0)
        {
            return -1;
        }

        //打开配置文件
        int iBufSize = ((iCfgBufLen+2)/1024)*1024+1024;  //align to KB
        if (m_pCfgBuf != NULL)
        {
            delete []m_pCfgBuf;
        }
        m_pCfgBuf = new char[iBufSize];
        if (m_pCfgBuf == NULL)
        {
            return -20;
        }
        memcpy(m_pCfgBuf, pCfgBuf, iCfgBufLen);
        m_pCfgBuf[iCfgBufLen] = m_pCfgBuf[iCfgBufLen+1] = '\0';
        m_iOffset = 0;

        m_bCaseSensitive = bCaseSensitive;
        return 0;
    }
    //FUNC  获取串形式的配置信息
    //PARAM pSetName：配置项所在集合名称
    //      pCfgName：配置项名称
    //      pDefaultRet：默认返回值
    //RET   pDefaultRet：获取失败
    //      其他：获取成功，返回配置信息串的首地址
    //NOTE  配置信息存在公共内存中，很快就会失效(在调用下一个GetXXX()时就会失效)
    const char *GetStr(const char *pSetName, const char *pCfgName, const char *pDefaultRet = NULL)
    {
        int iRet = GetStr(pSetName, pCfgName, m_DefaultLine, DEFAULT_LINE_LEN);
        if (iRet <= 0)
        {
            return pDefaultRet;
        }

        return m_DefaultLine;
    }
    //FUNC 获取串形式的配置信息
    //PARAM pSetName：配置项所在集合名称
    //      pCfgName：配置项名称
    //      pRetBuf：存放配置信息的Buf
    //      iMaxRetBufLen：最大的信息Buf长度
    //RET   >=0：获取成功，返回配置信息串长度
    //      <0：获取失败，返回错误码
    int GetStr(const char *pSetName, const char *pCfgName, char *pRetBuf, int iMaxRetBufLen)
    {
        if (m_pCfgBuf == NULL)
        {
            return -10;
        }
        
        if (pSetName == NULL)
        { 
            return -20;
        }
        
        if (pCfgName == NULL)
        { 
            return -30;
        }
        
        m_iOffset = 0;
        bool bFindSet = false;
        char szLine[DEFAULT_LINE_LEN+4] = "";
        while (GetLine(szLine, DEFAULT_LINE_LEN) > 0)
        {
            if (szLine[0] == '[')
            {//本行是一个[SetName]
                char *pSetBeg = szLine+1;
                char *pSetEnd = strchr(szLine, ']');
                if (pSetEnd != NULL)
                {
                    *pSetEnd = '\0';
                    if (m_bCaseSensitive)
                    {//大小写敏感
                        if (strcmp(pSetName, pSetBeg) == 0)
                        {
                            bFindSet = true;
                            continue;
                        }
                    }
                    else
                    {
                        if (nsWFPub::StriCmp(pSetName, pSetBeg) == 0)
                        {
                            bFindSet = true;
                            continue;
                        }
                    }                  
                }

                if (bFindSet)
                {//已经跳出了给定的Set，则结束查找
                    return -100;
                }
                else
                {
                    continue;
                }                    
            }
            else
            {
                if (!bFindSet)
                {//还未找到SetName
                    continue;
                }

                //是当前Set内的配置信息
                char *pCfgBeg = szLine;
                char *pCfgEnd = strchr(pCfgBeg, '=');
                if (pCfgEnd == NULL)
                {//不是有效的Cfg行
                    continue;
                }

                *pCfgEnd = '\0';
                if (m_bCaseSensitive)
                {
                    if (strcmp(szLine, pCfgName) != 0)
                    {//不是要找的Cfg行
                        continue;
                    }
                }
                else
                {
                    if (nsWFPub::StriCmp(szLine, pCfgName) != 0)
                    {//不是要找的Cfg行
                        continue;
                    }
                }

                pCfgBeg = pCfgEnd+1;
                pCfgEnd = strchr(pCfgBeg, '\r');
                if (pCfgEnd != NULL)
                {
                    *pCfgEnd = '\0';
                }
                pCfgEnd = strchr(pCfgBeg, '\n');
                if (pCfgEnd != NULL)
                {
                    *pCfgEnd = '\0';
                }
                if (pCfgEnd == NULL)
                {//Last line
                    pCfgEnd = pCfgBeg+strlen(pCfgBeg);
                }

                //复制配置串
                int iCfgLen = pCfgEnd-pCfgBeg;
                if (iCfgLen >= iMaxRetBufLen)
                {
                    return -200;
                }
                strcpy(pRetBuf, pCfgBeg);
                return iCfgLen;
            }
        }

        return -300;
    }
    //FUNC  获取整数形式的配置信息
    //PARAM pSetName：配置项所在集合名称
    //      pCfgName：配置项名称
    //      iDefaultRet：默认返回值
    //RET   iDefaultRet：获取失败
    //      其他：获取成功，返回配置信息的值
    int GetInt(const char *pSetName, const char *pCfgName, int iDefaultRet = -1234)
    {
        int iRet = GetStr(pSetName, pCfgName, m_DefaultLine, DEFAULT_LINE_LEN);
        if (iRet <= 0)
        {
            return iDefaultRet;
        }

        return atoi(m_DefaultLine);
    }
    //FUNC  获取双精度浮点数数形式的配置信息
    //PARAM pSetName：配置项所在集合名称
    //      pCfgName：配置项名称
    //      dDefaultRet：默认返回值
    //RET   dDefaultRet：获取失败
    //      其他：获取成功，返回配置信息的值
    double GetDouble(const char *pSetName, const char *pCfgName, double dDefaultRet = -0.1234)
    {
        int iRet = GetStr(pSetName, pCfgName, m_DefaultLine, DEFAULT_LINE_LEN);
        if (iRet <= 0)
        {
            return dDefaultRet;
        }

        return atof(m_DefaultLine);
    }
    //FUNC  获取单精度浮点数形式的配置信息
    //PARAM pSetName：配置项所在集合名称
    //      pCfgName：配置项名称
    //      fDefaultRet：默认返回值
    //RET   fDefaultRet：获取失败
    //      其他：获取成功，返回配置信息的值
    float GetFloat(const char *pSetName, const char *pCfgName, float fDefaultRet = -0.1234)
    {
        return (float)GetDouble(pSetName, pCfgName, fDefaultRet);
    }
    //FUNC  获取布尔型的配置信息
    //PARAM pSetName：配置项所在集合名称
    //      pCfgName：配置项名称
    //      fDefaultRet：默认返回值
    //RET   bDefaultRet：获取失败
    //      其他：获取成功，返回配置信息的值
    bool GetBool(const char *pSetName, const char *pCfgName, bool bDefaultRet = false)
    {
        int iRet = GetInt(pSetName, pCfgName, -1234);
        if (iRet == -1234)
        {
            return bDefaultRet;
        }
        else
        {
            return (iRet != false);
        }
    }    
protected:
    int GetLine(char *pLine, int iMaxLineLen)
    {
        if (pLine == NULL || iMaxLineLen <= 0)
        {
            return -1;
        }

        char *p = &m_pCfgBuf[m_iOffset];
        char *q = pLine;
        while (*p != '\0')
        {
            if (q-pLine >= iMaxLineLen)
            {
                return -10;
            }
            *q++ = *p++;
            ++m_iOffset;
            if (*(q-1) == '\r' || *(q-1) == '\n')
            {//a line finished
                while (*p == '\r' || *p == '\n')
                {//jump over 
                    p++;
                    ++m_iOffset;
                }
                break;
            }
        }
        *q = '\0';

        return q-pLine;
    }
private:
    char *m_pCfgBuf;                        //配置内容
    int  m_iOffset;                         //当前遍历到的位置
    char m_DefaultLine[DEFAULT_LINE_LEN+4]; //默认行，用于存放配置信息行
    bool m_bCaseSensitive;                  //是否大小写敏感
};

}   // end of namespace

#endif  // end of #ifndef
