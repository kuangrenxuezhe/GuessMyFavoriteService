#ifndef SERVER_H
#define SERVER_H

#include "UT_Queue.h"
#include "WFPub.h"

extern CDailyLog *g_log;
extern int g_showScreen;
extern int g_logLevel;

using namespace nsWFPub;

typedef struct
{
    CP_SOCKET_T socket_descrip;
    long acceptTime;//micro second	
    char peerAddr[24];
} conn_type;

class Server
{
    public:
        Server(){
            m_running = 1;
            m_quitOk = 1;
        }
        UT_Queue<conn_type> m_queue;
        int m_running;
        int m_quitOk;
        unsigned short m_port;
        int m_sendTimeout;
        int m_receiveTimeout;
    private:
    public:
        int init(int maxQueueLen )
        {
            int ret = m_queue.InitQueue(maxQueueLen);
            die(ret,"Server::init m_queue.InitQueue failed,ret = %d\n",ret);
            ret = cp_init_socket();
            die(ret,"Server::init cp_init_socket failed,ret = %d\n",ret);

            return 0;
        }
        int start(unsigned short port,int sendTimeout,int receiveTimeout)
        {
            m_port = port;
            m_sendTimeout = sendTimeout;
            m_receiveTimeout = receiveTimeout;
            int ret = cp_create_thread(server_thread,this);
            die(ret,"Server::start cp_create_thread failed, ret = %d\n",ret);	
            m_quitOk = 0;
            return 0;
        }

        static CP_THREAD_T server_thread(void *arg)
        { 
            CP_SOCKET_T serverSock,tempSock;
            struct timeval sTimeval;
            conn_type temp_conn;
            long acceptTime = 0;
            Server * server = (Server *) arg;
            const char * peer;
            unsigned short peerPort;
            int loop = 0;
            struct sockaddr_in peerAddr;
            socklen_t pAddrLen = sizeof(struct sockaddr_in);

            int ret = cp_listen_socket(serverSock,server->m_port);
            die( ret,"Server::server_thread cp_listen_socket failed,ret = %d\n",ret);
            g_log->LPrintf(g_showScreen,"info:Server::server_thread server thread(%ul) start\n",get_thread_id());

            while(server->m_running){
                tempSock = accept(serverSock,(struct sockaddr*) &peerAddr,&pAddrLen);
                if(tempSock < 0)
                {
                    g_log->LPrintf(g_showScreen,"error:Server::server_thread cp_accept_socket failed,ret=%d,syserror[%d:%s]\n"
                            ,tempSock,errno,strerror(errno));
                    SleepMilliSecond(1);
                    continue;
                }

                acceptTime = 0;
                if( ! gettimeofday(&sTimeval,NULL))
                    acceptTime = sTimeval.tv_sec*1000000 +(long)sTimeval.tv_usec;
                peer = inet_ntoa( peerAddr.sin_addr);
                peerPort = ntohs ( peerAddr.sin_port); 
                temp_conn.socket_descrip = tempSock;
                temp_conn.acceptTime = acceptTime;
                snprintf(temp_conn.peerAddr,24,"%s:%d", NULL == peer?"":peer,peerPort);

                setOvertime(tempSock,server->m_sendTimeout,server->m_receiveTimeout);

                if(g_logLevel >=  LOG_LEVEL_DEBUG)
                    g_log->LPrintf(g_showScreen,"debug:Server::server_thread accept socket on %ld from %s\n"
                            ,acceptTime,temp_conn.peerAddr);

                while(server->m_running){
                    if( (ret = server->m_queue.PushData_NB(temp_conn)) ){
                        if(g_logLevel >= LOG_LEVEL_DEBUG &&  0 == (loop= ++loop % 100000) )
                            g_log->LPrintf(g_showScreen,"debug:Server::server_thread m_queue.PushData failed,ret = %d,retry\n",ret);
                        SleepMilliSecond(1);
                        continue;
                    }
                    break;
                }
            }//end main while

            cp_close_socket(serverSock);
            server->m_quitOk = 1;
            g_log->LPrintf(g_showScreen,"info:Server::server_thread server thread(%ul) quit ok\n",get_thread_id());
            return 0;		
        }

        int stop()
        {
            m_running = false;
            g_log->LPrintf(g_showScreen,"info:Server::stop server stop ok\n");
            return 0;
        }

        static int setOvertime(CP_SOCKET_T in_socket, int sendTimeout, int receiveTimeout)
        {
#ifdef _WIN32_ENV_
            setsockopt(in_socket, SOL_SOCKET, SO_RCVTIMEO,(var_1*)&receiveTimeout, sizeof(receiveTimeout));
            setsockopt(in_socket, SOL_SOCKET, SO_SNDTIMEO,(var_1*)&sendTimeout, sizeof(sendTimeout));
#else
            struct timeval over_time;
            over_time.tv_sec = receiveTimeout / 1000;
            over_time.tv_usec = (receiveTimeout % 1000) * 1000;
            setsockopt(in_socket, SOL_SOCKET, SO_RCVTIMEO,(var_1*)&over_time, sizeof(over_time));

            over_time.tv_sec = sendTimeout / 1000;
            over_time.tv_usec = (sendTimeout % 1000) * 1000;
            setsockopt(in_socket, SOL_SOCKET, SO_SNDTIMEO,(var_1*)&over_time, sizeof(over_time));

            int reuse = 1;
            setsockopt(in_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));
#endif
            return 0;
        }
        //FUNC  根据socket，得到对方IP地址
        static  const char *getIPStr(CP_SOCKET_T sock)
        {
            struct sockaddr cIP;
            socklen_t iIPLen = sizeof(struct sockaddr);

            if (getpeername(sock, &cIP, &iIPLen) != 0)
            {
                return NULL;
            }

            return inet_ntoa(((struct sockaddr_in*)&cIP)->sin_addr);
        }
};

#endif

