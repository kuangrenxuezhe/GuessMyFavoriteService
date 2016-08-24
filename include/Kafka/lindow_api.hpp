#ifndef LINDOW_HPP_6299210249114d19b13f5ee6aaaa497e
#define LINDOW_HPP_6299210249114d19b13f5ee6aaaa497e

#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#else
#include <windows.h>
#endif
#include <dirent.h> 
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
// 系统类型定义
#ifdef WIN32

#define _WIN32_ENV_
#define _SYSTEM_X86_

#else

#define _LINUX_ENV_
#define _SYSTEM_X64_

#endif

#ifdef _WIN32_ENV_
#define cp_sleep(x)			Sleep(x)
#else
#define cp_sleep(x)			usleep(x*1000)
#endif

#define  closesocket(fd)  ::close(fd);

#pragma region 输出操作

#ifdef _LINUX_ENV_
#ifndef  _snprintf
#define _snprintf   snprintf
#endif

#ifndef _vsnprintf
#define _vsnprintf  vsnprintf
#endif
#endif
#pragma endregion

#pragma region 文件操作   

//确定文件访问操作
//iMode :  00 Existence only 02  Write permission 04 Read permission 06 Read and write permission 
inline const int  lw_access( const char * const filename,int icode)
{
#ifdef _LINUX_ENV_
	return access(filename,icode);
#else
	return _access(filename,icode);
#endif
}
//文件是否存在 
inline bool lw_exist( const char * const lpszFileName)
{
	if(lpszFileName)
		return (lw_access(lpszFileName,0)==0);
	return false;
}
//取得文件SIZE
inline long lw_file_size( const char * const lpszFileName)
{
	if(lpszFileName==NULL)
		return -1;
	if(!lw_exist(lpszFileName))
		return -2;

#ifdef _LINUX_ENV_
	struct stat sStatus;

	if(stat(lpszFileName,&sStatus)!=0)
		return 0;
	return sStatus.st_size;
#else
	struct _stat sFileInfo;
	if(_stat(lpszFileName,&sFileInfo)!=0)
		return 0;

	return sFileInfo.st_size ;
#endif
}

inline long lw_file_size( FILE * fp)
{
	if(fp==NULL)
		return -1;

#ifdef _LINUX_ENV_
	struct stat sStatus;

	if(fstat(fileno(fp),&sStatus)!=0)
		return 0;
	return sStatus.st_size;
#else
	struct _stat sFileInfo;
	if(_fstat(_fileno(fp),&sFileInfo)!=0)
		return 0;

	return sFileInfo.st_size ;
#endif
}

/*desc  删除文件*/
inline int lw_file_remove(char * file,bool bBack=0)
{
	while(0==access(file, 0))
	{
		if(0==bBack)
		{
			if(0==remove(file))
				break;
			printf("lw_file_remove.1 - remove %s error\n", file);
			cp_sleep(1);
			continue;
		}
		//则改名
		time_t tCur=0;
		time(&tCur);
		char szDes[1026];
		_snprintf(szDes,1024,"%slw%llu",file,tCur);
		lw_file_remove(szDes);
		if(0==rename(file,szDes))
		{
			break;
		}
		printf("lw_file_remove.2 - remove %s error\n", file);
		cp_sleep(1);
	}
	return 0;
}
inline int lw_direct_create(char *dir) 
{
	if(dir == NULL)
		return -1;

	int len = (int)strlen(dir);
	if(len <= 0 || len >= 256)
		return -1;

	char  dir_name[256];
	char* dir_ptr = dir_name;
	char* ptr = dir;

	if(*ptr == '\\' || *ptr == '/')
		*dir_ptr++ = *ptr++;

	if(*ptr == 0)
		return -1;

	for(;;)
	{
		while(*ptr)
		{
			if(*ptr == '\\' || *ptr == '/')
				break;
			*dir_ptr++ = *ptr++;
		}
		*dir_ptr = 0;
#ifdef _WIN32_ENV_
		if(_mkdir(dir_name))
#else
		if(mkdir(dir_name, S_IRWXO|S_IRWXU))
#endif
		{
			if(errno != EEXIST)
				return -1;
		}
		if(*ptr)
			*dir_ptr++ = *ptr++;

		if(ptr - dir >= len)
			break;
	}
	return 0;
}
void lw_direct_clean(char *path,bool bBack=0) ;

//删除目录及目录下的所有文件
inline void lw_direct_remove(char *path,bool bBack=0) 
{ 


	while(0==access(path, 0))
	{
		if(0==bBack)
		{
			lw_direct_clean(path);
			if(0==rmdir(path))
				break;
			printf("lw_direct_remove.1 - remove %s error\n", path);
			cp_sleep(1);
			continue;
		}
		//则改名
		time_t tCur=0;
		time(&tCur);
		char szDes[1026];
		_snprintf(szDes,1024,"%slw%llu",path,tCur);
		lw_direct_remove(szDes);
		if(0==rename(path,szDes))
		{
			break;
		}
		printf("lw_file_remove.2 - remove %s error\n", path);
		cp_sleep(1);
	}
} 
//清空目录下的所有文件
inline void lw_direct_clean(char *path,bool bBack) 
{ 
	if(NULL==path || '\0'==*path)
	{
		printf("Null the path path=0x%X",path); 
		return ;
	}
	DIR* dp = NULL; 
	DIR* dpin = NULL; 
	char pathname[512]; 
	struct dirent* dirp; 
	dp = opendir(path); 
	if(dp == NULL) 
	{ 
		printf("Can't open the path %s\n",path); 
		return; 
	} 

	while((dirp = readdir(dp)) != NULL) 
	{ 
		if(strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, ".") == 0) 
			continue; 
		strcpy(pathname, path); 
		strcat(pathname, "/"); 
		strcat(pathname, dirp->d_name); 
		dpin = opendir(pathname); 
		if(dpin != NULL) 
			lw_direct_remove(pathname,bBack); 
		else 
			lw_file_remove(pathname,bBack); 
		strcpy(pathname, ""); 
		closedir(dpin); 
		dpin = NULL; 
	} 
	closedir(dp); 
	dirp = NULL; 
} 

/*desc  重命名文件
char * src_file		[IN]  源文件
char * des_file		[IN]  目标文件,
bool bBack			[IN]  如果目标文件存在,则备份目标文件 

desc:
因无所确定目标文件的大小,所以默认不备份文件，防止占用太多的磁盘空间
*/
inline int lw_rename(char * src_file/*源文件*/, char * des_file/*目标文件*/,bool bBack=0)
{
	//源文件不存在则退出
	while(0==access(src_file, 0))
	{
		//目标文件存在
		while (0==access(des_file, 0))
		{
			if(0==bBack)
			{
				if(0==lw_file_remove(des_file))
					break;

				cp_sleep(1);
				continue;
			}
			//则改名
			time_t tCur=0;
			time(&tCur);
			char szDes[1026];
			_snprintf(szDes,1024,"%slw%llu",des_file,tCur);
			lw_file_remove(szDes);
			if(0==rename(des_file,szDes))
			{
				break;
			}
			printf("lw_rename_file-2 - rename %s to %s error\n", des_file, szDes);
			cp_sleep(1);
		}//end if(0==access(des_file, 0))
		if(0==rename(src_file, des_file))
		{
			break;
		}
		printf("cp_rename_file - rename %s to %s error\n", src_file, des_file);
		cp_sleep(1);
	}//end for(int i=0;i<iTryTimes;++i)
	return 0;
}
#pragma endregion


#endif //end LINDOW_HPP_6299210249114d19b13f5ee6aaaa497e