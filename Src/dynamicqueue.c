#include "dynamicqueue.h"


DynamicQueueTypeDef * DynamicQueueCreate(uint32_t size)
{
	DynamicQueueTypeDef * queue = (DynamicQueueTypeDef*)pvPortMalloc(sizeof(DynamicQueueTypeDef));
	if(queue == NULL) return NULL;
	queue->data = (uint8_t*)pvPortMalloc(sizeof(uint8_t[size]));
	if(queue->data == NULL) { vPortFree((void*)queue); return NULL; }
	
	queue->ReceiveTask = NULL;
	queue->OverflowTask = NULL;
	
	queue->size = size;
	queue->writepos = 0;
	queue->readpos = 0;
	queue->overflow = 0;
	queue->overflowcount = 0;
	queue->datafilled = 0;
	
	return queue;
}

uint32_t DynamicQueueSend(DynamicQueueTypeDef * queue, void * data, uint32_t size)
{
	uint32_t i;
	if(queue == NULL) return 0;
	if(size > queue->size) return 0;
	while(size > queue->size-queue->datafilled)
	{
		queue->overflow++;
		//return 0;
		//osDelay(1);
		//continue;
		
		queue->overflowcount = size;
		queue->OverflowTask = xTaskGetCurrentTaskHandle();
		vTaskSuspend(queue->OverflowTask);
		//taskEXIT_CRITICAL();
	}
	
	taskENTER_CRITICAL();
	for(i=0;i<size;i++)
	{
		queue->data[queue->writepos++] = ((uint8_t*)data)[i];
		if(queue->writepos >= queue->size) queue->writepos = 0;
	}
	queue->datafilled+=size;
	if(queue->ReceiveTask != NULL)
	{
		if(queue->datafilled >= queue->requestedsize)
		{
			vTaskResume(queue->ReceiveTask);
			queue->ReceiveTask = NULL;
		}
	}
	taskEXIT_CRITICAL();
	
	return i;
}

uint32_t DynamicQueueCheck(DynamicQueueTypeDef * queue)
{
	if(queue == NULL) return 0;
	return queue->datafilled;
}

void DynamicQueueClear(DynamicQueueTypeDef * queue)
{
	taskENTER_CRITICAL();
	queue->datafilled = 0;
	queue->overflow = 0;
	queue->readpos = 0;
	queue->writepos = 0;
	if(queue->OverflowTask != NULL && queue->overflowcount < queue->datafilled)
	{
		vTaskResume(queue->OverflowTask);
		queue->OverflowTask = NULL;
		queue->overflowcount = 0;
	}
	taskEXIT_CRITICAL();
}

uint32_t DynamicQueueReceive(DynamicQueueTypeDef * queue, void * data, uint32_t size)
{
	uint32_t i;
	if(queue == NULL) return 0;
	queue->requestedsize = size;
	
	while(queue->datafilled < queue->requestedsize)
	{
		queue->ReceiveTask = xTaskGetCurrentTaskHandle();
		vTaskSuspend(queue->ReceiveTask);
	}
	queue->ReceiveTask = NULL;
	
	taskENTER_CRITICAL();
	if(data != NULL)
	{
		for(i=0;i<size;i++)
		{
			((uint8_t*)data)[i] = queue->data[queue->readpos++];
			if(queue->readpos >= queue->size) queue->readpos = 0;
		}
	}
	else
	{
			queue->readpos = (queue->readpos+size) % queue->size;
	}
	queue->datafilled -= size;
	queue->requestedsize -= size;
	
	if(queue->OverflowTask != NULL && queue->overflowcount <= queue->size-queue->datafilled)
	{
		vTaskResume(queue->OverflowTask);
		queue->OverflowTask = NULL;
		queue->overflowcount = 0;
	}
	
	taskEXIT_CRITICAL();
	
	return queue->datafilled;
}


int32_t DynamicQueueReceiveUntil(DynamicQueueTypeDef * queue, void * data, char until)
{
	uint32_t i = 0;
	int32_t result = 0;
	int32_t counter = 0;
	if(queue == NULL) return 0;
	
	
	taskENTER_CRITICAL();
	queue->ReceiveTask = NULL;
	while(result == 0)
	{
		queue->requestedsize = 1;
		while(queue->datafilled < queue->requestedsize)
		{
			queue->ReceiveTask = xTaskGetCurrentTaskHandle();
			taskEXIT_CRITICAL();
			vTaskSuspend(queue->ReceiveTask);
			taskENTER_CRITICAL();
		}
		((uint8_t*)data)[i] = queue->data[queue->readpos++];
		if(queue->readpos >= queue->size) queue->readpos = 0;
		queue->datafilled -= 1;
		queue->requestedsize = -1;
		counter++;
		
		if(((uint8_t*)data)[i] == until) result = counter;
		i++;
	}
	taskEXIT_CRITICAL();
	
	return result;
}

