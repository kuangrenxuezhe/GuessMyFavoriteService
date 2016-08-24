#ifndef WORK_GROUP_H
#define WORK_GROUP_H
#include "Server.h"
#include "WorkerManager.h"
#include "Stream.h"

extern CDailyLog *g_log;
extern Config *g_config;
extern CMonitor * g_monitor;
extern int g_showScreen;
extern int g_logLevel;

typedef struct
{
	void	*object;
	int		index;
} ThreadParam;

class WorkGroup
{
public:
    int m_running;
    int m_workThreadCount;
    int m_reqHeadLen;
    int m_reqDataBodyMaxLen;
    int m_tempBufferLen;
    int m_ackHeadLen;
    int m_ackDataBodyMaxLen;

    UT_Queue<conn_type> *m_queue;	
    int * m_workGroupQuitFlags;
    WorkerManager m_workerManager;
    WorkGroup()
    {
        m_overflow = 1024;
        m_running = 1;
    };

    ~WorkGroup()
    {
        free_object_array(m_reqHeadBuffer);
        free_object_array(m_reqDataBuffer);

        free_object_array(m_tempDataBuffer);

        free_object_array(m_ackHeadBuffer);
        free_object_array(m_ackDataBuffer);

        free_object_array(m_paramArray);
        free_object_array(m_threadsParam);	
    };
    
    int init(int workThreadCount,int reqDataBodyMaxLen, int tempBufferLen, int ackDataBodyMaxLen )
    {
        int i,ret;

        m_workThreadCount = workThreadCount;
        m_reqDataBodyMaxLen = reqDataBodyMaxLen;
        m_tempBufferLen = tempBufferLen;
        m_ackDataBodyMaxLen = ackDataBodyMaxLen;

        m_workGroupQuitFlags = new int[ m_workThreadCount ];
        die(NULL == m_workGroupQuitFlags,"WorkGroup::init new int array for m_workGroupQuitFlags failed\n");
        for(i = 0; i< m_workThreadCount; i++)
            m_workGroupQuitFlags[i] = 1;

        m_reqHeadLen = g_config->req_protocol_field_len + g_config->req_data_length_field_len + g_config->req_type_field_len;
        m_ackHeadLen = g_config->ack_data_length_field_len;	

        m_reqHeadBuffer = new char[ m_workThreadCount * (m_reqHeadLen + m_overflow)];
        die(NULL == m_reqHeadBuffer,"WorkGroup::init new memory for m_reqHeadBuffer failed\n");
        m_reqDataBuffer = new char[ m_workThreadCount * ( reqDataBodyMaxLen + m_overflow) ];
        die(NULL == m_reqDataBuffer,"WorkGroup::init new memory for m_reqDataBuffer failed\n");

        m_tempDataBuffer = new char[ m_workThreadCount * ( tempBufferLen + m_overflow) ];
        die(NULL == m_tempDataBuffer,"WorkGroup::init new memory for m_tempDataBuffer failed\n");

        m_ackHeadBuffer = new char[ m_workThreadCount * (m_ackHeadLen + m_overflow)];
        die(NULL == m_ackHeadBuffer,"WorkGroup::init new memory for m_ackHeadBuffer failed\n");
        m_ackDataBuffer = new char[ m_workThreadCount * ( ackDataBodyMaxLen + m_overflow) ];
        die(NULL == m_ackDataBuffer,"WorkGroup::init new memory for m_ackDataBuffer failed\n");

        m_paramArray = new WorkParam[ m_workThreadCount ];
        die(NULL == m_paramArray,"WorkGroup::init new memory for m_paramArray failed\n");

        m_threadsParam = new ThreadParam[ m_workThreadCount ];
        die(NULL == m_threadsParam,"WorkGroup::init new memory for m_threadsParam failed\n");

        ret = m_workerManager.init();
        die(ret,"WorkGroup::init m_workerManager.init failed\n");

        return 0;
    }

