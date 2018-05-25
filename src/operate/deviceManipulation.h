/*
 Description:  device manipulation, such as operate, get status and etc.
 
 Author: Xuesong
 
 Date: 2018-04-03
 
 */


#ifndef deviceManipulation_H
#define deviceManipulation_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include "uartProtocolPacket.h"
#include "smartDevice.h"


#define RET_OK    0
#define RET_ERROR -1


class smartDevice;
class deviceManipulation
{

public:
    deviceManipulation(smartDevice *device, PubSubClient *mqttClient, HardwareSerial* serial);
    ~deviceManipulation(){};
    
    void mqttSubscribe(char* topic);
    void mqttUnsubscribe(char* topic);
    void mqttPublish(char* topic, char* msg);

    int receiveMQTTmsg(char* topic, byte* payload, unsigned int length);
    int receiveUARTmsg(byte *buf, int length);
    void sendUartProtocolData(byte *protData);
    boolean validUartProtocol(byte *data, int length);
    void testOperateMeshAgent();
    PubSubClient* mqttClient;
    
private:
//    PubSubClient* mqttClient;
    HardwareSerial* _Serial;
    smartDevice *_device;
    byte _uartBuf[128] = {0};
    uint8_t _uartBufOff = 0;
};




#endif
