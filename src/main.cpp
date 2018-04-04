/*
 Description:  Mesh Gateway
 
 Author: Xuesong
 
 Date: 2018-03-28
 
 */


#include <Arduino.h>
#include "common/common.h"
#include "Wifi/WifiManagement.h"
#include "MQTT/MQTTtransport.h"
#include "operate/deviceManipulation.h"
#include "UART/UARTdriver.h"


#define DEFAULT_WIFI_SSID     "ZQKL"
#define DEFAULT_WIFI_PASSWORD "zqkl123456.."

const char *mqtt_server = "www.futureSmart.top";
const char *mqtt_client_name = "gateway";

/* Global Variables */
WifiManagement WifiMg;
WiFiClient espClient;
MQTTtransport MQTTtp(espClient);
deviceManipulation deviceMp(&MQTTtp, &Serial);
UARTdriver uartEp;



void uartRxHandler(const char *buf, int len) {
    Serial.printf("received len %d:\n",len);
    Serial.println(buf);

    deviceMp.receiveUARTmsg((char*)buf, len);
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

void setup() {
    // put your setup code here, to run once:
    serial_init();
    uartEp.setRxHdl(uartRxHandler);
    
    //WifiManagement WifiMg(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD);
    delay(3000);
    WifiMg.connectWifi();
    MQTTtp.setup(mqtt_server, mqtt_client_name, receiveMQTTmsg);
}

void loop() {
    // put your main code here, to run repeatedly:
    while(1) {
        Serial.println("Hello");

        if(!MQTTtp.connected())
            MQTTtp.reconnect();
        else
            MQTTtp.loop();

        //serialEvent();

        uartEp.loop();

        delay(1000);

    }
}