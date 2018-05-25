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

void smartDevice::init()
{
    memset(_topicRegNoti, 0, 64);
    memset(_topicDevOp, 0, 64);
    memset(_topicGetSt, 0, 64);
    memset(_topicGetGrpSt, 0, 64);
    memset(_topicAppNoti, 0, 64);
    memset(_topicDeviceDel, 0, 64);        
    memset(_topicStUpd, 0, 64);
    memset(_topicOvaSt, 0, 64);
    memset(_topicGrpSt, 0, 64); 
    memset(_topicStateNoti, 0, 64);
    memset(_topicRegister, 0, 64);     

    memcpy(_topicRegister, "device/device_register", 64);
    memcpy(_topicRegNoti, "device/registration_notify", 64);

    _lastHeartbeat = 0;     
    _hbIntvlMs = 1000;    //300*1000, 5min         
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

void smartDevice::setMQTTDynTopic()
{
    /* sub topics */    
    memcpy(_topicDevOp, "device/device_operate", 64);
    memcpy(_topicGetSt, "device/get_status", 64);
    memcpy(_topicGetGrpSt, "device/get_group_status", 64); 
    memcpy(_topicAppNoti, "device/app_notify", 64);
    memcpy(_topicDeviceDel, "device/device_delete", 64); 

    /* pub topics */
    memcpy(_topicStUpd, "device/status_update", 64);
    memcpy(_topicOvaSt, "device/status_reply", 64);           
    memcpy(_topicGrpSt, "device/status_group", 64);
    memcpy(_topicStateNoti, "device/state_notify", 64);          
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
        case SUB_TOPIC_APP_NOTIFY:
            return _topicAppNoti;
        case SUB_TOPIC_DEVICE_DELETE:
            return _topicDeviceDel;      
        case PUB_TOPIC_OVERALL_STATUS:
            return _topicOvaSt;
        case PUB_TOPIC_GROUP_STATUS:
            return _topicGrpSt;
        case PUB_TOPIC_DEVICE_REGISTER:
            return _topicRegister;
        case PUB_TOPIC_STATE_NOTIFY:
            return _topicStateNoti;
    }
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
    if((now - _lastHeartbeat) < _hbIntvlMs)
        return;
    
    _lastHeartbeat = now;

    getMacAddress(mac);
    sprintf(msg, "{\"UUID\":\"");
    sprintf(mac_str, "%0x%0x%0x%0x%0x%0x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    strcat(msg, mac_str);
    strcat(msg, "\",\"attribute\":\"heartbeat\"}");

    DEBUG_DEVICE.printf("\nbegin to pub %s\n", msg);
    _deviceMp->mqttPublish(getMQTTtopic(PUB_TOPIC_STATE_NOTIFY), msg);
}

void smartDevice::deviceRegister() 
{
    uint8 mac[6];
    char mac_str[13] = {0};
    char bssid[32] = {0};
    char type[9] = {0};
    char msg[256] = {0};

    /* subscribe registration_notify first */
    _deviceMp->mqttSubscribe(_topicRegNoti);
    _deviceMp->mqttClient->loop();

    getMacAddress(mac);
    WiFi.BSSIDstr().toCharArray(bssid, 32, 0);
    DEBUG_DEVICE.printf("get mac %0x%0x%0x%0x%0x%0x\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    DEBUG_DEVICE.printf("get bssid %s\n", bssid);

    sprintf(type, "%04x%04x", this->_type.firstType, this->_type.secondType);
    sprintf(mac_str, "%02x%02x%02x%02x%02x%02x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    strcat(msg, "{\"type\":\"");
    strcat(msg, type);
    strcat(msg, "\",\"vendor\":\"AISmart\",\"MAC\":\"");
    strcat(msg, mac_str);
    strcat(msg, "\",\"BSSID\":\"");
    strcat(msg, bssid);
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
    else if(strcmp(topic, getMQTTtopic(SUB_TOPIC_APP_NOTIFY)) == 0)
        appNotify(valid, length);    
    else if(strcmp(topic, getMQTTtopic(SUB_TOPIC_DEVICE_OPERATE)) == 0)
        operateDevice(valid, length);
    else if(strcmp(topic, getMQTTtopic(SUB_TOPIC_GET_STATUS)) == 0)
        getOverallStatus(valid, length);
    else if(strcmp(topic, getMQTTtopic(SUB_TOPIC_GET_GROUP_STATUS)) == 0)
        getGroupStatus(valid, length);
}

int smartDevice::registrationNotify(byte* payload, unsigned int length)
{
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& data = jsonBuffer.parse(payload);
    if(!data.success()){
        return RET_ERROR;
    }

    const char* uuid = data["deviceId"];
    const char* userId = data["userId"];
    DEBUG_DEVICE.printf("%s, deviceId %s, userId %s\n", __FUNCTION__, uuid, userId);

    setUserId(userId);
    setMQTTDynTopic();

    _deviceMp->mqttUnsubscribe(_topicRegNoti);
    _deviceMp->mqttSubscribe(_topicGetSt);
    _deviceMp->mqttSubscribe(_topicDevOp);
    _deviceMp->mqttSubscribe(_topicGetGrpSt);

}

int smartDevice::appNotify(byte* payload, unsigned int length)
{
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& data = jsonBuffer.parse(payload);
    if(!data.success()){
        return RET_ERROR;
    }

    const char* userId = data["userId"];
    const char* hbIntvl = data["hbIntvl"];
    DEBUG_DEVICE.printf("%s, userId %s, hbIntvl %s\n", __FUNCTION__, userId, hbIntvl);

    _hbIntvlMs = atoi(hbIntvl);

}