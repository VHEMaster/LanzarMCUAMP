#include "mixer.h"
#include "usbd_audio_if.h"
#include "delay.h"
#include "task_dac.h"
#include "bt_audio.h"
#include "wifi_audio.h"
#include "task_gui.h"
#include "hd44780.h"

extern I2S_HandleTypeDef hi2s1;
extern I2S_HandleTypeDef hi2s2;

extern SAI_HandleTypeDef hsai_BlockA1;
extern SAI_HandleTypeDef hsai_BlockA2;

extern SPDIFRX_HandleTypeDef hspdif;
extern DMA_HandleTypeDef hdma_spdif_rx_dt;
extern DMA_HandleTypeDef hdma_spdif_rx_cs;

uint16_t * WIFI_Buffer;
uint16_t * USB_Buffer;
uint16_t * BT_Buffer;

uint32_t USB_Size = 0;
uint32_t BT_Size = 0;
uint32_t WIFI_Size = 0;


ModeTypeDef Mode = MODE_NONE;
ModeTypeDef OldMode = MODE_NONE;

uint64_t USB_LastTick = 0;
uint64_t BT_LastTick = 0;
uint64_t WIFI_LastTick = 0;

uint64_t USB_PrevTick = 0;
uint64_t BT_PrevTick = 0;
uint64_t WIFI_PrevTick = 0;

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	if(hi2s == &hi2s1)
	{
		if(Mode == MODE_USB)
			USBAUDIO_DAC_HalfTransfer_CallBack_FS();
		else if(Mode == MODE_BT)
			BTAUDIO_TX_HalfTransfer_CallBack();
		else if(Mode == MODE_WIFI) 
			WIFIAUDIO_TX_HalfTransfer_CallBack();
	}
}
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	if(hi2s == &hi2s1)
	{
		if(Mode == MODE_USB)
			USBAUDIO_DAC_TransferComplete_CallBack_FS();
		else if(Mode == MODE_BT)
			BTAUDIO_TX_TransferComplete_CallBack();
		else if(Mode == MODE_WIFI)
			WIFIAUDIO_TX_TransferComplete_CallBack();

	}
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
		BTAUDIO_RX_TransferComplete_CallBack();
}
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
		BTAUDIO_RX_HalfTransfer_CallBack();
}

void Mixer_AppendUSBBuffer(uint8_t * data, uint32_t size)
{
	USB_Buffer = (uint16_t *)data;
	USB_Size = size / 2;
}

void Mixer_AppendBTBuffer(uint16_t * data, uint32_t size)
{
	BT_Buffer = (uint16_t *)data;
	BT_Size = size;
}

void Mixer_AppendWIFIBuffer(uint16_t * data, uint32_t size)
{
	WIFI_Buffer = (uint16_t *)data;
	WIFI_Size = size;
}

#define SWITCH_DELAY 250
#define SWITCH_STANDBY 600000

extern RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
extern void GUI_SetMixerMode(ModeTypeDef mode);
extern void MX_I2S1_Init(void);
extern void MX_I2S2_Init(void);

