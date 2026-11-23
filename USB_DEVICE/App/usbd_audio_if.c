/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_audio_if.c
  * @version        : v1.0_Cube
  * @brief          : Generic media access layer.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
 /* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_audio_if.h"

/* USER CODE BEGIN INCLUDE */
#include "i2s.h"
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
  volatile uint32_t dbg_rx=0, dbg_st=0, dbg_pl=0, dbg_ht=0, dbg_tc=0;
  volatile uint32_t dbg_dma_error=0;
  volatile uint32_t dbg_pkt=0, dbg_sz=0;
  static volatile uint32_t s_i2s_cur_freq = 96000U;
/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_AUDIO_IF
  * @{
  */

/** @defgroup USBD_AUDIO_IF_Private_TypesDefinitions USBD_AUDIO_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_AUDIO_IF_Private_Defines USBD_AUDIO_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_AUDIO_IF_Private_Macros USBD_AUDIO_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_AUDIO_IF_Private_Variables USBD_AUDIO_IF_Private_Variables
  * @brief Private variables.
  * @{
  */

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_AUDIO_IF_Exported_Variables USBD_AUDIO_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_AUDIO_IF_Private_FunctionPrototypes USBD_AUDIO_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t AUDIO_Init_FS(uint32_t AudioFreq, uint32_t Volume, uint32_t options);
static int8_t AUDIO_DeInit_FS(uint32_t options);
static int8_t AUDIO_AudioCmd_FS(uint8_t* pbuf, uint32_t size, uint8_t cmd);
static int8_t AUDIO_VolumeCtl_FS(uint8_t vol);
static int8_t AUDIO_MuteCtl_FS(uint8_t cmd);
static int8_t AUDIO_PeriodicTC_FS(uint8_t *pbuf, uint32_t size, uint8_t cmd);
static int8_t AUDIO_GetState_FS(void);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */
static int8_t AudioFs_SwitchSampleRate(uint32_t freq);
/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_AUDIO_ItfTypeDef USBD_AUDIO_fops_FS =
{
  AUDIO_Init_FS,
  AUDIO_DeInit_FS,
  AUDIO_AudioCmd_FS,
  AUDIO_VolumeCtl_FS,
  AUDIO_MuteCtl_FS,
  AUDIO_PeriodicTC_FS,
  AUDIO_GetState_FS,
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the AUDIO media low layer over USB FS IP
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param  options: Reserved for future use
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_Init_FS(uint32_t AudioFreq, uint32_t Volume, uint32_t options)
{
  /* USER CODE BEGIN 0 */
  UNUSED(AudioFreq);
  UNUSED(Volume);
  UNUSED(options);
  return (USBD_OK);
  /* USER CODE END 0 */
}

/**
  * @brief  De-Initializes the AUDIO media low layer
  * @param  options: Reserved for future use
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_DeInit_FS(uint32_t options)
{
  /* USER CODE BEGIN 1 */
  UNUSED(options);
  return (USBD_OK);
  /* USER CODE END 1 */
}

/**
  * @brief  Handles AUDIO command.
  * @param  pbuf: Pointer to buffer of data to be sent
  * @param  size: Number of data to be sent (in bytes)
  * @param  cmd: Command opcode
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_AudioCmd_FS(uint8_t* pbuf, uint32_t size, uint8_t cmd)
{
  /* USER CODE BEGIN 2 */
  HAL_StatusTypeDef status;

  switch(cmd)
  {
  case AUDIO_CMD_START:
  case AUDIO_CMD_PLAY:
    if ((pbuf == NULL) || ((size % 4U) != 0U) ||   /* 4 字节 = 2ch x 16bit 帧 */
        ((size / sizeof(uint16_t)) > UINT16_MAX))
    {
      dbg_dma_error++;
      return (USBD_FAIL);
    }

    status = HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t *)pbuf,
                                  (uint16_t)(size / sizeof(uint16_t)));
    if (status != HAL_OK)
    {
      dbg_dma_error++;
      return (USBD_FAIL);
    }

    if (cmd == AUDIO_CMD_START)
    {
      dbg_st++;
    }
    else
    {
      dbg_pl++;
    }
    dbg_sz = size;
    break;

  case AUDIO_CMD_STOP:
    status = HAL_I2S_DMAStop(&hi2s2);
    if (status != HAL_OK)
    {
      dbg_dma_error++;
      return (USBD_FAIL);
    }
    break;

  default:
    dbg_dma_error++;
    return (USBD_FAIL);
  }

  return (USBD_OK);
  /* USER CODE END 2 */
}

