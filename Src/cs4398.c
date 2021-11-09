#include "cs4398.h"
#include "driver_i2c.h"

static I2C_HandleTypeDef * hi2c;
static I2S_HandleTypeDef * hi2s;

#define DAC_ADDR 0x9E

HAL_StatusTypeDef CS4398_SetVolume(uint8_t value)
{
	HAL_StatusTypeDef status = HAL_OK;
	int8_t volume = 115 - (value / 2.21f);
	status |= I2C_Write(hi2c, DAC_ADDR, 0x05, volume);
	status |= I2C_Write(hi2c, DAC_ADDR, 0x06, volume);
	return status;
}


HAL_StatusTypeDef CS4398_SetMute(uint8_t value)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t tempval;
	status |= I2C_Read(hi2c, DAC_ADDR, 0x04, &tempval);
	tempval = value > 0 ? tempval | 0x1F : tempval & 0xE7;
	status |= I2C_Write(hi2c, DAC_ADDR, 0x04, tempval);
	return status;
}

HAL_StatusTypeDef CS4398_Init(I2C_HandleTypeDef * dac_i2c, I2S_HandleTypeDef * dac_i2s)
{
	if(dac_i2c == NULL || dac_i2s == NULL)
		return HAL_ERROR;
	hi2s = dac_i2s;
	hi2c = dac_i2c;
	
	uint8_t readval;
	
	I2C_RegisterInterface(hi2c, 200);
	
	HAL_GPIO_WritePin(DAC_RST_GPIO_Port, DAC_RST_Pin, GPIO_PIN_SET);
	do
	{
		osDelay(10);
		I2C_Read(hi2c, DAC_ADDR, 0x01, &readval);
	}
	while((readval&0xF8) != 0x70);
	
	
	I2C_Write(hi2c, DAC_ADDR, 0x08, 0x40);
	//I2C_Write(hi2c, DAC_ADDR, 0x04, 0x03); //Unmuted
	I2C_Write(hi2c, DAC_ADDR, 0x04, 0x1B); //Muted
	I2C_Write(hi2c, DAC_ADDR, 0x02, 0x10);
	
	I2C_Write(hi2c, DAC_ADDR, 0x05, 255);
	I2C_Write(hi2c, DAC_ADDR, 0x06, 255);
	
	
	return HAL_OK;
}

