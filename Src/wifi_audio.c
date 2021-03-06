#include "wifi_audio.h"
#include "mp3common.h"
#include "esp8266.h"
#include "dynamicqueue.h"
#include "mixer.h"
#include <string.h>

static void StartWebTask(void const * argument);
static void StartMP3Task(void const * argument);
DynamicQueueTypeDef * FrameBuffer;


osThreadId WebTaskHandle;
osThreadId MP3TaskHandle;

volatile uint8_t FreqMode = 0;
volatile uint8_t dacstep = 0;
HMP3Decoder MP3;
MP3FrameInfo MP3Info;
short int SampleBuffer[2304*2];
#define MP3BufferSize 1024
uint8_t mp3buff[MP3BufferSize];
uint32_t MP3ERRORS = 0;
uint32_t MP3DECODEERRORS = 0;
uint32_t MP3SUCCESS = 0;

#define WIFI_SIZE (2304*2)
static uint16_t WIFI_Buffer[WIFI_SIZE];
/*
extern RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
void FreqUp(void)
{
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 113; //113
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 5; //5
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
}
void FreqDown(void)
{
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 158; //112
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 7; //7
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
}
*/

void WIFIAUDIO_TX_HalfTransfer_CallBack(void)
{
	dacstep |= 1;
	if(((dacstep&3) == 3) && ((dacstep&0xC) != 8)) dacstep |= 4;
	
	if(FreqMode == 1)
	{
		__HAL_RCC_PLLI2S_CONFIG(317 , RCC_PLLP_DIV2, 2, 7);
		//FreqUp();
	}
	else if(FreqMode == 2)
	{
		__HAL_RCC_PLLI2S_CONFIG(315 , RCC_PLLP_DIV2, 2, 7);
		//FreqDown();
	}
}

void WIFIAUDIO_TX_TransferComplete_CallBack(void)
{
	dacstep |= 2;
	if(((dacstep&3) == 3) && ((dacstep&0xC) != 4)) dacstep |= 8;

	if(FreqMode == 1)
	{
		__HAL_RCC_PLLI2S_CONFIG(317 , RCC_PLLP_DIV2, 2, 7);
		//FreqUp();
	}
	else if(FreqMode == 2)
	{
		__HAL_RCC_PLLI2S_CONFIG(315 , RCC_PLLP_DIV2, 2, 7);
		//FreqDown();
	}
}

void WIFI_Init(void)
{
	ESP_Start();
	
	Mixer_AppendWIFIBuffer(WIFI_Buffer, WIFI_SIZE);
	
  osThreadDef(WebTask, StartWebTask, osPriorityBelowNormal, 0, 512);
  WebTaskHandle = osThreadCreate(osThread(WebTask), NULL);

  osThreadDef(MP3Task, StartMP3Task, osPriorityLow, 0, 1024); //osPriorityIdle
  MP3TaskHandle = osThreadCreate(osThread(MP3Task), NULL);
	
}


char netbuff[512];
void StartWebTask(void const * argument)
{
  /* init code for LWIP */
	int fd;
	volatile int16_t lng,i;
	uint32_t bytes,all;
	
	FrameBuffer = DynamicQueueCreate(40960);
	
	osDelay(100);
	
	while(1)
	{
		
		osDelay(3000);
		
		esp_apconnect("TPLINKNET","hertotamdolboeb");
		
		fd = esp_connect("online-kissfm.tavrmedia.ua",80);
		strcpy(netbuff, "GET /KissFM_Digital HTTP/1.1\r\nhost: online-kissfm2.tavrmedia.ua\r\n\r\n");
		
		esp_send(fd,netbuff,strlen(netbuff),0);
		
		bytes = all = 0;
		for(;;)
		{
			lng = 512;
			esp_recv(fd,netbuff,lng);
			
			all += lng;
			bytes += lng;
			DynamicQueueSend(FrameBuffer,netbuff,lng);
		}
	}
  /* USER CODE END 5 */ 
}




