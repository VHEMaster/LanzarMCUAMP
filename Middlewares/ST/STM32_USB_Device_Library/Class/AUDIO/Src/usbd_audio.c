
/**
  ******************************************************************************
  * @file    usbd_audio.c
  * @author  MCD Application Team
  * @version V2.4.1
  * @date    19-June-2015
  * @brief   This file provides the Audio core functions.
  *
  * @verbatim
  *
  *          ===================================================================
  *                                AUDIO Class  Description
  *          ===================================================================
 *           This driver manages the Audio Class 1.0 following the "USB Device Class Definition for
  *           Audio Devices V1.0 Mar 18, 98".
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Standard AC Interface Descriptor management
  *             - 1 Audio Streaming Interface (with single channel, PCM, Stereo mode)
  *             - 1 Audio Streaming Endpoint
  *             - 1 Audio Terminal Input (1 channel)
  *             - Audio Class-Specific AC Interfaces
  *             - Audio Class-Specific AS Interfaces
  *             - AudioControl Requests: only SET_CUR and GET_CUR requests are supported (for Mute)
  *             - Audio Feature Unit (limited to Mute control)
  *             - Audio Synchronization type: Asynchronous
  *             - Single fixed audio sampling rate (configurable in usbd_conf.h file)
  *          The current audio class version supports the following audio features:
  *             - Pulse Coded Modulation (PCM) format
  *             - sampling rate: 48KHz.
  *             - Bit resolution: 16
  *             - Number of channels: 2
  *             - No volume control
  *             - Mute/Unmute capability
  *             - Asynchronous Endpoints
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "usbd_audio.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_AUDIO
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_AUDIO_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_AUDIO_Private_Defines
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_AUDIO_Private_Macros
  * @{
  */
#define AUDIO_SAMPLE_FREQ(frq)      (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))

#define AUDIO_PACKET_SZE(frq)          (uint8_t)((AUDIO_OUT_PACKET) & 0xFF), \
                                       (uint8_t)(((AUDIO_OUT_PACKET) >> 8) & 0xFF)

/**
  * @}
  */




/** @defgroup USBD_AUDIO_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_AUDIO_Init (USBD_HandleTypeDef *pdev,
                                 uint8_t cfgidx);

static uint8_t  USBD_AUDIO_DeInit (USBD_HandleTypeDef *pdev,
                                   uint8_t cfgidx);

static uint8_t  USBD_AUDIO_Setup (USBD_HandleTypeDef *pdev,
                                  USBD_SetupReqTypedef *req);

static uint8_t  *USBD_AUDIO_GetCfgDesc (uint16_t *length);

static uint8_t  *USBD_AUDIO_GetDeviceQualifierDesc (uint16_t *length);

static uint8_t  USBD_AUDIO_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_AUDIO_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_AUDIO_EP0_RxReady (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_AUDIO_EP0_TxReady (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_AUDIO_SOF (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_AUDIO_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_AUDIO_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void AUDIO_REQ_GetMin(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void AUDIO_REQ_GetMax(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void AUDIO_REQ_GetRes(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

#define CUR_MIN ((int16_t)0xDBE0)
#define CUR_MAX ((int16_t)0x2420)
#define CUR_RES ((int16_t)0x0023)

/**
  * @}
  */

/** @defgroup USBD_AUDIO_Private_Variables
  * @{
  */

USBD_ClassTypeDef  USBD_AUDIO =
{
  USBD_AUDIO_Init,
  USBD_AUDIO_DeInit,
  USBD_AUDIO_Setup,
  USBD_AUDIO_EP0_TxReady,
  USBD_AUDIO_EP0_RxReady,
  USBD_AUDIO_DataIn,
  USBD_AUDIO_DataOut,
  USBD_AUDIO_SOF,
  USBD_AUDIO_IsoINIncomplete,
  USBD_AUDIO_IsoOutIncomplete,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetDeviceQualifierDesc,
};

