#ifndef MODEL_WORKER_H
#define MODEL_WORKER_H
#include <string>
#include <rdkafkacpp.h>
#include <KafkaEx.hpp>
#include <KafkaWriteEx.hpp>
#include "Stream.h"
#include <UH_Define.h>
#include <errno.h>
#include "../json/json.h"
#include "WFLog.h"
#include "../WFShared/WFHash.h"
#include "tinyxml.h"
#include <sys/types.h>
#include <regex.h>
using namespace std;
extern Config *g_config;
extern CDailyLog *g_log;
extern int g_showScreen;

#ifdef _WIN32_ENV_
#define API_MKDIR(x)    mkdir(x)
#define API_ACCESS(x)   _access(x, 0)
#else
#define API_MKDIR(x)    mkdir(x, S_IRWXU)
#define API_ACCESS(x)   access(x, F_OK)
#endif
#define _PRINTF_ 1
#define IS_CHAR(ch) ((ch) >= 'a' && (ch) <= 'z')
#define IS_CHAR_X(ch) (((ch) >= 'a' && (ch) <= 'z') || ((ch) >= 'A' && (ch) <= 'Z'))
#define IS_DIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define IS_HEX(x) (IS_CHAR_X(x) || IS_DIGIT(x))
#define HEX_VAL(x) (IS_DIGIT(x) ? ((x)-'0') : (IS_CHAR(x) ? ((x)-'a'+10) : ((x)-'A'+10)))

int UrlUnescape(char *buffer, long length, char *rchar)
{
    long i;
    char *pos = rchar;
    long rlength = 0;
    for(i=0; i<length; i++)
    {
        if(buffer[i] == '%' && i+5 < length && buffer[i+1] == 'u')
        {
            *(int8_t *)pos = (int8_t)((HEX_VAL(buffer[i+2]) << 4) | HEX_VAL(buffer[i+3]));
            pos += sizeof(int8_t);
            *(int8_t *)pos = (int8_t)((HEX_VAL(buffer[i+4]) << 4) | HEX_VAL(buffer[i+5]));
            pos += sizeof(int8_t);
            i += 5;
        }
        else if(buffer[i] == '%' && i+2 < length && IS_HEX(buffer[i+1]) && IS_HEX(buffer[i+2]))
        {
            *(int8_t *)pos = (int8_t)((HEX_VAL(buffer[i+1]) << 4) | HEX_VAL(buffer[i+2]));
            pos += sizeof(int8_t);
            i += 2;
        }
        else if(buffer[i] == '+')
        {
            *(int8_t *)pos = ' ';
            pos += sizeof(int8_t);
        }
        else
        {
            *(int8_t *)pos = buffer[i];
            pos += sizeof(int8_t);
        }
    }
    return pos - rchar;
}

class CUsers
{
    public:
        struct SUserSrpInfo{wo
            const static int SRP_ID_MAX_LEN             = 36;
            const static int SRP_WORD_MAX_LEN           = 64;
            int         UserID;
            int         Op;
            char        SRPID[SRP_ID_MAX_LEN];
            char        SRPWord[SRP_WORD_MAX_LEN];
            char        SRPWordGbk[SRP_WORD_MAX_LEN];
            long        SubID;
            SUserSrpInfo  *pNext;
            SUserSrpInfo()
            {
                memset(this, 0, sizeof(SUserSrpInfo));
            }
        };
        CUsers()
        {
            m_pUserSrpInfoList = NULL;
            m_pSRPInfoMem = NULL;
        }
        ~CUsers()
        {
            if (m_pUserSrpInfoList != NULL)
            {
                delete m_pUserSrpInfoList;
            }
            if (m_pSRPInfoMem != NULL)
            {
                delete m_pSRPInfoMem;
            }
        }
        int Init(const char *host, int port, const char *pLogPath = "./log", const char *pLogPre = "log")
        {
            //Value = SUserSrpInfo
            m_pUserSrpInfoList = new nsWFHash::CHashTable<uint64_t>;
            if (m_pUserSrpInfoList == NULL)
            {
                return -40;
            }
            int iRet = m_pUserSrpInfoList->Init(8<<20, sizeof(SUserSrpInfo), 1<<20);
            if (iRet != 0)
            {
                return -41;
            }
            m_pSRPInfoMem = new nsWFMem::CBlockMemory;
            if (m_pSRPInfoMem == NULL)
            {
                return -80;
            }
            iRet = m_pSRPInfoMem->Init(8<<20, sizeof(SUserSrpInfo));
            if (iRet != 0)
            {
                return -81;
            }

            assert(strlen(host) < HOST_NAME_LEN);
            strcpy(m_host, host);
            m_port = port;

            if (API_ACCESS(pLogPath))
            {
                API_MKDIR(pLogPath);
            }
            if (0 != m_log.Init(pLogPath, pLogPre))
            {
                return -1;
            }
        }

