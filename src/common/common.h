/*
 * Description:  common header file for debug
 *
 * Author: ninja
 *
 * Date: 2018-03-07
 *
 */

#ifndef __COMMON_H
#define __COMMON_H

#include <Arduino.h>


#define TimeStamp   (String("[") + String(millis()) + String("] "))

/* debug for every feature */
#define DEBUG_WiFi   Serial1
#define DEBUG_MQTT   Serial1
//#define DEBUG_JSON   Serial


#ifndef DEBUG_BAUD_RATE
#define DEBUG_BAUD_RATE   115200
#endif


typedef struct
{
    uint8_t networkConfiged;
    uint8_t registered;
}LOCAL_METADATA;

typedef struct
{
    uint8_t mac[6];
    uint8_t registered;
    uint8_t reserved;
}MESH_NODE_METADATA;



void serial_init();

#endif