/* USB AUDIO device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_AUDIO_CfgDesc[USB_AUDIO_CONFIG_DESC_SIZ] __ALIGN_END =
{

  /* Configuration 1 */
  0x09,                                 /* bLength */
  USB_DESC_TYPE_CONFIGURATION,          /* bDescriptorType */
  LOBYTE(USB_AUDIO_CONFIG_DESC_SIZ),    /* wTotalLength  182 bytes*/
  HIBYTE(USB_AUDIO_CONFIG_DESC_SIZ),
  0x03,                                 /* bNumInterfaces */
  0x01,                                 /* bConfigurationValue */
  0x00,                                 /* iConfiguration */
  0xC0,                                 /* bmAttributes  BUS Powred*/
  0x7D,                                 /* bMaxPower = 100 mA*/
  /* 09 byte*/

  /* Interface Association Descriptor - Audio */
  0x08,            					          	/* bLength */
  0x0B,              					          /* bDescriptorType - Interface Association */
  0x00,                                 /* bFirstInterface */
  0x03,                                 /* bInterfaceCount */
  0x01,                                 /* bFunctionClass */
  0x00,                                 /* bFunctionSubClass */
  0x00,                                 /* bFunctionProtocol */
  0x00,                                 /* iFunction */
  /* 08 byte*/

  /* Interface Descriptor 0/0 - Audio, 0 Endpoints */
  0x09,                                 /* bLength */
  0x04,                                 /* bDescriptorType - Interface */
  0x00,                                 /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x00,                                 /* bNumEndpoints */
  0x01,                                 /* bInterfaceClass */
  0x01,                                 /* bInterfaceSubClass */
  0x00,                                 /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/

  /* Audio Control Interface Header Descriptor  */
  0x0A,                                 /* bLength */
  0x24,                                 /* bDescriptorType - Audio Control Interface Header */
  0x01,                                 /* bDescriptorSubtype */
  0x00,                                 /* bcdADC */
  0x01,                                 
  0x3D,                                 /* wTotalLength */
  0x00,                                 
  0x02,                                 /* bInCollection */
  0x01,                                 /* baInterfaceNr(1) */
  0x02,                                 /* baInterfaceNr(2) */
  /* 10 byte*/

  /* Audio Control Input Terminal Descriptor  */
  0x0C,                                 /* bLength */
  0x24,                                 /* bDescriptorType - Audio Control Input Terminal */
  0x02,                                 /* bDescriptorSubtype */
  0x1E,                                 /* bTerminalID */
  0x03,                                 /* wTerminalType */
  0x06,                                 
  0x00,                                 /* bAssocTerminal */
  0x02,                                 /* bNrChannels */
  0x03,                                 /* wChannelConfig */
  0x00,                                 
  0x00,                                 /* iChannelNames */
  0x00,                                 /* iTerminal */
  /* 12 byte*/
	
  /* Audio Control Output Terminal Descriptor  */
  0x09,                                 /* bLength */
  0x24,                                 /* bDescriptorType - Audio Control Output Terminal */
  0x03,                                 /* bDescriptorSubtype */
  0x23,                                 /* bTerminalID */
  0x01,                                 /* wTerminalType */
  0x01,                                 
  0x00,                                 /* bAssocTerminal */
  0x1E,                                 /* bSourceID */
  0x00,                                 /* iTerminal */
  /* 09 byte*/

  /* Audio Control Input Terminal Descriptor  */
  0x0C,                                 /* bLength */
  0x24,                                 /* bDescriptorType - Audio Control Input Terminal */
  0x02,                                 /* bDescriptorSubtype */
  0x0A,                                 /* bTerminalID */
  0x01,                                 /* wTerminalType */
  0x01,                                 
  0x00,                                 /* bAssocTerminal */
  0x02,                                 /* bNrChannels */
  0x03,                                 /* wChannelConfig */
  0x00,                                 
  0x00,                                 /* iChannelNames */
  0x00,                                 /* iTerminal */
  /* 12 byte*/
	


  /* USB Speaker Audio Feature Unit Descriptor */
  0x09,                                 /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_FEATURE_UNIT,           /* bDescriptorSubtype */
  AUDIO_OUT_FEATURE_ID,                 /* bUnitID */
  0x0A,                                 /* bSourceID */
  0x01,                                 /* bControlSize */
  AUDIO_CONTROL_SUPPORT_MUTE, //| AUDIO_CONTROL_SUPPORT_VOLUME, /* bmaControls(0) */
  0,                                    /* bmaControls(1) */
  0x00,                                 /* iTerminal */
  /* 09 byte*/

  /* Audio Control Output Terminal Descriptor  */
  0x09,                                 /* bLength */
  0x24,                                 /* bDescriptorType - Audio Control Output Terminal */
  0x03,                                 /* bDescriptorSubtype */
  0x0F,                                 /* bTerminalID */
  0x01,                                 /* wTerminalType */
  0x03,                                 
  0x00,                                 /* bAssocTerminal */
  AUDIO_OUT_FEATURE_ID,                 /* bSourceID */
  0x00,                                 /* iTerminal */
  /* 09 byte*/

  /* Interface Descriptor 1/0 - Audio, 0 Endpoints */
  0x09,                                 /* bLength */
  0x04,                                 /* bDescriptorType - Interface */
  0x01,                                 /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x00,                                 /* bNumEndpoints */
  0x01,                                 /* bInterfaceClass */
  0x02,                                 /* bInterfaceSubClass */
  0x00,                                 /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/

  /* Interface Descriptor 1/1 - Audio, 1 Endpoints */
  0x09,                                 /* bLength */
  0x04,                                 /* bDescriptorType - Interface */
  0x01,                                 /* bInterfaceNumber */
  0x01,                                 /* bAlternateSetting */
  0x01,                                 /* bNumEndpoints */
  0x01,                                 /* bInterfaceClass */
  0x02,                                 /* bInterfaceSubClass */
  0x00,                                 /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/

  /* Audio Streaming Interface Descriptor  */
  0x07,                                 /* bLength */
  0x24,                                 /* bDescriptorType - Audio Streaming Interface */
  0x01,                                 /* bDescriptorSubtype */
  0x0A,                                 /* bTerminalLink */
  0x01,                                 /* bDelay */
  0x01,                                 /* wFormatTag */
  0x00,                                 
  /* 07 byte*/

  /* Audio Streaming Format Type Descriptor   */
  0x0B,                                 /* bLength */
  0x24,                                 /* bDescriptorType - Audio Streaming Format Type  */
  0x02,                                 /* bDescriptorSubtype */
  0x01,                                 /* bFormatType */
  0x02,                                 /* bNrChannels */
  0x04,                                 /* bSubFrameSize */
  0x20,                                 /* bBitResolution */
  0x01,                                 /* bSamFreqType */
  AUDIO_SAMPLE_FREQ(USBD_AUDIO_FREQ),   /* tSamFreq */        
  /* 11 byte*/

  /* Endpoint Descriptor 01 1 Out, Isochronous, 1 ms */
  0x09,                                 /* bLength */
  0x05,                                 /* bDescriptorType - Endpoint */
  AUDIO_OUT_EP,                         /* bEndpointAddress */
  0x09,                                 /* bmAttributes - Isochronous, Asynchronous, Data */
  AUDIO_PACKET_SZE(USBD_AUDIO_FREQ),    /* wMaxPacketSize */                                 
  0x01,                                 /* bInterval */
  0x00,                                 /* bRefresh */
  0x00,                                 /* bSynchAddress */
  /* 09 byte*/
 
  /* Audio Streaming Isochronous Audio Data Endpoint Descriptor   */
  0x07,                                 /* bLength */
  0x25,                                 /* bDescriptorType - Audio Streaming Isochronous Audio Data Endpoint */
  0x01,                                 /* bDescriptorSubtype */
  0x00,                                 /* bmAttributes */
  0x00,                                 /* bLockDelayUnits */
  0x00,                                 /* wLockDelay */
  0x00,                                                                  
  /* 07 byte*/

  /* Interface Descriptor 2/0 - Audio, 0 Endpoints */
  0x09,                                 /* bLength */
  0x04,                                 /* bDescriptorType - Interface */
  0x02,                                 /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x00,                                 /* bNumEndpoints */
  0x01,                                 /* bInterfaceClass */
  0x02,                                 /* bInterfaceSubClass */
  0x00,                                 /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/

  /* Interface Descriptor 2/1 - Audio, 1 Endpoints */
  0x09,                                 /* bLength */
  0x04,                                 /* bDescriptorType - Interface */
  0x02,                                 /* bInterfaceNumber */
  0x01,                                 /* bAlternateSetting */
  0x01,                                 /* bNumEndpoints */
  0x01,                                 /* bInterfaceClass */
  0x02,                                 /* bInterfaceSubClass */
  0x00,                                 /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/

  /* Audio Streaming Interface Descriptor  */
  0x07,                                 /* bLength */
  0x24,                                 /* bDescriptorType - Audio Streaming Interface */
  0x01,                                 /* bDescriptorSubtype */
  0x23,                                 /* bTerminalLink */
  0x01,                                 /* bDelay */
  0x01,                                 /* wFormatTag */
  0x00,                                 
  /* 07 byte*/

  /* Audio Streaming Format Type Descriptor   */
  0x0B,                                 /* bLength */
  0x24,                                 /* bDescriptorType - Audio Streaming Format Type  */
  0x02,                                 /* bDescriptorSubtype */
  0x01,                                 /* bFormatType */
  0x02,                                 /* bNrChannels */
  0x04,                                 /* bSubFrameSize */
  0x20,                                 /* bBitResolution */
  0x01,                                 /* bSamFreqType */
  AUDIO_SAMPLE_FREQ(USBD_AUDIO_FREQ),   /* tSamFreq */        
  /* 11 byte*/

  /* Endpoint Descriptor 81 1 In, Isochronous, 1 ms */
  0x09,                                 /* bLength */
  0x05,                                 /* bDescriptorType - Endpoint */
  AUDIO_IN_EP,                          /* bEndpointAddress */
  0x29,                                 /* bmAttributes - Isochronous, Asynchronous, Implicit Feedback Data */
  AUDIO_PACKET_SZE(USBD_AUDIO_FREQ),    /* wMaxPacketSize */                                 
  0x01,                                 /* bInterval */
  0x00,                                 /* bRefresh */
  0x00,                                 /* bSynchAddress */
  /* 09 byte*/
  
  /* Audio Streaming Isochronous Audio Data Endpoint Descriptor   */
  0x07,                                 /* bLength */
  0x25,                                 /* bDescriptorType - Audio Streaming Isochronous Audio Data Endpoint */
  0x01,                                 /* bDescriptorSubtype */
  0x00,                                 /* bmAttributes */
  0x00,                                 /* bLockDelayUnits */
  0x00,                                 /* wLockDelay */
  0x00,                                                                  
  /* 07 byte*/
  
  

	
} ;

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_AUDIO_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/**
  * @}
  */

