#include "cs43l22.h"
#include "driver_i2c.h"

static I2C_HandleTypeDef * hi2c;
static I2S_HandleTypeDef * hi2s;

#define DAC_ADDR 0x94

HAL_StatusTypeDef CS43L22_SetVolume(uint8_t value)
{
	HAL_StatusTypeDef status = HAL_OK;
	int8_t volume = 115 - (value / 2.21f);
	I2C_Write(hi2c, DAC_ADDR, 0x22,-volume);
	I2C_Write(hi2c, DAC_ADDR, 0x23,-volume);
	I2C_Write(hi2c, DAC_ADDR, 0x24,-volume);
	I2C_Write(hi2c, DAC_ADDR, 0x25,-volume);
	return status;
}


HAL_StatusTypeDef CS43L22_SetMute(uint8_t value)
{
	HAL_StatusTypeDef status = HAL_OK;
	status |= I2C_Write(hi2c, DAC_ADDR, 0x04,value > 0 ? 0xFF : 0xAA);
	return status;
}

HAL_StatusTypeDef CS43L22_Init(I2C_HandleTypeDef * dac_i2c, I2S_HandleTypeDef * dac_i2s)
{
	if(dac_i2c == NULL || dac_i2s == NULL)
		return HAL_ERROR;
	hi2s = dac_i2s;
	hi2c = dac_i2c;
	
	uint8_t readval;
	
	I2C_RegisterInterface(hi2c, 200);
	
	HAL_GPIO_WritePin(DAC_HP_RST_GPIO_Port, DAC_HP_RST_Pin, GPIO_PIN_SET);
	do
	{
		osDelay(10);
		I2C_Read(hi2c, DAC_ADDR, 0x01, &readval);
	}
	while((readval&0xF8) != 0xE0);
	
	I2C_Write(hi2c, DAC_ADDR, 0x01,0x01);
	I2C_Write(hi2c, DAC_ADDR, 0x05,0xA1);
	I2C_Write(hi2c, DAC_ADDR, 0x06,0x04);
	I2C_Write(hi2c, DAC_ADDR, 0x0F,0x09);
	I2C_Write(hi2c, DAC_ADDR, 0x0A,0x00);
	I2C_Write(hi2c, DAC_ADDR, 0x0E,0x04);
	I2C_Write(hi2c, DAC_ADDR, 0x27,0x00);
	I2C_Write(hi2c, DAC_ADDR, 0x1F,0x88);
	
	//VOLUME
	I2C_Write(hi2c, DAC_ADDR, 0x1A,0);
	I2C_Write(hi2c, DAC_ADDR, 0x1B,0);
	
	I2C_Write(hi2c, DAC_ADDR, 0x02,0x9E); //PowerUP
	//I2C_Write(hi2c, DAC_ADDR, 0x02,0x01); //PowerDown
	
	I2C_Write(hi2c, DAC_ADDR, 0x22,0x40);
	I2C_Write(hi2c, DAC_ADDR, 0x23,0x40);
	I2C_Write(hi2c, DAC_ADDR, 0x24,0x40);
	I2C_Write(hi2c, DAC_ADDR, 0x25,0x40);
	
	//Swap channels
	I2C_Write(hi2c, DAC_ADDR, 0x26,0xF0);
	I2C_Write(hi2c, DAC_ADDR, 0x0F,0x09+4);
	
	//I2C_Write(hi2c, DAC_ADDR, 0x04,0xAA); //Unmute
	I2C_Write(hi2c, DAC_ADDR, 0x04,0xFF); //Mute
	
	
	
	return HAL_OK;
}

