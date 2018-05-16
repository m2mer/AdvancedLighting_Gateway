/*
 Description:  Mesh Gateway
 
 Author: Xuesong
 
 Date: 2018-03-28
 
 */


#include <Arduino.h>
#include "common/common.h"
#include "GPIO/io.h"
#include "GPIO/interruptEvent.h"
#include "Wifi/WifiManagement.h"
#include "MQTT/MQTTtransport.h"
#include "operate/deviceManipulation.h"
#include "operate/smartDevice.h"
#include "operate/meshDevice.h"
#include "UART/UARTdriver.h"


#define DEFAULT_WIFI_SSID     "ZQKL"
#define DEFAULT_WIFI_PASSWORD "zqkl123456.."

const char *mqtt_server = "www.futureSmart.top";
//const char *mqtt_server = "192.168.1.141";
const char *mqtt_client_name = "gateway";

/* Global Variables */
meshAgent localDevice;
WifiManagement WifiMg;
WiFiClient espClient;
MQTTtransport MQTTtp(espClient);
deviceManipulation deviceMp(&localDevice, &MQTTtp, &Serial);
UARTdriver uartEp;

bool isSmartConfiged = false;
bool isDeviceRegistered = true;


void uartRxHandler(const char *buf, int len) {
    Serial.printf("\nUart received len %d:\n  ",len);
    for(int i=0; i<len; i++)
    {
        Serial.printf("%02x ", buf[i]);
    }

    deviceMp.receiveUARTmsg((byte*)buf, len);
}

void smartConfigDone() {
    isSmartConfiged = true;
    isDeviceRegistered = false;
}

void receiveMQTTmsg(char* topic, byte* payload, unsigned int length) {
    #ifdef DEBUG_MQTT
    DEBUG_MQTT.print(TimeStamp + "Message arrived [");
    DEBUG_MQTT.print(topic);
    DEBUG_MQTT.print("] ");

    for (int i = 0; i < length; i++) {
        DEBUG_MQTT.print((char)payload[i]);
    }
    DEBUG_MQTT.println();
    #endif

    deviceMp.receiveMQTTmsg(topic, payload, length);

}

void buttonIntr()
{
    Serial.printf("button pushed\n");
}

void setup() {
    // put your setup code here, to run once:

    byte mac[6] = {0};

    serial_init();
    uartEp.setRxHdl(uartRxHandler);
    intr_register(BUTTON_RESET, buttonIntr);

    WiFi.macAddress(mac);
    localDevice.setMAC(mac);
    localDevice.setDeviceManipulator(&deviceMp);

    //WifiManagement WifiMg(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD);
    delay(3000);
    WifiMg.setSmartCfgCb(smartConfigDone);
    WifiMg.connectWifi();
    smartConfigDone();  //for test

    MQTTtp.setup(mqtt_server, mqtt_client_name, receiveMQTTmsg);
}

void loop() {
    // put your main code here, to run repeatedly:
    while(1) {
        //Serial.println("Hello");
#if 1
        if(!MQTTtp.connected())
            MQTTtp.reconnect();
        else
        {
            if(!isDeviceRegistered)
            {
                localDevice.deviceRegister();
                isDeviceRegistered = true;
            }
            MQTTtp.loop();
        }

#endif

        //serialEvent();

        uartEp.loop();

        /* send heartbeat every second */
        //localDevice.heartbeat();
    }
}