#include "main.h"

typedef struct
{
	I2C_HandleTypeDef * hi2c;
	volatile xSemaphoreHandle mutex;
	osMessageQId queue;
	uint32_t timeout;
} I2C_DriverInstanceTypeDef;

extern HAL_StatusTypeDef I2C_RegisterInterface(I2C_HandleTypeDef * hi2c, uint32_t timeout);
extern HAL_StatusTypeDef I2C_WriteData(I2C_HandleTypeDef * hi2c, uint8_t address, uint8_t * data, uint16_t size);
extern HAL_StatusTypeDef I2C_ReadData(I2C_HandleTypeDef * hi2c, uint8_t address, uint8_t * data, uint16_t size);
extern HAL_StatusTypeDef I2C_Write(I2C_HandleTypeDef * hi2c, uint8_t address, uint8_t reg, uint8_t val);
extern HAL_StatusTypeDef I2C_Read(I2C_HandleTypeDef * hi2c, uint8_t address, uint8_t reg, uint8_t * val);