        SUserSrpInfo *QueryUser(int uid)
        {
            void *pOldData = NULL;
            int iRet = m_pUserSrpInfoList->Search(uid, pOldData);
            if (iRet != 0)
            {//no this KeywordID
                return 0;
            }
            else
            {
                assert(pOldData != NULL);
                return  *(SUserSrpInfo**)pOldData;
            }
        }

        int AddUserToList(SUserSrpInfo *pNewSRPInfo, bool bSaveInc = true)
        {
            int ret = -1;
            void *pOldData = NULL;
            int iRet = m_pUserSrpInfoList->Search(pNewSRPInfo->UserID, pOldData);
            if (iRet != 0)
            {//no this KeywordID
                iRet = m_pUserSrpInfoList->Add(pNewSRPInfo->UserID, &pNewSRPInfo, pOldData);
                if (iRet != 0)
                {
                    return -10;
                }
            }
            else
            {
                assert(pOldData != NULL);
                SUserSrpInfo *pCurrSRPInfo = *(SUserSrpInfo**)pOldData;
                assert(pCurrSRPInfo != NULL);
                SUserSrpInfo *pPrevSRPInfo = NULL;
                while (pCurrSRPInfo != NULL)
                {
                    if (pNewSRPInfo->SubID > pCurrSRPInfo->SubID)
                    {
                        pPrevSRPInfo = pCurrSRPInfo;
                        pCurrSRPInfo = pPrevSRPInfo->pNext;
                    }
                    else
                    {
                        break;
                    }
                }
                if (pPrevSRPInfo != NULL)
                {
                    assert(pPrevSRPInfo->SubID < pNewSRPInfo->SubID);
                    pNewSRPInfo->pNext = pPrevSRPInfo->pNext;
                    pPrevSRPInfo->pNext = pNewSRPInfo;
                }
                else
                {
                    assert(pCurrSRPInfo != NULL);
                    pNewSRPInfo->pNext = pCurrSRPInfo;
                    iRet = m_pUserSrpInfoList->Update(pNewSRPInfo->UserID, &pNewSRPInfo);
                    if (iRet != 0)
                    {
                        return -20;
                    }
                }
            }
            return 0;
        }

        int ParseUserInfo(SUserSrpInfo *srpInfo, char *buffer, int length)
        {
            if(buffer == 0)
                return -33;
            string sBuffer = buffer;
            Json::Reader reader;
            Json::Value jsRoot;
            if (!reader.parse(sBuffer, jsRoot))
            {
                return -34;     //解析失败      
            }
            Json::Value value = jsRoot["sid"];
            if (!value.empty() && value.isString()) 
                snprintf(srpInfo->SRPID, SUserSrpInfo::SRP_ID_MAX_LEN, "%s", value.asCString());
            else
                return -35;     //srpid 不存在
            value = jsRoot["sword"];
            char desBuff[SUserSrpInfo::SRP_WORD_MAX_LEN] = {0}; 
            if (!value.empty() && value.isString()) 
            {
                snprintf(srpInfo->SRPWord, SUserSrpInfo::SRP_WORD_MAX_LEN, "%s", value.asCString());
                //    code_convert("utf-8","gb2312//IGNORE", srpInfo->SRPWord, strlen(srpInfo->SRPWord), \
                srpInfo->SRPWordGbk, SUserSrpInfo::SRP_WORD_MAX_LEN);  
            }
            else
            {
                return -36;     //srp word 不存在        
            }

