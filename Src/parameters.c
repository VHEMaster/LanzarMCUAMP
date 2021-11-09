#include "parameters.h"

volatile uint8_t * Parameters_BackupData = (volatile uint8_t*)(BKPSRAM_BASE);

ParametersTypeDef Parameters;
const ParametersTypeDef DefaultParameters = { 1, 0, "1111", "LanzarMCU" , "HomeWiFi", "12345678", 0};

void Params_SaveConfiguration(void)
{
	uint8_t crc1 = 0,crc2 = 0;
	uint16_t i = 0;
	Parameters_BackupData[0] = 0xAC;
	for(i = 0; i < sizeof(Parameters); i++)
	{
		crc1 ^= ((uint8_t*)&Parameters)[i];
		crc2 += ((uint8_t*)&Parameters)[i];
		Parameters_BackupData[i+1] = ((uint8_t*)&Parameters)[i];
	}
	Parameters_BackupData[sizeof(Parameters)+1] = crc1 ^ crc2;
}

HAL_StatusTypeDef Params_LoadConfiguration(void)
{
	uint8_t crc1 = 0,crc2 = 0;
	uint16_t i = 0;
	if(Parameters_BackupData[0] == 0xAC)
	{
		for(i = 0; i < sizeof(Parameters); i++)
		{
			crc1 ^= Parameters_BackupData[i+1];
			crc2 += Parameters_BackupData[i+1];
			((uint8_t*)&Parameters)[i] = Parameters_BackupData[i+1];
		}
	}
	else
	{
		crc1 = crc2 = 0;
		Parameters_BackupData[sizeof(ParametersTypeDef)+1] = 0xAC;
	}
	if(Parameters_BackupData[sizeof(ParametersTypeDef)+1] != (crc1 ^ crc2))
	{
		for(i = 0; i < sizeof(ParametersTypeDef); i++)
		{
			((uint8_t*)&Parameters)[i] = ((uint8_t*)&DefaultParameters)[i];
		}
		return HAL_ERROR;
	}
	return HAL_OK;
}
