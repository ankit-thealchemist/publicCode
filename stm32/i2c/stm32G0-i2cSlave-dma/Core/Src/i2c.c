/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include <assert.h>
#include <string.h>
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */

#define WHO_AM_I 			0x75
#define REG_WRITE_DATA_1 	0x6B
#define REG_WRITE_DATA_2    0X6C

/**
 * In valid value
 */
uint8_t transfer_direction = 2;
uint8_t i2cBuf[15]={0};
uint8_t data_buffer[15] = {0};
HAL_StatusTypeDef status;
HAL_I2C_StateTypeDef i2c_status;

typedef enum{
	I2C_LISTEN_MODE,
	I2C_ADDRESS_MATCHED,
	I2C_REGISTER_ADDRESS_MATCHED,
	I2C_WRITE_REGISTER_VALUE,
	I2C_WRITE_ANSWER,
	I2C_STATE_MAX
} i2c_states;

i2c_states curr_state=I2C_LISTEN_MODE;
i2c_states last_state=I2C_STATE_MAX;

uint8_t i2c_write_len = 0;
uint8_t register_to_be_queired = 0;
__IO uint8_t ubSlaveReceiveIndex = 0;
void run_i2c_state_machine();
void prepare_i2c_answer(uint8_t regAddress);
/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_rx;
DMA_HandleTypeDef hdma_i2c1_tx;

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00303D5B;
  hi2c1.Init.OwnAddress1 = 64;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
    PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PA9     ------> I2C1_SCL
    PA10     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_I2C1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();

    /* I2C1 DMA Init */
    /* I2C1_RX Init */
    hdma_i2c1_rx.Instance = DMA1_Channel1;
    hdma_i2c1_rx.Init.Request = DMA_REQUEST_I2C1_RX;
    hdma_i2c1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_i2c1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_i2c1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_i2c1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_i2c1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_i2c1_rx.Init.Mode = DMA_NORMAL;
    hdma_i2c1_rx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_i2c1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(i2cHandle,hdmarx,hdma_i2c1_rx);

    /* I2C1_TX Init */
    hdma_i2c1_tx.Instance = DMA1_Channel2;
    hdma_i2c1_tx.Init.Request = DMA_REQUEST_I2C1_TX;
    hdma_i2c1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_i2c1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_i2c1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_i2c1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_i2c1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_i2c1_tx.Init.Mode = DMA_NORMAL;
    hdma_i2c1_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_i2c1_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(i2cHandle,hdmatx,hdma_i2c1_tx);

    /* I2C1 interrupt Init */
    HAL_NVIC_SetPriority(I2C1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_IRQn);
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PA9     ------> I2C1_SCL
    PA10     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10);

    /* I2C1 DMA DeInit */
    HAL_DMA_DeInit(i2cHandle->hdmarx);
    HAL_DMA_DeInit(i2cHandle->hdmatx);

    /* I2C1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(I2C1_IRQn);
  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void i2c_loop()
{

	run_i2c_state_machine();
	asm("nop");
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	// if the last state is the write answer, means the transaction should be over. As we are sending the last frame
	if(I2C_WRITE_ANSWER == last_state)
	{
		// put the device in the listen mode by changing the state to the i2c listen
		curr_state = I2C_LISTEN_MODE;
	}
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{

	// we need to configure the chip according to the register.
	if(REG_WRITE_DATA_1 == i2cBuf[0])
	{
		// this means we have to receive another byte to process the data
		status = HAL_I2C_Slave_Seq_Receive_DMA(&hi2c1, &i2cBuf[1], 1, I2C_FIRST_FRAME);
		if(HAL_OK != status)
		{
			Error_Handler();
		}
	}

	else if(REG_WRITE_DATA_2 == i2cBuf[0])
	{
		status = HAL_I2C_Slave_Seq_Receive_DMA(&hi2c1, &i2cBuf[1], 10, I2C_FIRST_FRAME);
		if(HAL_OK != status)
		{
			Error_Handler();
		}
	}

	else
	{
		curr_state = I2C_WRITE_REGISTER_VALUE;
	}


}

/* @brief  Listen Complete callback.
* @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
*                the configuration information for the specified I2C.
* @retval None
*/
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
	memcpy(data_buffer,i2cBuf,sizeof(data_buffer));
	// back to listen mode, clear the receiving buffer and receiving buffer index
	memset(i2cBuf,0,sizeof(i2cBuf));
	ubSlaveReceiveIndex = 0;
	// sessioon is completed not we need to reenable the i2c peripheral in the listen mode
	status = HAL_I2C_EnableListen_IT(&hi2c1);
	if(HAL_OK != status)
	{
		Error_Handler();
	}
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{

	asm("nop");
}