            value = jsRoot["uid"];
            if (!value.empty() && value.isInt()) 
                srpInfo->UserID = value.asInt();
            else
                return -37; //非法用户id

            value = jsRoot["op"];
            if (!value.empty() && value.isString()) 
                srpInfo->Op = atoi(value.asCString());
            value = jsRoot["subId"];
            if (!value.empty() && value.isInt()) 
                srpInfo->SubID = value.asInt();
            return 0;
        }

        int SendStream(char *pDestStr, SUserSrpInfo *pSRPInfo)
        {
            char *pSrcStr =pDestStr;    
            CStream::SetBytes(pSrcStr, "KEY_EXTR", 8);  //PROTOCOL
            CStream::SetWord(pSrcStr, 9);               //OptType
            CStream::SetWord(pSrcStr, 1);               //DictType

            char *pCountStr = pSrcStr;  
            CStream::SetDWord(pSrcStr, 0);              //LEN
            CStream::SetDWord(pSrcStr, 0);              //max_relation  
            CStream::SetDWord(pSrcStr, 0);              //srpcount
            int counter = 0;
            int prevSubID = -1;
            while(pSRPInfo != NULL)
            {
                if(prevSubID != pSRPInfo->SubID && pSRPInfo->Op == 1)
                {
                    CStream::SetStr(pSrcStr, pSRPInfo->SRPID);          //SRPID
                    CStream::SetStr(pSrcStr, pSRPInfo->SRPWordGbk);     //SRPWORD
                    counter ++;
                }
                prevSubID = pSRPInfo->SubID;
                pSRPInfo = pSRPInfo->pNext;
            }

            int length = pSrcStr - pDestStr - 16;
            CStream::SetDWord(pCountStr, length);         //LEN
            CStream::SetDWord(pCountStr, 45);             //max_relation  
            CStream::SetDWord(pCountStr, counter);        //srpcount

            return pSrcStr - pDestStr;
        }

        int ParseResult(char *buffer, int length)
        {
            if(buffer == 0)
                return -33;
            string sBuffer = buffer;
            Json::Reader reader;
            Json::Value jsRoot;
            if (!reader.parse(sBuffer, jsRoot))
            {
                return -34;   //解析失败        
            }
            Json::Value DestItem, result;
            Json::Value infos = jsRoot["res"]["info"];
            for(int i=0;i<infos.size();i++)
            {
                Json::Value relation=  infos[i]["relation"];
                Json::Value srpid=  infos[i]["srpid"];
                Json::Value srpword=  infos[i]["srpword"];

                for(int j=0;j<relation.size();j++)
                {
                    DestItem["relaid"] =  relation[j]["relaid"];
                    DestItem["relaword"] =  relation[j]["relaword"];

                    result.append(DestItem);
                }
            }

            printf("result:%s\n", result.toStyledString().c_str());
            return 0;
        }

        SUserSrpInfo *AllocSRPInfo()
        {
            return (SUserSrpInfo *)m_pSRPInfoMem->Alloc();
        }

        void FreeSRPInfo(SUserSrpInfo *pSRPInfo)
        {
            if (pSRPInfo != NULL)
            {
                m_pSRPInfoMem->Free(pSRPInfo);
            }
        }

    private:
        const static int _OVERTIME_ = 10000;

        nsWFHash::CHashTable<uint64_t> *m_pUserSrpInfoList;    
        nsWFMem::CBlockMemory   *m_pSRPInfoMem;
        nsWFLog::CDailyLog      m_log;

        const static var_4 HOST_NAME_LEN = 20;
        char m_host[HOST_NAME_LEN];
        int m_port;
};

