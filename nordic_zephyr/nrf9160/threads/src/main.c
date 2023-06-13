/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(main,LOG_LEVEL_INF);

#define MY_STACK_SIZE 500
#define MID_PRIORITY 5

void blinky(void *,void*,void*)
{
	printk("I am thread ! %s\n", CONFIG_BOARD);
	
	return;
}

K_THREAD_DEFINE(blinkThread, MY_STACK_SIZE,
                blinky, NULL, NULL, NULL,
                MID_PRIORITY, 0, -1);  // waithing for the infinitite amounnt of time 


void main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);
	printk("I am great Ankit Bansal\r\n");

	// k_msleep(5 * 1000);

	// k_thread_start(blinkThread);
	static uint32_t factorial = 1;
	for(uint8_t index =1;index <=10;index ++)
	{
		factorial = factorial * index;
	}

	printk("Factorial is %d\r\n", factorial);

	LOG_ERR("I am error");
	LOG_WRN("I am warn");
	LOG_INF("I am info");
	LOG_DBG("I am debug");
}
