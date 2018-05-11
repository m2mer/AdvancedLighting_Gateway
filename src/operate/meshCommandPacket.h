/*
 * meshCommandPacket.h
 * This file defines mesh commands in mesh network
 *
 *  Created on: 2018-4-24
 *      Author: xuesong zhang
 */


#ifndef meshCommandPacket_H
#define meshCommandPacket_H


#include <Arduino.h>
#include "smartDevicePacket.h"



//------0x30 ~ 0x3f for customer

//down direction
#define         LGT_CMD_ADVLIGHT_DEVICE_OP          0x32
#define         LGT_CMD_ADVLIGHT_GROUP_OP           0x33
#define         LGT_CMD_ADVLIGHT_GET_STATUS         0x34
#define         LGT_CMD_ADVLIGHT_GET_GROUP          0x35
//up direction
#define         LGT_CMD_ADVLIGHT_STATUS_UPDT        0x36
#define         LGT_CMD_ADVLIGHT_GROUP_UPDT         0x37
#define         LGT_CMD_ADVLIGHT_OVERALL_STATUS     0x38
#define         LGT_CMD_ADVLIGHT_GROUP_STATUS       0x39
#define         LGT_CMD_ADVLIGHT_RESET_FACTORY      0x3a




typedef struct {
	uint8_t funcType;
	uint8_t funcPara[5];           //compatible for heelight command
}MESH_COMMAND_OPERATION;

typedef struct {
	uint16_t gwDevAddr;
	uint16_t ndDevAddr;
	uint8_t mac[6];
}MESH_COMMAND_GET_STATUS;

typedef struct {
	uint8_t sequence;
	uint8_t funcType;
	DEVICE_FUNCTION_PARA funcPara;
}MESH_COMMAND_STATUS_UPDATE;

typedef enum {
	MESH_OVERALL_STATUS_I = 0,
	MESH_OVERALL_STATUS_II = 1,
	MESH_OVERALL_STATUS_III = 2,
	MESH_OVERALL_STATUS_IV = 3,	
}MESH_OVERALL_STATUS_INDEX;

/*
 * 8 bytes status, 2 bytes kept for segment and sequence
 * */
typedef struct {
	uint8_t mac[6];
	uint8_t firstType;
	uint8_t secondType;
}MESH_COMMAND_OVERALL_STATUS_I;

typedef struct {
    uint8_t group;
    uint8_t onoff;
    uint8_t lightness;
    uint8_t mode;
	union{
	    uint16_t temperature;
	    struct{
	        uint16_t h;
	        uint8_t s;
	        uint8_t v;
	    }color;
	}rgbcw;
}MESH_COMMAND_OVERALL_STATUS_II;

typedef struct {
	uint16_t deviceAddr;
    uint16_t temperature;
    struct{
        uint16_t h;
        uint8_t s;
        uint8_t v;
    }color;
}MESH_COMMAND_OVERALL_STATUS_III;

typedef struct {
	uint16_t deviceAddr;
    uint16_t timerOn;
    uint16_t timerOff;
}MESH_COMMAND_OVERALL_STATUS_IV;

typedef struct {
	uint8_t sequence;
	uint8_t segment;
	union {
		MESH_COMMAND_OVERALL_STATUS_I statusI;
		MESH_COMMAND_OVERALL_STATUS_II statusII;
		MESH_COMMAND_OVERALL_STATUS_III statusIII;
		MESH_COMMAND_OVERALL_STATUS_IV statusIV;
	}status;
}MESH_COMMAND_OVERALL_STATUS;


typedef struct {
	uint8_t sequence;
	uint8_t reserved;	//padding
}MESH_COMMAND_RESET_FACTORY;

#endif