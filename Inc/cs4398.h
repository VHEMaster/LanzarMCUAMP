#include "main.h"

extern HAL_StatusTypeDef CS4398_Init(I2C_HandleTypeDef * dac_i2c, I2S_HandleTypeDef * dac_i2s);
extern HAL_StatusTypeDef CS4398_SetVolume(uint8_t value);
extern HAL_StatusTypeDef CS4398_SetMute(uint8_t value);
