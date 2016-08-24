#ifndef  KAFKA_EX_b7287176ef5542bcbd570672f0f59d53
#define  KAFKA_EX_b7287176ef5542bcbd570672f0f59d53


/*
 * Typically include path in a real application would be
 * #include <librdkafka/rdkafkacpp.h>
 */
#include "DiskVariationEx.hpp"
#include "rdkafkacpp.h"

//static void sigterm(int sig)
//{
//    run = false;
//}


//class ExampleDeliveryReportCb : public RdKafka::DeliveryReportCb
//{
//public:
//	void dr_cb(RdKafka::Message &message)
//	{
//		std::cout << "Message delivery for (" << message.len() << " bytes): " <<
//			message.errstr() << std::endl;
//	}
//};
/*
DESC 定义 时间接送器
*/
// class ExampleEventCb : public RdKafka::EventCb
// {
// public:
//     void event_cb(RdKafka::Event &event)
//     {
//         switch(event.type())
//         {
//             case RdKafka::Event::EVENT_ERROR:
//                 //std::cerr << "ERROR (" << RdKafka::err2str(event.err()) << "): " <<
//                         //  event.str() << std::endl;
// 
//                 if(event.err() == RdKafka::ERR__ALL_BROKERS_DOWN)
// 				{
// 					;//所有连接都断开了
// 				}
//                 break;
// 
//             case RdKafka::Event::EVENT_STATS:
//                 //std::cerr << "\"STATS\": " << event.str() << std::endl;
//                 break;
// 
//             case RdKafka::Event::EVENT_LOG:
//                 fprintf(stderr, "LOG-%i-%s: %s\n",
//                         event.severity(), event.fac().c_str(), event.str().c_str());
//                 break;
// 
//             default:
//               //  std::cerr << "EVENT " << event.type() <<
//                   //        " (" << RdKafka::err2str(event.err()) << "): " <<
//                    //       event.str() << std::endl;
//                 break;
//         }
//     }
// };

/* Use of this partitioner is pretty pointless since no key is provided
 * in the produce() call. */
// class MyHashPartitionerCb : public RdKafka::PartitionerCb
// {
// public:
//     int32_t partitioner_cb(const RdKafka::Topic *topic, const std::string *key,
//                            int32_t partition_cnt, void *msg_opaque)
//     {
//         return djb_hash(key->c_str(), key->size()) % partition_cnt;
//     }
// private:
// 
//     static inline unsigned int djb_hash(const char *str, size_t len)
//     {
//         unsigned int hash = 5381;
// 
//         for(size_t i = 0 ; i < len ; i++)
//             hash = ((hash << 5) + hash) + str[i];
// 
//         return hash;
//     }
// };

// void msg_consume(RdKafka::Message* message, void* opaque)
// {
//     switch(message->err())
//     {
//         case RdKafka::ERR__TIMED_OUT:
//             break;
// 
//         case RdKafka::ERR_NO_ERROR:
//             /* Real message */
//             //std::cout << "Read msg at offset " << message->offset() << std::endl;
// 
//             if(message->key())
//             {
//                // std::cout << "Key: " << *message->key() << std::endl;
//             }
// 
//             printf("%.*s\n",
//                    static_cast<int>(message->len()),
//                    static_cast<const char *>(message->payload()));
//             break;
//         case RdKafka::ERR__PARTITION_EOF:
//             /* Last message */
// 
//             break;
//         default:
//             /* Errors */
//             //std::cerr << "Consume failed: " << message->errstr() << std::endl;
// 			break;
//     }
// }
// 
// 
// class ExampleConsumeCb : public RdKafka::ConsumeCb
// {
// public:
//     void consume_cb(RdKafka::Message &msg, void *opaque)
//     {
//         msg_consume(&msg, opaque);
//     }
// };
#define  _ARG_INPUT_ 
#define  _ARG_OUTPUT_
/*
//错误码 -5000 数数据被截断。请用的GetLarge 获取最后一条的数据

*/