/** @defgroup USBD_AUDIO_Private_Functions
  * @{
  */

/**
  * @brief  USBD_AUDIO_Init
  *         Initialize the AUDIO interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */


static uint8_t  USBD_AUDIO_Init (USBD_HandleTypeDef *pdev,
                                 uint8_t cfgidx)
{
  USBD_AUDIO_HandleTypeDef   *haudio;

  /* Open EP OUT */
  USBD_LL_OpenEP(pdev,
                 AUDIO_OUT_EP,
                 USBD_EP_TYPE_ISOC,
                 AUDIO_OUT_PACKET);
  USBD_LL_OpenEP(pdev,
                 AUDIO_IN_EP,
                 USBD_EP_TYPE_ISOC,
                 AUDIO_IN_PACKET);

  /* Allocate Audio structure */
  pdev->pClassData = USBD_malloc(sizeof (USBD_AUDIO_HandleTypeDef));

  if (pdev->pClassData == NULL)
  {
    return USBD_FAIL;
  }
  else
  {
    haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
    haudio->alt_setting = 1;
    haudio->wr_step = 0;
    haudio->rd_step = 0;
    haudio->wr_ptr = 0;
    haudio->rd_ptr = 0;
    haudio->freq = USBD_AUDIO_FREQ;
    haudio->out_packet = (uint32_t)(((haudio->freq * 4 * 2) / 1000));
    haudio->buf_size = ((uint32_t)(haudio->out_packet * AUDIO_OUT_PACKET_NUM));
    /* Initialize the Audio output Hardware layer */
    haudio->vol = AUDIO_DEFAULT_VOLUME;
    if (((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->Init(USBD_AUDIO_FREQ, AUDIO_DEFAULT_VOLUME, 0, haudio->out_buffer, haudio->in_buffer, haudio->buf_size) != USBD_OK)
    {
      return USBD_FAIL;
    }

		
    /* Prepare Out endpoint to receive 1st packet */
    USBD_LL_PrepareReceive(pdev,
                           AUDIO_OUT_EP,
                           haudio->out_buffer,
                           haudio->out_packet);
  }
  return USBD_OK;
}

/**
  * @brief  USBD_AUDIO_Init
  *         DeInitialize the AUDIO layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_AUDIO_DeInit (USBD_HandleTypeDef *pdev,
                                   uint8_t cfgidx)
{

  /* Open EP OUT */
  USBD_LL_CloseEP(pdev,
                  AUDIO_OUT_EP);
  USBD_LL_CloseEP(pdev,
                  AUDIO_IN_EP);

  /* DeInit  physical Interface components */
  if (pdev->pClassData != NULL)
  {
    ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->DeInit(0);
    USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }

  return USBD_OK;
}

/**
  * @brief  USBD_AUDIO_Setup
  *         Handle the AUDIO specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
uint8_t sof_req = 0;
static uint8_t  USBD_AUDIO_Setup (USBD_HandleTypeDef *pdev,
                                  USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  uint16_t len;
  uint8_t *pbuf;
  uint8_t ret = USBD_OK;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :
    switch (req->bRequest)
    {
    case AUDIO_REQ_GET_CUR:
      AUDIO_REQ_GetCurrent(pdev, req);
      break;

    case AUDIO_REQ_GET_MIN:
      AUDIO_REQ_GetMin(pdev, req);
      break;
		
    case AUDIO_REQ_GET_RES:
      AUDIO_REQ_GetRes(pdev, req);
      break;

    case AUDIO_REQ_GET_MAX:
      AUDIO_REQ_GetMax(pdev, req);
      break;

    case AUDIO_REQ_SET_CUR:
      AUDIO_REQ_SetCurrent(pdev, req);
      break;

    default:
      USBD_CtlError (pdev, req);
      ret = USBD_FAIL;
    }
    break;

  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR:
      if ( (req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE)
      {
        pbuf = USBD_AUDIO_CfgDesc + 18;
        len = MIN(USB_AUDIO_DESC_SIZ , req->wLength);


        USBD_CtlSendData (pdev,
                          pbuf,
                          len);
      }
      break;

    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *) & (haudio->alt_setting),
                        1);
      break;

    case USB_REQ_SET_INTERFACE :
      if ((uint8_t)(req->wValue) <= USBD_MAX_NUM_INTERFACES)
      {
        haudio->alt_setting = (uint8_t)(req->wValue);
				if(req->wIndex == AUDIO_OUT_LINEIN_ID)
				{
					if(haudio->alt_setting == 0)
					{
						USBD_LL_FlushEP(pdev, AUDIO_IN_EP);
					}
					else sof_req = 1;
				}
      }
      else
      {
        /* Call the error management function (command will be nacked */
        USBD_CtlError (pdev, req);
      }
      break;

    default:
      USBD_CtlError (pdev, req);
      ret = USBD_FAIL;
    }
  }
  return ret;
}


