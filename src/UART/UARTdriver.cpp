/*
 Description:  Offer uart interfaces 
 
 Author: Xuesong
 
 Date: 2018-04-02
 
 */

#include "UARTdriver.h"
#include <Arduino.h>

UARTdriver::UARTdriver() {
    _Serial = &Serial;
}

UARTdriver::UARTdriver(HardwareSerial* serial) {
    _Serial = serial;
}

void UARTdriver::setRxHdl(uartRxHdl hdl) {
    this->_rxHdl = hdl;
}

void UARTdriver::loop() {

    rxProtocolData();
    rxProcess();
}

void UARTdriver::rxProtocolData() {  
    if (this->_protDataComplete) {
        this->_rxHdl(this->_protData, this->_protDataLen);
        // clear the data
        memset(this->_protData, 0, sizeof(this->_protData));
        this->_protDataLen = 0;
        this->_protDataComplete = false;        
  }

}

void UARTdriver::rxProcess()
{
    while (_Serial->available()) {
        // get the new byte:
        char inChar = (char)_Serial->read();
        // add it to the inputString:
        this->_protData[this->_protDataLen++] = inChar;

        //protocol data, end with \n
        //if (inChar == '\n' || inChar == '\r') {
            //this->_protDataComplete = true;
        //}
    }
    if(this->_protDataLen != 0)
        this->_protDataComplete = true;    

}


int UARTdriver::write(const char *buf, int len) {

    return _Serial->write(buf, len);
}


