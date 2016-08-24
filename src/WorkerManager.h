#ifndef WORKER_MANAGER_H
#define WORKER_MANAGER_H

#include <map>
#include "PublicDefine.h"
#include "Worker.h"
#include "SysWorker.h"
#include "ModelWorker.h"
using namespace std;

#define insert_worker(key,value) \
		insertRet = m_workerMap.insert( make_pair(key, value) );\
		die(false == insertRet.second,"worker with key %d have exist\n",key );

class WorkerManager{
public:
	map<int,worker> m_workerMap;
	int init(){
		//register deal method
		pair<std::map<int,worker>::iterator,bool> insertRet;
		
		insert_worker(CMD_SERVER_QUIT, server_shutdown);
		insert_worker(CMD_SERVER_ALIVE, server_alive);
        
        return 0;
	};

	worker getWorker(int id)
	{
		map<int,worker>::iterator iter;
		iter= m_workerMap.find(id);
		if(iter == m_workerMap.end())
		{
			return NULL;
		}
		return iter->second; 
	};
};

#endif
