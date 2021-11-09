#include "main.h"
#include "stm32f7xx_hal.h"
#include "cmsis_os.h"
#include "esp8266.h"
#include "dynamicqueue.h"
#include <stdarg.h>
#include <string.h>

extern UART_HandleTypeDef huart5;
extern DMA_HandleTypeDef hdma_uart5_rx;

#define ESP_BUFFER_SIZE 10240
#define ESP_MAILBOX_SIZE 2048
#define ESP_FD_BUFFER_SIZE 2048
#define ESP_FD_MAX_SEND_SIZE 512
#define ESP_TX_TASK_BUFFER_SIZE 512
#define ESP_RX_TASK_BUFFER_SIZE 4096
#define ESP_SEND_BUFFER_SIZE 2048
#define DMA_BUFFER_SIZE 10240

static volatile uint8_t ESP_TXDONE = 0;

osThreadId ESPRXTaskHandle;
osThreadId ESPTXTaskHandle;
osThreadId ESPTaskHandle;
osThreadId ESPStackTaskHandle;

DynamicQueueTypeDef * ESP_RX;
DynamicQueueTypeDef * ESP_TX;
DynamicQueueTypeDef * ESP_MAILBOX_RX;
DynamicQueueTypeDef * ESP_MAILBOX_TX;

static void StartESPRXTask(void const * argument);
static void StartESPTXTask(void const * argument);
static void StartESPIOTask(void const * argument);
static void StartESPHandlerTask(void const * argument);

static uint8_t DMA_BUFFER[DMA_BUFFER_SIZE];

void ESP_TxCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart5)
	{
		ESP_TXDONE = 1;
	}
}

void ESP_RxCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart5)
	{
		
	}
}

void ESP_ErrorCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart5)
	{
		HAL_UART_Receive_DMA(huart, DMA_BUFFER, DMA_BUFFER_SIZE);
	}
}

static void ESP_SetBaudrate(uint32_t baudrate)
{
  huart5.Init.BaudRate = baudrate;
  HAL_UART_Init(&huart5);
	HAL_UART_Receive_DMA(&huart5, DMA_BUFFER, DMA_BUFFER_SIZE);
}

void ESP_Start(void)
{
  osThreadDef(ESPTask, StartESPIOTask, osPriorityAboveNormal, 0, 512);
  ESPTaskHandle = osThreadCreate(osThread(ESPTask), NULL);
}

static char * ESP_SendPacket_Buffer;
uint8_t ESP_SendMessage(char * string, ...)
{
	char data[64];
	uint8_t res;
	uint16_t received = 0,size = 0;
	uint32_t started;
	va_list ap;
	va_start(ap, string);
	vsprintf(data, string, ap);
	va_end(ap);
	
	char * rec = ESP_SendPacket_Buffer;
	
	DynamicQueueSend(ESP_TX, data, strlen(data));
	
	started = HAL_GetTick();
	res = 2;
	
	while(res == 2)
	{
		if(DynamicQueueCheck(ESP_RX) > 0)
		{
			size = DynamicQueueCheck(ESP_RX);
			DynamicQueueReceive(ESP_RX, &rec[received], size);
			
				
			received += size;
			started = HAL_GetTick();
			if(received > 10)
			{
				if(	rec[received-8] == '\r' && \
						rec[received-7] == '\n' && \
						rec[received-6] == '\r' && \
						rec[received-5] == '\n' && \
						rec[received-4] == 'O' && \
						rec[received-3] == 'K' && \
						rec[received-2] == '\r' && \
						rec[received-1] == '\n' )
				{
					res = 0;
					break;
				}
			}
			if(received > 12)
			{
				if(	rec[received-10] == '\r' && \
						rec[received-9] == '\n' && \
						rec[received-8] == '\r' && \
						rec[received-7] == '\n' && \
						rec[received-6] == 'O' && \
						rec[received-5] == 'K' && \
						rec[received-4] == '\r' && \
						rec[received-3] == '\n' && \
						rec[received-2] == '>'  && \
						rec[received-1] == ' ' )
				{
					res = 0;
					break;
				}
			}
			if(received > 13)
			{
				if((	rec[received-11] == '\r' && \
							rec[received-10] == '\n' && \
							rec[received-9] == '\r' && \
							rec[received-8] == '\n' && \
							rec[received-7] == 'E' && \
							rec[received-6] == 'R' && \
							rec[received-5] == 'R' && \
							rec[received-4] == 'O' && \
							rec[received-3] == 'R' && \
							rec[received-2] == '\r' && \
							rec[received-1] == '\n' ) || (
				
							rec[received-10] == '\r' && \
							rec[received-9] == '\n' && \
							rec[received-8] == '\r' && \
							rec[received-7] == '\n' && \
							rec[received-6] == 'F' && \
							rec[received-5] == 'A' && \
							rec[received-4] == 'I' && \
							rec[received-3] == 'L' && \
							rec[received-2] == '\r' && \
							rec[received-1] == '\n' ) )
				{
					res = 1;
					break;
				}
			}
		}
		else
		{
			if(HAL_GetTick()-started > 30000)
			{
				res = 2;
				break;
			}
			osDelay(1);
		}
	}
		
	
	return res;
}

