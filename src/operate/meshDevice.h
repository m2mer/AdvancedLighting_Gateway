/*
 Description: mesh device
 
 Author: Xuesong
 
 Date: 2018-04-26
 
 */



#ifndef meshDevice_H
#define meshDevice_H


#include <Arduino.h>
#include <LinkedList.h>

#include "smartDevice.h"
#include "smartDevicePacket.h"


#define DEBUG_MESH Serial1



template <typename T>
class advLinkedList:public LinkedList<T> 
{

public:
    ListNode<T> * getNodePtr(int index);
};


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

    void setGatewayMAC(uint8_t *mac);
    uint16_t getDevAddr();
    boolean aggregateStatus(byte *buf, OVERALL_STATUS_AGGREGATION *stAgg);
    void clearStatus();
    int checkStatusUpdateSeq(uint8_t sequence);
    void deviceRegister();

private:
    uint16_t _devAddr;
    uint8_t _gwMAC[6];    //gateway mac
    uint32_t _lastActive;
    OVERALL_STATUS_AGGREGATION _stAgg;
    uint8_t _stUpdSeq;

    void init();
};


class meshAgent:public smartDevice
{

public:
    meshAgent();
    meshAgent(byte *mac);
    ~meshAgent(){_meshNodeList.clear();};

    boolean isMeshNodeExist(byte *mac);
    int addMeshNode(byte *mac, uint16_t devAddr);
    int delMeshNode(byte *mac);
    int getMeshNodeDevAddr(byte *mac, uint16_t *devAddr);
    int getMeshNodeMAC(uint16_t *devAddr, byte *mac);

    void deviceRegister();
    
    /* handle mesh message */
    void receiveUARTmsg(byte *buf, int len);

private:

    uint8_t _meshMAC[6];
    //LinkedList<meshNode> _meshNodeList;
    advLinkedList<meshNode> _meshNodeList;
    //meshNode* _meshNodePool[256];    // alternative method to maintain mesh nodes

    /* handle mqtt message */
    int operateDevice(byte* payload, unsigned int length);
    int getOverallStatus(byte* payload, unsigned int length);
    int getGroupStatus(byte* payload, unsigned int length);

    /* handle message from peer uart */    
    void recvStatusUpdate(uint16_t nodeAddr, byte *buf);
    void recvOverallStatus(uint16_t nodeAddr, byte *buf);
    void recvGroupStatus(byte *buf);
    void recvResetFactory(uint16_t nodeAddr, byte *buf);
    void recvPairedNotify(uint16_t nodeAddr, byte *buf);

    int _atoi(char a);
    void _getMeshCommandBinary(const char *buf, byte *bin);
    void _packageMeshAgentMsg(char *buf, int len, char *msg);

};




#endif