#include "main.h"

extern void DAC_InitTask(void);

extern HAL_StatusTypeDef DAC_HP_SetMute(uint8_t value);
extern HAL_StatusTypeDef DAC_HP_SetVolume(uint8_t value);
extern HAL_StatusTypeDef DAC_SetMute(uint8_t value);
extern HAL_StatusTypeDef DAC_SetVolume(uint8_t value);