uint8_t ESP_SendMessageWithResult(char * rec, char * string, ...)
{
	char data[64];
	uint8_t res;
	uint16_t received = 0,size = 0;
	uint32_t started;
	va_list ap;
	va_start(ap, string);
	vsprintf(data, string, ap);
	va_end(ap);
	
	
	DynamicQueueSend(ESP_TX, data, strlen(data));
	
	started = HAL_GetTick();
	res = 2;
	
	while(res == 2)
	{
		if(DynamicQueueCheck(ESP_RX) > 0)
		{
			size = DynamicQueueCheck(ESP_RX);
			DynamicQueueReceive(ESP_RX, &rec[received], size);
			
				
			received += size;
			started = HAL_GetTick();
			if(received > 10)
			{
				if(	rec[received-8] == '\r' && \
						rec[received-7] == '\n' && \
						rec[received-6] == '\r' && \
						rec[received-5] == '\n' && \
						rec[received-4] == 'O' && \
						rec[received-3] == 'K' && \
						rec[received-2] == '\r' && \
						rec[received-1] == '\n' )
				{
					res = 0;
					break;
				}
			}
			if(received > 13)
			{
				if((	rec[received-11] == '\r' && \
							rec[received-10] == '\n' && \
							rec[received-9] == '\r' && \
							rec[received-8] == '\n' && \
							rec[received-7] == 'E' && \
							rec[received-6] == 'R' && \
							rec[received-5] == 'R' && \
							rec[received-4] == 'O' && \
							rec[received-3] == 'R' && \
							rec[received-2] == '\r' && \
							rec[received-1] == '\n' ) || (
				
							rec[received-10] == '\r' && \
							rec[received-9] == '\n' && \
							rec[received-8] == '\r' && \
							rec[received-7] == '\n' && \
							rec[received-6] == 'F' && \
							rec[received-5] == 'A' && \
							rec[received-4] == 'I' && \
							rec[received-3] == 'L' && \
							rec[received-2] == '\r' && \
							rec[received-1] == '\n' ) )
				{
					res = 1;
					break;
				}
			}
		}
		if(HAL_GetTick()-started > 30000)
		{
			res = 2;
			break;
		}
	}
		
	
	return res;
}

uint8_t ESP_SendPacket(char * data, uint16_t length)
{
	uint8_t res;
	uint16_t received = 0,size = 0;
	uint32_t started;
	
	char * rec = ESP_SendPacket_Buffer;
	
	DynamicQueueSend(ESP_TX, data, length);
	
	started = HAL_GetTick();
	res = 2;
	
	while(res == 2)
	{
		if(DynamicQueueCheck(ESP_RX) > 0)
		{
			size = DynamicQueueCheck(ESP_RX);
			DynamicQueueReceive(ESP_RX, &rec[received], size);
			
				
			received += size;
			started = HAL_GetTick();
			if(received > 10)
			{
				if(	rec[received-9] == 'S' && \
						rec[received-8] == 'E' && \
						rec[received-7] == 'N' && \
						rec[received-6] == 'D' && \
						rec[received-5] == ' ' && \
						rec[received-4] == 'O' && \
						rec[received-3] == 'K' && \
						rec[received-2] == '\r' && \
						rec[received-1] == '\n' )
				{
					rec[received] = 0;
					res = 0;
					break;
				}
			}
			if(received > 13)
			{
				if((	rec[received-11] == '\r' && \
							rec[received-10] == '\n' && \
							rec[received-9] == '\r' && \
							rec[received-8] == '\n' && \
							rec[received-7] == 'E' && \
							rec[received-6] == 'R' && \
							rec[received-5] == 'R' && \
							rec[received-4] == 'O' && \
							rec[received-3] == 'R' && \
							rec[received-2] == '\r' && \
							rec[received-1] == '\n' ) || (
				
							rec[received-10] == '\r' && \
							rec[received-9] == '\n' && \
							rec[received-8] == '\r' && \
							rec[received-7] == '\n' && \
							rec[received-6] == 'F' && \
							rec[received-5] == 'A' && \
							rec[received-4] == 'I' && \
							rec[received-3] == 'L' && \
							rec[received-2] == '\r' && \
							rec[received-1] == '\n' ) )
				{
					rec[received] = 0;
					res = 1;
					break;
				}
			}
		}
		if(HAL_GetTick()-started > 5000)
		{
			res = 2;
			break;
		}
	}
		
	
	return res;
}

