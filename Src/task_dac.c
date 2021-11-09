#include "task_dac.h"
#include "cs4398.h"
#include "cs43l22.h"

osThreadId dacTaskHandle;
osThreadId dacHpTaskHandle;  

osMessageQId dacQueue;
osMessageQId dacHpQueue;

typedef struct 
{
	uint8_t command;
	uint8_t value;
	uint8_t extra_1;
	uint8_t extra_2;
} CommandTypeDef __attribute__ ((aligned(4)));

#define COMMAND_INIT 0
#define COMMAND_MUTE 1
#define COMMAND_VOLUME 2

static void StartTask(void const * argument)
{
  osMessageQDef(queue, 16, uint32_t);
	dacQueue = osMessageCreate(osMessageQ(queue), NULL);
	while(1)
	{
		osEvent event;
		event = osMessageGet(dacQueue, 1000);
		if(event.status == osEventMessage)
		{
			CommandTypeDef * command = (CommandTypeDef *)&(event.value.v);
			switch(command->command)
			{
				case COMMAND_INIT : 
					//TODO NEXT
					break;
				case COMMAND_MUTE :
					CS4398_SetMute(command->value);
					break;
				case COMMAND_VOLUME :
					CS4398_SetVolume(command->value);
					break;
			}
			
		}
	}
}
static void StartHpTask(void const * argument)
{
  osMessageQDef(queue, 16, uint32_t);
	dacHpQueue = osMessageCreate(osMessageQ(queue), NULL);
	while(1)
	{
		
		osEvent event;
		event = osMessageGet(dacHpQueue, 1000);
		if(event.status == osEventMessage)
		{
			CommandTypeDef * command = (CommandTypeDef *)&(event.value.v);
			switch(command->command)
			{
				case COMMAND_INIT : 
					//TODO NEXT
					break;
				case COMMAND_MUTE :
					CS43L22_SetMute(command->value);
					break;
				case COMMAND_VOLUME :
					CS43L22_SetVolume(command->value);
					break;
			}
		}
	}
}

HAL_StatusTypeDef DAC_HP_SetMute(uint8_t value)
{
	if(dacHpQueue == NULL)
		return HAL_ERROR;
	
	CommandTypeDef command;
	osStatus status;
	
	command.command = COMMAND_MUTE;
	command.value = value;
	
	status = osMessagePut(dacHpQueue, *((uint32_t*)&command), 100);
	
	if(status != osOK)
		return HAL_ERROR;
	
	return HAL_OK;
}

HAL_StatusTypeDef DAC_HP_SetVolume(uint8_t value)
{
	if(dacHpQueue == NULL)
		return HAL_ERROR;
	
	CommandTypeDef command;
	osStatus status;
	
	command.command = COMMAND_VOLUME;
	command.value = value;
	
	status = osMessagePut(dacHpQueue, *((uint32_t*)&command), 100);
	
	if(status != osOK)
		return HAL_ERROR;
	
	return HAL_OK;
}
HAL_StatusTypeDef DAC_SetMute(uint8_t value)
{
	if(dacQueue == NULL)
		return HAL_ERROR;
	
	CommandTypeDef command;
	osStatus status;
	
	command.command = COMMAND_MUTE;
	command.value = value;
	
	status = osMessagePut(dacQueue, *((uint32_t*)&command), 100);
	
	if(status != osOK)
		return HAL_ERROR;
	
	return HAL_OK;
}

HAL_StatusTypeDef DAC_SetVolume(uint8_t value)
{
	if(dacQueue == NULL)
		return HAL_ERROR;
	
	CommandTypeDef command;
	osStatus status;
	
	command.command = COMMAND_VOLUME;
	command.value = value;
	
	status = osMessagePut(dacQueue, *((uint32_t*)&command), 100);
	
	if(status != osOK)
		return HAL_ERROR;
	
	return HAL_OK;
}

void DAC_InitTask(void)
{
	
  osThreadDef(dacTask, StartTask, osPriorityHigh, 0, 512);
  dacTaskHandle = osThreadCreate(osThread(dacTask), NULL);
  osThreadDef(dacHpTask, StartHpTask, osPriorityHigh, 0, 512);
  dacHpTaskHandle = osThreadCreate(osThread(dacHpTask), NULL);
}

