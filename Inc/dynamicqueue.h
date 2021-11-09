#include "main.h"

typedef volatile struct {
	uint8_t * data;
	uint32_t datafilled;
	uint32_t size;
	uint32_t overflow;
	uint32_t overflowcount;
	uint32_t writepos;
	uint32_t readpos;
	osThreadId * ReceiveTask; 
	osThreadId * OverflowTask; 
	uint32_t requestedsize;
} DynamicQueueTypeDef;


DynamicQueueTypeDef * DynamicQueueCreate(uint32_t size);
uint32_t DynamicQueueSend(DynamicQueueTypeDef * queue, void * data, uint32_t size);
uint32_t DynamicQueueCheck(DynamicQueueTypeDef * queue);
void DynamicQueueClear(DynamicQueueTypeDef * queue);
void DynamicQueueExcange(DynamicQueueTypeDef * dest,DynamicQueueTypeDef * src, uint32_t size);
uint32_t DynamicQueueReceive(DynamicQueueTypeDef * queue, void * data, uint32_t size);
int32_t DynamicQueueReceiveUntil(DynamicQueueTypeDef * queue, void * data, char until);



