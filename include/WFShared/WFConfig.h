
#ifndef WF_CONFIG_H
#define WF_CONFIG_H

#include <stdio.h>
#include <stdlib.h>

namespace nsWFConfig
{

//////////////////////////////////////////////////////////////////////////
//��ȡ�����ļ�����
//������ϢӦ�����¸�ʽ��֯
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
    //FUNC  ��ʼ��
    //PARAM pCfgFile�������ļ���
    //      bCaseSensitive���Ƿ��Сд����
    //RET   0����ʼ���ɹ�
    //      ��������ʼ��ʧ�ܣ����ش�����
    int Init(const char *pCfgFile, bool bCaseSensitive = false)
    {
        if (pCfgFile == NULL)
        {
            return -1;
        }

        //�������ļ�
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
    //FUNC  ��ʼ��
    //PARAM pCfgFile�������ļ���
    //      bCaseSensitive���Ƿ��Сд����
    //RET   0����ʼ���ɹ�
    //      ��������ʼ��ʧ�ܣ����ش�����
    int Init(const char *pCfgBuf, int32_t iCfgBufLen, bool bCaseSensitive = false)
    {
        if (pCfgBuf == NULL || iCfgBufLen <= 0)
        {
            return -1;
        }

        //�������ļ�
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
    //FUNC  ��ȡ����ʽ��������Ϣ
    //PARAM pSetName�����������ڼ�������
    //      pCfgName������������
    //      pDefaultRet��Ĭ�Ϸ���ֵ
    //RET   pDefaultRet����ȡʧ��
    //      ��������ȡ�ɹ�������������Ϣ�����׵�ַ
    //NOTE  ������Ϣ���ڹ����ڴ��У��ܿ�ͻ�ʧЧ(�ڵ�����һ��GetXXX()ʱ�ͻ�ʧЧ)
    const char *GetStr(const char *pSetName, const char *pCfgName, const char *pDefaultRet = NULL)
    {
        int iRet = GetStr(pSetName, pCfgName, m_DefaultLine, DEFAULT_LINE_LEN);
        if (iRet <= 0)
        {
            return pDefaultRet;
        }

        return m_DefaultLine;
    }
    //FUNC ��ȡ����ʽ��������Ϣ
    //PARAM pSetName�����������ڼ�������
    //      pCfgName������������
    //      pRetBuf�����������Ϣ��Buf
    //      iMaxRetBufLen��������ϢBuf����
    //RET   >=0����ȡ�ɹ�������������Ϣ������
    //      <0����ȡʧ�ܣ����ش�����
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
            {//������һ��[SetName]
                char *pSetBeg = szLine+1;
                char *pSetEnd = strchr(szLine, ']');
                if (pSetEnd != NULL)
                {
                    *pSetEnd = '\0';
                    if (m_bCaseSensitive)
                    {//��Сд����
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
                {//�Ѿ������˸�����Set�����������
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
                {//��δ�ҵ�SetName
                    continue;
                }

                //�ǵ�ǰSet�ڵ�������Ϣ
                char *pCfgBeg = szLine;
                char *pCfgEnd = strchr(pCfgBeg, '=');
                if (pCfgEnd == NULL)
                {//������Ч��Cfg��
                    continue;
                }

                *pCfgEnd = '\0';
                if (m_bCaseSensitive)
                {
                    if (strcmp(szLine, pCfgName) != 0)
                    {//����Ҫ�ҵ�Cfg��
                        continue;
                    }
                }
                else
                {
                    if (nsWFPub::StriCmp(szLine, pCfgName) != 0)
                    {//����Ҫ�ҵ�Cfg��
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

                //�������ô�
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
    //FUNC  ��ȡ������ʽ��������Ϣ
    //PARAM pSetName�����������ڼ�������
    //      pCfgName������������
    //      iDefaultRet��Ĭ�Ϸ���ֵ
    //RET   iDefaultRet����ȡʧ��
    //      ��������ȡ�ɹ�������������Ϣ��ֵ
    int GetInt(const char *pSetName, const char *pCfgName, int iDefaultRet = -1234)
    {
        int iRet = GetStr(pSetName, pCfgName, m_DefaultLine, DEFAULT_LINE_LEN);
        if (iRet <= 0)
        {
            return iDefaultRet;
        }

        return atoi(m_DefaultLine);
    }
    //FUNC  ��ȡ˫���ȸ���������ʽ��������Ϣ
    //PARAM pSetName�����������ڼ�������
    //      pCfgName������������
    //      dDefaultRet��Ĭ�Ϸ���ֵ
    //RET   dDefaultRet����ȡʧ��
    //      ��������ȡ�ɹ�������������Ϣ��ֵ
    double GetDouble(const char *pSetName, const char *pCfgName, double dDefaultRet = -0.1234)
    {
        int iRet = GetStr(pSetName, pCfgName, m_DefaultLine, DEFAULT_LINE_LEN);
        if (iRet <= 0)
        {
            return dDefaultRet;
        }

        return atof(m_DefaultLine);
    }
    //FUNC  ��ȡ�����ȸ�������ʽ��������Ϣ
    //PARAM pSetName�����������ڼ�������
    //      pCfgName������������
    //      fDefaultRet��Ĭ�Ϸ���ֵ
    //RET   fDefaultRet����ȡʧ��
    //      ��������ȡ�ɹ�������������Ϣ��ֵ
    float GetFloat(const char *pSetName, const char *pCfgName, float fDefaultRet = -0.1234)
    {
        return (float)GetDouble(pSetName, pCfgName, fDefaultRet);
    }
    //FUNC  ��ȡ�����͵�������Ϣ
    //PARAM pSetName�����������ڼ�������
    //      pCfgName������������
    //      fDefaultRet��Ĭ�Ϸ���ֵ
    //RET   bDefaultRet����ȡʧ��
    //      ��������ȡ�ɹ�������������Ϣ��ֵ
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
    char *m_pCfgBuf;                        //��������
    int  m_iOffset;                         //��ǰ��������λ��
    char m_DefaultLine[DEFAULT_LINE_LEN+4]; //Ĭ���У����ڴ��������Ϣ��
    bool m_bCaseSensitive;                  //�Ƿ��Сд����
};

}   // end of namespace

#endif  // end of #ifndef
