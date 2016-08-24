#include "PublicDefine.h"
#include "Config.h"
#include "Server.h"
#include "Task.h"
#include "WorkGroup.h"
#include "ModelWorker.h"

#define MAX_LEN 10485760
#define MAX_KEYID_LEN 1<<10

CDailyLog *g_log;
Config *g_config;
CMonitor *g_monitor;
Server g_server;
WorkGroup g_workGroup; 
CTimer *g_timerTask;
CKafkaTask *g_loadUser;
CUsers *g_users;

int monitorId;
int g_mainQuitFlag;
int g_showScreen;
int g_logLevel;
int g_quitFlag;

int init(const char * configFile);
int start();
CP_THREAD_T quitThread(void * arg);
void clear();
void help_process();


int main(int argc, char * argv[])
{
    char * configFile = "../config/config.ini";

    //parse command line
    for (int i=1; i<argc; i++)
    {
        if (argv[i][0]!='-') 
            break;

        if ((!strcasecmp (argv[i], "--config" ) || !strcasecmp (argv[i], "-c" )) && (i+1)<argc)
        {
            configFile = argv[++i];
        } 
        else if ((!strcasecmp (argv[i], "--help") || !strcasecmp (argv[i], "-h")) )
        {
            help_process();
            exit(EXIT_SUCCESS); 
        }
        else
        {
            break;
        }
    }
    printf("Config File:%s\n",configFile);

    //init
    int ret = init(configFile);
    die(ret,"main::init failed\n");

    //start main thread
    ret = start();
    die(ret,"main::main start() failed\n");

    g_log->LPrintf(g_showScreen,"main thread(%ld) start,server start ok\n",get_thread_id());	
    while( !g_mainQuitFlag )
    {
        if(g_monitor->UpdateStatus(monitorId,get_thread_id()))
            g_log->LPrintf(g_showScreen,"main::main UpdateStatus failed\n");
        cp_sleep(1000);			
    }

    g_log->LPrintf(g_showScreen, "main thread(%ld) quit success!\n",get_thread_id());

    clear();
    return EXIT_SUCCESS;
}

int init(const char * configFile)
{
    int ret;

    g_quitFlag = 0;
    g_mainQuitFlag = 0;

    g_config = new Config();
    die(NULL == g_config,"new Config object failed\n");
    ret = g_config->init(configFile);
    die(ret,"init config object failed,config file:%s\n",configFile);

    g_showScreen = g_config->log_show_screen;	
    g_logLevel = g_config->log_level;	
    g_log = new CDailyLog();
    die(NULL == g_log,"new CDailyLog failed\n");
    ret = g_log->Init(g_config->log_file_dir,g_config->log_file_prefix);
    die(ret,"g_log->Init failed,may be the log dir is not exist\n");

    g_monitor = new CMonitor();
    die(NULL == g_monitor,"new Monitor failed\n");
    ret = g_monitor->Startup(g_config->service_work_thread_count +5,g_config->monitor_port
            ,g_config->monitor_timeout,g_config->monitor_log_dir);
    die(ret,"main::init g_monitor.Startup failed,may be the monitor log dir not exist,please check\n");		
    monitorId=g_monitor->LogIn(get_thread_id(),"main thread");
    die(monitorId <0,"main::init g_monitor->LogIn failed,ret=%d\n",monitorId);	

    //init and start work threads
    ret = g_server.init(g_config->service_job_queue_max_len);
    die(ret,"main::init server.init failed,max queue len:%d, ret = %d\n", g_config->service_job_queue_max_len,ret);

    ret = g_workGroup.init(g_config->service_work_thread_count,g_config->req_data_body_max_len,
            g_config->mem_temp_buffer_len,g_config->ack_data_body_max_len);
    die(ret,"main::init g_workGroup.init failed,ret = %d\n",ret );

    g_users = new CUsers();
	die(NULL == g_users,"new CUsers object failed\n");
    ret = g_users->Init(g_config->anchors[0].ip, g_config->anchors[0].port);
    die(ret,"main::init g_users.init failed,ret = %d\n",ret );
    
	g_timerTask = new CTimer();
	die(NULL == g_timerTask,"new CTimer object failed\n");
    
    g_loadUser = new CKafkaTask();
    die(NULL == g_loadUser,"new CKafkaTask object failed\n");
    
	ret = g_loadUser->Init(g_config->kafka_file_dir, g_config->kafka_file_name, \
    g_config->kafkas[0].ip, g_config->kafkas[0].port, g_config->kafkas[0].dbName, \
    -100, load_kafka_userInfo_process);
    die(ret,"main::init g_loadUser.init failed,ret = %d\n",ret );
    
    g_timerTask->SetTimer(1, 1000, g_loadUser, CKafkaTask::Load);
    	
    return 0;
}

int start()
{
    int ret = g_server.start(g_config->service_port,g_config->service_send_timeout,g_config->service_receive_timeout);
    die(ret,"main::start g_server.start failed,ret = %d\n",ret);

    ret = g_workGroup.start(&g_server.m_queue);
    die(ret,"main::start g_workGroup.start failed,ret = %d\n",ret);

    ret = cp_create_thread(quitThread, NULL);
    die(ret,"main::start quit monitor thread start ok\n");
    return 0;
}

CP_THREAD_T quitThread(void * arg)
{
    g_log->LPrintf(g_showScreen,"main:: quitThread(%ld) start success\n",get_thread_id());

    while( !g_quitFlag ){
        cp_sleep(1000);
    }

    g_log->LPrintf(g_showScreen,"main:: quitThread begin quit\n",get_thread_id());

    g_server.stop();
    g_workGroup.stop(); 

    g_log->LPrintf(g_showScreen,"main:: quitThread(%ld) quit success\n",get_thread_id());
    exit(EXIT_SUCCESS);
    return 0;
}

void clear()
{
    if(g_monitor->LogOut(monitorId ,get_thread_id()))
    {
        g_log->LPrintf(g_showScreen,"main::clear g_monitor->LogOut failed,ret=%d\n",monitorId);	
    }
    if(NULL != g_monitor) delete g_monitor;
    if(NULL != g_config) delete g_config;
    if(NULL != g_log) delete g_log;
}

void help_process()
{
    fprintf(stdout,
            "Usage: Param [OPTIONS]\n"
            "\n"
            "Options are:\n"
            "-h, --help\t\tdisplay this help message\n"
            "-c, --config <file>\tread configuration from specified file\n"
            "proc [config_file]\nthe default config file is ./config\n"
           );
}

