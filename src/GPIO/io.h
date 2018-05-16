#ifndef IO_H
#define IO_H

#include <Arduino.h>

#define BUTTON_RESET  5  //D1

#define LED_WIFI  14  //nodemcu D5
#define LED_MQTT  12  //nodemcu D6

#define LED_WIFI_ON   digitalWrite(LED_WIFI, LOW)
#define LED_WIFI_OFF  digitalWrite(LED_WIFI, HIGH)

#define LED_MQTT_ON   digitalWrite(LED_MQTT, LOW)
#define LED_MQTT_OFF  digitalWrite(LED_MQTT, HIGH)


typedef enum state_s{
  ON,
  OFF
}state_t;


void portInit();
void TurnON(state_t *state);
void TurnOFF(state_t *state);


#endif