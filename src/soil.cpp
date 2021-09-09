/**
 * @file soil.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Soil sensor initialization and readings
 * @version 0.1
 * @date 2021-08-17
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "main.h"

/** Sensor */
I2CSoilMoistureSensor sensor;

soil_data_s g_soil_data;

bool init_soil(void)
{
	// Check if sensors is available
	bool found_sensor = false;
	Wire.begin();
	for (uint8_t adr = 0; adr < 127; adr++)
	{
		Wire.beginTransmission(adr);
		if (Wire.endTransmission() == 0)
		{
			MYLOG("SOIL", "Found device on address %02x\n", adr);
			found_sensor = true;
		}
	}
	Wire.end();

	Wire.begin();
	sensor.begin(true);
	MYLOG("SOIL", "%0x",sensor.getVersion());
	sensor.sleep();
	return found_sensor;
}

void read_soil(void)
{
	float sensTemp;
	uint16_t sensHumid;
	float avgTemp;
	uint16_t avgHumid;

	// Get the sensor values
	while (sensor.isBusy())
	{
		delay(100);
	}

	sensTemp = sensor.getTemperature();
	sensHumid = sensor.getCapacitance();

	avgTemp = sensTemp;
	avgHumid = sensHumid;

	// Do 10 readings to get an average
	for (int avg = 0; avg < 10; avg++)
	{
		while (sensor.isBusy())
		{
			delay(100);
		}
		delay(100);
		sensTemp = sensor.getTemperature();
		if ((sensTemp != NAN) && (sensTemp != 255.255))
		{
			avgTemp += sensTemp;
			avgTemp /= 2;
		}

		sensHumid = sensor.getCapacitance();
		if ((sensHumid != NAN) && (sensHumid != 255.255))
		{
			avgHumid += sensHumid;
			avgHumid /= 2;
		}
	}

	avgTemp /= 10;

	uint16_t avgTemp_int = uint16_t(avgTemp * 100.0);
	uint16_t avgHumid_int = uint16_t(humidInPerc(avgHumid) * 100.0);

	MYLOG("SOIL", "T %.2f H %.2f", avgTemp, humidInPerc(avgHumid));
	MYLOG("SOIL", "T %.2f Hraw %d", avgTemp, avgHumid);
	if (ble_uart_is_connected)
	{
		ble_uart.printf("SW V %0x\n", sensor.getVersion());
		ble_uart.printf("T %.2f H %.2f\n", avgTemp, humidInPerc(avgHumid));
		ble_uart.printf("T %.2f Hraw %d\n", avgTemp, avgHumid);
	}

	g_soil_data.tempInt = (uint8_t)(avgTemp_int >> 8);
	g_soil_data.tempFrac = (uint8_t)(avgTemp_int);

	g_soil_data.humidInt = (uint8_t)(avgHumid_int >> 8);
	g_soil_data.humidFrac = (uint8_t)(avgHumid_int);
	g_soil_data.messInt = (uint8_t)(avgHumid >> 8);
	g_soil_data.messFrac = (uint8_t)(avgHumid);

	sensor.sleep();
}

float humidInPerc(uint16_t analogVal)
{
	// Analog moisture sensor
	// 250 ==> 0%
	// 500 ==> 100%

	return (analogVal - 250) * 0.4;

	if (analogVal >= 440)
	{
		return 100;
	}
	else if (analogVal >= 431)
	{
		return 95;
	}
	else if (analogVal >= 421)
	{
		return 90;
	}
	else if (analogVal >= 412)
	{
		return 85;
	}
	else if (analogVal >= 402)
	{
		return 80;
	}
	else if (analogVal >= 393)
	{
		return 75;
	}
	else if (analogVal >= 383)
	{
		return 70;
	}
	else if (analogVal >= 374)
	{
		return 65;
	}
	else if (analogVal >= 364)
	{
		return 60;
	}
	else if (analogVal >= 355)
	{
		return 55;
	}
	else if (analogVal >= 345)
	{
		return 50;
	}
	else if (analogVal >= 336)
	{
		return 45;
	}
	else if (analogVal >= 326)
	{
		return 40;
	}
	else if (analogVal >= 317)
	{
		return 35;
	}
	else if (analogVal >= 307)
	{
		return 30;
	}
	else if (analogVal >= 298)
	{
		return 25;
	}
	else if (analogVal >= 288)
	{
		return 20;
	}
	else if (analogVal >= 279)
	{
		return 15;
	}
	else if (analogVal >= 269)
	{
		return 10;
	}
	else if (analogVal >= 260)
	{
		return 5;
	}
	return 0;
}
