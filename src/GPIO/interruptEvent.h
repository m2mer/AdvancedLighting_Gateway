/*
 Description: handle envent like button click, long press end etc.
 
 Author: Xuesong
 
 Date: 2018-05-16
 
 */

#ifndef __INTERRUPT_EVENT_H
#define __INTERRUPT_EVENT_H

#include <Arduino.h>


typedef void (*INTR_HDL) ();

extern void intr_register(uint8_t pin, INTR_HDL hdl);





#endif