/*
 Description: mesh device
 
 Author: Xuesong
 
 Date: 2018-04-27
 
 */



#ifndef wifiDevice_H
#define wifiDevice_H


#include <Arduino.h>
#include "../lib/LinkedList/LinkedList.h"


#include "smartDevice.h"
#include "smartDevicePacket.h"



class wifiDevice:public smartDevice
{

public:
    wifiDevice(byte *mac):smartDevice(mac){};
    ~wifiDevice(){};

    /* handle uplink message from peer uart*/
    void receiveUARTmsg(byte *buf, int len);

private:

    /* handle downlink mqtt message */
    int operateDevice(byte* payload, unsigned int length);
    int getOverallStatus(byte* payload, unsigned int length);

    /* handle message from peer uart */
    void recvStatusUpdate(byte *buf);
    void recvOverallStatus(byte *buf);

    void _parseAttributeValue(byte *buf, String& attribute, String& value);
};







#endif