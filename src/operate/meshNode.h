/*
 Description: mesh node
 
 Author: Xuesong
 
 Date: 2018-05-25
 
 */



#ifndef meshNode_H
#define meshNode_H


#include <Arduino.h>

#include "smartDevice.h"
#include "smartDevicePacket.h"


#define DEBUG_MESH Serial1

typedef struct {
    uint8_t mac[6];
    uint8_t registered;
    uint8_t reserved;
}MESH_NODE_META_INFO;

/*
 * this is to aggregate segments of overall_status message from mesh node 
*/

typedef struct {
	uint8_t sequence;	
    uint8_t segmentMap;
    MESH_NODE_OVERALL_STATUS status;
}OVERALL_STATUS_AGGREGATION;




class meshNode:public smartDevice
{

public:
    meshNode();      //for LinkedList.get() bug
    meshNode(uint16_t devAddr);
    meshNode(byte *mac, uint16_t devAddr);
    ~meshNode(){};

    uint16_t getDevAddr();
    boolean aggregateStatus(byte *buf, OVERALL_STATUS_AGGREGATION *stAgg);
    uint8_t getAggBuffId();
    void setAggBuffId(uint8_t id);
    void clearStatus();
    int checkStatusUpdateSeq(uint8_t sequence);
    void deviceRegister(uint8_t *gwMac);

private:
    uint16_t _devAddr;
    uint32_t _lastActive;
    //OVERALL_STATUS_AGGREGATION _stAgg;
    uint8_t _aggBuffId;
    uint8_t _stUpdSeq;

    void init();
};


#endif