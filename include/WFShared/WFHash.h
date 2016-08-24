
#ifndef WF_HASH_H
#define WF_HASH_H

#include "WFMem.h"

namespace nsWFHash
{


template<typename KeyType>
class CHashTable
{

//Hash结点包括: Next指针, KEY, DATA
#pragma pack(1)
struct SHashNode
{
    SHashNode *pNext;
    KeyType key;
    char pData[0];
};
#pragma pack()

public:
    CHashTable()
    {
        m_pHashArray = NULL;
        m_HashSize = 0;
        m_DataSize = 0;
    }
    ~CHashTable()
    {
        Reset();   	  
    }
    //FUNC  初始化
    //PARAM MaxDataCount：Hash结构中能够存放的最大数据量
    //      DataSize：Hash结构中存放的附属信息大小
    //      HashSize：Hash结构的Hash数组大小
    //RET   0：初始化成功
    //      -10：Hash数值申请失败
    //      -20：Hash结点块内存分配器初始化失败
    int Init(size_t MaxDataCount, size_t DataSize, size_t HashSize)
    {
        Reset();    //初始化之前重置所有成员变量

        m_HashSize = HashSize;
        m_DataSize = DataSize;

        //根据HashSize初始化Hash数组
        m_pHashArray = nsWFMem::New<SHashNode *>(m_HashSize);
        if (m_pHashArray == NULL)
        {
            return -10;
        }
        memset(m_pHashArray, 0, sizeof(SHashNode*)*m_HashSize);
   
        //初始化Hash结点的内存
        int iRet = m_HashNodeMem.Init(MaxDataCount, sizeof(SHashNode)+m_DataSize);
        if (iRet != 0)
        {
            return -20;
        }

        return 0;
    }
    //FUNC  添加数据
    //PARAM key：数据的关键字
    //      pData：数据的附加信息首地址
    //      pOldData：用于指向已有数据附加信息的指针
    //RET   0：添加成功
    //      1：发现重复key，此时pOldData指向已有附加信息
    //      -1：添加失败，因为内存不足
    int Add(KeyType key, void *pData, void *&pOldData)
    {
        //计算Hash值，定位Hash数组
        size_t HashIndex = (size_t)(key%m_HashSize);

        nsWFLock::CAutolock autolock(&m_Lock);
        SHashNode *pHashNode = m_pHashArray[HashIndex];

        //遍历冲突链
        while (pHashNode != NULL)
        {
            if (key == pHashNode->key)
            {//已有此Key
                pOldData = pHashNode->pData;
                return 1;
            }
            pHashNode = pHashNode->pNext;
        }

        //尚无此Key，添加
        pHashNode = (SHashNode *)m_HashNodeMem.Alloc();
        if (pHashNode == NULL)
        {
            return -1;
        }

        pHashNode->key = key;
        if (m_DataSize > 0)
        {
            memcpy(pHashNode->pData, pData, m_DataSize);
        }
        pHashNode->pNext = m_pHashArray[HashIndex];
        m_pHashArray[HashIndex] = pHashNode;
        return 0;
    }
    //FUNC  查找数据
    //PARAM key：数据的关键字
    //      pOldData：用于指向已有数据附加信息的指针
    //RET   0：查找成功，此时pOldData指向已有附加信息
    //      -1：此Key不存在
    int Search(KeyType key, void *&pOldData)
    {
        size_t HashIndex = (size_t)(key%m_HashSize);

        nsWFLock::CAutolock autolock(&m_Lock);
        SHashNode *pHashNode = m_pHashArray[HashIndex];
        while (pHashNode != NULL)
        {
            if (key == pHashNode->key)
            {//已有此Key
                pOldData = pHashNode->pData;
                return 0;
            }
            pHashNode = pHashNode->pNext;
        }

        return -1;
    }
    //FUNC  更新数据
    //PARAM key：数据的关键字
    //      pData：数据的附加信息首地址
    //RET   0：更新成功
    //      -1：此Key不存在
    int Update(KeyType key, void *pData)
    {
        size_t HashIndex = (size_t)(key%m_HashSize);

        nsWFLock::CAutolock autolock(&m_Lock);
        SHashNode *pHashNode = m_pHashArray[HashIndex];
        while (pHashNode != NULL)
        {
            if (key == pHashNode->key)
            {//已有此Key
                memcpy(pHashNode->pData, pData, m_DataSize);
                return 0;
            }
            pHashNode = pHashNode->pNext;
        }

        return -1;
    }
    //FUNC  删除数据
    //PARAM key：数据的关键字
    //RET   0：删除成功
    //      -1：此Key不存在
    int Delete(KeyType key)
    {
        size_t HashIndex = (size_t)(key%m_HashSize);

        nsWFLock::CAutolock autolock(&m_Lock);
        SHashNode *pHashNode = m_pHashArray[HashIndex];
        SHashNode *pPreNode = NULL;
        while (pHashNode != NULL)
        {
            if (key == pHashNode->key)
            {//找到此Key
                break;
            }
            pPreNode = pHashNode;
            pHashNode = pPreNode->pNext;
        }

        if (pHashNode == NULL)
        {//此Key不存在
            return -1;
        }

        //修改链表
        SHashNode *pFreeNode = pHashNode;
        if (pPreNode == NULL)
        {//删除队首结点
            m_pHashArray[HashIndex] = pFreeNode->pNext;
        }
        else
        {//删除非队首结点
            pPreNode->pNext = pFreeNode->pNext;
        }

        //释放内存
        m_HashNodeMem.Free(pFreeNode);
        return 0;
    }
    //FUNC  删除所有数据
    void DeleteAll()
    {
        nsWFLock::CAutolock autolock(&m_Lock);
        for (size_t i = 0; i < m_HashSize; ++i)
        {
            SHashNode *pHashNode = m_pHashArray[i];
            while (pHashNode != NULL)
            {
                SHashNode *pFreeNode = pHashNode;
                pHashNode = pFreeNode->pNext;
            
                m_HashNodeMem.Free(pFreeNode);
            }
            m_pHashArray[i] = NULL;
        }
    }
    //FUNC  恢复所有成员变量的值
    //NOTE  会归还所有已申请的内存
    void Reset()
    {
        nsWFLock::CAutolock autolock(&m_Lock);
        if (m_pHashArray != NULL)
        {
            delete[] m_pHashArray;
            m_pHashArray = NULL;
        }
        m_HashSize = 0;
        m_DataSize = 0;
        m_HashNodeMem.Reset();
    }
    //FUNC  遍历，得到第一个结点
    bool TraversalFirst(KeyType &Key, char *&pOldData)
    {        
        for (m_CurrIndex = 0; m_CurrIndex < m_HashSize; ++m_CurrIndex)
        {
            if (m_pHashArray[m_CurrIndex] != NULL)
            {
                break;
            }
        }

        if (m_CurrIndex == m_HashSize)
        {
            return false;
        }

        m_pCurrNode = m_pHashArray[m_CurrIndex];
        Key = m_pCurrNode->key;
        pOldData = m_pCurrNode->pData;
        return true;
    }
    //FUNC  遍历，得到下一个结点
    bool TraversalNext(KeyType &Key, char *&pOldData)
    {
        if (m_pCurrNode == NULL)
        {
            return false;
        }
        //goto next
        m_pCurrNode = m_pCurrNode->pNext;

        //check it
        if (m_pCurrNode == NULL)
        {
            for (++m_CurrIndex; m_CurrIndex < m_HashSize; ++m_CurrIndex)
            {
                if (m_pHashArray[m_CurrIndex] != NULL)
                {
                    break;
                }
            }

            if (m_CurrIndex == m_HashSize)
            {
                return false;
            }
            m_pCurrNode = m_pHashArray[m_CurrIndex];
        }

        Key = m_pCurrNode->key;
        pOldData = m_pCurrNode->pData;
        return true;
    }

protected:
private:
    nsWFLock::CLock m_Lock;     //互斥锁
    SHashNode **m_pHashArray;   //Hash数组首地址
    size_t m_HashSize;          //Hash数据的大小
    size_t m_DataSize;          //数据附属信息的大小
    nsWFMem::CBlockMemory m_HashNodeMem;    //Hash结点的内存管理器
private:
    size_t m_CurrIndex;
    SHashNode *m_pCurrNode;
};

}   // end of namespace

#endif  // end of #ifndef
