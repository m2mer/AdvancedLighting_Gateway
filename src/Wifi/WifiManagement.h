/*
 Description:  Fetch and manage device's network address
 
 Author: Xuesong
 
 Date: 2018-03-28
 
 */

#ifndef WiFiManagement_h
#define WiFiManagement_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>


#define WiFi_SSID_LEN_ADDR  0
#define WiFi_PSW_LEN_ADDR   1

#define WiFi_SSID_ADDR      2
#define WiFi_PSW_ADDR      34


typedef struct WiFi_Obj_s{
  unsigned int ssid_len;
  unsigned int psw_len;
  String       wifi_ssid;
  String       wifi_psw;
}WiFi_Obj_t;


typedef void (*smartConfigCb) ();


class WifiManagement {
  public:
    WifiManagement();
    WifiManagement(const char* ssid, const char* password);
    ~WifiManagement();

    void connectWifi(int needConfig);
    void smartConfig();
    void setSmartCfgCb(smartConfigCb cb);
    void clearEEPROM();

  private:
    const char* _ssid = NULL;
    const char* _password = NULL;

    smartConfigCb _cb;

    WiFi_Obj_t _wifiValue;
    
    void getWifiFromEEPROM(WiFi_Obj_t *wifi_obj);
    void storeWifiToEEPROM();
};









#endif