// ReadConfigFile.h

#ifndef _UC_READ_CONFIG_FILE_H_
#define _UC_READ_CONFIG_FILE_H_

#include "UH_Define.h"

class UC_ReadConfigFile
{
public:
	// 构造, 析构函数
	UC_ReadConfigFile()
	{
		m_cpFile = NULL;
	}	
	~UC_ReadConfigFile()
	{
		if(m_cpFile)
			fclose(m_cpFile);
	}
	// 初始化配置文件
	var_4 InitConfigFile(const var_1* filename)
	{
#ifdef _WIN32_ENV_
		m_cpFile = _fsopen(filename, "r", _SH_DENYWR);
#else
		m_cpFile = fopen(filename, "r");
#endif
		if(m_cpFile == NULL)
			return -1;

		return 0;
	}
	// 取字符串类型参数
	var_4 GetFieldValue(const var_1* szpField, var_1* szpValue)
	{
		var_4 lFindFlag = 0;
		var_1 szaReadBuf[1024];
		
		fseek(m_cpFile, 0, SEEK_SET);
		while(fgets(szaReadBuf, 1024, m_cpFile))
		{
			if(szaReadBuf[0] == (var_1)'#')
				continue;
			
			cp_drop_useless_char(szaReadBuf);

			var_1* p = strchr(szaReadBuf, '=');
			if(p == NULL)
				continue;
			p--;
			while(p > szaReadBuf && *p == ' ')
				p--;
			*++p = 0;

#ifdef _WIN32_ENV_
			if(_stricmp(szpField, szaReadBuf))
#else
			if(strcasecmp(szpField, szaReadBuf))
#endif		
				continue;
			p++;
			while(*p && (*p == ' ' || *p == '='))
				p++;
			if(*p == 0)
				continue;

			for(;;)
			{
				var_1* q = strrchr(p, ' ');
				if(q == NULL)
					break;
				*q = 0;
			}

			strcpy(szpValue, p);			
			lFindFlag = 1;
			break;
		}
		if(lFindFlag)
			return 0;
		return -1;
	}
	// 取长整形类型参数
	var_4 GetFieldValue(const var_1* szpField, var_4& lrValue)
	{		
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		lrValue = (var_4)atol(szaReadBuf);
		return 0;
	}
	var_4 GetFieldValue(const var_1* szpField, var_u4& lrValue)
	{		
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		lrValue = (var_u4)atol(szaReadBuf);
		return 0;
	}
	var_4 GetFieldValue(const var_1* szpField, var_8& lrValue)
	{
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		lrValue = atol(szaReadBuf);
		return 0;
	}
	var_4 GetFieldValue(const var_1* szpField, var_u8& lrValue)
	{
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		lrValue = (var_u8)atol(szaReadBuf);
		return 0;
	}	
	var_4 GetFieldValue(const var_1* szpField, var_f4& frValue)
	{
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		frValue = atof(szaReadBuf);
		return 0;
	}
	var_4 GetFieldValue(const var_1* szpField, var_d8& drValue)
	{
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		drValue = atof(szaReadBuf); // atof return double type
		return 0;
	}
	// 取短整形类型参数
	var_4 GetFieldValue(const var_1* szpField, var_u2& usrValue)
	{
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		usrValue = (var_u2)atol(szaReadBuf);
		return 0;
	}

	var_4 GetFieldValue(const var_1* szpField, var_1& usrValue)
	{
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		usrValue = *szaReadBuf;
		return 0;
	}

	// 取IP地址及端口号参数
	var_4 GetMacIPAndPort(const var_1* szpField, var_1* szpValue, var_u2& usrValue)
	{
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		
		var_1* p = szaReadBuf;
		var_1* q = p + strlen(p) - 1;
		while(q > p && *q != ':')
		{
			if(*q < '0' || *q > '9')
				break;
			q--;
		}
		if(*q != ':')
			return -1;
		*q++ = 0;
		usrValue = (var_u2)atol(q);
		strcpy(szpValue, p);

		return 0;
	}
	// 取IP地址及2个端口号参数
	var_4 GetMacIPAndPort(const var_1* szpField, var_1* szpIp, var_u2& usrPort_1, var_u2& usrPort_2)
	{
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		
		var_1* p = szaReadBuf;
		var_1* q = p + strlen(p) - 1;
		
		while(q > p && *q != ':')
		{
			if(*q < '0' || *q > '9')
				break;
			q--;
		}
		if(*q != ':')
			return -1;	
		q++;
		usrPort_2 = (var_u2)atol(q);
		q -= 2;
		if(q < p)
			return -1;
		
		while(q > p && *q != ':')
		{
			if(*q < '0' || *q > '9')
				break;
			q--;
		}
		if(*q != ':')
			return -1;
		*q++ = 0;
		usrPort_1 = (var_u2)atol(q);		
		
		strcpy(szpIp, p);
		
		return 0;
	}
	// 得到当前版本号
	const var_1* GetVersion()
	{
		// v1.000 - 2008.08.26 - 初始版本
		// v1.100 - 2009.03.31 - 增加跨平台支持
		return "v1.100";
	}

private:
	FILE* m_cpFile;
};

#endif // _UC_READ_CONFIG_FILE_H_