void StartESPTXTask(void const * argument)
{
	uint8_t * string = pvPortMalloc(ESP_TX_TASK_BUFFER_SIZE);
	uint16_t length;
	for(;;)
	{
		length = DynamicQueueCheck(ESP_TX);
		if(length > 0)
		{
			if(length > ESP_TX_TASK_BUFFER_SIZE) length = ESP_TX_TASK_BUFFER_SIZE;
			DynamicQueueReceive(ESP_TX,string,length);
			ESP_TXDONE = 0;
			HAL_UART_Transmit_DMA(&huart5,string,length);
			while(!ESP_TXDONE) osDelay(1);
		}
		else osDelay(1);
	}
}

void StartESPRXTask(void const * argument)
{
	char * string = pvPortMalloc(ESP_RX_TASK_BUFFER_SIZE);
	uint16_t length,i,dmapnt;
	uint16_t dmasize;
	uint16_t pointer = 65535;
	
  for(;;)
  {
		if(huart5.RxState == HAL_UART_STATE_BUSY_RX)
		{
			dmasize = huart5.RxXferSize;
			dmapnt = hdma_uart5_rx.Instance->NDTR;
			if(pointer == 65535) pointer = dmapnt;
			if(dmapnt > pointer)
				length = (dmasize-dmapnt)+pointer;
			else length = pointer-dmapnt;
			if(length > 0)
			{
				if(length > ESP_RX_TASK_BUFFER_SIZE) length = ESP_RX_TASK_BUFFER_SIZE;
				for(i=0;i<length;i++)
				{
					string[i] = DMA_BUFFER[dmasize-pointer];
					if(pointer == 1) pointer = dmasize;
					else pointer--;
				}
				string[length] = 0;
				
				//DEBUG PRINTF!!!
				printf("%s", string);
				
				DynamicQueueSend(ESP_RX,string,length);
			}
			else osDelay(1);
		}
		else pointer = 65535, osDelay(1);
  }
  /* USER CODE END 5 */ 
}



typedef enum 
{
  ESP_PACKET_LISTEN,
  ESP_PACKET_INCOMING,
  ESP_PACKET_UNLISTEN,
  ESP_PACKET_SEND,
  ESP_PACKET_RECV,
  ESP_PACKET_DISCONNECT,
  ESP_PACKET_CONNECT,
  ESP_PACKET_APCONNECT,
  ESP_PACKET_APDISCONNECT,
  ESP_PACKET_APLIST,
  ESP_PACKET_GETIP,
  ESP_PACKET_NSLOOKUP,
	ESP_PACKET_GETMAC,
	ESP_PACKET_SETMAC,
	ESP_PACKET_SETHOST,
	ESP_PACKET_GETHOST,
	ESP_PACKET_STATUS,
  ESP_PACKET_PING
} ESP_PacketTypeDef;

typedef enum
{
	FD_CONNECTED,
	FD_DISCONNECTED
} fdConnected_TypeDef;

typedef struct 
{
	char host[16];
	unsigned int port;
	DynamicQueueTypeDef * rxqueue;
	fdConnected_TypeDef status;
	
}fdDataTypeDef;

fdDataTypeDef fdData[5];
volatile xSemaphoreHandle fdMutex;

void SendBlocking(DynamicQueueTypeDef * queue, void * data, uint16_t * size)
{
	TaskHandle_t currenttask = xTaskGetCurrentTaskHandle();
	taskENTER_CRITICAL();
	DynamicQueueSend(queue, &currenttask, sizeof(TaskHandle_t));
	DynamicQueueSend(queue, &data, sizeof(char*));
	DynamicQueueSend(queue, &size, sizeof(uint16_t*));
	DynamicQueueSend(queue, data, *size);
	vTaskSuspend(currenttask);
	
	taskEXIT_CRITICAL();
}

void SendLargeBlocking(DynamicQueueTypeDef * queue, void * data, uint16_t * size, void * buffer, uint16_t buffersize)
{
	TaskHandle_t currenttask = xTaskGetCurrentTaskHandle();
	taskENTER_CRITICAL();
	DynamicQueueSend(queue, &currenttask, sizeof(TaskHandle_t));
	DynamicQueueSend(queue, &data, sizeof(char*));
	DynamicQueueSend(queue, &size, sizeof(uint16_t*));
	DynamicQueueSend(queue, data, *size);
	DynamicQueueSend(queue, buffer, buffersize);
	vTaskSuspend(currenttask);
	
	taskEXIT_CRITICAL();
}

void SendLargeNonBlocking(DynamicQueueTypeDef * queue, void * data, uint16_t * size, void * buffer, uint16_t buffersize)
{
	TaskHandle_t currenttask = xTaskGetCurrentTaskHandle();
	taskENTER_CRITICAL();
	DynamicQueueSend(queue, &currenttask, sizeof(TaskHandle_t));
	DynamicQueueSend(queue, &data, sizeof(char*));
	DynamicQueueSend(queue, &size, sizeof(uint16_t*));
	DynamicQueueSend(queue, data, *size);
	DynamicQueueSend(queue, buffer, buffersize);
	taskEXIT_CRITICAL();
}