/**
  * @brief  USBD_AUDIO_GetCfgDesc
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_AUDIO_GetCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_AUDIO_CfgDesc);
  return USBD_AUDIO_CfgDesc;
}

/**
  * @brief  USBD_AUDIO_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */




uint8_t d_reset = 0;
uint32_t d_in = 0, d_out = 0;


uint32_t sync_tick_dac = 0;
uint32_t sync_tick_adc = 0;
uint32_t sync_tick_sof = 0;
volatile uint8_t sync_active = 0;

#define SYNC_ADC 1
#define SYNC_DAC 2


static uint8_t  USBD_AUDIO_DataIn (USBD_HandleTypeDef *pdev,
                                   uint8_t epnum)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
	
	if(epnum == (AUDIO_IN_EP & 0x7F))
	{
		uint8_t * data = &haudio->in_buffer[0];
		data += haudio->rd_ptr;
		
		haudio->rd_ptr += AUDIO_IN_PACKET;
		if(haudio->rd_ptr >= haudio->buf_size) haudio->rd_ptr = 0;
		
		if(haudio->rd_step == 1)
		{
			if(haudio->rd_ptr > haudio->buf_size/3*2)
			{
				haudio->rd_ptr = 0;
				data = &haudio->in_buffer[haudio->rd_ptr];
			}
		}
		else if(haudio->rd_step == 2)
		{
			if(haudio->rd_ptr < haudio->buf_size/3)
			{
				haudio->rd_ptr = haudio->buf_size>>1;
				data = &haudio->in_buffer[haudio->rd_ptr];
			}
		}
		haudio->rd_step = 0;
		
		d_in++;
		
		if(d_reset) {d_out=d_in=d_reset=0;}
	
		//if((sync_active&SYNC_ADC) == SYNC_ADC) USBAUDIO_Sync_FS();
		sync_tick_adc = sync_tick_sof;
		
		USBD_LL_FlushEP(pdev, AUDIO_IN_EP);
		USBD_LL_Transmit(pdev, AUDIO_IN_EP, data, AUDIO_IN_PACKET);
	}
		
  return USBD_OK;
}