    int start( UT_Queue<conn_type> * queue)
    {
        assert(NULL != queue);
        m_queue = queue;
        int i,ret;
        for( i = 0; i<m_workThreadCount; i++)
        {
            m_threadsParam[i].object = (void * )this;
            m_threadsParam[i].index = i;
            ret = cp_create_thread(work_thread, (void *) &m_threadsParam[i] );
            die(ret,"WorkGroup::start cp_create_thread failed,i = %d, ret = %d\n",i,ret);
            m_workGroupQuitFlags[i] = 0;
        } 
        g_log->LPrintf(g_showScreen,"info:WorkGroup::start WorkGroup start ok\n");
        return 0;
    }
    static CP_THREAD_T work_thread( void * arg)
    {
        ThreadParam * param = (ThreadParam *) arg;
        WorkGroup * workGroup = (WorkGroup *) param->object;
        WorkerManager * workerManager = &workGroup->m_workerManager;
        worker lworker;
        int threadIndex = param->index;
        UT_Queue<conn_type> * queue = workGroup->m_queue;
        WorkParam * workParam = &workGroup->m_paramArray[threadIndex] ;	

        char * reqHead = workGroup->m_reqHeadBuffer + threadIndex*(workGroup->m_reqHeadLen + workGroup->m_overflow) ;
        int reqHeadLen = workGroup->m_reqHeadLen;
        char * reqDataBodyBuffer =  workGroup->m_reqDataBuffer 
            + threadIndex * ( workGroup->m_reqDataBodyMaxLen + workGroup-> m_overflow) ;
        int reqDataBodyMaxLen = g_config->req_data_body_max_len;
        int reqDataBodyLen;

        //char * ackHead = workGroup->m_ackHeadBuffer + threadIndex*(workGroup->m_ackHeadLen + workGroup->m_overflow) ;
        int ackHeadLen = workGroup->m_ackHeadLen;
        int ackBodyLen;
        char * ackDataBodyBuffer =  workGroup->m_ackDataBuffer
            + threadIndex * ( workGroup->m_ackDataBodyMaxLen +workGroup-> m_overflow);
        int ackDataBodyMaxLen = workGroup->m_ackDataBodyMaxLen;		

        char * tempBuffer = workGroup->m_tempDataBuffer + threadIndex*(workGroup->m_tempBufferLen + workGroup->m_overflow)  ;		
        int tempBufferLen = workGroup->m_tempBufferLen;		

        const char * protocol = g_config->req_protocol;
        int protocolLen = g_config->req_protocol_field_len;
        int bodyWithModeLen;//length in head	
        int lenFieldLen = g_config->req_data_length_field_len;

        int typeFieldLen = g_config->req_type_field_len;
        int retrievalType;

        int ackLengthFieldLen = g_config->ack_data_length_field_len;

        int ackCode,ret,loop, db_index,i;
        conn_type conn;
        CP_SOCKET_T sock;
        long acceptTime,endTime, interTime;
        const char * peerAddr, *logStr;
        char * endstr;			
        char temp;
        struct timeval sTimeval;

        char threadIdStr[64];
        int lastTraceLen = 0;
        char timeTrace[1024];
        timeTrace[0] = '0';
#define recodeTime(tag) \
        interTime = 0;\
        if( ! gettimeofday(&sTimeval,NULL))\
        interTime = sTimeval.tv_sec*1000000 +(long)sTimeval.tv_usec;\
        snprintf(timeTrace+lastTraceLen, 1024 - lastTraceLen, " [%s:%ld:%ld]", tag,interTime,interTime - acceptTime);\
        lastTraceLen = strlen(timeTrace);
#define time_addr_str ".acceptTime:%ld,endTime:%ld,diff:%ld,peerAddr:%s,timeTrace:%s.%s"
#define time_addr_param	acceptTime,endTime,endTime-acceptTime,peerAddr,timeTrace,threadIdStr

        int monitorId=g_monitor->LogIn(get_thread_id(),"work thread");
        die(monitorId <0,"WorkGroup::work_thread g_monitor->LogIn failed,ret=%d\n",monitorId);	

        g_log->LPrintf(g_showScreen,"work thread %d:%ul start\n",param->index,get_thread_id());
        snprintf(threadIdStr,64,"thread[%d:%ul]", param->index,get_thread_id());

        while(workGroup->m_running){
            //printf("work thead %d:%ul work\n",param->index,get_thread_id());
            //cp_sleep(3000);
            ret = g_monitor->UpdateStatus(monitorId,get_thread_id());
            if(ret)
                g_log->LPrintf(g_showScreen,"WorkGroup::work_thread UpdateStatus failed\n");

            loop = 0;
            while(workGroup->m_running && (ret = queue->PopData_NB(conn) )){
                ret = g_monitor->UpdateStatus(monitorId,get_thread_id());
                if(ret)
                    g_log->LPrintf(g_showScreen,"WorkGroup::work_thread UpdateStatus failed\n");

                if(g_logLevel >= LOG_LEVEL_DEBUG &&  0 == (loop= ++loop % 100000) )
                    g_log->LPrintf(g_showScreen,"debug:WorkGroup::work_thread m_queue.PopData_NB failed"
                            ",ret = %d,retry\n",ret);
                SleepMilliSecond(1);
                continue;
            };
            if( !workGroup->m_running)
                goto quit;

            sock = conn.socket_descrip;
            acceptTime = conn.acceptTime;
            peerAddr = conn.peerAddr;

            lastTraceLen = 0;
            timeTrace[0] = '0';    
            recodeTime("popFromQueue");
            
            ret = cp_recvbuf(sock, reqHead, reqHeadLen);
            if(ret){
                endTime = 0;		
                if( ! gettimeofday(&sTimeval,NULL))
                    endTime = sTimeval.tv_sec*1000000 +(long)sTimeval.tv_usec;
                g_log->LPrintf(g_showScreen
                        ,"error:WorkGroup::work_thread can not get enough head,ret = %d,sys error[%d:%s] drop" time_addr_str "\n"
                        ,ret,errno,strerror(errno),  time_addr_param );

                cp_close_socket(conn.socket_descrip );
                continue;
            }

            recodeTime( "receiveRequestHeadDataFromConn");
            if(g_logLevel >= LOG_LEVEL_DEBUG)
                    g_log->LPrintf(g_showScreen,"debug:WorkGroup::work_thread receive a request ,head:[%.*s]\n",reqHeadLen,reqHead);

            char *header = reqHead;
            char *protocolBuffer;
			CStream::GetBytes(header, protocolBuffer, protocolLen);
            
            if(strncmp(protocol, protocolBuffer, protocolLen))
            {
                ackCode = RET_SYS_PROTOCOL_ERROR;
                logStr = "protocol error";
                goto errHead;
            }

            errno = 0;
			retrievalType = CStream::GetDWord(header);
            if(errno != 0 || endstr == tempBuffer || retrievalType < 0){		
                ackCode = RET_SYS_TYPE_INVALID ;
                logStr = "type field invalid";
                goto errHead; 
            }
            
            errno = 0;
			bodyWithModeLen = CStream::GetDWord(header);
            if(errno != 0 || endstr == tempBuffer || bodyWithModeLen < 0){		
                ackCode = RET_SYS_DATA_LENGTH_INVALID;
                logStr = "data length field invalid";
                goto errHead; 
            }

            reqDataBodyLen = bodyWithModeLen;
            if(reqDataBodyLen > reqDataBodyMaxLen){
                ackCode =RET_SYS_DATA_LENGTH_TOBIG;
                logStr = "data body length too long";
                goto errHead; 
            }
            
            ret = cp_recvbuf(sock, reqDataBodyBuffer,reqDataBodyLen);
            if(ret){
                ackCode = RET_SYS_NO_ENOUGH_DATA;
                logStr = "cannot receive enough data body ";
                goto errHead;
            }		

            recodeTime("finishParseHeadAndreceiveRequestBodyDataFromConn");
            lworker = workerManager->getWorker(retrievalType);
            if(NULL == lworker)
            {
                ackCode = RET_SYS_TYPE_UNSUPPORT;
                logStr = "unsupport type";
                goto errHead; 
            } 

            workParam->reqBody = reqDataBodyBuffer;	
            workParam->reqBodyLen = reqDataBodyLen;
            workParam->tempBuffer = tempBuffer;
            workParam->tempBufferLen = tempBufferLen;
            workParam->ackBody = ackDataBodyBuffer;
            workParam->ackBodyMaxLen = ackDataBodyMaxLen;

            ackBodyLen = (* lworker)(workParam);
			
            recodeTime( "workerFinished")
            if(ackBodyLen > ackDataBodyMaxLen)
            {
                ackCode = RET_ACK_DATA_TOLONG;
                logStr = "ack data body too long";
                goto errHead; 
            }	

            header = tempBuffer;
			CStream::SetDWord(header, ackBodyLen);
            //if(ackBodyLen >=0) printf("body:%.*s\n",ackBodyLen,ackDataBodyBuffer);
            if( ackBodyLen >= 0){ 
				CStream::SetBytes(header, ackDataBodyBuffer, ackBodyLen<tempBufferLen-4?ackBodyLen:tempBufferLen-4);
                ret = cp_sendbuf(sock,tempBuffer, 4 + ackBodyLen);
            }
            else{
                ret = cp_sendbuf(sock,tempBuffer, 4);
            }
            if(ret){
                ackCode = RET_SYS_SEND_ACK_DATA_FAILED;
                logStr = "send ack body failed";
                goto errHead;
            }

            recodeTime( "doneAndbeginReceiveLastCharToEnd");

            cp_recvbuf(sock,&temp ,1);
            cp_close_socket(sock);

            recodeTime( "haveReceivedLastCharAndCloseSocket");
            endTime = 0;		
            if( ! gettimeofday(&sTimeval,NULL))
                endTime = sTimeval.tv_sec*1000000 +(long)sTimeval.tv_usec;
            g_log->LPrintf(g_showScreen,"info:WorkGroup::work_thread finish a request,ack head[%.*s], protocolNum[%d], ack body len :%d, return value :%d "time_addr_str "\n"
                    ,protocolLen,protocolBuffer, retrievalType,reqDataBodyLen, ackBodyLen,time_addr_param);

            continue;

errHead:
            {
                header = tempBuffer;
				CStream::SetDWord(header, ackCode);
                ret = cp_sendbuf(sock,tempBuffer, 4);
                if(ret){
                    cp_close_socket(sock);

                    endTime = 0;		
                    if( ! gettimeofday(&sTimeval,NULL))
                        endTime = sTimeval.tv_sec*1000000 +(long)sTimeval.tv_usec;
                    g_log->LPrintf(g_showScreen,"error:WorkGroup::work_thread send ack failed.logStr[%s] ,ack[%.*s],"
                            "request[%.*s],ret =%d ,sys error[%d:%s]," time_addr_str "\n"
                            ,logStr,ackHeadLen,tempBuffer,reqHeadLen,reqHead,ret,errno,strerror(errno), time_addr_param);

                    continue;
                }else{
                    cp_recvbuf(sock,&temp ,1);
                    cp_close_socket(sock);

                    endTime = 0;
                    if( ! gettimeofday(&sTimeval,NULL))
                        endTime = sTimeval.tv_sec*1000000 +(long)sTimeval.tv_usec;
                    g_log->LPrintf(g_showScreen,"error:WorkGroup::work_thread invalid request.logStr[%s],ack[%.*s],"
                            "request[%.*s],ret =%d ,sys error[%d:%s]," time_addr_str "\n"
                            ,logStr,ackHeadLen,tempBuffer,reqHeadLen,reqHead,ret,errno,strerror(errno), time_addr_param );
                    continue;
                }
            }//end errHead				
        }//end main loop
quit:	
        workGroup->m_workGroupQuitFlags[threadIndex] = 1;
        g_log->LPrintf(g_showScreen,"info:WorkGroup::workThread: work thread(%d:%ld) quit\n",threadIndex,get_thread_id());
        return 0;
    }

    int stop()
    {
        int i;
        m_running = 0;
        g_log->LPrintf(g_showScreen,"info:WorkGroup::stop begin stop thread\n");
        for( i = 0;i < m_workThreadCount; i++)
        {
            while( !m_workGroupQuitFlags[i] )
                cp_sleep(1);
        }
        g_log->LPrintf(g_showScreen,"info:WorkGroup::stop finished\n");
        return 0;
    }
private:
    int m_overflow;
    char * m_reqHeadBuffer;
    char * m_reqDataBuffer;
    char * m_tempDataBuffer;
    char * m_ackHeadBuffer;
    char * m_ackDataBuffer;
    ThreadParam * m_threadsParam;
    WorkParam * m_paramArray;
};

#endif