int esp_apconnect(char * ssid, char * key)
{
	int result;
	char data[65];
	uint16_t length;
	if(strlen(ssid) > 31 || strlen(key) > 31) result = -3;
	else
	{
		length = 65;
		data[0] = ESP_PACKET_APCONNECT;
		strcpy(&data[1],ssid);
		strcpy(&data[33],key);
		SendBlocking(ESP_MAILBOX_RX, data, &length);
		
		if(data[0] != ESP_PACKET_APCONNECT) result = -2;
		else if(data[1] != 0) result = -1;
		
		
	}
	return result;
}

int esp_gethost(int fd, char * host)
{
	uint16_t length;
	uint8_t i;
	if(fd > 5 || fd < 0) return -4;
	if(fdData[fd].status == FD_DISCONNECTED) return -1;
	if(fdData[fd].host[0] == 0)
	{
		char data[104];
		length = 1;
		data[0] = ESP_PACKET_STATUS;
		SendBlocking(ESP_MAILBOX_RX, data, &length);
		
		if(data[0] != ESP_PACKET_STATUS) return -3;
		else if(data[1] != 0) return -2;
		length = data[2];
		for(i=0;i<length;i++)
		{
			if(data[4+(i*20)] == fd)
			{
				strcpy(host, &data[4+(i*20)+1]);
				strcpy(fdData[fd].host, &data[4+(i*20)+1]);
				return 0;
			}
		}
		
		
	}
	else
	{
		strcpy(host, fdData[fd].host);
		return 0;
	}
	return -1;
}

int esp_connect(char * host, uint16_t port)
{
	int fd = -1;
	uint8_t i;
	char data[36];
	uint16_t length;
	if(strlen(host) > 31) fd = -4;
	else
	{
		xSemaphoreTake(fdMutex, portMAX_DELAY);
		for(i=0;i<5;i++)
		{
			if(fdData[i].status == FD_DISCONNECTED)
			{
				fd = i;
				break;
			}
		}
		if(fd >= 0)
		{
			length = 36;
			data[0] = ESP_PACKET_CONNECT;
			data[1] = (char)fd;
			strcpy(&data[2], host);
			data[34] = port&0xFF;
			data[35] = port>>8;
			SendBlocking(ESP_MAILBOX_RX, data, &length);
			
			if(data[0] != ESP_PACKET_CONNECT) fd = -3;
			else if(data[1] != fd) fd = -2;
			else if(data[2] != 0) fd = -1;
			else
			{
				fdData[fd].host[0] = 0;
				fdData[fd].port = port;
				fdData[fd].status = FD_CONNECTED;
			}
		}
		xSemaphoreGive(fdMutex);
		
	}
	return fd;
}

int esp_disconnect(int fd)
{
	uint16_t length;
	char data[3];
	if(fd > 5 || fd < 0) return -4;
	if(fdData[fd].status == FD_DISCONNECTED) return -1;
	else
	{
		xSemaphoreTake(fdMutex, portMAX_DELAY);
		fdData[fd].host[0] = 0;
		fdData[fd].port = 0;
		fdData[fd].status = FD_DISCONNECTED;
		
		length = 2;
		data[0] = ESP_PACKET_DISCONNECT;
		data[1] = (char)fd;
		SendBlocking(ESP_MAILBOX_RX, data, &length);
		
		xSemaphoreGive(fdMutex);
		
		if(data[0] != ESP_PACKET_DISCONNECT) fd = -3;
		else if(data[1] != fd) fd = -2;
		else if(data[2] != 0) fd = -2;
		else fd = 0;
	}
	return fd;
}

int esp_send(int fd, char * buffer, uint16_t size, uint8_t mode) //mode==0 - BLOCKING, mode>=1 -  NON-BLOCKING
{
	uint16_t length,count;
	char data[5];
	if(fd > 5 || fd < 0) return -4;
	if(fdData[fd].status == FD_DISCONNECTED) return -2;
	else
	{
		while(size > 0)
		{
			if(size > ESP_FD_MAX_SEND_SIZE) count = ESP_FD_MAX_SEND_SIZE;
			else count = size;
			
			length = 5;
			data[0] = ESP_PACKET_SEND;
			data[1] = fd;
			data[2] = mode;
			data[3] = count&0xFF;
			data[4] = count>>8;
			if(mode > 0) SendLargeNonBlocking(ESP_MAILBOX_RX, data, &length, buffer, count);
			else
			{
				SendLargeBlocking(ESP_MAILBOX_RX, data, &length,buffer, count);
				
				if(data[0] != ESP_PACKET_SEND) return -3;
				else if(data[1] != fd) return -2;
				else if(data[2] != 0) return -1;
			}
			buffer += count;
			size -= count;
		}
		
		
	}
	return 0;
}