extern CUsers *g_users;
var_4 Communicate(char *requestBuf, int requestLen, char *responseBuf, int iMaxOutLen)
{
    CP_SOCKET_T sClient;
    int ret = cp_connect_socket(sClient, "103.29.134.217", 10006);
    if(0 != ret)                                //连接server
    {
        printf("connect error! ip=%s, port=%d\n", "103.29.134.217", 10006);
        return -1;
    }
    ret = cp_set_overtime(sClient, 10000);
    if(0 != ret)
    {
        cp_close_socket(sClient);
        printf("set overtime over! ip=%s, port=%d\n", "103.29.134.217", 10006);
        return -2;
    }
    ret = cp_sendbuf(sClient, requestBuf, requestLen);//发送查询串
    if(0 != ret) 
    {
        cp_close_socket(sClient);
        printf("send request error! ip=%s, port=%d\n", "103.29.134.217", 10006);
        return -3;
    }
    char status[8 + 4] = "\0";
    ret = cp_recvbuf(sClient, (char*)status, 4);//查询结果标识
    if(0 != ret) 
    {
        cp_close_socket(sClient);
        printf("recv length error! ip=%s, port=%d\n", "103.29.134.217", 10006);
        return -4;
    }
    int cmd_code = *(int *)status;
    if(cmd_code <= 0 || cmd_code >= iMaxOutLen)
    {
        cp_close_socket(sClient);
        printf("length error! length=%d\n", cmd_code);
        return -5;
    }
    ret = cp_recvbuf(sClient, responseBuf, cmd_code);
    if(0 != ret)
    {
        cp_close_socket(sClient);
        printf("recv result error! length=%d\n", cmd_code);
        return -6;
    }
    cp_close_socket(sClient);
    responseBuf[cmd_code] = 0;
    printf("responseBuf=%s\n", responseBuf);
    return cmd_code;
}

int load_kafka_userInfo_process(char *buffer, int length)
{
    if(length <= 0)
    {
        g_log->LPrintf(g_showScreen, "load_kafka_userInfo_process buffer is empty!!!\n");
        return 0;
    }
    CUsers::SUserSrpInfo *pNewSRPInfo = g_users->AllocSRPInfo();
    if(pNewSRPInfo == 0)
    {
        g_log->LPrintf(g_showScreen,"Alloc SRPInfo fail!!!\n");
        return -10;
    }

    int ret = -1;
    if((ret = g_users->ParseUserInfo(pNewSRPInfo, buffer, length))< 0)
    {
        g_log->LPrintf(g_showScreen,"ParseUserInfo fail return code %d\n", ret);
        g_users->FreeSRPInfo(pNewSRPInfo);
        return ret;
    }
    if((ret = g_users->AddUserToList(pNewSRPInfo))< 0)
    {
        g_log->LPrintf(g_showScreen,"AddUserToList fail return code %d\n", ret);
        g_users->FreeSRPInfo(pNewSRPInfo);
        return ret;
    }

    return 0;
}

int query()
{
    char *requestBuf = (char *)new char[1<<15];
    char *responseBuf = (char *)new char[1<<17];

    int length = g_users->SendStream(requestBuf, g_users->QueryUser(36));
    length = Communicate(requestBuf, length, responseBuf, 1<<17);
    if(length > 0)
    {
        g_users->ParseResult(responseBuf, length);
    }
    delete [] requestBuf;
    delete [] responseBuf;

    return 0;
}

static char* substr(const char*str, unsigned start, unsigned end)
{
    unsigned n = end - start;
    static char stbuf[256]= {0};
    static char srbuf[256]= {0};
    strncpy(stbuf, str + start, n);
    UrlUnescape(stbuf, n, srbuf);
    return srbuf;
}

