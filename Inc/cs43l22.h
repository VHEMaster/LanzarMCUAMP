#include "main.h"

extern HAL_StatusTypeDef CS43L22_Init(I2C_HandleTypeDef * dac_i2c, I2S_HandleTypeDef * dac_i2s);
extern HAL_StatusTypeDef CS43L22_SetVolume(uint8_t value);
extern HAL_StatusTypeDef CS43L22_SetMute(uint8_t value);
