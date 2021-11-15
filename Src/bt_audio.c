#include "bt_audio.h"
#include "mixer.h"
#include "delay.h"

#define BT_I2S hi2s3
#define BT_SAI hsai_BlockA2

extern I2S_HandleTypeDef BT_I2S;
extern SAI_HandleTypeDef BT_SAI;


#define BT_RX_DATA_SIZE 32
#define BT_TX_DATA_SIZE 128

//#define BT_DATA_16BIT


#ifdef BT_DATA_16BIT
uint16_t BT_RX_Data[BT_RX_DATA_SIZE];
#else
uint32_t BT_RX_Data[BT_RX_DATA_SIZE];
#endif
uint16_t BT_TX_Data[BT_TX_DATA_SIZE];


#define I2S_THRESHOLD_MIN_8k 56
#define I2S_THRESHOLD_MID_8k 57
#define I2S_THRESHOLD_MAX_8k 58
#define I2S_THRESHOLD_OVERRUN_8k 60

#define I2S_THRESHOLD_MIN_44k 315
#define I2S_THRESHOLD_MID_44k 316
#define I2S_THRESHOLD_MAX_44k 317
#define I2S_THRESHOLD_OVERRUN_44k 320

#define I2S_THRESHOLD_MIN_48k 342
#define I2S_THRESHOLD_MID_48k 343
#define I2S_THRESHOLD_MAX_48k 345
#define I2S_THRESHOLD_OVERRUN_48k 360

volatile uint32_t I2S_THRESHOLD_MIN = I2S_THRESHOLD_MIN_44k;
volatile uint32_t I2S_THRESHOLD_MID = I2S_THRESHOLD_MID_44k;
volatile uint32_t I2S_THRESHOLD_MAX = I2S_THRESHOLD_MAX_44k;
volatile uint32_t I2S_THRESHOLD_OVERRUN = I2S_THRESHOLD_OVERRUN_44k;
volatile uint8_t SampleRate = 44;
volatile float fSampleRate = 0;

void BTAUDIO_CK_Callback()
{
	static uint32_t time_last = 0;
	
	uint32_t time_cur = Delay_Tick;
	uint32_t time = DelayDiff(time_cur, time_last);
	time_last = time_cur;
	float samplerate = 1000000.0f / (float)time;
	fSampleRate = samplerate;
	if(samplerate > 38000 && samplerate < 46000)
		SampleRate = 44;
	else if(samplerate > 46000 && samplerate < 52000)
		SampleRate = 48;
	else if(samplerate > 5000 && samplerate < 12000)
		SampleRate = 8;
	else if(samplerate > 12000 && samplerate < 18000)
		SampleRate = 16;
	else if(samplerate > 18000 && samplerate < 26000)
		SampleRate = 22;
	else if(samplerate > 26000 && samplerate < 38000)
		SampleRate = 33;
	else SampleRate = 0;
	
}

void ChangeSampleRate(void)
{
	static uint8_t samplerate_old = 0;
	uint8_t rate = SampleRate;
	if(rate != samplerate_old)
	{
		samplerate_old = rate;
		switch(rate)
		{
			case 8:
				I2S_THRESHOLD_MIN = I2S_THRESHOLD_MIN_8k;
				I2S_THRESHOLD_MID = I2S_THRESHOLD_MID_8k;
				I2S_THRESHOLD_MAX = I2S_THRESHOLD_MAX_8k;
				I2S_THRESHOLD_OVERRUN = I2S_THRESHOLD_OVERRUN_8k;
				break;
			case 44:
				I2S_THRESHOLD_MIN = I2S_THRESHOLD_MIN_44k;
				I2S_THRESHOLD_MID = I2S_THRESHOLD_MID_44k;
				I2S_THRESHOLD_MAX = I2S_THRESHOLD_MAX_44k;
				I2S_THRESHOLD_OVERRUN = I2S_THRESHOLD_OVERRUN_44k;
				break;
			case 48:
				I2S_THRESHOLD_MIN = I2S_THRESHOLD_MIN_48k;
				I2S_THRESHOLD_MID = I2S_THRESHOLD_MID_48k;
				I2S_THRESHOLD_MAX = I2S_THRESHOLD_MAX_48k;
				I2S_THRESHOLD_OVERRUN = I2S_THRESHOLD_OVERRUN_48k;
				break;
		}
	}
}