int regex_txt(char *pattern, char *buffer)
{
    regex_t reg;
    char ebuf[128];
    regmatch_t pm[10];
    const size_t nmatch = 10;
    int z = regcomp(&reg, pattern, REG_EXTENDED);
    if (z != 0){
        regerror(z, &reg, ebuf, sizeof(ebuf));
        fprintf(stderr, "%s: pattern '%s' \n", ebuf, pattern);
        return 1;
    }
    z = regexec(&reg, buffer, nmatch, pm, 0);
    if (z != 0) {
        regerror(z, &reg, ebuf, sizeof(ebuf));
        fprintf(stderr, "%s: regcom('%s')\n", ebuf, buffer);
        return 2;
    }
    /* 输出处理结果 */
    for (int x = 0; x < nmatch && pm[x].rm_so != -1; ++ x) {
        printf(" $%d='%s'\n", x, substr(buffer, pm[x].rm_so, pm[x].rm_eo));
    }
    regfree(&reg);
}


TiXmlNode *Search(char *pName, TiXmlNode* pParent)//遍历时候，把每个节点都是做一个父节点，即假定其都有子节点ChildNode
{
    if ( !pParent ) return NULL;
    TiXmlNode* pChild= NULL;
    if (TiXmlNode::ELEMENT == pParent->Type() && (strcmp(pName, pParent->Value()) == 0))
        //搜索元素值为pName的节点
    {
        return pParent;
    }

    for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
    {
        TiXmlNode* finded = Search(pName, pChild);

        if(finded != NULL)
            return finded;
    }
    return NULL;
}

int paserXML(const char *buffer)
{
    TiXmlDocument doc;
    doc.Parse(buffer);
    /*
       bool loadOkay = doc.LoadFile();  
       if (!loadOkay) {      
       printf( "Could not load test file %s. Error='%s'. Exiting.\n", filepath,doc.ErrorDesc() );  
       exit( 1 ); 
       }  
       */
    TiXmlElement* root = doc.RootElement();  
    for (TiXmlElement* node = root->FirstChildElement(); node; node = node->NextSiblingElement()) 
    { 
        //递归处理子节点，获取节点指针

        if (strcmp("time", node->Value())==0  && node->GetText())
        {
            memcpy(appNode->entity, node->GetText(), strlen(node->GetText()));
        }


        //printf("%s :[%.*s]\n", node->Value(), strlen(node->GetText()), node->GetText());
        if (strcmp("text", node->Value())==0  && node->GetText())
        {
            int len = strlen(node->GetText());
            char *content = new char[len+10];
            snprintf(content, len+10, "%s", node->GetText());
            regex_txt("keyword%3D(%\w\w){2,}", content);
            delete [] content;
        }
    }
    return 0;
}

int SetParam(char *pDestStr, char*url)
{
    char *pSrcStr =pDestStr;
    CStream::SetBytes(pSrcStr, "ZSWAPQUE", 8);  //PROTOCOL
    CStream::SetDWord(pSrcStr, 2);              //OptType
    char *pCountStr = pSrcStr;
    CStream::SetDWord(pSrcStr, 0);              //LEN
    //CStream::SetStr(pSrcStr, "http://z.zhongsou.net/news/080808_6515630.html"); //Url
    CStream::SetStr(pSrcStr, url); //Url
    CStream::SetStr(pSrcStr, "test");           //keyword
    CStream::SetStr(pSrcStr, "1966511");        //userid
    CStream::SetStr(pSrcStr, "beijing");        //city
    CStream::SetQWord(pSrcStr, 0);              //经度
    CStream::SetQWord(pSrcStr, 0);              //纬度
    CStream::SetDWord(pSrcStr, 0);              //摘要最大长度
    CStream::SetFloat(pSrcStr, 0);              //摘要百分比
    CStream::SetDWord(pSrcStr, 0);              //分页类型：
    int length = pSrcStr - pDestStr - 16;
    CStream::SetDWord(pCountStr, length);         //LEN
    return pSrcStr - pDestStr;
}

int query_news()
{
    char *requestBuf = (char *)new char[1<<15];
    char *responseBuf = (char *)new char[1<<17];
    int length = SetParam(requestBuf, "http://z.zhongsou.net/news/080808_6515630.html");
    length = Communicate(requestBuf, length, responseBuf, 1<<17);
    if(length > 0)
    {
        paserXML(responseBuf);
    }

    delete [] requestBuf;
    delete [] responseBuf;

    return 0;
}
#endif