int esp_recv(int fd, char * buffer, uint16_t size)
{
	if(fd > 5 || fd < 0) return -4;
	if(fdData[fd].status == FD_DISCONNECTED) return -2;
	else
	{
		DynamicQueueReceive(fdData[fd].rxqueue,buffer,size);
	}
	return 0;
}

int esp_rxcheck(int fd)
{
	if(fd > 5 || fd < 0) return -4;
	if(fdData[fd].status == FD_DISCONNECTED) return -2;
	else
	{
		fd = DynamicQueueCheck(fdData[fd].rxqueue);
	}
	return fd;
}

void StartESPHandlerTask(void const * argument)
{
	/*
	printf("ESP8266 Handler Task started!\r\n");
	char * data = pvPortMalloc(1024);
	uint16_t length;
	
	
	int fd;
	esp_apconnect("DLINKNET","hertotamdolboeb");
	fd = esp_connect("google.com",80);
	strcpy(data, "GET / HTTP/1.1\r\nhost: google.com\r\n\r\n");
	esp_send(fd,data,strlen(data),0);
	osDelay(100);
	esp_recv(fd,data,esp_rxcheck(fd));
	esp_disconnect(fd);
	
	
	
	fd = esp_connect("online-kissfm.tavrmedia.ua",80);
	strcpy(data, "GET /KissFM HTTP/1.1\r\nhost: online-kissfm.tavrmedia.ua\r\n\r\n");
	esp_send(fd,data,strlen(data),0);
	
	for(;;)
	{
		length = esp_rxcheck(fd);
		if(length > 0)
			esp_recv(fd,data,length);
		else osDelay(1);
	}
	*/
	while(1) osDelay(1);
	
}

