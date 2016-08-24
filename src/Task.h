#ifndef TASK_WORKER_H
#define TASK_WORKER_H

#include <string>
#include <rdkafkacpp.h>
#include <KafkaEx.hpp>
#include <KafkaWriteEx.hpp>
#include "Stream.h"
#include <UH_Define.h>
#include <errno.h>
#include "../json/json.h"
#include "../WFShared/WFHash.h"

extern CDailyLog *g_log;
extern int g_showScreen;
extern int g_logLevel;
extern Config *g_config;

using namespace nsWFPub;

class CKafkaTask
{
public:
	CKafkaTask()
    {
    }
    
    ~CKafkaTask()
    {
        m_kafka.Disconnect(); 
    }
    
    int Init(char *dict, char* fname, char *host, short port, char*dbname, long offset, int (*process_func)(char* buffer, int length))
    {
        int ret = m_kafka.Init(dict, fname);
        if(ret < 0)
        {
            g_log->LPrintf(g_showScreen, "kafka Init failed dict:[%s], fname:[%s]\n", dict, fname);
            return ret;
        }

        m_process = process_func;
        m_piOffset = offset;
        ret = m_kafka.Connect(host, port, dbname, "0", m_piOffset);
        if(ret < 0)
        {
            g_log->LPrintf(g_showScreen, "kafka Connect failed host:[%s], port:[%d], dbname:[%s], \
            offset:[%d]\n", host, port, dbname, m_piOffset);
        }
        return ret;
    }

	static void Load(void *param)
    {
        CKafkaTask *task = (CKafkaTask *)param;
        int ret = 0;
        while(ret == 0)
        {
            int piLen = 0;
            memset(task->m_buffer, 0, task->max_buffer_size*sizeof(char));
            ret = task->m_kafka.GetMessage(task->max_buffer_size, task->m_buffer, &piLen, &task->m_piOffset); 
            
            if(task->m_process != 0)
                task->m_process(task->m_buffer, piLen);
        }
    }
    
private:
    CKafkeEx m_kafka;
    long m_piOffset;
    
    const static long max_buffer_size = (1<<10);
    char m_buffer[max_buffer_size];
    int (*m_process)(char* buffer, int length);
};

class CTask
{
public:
	static void Reload(void *param){
		g_log->LPrintf(true,"Task Reload start success\n");
		sleep(5);
	}

	static int load()
    {
        CKafkeEx kafka;
        int ret = kafka.Init(g_config->kafka_file_dir, g_config->kafka_file_name);
        if(ret < 0)
            return ret;

        long piOffset = -100;
        ret = kafka.Connect(g_config->kafkas[0].ip, g_config->kafkas[0].port, g_config->kafkas[0].dbName, "0", piOffset);
        if(ret < 0)
            return ret;

        long max_buffer_size = 1<<10;
        char *buffer =(char *) new char[max_buffer_size];
        if(buffer == 0)
            return -10000;
        ret = 0;
        while(ret==0)
        {
            memset(buffer, 0, max_buffer_size*sizeof(char));
            int piLen = 0;
            ret = kafka.GetMessage(max_buffer_size, buffer, &piLen, &piOffset); 
        //  users.AddToUserList(buffer, piLen);
        }
        
        delete [] buffer;
        kafka.Disconnect();
        return 0;
    }

	static void Test(void *param){
		g_log->LPrintf(true,"Task Test start success\n");
	}
    
private:

};

#endif