/**
  * @brief  Controls AUDIO Volume.
  * @param  vol: volume level (0..100)
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_VolumeCtl_FS(uint8_t vol)
{
  /* USER CODE BEGIN 3 */
  UNUSED(vol);
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  Controls AUDIO Mute.
  * @param  cmd: command opcode
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_MuteCtl_FS(uint8_t cmd)
{
  /* USER CODE BEGIN 4 */
  UNUSED(cmd);
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  AUDIO_PeriodicT_FS
  * @param  cmd: Command opcode
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_PeriodicTC_FS(uint8_t *pbuf, uint32_t size, uint8_t cmd)
{
  /* USER CODE BEGIN 5 */
  dbg_rx++; dbg_pkt = size;

  /* 采样率自适应：引擎启动后，按 USB 实际包长重配 I2S
     （16bit 版包长：192 字节/包 = 48k，384 字节/包 = 96k） */
  if (dbg_st != 0U)
  {
    if ((size >= 384U) && (s_i2s_cur_freq != 96000U))
    {
      AudioFs_SwitchSampleRate(96000U);
    }
    else if ((size < 384U) && (s_i2s_cur_freq != 48000U))
    {
      AudioFs_SwitchSampleRate(48000U);
    }
  }

  UNUSED(pbuf);
  UNUSED(size);
  UNUSED(cmd);
  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Gets AUDIO State.
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_GetState_FS(void)
{
  /* USER CODE BEGIN 6 */
  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  Manages the DMA full transfer complete event.
  * @retval None
  */
void TransferComplete_CallBack_FS(void)
{
  /* USER CODE BEGIN 7 */
  dbg_tc++;
  USBD_AUDIO_Sync(&hUsbDeviceFS, AUDIO_OFFSET_FULL);
  /* USER CODE END 7 */
}

/**
  * @brief  Manages the DMA Half transfer complete event.
  * @retval None
  */
void HalfTransfer_CallBack_FS(void)
{
  /* USER CODE BEGIN 8 */
  /* 注意：不要把本函数接回 HAL_I2S_TxHalfCpltCallback！Sync(HALF) 会使读指针记账双倍 -> 周期性咔啦。详见文档库《STM32-USB-Audio声卡调试记录》 */
  dbg_ht++;
  USBD_AUDIO_Sync(&hUsbDeviceFS, AUDIO_OFFSET_HALF);
  /* USER CODE END 8 */
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @brief  按目标采样率重配 I2S（76.8MHz 时钟下 48k/96k 均为整数分频）
  *         停 DMA -> DeInit（完整拆底层）-> 状态归位 RESET -> 恢复配置 -> Init -> 催引擎重启
  */
static int8_t AudioFs_SwitchSampleRate(uint32_t freq)
{
  I2S_InitTypeDef saved_init = hi2s2.Init;   /* 备份配置（DeInit 可能改动 Init 结构） */

  if ((freq != 48000U) && (freq != 96000U))
  {
    return (USBD_FAIL);
  }

  HAL_I2S_DMAStop(&hi2s2);
  HAL_I2S_DeInit(&hi2s2);                    /* 完整拆掉底层（GPIO/时钟/DMA） */
  hi2s2.State = HAL_I2S_STATE_RESET;         /* 确保 Init 走完整初始化路径 */

  hi2s2.Init = saved_init;
  hi2s2.Init.AudioFreq = (freq == 96000U) ? I2S_AUDIOFREQ_96K : I2S_AUDIOFREQ_48K;

  if (HAL_I2S_Init(&hi2s2) != HAL_OK)
  {
    return (USBD_FAIL);
  }

  s_i2s_cur_freq = freq;
  /* 催 ST 引擎发下一段 PLAY，把 DMA 重新拉起来 */
  USBD_AUDIO_Sync(&hUsbDeviceFS, AUDIO_OFFSET_FULL);
  return (USBD_OK);
}

 void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s){
	 //HalfTransfer_CallBack_FS(); 
   dbg_ht++;
   UNUSED(hi2s);
 }
 void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s){
   if (hi2s == &hi2s2)
   {
     TransferComplete_CallBack_FS();
   }
  } 
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
