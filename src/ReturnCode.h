#ifndef RETURN_CODE_H
#define RETURN_CODE_H

enum SRP_SERVER_CODE
{ 
    //RET_OK = 0, 
    RET_SYS_BASE = -10000 , 

	//request
    RET_SYS_PROTOCOL_ERROR          = -1, 

    RET_SYS_DATA_LENGTH_INVALID     = -2, 
    RET_SYS_DATA_LENGTH_TOBIG       = -3, 
    RET_SYS_TYPE_INVALID            = -4, 
    RET_SYS_TYPE_UNSUPPORT          = -5, 
    RET_SYS_NO_ENOUGH_DATA          = -6, 
    RET_SYS_UNKNOWN_ERROR           = -7,
    RET_SYS_SEND_ACK_DATA_FAILED    = -8, 
    RET_SYS_MONGO_UNUSEABLE         = -9,
	RET_ACK_DATA_TOLONG				= -10,
    
    //-3000 -3999 
    RET_SYS_NONE
}; 
#endif

