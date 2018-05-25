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
    /* down direction */
    SUB_TOPIC_REGISTER_NOTIFY = 1,
    SUB_TOPIC_DEVICE_OPERATE = 2,
    SUB_TOPIC_GET_STATUS = 3,
    SUB_TOPIC_GET_GROUP_STATUS = 4,
    SUB_TOPIC_APP_NOTIFY = 5,
    SUB_TOPIC_DEVICE_DELETE = 6,  
    /* up direction */  
    PUB_TOPIC_STATUS_UPDATE = 7,
    PUB_TOPIC_OVERALL_STATUS = 8,
    PUB_TOPIC_GROUP_STATUS = 9,
    PUB_TOPIC_DEVICE_REGISTER = 10,
    PUB_TOPIC_STATE_NOTIFY = 11,
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
    virtual int hardwareReset(){};
    char* getMQTTtopic(DEVICE_MQTT_TOPIC topic);
    void setMQTTDynTopic();

    /* handle mqtt message */
    void receiveMQTTmsg(char* topic, byte* payload, unsigned int length);

    /* handle uart message */
    virtual void receiveUARTmsg(byte *buf, int len){};

protected:
    deviceManipulation *_deviceMp;
    char _topicRegNoti[64];
    char _topicDevOp[64];
    char _topicGetSt[64];
    char _topicGetGrpSt[64];
    char _topicAppNoti[64];
    char _topicDeviceDel[64];
    char _topicStUpd[64];
    char _topicOvaSt[64];
    char _topicGrpSt[64];
    char _topicRegister[64];
    char _topicStateNoti[64];
    DEVICE_TYPE _type;
    byte _mac[6];
    
private:
    String _userId;
    String _UUID;
    uint32_t _lastHeartbeat;
    uint32_t _hbIntvlMs;

    void init();
    int registrationNotify(byte* payload, unsigned int length);
    int appNotify(byte* payload, unsigned int length);

    virtual int operateDevice(byte* payload, unsigned int length){};
    virtual int getOverallStatus(byte* payload, unsigned int length){};
    virtual int getGroupStatus(byte* payload, unsigned int length){};
    virtual int deviceDelete(byte* payload, unsigned int length){};
};






#endif