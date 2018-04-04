/*
 Description:  device operate format, containing BLE Mesh comamnd and wifi device command
 
 Author: Xuesong
 
 Date: 2018-04-03
 
 */

#ifndef deviceFunctionFormat_H
#define deviceFunctionFormat_H


#include <Arduino.h>


#define MESH_DEVICE_FUNCTION_PARA_LENGTH 10


/*
 * BLE mesh devices function data
*/
typedef enum {
    MESH_DEVICE_FUNCTION_ONOFF = 0,
}MESH_DEVICE_FUNCTION_TYPE;

typedef struct {
    MESH_DEVICE_FUNCTION_TYPE funcType;
    byte funcPara[MESH_DEVICE_FUNCTION_PARA_LENGTH];
}MESH_DEVICE_FUNCTION_DATA;

/*
 * wifi devices function data
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

typedef union {
    ushort para;
    struct colorPara {
        ushort h;
        ushort s;
        ushort v;
    };
}WIFI_DEVICE_FUNCTION_PARA;

typedef struct {
    WIFI_DEVICE_FUNCTION_TYPE funcType;
    WIFI_DEVICE_FUNCTION_PARA funcPara;
}WIFI_DEVICE_FUNCTION_DATA;

/*
 * communicate device function data through uart
*/
typedef enum {
    PROTOCOL_TYPE_OPERATE_MESH_AGENT = 0,   //ble mesh agent
    PROTOCOL_TYPE_OPERATE_WIFI_DEVICE = 1,  //mainly wifi device
    PROTOCOL_TYPE_MESH_AGENT_STATUS = 2, 
    PROTOCOL_TYPE_WIFI_DEVICE_STATUS = 3, 
}UART_PROTOCOL_TYPE;

typedef union {
    MESH_DEVICE_FUNCTION_DATA meshData;
    WIFI_DEVICE_FUNCTION_DATA wifiData;
}UART_PROTOCOL_PAYLOAD;

typedef struct {
    UART_PROTOCOL_TYPE protType;
    UART_PROTOCOL_PAYLOAD protPayload;
}UART_PROTOCOL_DATA;


#endif