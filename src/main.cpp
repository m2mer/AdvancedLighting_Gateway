/*
 Description:  Mesh Gateway
 
 Author: Xuesong
 
 Date: 2018-03-28
 
 */


#include <Arduino.h>
#include "common/common.h"
#include "Wifi/WifiManagement.h"


#define DEFAULT_WIFI_SSID     "ZQKL"
#define DEFAULT_WIFI_PASSWORD "zqkl123456.."


void setup() {
    // put your setup code here, to run once:

    serial_init();

    //WifiManagement WifiMg(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD);
    WifiManagement WifiMg;
    delay(3000);
    WifiMg.connectWifi();
}

void loop() {
    // put your main code here, to run repeatedly:
    while(1) {
        Serial.print("Hello");
        delay(1000);
    }
}