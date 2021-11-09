/**
  ******************************************************************************
  * @file           : usbd_audio_if.c
  * @brief          : Generic media access Layer.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "usbd_audio_if.h"
#include "mixer.h"

extern USBD_HandleTypeDef hUsbDeviceFS;


static int8_t  AUDIO_Init_FS         (uint32_t  AudioFreq, uint32_t Volume, uint32_t options, uint8_t * dac_buffer, uint8_t * adc_buffer, uint16_t size);
static int8_t  AUDIO_DeInit_FS       (uint32_t options);
static int8_t  AUDIO_AudioCmd_FS     (uint8_t* pbuf, uint32_t size, uint8_t cmd);
static int8_t  AUDIO_VolumeCtl_FS    (uint8_t vol);
static int8_t  AUDIO_MuteCtl_FS      (uint8_t cmd);
static int8_t  AUDIO_FreqCtl_FS      (uint32_t freq);
static int8_t  AUDIO_PeriodicTC_FS   (uint8_t cmd);
static int8_t  AUDIO_GetState_FS     (void);

USBD_AUDIO_ItfTypeDef USBD_AUDIO_fops_FS = 
{
  AUDIO_Init_FS,
  AUDIO_DeInit_FS,
  AUDIO_AudioCmd_FS,
  AUDIO_VolumeCtl_FS,
  AUDIO_MuteCtl_FS,
	AUDIO_FreqCtl_FS,
  AUDIO_PeriodicTC_FS,
  AUDIO_GetState_FS,
};

extern uint8_t DAC_Write(uint8_t addr, uint8_t data);
extern I2S_HandleTypeDef hi2s1;
extern I2S_HandleTypeDef hi2s2;

static int8_t AUDIO_Init_FS(uint32_t  AudioFreq, uint32_t Volume, uint32_t options, uint8_t * dac_buffer, uint8_t * adc_buffer, uint16_t size)
{ 
	
	//HAL_I2S_Receive_DMA(&hi2s1,(uint16_t*)adc_buffer, size/4);
	
	Mixer_AppendUSBBuffer(dac_buffer, size);
	
	/*
	DAC_Write(0x22,Volume-100);
	DAC_Write(0x23,Volume-100);
	DAC_Write(0x24,Volume-100);
	DAC_Write(0x25,Volume-100);
	*/
	//d_volume = Volume-100;
	//d_mute = 0;
	
	
	//volch = mutech = 1;
  return (USBD_OK);
  /* USER CODE END 0 */
}

static int8_t AUDIO_DeInit_FS(uint32_t options)
{
  /* USER CODE BEGIN 1 */ 
  return (USBD_OK);
  /* USER CODE END 1 */
}


static int8_t AUDIO_AudioCmd_FS (uint8_t* pbuf, uint32_t size, uint8_t cmd)
{
  /* USER CODE BEGIN 2 */
  switch(cmd)
  {
    case AUDIO_CMD_START: 
			
			break;
    case AUDIO_CMD_STOP: 
			
			break;
  }
  return (USBD_OK);
  /* USER CODE END 2 */
  
}

static int8_t AUDIO_VolumeCtl_FS (uint8_t vol)
{
  /* USER CODE BEGIN 3 */ 
	
	//DAC_Write(0x22,vol-100);
	//DAC_Write(0x23,vol-100);
	//DAC_Write(0x24,vol-100);
	//DAC_Write(0x25,vol-100);
	
	//d_volume = vol-100;
	//volch = 1;
  return (USBD_OK);
  /* USER CODE END 3 */
}

static int8_t AUDIO_FreqCtl_FS (uint32_t freq)
{
  /* USER CODE BEGIN 3 */ 
  return (USBD_OK);
  /* USER CODE END 3 */
}


static int8_t AUDIO_MuteCtl_FS (uint8_t cmd)
{
  /* USER CODE BEGIN 4 */ 
	//DAC_Write(0x04,cmd?0xFF:0xAA); //Mute
	//d_mute = cmd;
	//mutech = 1;
  return (USBD_OK);
  /* USER CODE END 4 */
}


static int8_t AUDIO_PeriodicTC_FS (uint8_t cmd)
{
  /* USER CODE BEGIN 5 */ 
  return (USBD_OK);
  /* USER CODE END 5 */
}


static int8_t AUDIO_GetState_FS (void)
{
  /* USER CODE BEGIN 6 */ 
  return (USBD_OK);
  /* USER CODE END 6 */
}


void USBAUDIO_DAC_TransferComplete_CallBack_FS(void)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) hUsbDeviceFS.pClassData;
	haudio->wr_step = 2;
}

void USBAUDIO_DAC_HalfTransfer_CallBack_FS(void)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) hUsbDeviceFS.pClassData;
	haudio->wr_step = 1;
}

void USBAUDIO_ADC_TransferComplete_CallBack_FS(void)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) hUsbDeviceFS.pClassData;
	
	uint16_t i;
	uint16_t * data = (uint16_t*)&haudio->in_buffer[AUDIO_TOTAL_BUF_SIZE/2];
	
	SCB_InvalidateDCache_by_Addr((uint32_t*)((uint32_t)data - ((uint32_t)data%32)), (AUDIO_TOTAL_BUF_SIZE/2) + ((uint32_t)data%32 > 0 ? 32 : 0));
	for(i=0;i<AUDIO_TOTAL_BUF_SIZE/4;i+=2)
	{
		data[i] ^= data[i+1];
		data[i+1] ^= data[i];
		data[i] ^= data[i+1];
	}
	
	haudio->rd_step = 2;
}


void USBAUDIO_ADC_HalfTransfer_CallBack_FS(void)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) hUsbDeviceFS.pClassData;

	uint16_t i;
	uint16_t * data = (uint16_t*)&haudio->in_buffer[0];
	SCB_InvalidateDCache_by_Addr((uint32_t*)((uint32_t)data - ((uint32_t)data%32)), (AUDIO_TOTAL_BUF_SIZE/2) + ((uint32_t)data%32 > 0 ? 32 : 0));
	for(i=0;i<AUDIO_TOTAL_BUF_SIZE/4;i+=2)
	{
		data[i] ^= data[i+1];
		data[i+1] ^= data[i];
		data[i] ^= data[i+1];
	}
	
	haudio->rd_step = 1;
}

