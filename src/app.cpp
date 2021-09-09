/**
 * @file app.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Application specific functions. Mandatory to have init_app(), 
 *        app_event_handler(), ble_data_handler(), lora_data_handler()
 *        and lora_tx_finished()
 * @version 0.1
 * @date 2021-04-23
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "main.h"

/** Set the device name, max length is 10 characters */
char ble_dev_name[10] = "RAK-SOIL";

/** Flag showing if TX cycle is ongoing */
bool lora_busy = false;

/** Required for give semaphore from ISR */
BaseType_t g_higher_priority_task_woken = pdTRUE;

/** Battery level uinion */
batt_s batt_level;

/** Send Fail counter **/
uint8_t send_fail = 0;

/**
 * @brief Application specific setup functions
 * 
 */
void setup_app(void)
{
	// Enable BLE
	enable_ble = true;
}

/**
 * @brief Application specific initializations
 * 
 * @return true Initialization success
 * @return false Initialization failure
 */
bool init_app(void)
{
	MYLOG("APP", "init_app");

	// Initialize Soil module
	return init_soil();
}

/**
 * @brief Application specific event handler
 *        Requires as minimum the handling of STATUS event
 *        Here you handle as well your application specific events
 */
void app_event_handler(void)
{
	// Timer triggered event
	if ((g_task_event_type & STATUS) == STATUS)
	{
		g_task_event_type &= N_STATUS;
		MYLOG("APP", "Timer wakeup");

		// If BLE is enabled, restart Advertising
		if (enable_ble)
		{
			restart_advertising(15);
		}

		if (lora_busy)
		{
			MYLOG("APP", "LoRaWAN TX cycle not finished, skip this event");
			if (ble_uart_is_connected)
			{
				ble_uart.println("LoRaWAN TX cycle not finished, skip this event");
			}
		}
		else
		{
			// Get soil sensor values
			read_soil();

			// Get battery level
			batt_level.batt16 = read_batt() / 10;
			g_soil_data.batt_1 = batt_level.batt8[1];
			g_soil_data.batt_2 = batt_level.batt8[0];

			lmh_error_status result = send_lora_packet((uint8_t *)&g_soil_data, SOIL_DATA_LEN);
			switch (result)
			{
			case LMH_SUCCESS:
				MYLOG("APP", "Packet enqueued");
				/// \todo set a flag that TX cycle is running
				lora_busy = true;
				if (ble_uart_is_connected)
				{
					ble_uart.println("Packet enqueued");
				}
				break;
			case LMH_BUSY:
				MYLOG("APP", "LoRa transceiver is busy");
				if (ble_uart_is_connected)
				{
					ble_uart.println("LoRa transceiver is busy");
				}
				break;
			case LMH_ERROR:
				MYLOG("APP", "Packet error, too big to send with current DR");
				if (ble_uart_is_connected)
				{
					ble_uart.println("Packet error, too big to send with current DR");
				}
				break;
			}
		}
	}
}

/**
 * @brief Handle BLE UART data
 * 
 */
void ble_data_handler(void)
{
	if (enable_ble)
	{
		// BLE UART data handling
		if ((g_task_event_type & BLE_DATA) == BLE_DATA)
		{
			MYLOG("AT", "RECEIVED BLE");
			/** BLE UART data arrived */
			g_task_event_type &= N_BLE_DATA;

			while (ble_uart.available() > 0)
			{
				at_serial_input(uint8_t(ble_uart.read()));
				delay(5);
			}
			at_serial_input(uint8_t('\n'));
		}
	}
}

/**
 * @brief Handle received LoRa Data
 * 
 */
void lora_data_handler(void)
{
	// LoRa data handling
	if ((g_task_event_type & LORA_DATA) == LORA_DATA)
	{
		/**************************************************************/
		/**************************************************************/
		/// \todo LoRa data arrived
		/// \todo parse them here
		/**************************************************************/
		/**************************************************************/
		g_task_event_type &= N_LORA_DATA;
		MYLOG("APP", "Received package over LoRa");
		char log_buff[g_rx_data_len * 3] = {0};
		uint8_t log_idx = 0;
		for (int idx = 0; idx < g_rx_data_len; idx++)
		{
			sprintf(&log_buff[log_idx], "%02X ", g_rx_lora_data[idx]);
			log_idx += 3;
		}
		lora_busy = false;
		MYLOG("APP", "%s", log_buff);

		if (ble_uart_is_connected && enable_ble)
		{
			for (int idx = 0; idx < g_rx_data_len; idx++)
			{
				ble_uart.printf("%02X ", g_rx_lora_data[idx]);
			}
			ble_uart.println("");
		}
	}

	// LoRa TX finished handling
	if ((g_task_event_type & LORA_TX_FIN) == LORA_TX_FIN)
	{
		g_task_event_type &= N_LORA_TX_FIN;

		MYLOG("APP", "LPWAN TX cycle %s", rx_fin_result ? "finished ACK" : "failed NAK");
		if (ble_uart_is_connected)
		{
			ble_uart.printf("LPWAN TX cycle %s", rx_fin_result ? "finished ACK" : "failed NAK");
		}

		if (!rx_fin_result)
		{
			// Increase fail send counter
			send_fail++;

			if (send_fail == 10)
			{
				// Too many failed sendings, reset node and try to rejoin
				delay(100);
				sd_nvic_SystemReset();
			}
		}
		/// \todo reset flag that TX cycle is running
		lora_busy = false;
	}
}

void tud_cdc_rx_cb(uint8_t itf)
{
	g_task_event_type |= AT_CMD;
	if (g_task_sem != NULL)
	{
		xSemaphoreGiveFromISR(g_task_sem, pdFALSE);
	}
}