void Mixer_TimerCallback(void)
{
	USB_LastTick = HAL_GetTick64() - USB_PrevTick;
	BT_LastTick = HAL_GetTick64() - BT_PrevTick;
	WIFI_LastTick = HAL_GetTick64() - WIFI_PrevTick;
	
	if(Mode == MODE_NONE)
	{
		if(BT_LastTick < SWITCH_DELAY)
		{
			Mode = MODE_BT;
		}
		else if(USB_LastTick < SWITCH_DELAY)
		{
			Mode = MODE_USB;
		}
		else if(WIFI_LastTick < SWITCH_DELAY)
		{
			Mode = MODE_WIFI;
		}
	}
	else if(Mode == MODE_BT)
	{
		if(BT_LastTick > USB_LastTick + SWITCH_DELAY)
		{
			Mode = MODE_USB;
		}
		else if(BT_LastTick > WIFI_LastTick + SWITCH_DELAY)
		{
			Mode = MODE_WIFI;
		}
		else if(BT_LastTick > SWITCH_DELAY)
		{
			Mode = MODE_NONE;
		}
	}
	else if(Mode == MODE_USB)
	{
		if(BT_LastTick < SWITCH_DELAY)
		{
			Mode = MODE_BT;
		}
		else if(USB_LastTick > WIFI_LastTick + SWITCH_DELAY)
		{
			Mode = MODE_WIFI;
		}
		else if(USB_LastTick > SWITCH_DELAY)
		{
			Mode = MODE_NONE;
		}
	}
	else if(Mode == MODE_WIFI)
	{
		if(USB_LastTick < SWITCH_DELAY)
		{
			Mode = MODE_USB;
		}
		if(BT_LastTick < SWITCH_DELAY)
		{
			Mode = MODE_BT;
		}
		else if(WIFI_LastTick > SWITCH_DELAY)
		{
			Mode = MODE_NONE;
		}
	}
	
	if(Mode != OldMode)
	{
		if(Mode == MODE_NONE)
		{
			DAC_SetMute(1);
			DAC_HP_SetMute(1);
		}
		else
		{
			uint32_t isr;
			
			isr = taskENTER_CRITICAL_FROM_ISR();
		  if((hi2s1.Instance->SR & I2S_FLAG_BSY) > 0)
			{
				while((hi2s1.hdmatx->Instance->NDTR % 16) == 0) {}
				while((hi2s1.hdmatx->Instance->NDTR % 16) > 0) {}
			}
			HAL_I2S_DMAStop(&hi2s1);
			HAL_I2S_DeInit(&hi2s1);
			taskEXIT_CRITICAL_FROM_ISR(isr);
			
			isr = taskENTER_CRITICAL_FROM_ISR(); 
		  if((hi2s2.Instance->SR & I2S_FLAG_BSY) > 0)
			{
				while((hi2s2.hdmatx->Instance->NDTR % 16) == 0) {}
				while((hi2s2.hdmatx->Instance->NDTR % 16) > 0) {}
			}
			HAL_I2S_DMAStop(&hi2s2);
			HAL_I2S_DeInit(&hi2s2);
			taskEXIT_CRITICAL_FROM_ISR(isr);
			
			isr = taskENTER_CRITICAL_FROM_ISR();
			MX_I2S1_Init();
			MX_I2S2_Init();
			taskEXIT_CRITICAL_FROM_ISR(isr);
			
			if(Mode == MODE_USB)
			{
				if(USB_Buffer == NULL) return;
				__HAL_RCC_PLLI2S_CONFIG(343 , RCC_PLLP_DIV2, 2, 7);
				HAL_I2S_Transmit_DMA(&hi2s1,(uint16_t*)USB_Buffer, USB_Size);
				HAL_I2S_Transmit_DMA(&hi2s2,(uint16_t*)USB_Buffer, USB_Size);
			}
			else if(Mode == MODE_WIFI)
			{
				if(WIFI_Buffer == NULL) return;
				__HAL_RCC_PLLI2S_CONFIG(343 , RCC_PLLP_DIV2, 2, 7);
				HAL_I2S_Transmit_DMA(&hi2s1,(uint16_t*)WIFI_Buffer, WIFI_Size);
				HAL_I2S_Transmit_DMA(&hi2s2,(uint16_t*)WIFI_Buffer, WIFI_Size);
			}
			else if(Mode == MODE_BT)
			{
				if(BT_Buffer == NULL) return;
				__HAL_RCC_PLLI2S_CONFIG(343 , RCC_PLLP_DIV2, 2, 7);
				HAL_I2S_Transmit_DMA(&hi2s1,(uint16_t*)BT_Buffer, BT_Size);
				HAL_I2S_Transmit_DMA(&hi2s2,(uint16_t*)BT_Buffer, BT_Size);
			}
			DAC_SetMute(0);
			DAC_HP_SetMute(0);
		}
		OldMode = Mode;
		GUI_SetMixerMode(Mode);
	}
	if(USB_LastTick > SWITCH_STANDBY && BT_LastTick > SWITCH_STANDBY && WIFI_LastTick > SWITCH_STANDBY)
	{
		HAL_GPIO_WritePin(PWR_EN_GPIO_Port, PWR_EN_Pin, GPIO_PIN_RESET);
		lcd_led_off();
	}
	else 
	{
		HAL_GPIO_WritePin(PWR_EN_GPIO_Port, PWR_EN_Pin, GPIO_PIN_SET);
		lcd_led_on();
	}
}


void Mixer_SyncUSB(void)
{
	static int32_t CCR_OLD = 0;
	static uint16_t I2SN_old = 0;
	static int32_t CCR_DELTA = 0;
	static uint16_t I2SN = 343;
	int32_t CCR = TIM2->CNT;
	
	if(CCR_OLD > CCR) CCR_DELTA = ((int32_t)0xFFFFFFFF-CCR_OLD+CCR+CCR_DELTA-12288)%12288;
	else CCR_DELTA = (CCR_DELTA+CCR-CCR_OLD-12288)%12288;
		
	CCR_OLD = CCR;
	
	
	if(Mode == MODE_USB)
	{		
		if(CCR_DELTA < 25 && CCR_DELTA > -25) I2SN = 343;
		else if(CCR_DELTA > 100) I2SN = 341;
		else if(CCR_DELTA > 0) I2SN = 342;
		else if(CCR_DELTA < -100) I2SN = 345;
		else if(CCR_DELTA < 0) I2SN = 344;
		if(I2SN_old != I2SN)
		{
			I2SN_old = I2SN;
			__HAL_RCC_PLLI2S_CONFIG(I2SN , RCC_PLLP_DIV2, 2, 7);
		}
		
	}
	USB_PrevTick = HAL_GetTick64();
}



void Mixer_SyncBT(void)
{
	BT_PrevTick = HAL_GetTick64();
}

void Mixer_SyncWIFI(void)
{
	WIFI_PrevTick = HAL_GetTick64();
}