/**
  * @brief  USBD_AUDIO_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */

extern void Mixer_SyncUSB(void);

static uint8_t  USBD_AUDIO_DataOut (USBD_HandleTypeDef *pdev,
                                    uint8_t epnum)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;

  if (epnum == AUDIO_OUT_EP)
  {
		d_out++;
		if(d_reset) {d_out=d_in=d_reset=0;}
    /* Increment the Buffer pointer or roll it back when all buffers are full */

		uint16_t i=0;
		
		uint32_t data = (uint32_t)&haudio->out_buffer[haudio->wr_ptr];
		
		
		/*
		for(i=0;i<haudio->out_packet;i+=4)
		{
			((uint16_t*)(data+i))[0] ^= ((uint16_t*)(data+i))[1];
			((uint16_t*)(data+i))[1] ^= ((uint16_t*)(data+i))[0];
			((uint16_t*)(data+i))[0] ^= ((uint16_t*)(data+i))[1];
		}
		*/
		SCB_CleanInvalidateDCache_by_Addr((uint32_t*)(data - (data%32)), haudio->out_packet + (data%32 > 0 ? 32 : 0));
		
		
		haudio->wr_ptr += haudio->out_packet;
		if(haudio->wr_ptr >= haudio->buf_size) haudio->wr_ptr = 0;
		
		if(haudio->wr_step == 1)
		{
			if(haudio->wr_ptr > haudio->buf_size/3*2)
			{
				haudio->wr_ptr = 0;
			}
		}
		else if(haudio->wr_step == 2)
		{
			if(haudio->wr_ptr < haudio->buf_size/3)
			{
				haudio->wr_ptr = haudio->buf_size>>1;

			}
		}
		data = (uint32_t)&haudio->out_buffer[haudio->wr_ptr];
		haudio->wr_step = 0;
		
		
		if(d_reset) {d_out=d_in=d_reset=0;}

		if((sync_active&SYNC_DAC) == SYNC_DAC) Mixer_SyncUSB();
		sync_tick_dac = sync_tick_sof;
		
    USBD_LL_PrepareReceive(pdev, AUDIO_OUT_EP, (uint8_t*)data, haudio->out_packet);

  }

  return USBD_OK;
}

