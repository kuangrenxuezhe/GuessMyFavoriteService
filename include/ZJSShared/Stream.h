#ifndef _ZJS_STREAM_H
#define _ZJS_STREAM_H

#ifdef _WIN32
#define WINDOWS_PLATFORM
#endif

#ifdef WIN64
#define WINDOWS_PLATFORM
#endif

#include <string.h>

#ifndef WINDOWS_PLATFORM
#include <arpa/inet.h>
#endif

//数据流读写采用小头编码方式
//在每一个函数调用后数据流指针都会向后移动 注意保存初始指针地址！！！！！！
namespace nsZJSStream
{
    class CStream
    {
        public:  
            //设置字节流 
            static void SetBytes(char *& pp, const char * bytes, int len)
            {
                char * ptr;
                int i;

                ptr = pp;
                if(ptr)
                    for(i=0; i<len; i++)
                        *ptr++ = bytes[i];
                pp = ptr;
            }

            //设置四字节数字
            static void SetDWord(char *& pp, unsigned int value)
            {
                unsigned char * b = (unsigned char*) pp;
                b[0] =(value & 0xFF);
                b[1] =(value >> 8) & 0xff;
                b[2] =(value >> 16) & 0xff;
                b[3] =(value >> 24) & 0xff;
                pp += 4;
            }

            //设置双字节数字
            static void SetWord(char *& pp, unsigned short value)
            {
                unsigned char * b = (unsigned char*) pp;
                b[0] =(value & 0xFF);
                b[1] =(value >> 8);
                pp += 2;
            }

            //设置字符串 
            static void SetStr(char *& pp, const char * s)
            {
                int len;
                len = s ? (int)strlen(s) : 0;
                SetDWord(pp, len);
                SetBytes(pp, s, len);
            }

            //设置八字节数字
            static void SetQWord(char *& pp, unsigned long long value)
            {
                SetDWord(pp, (int)( value & ((unsigned long long)0xffffffffL)));
                SetDWord(pp, (int)( value >> 32));
            }

            //设置浮点数
            static void SetFloat(char *& pp, float value)
            {
                union
                {
                    float f;
                    int i;
                } u;

                u.f = value;
                SetDWord(pp, u.i);
            }

            //获得双字节数字
            static unsigned short GetWord(char *& cur)
            {
                unsigned short v = *(unsigned short *)cur;
                cur += sizeof(unsigned short);
                return v;
            }

            //获得四字节数字
            static unsigned int GetDWord(char *& cur)
            {
                unsigned int v = *(unsigned int *)cur;
                cur += sizeof(unsigned int);
                return v;
            }

            //获得字节流
            static void GetBytes(char *& cur, char *&buffer, unsigned int length)
            {
                buffer = cur;
                cur += length;
            }

            //获得字符串 不改原始流cur
            static void GetStr(char *& cur, char *&buffer, unsigned int &length)
            {
                unsigned int len;
                len = GetDWord(cur);
                buffer = cur;
                length = len;
                cur += len;
            }

            //获得字符串 改变原始流cur 
            static char *GetStr(char *& cur)
            {
                // we play a trick
                // we move the string in-place to free space for trailing zero but avoid malloc
                unsigned int len;
                len = GetDWord ( cur );
                memmove ( cur-1, cur, len );
                cur += len;
                cur[-1] = '\0';
                return cur-len-1;
            }

            //获得八字节数字
            static unsigned long long GetQWord(char *& cur)
            {
                unsigned long long hi, lo;
                lo = GetDWord(cur);
                hi = GetDWord(cur);
                return(hi<<32) + lo;
            }

            //获得浮点数
            static float GetFloat(char *& cur)
            {
                union
                {
                    unsigned int n;
                    float f;
                } u;
                u.n = GetDWord(cur);
                return u.f;
            }
    };
}   // end of namespace
#endif //_ZJS_STREAM_H
