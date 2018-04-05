/*
 Description:  device manipulation, such as operate, get status and etc.
 
 Author: Xuesong
 
 Date: 2018-04-03
 
 */

#include "deviceManipulation.h"
#include "deviceFunctionFormat.h"

/* pub topics */
const char* topicRegister = "device/device_register";
const char* topicUpdateStatus = "device/status_update";
const char* topicOverallStatus = "device/status_reply";
const char* topicBriefStatus = "device/update_brief";

/* sub topics */
const char* topicOperate = "device/device_operate";
const char* topicGetStatus = "device/get_status";


deviceManipulation::deviceManipulation(PubSubClient *mqttClient, HardwareSerial* serial) {
    this->mqttClient = mqttClient;
    this->_Serial = serial;
    this->_isLocal = false;

    mqttClient->subscribe(topicOperate);
    mqttClient->subscribe(topicGetStatus);
}

deviceManipulation::deviceManipulation(PubSubClient *mqttClient, HardwareSerial* serial, boolean isLocal) {
    this->mqttClient = mqttClient;
    this->_Serial = serial;
    this->_isLocal = isLocal;

    mqttClient->subscribe(topicOperate);
    mqttClient->subscribe(topicGetStatus);
}

int deviceManipulation::deviceRegister() {
    uint8 mac[6];
    char mac_str[12] = {0};
    char msg[256] = {0};

    //wifi_get_macaddr(STATION_IF, mac);
    WiFi.macAddress(mac);
    printf("get mac %0x%0x%0x%0x%0x%0x\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    sprintf(msg, "{\"type\":\"lamp\",\"vendor\":\"ht\",\"MAC\":\"");
    sprintf(mac_str, "%0x%0x%0x%0x%0x%0x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    strcat(msg, mac_str);
    strcat(msg, "\"}");

    printf("\nbegin to pub %s\n", msg);
    mqttClient->publish(topicRegister, msg);

    return 0;
}

/*
 * Interface to handle MQTT message from APP
*/
int deviceManipulation::receiveMQTTmsg(char* topic, byte* payload, unsigned int length) {
    Serial.printf("%s, topic %s, payload %s, len %d\n", __FUNCTION__, topic, payload, length);
    if(strcmp(topic, "device/device_operate") == 0)
        operateDevice(payload, length);
    else if(strcmp(topic, "device/get_status") == 0)
        getDeviceOverallStatus(payload, length);
}

/*
 * Interface to handle UART message from peer device
*/
int deviceManipulation::receiveUARTmsg(char *buf, int length) {
    UART_PROTOCOL_DATA *protData = (UART_PROTOCOL_DATA*)buf;
    UART_PROTOCOL_TYPE protType = protData->protType;

    Serial.printf("%s, len %d, type %d\n", __FUNCTION__, length, protType);
    for(int i=0; i<length; i++) {
        Serial.printf("%0x", buf[i]);
    }
    Serial.println("");

    if(protType == PROTOCOL_TYPE_WIFI_DEVICE_STATUS) {
        wifiDeviceStatus(buf, length);
    }
    else if(protType == PROTOCOL_TYPE_WIFI_DEVICE_OVERALL) {
        wifiDeviceOverallStatus(buf, length);
    }
    else if(protType == PROTOCOL_TYPE_WIFI_DEVICE_BRIEF) {
        wifiDeviceBriefStatus(buf, length);
    }
    else if(protType == PROTOCOL_TYPE_MESH_AGENT_STATUS) {
        meshAgentStatus(buf, length);
    }    
    else if(protType == PROTOCOL_TYPE_MESH_AGENT_OVERALL) {
        meshAgentOverallStatus(buf, length);
    }      
    else if(protType == PROTOCOL_TYPE_MESH_AGENT_BRIEF) {
        meshAgentBriefStatus(buf, length);
    }   
}

int deviceManipulation::operateDevice(byte* payload, unsigned int length) {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& data = jsonBuffer.parse(payload);
    //JsonObject data = jsonBuffer.parse(payload);
    if(!data.success()){
        return false;
    }
    const char* uuid = data["UUID"];
    const char* action = data["action"];
    const char* value = NULL;

    Serial.printf("uuid is %s\n", uuid);
    Serial.printf("action is %s\n", action);

    if(strcmp(action, "color") == 0) {
        JsonObject& color = data["value"];
        const char *h = color["h"];
        const char *s = color["s"];
        const char *v = color["v"];
        value = h;
        Serial.printf("value is %s\n", value);
    }
    else {
        value = data["value"];
        Serial.printf("value is %s\n", value);
    }
        
    if(strcmp(action, "mesh_agent") == 0)
        operateMeshAgent(value);
    else {
        if(!_isLocal)
            operateWifiDevice(action, value);
        else
            operateLocalDevice(action, value);
    }

    return 0;
}

/*
 * passthrough binary BLE Mesh command data to Mesh agent
*/
int deviceManipulation::operateMeshAgent(const char* value) {
    UART_PROTOCOL_DATA protData;
    byte* payload;

    Serial.printf("%s, command:%s\n", __FUNCTION__, value);
    
    memset(&protData, 0, sizeof(UART_PROTOCOL_DATA));
    protData.protType = PROTOCOL_TYPE_OPERATE_MESH_AGENT;
    payload = (byte*) &protData.protPayload.meshData; 
    memcpy(payload, value, sizeof(MESH_DEVICE_FUNCTION_DATA));

    _Serial->write((byte*)&protData, sizeof(UART_PROTOCOL_DATA));
}

/*
 * convert ascll device operation data to binary and send to device
*/
int deviceManipulation::operateWifiDevice(const char* action, const char* value) {
    UART_PROTOCOL_DATA protData;
    WIFI_DEVICE_FUNCTION_DATA *deviceData;

    Serial.printf("%s, action:%s, value:%s\n", __FUNCTION__, action, value);

    memset(&protData, 0, sizeof(UART_PROTOCOL_DATA));
    protData.protType = PROTOCOL_TYPE_OPERATE_WIFI_DEVICE;
    deviceData = &protData.protPayload.wifiData;

    if(strcmp(action, "onoff") == 0) {
        deviceData->funcType = WIFI_DEVICE_FUNCTION_ONOFF;
        deviceData->funcPara.para = *value;
    }
    else if(strcmp(action, "lightness") == 0) {
        deviceData->funcType = WIFI_DEVICE_FUNCTION_LIGHTNESS;
        deviceData->funcPara.para = *value;
    }
    else if(strcmp(action, "temperature") == 0) {
        deviceData->funcType = WIFI_DEVICE_FUNCTION_TEMPERATURE;
        deviceData->funcPara.para = *value;
    }
     else if(strcmp(action, "color") == 0) {
        deviceData->funcType = WIFI_DEVICE_FUNCTION_COLOR;
        deviceData->funcPara.para = *value;
    }
    else if(strcmp(action, "mode") == 0) {
        deviceData->funcType = WIFI_DEVICE_FUNCTION_MODE;
        deviceData->funcPara.para = *value;
    }
    else if(strcmp(action, "timer_on") == 0) {
        deviceData->funcType = WIFI_DEVICE_FUNCTION_TIMER_ON;
        deviceData->funcPara.para = *value;
    }
    else if(strcmp(action, "timer_off") == 0) {
        deviceData->funcType = WIFI_DEVICE_FUNCTION_TIMER_OFF;
        deviceData->funcPara.para = *value;
    }

    _Serial->write((byte*)&protData, sizeof(UART_PROTOCOL_DATA));
}

/*
 * parse ascll device operation data and execute
*/
int deviceManipulation::operateLocalDevice(const char* action, const char* value) {
    return 0;
}

int deviceManipulation::getDeviceOverallStatus(byte* payload, unsigned int length) {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& data = jsonBuffer.parse(payload);
    //JsonObject data = jsonBuffer.parse(payload);
    if(!data.success()){
        return false;
    }
    const char* uuid = data["UUID"];
    const char* action = data["action"];
    const char* value = data["value"];
    Serial.printf("uuid is %s\n", uuid);
    Serial.printf("action is %s\n", action);
    Serial.printf("value is %s\n", value);

    if(strcmp(action, "mesh_agent") == 0)
        getMeshAgentStatus(value);
    else {
        if(!_isLocal)
            getWifiDeviceStatus();
        else
            getLocalDeviceStatus();
    }

    return 0;
}

/*
 * passthrough binary BLE Mesh command data to Mesh agent
*/
int deviceManipulation::getMeshAgentStatus(const char* value) {
    UART_PROTOCOL_DATA protData;
    byte* payload;

    Serial.printf("%s, command:%s\n", __FUNCTION__, value);
    
    memset(&protData, 0, sizeof(UART_PROTOCOL_DATA));
    protData.protType = PROTOCOL_TYPE_GET_MESH_AGENT;
    payload = (byte*) &protData.protPayload.meshData; 
    memcpy(payload, value, sizeof(MESH_DEVICE_FUNCTION_DATA));

    _Serial->write((byte*)&protData, sizeof(UART_PROTOCOL_DATA));
}

/*
 * send binary get_status command by uart
*/
int deviceManipulation::getWifiDeviceStatus() {
    UART_PROTOCOL_DATA protData;
    WIFI_DEVICE_FUNCTION_DATA *deviceData;

    Serial.printf("%s\n", __FUNCTION__);

    memset(&protData, 0, sizeof(UART_PROTOCOL_DATA));
    protData.protType = PROTOCOL_TYPE_GET_WIFI_DEVICE;  

    _Serial->write((byte*)&protData, sizeof(UART_PROTOCOL_DATA));    
}

/*
 * execute get_status locally
*/
int deviceManipulation::getLocalDeviceStatus() {
    return 0;
}

/*
 * passthrough binary device status from Mesh agent
*/
int deviceManipulation::meshAgentStatus(char *buf, int length) {
    char msg[256] = {0};
    Serial.printf("%s\n", __FUNCTION__);

    _packageMeshAgentMsg(buf, msg);

    Serial.printf("\nbegin to pub %s\n", msg);
    mqttClient->publish(topicUpdateStatus, msg);

    return 0;
}

/*
 * passthrough binary overall device status from Mesh agent
*/
int deviceManipulation::meshAgentOverallStatus(char *buf, int length) {
    char msg[256] = {0};
    Serial.printf("%s\n", __FUNCTION__);

    _packageMeshAgentMsg(buf, msg);

    Serial.printf("\nbegin to pub %s\n", msg);
    mqttClient->publish(topicOverallStatus, msg);

    return 0;
}

/*
 * passthrough binary brief device status from Mesh agent
*/
int deviceManipulation::meshAgentBriefStatus(char *buf, int length) {
    char msg[256] = {0};
    Serial.printf("%s\n", __FUNCTION__);

    _packageMeshAgentMsg(buf, msg);

    Serial.printf("\nbegin to pub %s\n", msg);
    mqttClient->publish(topicBriefStatus, msg);

    return 0;
}

/*
 * package passthrough Mesh agent message
*/
void deviceManipulation::_packageMeshAgentMsg(char *buf, char *msg) {
    UART_PROTOCOL_DATA *protData = (UART_PROTOCOL_DATA*)buf;

    Serial.printf("%s\n", __FUNCTION__);

    byte *funcPara = protData->protPayload.meshData.funcPara;
    byte mac[6] = {0};
    char uuid[12] = {0};

    WiFi.macAddress(mac);
    sprintf(uuid, "%0x%0x%0x%0x%0x%0x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    strcat(msg, "{\"UUID\":\"");
    strncat(msg, uuid, 12);
    strcat(msg, "\",\"attribute\":\"");
    strcat(msg, "mesh_agent");
    strcat(msg, "\",\"value\":\"");
    strcat(msg, (const char*)funcPara);
    strcat(msg, "\"}");
}

/*
 * covert binary device status from Wifi device to string
*/
int deviceManipulation::wifiDeviceStatus(char *buf, int length) {
    char msg[256] = {0};
    byte mac[6] = {0};
    char uuid[12] = {0};
    String attribute = "";
    String value = "";

    Serial.printf("%s\n", __FUNCTION__);

    _parseAttributeValue(buf, attribute, value);

    WiFi.macAddress(mac);
    sprintf(uuid, "%0x%0x%0x%0x%0x%0x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    strcat(msg, "{\"UUID\":\"");
    strncat(msg, uuid, 12);
    strcat(msg, "\",\"attribute\":\"");
    strcat(msg, attribute.c_str());
    strcat(msg, "\",\"value\":");
    strcat(msg, value.c_str());
    strcat(msg, "}");

    Serial.printf("\nbegin to pub %s\n", msg);
    mqttClient->publish(topicUpdateStatus, msg);

    return 0;
}

/*
 * parse binary device atrribute/value to string
*/
void deviceManipulation::_parseAttributeValue(char *buf, String& attribute, String& value) {
    UART_PROTOCOL_DATA *protData = (UART_PROTOCOL_DATA*)buf;
    WIFI_DEVICE_FUNCTION_TYPE type = protData->protPayload.wifiData.funcType;
    WIFI_DEVICE_FUNCTION_PARA para = protData->protPayload.wifiData.funcPara;
    char colorStr[32] = {0};
    char tmp[5] = {0};

    switch(type) {
        case WIFI_DEVICE_FUNCTION_ONOFF:
            attribute += "onoff";
            value += String(para.para, DEC);
            break;
        case WIFI_DEVICE_FUNCTION_LIGHTNESS:
            attribute += "lightness";
            value += String(para.para, DEC);
            break;
        case WIFI_DEVICE_FUNCTION_TEMPERATURE:
            attribute += "temperature";
            value += String(para.para, DEC);
            break;
        case WIFI_DEVICE_FUNCTION_COLOR:
            attribute += "color";
            strcat(colorStr, "{\"h\":");
            itoa(para.colorPara.h, tmp, 10);
            strcat(colorStr,tmp);
            strcat(colorStr, ",\"s\":");
            itoa(para.colorPara.s, tmp, 10);
            strcat(colorStr,tmp);
            strcat(colorStr, ",\"v\":");
            itoa(para.colorPara.v, tmp, 10);
            strcat(colorStr,tmp);
            strcat(colorStr, "}");
            break;
        case WIFI_DEVICE_FUNCTION_MODE:
            attribute += "mode";
            value += String(para.para, DEC);
            break;
        case WIFI_DEVICE_FUNCTION_TIMER_ON:
            attribute += "timer_on";
            value += String(para.para, DEC);
            break;
        case WIFI_DEVICE_FUNCTION_TIMER_OFF:
            attribute += "timer_off";
            value += String(para.para, DEC);
            break;
        default:
            break;
    }

}

/*
 * covert binary overall device status from Wifi device to string
*/
int deviceManipulation::wifiDeviceOverallStatus(char *buf, int length) {
    char msg[256] = {0};
    byte mac[6] = {0};
    char uuid[12] = {0};
    char colorStr[32] = {0};
    char tmp[5] = {0};
    UART_PROTOCOL_DATA *protData = (UART_PROTOCOL_DATA*)buf;
    WIFI_DEVICE_FUNCTION_TYPE type = protData->protPayload.wifiData.funcType;
    WIFI_DEVICE_FUNCTION_PARA para = protData->protPayload.wifiData.funcPara;

    Serial.printf("%s\n", __FUNCTION__);

    WiFi.macAddress(mac);
    sprintf(uuid, "%0x%0x%0x%0x%0x%0x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    strcat(msg, "{\"UUID\":\"");
    strncat(msg, uuid, 12);
    strcat(msg, "\",\"onoff\":");
    itoa(para.overallStatus.onoff, tmp, 10);
    strcat(msg, tmp);
    strcat(msg, ",\"lightness\":");
    itoa(para.overallStatus.lightness, tmp, 10);
    strcat(msg, tmp);
    strcat(msg, ",\"temperature\":");
    itoa(para.overallStatus.temperature, tmp, 10);
    strcat(msg, tmp); 
    strcat(msg, ",\"color\":");
    strcat(colorStr, "{\"h\":");
    itoa(para.overallStatus.color.h, tmp, 10);
    strcat(colorStr,tmp);
    strcat(colorStr, ",\"s\":");
    itoa(para.overallStatus.color.s, tmp, 10);
    strcat(colorStr,tmp);
    strcat(colorStr, ",\"v\":");
    itoa(para.overallStatus.color.v, tmp, 10);
    strcat(colorStr,tmp);
    strcat(colorStr, "}");
    strcat(msg, ",\"mode\":");
    itoa(para.overallStatus.mode, tmp, 10);
    strcat(msg, tmp);
    strcat(msg, ",\"timer_on\":");
    itoa(para.overallStatus.timer_on, tmp, 10);
    strcat(msg, tmp);
    strcat(msg, ",\"timer_off\":");
    itoa(para.overallStatus.timer_off, tmp, 10);
    strcat(msg, tmp);
    strcat(msg, "}");

    Serial.printf("\nbegin to pub %s\n", msg);
    mqttClient->publish(topicOverallStatus, msg);

    return 0;
}

/*
 * covert binary brief device status from Wifi device to string
*/
int deviceManipulation::wifiDeviceBriefStatus(char *buf, int length) {
    char msg[256] = {0};
    byte mac[6] = {0};
    char uuid[12] = {0};
    char colorStr[32] = {0};
    char tmp[5] = {0};
    UART_PROTOCOL_DATA *protData = (UART_PROTOCOL_DATA*)buf;
    WIFI_DEVICE_FUNCTION_TYPE type = protData->protPayload.wifiData.funcType;
    WIFI_DEVICE_FUNCTION_PARA para = protData->protPayload.wifiData.funcPara;

    Serial.printf("%s\n", __FUNCTION__);

    WiFi.macAddress(mac);
    sprintf(uuid, "%0x%0x%0x%0x%0x%0x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    strcat(msg, "{\"UUID\":\"");
    strncat(msg, uuid, 12);
    strcat(msg, "\",\"onoff\":");
    itoa(para.overallStatus.onoff, tmp, 10);
    strcat(msg, tmp);
    strcat(msg, ",\"lightness\":");
    itoa(para.overallStatus.lightness, tmp, 10);
    strcat(msg, tmp);
    strcat(msg, ",\"temperature\":");
    itoa(para.overallStatus.temperature, tmp, 10);
    strcat(msg, tmp); 
    strcat(msg, ",\"mode\":");
    itoa(para.overallStatus.mode, tmp, 10);
    strcat(msg, tmp);
    strcat(msg, "}");

    Serial.printf("\nbegin to pub %s\n", msg);
    mqttClient->publish(topicBriefStatus, msg);

    return 0;
}
