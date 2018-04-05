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

    int operateDevice(byte* payload, unsigned int length);
    int operateMeshAgent(const char* value);
    int operateWifiDevice(const char* action, const char* value);
    int operateLocalDevice(const char* action, const char* value);
    int getDeviceOverallStatus(byte* payload, unsigned int length);
    int getMeshAgentStatus(const char* value);
    int getWifiDeviceStatus();
    int getLocalDeviceStatus();
    int meshAgentStatus(char *buf, int length);
    void _packageMeshAgentMsg(char *buf, char *msg);
    int meshAgentOverallStatus(char *buf, int length);
    int meshAgentBriefStatus(char *buf, int length);        
    int wifiDeviceStatus(char *buf, int length);
    void _parseAttributeValue(char *buf, String& attribute, String& value);
    int wifiDeviceOverallStatus(char *buf, int length);
    int wifiDeviceBriefStatus(char *buf, int length);
};




#endif
