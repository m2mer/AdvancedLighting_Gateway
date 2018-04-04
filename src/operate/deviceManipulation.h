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
#include "deviceFunctionFormat.h"


class deviceManipulation
{

public:
    deviceManipulation(PubSubClient *mqttClient, HardwareSerial* serial);
    deviceManipulation(PubSubClient *mqttClient, HardwareSerial* serial, boolean isLocal);
    ~deviceManipulation(){};
    int deviceRegister();
    int receiveMQTTmsg(char* topic, byte* payload, unsigned int length);
    int receiveUARTmsg(char *buf, int length);

private:
    PubSubClient* mqttClient;
    HardwareSerial* _Serial;
    boolean _isLocal;
    const char* topicRegister = "device/device_register";
    const char* topicStatus = "device/status_update";

    int operateDevice(byte* payload, unsigned int length);
    int operateMeshAgent(const char* value);
    int operateWifiDevice(const char* action, const char* value);
    int operateLocalDevice(const char* action, const char* value);
    int getDeviceStatusAll(byte* payload, unsigned int length);
    int meshAgentStatus(char *buf, int length);
    int wifiDeviceStatus(char *buf, int length);
    String funcTypeToAttribute(WIFI_DEVICE_FUNCTION_TYPE funcType);

};




#endif
