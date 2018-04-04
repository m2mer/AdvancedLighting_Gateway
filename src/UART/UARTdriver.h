/*
 Description:  Offer uart interfaces 
 
 Author: Xuesong
 
 Date: 2018-04-02
 
 */

#ifndef UARTdrvier_H
#define UARTdrvier_H


#include <Arduino.h>


typedef void (*uartRxHdl) (const char *buf, int len);

class UARTdriver {
    public:
        UARTdriver();
        UARTdriver(HardwareSerial* serial);
        ~UARTdriver(){};

        void setRxHdl(uartRxHdl hdl);
        void loop();
        int write(const char *buf, int len);
    
    private:
        HardwareSerial* _Serial;
        char _protData[256] = {0};
        int _protDataLen = 0;
        boolean _protDataComplete = false;
        uartRxHdl _rxHdl;
        void rxProcess();
        void rxProtocolData();

};





#endif