void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c)
{
	asm("nop");
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
	if(I2C1 == hi2c->Instance)
	{

		// since the directino is to write, thus the we are reading the register address address here
		if( I2C_DIRECTION_TRANSMIT == TransferDirection) // direction is context with the direction of master
		{
			//clear the receiving buffer and receiving buffer index
				memset(i2cBuf,0,sizeof(i2cBuf));
				ubSlaveReceiveIndex = 0;
			// configure the device to receive the data
			status = HAL_I2C_Slave_Seq_Receive_DMA(&hi2c1, (uint8_t *)i2cBuf, 1, I2C_FIRST_FRAME);
			if(HAL_OK != status)
			{
				Error_Handler();
			}
		}
		// master send the read command to read from the register it previously send
		if(I2C_DIRECTION_RECEIVE == TransferDirection)
		{
			curr_state = I2C_WRITE_ANSWER;
		}
	}

}

void run_i2c_state_machine()
{
	if(curr_state != last_state)
	{
		switch(curr_state)
		{
		case I2C_LISTEN_MODE:
			status = HAL_I2C_EnableListen_IT(&hi2c1);
			last_state= curr_state;
			break;
		case I2C_ADDRESS_MATCHED:
			last_state = I2C_ADDRESS_MATCHED;

			// now put the device into the receiving mode. Now the master is placing the register on the line
			// i2c would be monitoring only the start condition without the stop condition
			status = HAL_I2C_Slave_Seq_Receive_DMA(&hi2c1, i2cBuf, 1, I2C_FIRST_FRAME);
			if(HAL_OK != status)
			{
				Error_Handler();
			}
			break;

		case I2C_REGISTER_ADDRESS_MATCHED:
			// we have received the register address now check its validaity and send 2 byte message else write 0xFF for invalid address
			prepare_i2c_answer(register_to_be_queired);
			break;

		case I2C_WRITE_REGISTER_VALUE:
			// this is the case when the master write a value to the register. handle the register value here
			// since we received the value, thus the transaction is over. put the device in listening moode.
			curr_state = I2C_LISTEN_MODE;
			break;

		case I2C_WRITE_ANSWER:
			prepare_i2c_answer(i2cBuf[0]);
			i2c_status = HAL_I2C_GetState(&hi2c1);
			status = HAL_I2C_Slave_Sequential_Transmit_DMA(&hi2c1, i2cBuf, i2c_write_len,I2C_LAST_FRAME);
			if(HAL_OK != status)
			{
				Error_Handler();
			}
			curr_state = I2C_LISTEN_MODE;
			break;
		default:
			curr_state = last_state;
			break;
		}
	}
	asm("nop");
}

void prepare_i2c_answer(uint8_t regAddress)
{
	switch(regAddress)
	{
	case WHO_AM_I:
		i2cBuf[0] = 0x71;
		i2c_write_len = 1;
		break;

	case REG_WRITE_DATA_1:
//		status = HAL_I2C_Slave_Sequential_Receive_IT(&hi2c1, i2cBuf, 1, I2C_FIRST_FRAME);
		break;
	default:
		i2cBuf[0] = 0xFF;
		i2cBuf[1] = 0xFF;
		i2c_write_len = 2;
		break;
	}
}


/* USER CODE END 1 */
