#ifndef WORKER_H
#define WORKER_H

enum CMD_CODE
{
    CMD_SERVER_QUIT     = 1,   // 关闭服务器
    CMD_SERVER_ALIVE    = 2,   // 测试服务器通信
};

typedef struct
{
    char *reqBody;
    int reqBodyLen;
    char *tempBuffer;
    int tempBufferLen;
    char *ackBody;
    int ackBodyMaxLen;
} WorkParam;

typedef int (*worker)( WorkParam *);

#endif
