/*
 Description: wifi device
 
 Author: Xuesong
 
 Date: 2018-04-27
 
 */


#include <ArduinoJson.h>

#include "wifiDevice.h"
#include "uartProtocolPacket.h"





int wifiDevice::operateDevice(byte* payload, unsigned int length)
{
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& data = jsonBuffer.parse(payload);
    if(!data.success()){
        return false;
    }
    const char* uuid = data["UUID"];
    const char* action = data["action"];
    const char* value = NULL;

    UART_PROTOCOL_DATA protData;
    WIFI_DEVICE_FUNCTION_DATA *deviceData;

    DEBUG_DEVICE.printf("%s, uuid is %s, action is %s\n", __FUNCTION__, uuid, action);

    if(strcmp(action, "color") == 0) {
        JsonObject& color = data["value"];
        const char *h = color["h"];
        const char *s = color["s"];
        const char *v = color["v"];
        value = h;
        DEBUG_DEVICE.printf("value is %s\n", value);
    }
    else {
        value = data["value"];
        DEBUG_DEVICE.printf("value is %s\n", value);
    }

    memset(&protData, 0, sizeof(UART_PROTOCOL_DATA));
    protData.protType = PROTOCOL_TYPE_OPERATE_WIFI_DEVICE;
    deviceData = &protData.protPayload.wifiData;
    deviceData->cmdType = WIFI_DEVICE_COMMAND_OPERATION;

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

    _deviceMp->sendUartProtocolData((byte*)&protData);

    return RET_OK;
}

int wifiDevice::getOverallStatus(byte* payload, unsigned int length) 
{
    UART_PROTOCOL_DATA protData;
    WIFI_DEVICE_FUNCTION_DATA *deviceData;

    DEBUG_DEVICE.printf("%s\n", __FUNCTION__);

    memset(&protData, 0, sizeof(UART_PROTOCOL_DATA));
    protData.protType = PROTOCOL_TYPE_GET_WIFI_DEVICE;
    protData.protPayload.wifiData.cmdType = WIFI_DEVICE_COMMAND_GET_STATUS;

    _deviceMp->sendUartProtocolData((byte*)&protData);

    return RET_OK;
}

/* 
 * interface to receive message from peer uart
 * 
 */
void wifiDevice::receiveUARTmsg(byte *buf, int len)
{
    WIFI_DEVICE_FUNCTION_DATA *data = (WIFI_DEVICE_FUNCTION_DATA*) buf;
    uint8_t cmd = 0;

    if(!buf)
        return;

    DEBUG_DEVICE.printf("%s, receive data:", __FUNCTION__);
    for(int i=0; i<len; i++) {
        DEBUG_DEVICE.printf("%02x ", buf[i]);
    }

    cmd = data->cmdType;
    if(cmd == WIFI_DEVICE_COMMAND_STATUS_UPDATE)
        recvStatusUpdate(buf);
    else if(cmd == WIFI_DEVICE_COMMAND_OVERALL_STATUS)
        recvOverallStatus(buf);
}

void wifiDevice::recvStatusUpdate(byte *buf)
{
    char msg[256] = {0};
    byte mac[6] = {0};
    char uuid[12] = {0};
    String attribute = "";
    String value = "";

    DEBUG_DEVICE.printf("%s\n", __FUNCTION__);

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

    DEBUG_DEVICE.printf("\nbegin to pub %s\n", msg);
    _deviceMp->mqttPublish(getMQTTtopic(PUB_TOPIC_STATUS_UPDATE), msg);
}

/*
 * covert binary overall device status from Wifi device to string
*/
void wifiDevice::recvOverallStatus(byte *buf)
{
    char msg[256] = {0};
    byte mac[6] = {0};
    char uuid[12] = {0};
    char colorStr[32] = {0};
    char tmp[5] = {0};
    WIFI_DEVICE_FUNCTION_DATA *data = (WIFI_DEVICE_FUNCTION_DATA*)buf;
    uint8_t type = (WIFI_DEVICE_FUNCTION_TYPE)data->funcType;
    WIFI_DEVICE_FUNCTION_PARA *para = (WIFI_DEVICE_FUNCTION_PARA*)&data->funcPara;

    DEBUG_DEVICE.printf("%s\n", __FUNCTION__);

    getMacAddress(mac);
    sprintf(uuid, "%0x%0x%0x%0x%0x%0x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    strcat(msg, "{\"UUID\":\"");
    strncat(msg, uuid, 12);
    strcat(msg, "\",\"onoff\":");
    itoa(para->overallStatus.onoff, tmp, 10);
    strcat(msg, tmp);
    strcat(msg, ",\"lightness\":");
    itoa(para->overallStatus.lightness, tmp, 10);
    strcat(msg, tmp);
    strcat(msg, ",\"temperature\":");
    itoa(para->overallStatus.temperature, tmp, 10);
    strcat(msg, tmp); 
    strcat(msg, ",\"color\":");
    strcat(colorStr, "{\"h\":");
    itoa(para->overallStatus.color.h, tmp, 10);
    strcat(colorStr,tmp);
    strcat(colorStr, ",\"s\":");
    itoa(para->overallStatus.color.s, tmp, 10);
    strcat(colorStr,tmp);
    strcat(colorStr, ",\"v\":");
    itoa(para->overallStatus.color.v, tmp, 10);
    strcat(colorStr,tmp);
    strcat(colorStr, "}");
    strcat(msg, ",\"mode\":");
    itoa(para->overallStatus.mode, tmp, 10);
    strcat(msg, tmp);
    strcat(msg, ",\"timer_on\":");
    itoa(para->overallStatus.timerOn, tmp, 10);
    strcat(msg, tmp);
    strcat(msg, ",\"timer_off\":");
    itoa(para->overallStatus.timerOff, tmp, 10);
    strcat(msg, tmp);
    strcat(msg, "}");

    DEBUG_DEVICE.printf("\nbegin to pub %s\n", msg);
    _deviceMp->mqttPublish(getMQTTtopic(PUB_TOPIC_OVERALL_STATUS), msg);

}


/*
 * parse binary device atrribute/value to string
*/
void wifiDevice::_parseAttributeValue(byte *buf, String& attribute, String& value) 
{
    WIFI_DEVICE_FUNCTION_DATA *data = (WIFI_DEVICE_FUNCTION_DATA*)buf;
    uint8_t type = data->funcType;
    WIFI_DEVICE_FUNCTION_PARA para = data->funcPara;
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