/*
 * uartProtocolPacket.h
 * This file define format of mesh and wifi commands commiunicated on uart protocol
 *
 *  Created on: 2018-04-03
 *      Author: xuesong zhang
 */

#ifndef uartProtocolPacket_H
#define uartProtocolPacket_H


#include <Arduino.h>
#include "meshCommandPacket.h"
#include "wifiCommandPacket.h"



#define UART_PROTOCOL_HEAD "smart"
#define UART_PROTOCOL_TAIL "trams"

#define UART_PROTOCOL_HEAD_LEN sizeof(UART_PROTOCOL_HEAD)
#define UART_PROTOCOL_TAIL_LEN sizeof(UART_PROTOCOL_TAIL)
#define UART_PROTOCOL_FLAG_LEN (UART_PROTOCOL_HEAD_LEN+UART_PROTOCOL_TAIL_LEN)



typedef union {
    //down
	MESH_COMMAND_OPERATION operation;
    MESH_COMMAND_GET_STATUS getStatus;
    //up
	MESH_COMMAND_STATUS_UPDATE update;
	MESH_COMMAND_OVERALL_STATUS status;
    MESH_COMMAND_RESET_FACTORY rstFacty;
}MESH_COMMAND_PARA;

typedef struct {
	uint8_t meshCmd;
	uint8_t reserved;    //for padding
	uint16_t devAddr;    //dst when down, src when up
	MESH_COMMAND_PARA cmdPara;
}MESH_DEVICE_COMMAND_DATA;



typedef union {
    uint16_t para;
    DEVICE_COLOR colorPara;
    WIFI_DEVICE_OVERALL_STATUS overallStatus;
}WIFI_DEVICE_FUNCTION_PARA;

typedef struct {
    uint8_t cmdType;
	uint8_t funcType;
    WIFI_DEVICE_FUNCTION_PARA funcPara;
}WIFI_DEVICE_FUNCTION_DATA;

/*
 * communicate device function data through uart
*/
typedef enum {
    //down direction
    PROTOCOL_TYPE_OPERATE_MESH_AGENT = 0,   //ble mesh agent
    PROTOCOL_TYPE_OPERATE_WIFI_DEVICE = 1,  //mainly wifi device
    PROTOCOL_TYPE_GET_MESH_AGENT = 2,
    PROTOCOL_TYPE_GET_WIFI_DEVICE = 3,
    //up direction
    PROTOCOL_TYPE_MESH_AGENT_STATUS = 4,    //status after operation
    PROTOCOL_TYPE_WIFI_DEVICE_STATUS = 5,
    PROTOCOL_TYPE_MESH_AGENT_OVERALL = 6,   //overall status after get_status
    PROTOCOL_TYPE_WIFI_DEVICE_OVERALL = 7,
    PROTOCOL_TYPE_MESH_AGENT_BRIEF = 8,   //brief status notify timely
    PROTOCOL_TYPE_WIFI_DEVICE_BRIEF = 9,
}UART_PROTOCOL_TYPE;

typedef union {
    MESH_DEVICE_COMMAND_DATA meshData;
    WIFI_DEVICE_FUNCTION_DATA wifiData;
}UART_PROTOCOL_PAYLOAD;

typedef struct {
	uint8_t protType;
    uint8_t reserved;    //for padding
    UART_PROTOCOL_PAYLOAD protPayload;
}UART_PROTOCOL_DATA;



#endif