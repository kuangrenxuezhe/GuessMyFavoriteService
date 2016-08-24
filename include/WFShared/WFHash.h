
#ifndef WF_HASH_H
#define WF_HASH_H

#include "WFMem.h"

namespace nsWFHash
{


template<typename KeyType>
class CHashTable
{

//Hash������: Nextָ��, KEY, DATA
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
    //FUNC  ��ʼ��
    //PARAM MaxDataCount��Hash�ṹ���ܹ���ŵ����������
    //      DataSize��Hash�ṹ�д�ŵĸ�����Ϣ��С
    //      HashSize��Hash�ṹ��Hash�����С
    //RET   0����ʼ���ɹ�
    //      -10��Hash��ֵ����ʧ��
    //      -20��Hash�����ڴ��������ʼ��ʧ��
    int Init(size_t MaxDataCount, size_t DataSize, size_t HashSize)
    {
        Reset();    //��ʼ��֮ǰ�������г�Ա����

        m_HashSize = HashSize;
        m_DataSize = DataSize;

        //����HashSize��ʼ��Hash����
        m_pHashArray = nsWFMem::New<SHashNode *>(m_HashSize);
        if (m_pHashArray == NULL)
        {
            return -10;
        }
        memset(m_pHashArray, 0, sizeof(SHashNode*)*m_HashSize);
   
        //��ʼ��Hash�����ڴ�
        int iRet = m_HashNodeMem.Init(MaxDataCount, sizeof(SHashNode)+m_DataSize);
        if (iRet != 0)
        {
            return -20;
        }

        return 0;
    }
    //FUNC  �������
    //PARAM key�����ݵĹؼ���
    //      pData�����ݵĸ�����Ϣ�׵�ַ
    //      pOldData������ָ���������ݸ�����Ϣ��ָ��
    //RET   0����ӳɹ�
    //      1�������ظ�key����ʱpOldDataָ�����и�����Ϣ
    //      -1�����ʧ�ܣ���Ϊ�ڴ治��
    int Add(KeyType key, void *pData, void *&pOldData)
    {
        //����Hashֵ����λHash����
        size_t HashIndex = (size_t)(key%m_HashSize);

        nsWFLock::CAutolock autolock(&m_Lock);
        SHashNode *pHashNode = m_pHashArray[HashIndex];

        //������ͻ��
        while (pHashNode != NULL)
        {
            if (key == pHashNode->key)
            {//���д�Key
                pOldData = pHashNode->pData;
                return 1;
            }
            pHashNode = pHashNode->pNext;
        }

        //���޴�Key�����
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
    //FUNC  ��������
    //PARAM key�����ݵĹؼ���
    //      pOldData������ָ���������ݸ�����Ϣ��ָ��
    //RET   0�����ҳɹ�����ʱpOldDataָ�����и�����Ϣ
    //      -1����Key������
    int Search(KeyType key, void *&pOldData)
    {
        size_t HashIndex = (size_t)(key%m_HashSize);

        nsWFLock::CAutolock autolock(&m_Lock);
        SHashNode *pHashNode = m_pHashArray[HashIndex];
        while (pHashNode != NULL)
        {
            if (key == pHashNode->key)
            {//���д�Key
                pOldData = pHashNode->pData;
                return 0;
            }
            pHashNode = pHashNode->pNext;
        }

        return -1;
    }
    //FUNC  ��������
    //PARAM key�����ݵĹؼ���
    //      pData�����ݵĸ�����Ϣ�׵�ַ
    //RET   0�����³ɹ�
    //      -1����Key������
    int Update(KeyType key, void *pData)
    {
        size_t HashIndex = (size_t)(key%m_HashSize);

        nsWFLock::CAutolock autolock(&m_Lock);
        SHashNode *pHashNode = m_pHashArray[HashIndex];
        while (pHashNode != NULL)
        {
            if (key == pHashNode->key)
            {//���д�Key
                memcpy(pHashNode->pData, pData, m_DataSize);
                return 0;
            }
            pHashNode = pHashNode->pNext;
        }

        return -1;
    }
    //FUNC  ɾ������
    //PARAM key�����ݵĹؼ���
    //RET   0��ɾ���ɹ�
    //      -1����Key������
    int Delete(KeyType key)
    {
        size_t HashIndex = (size_t)(key%m_HashSize);

        nsWFLock::CAutolock autolock(&m_Lock);
        SHashNode *pHashNode = m_pHashArray[HashIndex];
        SHashNode *pPreNode = NULL;
        while (pHashNode != NULL)
        {
            if (key == pHashNode->key)
            {//�ҵ���Key
                break;
            }
            pPreNode = pHashNode;
            pHashNode = pPreNode->pNext;
        }

        if (pHashNode == NULL)
        {//��Key������
            return -1;
        }

        //�޸�����
        SHashNode *pFreeNode = pHashNode;
        if (pPreNode == NULL)
        {//ɾ�����׽��
            m_pHashArray[HashIndex] = pFreeNode->pNext;
        }
        else
        {//ɾ���Ƕ��׽��
            pPreNode->pNext = pFreeNode->pNext;
        }

        //�ͷ��ڴ�
        m_HashNodeMem.Free(pFreeNode);
        return 0;
    }
    //FUNC  ɾ����������
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
    //FUNC  �ָ����г�Ա������ֵ
    //NOTE  ��黹������������ڴ�
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
    //FUNC  �������õ���һ�����
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
    //FUNC  �������õ���һ�����
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
    nsWFLock::CLock m_Lock;     //������
    SHashNode **m_pHashArray;   //Hash�����׵�ַ
    size_t m_HashSize;          //Hash���ݵĴ�С
    size_t m_DataSize;          //���ݸ�����Ϣ�Ĵ�С
    nsWFMem::CBlockMemory m_HashNodeMem;    //Hash�����ڴ������
private:
    size_t m_CurrIndex;
    SHashNode *m_pCurrNode;
};

}   // end of namespace

#endif  // end of #ifndef
