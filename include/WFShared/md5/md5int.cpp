//////////////////////////////////////////////////////////////////////////
//MD5Int.cpp
//////////////////////////////////////////////////////////////////////////
#include "md5_global.h"
#include "md5int.h"
#include "md5.h"
#include <assert.h>

//////////////////////////////////////////////////////////////////////////
void MD5String (const char *string, unsigned char *pstrReturn, unsigned int uLength)
{
	assert(string && string[0] && pstrReturn);
	assert(uLength > 0 && uLength <= 16);
	
	MD5_CTX context;
	unsigned char digest[16];
	unsigned int len = strlen (string);
	
	MD5Init(&context);
	MD5Update(&context, (unsigned char *)string, len);
	MD5Final(digest, &context);

	memcpy(pstrReturn, digest, uLength);
}


void MD5Segment (const char *string, unsigned int uStringLength, unsigned char *pstrReturn, unsigned int uReturnLength)
{
	assert(string && string[0] && uStringLength > 0 && pstrReturn);
	assert(uReturnLength > 0 &&uReturnLength <= 16);
	
	MD5_CTX context;
	unsigned char digest[16];
	unsigned int len = uStringLength;
	
	MD5Init(&context);
	MD5Update(&context, (unsigned char *)string, len);
	MD5Final(digest, &context);

	memcpy(pstrReturn, digest, uReturnLength);
}

void MD5Default (const char *szString, unsigned int uLength, unsigned char *szReturn)
{
	assert(szString && uLength > 0 && szReturn);

	MD5_CTX context;
	unsigned int len = uLength;
	
	MD5Init(&context);
	MD5Update(&context, (unsigned char *)szString, len);
	MD5Final(szReturn, &context);
}


