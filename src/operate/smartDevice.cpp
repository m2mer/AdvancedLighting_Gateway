/*
 Description: smart device
 
 Author: Xuesong
 
 Date: 2018-04-25
 
 */

#include <ArduinoJson.h>

#include "smartDevice.h"




smartDevice::smartDevice()
{
    init();
}

smartDevice::smartDevice(byte *mac) 
{
    init();
    memcpy(_mac, mac, 6);
}

void smartDevice::setDeviceManipulator(deviceManipulation *deviceMp)
{
    _deviceMp = deviceMp;
}

void smartDevice::setMAC(byte *mac)
{
    memcpy(_mac, mac, 6);
}

void smartDevice::setDeviceType(uint8_t firstType, uint8_t secondType)
{
    _type.firstType = firstType;
    _type.secondType = secondType;
}


void smartDevice::setDeviceUUID(const char* uuid)
{
    _UUID = String(uuid);
}

void smartDevice::setUserId(const char* userId)
{
    _userId = String(userId);
}

void smartDevice::getMacAddress(byte *mac)
{
    memcpy(mac, _mac, 6);
}

/*
 * send Heartbeat every second
 * */
void smartDevice::heartbeat()
{
    uint8 mac[6];
    char mac_str[12] = {0};
    char msg[256] = {0};
    uint32_t now = millis();
    if((now - _lastHeartbeat) < 1000)
        return;
    
    _lastHeartbeat = now;

    getMacAddress(mac);
    sprintf(msg, "{\"MAC\":\"");
    sprintf(mac_str, "%0x%0x%0x%0x%0x%0x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    strcat(msg, mac_str);
    strcat(msg, "\"}");

    DEBUG_DEVICE.printf("\nbegin to pub %s\n", msg);
    _deviceMp->mqttPublish(getMQTTtopic(PUB_TOPIC_DEVICE_HEARBEAT), msg);
}

void smartDevice::deviceRegister() 
{
    uint8 mac[6];
    char mac_str[13] = {0};
    char firstType[3] = {0};
    char secondType[3] = {0};
    char msg[256] = {0};

    /* subscribe registration_notify first */
    _deviceMp->mqttSubscribe(_topicRegNoti);

    getMacAddress(mac);
    DEBUG_DEVICE.printf("get mac %0x%0x%0x%0x%0x%0x\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    itoa(this->_type.firstType, firstType, 10);
    itoa(this->_type.secondType, secondType, 10);
    sprintf(mac_str, "%0x%0x%0x%0x%0x%0x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    strcat(msg, "{\"firstType\":\"");
    strcat(msg, firstType);
    strcat(msg,"\",\"secondType\":\"");
    strcat(msg, secondType);
    strcat(msg, "\",\"vendor\":\"AISmart\",\"MAC\":\"");
    strcat(msg, mac_str);
    strcat(msg, "\"}");
    _deviceMp->mqttPublish(getMQTTtopic(PUB_TOPIC_DEVICE_REGISTER), msg);
    DEBUG_DEVICE.printf("pub device register %s\n", msg);
}

void smartDevice::receiveMQTTmsg(char* topic, byte* payload, unsigned int length)
{
    byte valid[256] = {0};
    memcpy(valid, payload, length);
    DEBUG_DEVICE.printf("%s, topic %s, payload %s, len %d\n", __FUNCTION__, topic, valid, length);

    if(strcmp(topic, getMQTTtopic(SUB_TOPIC_REGISTER_NOTIFY)) == 0)
        registrationNotify(valid, length);
    else if(strcmp(topic, getMQTTtopic(SUB_TOPIC_DEVICE_OPERATE)) == 0)
        operateDevice(valid, length);
    else if(strcmp(topic, getMQTTtopic(SUB_TOPIC_GET_STATUS)) == 0)
        getOverallStatus(valid, length);
    else if(strcmp(topic, getMQTTtopic(SUB_TOPIC_GET_GROUP_STATUS)) == 0)
        getGroupStatus(valid, length);
}


void smartDevice::init()
{
    memset(_topicRegNoti, 0, 64);
    memset(_topicDevOp, 0, 64);
    memset(_topicGetSt, 0, 64);
    memset(_topicGetGrpSt, 0, 64);
    memset(_topicStUpd, 0, 64);
    memset(_topicOvaSt, 0, 64);
    memset(_topicGrpSt, 0, 64); 
    memset(_topicHeartbeat, 0, 64);
    memset(_topicRegister, 0, 64);     

    memcpy(_topicRegister, "device/device_register", 64);
    memcpy(_topicRegNoti, "device/registration_notify", 64);

    _lastHeartbeat = 0;              
}

void smartDevice::setMQTTDynTopic()
{
    /* sub topics */    
    memcpy(_topicDevOp, "device/device_operate", 64);
    memcpy(_topicGetSt, "device/get_status", 64);
    memcpy(_topicGetGrpSt, "device/get_group_status", 64); 

    /* pub topics */
    memcpy(_topicStUpd, "device/status_update", 64);
    memcpy(_topicOvaSt, "device/status_reply", 64);           
    memcpy(_topicGrpSt, "device/status_group", 64);
    memcpy(_topicHeartbeat, "device/heartbeat", 64);
    memcpy(_topicRstFactory, "device/reset_factory", 64);          
}

char* smartDevice::getMQTTtopic(DEVICE_MQTT_TOPIC topic)
{
    switch(topic)
    {
        case SUB_TOPIC_REGISTER_NOTIFY:
            return _topicRegNoti;
        case SUB_TOPIC_DEVICE_OPERATE:
            return _topicDevOp;
        case SUB_TOPIC_GET_STATUS:
            return _topicGetSt;
        case SUB_TOPIC_GET_GROUP_STATUS:
            return _topicGetGrpSt;
        case PUB_TOPIC_STATUS_UPDATE:
            return _topicStUpd;      
        case PUB_TOPIC_OVERALL_STATUS:
            return _topicOvaSt;
        case PUB_TOPIC_GROUP_STATUS:
            return _topicGrpSt;
        case PUB_TOPIC_DEVICE_REGISTER:
            return _topicRegister;
        case PUB_TOPIC_DEVICE_HEARBEAT:
            return _topicHeartbeat;
        case PUB_TOPIC_RESET_FACTORY:
            return _topicRstFactory;
    }
}

int smartDevice::registrationNotify(byte* payload, unsigned int length)
{
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& data = jsonBuffer.parse(payload);
    if(!data.success()){
        return RET_ERROR;
    }

    const char* uuid = data["UUID"];
    const char* userId = data["userId"];
    DEBUG_DEVICE.printf("%s, UUID %s, userId %s\n", __FUNCTION__, uuid, userId);

    setUserId(userId);
    setMQTTDynTopic();

    _deviceMp->mqttUnsubscribe(_topicRegNoti);

    _deviceMp->mqttSubscribe(_topicGetSt);
    _deviceMp->mqttSubscribe(_topicDevOp);
    _deviceMp->mqttSubscribe(_topicGetGrpSt);

}