volatile uint8_t result;
void StartESPIOTask(void const * argument)
{
	uint32_t length,i,fd,ch,pt,st;
	TaskHandle_t * returntask;
	uint16_t * returnsize;
	char * returndata;
	uint8_t idle;
	int32_t rssi;
	ESP_PacketTypeDef packettype;
	printf("ESP8266 IO Task started!\r\n");
	char * data = pvPortMalloc(1536);
	
	fdMutex = xSemaphoreCreateMutex();
	for(i=0;i<5;i++)
	{
		fdData[i].host[0] = 0;
		fdData[i].port = 0;
		fdData[i].status = FD_DISCONNECTED;
		fdData[i].rxqueue = DynamicQueueCreate(ESP_FD_BUFFER_SIZE);
	}
	
	ESP_TX = DynamicQueueCreate(ESP_BUFFER_SIZE);
	ESP_RX = DynamicQueueCreate(ESP_BUFFER_SIZE);
	ESP_MAILBOX_TX = DynamicQueueCreate(ESP_MAILBOX_SIZE);
	ESP_MAILBOX_RX = DynamicQueueCreate(ESP_MAILBOX_SIZE);
	ESP_SendPacket_Buffer = pvPortMalloc(ESP_SEND_BUFFER_SIZE);
	
  osThreadDef(ESPRXTask, StartESPRXTask, osPriorityRealtime, 0, 512);
  ESPRXTaskHandle = osThreadCreate(osThread(ESPRXTask), NULL);
  osThreadDef(ESPTXTask, StartESPTXTask, osPriorityRealtime, 0, 512);
  ESPTXTaskHandle = osThreadCreate(osThread(ESPTXTask), NULL);
	
	//HAL_GPIO_WritePin(ESP_CH_PD_GPIO_Port, ESP_CH_PD_Pin, GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(ESP_NRST_GPIO_Port, ESP_NRST_Pin, GPIO_PIN_RESET);
	
	//HAL_GPIO_WritePin(ESP_NSS_GPIO_Port, ESP_NSS_Pin, GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(ESP_CH_PD_GPIO_Port, ESP_CH_PD_Pin, GPIO_PIN_SET);
	osDelay(10);
	//HAL_GPIO_WritePin(ESP_NRST_GPIO_Port, ESP_NRST_Pin, GPIO_PIN_SET);
	
	osDelay(1000);
	ESP_SetBaudrate(115200);
	result = ESP_SendMessage("AT\r\n");
	result = ESP_SendMessage("AT\r\n");
	result = ESP_SendMessage("AT+UART_CUR=2304000,8,1,0,0\r\n");
	osDelay(100);
	ESP_SetBaudrate(2304000);
	osDelay(100);
	result = ESP_SendMessage("AT\r\n");
	result = ESP_SendMessage("AT+CIPMUX=1\r\n");
	
	result = ESP_SendMessage("AT+CWMODE_CUR=1\r\n");
	osDelay(100);
	
	result = ESP_SendMessage("AT+CWQAP\r\n");
	osDelay(100);
	
	
  osThreadDef(ESPStackTask, StartESPHandlerTask, osPriorityNormal, 0, 512);
  ESPStackTaskHandle = osThreadCreate(osThread(ESPStackTask), NULL);
	
  for(;;)
  {
		idle = 1;
		if(DynamicQueueCheck(ESP_RX) > 2)
		{
			idle = 0;
			DynamicQueueReceive(ESP_RX, data, 2);
			if(data[0] == '\r' && data[1] == '\n')
			{
				length = DynamicQueueReceiveUntil(ESP_RX, data, ':');
				if(length > 7 && data[0] == '+')
				{
					if(data[1] == 'I' && data[2] == 'P' && data[3] == 'D' && data[4] == ',')
					{
						if(sscanf(data,"+IPD,%d,%d:",&fd,&length) > 0)
						{
							if(length > 0)
							{
								/*
								data[0] = ESP_PACKET_RECV;
								data[1] = fd;
								((uint16_t*)&data[2])[0] = length;
								DynamicQueueReceive(ESP_RX, &data[4], length);
								DynamicQueueSend(ESP_MAILBOX_TX, data, length+4);
								*/
								
								DynamicQueueReceive(ESP_RX, data, length);
								DynamicQueueSend(fdData[fd].rxqueue, data, length);
								
							}
						}
					}
				}
			}
			else
			{
				length = DynamicQueueReceiveUntil(ESP_RX, &data[2], '\n');
				if(length > 1)
				{
					data[length] = 0;
					if(sscanf(data,"%d,%s", &fd, &data[1024]) > 0)
					{
						if(strcmp(&data[1024], "CONNECT") == 0)
						{
							data[0] = ESP_PACKET_INCOMING;
							data[1] = fd;
							DynamicQueueSend(ESP_MAILBOX_TX, data, 2);
							DynamicQueueClear(fdData[fd].rxqueue);
						}
						else if(strcmp(&data[1024], "CLOSED") == 0)
						{
							data[0] = ESP_PACKET_DISCONNECT;
							data[1] = fd;
							DynamicQueueSend(ESP_MAILBOX_TX, data, 2);
						}
					}
				}
			}
			
		}
		if(DynamicQueueCheck(ESP_MAILBOX_RX) > 1)
		{
			idle = 0;
			DynamicQueueReceive(ESP_MAILBOX_RX, &returntask, sizeof(TaskHandle_t));
			DynamicQueueReceive(ESP_MAILBOX_RX, &returndata, sizeof(char*));
			DynamicQueueReceive(ESP_MAILBOX_RX, &returnsize, sizeof(uint16_t*));
			if(returntask == NULL) continue;
			
			DynamicQueueReceive(ESP_MAILBOX_RX, &packettype, 1);
			switch(packettype)
			{
				case ESP_PACKET_LISTEN :
				{
					DynamicQueueReceive(ESP_MAILBOX_RX, returndata, 2);
					result = ESP_SendMessage("AT+CIPSERVER=1,%d\r\n",((uint16_t*)&returndata[0])[0]);
					returndata[0] = ESP_PACKET_LISTEN;
					returndata[1] = result;
					*returnsize = 2;
					break;
				}
				case ESP_PACKET_UNLISTEN :
				{
					DynamicQueueReceive(ESP_MAILBOX_RX, returndata, 2);
					result = ESP_SendMessage("AT+CIPSERVER=0,%d\r\n",((uint16_t*)&returndata[0])[0]);
					returndata[0] = ESP_PACKET_LISTEN;
					returndata[1] = fd;
					returndata[2] = result;
					*returnsize = 3;
					break;
				}
				case ESP_PACKET_SEND :
				{
					DynamicQueueReceive(ESP_MAILBOX_RX, returndata, 4);
					fd = returndata[0];
					pt = returndata[1];
					result = ESP_SendMessage("AT+CIPSEND=%d,%d\r\n",fd,(returndata[2]&0xFF)+(returndata[3]<<8));
					length = (returndata[2]&0xFF)+(returndata[3]<<8);
					if(result != 0)
					{
						DynamicQueueReceive(ESP_MAILBOX_RX, NULL, length);
						if(pt == 0)
						{
							returndata[0] = ESP_PACKET_SEND;
							returndata[1] = fd;
							returndata[2] = result;
							*returnsize = 3;
						} else returntask = NULL;
					}
					else
					{
						DynamicQueueReceive(ESP_MAILBOX_RX, data, length);
						ESP_SendPacket(data, length);
						if(pt == 0)
						{
							returndata[0] = ESP_PACKET_SEND;
							returndata[1] = fd;
							returndata[2] = result;
							*returnsize = 3;
						} else returntask = NULL;
					}
					break;
				}
				case ESP_PACKET_RECV :
				{
					
					break;
				}
				case ESP_PACKET_INCOMING :
				{
					
					break;
				}
				case ESP_PACKET_DISCONNECT :
				{
					DynamicQueueReceive(ESP_MAILBOX_RX, returndata, 1);
					fd = returndata[0];
					result = ESP_SendMessage("AT+CIPCLOSE=%d\r\n",fd);
					returndata[0] = ESP_PACKET_DISCONNECT;
					returndata[1] = fd;
					returndata[2] = result;
					*returnsize = 3;
					break;
				}
				case ESP_PACKET_CONNECT :
				{
					DynamicQueueReceive(ESP_MAILBOX_RX, returndata, 35);
					fd = returndata[0];
					result = ESP_SendMessage("AT+CIPSTART=%d,\"TCP\",\"%s\",%d\r\n",returndata[0], &returndata[1], (returndata[33]&0xFF)+(returndata[34]<<8));
					returndata[0] = ESP_PACKET_CONNECT;
					returndata[1] = fd;
					returndata[2] = result;
					*returnsize = 3;
					DynamicQueueClear(fdData[fd].rxqueue);
					break;
				}
				case ESP_PACKET_APCONNECT :
				{
					DynamicQueueReceive(ESP_MAILBOX_RX, returndata, 64);
					result = ESP_SendMessage("AT+CWJAP_CUR=\"%s\",\"%s\"\r\n",&returndata[0],&returndata[32]);
					returndata[0] = ESP_PACKET_APCONNECT;
					returndata[1] = result;
					*returnsize = 2;
					break;
				}
				case ESP_PACKET_APDISCONNECT :
				{
					result = ESP_SendMessage("AT+CWQAP\r\n");
					returndata[0] = ESP_PACKET_APDISCONNECT;
					returndata[1] = result;
					*returnsize = 2;
					break;
				}
				case ESP_PACKET_APLIST :
				{
					result = ESP_SendMessageWithResult(data,"AT+CWLAP\r\n");
					length = 0;
					if(result == 0)
					{
						i = 0;
						while(data[i] != 0)
						{
							if(data[i] == '\r' || data[i] == '\n')
								data[i] = 0;
							i++;
						}
						fd = i;
						for(i=0;i<fd;i++)
						{
							if(data[i] == '+')
							{
								if(sscanf(&data[i],"+CWLAP:(%d,\"%[^\"]\",%d,\"%[^\"]\"",&ch,&returndata[4+(length*52)+20],&rssi,&returndata[4+(length*52)+2]) > 0)
								{
									returndata[4+(length*52)] = ch;
									returndata[4+(length*52)+1] = (int8_t)rssi;
									length++;
									i+=strlen(&data[i]);
								}
							}
						}
					}
					returndata[0] = ESP_PACKET_APDISCONNECT;
					returndata[1] = result;
					returndata[2] = length;
					returndata[3] = ':';
					*returnsize = 4+(length*52);
					break;
				}
				case ESP_PACKET_STATUS :
				{
					result = ESP_SendMessageWithResult(data,"AT+CIPSTATUS\r\n");
					length = 0;
					if(result == 0)
					{
						i = 0;
						while(data[i] != 0)
						{
							if(data[i] == '\r' || data[i] == '\n')
								data[i] = 0;
							i++;
						}
						fd = i;
						for(i=0;i<fd;i++)
						{
							if(data[i] == 'S')
							{
								if(sscanf(&data[i],"STATUS:%d",&st) > 0)
								{
									returndata[3] = ch;
									i+=strlen(&data[i]);
								}
							}
							else if(data[i] == '+')
							{
								if(sscanf(&data[i],"+CIPSTATUS:%d,\"%[^\"]\",\"%[^\"]\",%d,%d",&ch,(char*)NULL,&returndata[4+(length*20)+1],&pt,&st) > 0)
								{
									returndata[4+(length*20)+0] = ch;
									returndata[4+(length*20)+17] = pt&0xFF;
									returndata[4+(length*20)+18] = pt>>8;
									returndata[4+(length*20)+19] = st;
									length++;
									i+=strlen(&data[i]);
								}
							}
						}
					}
					returndata[0] = ESP_PACKET_STATUS;
					returndata[1] = result;
					returndata[2] = length;
					//returndata[3] = ':';
					*returnsize = 4+(length*20);
					break;
				}
				case ESP_PACKET_GETIP :
				{
					result = ESP_SendMessageWithResult(data,"AT+CIFSR\r\n");
					length = 0;
					if(result == 0)
					{
						i = 0;
						while(data[i] != 0)
						{
							if(data[i] == '\r' || data[i] == '\n')
								data[i] = 0;
							i++;
						}
						fd = i;
						for(i=0;i<fd;i++)
						{
							if(data[i] == '+')
							{
								if(sscanf(&data[i],"+CIFSR:STAIP,\"%[^\"]\"",&returndata[4+(length*20)+1]) > 0)
								{
									returndata[4+(length*20)] = 'I';
									length++;
									i+=strlen(&data[i]);
								}
								else if(sscanf(&data[i],"+CIFSR:STAMAC,\"%[^\"]\"",&returndata[4+(length*20)+1]) > 0)
								{
									returndata[4+(length*20)] = 'M';
									length++;
									i+=strlen(&data[i]);
								}
							}
						}
					}
					returndata[0] = ESP_PACKET_GETIP;
					returndata[1] = result;
					returndata[2] = length;
					returndata[3] = ':';
					*returnsize = 4+(length*20);
					break;
				}
				case ESP_PACKET_NSLOOKUP :
				{
					//SOMEWHY DOES NOT WORK?????
					DynamicQueueReceive(ESP_MAILBOX_RX, returndata, 32);
					result = ESP_SendMessageWithResult(data,"AT+CIPDOMAIN=\"%s\"\r\n",&returndata[0]);
					length = 0;
					if(result == 0)
					{
						i = 0;
						while(data[i] != 0)
						{
							if(data[i] == '\r' || data[i] == '\n')
								data[i] = 0;
							i++;
						}
						fd = i;
						for(i=0;i<fd;i++)
						{
							if(data[i] == '+')
							{
								if(sscanf(&data[i],"+CIPDOMAIN:\"%[^\"]\"",&returndata[4+(length*16)]) > 0)
								{
									length++;
									i+=strlen(&data[i]);
								}
							}
						}
					}
					returndata[0] = ESP_PACKET_NSLOOKUP;
					returndata[1] = result;
					returndata[2] = length;
					returndata[3] = ':';
					*returnsize = 4+(length*16);
					break;
				}
				case ESP_PACKET_GETMAC :
				{
					result = ESP_SendMessageWithResult(data,"AT+CIPSTAMAC?\r\n");
					if(result == 0)
					{
						i = 0;
						while(data[i] != 0)
						{
							if(data[i] == '\r' || data[i] == '\n')
								data[i] = 0;
							i++;
						}
						fd = i;
						result = 2;
						for(i=0;i<fd;i++)
						{
							if(returndata[i] == '+')
							{
								if(sscanf(&data[i],"+CIPSTAMAC:\"%[^\"]\"",&returndata[2]) > 0)
								{
									i+=strlen(&data[i]);
									result = 0;
								}
							}
						}
					}
					returndata[0] = ESP_PACKET_GETMAC;
					returndata[1] = result;
					*returnsize = 2+18;
					break;
				}
				case ESP_PACKET_GETHOST :
				{
					result = ESP_SendMessageWithResult(data,"AT+CWHOSTNAME?\r\n");
					if(result == 0)
					{
						i = 0;
						while(returndata[i] != 0)
						{
							if(data[i] == '\r' || data[i] == '\n')
								data[i] = 0;
							i++;
						}
						fd = i;
						result = 2;
						for(i=0;i<fd;i++)
						{
							if(returndata[i] == '+')
							{
								if(sscanf(&data[i],"+CWHOSTNAME:\"%[^\"]\"",&returndata[2]) > 0)
								{
									i+=strlen(&data[i]);
									result = 0;
								}
							}
						}
					}
					returndata[0] = ESP_PACKET_GETHOST;
					returndata[1] = result;
					*returnsize = 2+32;
					break;
				}
				case ESP_PACKET_SETMAC :
				{
					DynamicQueueReceive(ESP_MAILBOX_RX, returndata, 18);
					result = ESP_SendMessage("AT+CIPSTAMAC=\"%s\"\r\n",&returndata[0]);
					returndata[0] = ESP_PACKET_SETMAC;
					returndata[1] = result;
					*returnsize = 2;
					break;
				}
				case ESP_PACKET_SETHOST :
				{
					DynamicQueueReceive(ESP_MAILBOX_RX, returndata, 32);
					result = ESP_SendMessage("AT+CWHOSTNAME=\"%s\"\r\n",&returndata[0]);
					returndata[0] = ESP_PACKET_SETHOST;
					returndata[1] = result;
					*returnsize = 2;
					break;
				}
				case ESP_PACKET_PING :
				{
					DynamicQueueReceive(ESP_MAILBOX_RX, returndata, 32);
					result = ESP_SendMessageWithResult(data,"AT+PING=\"%s\"\r\n",&returndata[0]);
					if(result == 0)
					{
						i = 0;
						while(data[i] != 0)
						{
							if(data[i] == '\r' || data[i] == '\n')
								data[i] = 0;
							i++;
						}
						fd = i;
						result = 2;
						for(i=0;i<fd;i++)
						{
							if(data[i] == '+')
							{
								if(sscanf(&data[i],"+%d",&length) > 0)
								{
									i+=strlen(&data[i]);
									result = 0;
								}
							}
						}
					}
					returndata[0] = ESP_PACKET_PING;
					returndata[1] = result;
					returndata[2] = length&0xFF;
					returndata[3] = length>>8;
					*returnsize = 4;
					break;
				}
				default : 
				{
					continue;
				}
			}
			if(returntask != NULL) vTaskResume(returntask);
		}
		if(idle > 0) osDelay(1);
  }
}

