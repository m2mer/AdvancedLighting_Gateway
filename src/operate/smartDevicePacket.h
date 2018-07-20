/*
 * smartDevicePacket.h
 * This file define format of mesh command in MQTT message
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
	SMART_SERVICE_DEFAULT = 0,
	SMART_LIGHT_TYPE_W = 1,
	SMART_LIGHT_TYPE_WC = 2,
	SMART_LIGHT_TYPE_RGB = 3,
	SMART_LIGHT_TYPE_RGBW = 4,
	SMART_LIGHT_TYPE_RGBWC = 5,
}SMART_SERVICE_TYPE;

typedef struct {
	uint8_t firstType;
	uint8_t secondType;
}DEVICE_TYPE;

typedef enum {
	SMART_LIGHT_MODE_W = 1,
	SMART_LIGHT_MODE_WC = 2,
	SMART_LIGHT_MODE_RGB = 3,
	SMART_LIGHT_MODE_SCENE = 4,
}SMART_LIGHT_MODE;

typedef struct {
	uint16_t temperature;
	uint8_t lightness;
	uint8_t reserved;
}DEVICE_CT;

typedef struct {
    uint16_t h;
    uint8_t s;
    uint8_t v;
}DEVICE_COLOR;

typedef struct {
    uint8_t onoff;
    uint8_t lightness;
    uint8_t group;
    uint8_t reserved;
}DEVICE_BRIEF;

typedef union {
	uint8_t offline;
    uint8_t onoff;
    uint8_t lightness;
    uint8_t mode;
    uint8_t group;
    DEVICE_CT temperature;
    DEVICE_COLOR color;
    DEVICE_BRIEF brief;
    uint16_t timerOn;
    uint16_t timerOff;
}DEVICE_FUNCTION_PARA;

typedef enum
{
	DEVICE_RESET_SOFTWARE_DELETED = 0,
	DEVICE_RESET_HARDWARE_RESET = 1,
}DEVICE_RESET_FLAG;

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
	MESH_DEVICE_FUNCTION_TIMER_SET = 5,
	MESH_DEVICE_FUNCTION_TIMER_DEL = 6,
	MESH_DEVICE_FUNCTION_TIMER_ON = 7,
	MESH_DEVICE_FUNCTION_TIMER_OFF = 8,
	MESH_DEVICE_FUNCTION_SCENE_SET = 9,
	MESH_DEVICE_FUNCTION_SCENE_CLEAR = 10,
	MESH_DEVICE_FUNCTION_SCENE_START = 11,
	MESH_DEVICE_FUNCTION_GROUP_CFG = 12,
	MESH_DEVICE_FUNCTION_OFFLINE = 13,
	MESH_DEVICE_FUNCTION_BRIEF = 14,
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
	uint8_t sequence;
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