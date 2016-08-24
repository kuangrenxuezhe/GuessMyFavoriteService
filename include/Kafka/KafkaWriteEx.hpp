/*
 * Typically include path in a real application would be
 * #include <librdkafka/rdkafkacpp.h>
 */
#ifndef _KAFKAWRITEEX_H
#define _KAFKAWRITEEX_H

#include <stdio.h>
//#include <unistd.h>
#include "rdkafkacpp.h"

class CKafkeWriteEx
{
public:
	//111:00,33:33,333 
	inline int Connect( char *pHost,  int iPort,char *pTopic,int iPartition,int iOffset);
	inline int Send(char *pBut,int iLen, int iParttion=-1);
	inline  int Disconnect();
public:
	CKafkeWriteEx()
	{
		mp_conf=NULL;
	mp_tconf=NULL;
	m_iPartition=0;
	m_iOffset=0;
	};
	~CKafkeWriteEx(){};
public:

private:

	RdKafka::Conf *mp_conf;
	RdKafka::Conf *mp_tconf;
	long m_iPartition;
	long m_iOffset;
private:
	RdKafka::Producer *m_producer;	//Éú²úÕß
	RdKafka::Topic *mp_topic; //producer topic
public:

};


int CKafkeWriteEx::Connect(char *pHost,  int iPort,char *pTopic,int iPartition,int iOffset)
{
	std::string errstr;

	if(pTopic == NULL || pHost == NULL )
	{
		printf("--Error-- In producer_con null ponit!\n");
		return -1;
	}
	m_iPartition=iPartition;
	m_iOffset=iOffset;
	char szHostPort[128];
	sprintf(szHostPort,"%s:%d", pHost, iPort);
	std::string stBroker = szHostPort;
	RdKafka::Conf::ConfResult result;

	mp_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
	mp_tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
	if(NULL==mp_conf || NULL==mp_tconf)
	{
		printf("--Error-- is Empty{mp_conf=0x%X,m_tconf=0x%X} %s \n",mp_conf,mp_tconf);
		return -50;
	}

	result=mp_conf->set("metadata.broker.list", stBroker, errstr);
	if(result != RdKafka::Conf::CONF_OK)
	{
		printf("--Error-- %s \n","mp_conf->set(\"metadata.broker.list\"...)");
		return -100;
	}
	//mp_conf->set("dr_cb", &ex_dr_cb, errstr);


	m_producer = RdKafka::Producer::create(mp_conf, errstr);
	if (!m_producer) 
	{
    	printf("Failed to create producer: %s \n",errstr.c_str());
		return -1;
	}

	std::string stTopic = pTopic;
	mp_topic = RdKafka::Topic::create(m_producer, stTopic, mp_tconf, errstr);
    if (!mp_topic) 
	{
		printf( "Failed to create topic: %s \n",errstr.c_str());
		return -2;
	}
	return 0;
}

int CKafkeWriteEx::Send(char *pBuf,int iLen, int iParttion)
{
	if(-1==iParttion)
		iParttion=m_iPartition;
//printf("------iLen = %d ------- PSendBuf = \n%s\n", iLen, pBuf);
	RdKafka::ErrorCode resp = RdKafka::ERR_NO_ERROR;

	while (true)
	{
		resp = m_producer->produce(mp_topic, iParttion, RdKafka::Producer::RK_MSG_COPY /* Copy payload */, pBuf, iLen, NULL, NULL);
		if (resp == RdKafka::ERR_NO_ERROR)
			break;
		printf("--Error=%d-- %s \n", resp, "m_producer->produce(100)");
		cp_sleep(1);
	}

	int iE = 0;
	while (true)
	{
		iE = m_producer->poll(1000);
		if (0 == iE)
			break;
		printf("--Error=%d-- %s \n", iE, "m_producer->poll(100)");
		cp_sleep(1);
	}
	return 0;

	/*
	m_producer->rk_

	while (rd_kafka_outq_len(rk) > 0)

	usleep(50000);

	*/
	
}

int CKafkeWriteEx::Disconnect()
{
	while (m_producer->outq_len() > 0) 
	{
		printf( "kafka.write saiting for length %d \n", m_producer->outq_len() );
		m_producer->poll(1000);
		cp_sleep(4);
	}

	if(mp_topic != NULL)
	{
		delete mp_topic;
		mp_topic = NULL;
	}
	if(m_producer != NULL)
	{
		m_producer = NULL;
		delete m_producer;
	}
}

#endif