class CKafkeEx
{
public:
	//
	inline int  Init(const char *pPath,const char *pName);
	/*
	DESC 连接Kafka服务器
	char *pHost [in] 服务器
	int iPort	[in] 端口号
	char *pTopic	[in] TOPIC
	char *pPart	[in]  partition
	long iPos	[in]   position   -100 起始位置  -200 末尾位置
	返回值：0 成功,  其它值 错误
	*/  
	inline int Connect(char *pHost _ARG_INPUT_/*Kafka hsot*/,int iPort _ARG_INPUT_/*Kafka port*/,
		char *pTopic _ARG_INPUT_ /*Kafka topic*/,char *pPart _ARG_INPUT_/*Kafka topic*/,long iPos _ARG_INPUT_ /*Kafka topic*/);

	inline int Connect(char *pHost _ARG_INPUT_/*Kafka hsot*/,int iPort _ARG_INPUT_/*Kafka port*/,
		char *pTopic _ARG_INPUT_ /*Kafka topic*/,char *pPart _ARG_INPUT_/*Kafka topic*/,long iPos _ARG_INPUT_ /*Kafka topic*/, char *pHost2 _ARG_INPUT_,int iPort2);
	/*
	DESC 断开Kafka服务器
	返回值：0 成功,  其它值 错误
	*/  
	inline int Disconnect();
	inline int  GetLarge(int iSizeData,char *pData,long *piLenData);
	/*
	DESC 获取一条消息
	int iSize [in]  消息缓存大小
	char *pBuff [in]  消息缓存
	int *iLen [out]  消息缓存长度
	,long *piOffset [out] 消息编号
	返回值：0 成功,  其它值 错误
	*/ 
	inline int GetMessage(int iSize _ARG_INPUT_ /*message buffer size*/,char *pBuff _ARG_OUTPUT_ /*message buffer */,
		int *iLen _ARG_OUTPUT_ /*return message length*/,long *piOffset _ARG_OUTPUT_ /*return message position*/);
public:
	CDiskVariation m_dkLage;//程序状态 定时落地文件
public:
	CKafkeEx()
	{
		m_conf=NULL;
		m_tconf=NULL;
		m_partition=0;
		m_start_offset=0;

	};
	~CKafkeEx()
	{

	};
public:

private:
	RdKafka::Conf *m_conf;
	RdKafka::Conf *m_tconf;
	int32_t m_partition;
	int64_t m_start_offset;
private:
	//ExampleEventCb ex_event_cb; //事件处理器-暂
	RdKafka::Consumer *m_consumer; //消费者
	RdKafka::Topic *m_topic; //TOPIC 
	//MyHashPartitionerCb hash_partitioner;
public:
	
	//ExampleConsumeCb ex_consume_cb;//消息者 回调

};


