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

typedef struct {
    ushort onoff;
    ushort lightness;
    ushort temperature;
    struct{
        ushort h;
        ushort s;
        ushort v;        
    }color;
    ushort mode;
    ushort timer_on;
    ushort timer_off;
}WIFI_DEVICE_OVERALL_STATUS;

typedef struct {
    ushort onoff;
    ushort lightness;
    ushort temperature;
    ushort mode;
}WIFI_DEVICE_BRIEF_STATUS;

typedef union {
    ushort para;
    struct{
        ushort h;
        ushort s;
        ushort v;
    }colorPara;
    WIFI_DEVICE_OVERALL_STATUS overallStatus;
    WIFI_DEVICE_BRIEF_STATUS briefStatus;
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
    PROTOCOL_TYPE_GET_MESH_AGENT = 2,  
    PROTOCOL_TYPE_GET_WIFI_DEVICE = 3,     
    PROTOCOL_TYPE_MESH_AGENT_STATUS = 4,    //status after operation
    PROTOCOL_TYPE_WIFI_DEVICE_STATUS = 5,
    PROTOCOL_TYPE_MESH_AGENT_OVERALL = 6,   //overall status after get_status
    PROTOCOL_TYPE_WIFI_DEVICE_OVERALL = 7,
    PROTOCOL_TYPE_MESH_AGENT_BRIEF = 8,   //brief status notify timely
    PROTOCOL_TYPE_WIFI_DEVICE_BRIEF = 9,             
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