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

    void notifyMeshAgent(int networkConfiged);
    void deviceRegister();
    int hardwareReset();
    
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
    int deviceDelete(byte* payload, unsigned int length);

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