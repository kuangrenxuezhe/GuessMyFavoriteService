#ifndef DISK_VARIATION_H_60d14eba44ba47f1890bd52932853737
#define  DISK_VARIATION_H_60d14eba44ba47f1890bd52932853737
//设置根目录
static char gszDiskVariationRoot[512]={0};
int DiskVariationRoot(char *pszRoot)
{
	strncpy(gszDiskVariationRoot,pszRoot,510);
	return 0;

}

//加载变量的值 从文件
inline int LoadVariation(const char *pFName,unsigned long &uVar)
{
	if(NULL==pFName || '\0'==pFName)
		return -1;

	char szPathFile[512]={0};
	sprintf(szPathFile,"%s%s",gszDiskVariationRoot,pFName);
	FILE* fFile = fopen(szPathFile, "rb");
	if(NULL==fFile)
		return -2;
	fread((char *)&uVar, sizeof(unsigned long), 1, fFile);
	fclose(fFile);
	fFile=NULL;
	return 0;
}
//加载变量的值 从文件
inline int LoadVariation(const char *pFName,int iSizeData,char *pData, long *piLenData)
{
	if(NULL==pFName || '\0'==pFName || NULL==pData)
		return -1;

	char szPathFile[512]={0};
	sprintf(szPathFile,"%s%s",gszDiskVariationRoot,pFName);
	FILE* fFile = fopen(szPathFile, "rb");
	if(NULL==fFile)
		return -2;
	*piLenData=fread((char *)pData, 1, iSizeData, fFile);
	fclose(fFile);
	fFile=NULL;
	return 0;
}
inline int LoadVariation(const char *pFName,long &uVar)
{
	if(NULL==pFName || '\0'==pFName)
		return -1;

	char szPathFile[512]={0};
	sprintf(szPathFile,"%s%s",gszDiskVariationRoot,pFName);
	FILE* fFile = fopen(szPathFile, "rb");
	if(NULL==fFile)
		return -2;
	fread((char *)&uVar, sizeof(long), 1, fFile);
	fclose(fFile);
	fFile=NULL;
	return 0;
}
//保存变量的值 到文件
inline int SaveVariation(const char *pFName,unsigned long uVar)
{
	if(NULL==pFName || '\0'==pFName)
		return -1;

	char szPathFile[512]={0};
	sprintf(szPathFile,"%s%s",gszDiskVariationRoot,pFName);
	FILE* fFile = fopen(szPathFile, "wb");
	if(NULL==fFile)
		return -2;
	fwrite((char *)&uVar, sizeof(unsigned long), 1, fFile);
	fflush(fFile);
	fclose(fFile);
	fFile=NULL;
	return 0;
}
//保存变量的值 到文件
inline int SaveVariation(const char *pFName,long uVar)
{
	if(NULL==pFName || '\0'==pFName)
		return -1;

	char szPathFile[512]={0};
	sprintf(szPathFile,"%s%s",gszDiskVariationRoot,pFName);
	FILE* fFile = fopen(szPathFile, "wb");
	if(NULL==fFile)
		return -2;
	fwrite((char *)&uVar, sizeof(long), 1, fFile);
	fflush(fFile);
	fclose(fFile);
	fFile=NULL;
	return 0;
}
//保存变量的值 到文件
inline int SaveVariation(const char *pFName,char *pData,long iLenData)
{
	if(NULL==pFName || '\0'==pFName)
		return -1;

	char szPathFile[512]={0};
	sprintf(szPathFile,"%s%s",gszDiskVariationRoot,pFName);
	FILE* fFile = fopen(szPathFile, "wb");
	if(NULL==fFile)
		return -2;
	fwrite((char *)pData, iLenData, 1, fFile);
	fflush(fFile);
	fclose(fFile);
	fFile=NULL;
	return 0;
}



#endif //END DISK_VARIATION_H_60d14eba44ba47f1890bd52932853737



