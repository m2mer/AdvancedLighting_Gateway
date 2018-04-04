/*
 Description:  Build MQTT transport layer
 
 Author: Xuesong
 
 Date: 2018-03-30
 
 */

#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


#define SSDP_PORT 1883


typedef void (*receiveMsgCb)(char* topic, byte* payload, unsigned int length);

class MQTTtransport:public PubSubClient {
    public:
        MQTTtransport(Client& client);
        ~MQTTtransport();

    void setup(const char *server, const char *client, receiveMsgCb cb);
    void reconnect();

  private:
    String _clientId;
};
