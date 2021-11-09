#include "driver_i2c.h"

#define MAX_NUM_INTERFACES 6

#define I2C_EV_MSG (uint32_t)1
#define I2C_ER_MSG (uint32_t)2

I2C_DriverInstanceTypeDef * I2C_DriverInstance[MAX_NUM_INTERFACES];
static volatile xSemaphoreHandle I2C_CommonMutex = NULL;

static HAL_StatusTypeDef WaitCplt(I2C_DriverInstanceTypeDef * instance)
{
  osEvent event;
	event = osMessageGet(instance->queue, instance->timeout);
  if(event.status == osEventMessage)
  {
    if(event.value.v == I2C_EV_MSG)
    {
			return HAL_OK;
		}
		else if(event.value.v == I2C_ER_MSG)
			return HAL_ERROR;
	}
	else if(event.status == osEventTimeout)
	{
		if((CoreDebug->DHCSR & 1) == 1)
			__BKPT(0); 
		return HAL_TIMEOUT;
	}
	
	return HAL_ERROR;
}

static void SendEvent(I2C_HandleTypeDef *hi2c, uint32_t event)
{
	
	for(uint32_t i = 0; i < MAX_NUM_INTERFACES; i++)
	{
		I2C_DriverInstanceTypeDef * instance = I2C_DriverInstance[i];
		if(instance != NULL)
			if(instance->hi2c == hi2c)
				osMessagePut(instance->queue, event, 1000);
	}
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	SendEvent(hi2c, I2C_EV_MSG);
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	SendEvent(hi2c, I2C_EV_MSG);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	SendEvent(hi2c, I2C_ER_MSG);
}

HAL_StatusTypeDef I2C_RegisterInterface(I2C_HandleTypeDef * hi2c, uint32_t timeout)
{
	I2C_DriverInstanceTypeDef * instance = NULL;
	
	if(I2C_CommonMutex == NULL)
	{
		I2C_CommonMutex = xSemaphoreCreateMutex();
	}
	
	xSemaphoreTake(I2C_CommonMutex, 1000);
	for(uint32_t i = 0; i < MAX_NUM_INTERFACES; i++)
	{
		if(I2C_DriverInstance[i] == NULL)
		{
			instance = pvPortMalloc(sizeof(I2C_DriverInstanceTypeDef));
			I2C_DriverInstance[i] = instance;
			break;
		}
	}
	xSemaphoreGive(I2C_CommonMutex);
	
	if(instance == NULL)
		return HAL_ERROR;
	
  osMessageQDef(queue, 10, uint32_t);
  instance->queue = osMessageCreate(osMessageQ(queue), NULL);
	instance->mutex = xSemaphoreCreateMutex();
	instance->timeout = 1000;
	instance->hi2c = hi2c;
	
	if(instance->queue == NULL || instance->mutex == NULL)
		return HAL_ERROR;
	
	return HAL_OK;	
}

HAL_StatusTypeDef I2C_WriteData(I2C_HandleTypeDef * hi2c, uint8_t address, uint8_t * data, uint16_t size)
{
	I2C_DriverInstanceTypeDef * instance = NULL;
	for(uint32_t i = 0; i < MAX_NUM_INTERFACES; i++)
	{
		if(I2C_DriverInstance[i] != NULL)
		{	
			if(I2C_DriverInstance[i]->hi2c == hi2c)
			{
				instance = I2C_DriverInstance[i];
				break;
			}

		}
	}
	if(instance == NULL) 
		return HAL_ERROR;
	
	xSemaphoreTake(instance->mutex, 1000);
	HAL_StatusTypeDef status;
	status = HAL_I2C_Master_Transmit_IT(instance->hi2c, address, data, size);
	if(status != HAL_OK) return status;
	status =  WaitCplt(instance);
	xSemaphoreGive(instance->mutex);
	return status;
}

HAL_StatusTypeDef I2C_ReadData(I2C_HandleTypeDef * hi2c, uint8_t address, uint8_t * data, uint16_t size)
{
	I2C_DriverInstanceTypeDef * instance = NULL;
	for(uint32_t i = 0; i < MAX_NUM_INTERFACES; i++)
	{
		if(I2C_DriverInstance[i] != NULL)
		{	
			if(I2C_DriverInstance[i]->hi2c == hi2c)
			{
				instance = I2C_DriverInstance[i];
				break;
			}
		}
	}
	if(instance == NULL) 
		return HAL_ERROR;
	
	xSemaphoreTake(instance->mutex, 1000);
	HAL_StatusTypeDef status;
	status = HAL_I2C_Master_Receive_IT(instance->hi2c, address, data, size);
	if(status != HAL_OK) return status;
	status =  WaitCplt(instance);
	xSemaphoreGive(instance->mutex);
	return status;
}

HAL_StatusTypeDef I2C_Write(I2C_HandleTypeDef * hi2c, uint8_t address, uint8_t reg, uint8_t val)
{
	uint8_t data[2] = {reg, val};
	return I2C_WriteData(hi2c, address, data, 2);
}

HAL_StatusTypeDef I2C_Read(I2C_HandleTypeDef * hi2c, uint8_t address, uint8_t reg, uint8_t * val)
{
	HAL_StatusTypeDef status;
	status = I2C_WriteData(hi2c, address, &reg, 1);
	if(status != HAL_OK) return status;
	return I2C_ReadData(hi2c, address, val, 1);
}

