/*
 Description:  Fetch and manage device's network address
 
 Author: Xuesong
 
 Date: 2018-03-28
 
 */

#include "WifiManagement.h"
#include "common/common.h"
#include "GPIO/io.h"


WifiManagement::WifiManagement() {
}

WifiManagement::WifiManagement(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;
}

WifiManagement::~WifiManagement() {
}

void WifiManagement::setSmartCfgCb(smartConfigCb cb) {
    _cb = cb;
}

void WifiManagement::connectWifi() {
    char* ssid = NULL;
    char* password = NULL;
    unsigned int timeOut = 20;
    boolean ledOn = false;

    //WiFi.mode(WIFI_STA);

    if(this->_ssid && this->_password) {
        ssid = (char*)this->_ssid;
        password = (char*)this->_password;
        #ifdef DEBUG_WiFi
            DEBUG_WiFi.println("");
            DEBUG_WiFi.printf("default SSID: %s\n", ssid);
            DEBUG_WiFi.printf("default PASSWORD: %s\n", password);
        #endif        
    }
    else {
        getWifiFromEEPROM(&this->_wifiValue);
        ssid = (char*)this->_wifiValue.wifi_ssid.c_str();
        password = (char*)this->_wifiValue.wifi_psw.c_str();
        #ifdef DEBUG_WiFi
            DEBUG_WiFi.println("");
            DEBUG_WiFi.printf("EEPROM SSID: %s\n", ssid);
            DEBUG_WiFi.printf("EEPROM PASSWORD: %s\n", password);
        #endif
    }

    if((*ssid != 0) && (*password != 0)) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);

        while(timeOut --){
            if(WiFi.status() != WL_CONNECTED){
                Serial.print(".");
                ledOn = !ledOn;
                if(ledOn)
                    digitalWrite(LED_WIFI, HIGH);
                else
                    digitalWrite(LED_WIFI, LOW);
            }
            if(WiFi.status() == WL_CONNECTED){
                Serial.println("wifi connected...");
                digitalWrite(LED_WIFI, HIGH);
                break;
            }
            delay(500);
        }
    }

    if(WiFi.status() == WL_CONNECTED) {
        Serial.println("Wifi already connected...");
        Serial.print("ip: ");
        Serial.println(WiFi.localIP());
    }
    else {
        digitalWrite(LED_WIFI, LOW);
        smartConfig();
    }
}

void WifiManagement::smartConfig(){
    boolean ledOn = false;

    WiFi.mode(WIFI_STA);
    WiFi.beginSmartConfig();
    Serial.println("start SmartConfig...");
  
    while(1){
        Serial.print(".");
        ledOn = !ledOn;
        if(ledOn)
            digitalWrite(LED_WIFI, HIGH);
        else
            digitalWrite(LED_WIFI, LOW);       
        delay(1000);
    
        if(WiFi.smartConfigDone()){
            digitalWrite(LED_BUILTIN, HIGH);
            #ifdef DEBUG_WiFi
            DEBUG_WiFi.println();
            DEBUG_WiFi.println("SmartConfig succes!!!");
            DEBUG_WiFi.printf("SSID: %s\r\n", WiFi.SSID().c_str());
            DEBUG_WiFi.printf("PSW: %s\r\n", WiFi.psk().c_str());
            #endif

            /* smartconfig done callback */
            _cb();

            /* store to EEPROM */
            clearEEPROM();
            storeWifiToEEPROM();
            break;
        }
    }

}

void WifiManagement::clearEEPROM(){
    EEPROM.begin(512);
    for(unsigned int i=0;i<512;i++){
        EEPROM.write(i,0);
    }
    EEPROM.end();
}

void WifiManagement::getWifiFromEEPROM(WiFi_Obj_t *wifi_obj) {
    String ssid;
    String psw;
    EEPROM.begin(512);
    unsigned int ssid_len = EEPROM.read(WiFi_SSID_LEN_ADDR);
    unsigned int psw_len = EEPROM.read(WiFi_PSW_LEN_ADDR);

    for(unsigned int i=0; i<ssid_len; i++){
      ssid += (char)EEPROM.read(WiFi_SSID_ADDR + i);
    }

    for(unsigned int j=0; j<psw_len; j++){
      psw += (char)EEPROM.read(WiFi_PSW_ADDR + j);
    }

    EEPROM.commit();
    EEPROM.end();
    wifi_obj->ssid_len = ssid_len;
    wifi_obj->psw_len = psw_len;
    wifi_obj->wifi_ssid = ssid;
    wifi_obj->wifi_psw = psw;

#ifdef DEBUG_WiFi
    DEBUG_WiFi.println("get wifi from eeprom...");
    DEBUG_WiFi.printf("ssid len: %d\n", ssid_len);
    DEBUG_WiFi.printf("psw len: %d\n", psw_len);
    DEBUG_WiFi.printf("ssid: %s\n", ssid.c_str());
    DEBUG_WiFi.printf("psw: %s\n", psw.c_str());
#endif

}

void WifiManagement::storeWifiToEEPROM(){
    unsigned int ssid_len = WiFi.SSID().length();
    unsigned int psw_len  = WiFi.psk().length();
    EEPROM.begin(512);

    EEPROM.write(WiFi_SSID_LEN_ADDR, ssid_len);
    EEPROM.write(WiFi_PSW_LEN_ADDR, psw_len);

    String ssid = WiFi.SSID();
    String psw = WiFi.psk();

    for(unsigned int i=0;i<ssid_len;i++){
        EEPROM.write(WiFi_SSID_ADDR+i, ssid[i]);
    }

    for(unsigned int j=0;j<psw_len;j++){
        EEPROM.write(WiFi_PSW_ADDR+j, psw[j]);
    }

    EEPROM.commit();
    EEPROM.end();
}