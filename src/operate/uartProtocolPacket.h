/*
 Description:  device operate format, containing BLE Mesh comamnd and wifi device command
 
 Author: Xuesong
 
 Date: 2018-04-03
 
 */

#ifndef uartProtocolPacket_H
#define uartProtocolPacket_H


#include <Arduino.h>
#include "meshCommandPacket.h"


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
}MESH_COMMAND_PARA;

typedef struct {
	uint8_t meshCmd;
	uint8_t reserved;    //for padding
	MESH_COMMAND_PARA cmdPara;
}MESH_DEVICE_COMMAND_DATA;


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