/*
 Description:  Build MQTT transport layer
 
 Author: Xuesong
 
 Date: 2018-03-30
 
 */

#include <ESP8266WiFi.h>

#include "MQTTtransport.h"
#include "common/common.h"

const char *pubTopic = "esp8266.hello_word";
const char *subTopic = "device/device_operate";



MQTTtransport::MQTTtransport(Client& client):PubSubClient(client) {
}

MQTTtransport::~MQTTtransport() {
}

void MQTTtransport::setup(const char *server, const char *client, receiveMsgCb cb) {
    Serial.println(TimeStamp + "start MQTTSetup...");
    
    this->_clientId = String("ESP8266-")+String(client);
    
    this->setServer(server, SSDP_PORT);
    this->setCallback(cb);
}


void MQTTtransport::reconnect() {
  // Loop until we're reconnected
  if(!this->connected()) {
    #ifdef DEBUG_MQTT
    DEBUG_MQTT.println(TimeStamp + "Attempting MQTT connection...");
    DEBUG_MQTT.print("Wifi status: ");
    DEBUG_MQTT.println(WiFi.status());
    DEBUG_MQTT.print("ip: ");
    DEBUG_MQTT.println(WiFi.localIP());
    #endif

    // Attempt to connect
    if (this->connect(this->_clientId.c_str())) {
    #ifdef DEBUG_MQTT
      DEBUG_MQTT.println(TimeStamp + "mqtt broker connected");
      DEBUG_MQTT.print("Wifi status: ");
      DEBUG_MQTT.println(WiFi.status());
      DEBUG_MQTT.print("ip: ");
      DEBUG_MQTT.println(WiFi.localIP());      
    #endif
      // Once connected, publish an announcement...
      this->publish(pubTopic, "hello world");
      this->subscribe(subTopic);
    } 
    else 
    {

    #ifdef DEBUG_MQTT
      DEBUG_MQTT.print(TimeStamp + "failed, rc=");
      DEBUG_MQTT.print(this->state());
      DEBUG_MQTT.println(" try again in 3 seconds");
    #endif
      delay(3000);
    }
  }
}