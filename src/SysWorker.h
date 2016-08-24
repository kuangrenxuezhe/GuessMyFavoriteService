#ifndef SYS_WORKER_H
#define SYS_WORKER_H

#include "Worker.h"
#include "ReturnCode.h"
//
//RET  success :return the length of ack data body
//     failed : <0, return the error code ,the error code should define in ReturnCode.h
//
extern int g_quitFlag;

int server_shutdown(WorkParam *param)
{
	g_quitFlag = 1; 
	return sprintf(param->ackBody,"%s", "{\"server\":\"will be shutdown\"}");
}

//example success 
int server_alive(WorkParam * param)
{
	printf("I am server alive\n");
	printf("req[%.*s]\n",param->reqBodyLen,param->reqBody);
	printf("req[%p,%d]\ntemp[%p,%d]\nack[%p,%d]",param->reqBody,param->reqBodyLen
		, param->tempBuffer,param->tempBufferLen,param->ackBody,param->ackBodyMaxLen );
	
	return sprintf(param->ackBody,"%s", "{\"server\":\"alive\"}");
}

int server_test(WorkParam * param)
{
	printf("I am server_test\n");
	printf("req[%.*s]\n",param->reqBodyLen,param->reqBody);
	printf("req[%p,%d]\ntemp[%p,%d]\nack[%p,%d]",param->reqBody,param->reqBodyLen
		, param->tempBuffer,param->tempBufferLen,param->ackBody,param->ackBodyMaxLen );
	
	return sprintf(param->ackBody, "%s", "{\"worker\":\"worker_j1\"}");
}

#endif
