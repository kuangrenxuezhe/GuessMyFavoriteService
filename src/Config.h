#ifndef CONFIG_H 
#define CONFIG_H
#include "UH_Define.h"
#include "UC_ReadConfigFile.h"

#define get_value(field,value) \
	ret = m_config.GetFieldValue(field,value);\
	die( ret || value < 0 ,"get config item faild:ret = %d,field=%s\r\n",ret,field);\
	printf("%s:%d\n",field,(int)value);

#define get_value_str(field,value) \
	ret = m_config.GetFieldValue(field,value);\
	if(ret){\
		printf("warnning:get config item faild:ret = %d,field=%s\r\n",ret,field);\
		*value = '\0';\
	}\
	printf("%s:%s\n",field,value);

class Config
{
public:
	Config(){};
	~Config(){};

	int init(const char * file)
	{
		assert(NULL != file );
		char tempBuf[CONFIG_MAX_ITEM_LEN];
		int i;
		int ret =m_config.InitConfigFile(file);
		if( 0 != ret )
			return -1;
		
		//request
		get_value("request_protocol_field_length",req_protocol_field_len);
		get_value_str("request_protocol",req_protocol);
		get_value("request_data_length_field_length",req_data_length_field_len);
		get_value("request_type_field_length",req_type_field_len);
		get_value("request_data_body_max_length",req_data_body_max_len);
			
			//ack
		get_value("ack_data_length_field_length",ack_data_length_field_len);
		get_value("ack_data_body_max_length", ack_data_body_max_len);

		get_value("memory_temp_buffer_length",mem_temp_buffer_len);

		get_value("service_port",service_port); 
		get_value("service_job_queue_max_length",service_job_queue_max_len);
		get_value("service_send_timeout",service_send_timeout);
		get_value("service_receive_timeout",service_receive_timeout);
		get_value("service_work_thread_count",service_work_thread_count);

		get_value_str("kafka_file_dir",kafka_file_dir);
		get_value_str("kafka_file_name",kafka_file_name);

		get_value("user_kafka_count", kafka_count);
		die( kafka_count <0,"kafka_count invalid: kafka_count=%d\n"
			,kafka_count);
		
		kafkas = (kafka_info *) malloc( sizeof(kafka_info) * kafka_count );
		die(NULL == kafkas,"new entitys failed\n");		
		for(i = 1; i <= kafka_count;i++ )
		{
			snprintf(tempBuf,CONFIG_MAX_ITEM_LEN,"user_kafka_ip_%02d",i);
			die(ret,"get %s failed,ret:%d\n",tempBuf,ret);
			//
			snprintf(tempBuf,CONFIG_MAX_ITEM_LEN,"user_kafka_ip_%02d",i);
			get_value_str(tempBuf,kafkas[i-1].ip);
			//
			snprintf(tempBuf,CONFIG_MAX_ITEM_LEN,"user_kafka_port_%02d",i);
            get_value(tempBuf,kafkas[i-1].port);

			snprintf(tempBuf,CONFIG_MAX_ITEM_LEN,"user_kafka_name_%02d",i);
            get_value_str(tempBuf, kafkas[i-1].dbName);
		}

		get_value("anchor_count", anchor_count);
		die( anchor_count <0,"anchor_count invalid: anchor_count=%d\n"
			,anchor_count);
		
		anchors = (anchor_info *) malloc( sizeof(anchor_info) * anchor_count );
		die(NULL == anchors,"new anchors failed\n");		
		for(i = 1; i <= anchor_count;i++ )
		{
			snprintf(tempBuf,CONFIG_MAX_ITEM_LEN,"anchor_ip_%02d",i);
			die(ret,"get %s failed,ret:%d\n",tempBuf,ret);
			//
			snprintf(tempBuf,CONFIG_MAX_ITEM_LEN,"anchor_ip_%02d",i);
			get_value_str(tempBuf,anchors[i-1].ip);
			//
			snprintf(tempBuf,CONFIG_MAX_ITEM_LEN,"anchor_port_%02d",i);
            get_value(tempBuf,anchors[i-1].port);
		}
		//0:run 1:debug; debug will print more info for debug
		get_value("log_level",log_level);
		//1:print 0:no
		get_value("log_show_screen",log_show_screen);
		get_value_str("log_file_dir",log_file_dir);
		get_value_str("log_file_prefix",log_file_prefix);

		get_value("monitor_port",monitor_port);
		get_value("monitor_timeout",monitor_timeout);
		get_value_str("monitor_log_dir",monitor_log_dir);

		return 0;
	}

	//request
	int req_protocol_field_len;
	char req_protocol[CONFIG_MAX_ITEM_LEN];
	int req_data_length_field_len;
	int req_type_field_len;
	int req_data_body_max_len;
	
	//ack
	int ack_data_length_field_len;
	int ack_data_body_max_len;

	int mem_temp_buffer_len;

	int service_port; 
	int service_job_queue_max_len;
	int service_send_timeout;//millisecond
	int service_receive_timeout;//millisecond
	int service_work_thread_count;

	char kafka_file_dir[CONFIG_MAX_ITEM_LEN];
	char kafka_file_name[CONFIG_MAX_ITEM_LEN];
	int kafka_count;
	kafka_info *kafkas;		

	int anchor_count;
	anchor_info *anchors;
	//0:run 1:debug; debug will print more info for debug
	int log_level;
    
	//1:print 0:no
	int log_show_screen;
	char log_file_dir[CONFIG_MAX_ITEM_LEN];
	char log_file_prefix[CONFIG_MAX_ITEM_LEN];
    
	int monitor_port;
	int monitor_timeout;
	char monitor_log_dir[CONFIG_MAX_ITEM_LEN];
private:
	UC_ReadConfigFile m_config;
};

#endif