/**
  * @brief  USBD_AUDIO_EP0_RxReady
  *         handle EP0 Rx Ready event
  * @param  pdev: device instance
  * @retval status
  */

#define DAT_TO_VOL(x) ((((float)((int16_t)((x)[0]+((x)[1]<<8)))-CUR_MIN)/(float)(CUR_MAX-CUR_MIN))*100.0f)
#define VOL_TO_DAT(x) (uint16_t)(((x)*0.01f)*(float)(CUR_MAX-CUR_MIN)+CUR_MIN)

static uint8_t  USBD_AUDIO_EP0_RxReady (USBD_HandleTypeDef *pdev)
{
	USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;

  if (haudio->control.cmd == AUDIO_REQ_SET_CUR)
  { /* In this driver, to simplify code, only SET_CUR request is managed */
    switch (haudio->control.dest ) {
    case AUDIO_REQ_CONTROL:
      if (haudio->control.unit == AUDIO_OUT_FEATURE_ID) {
        switch (haudio->control.cs) {
        case AUDIO_CONTROL_MUTE:
          haudio->mute = haudio->control.data[0];
          ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->MuteCtl(haudio->mute);
          break;
        case AUDIO_CONTROL_VOLUME:
					haudio->vol = DAT_TO_VOL(haudio->control.data);
          //haudio->vol = (uint8_t)((float)((int16_t)((uint16_t)haudio->control.data[0] + ((uint16_t)haudio->control.data[1] << 8))) / 32767.0f * 100.0f);
          ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->VolumeCtl(haudio->vol);
          break;
        }
        haudio->control.cmd = 0;
        haudio->control.len = 0;
      }
      break;
    case AUDIO_REQ_STREAMING:
			haudio->alt_setting = 1;
      haudio->wr_ptr = 0;
      haudio->rd_ptr = 0;
      haudio->control.cmd = 0;
      haudio->control.len = 0;
      haudio->freq = haudio->control.data[0] + ((uint32_t)haudio->control.data[1] << 8) + ((uint32_t)haudio->control.data[2] << 16);
      haudio->out_packet = (uint32_t)(((haudio->freq * 4 * 2) / 1000));
      haudio->buf_size = ((uint32_t)(haudio->out_packet * AUDIO_OUT_PACKET_NUM));

			((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->FreqCtl(haudio->freq);
      
      break;
    }
  }

  return USBD_OK;
}
/**
  * @brief  USBD_AUDIO_EP0_TxReady
  *         handle EP0 TRx Ready event
  * @param  pdev: device instance
  * @retval status
  */


static uint8_t  USBD_AUDIO_EP0_TxReady (USBD_HandleTypeDef *pdev)
{
  /* Only OUT control data are processed */
  return USBD_OK;
}
/**
  * @brief  USBD_AUDIO_SOF
  *         handle SOF event
  * @param  pdev: device instance
  * @retval status
  */


/*

float CCR_AVG;
double I2SN_AVG = 393.2f;
int32_t CCR_DELTA;
uint16_t I2SN = 393;
float KOFF = 0.00005;
*/

static uint8_t  USBD_AUDIO_SOF (USBD_HandleTypeDef *pdev)
{
	static uint8_t sync_old;
	USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
	
  ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->PeriodicTC(0);
	
	
	if(sync_active == 0)
	{
		if(sync_tick_dac+3 > sync_tick_sof) sync_active = SYNC_DAC;
		else if(sync_tick_adc+3 > sync_tick_sof) sync_active = SYNC_ADC;
		//else Mixer_SyncUSB();
	}
	else if(sync_tick_dac+3 < sync_tick_sof && sync_active == SYNC_DAC) 
	{
		if(sync_tick_adc+3 > sync_tick_sof) sync_active = SYNC_ADC;
		else sync_active = 0;
	}
	else if(sync_active == SYNC_ADC) 
	{
		if(sync_tick_dac+3 > sync_tick_sof) sync_active = SYNC_DAC;
		else if(sync_tick_adc+3 < sync_tick_sof) sync_active = 0;
	}
	
	if(sync_old != 0 && sync_active == 0)
	{
		uint32_t * data_dac = (uint32_t*)haudio->out_buffer;
		uint16_t size = haudio->buf_size>>2;
		
		for(uint16_t i=0;i<size;i++)
			*data_dac++ = 0;
	}
	
	sync_old = sync_active;
		
	sync_tick_sof++;
	
	if(sof_req == 1)
	{
		USBD_LL_Transmit(pdev, AUDIO_IN_EP, NULL, AUDIO_IN_PACKET);
		sof_req = 2;
	}
	
	
	
  return USBD_OK;
}


/**
  * @brief  USBD_AUDIO_IsoINIncomplete
  *         handle data ISO IN Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_AUDIO_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  return USBD_OK;
}
/**
  * @brief  USBD_AUDIO_IsoOutIncomplete
  *         handle data ISO OUT Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_AUDIO_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum)
{

  return USBD_OK;
}
/**
  * @brief  AUDIO_Req_GetCurrent
  *         Handles the GET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;

  haudio->control.cmd = AUDIO_REQ_GET_CUR;     /* Set the request value */
  haudio->control.len = req->wLength;          /* Set the request data length */
  haudio->control.unit = HIBYTE(req->wIndex);  /* Set the request target unit */
  haudio->control.cs = HIBYTE(req->wValue);
  haudio->control.dest = req->bmRequest & 0x0f;

  int len = 0;
  uint8_t data[8];
  int v;
  switch (haudio->control.dest ) {
  case AUDIO_REQ_CONTROL:
    if (haudio->control.unit == AUDIO_OUT_FEATURE_ID) {
      switch (haudio->control.cs) {
      case AUDIO_CONTROL_MUTE:
        len = 1;
        data[0] = haudio->mute;
        break;
      case AUDIO_CONTROL_VOLUME:
        //v = (float)haudio->vol / 100.0f * 32767.0f;
				v = VOL_TO_DAT(haudio->vol);
        len = 2;
        data[0] = v & 0xff;
        data[1] = v >> 8;
        break;

      }
      haudio->control.cmd = 0;
      haudio->control.len = 0;
    }
    break;
  case AUDIO_REQ_STREAMING:
    v = haudio->freq;
    len = 3;
    data[0] = v & 0xff;
    data[1] = v >> 8;
    data[2] = v >> 16;

    haudio->control.cmd = 0;
    haudio->control.len = 0;

    break;
  }

//  memset(haudio->control.data, 0, 64);
  /* Send the current mute state */


  USBD_CtlSendData (pdev,
                    data,
                    len);

}


static void AUDIO_REQ_GetMin(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;

  haudio->control.cmd = AUDIO_REQ_GET_MIN;     /* Set the request value */
  haudio->control.len = req->wLength;          /* Set the request data length */
  haudio->control.unit = HIBYTE(req->wIndex);  /* Set the request target unit */
  haudio->control.cs = HIBYTE(req->wValue);
  haudio->control.dest = req->bmRequest & 0x0f;

  int len = 0;
  uint8_t data[8];
  switch (haudio->control.dest ) {
  case AUDIO_REQ_CONTROL:
    if (haudio->control.unit == AUDIO_OUT_FEATURE_ID) {
      switch (haudio->control.cs) {
      case AUDIO_CONTROL_VOLUME:
        len = 2;
        //data[0] = 0xE0;
        //data[1] = 0xDB;
        data[0] = CUR_MIN&0xFF;
        data[1] = (CUR_MIN>>8)&0xFF;
        break;

      }
      haudio->control.cmd = 0;
      haudio->control.len = 0;
    }
    break;
  }

//  memset(haudio->control.data, 0, 64);
  /* Send the current mute state */


  USBD_CtlSendData (pdev,
                    data,
                    len);

}

static void AUDIO_REQ_GetMax(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;

  haudio->control.cmd = AUDIO_REQ_GET_MAX;     /* Set the request value */
  haudio->control.len = req->wLength;          /* Set the request data length */
  haudio->control.unit = HIBYTE(req->wIndex);  /* Set the request target unit */
  haudio->control.cs = HIBYTE(req->wValue);
  haudio->control.dest = req->bmRequest & 0x0f;

  int len = 0;
  uint8_t data[8];
  switch (haudio->control.dest ) {
  case AUDIO_REQ_CONTROL:
    if (haudio->control.unit == AUDIO_OUT_FEATURE_ID) {
      switch (haudio->control.cs) {
      case AUDIO_CONTROL_VOLUME:
        len = 2;
        //data[0] = 0x20;
        //data[1] = 0x24;
        data[0] = CUR_MAX&0xFF;
        data[1] = (CUR_MAX>>8)&0xFF;
        break;

      }
      haudio->control.cmd = 0;
      haudio->control.len = 0;
    }
    break;

  }

//  memset(haudio->control.data, 0, 64);
  /* Send the current mute state */


  USBD_CtlSendData (pdev,
                    data,
                    len);

}

static void AUDIO_REQ_GetRes(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;

  haudio->control.cmd = AUDIO_REQ_GET_RES;     /* Set the request value */
  haudio->control.len = req->wLength;          /* Set the request data length */
  haudio->control.unit = HIBYTE(req->wIndex);  /* Set the request target unit */
  haudio->control.cs = HIBYTE(req->wValue);
  haudio->control.dest = req->bmRequest & 0x0f;

  int len = 0;
  uint8_t data[8];
  switch (haudio->control.dest ) {
  case AUDIO_REQ_CONTROL:
    if (haudio->control.unit == AUDIO_OUT_FEATURE_ID) {
      switch (haudio->control.cs) {
      case AUDIO_CONTROL_VOLUME:
        len = 2;
        //data[0] = 0x23;
        //data[1] = 0x00;
        data[0] = CUR_RES&0xFF;
        data[1] = (CUR_RES>>8)&0xFF;
        break;

      }
      haudio->control.cmd = 0;
      haudio->control.len = 0;
    }
    break;

  }

//  memset(haudio->control.data, 0, 64);
  /* Send the current mute state */


  USBD_CtlSendData (pdev,
                    data,
                    len);

}
/**
  * @brief  AUDIO_Req_SetCurrent
  *         Handles the SET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;

  if (req->wLength)
  {
    /* Prepare the reception of the buffer over EP0 */
    USBD_CtlPrepareRx (pdev,
                       haudio->control.data,
                       req->wLength);

    haudio->control.cmd = AUDIO_REQ_SET_CUR;     /* Set the request value */
    haudio->control.len = req->wLength;          /* Set the request data length */
    haudio->control.unit = HIBYTE(req->wIndex);  /* Set the request target unit */
    haudio->control.cs = HIBYTE(req->wValue);
    haudio->control.dest = req->bmRequest & 0x0f;
  }
}


/**
* @brief  DeviceQualifierDescriptor
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
static uint8_t  *USBD_AUDIO_GetDeviceQualifierDesc (uint16_t *length)
{
  *length = sizeof (USBD_AUDIO_DeviceQualifierDesc);
  return USBD_AUDIO_DeviceQualifierDesc;
}

/**
* @brief  USBD_AUDIO_RegisterInterface
* @param  fops: Audio interface callback
* @retval status
*/
uint8_t  USBD_AUDIO_RegisterInterface  (USBD_HandleTypeDef   *pdev,
                                        USBD_AUDIO_ItfTypeDef *fops)
{
  if (fops != NULL)
  {
    pdev->pUserData = fops;
  }
  return 0;
}
