#ifndef DISK_VARIATION_EX_5e4bbd16a23d4de8827e1a0d245859e4
#define  DISK_VARIATION_EX_5e4bbd16a23d4de8827e1a0d245859e4
#include "lindow_api.hpp"


const char gszConfItemBegin[]="@#START{";
const char gszConfItemEnd[]="}END&#@";
const   int giLenConfItemBegin=strlen("@#START{");
const   int giLenConfItemEnd=strlen("}END&#@");

//根目录
static char gszDiskVariationRoot[512]={0};
//设置根目录
static int DiskVariationRoot(const char *pszRoot/*根目录*/)
{
	if(NULL==pszRoot)
		return -1;
	int ilen=strlen(pszRoot);
	if(ilen>510)
		return -2;
	memcpy(gszDiskVariationRoot,pszRoot,ilen);
	char *pPos=gszDiskVariationRoot+ilen-1;
	if('/'!=*pPos || '\\'!=*pPos)
	{
		++pPos;
		*(pPos++)='/';
		*(pPos++)='\0';
		*(pPos++)='\0';
	}
	return 0;

}
//desc 从末端向前查找 不支持中文
static char * StringReverseString(char *pText,const char *pSub)
{    
	int iIter = 0;
	const int icLenSub=strlen(pSub);
	char *pPosText=pText+strlen(pText)-icLenSub;
	for (;pPosText>=pText;--pPosText)  
	{         
		if(0==strncmp(pPosText,pSub,icLenSub))
		{
			return pPosText;
		}
	}     
	return NULL;
};
/*

最大数据长度不能超过1024个字节
*/
class CDiskVariation
{
protected:
	//
	FILE* m_hdFile;
	long m_iSizeFile;
	long m_iMaxSize;
	int m_iLenBlock;
	char m_szFile[512];
	static const int m_iMaxSizeBlock=1400;
private:
	//打开文件
	inline int Reopen();

public:
	//保存数据  如果固定块长  则不传入长度  非固定块长 必须传数据长度
	inline int Save(const char *pData,const long iLenData=-1);
	//加载最后一次的正确配置
	inline int Load(int iSizeData,char *pData,long *piLenData=NULL) const;
	/*初始化文件目录及名称
	char *pFName	 [IN]  名称
	int iBlockLen=-1 [IN]  大于零表示 每次存入的数据为固定长度
	*/
	int Init(const char *pFName/*文件名*/,int iMaxSizeFile/*最大使用空间*/, int iBlockLen=-1/*块长*/)
	{
		if(NULL==pFName || '\0'==pFName)
			return -1;
		
		m_iMaxSize=iMaxSizeFile/2;
		if(strlen(pFName)>500)
		{
			//文件名称太长了
			return -2;
		}
		strcpy(m_szFile,pFName);
		m_iSizeFile=0;
		m_iLenBlock=iBlockLen;
		return 0;
	};
	//清空数据
	inline int Delete()
	{
		char szTT[1024]={0};
		sprintf(szTT,"%s%s",gszDiskVariationRoot,m_szFile);
		lw_file_remove(szTT,true);
		sprintf(szTT,"%s%s_bak",gszDiskVariationRoot,m_szFile);
		lw_file_remove(szTT,true);
		return 0;
	};
public:
	CDiskVariation()
	{
		m_hdFile=NULL;
		m_iSizeFile=0;
		m_iMaxSize=0;
		m_iLenBlock=0;
		memset(m_szFile,0,sizeof(m_szFile));
	}
	~CDiskVariation()
	{
		if(NULL!=m_hdFile)
		{
			fclose(m_hdFile);
			m_hdFile=NULL;
		}
	}
};
/*
打开文件 
 根据文件长度 改变文件
*/
inline int CDiskVariation::Reopen()
{

	//如果文件太小 备份以后重新打开文件
	if(m_iSizeFile>m_iMaxSize)
	{
		char szOrg[1024]={0};
		char szBak[1024]={0};
		sprintf(szOrg,"%s%s",gszDiskVariationRoot,m_szFile);
		sprintf(szBak,"%s%s_bak",gszDiskVariationRoot,m_szFile);
		lw_rename(szOrg,szBak);
		if(NULL!=m_hdFile)
		{
			fclose(m_hdFile);
			m_hdFile=NULL;
		}
		m_iSizeFile=0;
	}
	//重新打开文件
	if(NULL==m_hdFile)
	{
		//原始文件
		char szOrg[1024]={0};
		sprintf(szOrg,"%s%s",gszDiskVariationRoot,m_szFile);
		m_hdFile= fopen(szOrg, "ab");
		if(NULL==m_hdFile)
			return -2;
		m_iSizeFile=lw_file_size(szOrg);
		if(m_iSizeFile>m_iMaxSize)
			return Reopen();

		if(m_iSizeFile>0)
			m_hdFile = fopen(szOrg, "ab");
		else
			m_hdFile = fopen(szOrg, "wb");
		if(NULL==m_hdFile)
			return -2;
	}
	return 0;

	
}; 
int CDiskVariation::Save(const char *pData,const long iLenData)
{
	if(NULL==m_hdFile || m_iSizeFile>m_iMaxSize)
	{ 
		int iError=Reopen();
		if(0!=iError)
		{
			return iError;
		}
	}

	int iError=0;
	const int iSizeBuffer=m_iMaxSizeBlock+240;
	if(iLenData>iSizeBuffer)
		return -20;
	int iLen=0;
	char szBuf[iSizeBuffer]={0};
	char *pPos=szBuf;
	//如果是固定长度
	if ( m_iLenBlock>0 && iLenData<=0)
	{
		//iLen=sprintf(szBuf,"%.*s",iLenData,pData);
		memcpy(pPos,pData,m_iLenBlock);
		pPos+=m_iLenBlock;
	}
	else if(iLenData>0)
	{
		//iLen=sprintf(szBuf,"@#START{%.*s}END&#@",iLenData,pData);//\n
		memcpy(pPos,gszConfItemBegin,giLenConfItemBegin);
		pPos+=giLenConfItemBegin;
		memcpy(pPos,pData,iLenData);
		pPos+=iLenData;
		memcpy(pPos,gszConfItemEnd,giLenConfItemEnd);
		pPos+=giLenConfItemEnd;
	}
	else 
		return -50;

	iLen=pPos-szBuf;
	while (true)
	{
		iError=fwrite((char *)szBuf, iLen, 1, m_hdFile);
		if(iError>0)
			break;
		printf("write file error file=%s\n",m_szFile);
		cp_sleep(5);
	}
	while (true)
	{
		iError=fflush(m_hdFile);
		if(0==iError)
			break;
		printf("flush file error file=%s\n",m_szFile);
		cp_sleep(5);
	}
	m_iSizeFile+=iLen;
	return 0;
};
int CDiskVariation::Load(int iSizeData,char *pData,long *piLenData) const
{
	if(m_iLenBlock>0 && iSizeData<m_iLenBlock)
		return -20;

	const int iSizeBuffer=m_iMaxSizeBlock*2+240*2;
	char szBuf[iSizeBuffer];
	char szOrg[1024]={0};
	char szBak[1024]={0};
	sprintf(szOrg,"%s%s",gszDiskVariationRoot,m_szFile);
	sprintf(szBak,"%s%s_bak",gszDiskVariationRoot,m_szFile);
	
	bool bOpenBak=false;
	FILE* hdFile=NULL;
	do 
	{
		if(NULL!=hdFile)
		{
			fclose(hdFile);
			hdFile=NULL;
		}
		if(false==bOpenBak)
			hdFile= fopen(szOrg, "rb");
		else
			hdFile = fopen(szBak, "rb");
		if(NULL==hdFile)
		{
			if(false==bOpenBak)
			{
				bOpenBak=true;
				continue;
			}
			return -1;
		}
		long iSizeFile=lw_file_size(hdFile);
		if(iSizeFile<=0)
		{
			if(false==bOpenBak)
			{
				bOpenBak=true;
				continue;
			}
			return -1;
		}
		if(m_iLenBlock>0)
		{
			if(iSizeFile<m_iLenBlock )
			{
				if( false==bOpenBak)
				{
					bOpenBak=true;
					continue;
				}
				return -20000;
			}
			int tLastBlock=(int)(iSizeFile/m_iLenBlock)-1;
			const int iStart=tLastBlock*(m_iLenBlock);
			const int iLiff=iSizeFile-iStart;
			fseek(hdFile,-iLiff,SEEK_END);
			int iLeaf=m_iLenBlock;
			char *pPos=pData;
			while(iLeaf>0)
			{
				int iLen=fread((char *)pPos, 1, iLeaf, hdFile);
				if(iLen>0)
				{
					pPos+=iLen;
					iLeaf-=iLen;
				}
				else
				{
					cp_sleep(1);
				}
			}
			*pPos='\0';
			fclose(hdFile);
			hdFile=NULL;
			return 0;
		}
		int iLeaf=0;
		if(iSizeFile<(iSizeBuffer-2))//加载整个文件
			iLeaf=iSizeFile;
		else
		{
			iLeaf=iSizeBuffer-2;
			fseek(hdFile,-iLeaf,SEEK_END);
		}
		char *pPos=szBuf;
		while(iLeaf>0)
		{
			int iLen=fread((char *)pPos, 1, iLeaf, hdFile);
			if(iLen>0)
			{
				pPos+=iLen;
				iLeaf-=iLen;
			}
			else
			{
				cp_sleep(1);
			}
		}
		*pPos='\0';
		fclose(hdFile);
		hdFile=NULL;
		bool bSuccess=false;
		int iTimes=0;
		while (iTimes<2)
		{
			char *pStart=StringReverseString(szBuf,gszConfItemBegin);
			if(NULL==pStart)
			{
				return -1101;
			}
			pStart+=giLenConfItemBegin;
			char *pEnd=strstr(pStart,gszConfItemEnd);
			if(NULL==pEnd)
			{
				++iTimes;
				*pStart='\0';
				continue;
			}
			*pEnd='\0';
			int isz=pEnd-pStart;
			if(isz>iSizeData)
				return -2000;
			memcpy(pData,pStart,isz);
			pData[isz]='\0';
			if(piLenData)
				*piLenData=isz;
			return 0;
		}
		if(false==bOpenBak)
		{
			bOpenBak=true;
			continue;
		}
	} while (true);
	if(NULL!=hdFile)
	{
		fclose(hdFile);
		hdFile=NULL;
	}
	return -1000;
}
#endif //END DISK_VARIATION_EX_5e4bbd16a23d4de8827e1a0d245859e4