//mdy0616   初始化
//初始化数
int  CKafkeEx::Init(const char *pPath,const char *pName)
{
	if(NULL==pPath || NULL==pName)
		DiskVariationRoot(pPath);
	m_dkLage.Init(pName,20971520);//20971520=20M
	return 0;
}
//mdy0616 加载最后一条大数据
/*
desc  获取最近一次的超长数据 
const int iSizeData	[IN]  缓存大小
char *pData	[out]   数据缓存大小
int *piLenData	[out]	数据长度
*/
int  CKafkeEx::GetLarge(int iSizeData,char *pData,long *piLenData)
{
	return m_dkLage.Load(iSizeData,pData,piLenData);
}
/*

RdKafka::Topic::OFFSET_STORED   来使用存储的offset，
RdKafka::Topic::OFFSET_BEGINNING，从partition消息队列的开始进行consume；
RdKafka::Topic::OFFSET_END：从partition中的将要produce的下一条信息开始（忽略即当前所有的消息）。
*/
int CKafkeEx::Connect(char *pHost,int iPort,char *pTopic,char *pPart,long iPos)
{
	
	//m_partition = RdKafka::Topic::PARTITION_UA;
	m_partition=atoi(pPart);
	//
	if(-100==iPos)
		m_start_offset=RdKafka::Topic::OFFSET_BEGINNING;
	else if(-200==iPos) 
		m_start_offset=RdKafka::Topic::OFFSET_END;
	else if(-300==iPos) 
		m_start_offset=RdKafka::Topic::OFFSET_STORED;
	else if(iPos>=0)
		m_start_offset=iPos;
	else
		return -20;

	//m_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
	m_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
	m_tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
	if(NULL==m_conf || NULL==m_tconf)
	{
		printf("--Error-- is Empty{m_conf=0x%X,m_tconf=0x%X} %s \n",m_conf,m_tconf);
		return -50;
	}
	//设置配置属性-服务地址列表
	std::string stError;
	std::string stTopic;
	std::string stBroker;
	stTopic=pTopic;
	char szHostPort[512]={0};
	//sprintf(szHostPort,"%s",pHost);
	sprintf(szHostPort,"%s:%d",pHost,iPort);
	stBroker=szHostPort;
	RdKafka::Conf::ConfResult result;
	result=m_conf->set("metadata.broker.list", stBroker, stError);
	if(result != RdKafka::Conf::CONF_OK)
	{
		printf("--Error-- %s \n","m_conf->set(\"metadata.broker.list\"...)");
		return -100;
	}

// 	设置配置属性-事件处理器
// 		result=m_conf->set("event_cb", &ex_event_cb,stError);
// 		if(result != RdKafka::Conf::CONF_OK)
// 		{
// 			printf("--Error-- %s \n","m_conf->set(\"event_cb\"...)");
// 			return -200;
// 		}
	// 创建消费者
	// Create consumer using accumulated global configuration.
	m_consumer = RdKafka::Consumer::create(m_conf, stError);
	if(NULL==m_consumer)
	{
		//std::cerr << "--Error--  create consumer: " << stError << std::endl;
		return -200;
	}
	//std::cout << "% Created consumer " << m_consumer->name() << std::endl;
	// Create topic handle.
	m_topic = RdKafka::Topic::create(m_consumer,stTopic ,m_tconf, stError);
	if(!m_topic)
	{
		//std::cerr << "--Error--  create m_topic: " << stError << std::endl;
		return -300;
	}
	// Start consumer for topic+partition at start offset
	 RdKafka::ErrorCode resp = m_consumer->start(m_topic, m_partition, m_start_offset);
	if(resp != RdKafka::ERR_NO_ERROR)
	{
		//std::cerr << "--Error--  start consumer: " <<
		//	RdKafka::err2str(resp) << std::endl;
		return -400;
	}
	return 0;
}

int CKafkeEx::Connect(char *pHost,int iPort,char *pTopic,char *pPart,long iPos, char *pHost2,int iPort2)
{
	
	//m_partition = RdKafka::Topic::PARTITION_UA;
	m_partition=atoi(pPart);
	//
	if(-100==iPos)
		m_start_offset=RdKafka::Topic::OFFSET_BEGINNING;
	else if(-200==iPos) 
		m_start_offset=RdKafka::Topic::OFFSET_END;
	else if(-300==iPos) 
		m_start_offset=RdKafka::Topic::OFFSET_STORED;
	else if(iPos>=0)
		m_start_offset=iPos;
	else
		return -20;

	//m_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
	m_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
	m_tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
	if(NULL==m_conf || NULL==m_tconf)
	{
		printf("--Error-- is Empty{m_conf=0x%X,m_tconf=0x%X} %s \n",m_conf,m_tconf);
		return -50;
	}
	//设置配置属性-服务地址列表
	std::string stError;
	std::string stTopic;
	std::string stBroker;
	stTopic=pTopic;
	char szHostPort[512]={0};
	//sprintf(szHostPort,"%s",pHost);
	sprintf(szHostPort,"%s:%d, %s:%d",pHost,iPort,pHost2,iPort2);
	stBroker=szHostPort;
	RdKafka::Conf::ConfResult result;
	result=m_conf->set("metadata.broker.list", stBroker, stError);
	if(result != RdKafka::Conf::CONF_OK)
	{
		printf("--Error-- %s \n","m_conf->set(\"metadata.broker.list\"...)");
		return -100;
	}

// 	设置配置属性-事件处理器
// 		result=m_conf->set("event_cb", &ex_event_cb,stError);
// 		if(result != RdKafka::Conf::CONF_OK)
// 		{
// 			printf("--Error-- %s \n","m_conf->set(\"event_cb\"...)");
// 			return -200;
// 		}
	// 创建消费者
	// Create consumer using accumulated global configuration.
	m_consumer = RdKafka::Consumer::create(m_conf, stError);
	if(NULL==m_consumer)
	{
		//std::cerr << "--Error--  create consumer: " << stError << std::endl;
		return -200;
	}
	//std::cout << "% Created consumer " << m_consumer->name() << std::endl;
	// Create topic handle.
	m_topic = RdKafka::Topic::create(m_consumer,stTopic ,m_tconf, stError);
	if(!m_topic)
	{
		//std::cerr << "--Error--  create m_topic: " << stError << std::endl;
		return -300;
	}
	// Start consumer for topic+partition at start offset
	 RdKafka::ErrorCode resp = m_consumer->start(m_topic, m_partition, m_start_offset);
	if(resp != RdKafka::ERR_NO_ERROR)
	{
		//std::cerr << "--Error--  start consumer: " <<
		//	RdKafka::err2str(resp) << std::endl;
		return -400;
	}
	return 0;
}

