/*
 Description: mesh Agent
 
 Author: Xuesong
 
 Date: 2018-04-26
 
 */



#ifndef meshAgent_H
#define meshAgent_H


#include <Arduino.h>
#include <LinkedList.h>

#include "meshNode.h"



#define MESH_NODE_NUM_MAX     64
#define AGGREGATION_UNIT_NUM  8


typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t date;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
}MESH_AGENT_RTC;

typedef struct
{
    uint8_t networkConfiged;
    uint8_t registered;
}MESH_AGENT_METADATA;

typedef struct
{
    uint8_t mac[6];
    uint16_t deviceAddress;    
    uint8_t registered;
    uint8_t reserved;
}MESH_NODE_METADATA;


typedef struct
{
    uint16_t devAddr;
    uint32_t startTime;
    OVERALL_STATUS_AGGREGATION agg;
}AGGREGATION_UNIT;



template <typename T>
class advLinkedList:public LinkedList<T> 
{

public:
    ListNode<T> * getNodePtr(int index);
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
    
    /* handle mesh message */
    void receiveUARTmsg(byte *buf, int len);

    void setNetworkConfiged(int flag);
    int getNetworkConfged();
    void notifyMeshAgent(int networkConfiged);
    void deviceRegister();
    int hardwareReset();

    void resumeMetaData();
    void storeMetaData();
    void metaInfoManage();

    void badNodeManage();
    void aggBuffManage();
    void loop();

private:

    MESH_AGENT_RTC _rtc;
    uint8_t _meshMAC[6];
    //LinkedList<meshNode> _meshNodeList;
    advLinkedList<meshNode> _meshNodeList;
    //meshNode* _meshNodePool[256];    // alternative method to maintain mesh nodes
    AGGREGATION_UNIT _aggBuff[AGGREGATION_UNIT_NUM];      //8 is ok
    uint8_t _curAggBuffId;

    uint8_t _networkConfiged;
    uint32_t _metaInfoTime;
    uint32_t _aggBuffMgTime;
    uint32_t _badNodeTime;

    void _init();

    /* handle mqtt message */
    int operateDevice(byte* payload, unsigned int length);
    int getOverallStatus(byte* payload, unsigned int length);
    int getGroupStatus(byte* payload, unsigned int length);
    int deviceDelete(byte* payload, unsigned int length);
    int registrationNotify(byte* payload, unsigned int length);

    /* handle message from peer uart */    
    void recvStatusUpdate(uint16_t nodeAddr, byte *buf);
    void recvOverallStatus(uint16_t nodeAddr, byte *buf);
    void recvGroupStatus(byte *buf);
    void recvResetFactory(uint16_t nodeAddr, byte *buf);
    void recvPairedNotify(uint16_t nodeAddr, byte *buf);

    /* */
    void syncDeviceTime(uint16_t devAddr);
    int _atoi(char a);
    void _getMeshCommandBinary(const char *buf, byte *bin);
    void _packageMeshAgentMsg(char *buf, int len, char *msg);
    boolean aggregateStatus(meshNode *node, int buffId, byte *buf, OVERALL_STATUS_AGGREGATION *stAgg);
    void _checkRepeatNode(byte *mac, uint16_t devAddr);
};




#endif