void StartMP3Task(void const * argument)
{
	int syncword;
	uint16_t shift,i;
	uint16_t left = 0;
	uint8_t * tempbuffer;
	int16_t * samplebuffer;
  MP3 = MP3InitDecoder();
	
	printf("Started!\r\n");
  for(;;)
  {
		
		if(DynamicQueueCheck(FrameBuffer) < 4096)
		{
			//DAC_Write(0x04,0xFF); //Mute
			for(i=0;i<2304*2;i++)
				SampleBuffer[i] = WIFI_Buffer[0] = 0;
			SCB_CleanInvalidateDCache_by_Addr((uint32_t*)((uint32_t)WIFI_Buffer - ((uint32_t)WIFI_Buffer%32)), sizeof(WIFI_Buffer) + ((uint32_t)WIFI_Buffer%32 > 0 ? 32 : 0));
			while(DynamicQueueCheck(FrameBuffer) < 20480)
			{
				osDelay(1);
			}
			//DAC_Write(0x04,0xAA); //Unmute
		}
		else if(DynamicQueueCheck(FrameBuffer) > 36720)
		{
			//DAC_Write(0x04,0xFF); //Mute
			
			for(i=0;i<2304*2;i++)
				SampleBuffer[i] = WIFI_Buffer[0] = 0;
			SCB_CleanInvalidateDCache_by_Addr((uint32_t*)((uint32_t)WIFI_Buffer - ((uint32_t)WIFI_Buffer%32)), sizeof(WIFI_Buffer) + ((uint32_t)WIFI_Buffer%32 > 0 ? 32 : 0));
			
			while(DynamicQueueCheck(FrameBuffer) > 20480)
			{
				DynamicQueueReceive(FrameBuffer,NULL,1000);
			}
			//DAC_Write(0x04,0xAA); //Unmute
		}
		
		if(DynamicQueueCheck(FrameBuffer) < 20480)
		{
			
			//if(FreqMode != 2)
			//{
				FreqMode = 2;
				//FreqDown();
			//}
		}
		else if(DynamicQueueCheck(FrameBuffer) > 30720) //57344
		{
			
			//if(FreqMode != 1)
			//{
				FreqMode = 1;
				//FreqUp();
			//}
		}
		
		DynamicQueueReceive(FrameBuffer,&mp3buff[left],MP3BufferSize-left);
		syncword = MP3FindSyncWord(mp3buff, MP3BufferSize);
		//printf("MP3FindSyncWord - %d\r\n",syncword);
		if(syncword < 0)
		{
			left = 0;
			continue;
		}
		else if(syncword > 0)
		{
			left = MP3BufferSize-syncword;
			for(i=0;i<left;i++)
				mp3buff[i] = mp3buff[i+syncword];
			DynamicQueueReceive(FrameBuffer,&mp3buff[left],syncword);
		}
		left = MP3BufferSize;
		syncword = MP3GetNextFrameInfo(MP3, &MP3Info, mp3buff);
		//printf("MP3GetNextFrameInfo - %d\r\n",syncword);
		if(syncword != 0)
		{
			MP3ERRORS++;
			left-=2;
			shift = MP3BufferSize-left;
			for(i=0;i<left;i++)
				mp3buff[i] = mp3buff[i+shift];
			continue;
		}
		
		tempbuffer = mp3buff;
		
		while(dacstep == 0) osDelay(1);
		
		if(((dacstep&5) > 0) && ((dacstep&0xC) != 8))
		{
			samplebuffer = SampleBuffer;
			syncword = MP3Decode(MP3, &tempbuffer, (int*)&left, samplebuffer, 0);
			for(i = 0; i < 2304; i++)
				WIFI_Buffer[i] = SampleBuffer[i];
			SCB_CleanDCache_by_Addr((uint32_t*)((uint32_t)WIFI_Buffer - ((uint32_t)WIFI_Buffer%32)), sizeof(WIFI_Buffer) + ((uint32_t)WIFI_Buffer%32 > 0 ? 32 : 0));
			dacstep &= ~5;
		}
		if(((dacstep&10) > 0) && ((dacstep&0xC) != 4))
		{
			samplebuffer = &SampleBuffer[2304];
			syncword = MP3Decode(MP3, &tempbuffer, (int*)&left, samplebuffer, 0);
			for(i = 2304; i < 2304*2; i++)
				WIFI_Buffer[i] = SampleBuffer[i] << 16;
			SCB_CleanDCache_by_Addr((uint32_t*)((uint32_t)WIFI_Buffer - ((uint32_t)WIFI_Buffer%32)), sizeof(WIFI_Buffer) + ((uint32_t)WIFI_Buffer%32 > 0 ? 32 : 0));
			dacstep &= ~10;
		}
		
		//printf("MP3Decode - %d, left %d\r\n",syncword,left);
		
		//HAL_GPIO_TogglePin(LED_D6_GPIO_Port, LED_D6_Pin);
		if(syncword != 0)
		{
			MP3DECODEERRORS++;
			left = 0;
			continue;
		}
		MP3GetLastFrameInfo(MP3, &MP3Info);
		
		MP3SUCCESS++;
		
		Mixer_SyncWIFI();
		shift = MP3BufferSize-left;
		for(i=0;i<left;i++)
			mp3buff[i] = mp3buff[i+shift];
		
  }
  /* USER CODE END StartMP3Task */
}

