/**
 * @file main.c
 * @author Ankit Bansal (iotdevelope@gmail.com)
 * @brief Async uart driver implementation 
 * @version 1.1
 * @date 2023-05-21
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <string.h>
#include <zephyr/logging/log.h>



#define UART_INSTANCE_DEMO DT_NODELABEL(uart1)
LOG_MODULE_REGISTER(MAGPIO_PIN_NUM,LOG_LEVEL_INF);

/**
 * @brief Getting the uart0 instance to be used in the example
 * 
 */
const struct device *uart = DEVICE_DT_GET(UART_INSTANCE_DEMO);

static uint8_t rx_buf[10] = {0}; //A buffer to store incoming UART data

static uint8_t data[10] = {0};
static uint8_t length =0;

/**
 * @brief uart call back function
 * 
 * @param dev 
 * @param evt 
 * @param user_data 
 */
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
	switch (evt->type) {
	
	case UART_TX_DONE:
		LOG_INF("UART_TX_DONE");
		break;

	case UART_TX_ABORTED:
		LOG_INF("UART_TX_ABORTED");
		break;
		
	case UART_RX_RDY:
		LOG_INF("UART_RX_RDY");
		length = evt->data.rx.len;
		memcpy(data,evt->data.rx.buf + evt->data.rx.offset,evt->data.rx.len);
		break;

	case UART_RX_BUF_REQUEST:
		LOG_INF("UART_RX_BUF_REQUEST");
		break;

	case UART_RX_BUF_RELEASED:
		LOG_INF("UART_RX_BUF_RELEASED");
		break;
		
	case UART_RX_DISABLED:
		LOG_INF("UART_RX_DISABLED");
		memset(rx_buf,0,sizeof rx_buf);
		uart_rx_enable(dev, rx_buf, sizeof(rx_buf), 100);
		break;

	case UART_RX_STOPPED:
		LOG_INF("UART_RX_STOPPED");
		break;
		
	default:
		break;
	}
}



void main()
{	

	LOG_INF("Hello System");
	/**
	 * @brief  check if device is ready or not
	 * 
	 */
	if(!device_is_ready(uart))
	{
		return;
	}
	
	
	const struct uart_config uart_cfg = {
		.baudrate = 115200,
		.parity = UART_CFG_PARITY_NONE,
		.stop_bits = UART_CFG_STOP_BITS_1,
		.data_bits = UART_CFG_DATA_BITS_8,
		.flow_ctrl = UART_CFG_FLOW_CTRL_NONE
	};

	int err = uart_configure(uart, &uart_cfg);

	if (err == -ENOSYS) {
		return -ENOSYS;
	}

	// regisering the call back 
	err = uart_callback_set(uart, uart_cb, NULL);
	if (err)
	{
		return err;
	}

	// enabling the receiving of the data 
	err = uart_rx_enable(uart ,rx_buf,sizeof(rx_buf),100);  // timeout is 100 microseconds
	if (err) {
		return err;
	}
	static uint8_t tx_buf[] =  {"Hello I am uart!! \n\r"};

	err = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);
	if (err) {
		return err;
	}

}