volatile uint16_t BT_I2S_MUL = 330;
volatile uint32_t BT_PNT = 0;

void BT_Init(void)
{
	HAL_GPIO_WritePin(BT_VEN_GPIO_Port, BT_VEN_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(BT_RST_GPIO_Port, BT_RST_Pin, GPIO_PIN_RESET);
	Mixer_AppendBTBuffer(BT_TX_Data, BT_TX_DATA_SIZE);
	HAL_SAI_Receive_DMA(&BT_SAI,(uint8_t*)BT_RX_Data, BT_RX_DATA_SIZE);
}

void BT_DeInit(void)
{
	HAL_SAI_DMAStop(&BT_SAI);
	HAL_GPIO_WritePin(BT_RST_GPIO_Port, BT_RST_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(BT_VEN_GPIO_Port, BT_VEN_Pin, GPIO_PIN_RESET);
}

void BTAUDIO_RX_HalfTransfer_CallBack(void)
{
	uint32_t i;
	ChangeSampleRate();
	if(BT_I2S_MUL > I2S_THRESHOLD_OVERRUN) 
	{
		for(i=0;i<BT_TX_DATA_SIZE;i++)
			BT_TX_Data[i] = 0;
		for(i=0;i<BT_RX_DATA_SIZE/2;i++,BT_PNT++)
		{
			if(BT_PNT >= BT_TX_DATA_SIZE) BT_PNT = 0;
		}
	}
	else
	{
		SCB_InvalidateDCache_by_Addr((uint32_t*)((uint32_t)BT_RX_Data - ((uint32_t)BT_RX_Data%32)), sizeof(BT_RX_Data)/2 + ((uint32_t)BT_RX_Data%32 > 0 ? 32 : 0));
		#ifndef BT_DATA_16BIT
		for(i=0;i<BT_RX_DATA_SIZE/2;i++,BT_PNT++)
		{
			if(BT_PNT >= BT_TX_DATA_SIZE) BT_PNT = 0;
			BT_TX_Data[BT_PNT] = BT_RX_Data[i] >> 16;
		}
		#else
		for(i=0;i<BT_RX_DATA_SIZE/2;i++,BT_PNT++)
		{
			if(BT_PNT >= BT_TX_DATA_SIZE) BT_PNT = 0;
			BT_TX_Data[BT_PNT] = BT_RX_Data[i] << 16;
		}
		#endif
	}
	SCB_CleanInvalidateDCache_by_Addr((uint32_t*)((uint32_t)BT_TX_Data - ((uint32_t)BT_TX_Data%32)), sizeof(BT_TX_Data) + ((uint32_t)BT_TX_Data%32 > 0 ? 32 : 0));
	Mixer_SyncBT();
}
#include "delay.h"
volatile uint32_t st,st1;

void BTAUDIO_RX_TransferComplete_CallBack(void)
{
	uint32_t i;
	
	ChangeSampleRate();
	
	if(BT_I2S_MUL > I2S_THRESHOLD_OVERRUN) 
	{
		
		for(i=0;i<BT_TX_DATA_SIZE;i++)
			BT_TX_Data[i] = 0;
		
		for(i=0;i<BT_RX_DATA_SIZE/2;i++,BT_PNT++)
		{
			if(BT_PNT >= BT_TX_DATA_SIZE) BT_PNT = 0;
		}
	}
	else
	{
		SCB_InvalidateDCache_by_Addr((uint32_t*)((uint32_t)&BT_RX_Data[BT_RX_DATA_SIZE/2] - ((uint32_t)&BT_RX_Data[BT_RX_DATA_SIZE/2]%32)), sizeof(BT_RX_Data)/2 + ((uint32_t)&BT_RX_Data[BT_RX_DATA_SIZE/2]%32 > 0 ? 32 : 0));
		
		#ifndef BT_DATA_16BIT
		for(i=BT_RX_DATA_SIZE/2;i<BT_RX_DATA_SIZE;i++,BT_PNT++)
		{
			if(BT_PNT >= BT_TX_DATA_SIZE) BT_PNT = 0;
			BT_TX_Data[BT_PNT] = BT_RX_Data[i] >> 16;
		}
		#else
		for(i=BT_RX_DATA_SIZE/2;i<BT_RX_DATA_SIZE;i++,BT_PNT++)
		{
			if(BT_PNT >= BT_TX_DATA_SIZE) BT_PNT = 0;
			BT_TX_Data[BT_PNT] = BT_RX_Data[i] << 16;
		}
		#endif
	}
	SCB_CleanInvalidateDCache_by_Addr((uint32_t*)((uint32_t)BT_TX_Data - ((uint32_t)BT_TX_Data%32)), sizeof(BT_TX_Data) + ((uint32_t)BT_TX_Data%32 > 0 ? 32 : 0));
	Mixer_SyncBT();
}


void BTAUDIO_TX_HalfTransfer_CallBack(void)
{
	ChangeSampleRate();
	st = DelayDiff(Delay_Tick,st1);
	st1 = Delay_Tick;
	uint16_t i;
	
	if(BT_PNT > BT_TX_DATA_SIZE/4*3-BT_TX_DATA_SIZE/64*3 && BT_PNT < BT_TX_DATA_SIZE/4*3+BT_TX_DATA_SIZE/64*3)
	{
		BT_I2S_MUL = I2S_THRESHOLD_MID;
	}
	else if(BT_PNT < BT_TX_DATA_SIZE/2) 
	{
		BT_I2S_MUL = I2S_THRESHOLD_OVERRUN;
		for(i=0;i<BT_TX_DATA_SIZE;i++)
			BT_TX_Data[i] = 0;
		SCB_CleanInvalidateDCache_by_Addr((uint32_t*)((uint32_t)BT_TX_Data - ((uint32_t)BT_TX_Data%32)), sizeof(BT_TX_Data) + ((uint32_t)BT_TX_Data%32 > 0 ? 32 : 0));
	}
	else
	{
		if(BT_PNT < BT_TX_DATA_SIZE/4*3-BT_TX_DATA_SIZE/16*3) BT_I2S_MUL = I2S_THRESHOLD_MIN; //Decrease DAC freq
		else if(BT_PNT > BT_TX_DATA_SIZE/4*3+BT_TX_DATA_SIZE/16*3) BT_I2S_MUL = I2S_THRESHOLD_MAX; //Increase DAC freq
	}
	__HAL_RCC_PLLI2S_CONFIG(BT_I2S_MUL , RCC_PLLP_DIV2, 2, 7);
	
}

void BTAUDIO_TX_TransferComplete_CallBack(void)
{	
	uint16_t i;
	ChangeSampleRate();
	
	if(BT_PNT > BT_TX_DATA_SIZE/4-BT_TX_DATA_SIZE/64*3 && BT_PNT < BT_TX_DATA_SIZE/4+BT_TX_DATA_SIZE/64*3)
	{
		BT_I2S_MUL = I2S_THRESHOLD_MID;
	}
	else if(BT_PNT > BT_TX_DATA_SIZE/2) 
	{
		BT_I2S_MUL = I2S_THRESHOLD_OVERRUN;
		for(i=0;i<BT_TX_DATA_SIZE;i++)
			BT_TX_Data[i] = 0;
		SCB_CleanInvalidateDCache_by_Addr((uint32_t*)((uint32_t)BT_TX_Data - ((uint32_t)BT_TX_Data%32)), sizeof(BT_TX_Data) + ((uint32_t)BT_TX_Data%32 > 0 ? 32 : 0));
	}
	else
	{
		if(BT_PNT < BT_TX_DATA_SIZE/4-BT_TX_DATA_SIZE/16*3) BT_I2S_MUL = I2S_THRESHOLD_MIN; //Decrease DAC freq
		else if(BT_PNT > BT_TX_DATA_SIZE/4+BT_TX_DATA_SIZE/16*3) BT_I2S_MUL = I2S_THRESHOLD_MAX; //Increase DAC freq
	}
	
	__HAL_RCC_PLLI2S_CONFIG(BT_I2S_MUL , RCC_PLLP_DIV2, 2, 7);
	
}

