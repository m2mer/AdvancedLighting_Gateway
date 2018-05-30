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
#include "operate/meshAgent.h"
#include "operate/meshNode.h"
#include "UART/UARTdriver.h"


#define DEFAULT_WIFI_SSID     "ZQKL"
#define DEFAULT_WIFI_PASSWORD "zqkl123456.."

const char *mqtt_server = "www.futureSmart.top";
//const char *mqtt_server = "192.168.1.141";
const char *mqtt_client_prefix = "MESH-GW-";

/* Global Variables */
LOCAL_METADATA localMetadata;
meshAgent localDevice;
WifiManagement WifiMg;
WiFiClient espClient;
MQTTtransport MQTTtp(espClient);
deviceManipulation deviceMp(&localDevice, &MQTTtp, &Serial);
UARTdriver uartEp;



void uartRxHandler(const char *buf, int len) {
    Serial.printf("\nUart received len %d:\n  ",len);
    for(int i=0; i<len; i++)
    {
        Serial.printf("%02x ", buf[i]);
    }

    deviceMp.receiveUARTmsg((byte*)buf, len);
}

void flashDataResume()
{

}

void smartConfigDone() {
    localMetadata.networkConfiged = 1;
    localMetadata.registered = 0;
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

    int need_network_Cfg = 0;
    byte mac[6] = {0};
    char mac_str[13] = {0};
    char mqtt_client_name[32] = {0};

    serial_init();
    uartEp.setRxHdl(uartRxHandler);
    intr_register(BUTTON_RESET, buttonIntr);

    WiFi.macAddress(mac);
    localDevice.setMAC(mac);
    localDevice.setDeviceManipulator(&deviceMp);

    /* fetch data from flash */
    flashDataResume();
    localDevice.notifyMeshAgent(localMetadata.networkConfiged);
    if(!localMetadata.networkConfiged)
        need_network_Cfg = 1;

    //WifiManagement WifiMg(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD);
    delay(3000);
    WifiMg.setSmartCfgCb(smartConfigDone);
    WifiMg.connectWifi(need_network_Cfg);
    smartConfigDone();  //for test

    sprintf(mac_str, "%02x%02x%02x%02x%02x%02x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    strcat(mqtt_client_name, mqtt_client_prefix);
    strcat(mqtt_client_name, mac_str);
    MQTTtp.setup(mqtt_server, mqtt_client_name, receiveMQTTmsg);
}

void loop() {

    while(1) {

        /* reconnect if network broken for some reason */
        if(WiFi.status() != WL_CONNECTED)
        {
            WifiMg.connectWifi(0);
        }
#if 1
        if(!MQTTtp.connected())
            MQTTtp.reconnect();
        else
        {
            if(!localMetadata.registered)
            {
                localDevice.deviceRegister();
                localMetadata.registered = 1;
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