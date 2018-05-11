/*
 * wifiCommandPacket.h
 * This file defines wifi commands for uart peer wifi device
 *
 *  Created on: 2018-4-30
 *      Author: xuesong zhang
 */


#ifndef wifiCommandPacket_H
#define wifiCommandPacket_H


#include <Arduino.h>
#include "smartDevicePacket.h"



/*
 * wifi devices command type
*/
typedef enum {
    WIFI_DEVICE_COMMAND_OPERATION = 1,
    WIFI_DEVICE_COMMAND_GET_STATUS = 2,
    WIFI_DEVICE_COMMAND_STATUS_UPDATE = 3,
    WIFI_DEVICE_COMMAND_OVERALL_STATUS = 4,
}WIFI_DEVICE_COMMAND_TYPE;

/*
 * wifi devices function type
*/
typedef enum {
    WIFI_DEVICE_FUNCTION_ONOFF = 0,
    WIFI_DEVICE_FUNCTION_LIGHTNESS = 1,
    WIFI_DEVICE_FUNCTION_TEMPERATURE = 2,
    WIFI_DEVICE_FUNCTION_COLOR = 3,
    WIFI_DEVICE_FUNCTION_MODE = 4,
    WIFI_DEVICE_FUNCTION_TIMER_ON = 5,
    WIFI_DEVICE_FUNCTION_TIMER_OFF = 6,
}WIFI_DEVICE_FUNCTION_TYPE;

typedef struct {
    uint8_t onoff;
    uint8_t lightness;
    uint16_t temperature;
    DEVICE_COLOR color;
    uint8_t mode;
    uint8_t reserved;   //for padding
    uint16_t timerOn;
    uint16_t timerOff;
}WIFI_DEVICE_OVERALL_STATUS;



#endif