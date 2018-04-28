/*
 Description: smart device
 
 Author: Xuesong
 
 Date: 2018-04-25
 
 */

#ifndef smartDevice_H
#define smartDevice_H


#include <Arduino.h>
#include "../lib/LinkedList/LinkedList.h"

#include "deviceManipulation.h"
#include "smartDevicePacket.h"


#define DEBUG_DEVICE Serial1


typedef enum {
    SUB_TOPIC_REGISTER_NOTIFY = 1,
    SUB_TOPIC_DEVICE_OPERATE = 2,
    SUB_TOPIC_GET_STATUS = 3,
    SUB_TOPIC_GET_GROUP_STATUS = 4,
    PUB_TOPIC_STATUS_UPDATE = 5,
    PUB_TOPIC_OVERALL_STATUS = 6,
    PUB_TOPIC_GROUP_STATUS = 7,
    PUB_TOPIC_DEVICE_REGISTER = 8,
    PUB_TOPIC_DEVICE_HEARBEAT = 9,
}DEVICE_MQTT_TOPIC;


class deviceManipulation;
class smartDevice 
{

public:
    smartDevice();
    smartDevice(byte *mac);
    ~smartDevice(){};

    void setDeviceManipulator(deviceManipulation *deviceMp);
    void setMAC(byte *mac);
    void setDeviceType(uint8_t firstType, uint8_t secondType);
    void setDeviceUUID(const char* uuid);
    void setUserId(const char* userId);
    void getMacAddress(byte *mac);

    void heartbeat();
    void deviceRegister();
    char* getMQTTtopic(DEVICE_MQTT_TOPIC topic);
    void setMQTTtopic();

    /* handle mqtt message */
    void receiveMQTTmsg(char* topic, byte* payload, unsigned int length);

    /* handle uart message */
    virtual void receiveUARTmsg(byte *buf, int len){};

protected:
    deviceManipulation *_deviceMp;

private:
    byte _mac[6];
    String _userId;
    String _UUID;
    DEVICE_TYPE _type;
    char _topicRegNoti[64];
    char _topicDevOp[64];
    char _topicGetSt[64];
    char _topicGetGrpSt[64];
    char _topicStUpd[64];
    char _topicOvaSt[64];
    char _topicGrpSt[64];
    char _topicRegister[64];
    char _topicHeartbeat[64];
    uint32_t _lastHeartbeat;

    void init();

    virtual int operateDevice(byte* payload, unsigned int length){};
    virtual int getOverallStatus(byte* payload, unsigned int length){};
    virtual int getGroupStatus(byte* payload, unsigned int length){};
};






#endif