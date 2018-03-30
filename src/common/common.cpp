/*
 * Description:  common source file for debug init
 *
 * Author: ninja
 *
 * Date: 2018-03-07
 *
 */

 #include "common.h"

 void serial_init(){
   Serial.begin(DEBUG_BAUD_RATE);
 }
