/*
 Description: handle envent like button click, long press end etc.
 
 Author: Xuesong
 
 Date: 2018-05-16
 
 */

#include "interruptEvent.h"


void intr_register(uint8_t pin, INTR_HDL hdl)
{

    pinMode(pin, INPUT_PULLUP);
    attachInterrupt(pin, hdl, FALLING);
}