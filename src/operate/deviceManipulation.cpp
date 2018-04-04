/*
 Description:  device manipulation, such as operate, get status and etc.
 
 Author: Xuesong
 
 Date: 2018-04-03
 
 */

#include "deviceManipulation.h"
#include "deviceFunctionFormat.h"


deviceManipulation::deviceManipulation(PubSubClient *mqttClient, HardwareSerial* serial) {
    this->mqttClient = mqttClient;
    this->_Serial = serial;
    this->_isLocal = false;
}

deviceManipulation::deviceManipulation(PubSubClient *mqttClient, HardwareSerial* serial, boolean isLocal) {
    this->mqttClient = mqttClient;
    this->_Serial = serial;
    this->_isLocal = isLocal;
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

int deviceManipulation::receiveMQTTmsg(char* topic, byte* payload, unsigned int length) {
    Serial.printf("%s, topic %s, payload %s, len %d\n", __FUNCTION__, topic, payload, length);
    if(strcmp(topic, "device/device_operate") == 0)
        operateDevice(payload, length);
    else if(strcmp(topic, "device/get_status") == 0)
        getDeviceStatusAll(payload, length);
}

int deviceManipulation::receiveUARTmsg(char *buf, int length) {
    UART_PROTOCOL_DATA *protData = (UART_PROTOCOL_DATA*)buf;
    UART_PROTOCOL_TYPE protType = protData->protType;

    Serial.printf("%s, len %d, type %d\n", __FUNCTION__, length, protType);
    for(int i=0; i<length; i++) {
        Serial.printf("%0x", buf[i]);
    }
    Serial.println("");

    if(protType == PROTOCOL_TYPE_MESH_AGENT_STATUS) {
        meshAgentStatus(buf, length);
    }
    else if(protType == PROTOCOL_TYPE_WIFI_DEVICE_STATUS) {
        wifiDeviceStatus(buf, length);
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

    Serial.printf("operateMeshAgent, command:%s\n", value);

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

}

int deviceManipulation::getDeviceStatusAll(byte* payload, unsigned int length) {

}

int deviceManipulation::meshAgentStatus(char *buf, int length) {
    UART_PROTOCOL_DATA *protData = (UART_PROTOCOL_DATA*)buf;
    MESH_DEVICE_FUNCTION_TYPE funcType = protData->protPayload.meshData.funcType;

    Serial.printf("meshAgentStatus\n");

    byte *funcPara = protData->protPayload.meshData.funcPara;
    char msg[256] = {0};
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

    Serial.printf("\nbegin to pub %s\n", msg);
    mqttClient->publish(topicStatus, msg);

}

int deviceManipulation::wifiDeviceStatus(char *buf, int length) {
    UART_PROTOCOL_DATA *protData = (UART_PROTOCOL_DATA*)buf;
    WIFI_DEVICE_FUNCTION_TYPE funcType = protData->protPayload.wifiData.funcType;

    //TBD: more kinds of parameter
    ushort funcPara = protData->protPayload.wifiData.funcPara.para;
    Serial.printf("wifiDeviceStatus, attr %d, value %d\n", funcType, funcPara);

    char msg[256] = {0};
    byte mac[6] = {0};
    char uuid[12] = {0};

    WiFi.macAddress(mac);
    sprintf(uuid, "%0x%0x%0x%0x%0x%0x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    String attribute = funcTypeToAttribute(funcType);
    String value = String(funcPara, DEC);

    strcat(msg, "{\"UUID\":\"");
    strncat(msg, uuid, 12);
    strcat(msg, "\",\"attribute\":\"");
    strcat(msg, attribute.c_str());
    strcat(msg, "\",\"value\":\"");
    strcat(msg, value.c_str());
    strcat(msg, "\"}");

    Serial.printf("\nbegin to pub %s\n", msg);
    mqttClient->publish(topicStatus, msg);
}

String deviceManipulation::funcTypeToAttribute(WIFI_DEVICE_FUNCTION_TYPE funcType) {
    String attribute = "";

    switch(funcType) {
        case WIFI_DEVICE_FUNCTION_ONOFF:
            attribute += "onoff";
            break;
        case WIFI_DEVICE_FUNCTION_LIGHTNESS:
            attribute += "lightness";
            break;
        case WIFI_DEVICE_FUNCTION_TEMPERATURE:
            attribute += "temperature";
            break;
        case WIFI_DEVICE_FUNCTION_COLOR:
            attribute += "color";
            break;
        case WIFI_DEVICE_FUNCTION_MODE:
            attribute += "mode";
            break;
        case WIFI_DEVICE_FUNCTION_TIMER_ON:
            attribute += "timer_on";
            break;
        case WIFI_DEVICE_FUNCTION_TIMER_OFF:
            attribute += "timer_off";
            break;
        default:
            break;
    }

    return attribute;
}