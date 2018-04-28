/*
 Description:  device manipulation, such as operate, get status and etc.
 
 Author: Xuesong
 
 Date: 2018-04-03
 
 */

#include "deviceManipulation.h"
#include "uartProtocolPacket.h"

#define DEBUG_DEVICE Serial1



deviceManipulation::deviceManipulation(smartDevice *device, PubSubClient *mqttClient, HardwareSerial* serial) {
    this->mqttClient = mqttClient;
    this->_Serial = serial;
    this->_device = device;
}

void deviceManipulation::mqttSubscribe(char *topic)
{
    mqttClient->subscribe(topic);
}

void deviceManipulation::mqttPublish(char* topic, char* msg)
{
    mqttClient->publish(topic, msg);
}

/*
 * Interface to handle MQTT message from APP
*/
int deviceManipulation::receiveMQTTmsg(char* topic, byte* payload, unsigned int length) {
    DEBUG_DEVICE.printf("%s, topic %s, payload %s, len %d\n", __FUNCTION__, topic, payload, length);
    if(strcmp(topic, "device/device_operate") == 0 
        || strcmp(topic, "device/get_status") == 0
        || strcmp(topic, "device/get_group_status") == 0)
        this->_device->receiveMQTTmsg(topic, payload, length);
}

/*
 * Interface to handle UART message from peer device
*/
int deviceManipulation::receiveUARTmsg(byte *buf, int length) {
    UART_PROTOCOL_DATA *protData = NULL;
    uint8_t protType = 0;
    byte *payload = NULL;

    if(strncmp((const char*)buf, UART_PROTOCOL_HEAD, UART_PROTOCOL_HEAD_LEN) != 0 
        || strncmp((const char*)&buf[length-UART_PROTOCOL_TAIL_LEN], UART_PROTOCOL_TAIL, UART_PROTOCOL_TAIL_LEN) != 0)
        return RET_ERROR;

    DEBUG_DEVICE.printf("%s, len %d\n", __FUNCTION__, length);

    protData = (UART_PROTOCOL_DATA*)&buf[UART_PROTOCOL_HEAD_LEN];
    protType = protData->protType;

    if(protType == PROTOCOL_TYPE_WIFI_DEVICE_STATUS) {
        this->_device->receiveUARTmsg((byte*)&protData->protPayload, length-UART_PROTOCOL_FLAG_LEN-2);
    }
    else if(protType == PROTOCOL_TYPE_MESH_AGENT_STATUS) {        
        this->_device->receiveUARTmsg((byte*)&protData->protPayload, length-UART_PROTOCOL_FLAG_LEN-2);
    }    
 
}

void deviceManipulation::sendUartProtocolData(byte *protData)
{
    int len = 0;

    byte buf[sizeof(UART_PROTOCOL_DATA)+10] = {0};

    memcpy(buf, "smart", 5);
    len = 5;
    memcpy(&buf[5], protData, sizeof(UART_PROTOCOL_DATA));
    len += sizeof(UART_PROTOCOL_DATA);
    memcpy(&buf[len], "trams", 5);
    len += 5;

    _Serial->write(buf, len);
}


void deviceManipulation::testOperateMeshAgent() {
    uint8_t buf[64] = {0};
    int len = 0;
    UART_PROTOCOL_DATA protData;
    uint8_t* payload;
    uint16_t mesh_cmd = 0x33;
    uint32_t mac = 0x01020304;
    uint8_t para[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};

    memset(&protData, 0, sizeof(UART_PROTOCOL_DATA));
    protData.protType = 1;    //PROTOCOL_TYPE_OPERATE_MESH_AGENT;
    payload = (uint8_t*)&protData.protPayload.meshData;
    *(uint16_t*)payload = mesh_cmd;
    memcpy(&payload[2], &mac, 4);
    memcpy(&payload[6], &para, 6);

    memcpy(buf, "smart", 5);
    len = 5;
    memcpy(&buf[5], &protData, sizeof(UART_PROTOCOL_DATA));
    len += sizeof(UART_PROTOCOL_DATA);
    memcpy(&buf[len], "trams", 5);
    len += 5;

    _Serial->write(buf, len);
}
