//////////////////////////////////////////////////////////////////////////
//MD5Int.h
//////////////////////////////////////////////////////////////////////////
#ifndef _MD5_INTERFACE_H_
#define _MD5_INTERFACE_H_

#ifdef WIN32
#define WINDOWS_PLATFORM
#endif

#ifdef WIN64
#define WINDOWS_PLATFORM
#endif

//define
#ifdef WINDOWS_PLATFORM
#ifndef EXPORT
	#ifdef _cplusplus
		#define extern "C" EXPORT __declspec(dllexport)
	#else
		#define EXPORT __declspec(dllexport)
	#endif
#endif
#else
	#ifndef EXPORT
		#define EXPORT ;
	#endif
#endif

//////////////////////////////////////////////////////////////////////////
//Interface of MD5

//////////////////////////////////////////////////////////////////////////
// ���ܣ�ȡ��MD5ֵ�������ڽ϶̵��ַ���
// ������szString   �ַ���   szReturn  ����MD5ֵ   uLength   ����MD5ֵ�ĳ���
// ����ֵ����
// ˵����
//////////////////////////////////////////////////////////////////////////
EXPORT	void MD5String(const char *szString, unsigned char *szReturn, unsigned int uLength);

//////////////////////////////////////////////////////////////////////////
// ���ܣ�ȡ��MD5ֵ�������ڽϳ����ַ���
// ������szString   �ַ���   uLength  �ַ�������   szReturn  ����MD5ֵĬ��128λ
// ����ֵ����
// ˵����
//////////////////////////////////////////////////////////////////////////
EXPORT  void MD5Default(const char *szString, unsigned int uLength, unsigned char *szReturn);

//////////////////////////////////////////////////////////////////////////
// ���ܣ�ȡ��MD5ֵ
// ������szString   �ַ���   uStringLength  �ַ�������   szReturn  ����MD5ֵ 
//		uReturnLength  ����MD5ֵ����
// ����ֵ����
// ˵����
//////////////////////////////////////////////////////////////////////////
EXPORT	void MD5Segment(const char *szString, unsigned int uStringLength, unsigned char *szReturn , unsigned int uReturnLength);

#endif

