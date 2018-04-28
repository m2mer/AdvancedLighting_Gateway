/*
 * mesh_device_packet.h
 *
 *  Created on: 2018-4-23
 *      Author: xuesong zhang
 */

#ifndef smartDevicePacket_H
#define smartDevicePacket_H


#include <Arduino.h>


typedef enum
{
    SMART_DEVICE_WIFI_LIGHT = 0,
	SMART_DEVICE_TYPE_MESH_GATEWAY = 1,
	SMART_DEVICE_TYPE_MESH_LIGHT = 2,
	SMART_DEVICE_TYPE_MESH_SWITCH = 3,
	SMART_DEVICE_TYPE_MESH_SENSOR = 4,
}SMART_DEVICE_TYPE;

typedef enum
{
	SMART_LIGHT_TYPE_WC = 1,
	SMART_LIGHT_TYPE_RGB = 2,
	SMART_LIGHT_TYPE_RGBW = 3,
	SMART_LIGHT_TYPE_RGBWC = 4,
}SMART_LIGHT_TYPE;

typedef struct {
	uint8_t firstType;
	uint8_t secondType;
}DEVICE_TYPE;


typedef struct {
    uint16_t h;
    uint8_t s;
    uint8_t v;
}DEVICE_COLOR;

typedef union {
    uint8_t onoff;
    uint8_t lightness;
    uint16_t temperature;
    DEVICE_COLOR color;
    uint8_t mode;
    uint16_t timerOn;
    uint16_t timerOff;
}DEVICE_FUNCTION_PARA;

typedef struct {
	uint8_t mac[6];
	uint8_t firstType;
	uint8_t secondType;
    uint8_t group;
    uint8_t onoff;
    uint8_t lightness;
    uint8_t mode;
	uint16_t temperature;
	struct{
        uint16_t h;
        uint8_t s;
        uint8_t v;
    }color;
	uint16_t timerOn;
    uint16_t timerOff;
}MESH_NODE_OVERALL_STATUS;


/*
 * BLE mesh devices function type
*/
typedef enum {
	MESH_DEVICE_FUNCTION_ONOFF = 0,
	MESH_DEVICE_FUNCTION_LIGHTNESS = 1,
	MESH_DEVICE_FUNCTION_TEMPERATURE = 2,
	MESH_DEVICE_FUNCTION_COLOR = 3,
	MESH_DEVICE_FUNCTION_MODE = 4,
	MESH_DEVICE_FUNCTION_TIMER_ON = 5,
	MESH_DEVICE_FUNCTION_TIMER_OFF = 6,
	MESH_DEVICE_FUNCTION_GROUP_CFG = 7,
	MESH_DEVICE_FUNCTION_HEARTBEAT = 8,
	MESH_DEVICE_FUNCTION_HEELIGHT = 9,
}MESH_DEVICE_FUNCTION_TYPE;



/* 
 * down direction 
 */

typedef struct {
	uint8_t command;
	uint8_t reserved;    //for padding
	uint8_t mac[6];
	uint8_t funcType;
	uint8_t funcPara[5];
}MESH_DEVICE_OPERATION;

typedef struct {
	uint8_t command;
	uint8_t reserved;    //for padding
	uint8_t mac[6];
}MESH_DEVICE_GET_STATUS;


/* 
 * up direction 
 */

typedef struct {
	uint8_t command;
	uint8_t reserved;    //for padding	
	uint8_t mac[6];
	uint8_t sequence;
	uint8_t funcType;
	DEVICE_FUNCTION_PARA status;
}MESH_DEVICE_STATUS_UPDATE;

typedef struct {
	uint8_t command;
	uint8_t sequence;
	MESH_NODE_OVERALL_STATUS status;
}MESH_DEVICE_OVERALL_STATUS;


#endif /* smartDevicePacket_H */ 