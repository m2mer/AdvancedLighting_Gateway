/*
 Description:  device manipulation, such as operate, get status and etc.
 
 Author: Xuesong
 
 Date: 2018-04-03
 
 */

#include "deviceManipulation.h"
#include "uartProtocolPacket.h"



deviceManipulation::deviceManipulation(smartDevice *device, PubSubClient *mqttClient, HardwareSerial* serial) {
    this->mqttClient = mqttClient;
    this->_Serial = serial;
    this->_device = device;
}

void deviceManipulation::mqttSubscribe(char *topic)
{
    mqttClient->subscribe(topic);
}

void deviceManipulation::mqttUnsubscribe(char* topic)
{
    mqttClient->unsubscribe(topic);
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

    this->_device->receiveMQTTmsg(topic, payload, length);
}

boolean deviceManipulation::validUartProtocol(byte *data, int length)
{
    if(strncmp((const char*)data, UART_PROTOCOL_HEAD, UART_PROTOCOL_HEAD_LEN) != 0 
        || strncmp((const char*)&data[length-UART_PROTOCOL_TAIL_LEN], UART_PROTOCOL_TAIL, UART_PROTOCOL_TAIL_LEN) != 0)
        return false;

    return true;
}

/*
 * Interface to handle UART message from peer device
*/
int deviceManipulation::receiveUARTmsg(byte *buf, int length) {
    UART_PROTOCOL_DATA *protData = NULL;
    uint8_t protType = 0;
    byte *payload = NULL;

    DEBUG_DEVICE.printf("UART RX, len %d\n", length);

    if(validUartProtocol(buf, length))
    {
        memcpy(&_uartBuf[0], buf, length);
        _uartBufOff = length;
    }
    else if(strncmp((const char*)buf, UART_PROTOCOL_HEAD, UART_PROTOCOL_HEAD_LEN) == 0)
    {
        memcpy(&_uartBuf[0], buf, length);
        _uartBufOff = length; 
        return RET_ERROR;           
    }    
    else if(_uartBufOff != 0)
    {
        if(_uartBufOff+length > 128)
        {
            memset(_uartBuf, 0, 128);            
            _uartBufOff = 0;
            return RET_ERROR;
        }

        memcpy(&_uartBuf[_uartBufOff], buf, length);
        _uartBufOff += length;
        if(!validUartProtocol(_uartBuf, _uartBufOff))
            return RET_ERROR;
    }

    protData = (UART_PROTOCOL_DATA*)&_uartBuf[UART_PROTOCOL_HEAD_LEN];
    protType = protData->protType;

    if(protType == PROTOCOL_TYPE_WIFI_DEVICE_STATUS) {
        this->_device->receiveUARTmsg((byte*)&protData->protPayload, _uartBufOff-UART_PROTOCOL_FLAG_LEN-2);
    }
    else if(protType == PROTOCOL_TYPE_MESH_AGENT_STATUS) {        
        this->_device->receiveUARTmsg((byte*)&protData->protPayload, _uartBufOff-UART_PROTOCOL_FLAG_LEN-2);
    }    
 
    memset(_uartBuf, 0, 128);
    _uartBufOff = 0;
    return RET_OK;
}

void deviceManipulation::sendUartProtocolData(byte *protData)
{
    int len = 0;

    DEBUG_DEVICE.printf("%s", __FUNCTION__);

    byte buf[sizeof(UART_PROTOCOL_DATA)+10] = {0};

    memcpy(buf, "smart", 5);
    len = 5;
    memcpy(&buf[5], protData, sizeof(UART_PROTOCOL_DATA));
    len += sizeof(UART_PROTOCOL_DATA);
    memcpy(&buf[len], "trams", 5);
    len += 5;

    DEBUG_DEVICE.printf("send: ");
    for(int i=0; i<len; i++)
    {
        DEBUG_DEVICE.printf("%02x", buf[i]);
    }
    DEBUG_DEVICE.println();

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