int CKafkeEx::Disconnect()
{
	if(NULL!=m_consumer)
	{
		if(NULL!=m_topic)
		{
			m_consumer->stop(m_topic, m_partition);
			m_consumer->poll(1000);
			delete m_topic;
			m_topic=NULL;
		}
		// Stop consumer
		delete m_consumer;
		m_consumer=NULL;
		RdKafka::wait_destroyed(5000);
	}
	return 0;

}
/*
DESC 接收消息
//错误码 -5000 数数据被截断。请用的GetLarge 获取最后一天的数据
const int icSize	[IN]  缓存大小
char *pBuff	[out]   数据缓存大小
int *piLen	[out]	数据长度
long *piOffset	[out]  kafka偏移量
*/
//

int CKafkeEx::GetMessage(const int icSize,char *pBuff,int *piLen,long *piOffset)
{
	if(NULL==m_consumer || NULL==m_topic)
	{
		printf("--Error.Argument {0x%X,0x%X}",m_consumer,m_topic);
		return -100;
	}
	int iError=0;
	//bool bUseCall=false;
	//if(bUseCall)
	//{
	//	m_consumer->consume_callback(m_topic, m_partition, 1000,
	//		&ex_consume_cb, &use_ccb);
	//}
	RdKafka::Message *pMsg = m_consumer->consume(m_topic, m_partition, 5000/*最长等待时间*/);
	switch(pMsg->err())
	{
	case RdKafka::ERR__TIMED_OUT:
		//printf("--message.timeout \n ");
		iError=1000;
		break;
	case RdKafka::ERR_NO_ERROR:
		/* Real message */
		//std::cout << "Read message at offset " << pMsg->offset() << std::endl;
		*piOffset=static_cast<const int >(pMsg->offset());
		//if(pMsg->key())
		//{
		//	//std::cout << "Key: " << *pMsg->key() << std::endl;
		//}
		{
			//printf("%.*s\n",
			//	static_cast<int>(pMsg->len()),
			//	static_cast<const char *>(pMsg->payload()));

			const int icSizeBuffer=icSize-2;
			int iLen=static_cast<int>(pMsg->len());
			const char *pData=static_cast<const char *>(pMsg->payload());
			//mdy0616
			if(iLen>icSizeBuffer)
			{
				//将数据追加到文件中
				m_dkLage.Save(pData,iLen);
				//返回错误代码并报错
				iLen=icSizeBuffer;
				iError=-5000;
			}
			memcpy(pBuff,pData,iLen);
			
			*piLen=iLen;
			//error -return -errror 
		}
		iError=0;
		break;
	case RdKafka::ERR__PARTITION_EOF:
		//printf("--message.partition eof \n ");
		iError=2000;
		break;
	default:
		printf("--message.error=%s \n ",pMsg->errstr().c_str());
		iError=pMsg->err();
	 break;
	}
	delete pMsg;
	pMsg=NULL;
	//error -  what is poll 
	m_consumer->poll(0);
	return iError;
}

#endif


