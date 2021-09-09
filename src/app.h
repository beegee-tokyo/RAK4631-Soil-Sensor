/**
 * @file app.h
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief For application specific includes and definitions
 *        Will be included from main.h
 * @version 0.1
 * @date 2021-04-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef APP_H
#define APP_H

#include <Arduino.h>
/** Add you required includes after Arduino.h */
#include <Wire.h>

/** Application function definitions */
void setup_app(void);
bool init_app(void);
void app_event_handler(void);
void ble_data_handler(void) __attribute__((weak));
void lora_data_handler(void);

/** Application stuff */
extern BaseType_t g_higher_priority_task_woken;

/** Soil sensor stuff */
#include <I2CSoilMoistureSensor.h>
#define INT1_PIN WB_IO5
bool init_soil(void);
void read_soil(void);
float humidInPerc(uint16_t analogVal);
extern uint8_t tempInt;
extern uint8_t tempFrac;
extern uint8_t humidInt;
extern uint8_t humidFrac;


// LoRaWan functions
struct soil_data_s
{
	uint8_t data_flag1 = 0x02; // 1
	uint8_t tempInt;		   // 2
	uint8_t tempFrac;		   // 3
	uint8_t humidInt;		   // 4
	uint8_t humidFrac;		   // 5
	uint8_t batt_1;			   // 6
	uint8_t batt_2;			   // 7
	uint8_t messInt;           // 8
	uint8_t messFrac;          // 9
};
extern soil_data_s g_soil_data;
#define SOIL_DATA_LEN 9 // sizeof(g_soil_data)

/** Battery level uinion */
union batt_s
{
	uint16_t batt16 = 0;
	uint8_t batt8[2];
};

#endif