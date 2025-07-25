/**
  ******************************************************************************
  * @file    LowPower.c
  * @author  Frederic Pillon
  * @brief   Provides a Low Power interface
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020-2021, STMicroelectronics
  * All rights reserved.
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

#include "Arduino.h"
#include "low_power.h"
#include "stm32yyxx_ll_cortex.h"
#include "stm32yyxx_ll_pwr.h"

#if defined(HAL_PWR_MODULE_ENABLED) && !defined(HAL_PWR_MODULE_ONLY)

#if defined(HAL_UART_MODULE_ENABLED) && !defined(HAL_UART_MODULE_ONLY) \
  && (defined(UART_IT_WUF) || defined(LPUART1_BASE))
  #define UART_WKUP_SUPPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(UART_WKUP_SUPPORT)
/* Save UART handler for callback */
static UART_HandleTypeDef *WakeUpUart = NULL;
#endif
/* Save callback pointer */
static void (*WakeUpUartCb)(void) = NULL;

#if defined(PWR_WAKEUP_SELECT_0) || defined(PWR_WAKEUP1_SOURCE_SELECTION_0)
typedef struct {
  PinName pin;
  uint32_t wkup_pin;
  uint32_t selection;
#if defined(PWR_WAKEUP1_SOURCE_SELECTION_0)
  uint32_t low;
#endif
} WakePinSel;

const WakePinSel WakeupPinSel[] = {
#if defined(PWR_WAKEUP_SELECT_0)
  /* PWR_WAKEUP_SELECT_0 */
  {PA_0,    PWR_WAKEUP_LINE1, PWR_WAKEUP_SELECT_0},
  {PA_4,    PWR_WAKEUP_LINE2, PWR_WAKEUP_SELECT_0},
  {PE_6,    PWR_WAKEUP_LINE3, PWR_WAKEUP_SELECT_0},
  {PA_2,    PWR_WAKEUP_LINE4, PWR_WAKEUP_SELECT_0},
  {PC_5,    PWR_WAKEUP_LINE5, PWR_WAKEUP_SELECT_0},
  {PB_5,    PWR_WAKEUP_LINE6, PWR_WAKEUP_SELECT_0},
  {PB_15,   PWR_WAKEUP_LINE7, PWR_WAKEUP_SELECT_0},
  /* PWR_WAKEUP_SELECT_1 */
  {PB_2,    PWR_WAKEUP_LINE1, PWR_WAKEUP_SELECT_1},
  {PC_13,   PWR_WAKEUP_LINE2, PWR_WAKEUP_SELECT_1},
  {PA_1,    PWR_WAKEUP_LINE3, PWR_WAKEUP_SELECT_1},
  {PB_1,    PWR_WAKEUP_LINE4, PWR_WAKEUP_SELECT_1},
  {PA_3,    PWR_WAKEUP_LINE5, PWR_WAKEUP_SELECT_1},
  {PA_5,    PWR_WAKEUP_LINE6, PWR_WAKEUP_SELECT_1},
  {PA_6,    PWR_WAKEUP_LINE7, PWR_WAKEUP_SELECT_1},
  {PA_7,    PWR_WAKEUP_LINE8, PWR_WAKEUP_SELECT_1},
  /* PWR_WAKEUP_SELECT_2 */
  {PE_4,    PWR_WAKEUP_LINE1, PWR_WAKEUP_SELECT_2},
  {PE_5,    PWR_WAKEUP_LINE2, PWR_WAKEUP_SELECT_2},
  {PB_6,    PWR_WAKEUP_LINE3, PWR_WAKEUP_SELECT_2},
  {PB_7,    PWR_WAKEUP_LINE4, PWR_WAKEUP_SELECT_2},
  {PB_8,    PWR_WAKEUP_LINE5, PWR_WAKEUP_SELECT_2},
  {PE_7,    PWR_WAKEUP_LINE6, PWR_WAKEUP_SELECT_2},
  {PE_8,    PWR_WAKEUP_LINE7, PWR_WAKEUP_SELECT_2},
  {PB_10,   PWR_WAKEUP_LINE8, PWR_WAKEUP_SELECT_2},
  {NC,   0, 0}
#else /* PWR_WAKEUP_SELECT_0 */
  /* PWR_WAKEUPx_SOURCE_SELECTION_0 */
  {PA_0,    PWR_WAKEUP_PIN1, PWR_WAKEUP1_SOURCE_SELECTION_0, PWR_WAKEUP1_POLARITY_LOW},
#if defined(PWR_WAKEUP2_SOURCE_SELECTION_0)
  {PA_4,    PWR_WAKEUP_PIN2, PWR_WAKEUP2_SOURCE_SELECTION_0, PWR_WAKEUP2_POLARITY_LOW},
#endif
#if defined(PWR_WAKEUP3_SOURCE_SELECTION_0)
  {PE_6,    PWR_WAKEUP_PIN3, PWR_WAKEUP3_SOURCE_SELECTION_0, PWR_WAKEUP3_POLARITY_LOW},
#endif
  {PA_2,    PWR_WAKEUP_PIN4, PWR_WAKEUP4_SOURCE_SELECTION_0, PWR_WAKEUP4_POLARITY_LOW},
#if defined(PWR_WAKEUP5_SOURCE_SELECTION_0)
  {PC_5,    PWR_WAKEUP_PIN5, PWR_WAKEUP5_SOURCE_SELECTION_0, PWR_WAKEUP5_POLARITY_LOW},
#endif
#if defined(PWR_WAKEUP8_SOURCE_SELECTION_0)
  /* STM32U5xx */
  {PB_5,    PWR_WAKEUP_PIN6, PWR_WAKEUP6_SOURCE_SELECTION_0, PWR_WAKEUP6_POLARITY_LOW},
  {PB_15,   PWR_WAKEUP_PIN7, PWR_WAKEUP7_SOURCE_SELECTION_0, PWR_WAKEUP7_POLARITY_LOW},
  {PF_2,    PWR_WAKEUP_PIN8, PWR_WAKEUP8_SOURCE_SELECTION_0, PWR_WAKEUP8_POLARITY_LOW},
#else
  /* STM32WBAxx */
  {PA_12,   PWR_WAKEUP_PIN6, PWR_WAKEUP6_SOURCE_SELECTION_0, PWR_WAKEUP6_POLARITY_LOW},
  {PB_14,   PWR_WAKEUP_PIN7, PWR_WAKEUP7_SOURCE_SELECTION_0, PWR_WAKEUP7_POLARITY_LOW},
#endif
  /* PWR_WAKEUPx_SOURCE_SELECTION_1 */
#if defined(PWR_WAKEUP1_SOURCE_SELECTION_1)
  {PB_2,    PWR_WAKEUP_PIN1, PWR_WAKEUP1_SOURCE_SELECTION_1, PWR_WAKEUP1_POLARITY_LOW},
#endif
#if defined(PWR_WAKEUP2_SOURCE_SELECTION_1)
  {PC_13,   PWR_WAKEUP_PIN2, PWR_WAKEUP2_SOURCE_SELECTION_1, PWR_WAKEUP2_POLARITY_LOW},
#endif
  {PA_1,    PWR_WAKEUP_PIN3, PWR_WAKEUP3_SOURCE_SELECTION_1, PWR_WAKEUP3_POLARITY_LOW},
#if defined(PWR_WAKEUP4_SOURCE_SELECTION_1)
  {PB_1,    PWR_WAKEUP_PIN4, PWR_WAKEUP4_SOURCE_SELECTION_1, PWR_WAKEUP4_POLARITY_LOW},
#endif
#if defined(PWR_WAKEUP5_SOURCE_SELECTION_1)
  {PA_3,    PWR_WAKEUP_PIN5, PWR_WAKEUP5_SOURCE_SELECTION_1, PWR_WAKEUP5_POLARITY_LOW},
#endif
  {PA_5,    PWR_WAKEUP_PIN6, PWR_WAKEUP6_SOURCE_SELECTION_1, PWR_WAKEUP6_POLARITY_LOW},
  {PA_6,    PWR_WAKEUP_PIN7, PWR_WAKEUP7_SOURCE_SELECTION_1, PWR_WAKEUP7_POLARITY_LOW},
  {PA_7,    PWR_WAKEUP_PIN8, PWR_WAKEUP8_SOURCE_SELECTION_1, PWR_WAKEUP8_POLARITY_LOW},
  /* PWR_WAKEUPx_SOURCE_SELECTION_2 */
#if defined(PWR_WAKEUP1_SOURCE_SELECTION_2)
  {PE_4,    PWR_WAKEUP_PIN1, PWR_WAKEUP1_SOURCE_SELECTION_2, PWR_WAKEUP1_POLARITY_LOW},
#endif
#if defined(PWR_WAKEUP2_SOURCE_SELECTION_2)
  {PE_5,    PWR_WAKEUP_PIN2, PWR_WAKEUP2_SOURCE_SELECTION_2, PWR_WAKEUP2_POLARITY_LOW},
#endif
#if defined(PWR_WAKEUP3_SOURCE_SELECTION_2)
  {PB_6,    PWR_WAKEUP_PIN3, PWR_WAKEUP3_SOURCE_SELECTION_2, PWR_WAKEUP3_POLARITY_LOW},
#endif
#if defined(PWR_WAKEUP4_SOURCE_SELECTION_2)
  {PB_7,    PWR_WAKEUP_PIN4, PWR_WAKEUP4_SOURCE_SELECTION_2, PWR_WAKEUP4_POLARITY_LOW}, /* Only for STM32U5 */
#endif
#if defined(PWR_WAKEUP5_SOURCE_SELECTION_2)
  {PB_7,    PWR_WAKEUP_PIN5, PWR_WAKEUP5_SOURCE_SELECTION_2, PWR_WAKEUP5_POLARITY_LOW}, /* Only for STM32WBA */
  {PB_8,    PWR_WAKEUP_PIN5, PWR_WAKEUP5_SOURCE_SELECTION_2, PWR_WAKEUP5_POLARITY_LOW}, /* Only for STM32U5 */
#endif
#if defined(PWR_WAKEUP6_SOURCE_SELECTION_2)
  {PE_7,    PWR_WAKEUP_PIN6, PWR_WAKEUP6_SOURCE_SELECTION_2, PWR_WAKEUP6_POLARITY_LOW},
#endif
#if defined(PWR_WAKEUP7_SOURCE_SELECTION_2)
  {PE_8,    PWR_WAKEUP_PIN7, PWR_WAKEUP7_SOURCE_SELECTION_2, PWR_WAKEUP7_POLARITY_LOW},
#endif
#if defined(PWR_WAKEUP8_SOURCE_SELECTION_2)
  {PB_9,    PWR_WAKEUP_PIN8, PWR_WAKEUP8_SOURCE_SELECTION_2, PWR_WAKEUP8_POLARITY_LOW}, /* Only for STM32WBA */
  {PB_10,   PWR_WAKEUP_PIN8, PWR_WAKEUP8_SOURCE_SELECTION_2, PWR_WAKEUP8_POLARITY_LOW}, /* Only for STM32U5 */
#endif
  {NC,   0, 0, 0}
#endif /* PWR_WAKEUP_SELECT_0 */
};
#endif /* PWR_WAKEUP_SELECT_0 || PWR_WAKEUP1_SOURCE_SELECTION_0 */


#if defined(PWR_FLAG_WUF)
#define PWR_FLAG_WU PWR_FLAG_WUF
#elif defined(PWR_WAKEUP_ALL_FLAG)
#define PWR_FLAG_WU PWR_WAKEUP_ALL_FLAG
#elif defined(PWR_WU_FLAG_ALL)
#define PWR_FLAG_WU PWR_WU_FLAG_ALL
#endif
#if defined(PWR_FLAG_SBF)
#define PWR_FLAG_SB PWR_FLAG_SBF
#endif

/**
  * @brief  Initialize low power mode
  * @param  None
  * @retval None
  */
void LowPower_init()
{
#if defined(__HAL_RCC_PWR_CLK_ENABLE)
  /* Enable Power Clock */
  __HAL_RCC_PWR_CLK_ENABLE();
#endif

#if defined(PWR_CR_DBP) || defined(PWR_CR1_DBP) || defined(PWR_DBPR_DBP)
  /* Allow access to Backup domain */
  HAL_PWR_EnableBkUpAccess();
#endif

#ifdef __HAL_RCC_WAKEUPSTOP_CLK_CONFIG
  /* Ensure that HSI is wake-up system clock */
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI);
#endif
#ifdef PWR_FLAG_SB
  /* Check if the system was resumed from StandBy mode */
  if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET) {
    /* Clear Standby flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
  }
#endif
  /* Clear all related wakeup flags */
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
}

/**
  * @brief  Configure a pin as wakeup source if compatible.
  * @param  pin: pin to configure
  * @param  mode: pin mode (edge or state). The configuration have to be compatible.
  * @retval None
  */
void LowPower_EnableWakeUpPin(uint32_t pin, uint32_t mode)
{
  PinName p = digitalPinToPinName(pin);
#if defined(PWR_WAKEUP_SELECT_0) || defined(PWR_WAKEUP1_SOURCE_SELECTION_0)
  /* Two cases:
   * 1- STM32U5 and STM32WBA provides a compatible way to configure
   *    the wakeup pin using HAL_PWR_EnableWakeUpPin but only selection 0
   *    pins are managed.
   * 2- STM32U3 uses HAL_PWR_EnableWakeUpLine.
   */
  /* First find index of the pin if valid */
  WakePinSel *WakeupPinSel_idx = (WakePinSel *) WakeupPinSel;

  /* Read through PinMapAnalogSwitch array */
  while (WakeupPinSel_idx->pin != NC) {
    /* Check whether pin is or is associated to dualpad Analog Input */
    if (WakeupPinSel_idx->pin == p) {
      break;
    }
    WakeupPinSel_idx ++;
  }
  if (WakeupPinSel_idx->pin != NC) {
#if defined(PWR_WAKEUP1_SOURCE_SELECTION_0)
    uint32_t wkup_pin = WakeupPinSel_idx->wkup_pin | WakeupPinSel_idx->selection;
    if (mode != RISING) {
      wkup_pin |= PWR_WAKEUP1_POLARITY_LOW;
    }
    if (IS_PWR_WAKEUP_PIN(wkup_pin)) {
      HAL_PWR_EnableWakeUpPin(wkup_pin);
    }
#else
    if (IS_PWR_WAKEUP_LINE(WakeupPinSel_idx->wkup_pin)) {
      HAL_PWR_EnableWakeUpLine(WakeupPinSel_idx->wkup_pin, WakeupPinSel_idx->selection, (mode != RISING) ? PWR_WAKEUP_POLARITY_LOW : PWR_WAKEUP_POLARITY_HIGH);
      /* Enable the NVIC for Wake-up pin */
      HAL_NVIC_SetPriority(PWR_IRQn, 0, 0x00);
      HAL_NVIC_EnableIRQ(PWR_IRQn);
    }
#endif /* PWR_WAKEUP1_SOURCE_SELECTION_0 */
  }
#elif defined(PWR_WAKEUP_PA0)
  uint32_t wkup_pin = 0;
  if (p != NC) {
#if defined(PWR_WAKEUP_PB0)
    if (p == PB_0) {
      wkup_pin = PWR_WAKEUP_PB0;
    }
#endif
#ifdef PWR_WAKEUP_PB1
    if (p == PB_1) {
      wkup_pin = PWR_WAKEUP_PB1;
    }
#endif
#ifdef PWR_WAKEUP_PB2
    if (p == PB_2) {
      wkup_pin = PWR_WAKEUP_PB0;
    }
#endif
#ifdef PWR_WAKEUP_PB3
    if (p == PB_3) {
      wkup_pin = PWR_WAKEUP_PB3;
    }
#endif
#ifdef PWR_WAKEUP_PB4
    if (p == PB_4) {
      wkup_pin = PWR_WAKEUP_PB4;
    }
#endif
#ifdef PWR_WAKEUP_PB5
    if (p == PB_5) {
      wkup_pin = PWR_WAKEUP_PB5;
    }
#endif
#ifdef PWR_WAKEUP_PB6
    if (p == PB_6) {
      wkup_pin = PWR_WAKEUP_PB6;
    }
#endif
#ifdef PWR_WAKEUP_PB7
    if (p == PB_7) {
      wkup_pin = PWR_WAKEUP_PB7;
    }
#endif
#ifdef PWR_WAKEUP_PA8
    if (p == PA_8) {
      wkup_pin = PWR_WAKEUP_PA8;
    }
#endif
#ifdef PWR_WAKEUP_PA9
    if (p == PA_9) {
      wkup_pin = PWR_WAKEUP_PA9;
    }
#endif
#ifdef PWR_WAKEUP_PA10
    if (p == PA_10) {
      wkup_pin = PWR_WAKEUP_PA10;
    }
#endif
#ifdef PWR_WAKEUP_PA11
    if (p == PA_11) {
      wkup_pin = PWR_WAKEUP_PA11;
    }
#endif
#ifdef PWR_WAKEUP_PA0
    if (p == PA_0) {
      wkup_pin = PWR_WAKEUP_PA0;
    }
#endif
#ifdef PWR_WAKEUP_PA1
    if (p == PA_1) {
      wkup_pin = PWR_WAKEUP_PA1;
    }
#endif
#ifdef PWR_WAKEUP_PA2
    if (p == PA_2) {
      wkup_pin = PWR_WAKEUP_PA2;
    }
#endif
#ifdef PWR_WAKEUP_PA3
    if (p == PA_3) {
      wkup_pin = PWR_WAKEUP_PA3;
    }
#endif
#ifdef PWR_WAKEUP_PA4
    if (p == PA_4) {
      wkup_pin = PWR_WAKEUP_PA4;
    }
#endif
#ifdef PWR_WAKEUP_PA5
    if (p == PA_5) {
      wkup_pin = PWR_WAKEUP_PA5;
    }
#endif
#ifdef PWR_WAKEUP_PA6
    if (p == PA_6) {
      wkup_pin = PWR_WAKEUP_PA6;
    }
#endif
#ifdef PWR_WAKEUP_PA7
    if (p == PA_7) {
      wkup_pin = PWR_WAKEUP_PA7;
    }
#endif
#ifdef PWR_WAKEUP_PB8
    if (p == PB_8) {
      wkup_pin = PWR_WAKEUP_PB8;
    }
#endif
#ifdef PWR_WAKEUP_PB9
    if (p == PB_9) {
      wkup_pin = PWR_WAKEUP_PB9;
    }
#endif
#ifdef PWR_WAKEUP_PB10
    if (p == PB_10) {
      wkup_pin = PWR_WAKEUP_PB10;
    }
#endif
#ifdef PWR_WAKEUP_PB11
    if (p == PB_11) {
      wkup_pin = PWR_WAKEUP_PB11;
    }
#endif
#ifdef PWR_WAKEUP_PA12
    if (p == PA_12) {
      wkup_pin = PWR_WAKEUP_PA12;
    }
#endif
#ifdef PWR_WAKEUP_PA13
    if (p == PA_13) {
      wkup_pin = PWR_WAKEUP_PA13;
    }
#endif
#ifdef PWR_WAKEUP_PA14
    if (p == PA_14) {
      wkup_pin = PWR_WAKEUP_PA14;
    }
#endif
#ifdef PWR_WAKEUP_PA15
    if (p == PA_15) {
      wkup_pin = PWR_WAKEUP_PA15;
    }
#endif
    if (IS_PWR_WAKEUP_PIN(wkup_pin)) {
      HAL_PWR_EnableWakeUpPin(wkup_pin, ((mode == RISING) || (mode == HIGH)) ? PWR_WUP_FALLEDG : PWR_WUP_RISIEDG);
    }
  }
#else
#if !defined(PWR_WAKEUP_PIN1_HIGH)
  UNUSED(mode);
#endif
  uint32_t wkup_pin = 0;
  if (p != NC) {
#if defined(PWR_WAKEUP_PIN1)
    if (p == SYS_WKUP1) {
      wkup_pin = PWR_WAKEUP_PIN1;
#ifdef PWR_WAKEUP_PIN1_HIGH
      if (mode != RISING) {
        wkup_pin = PWR_WAKEUP_PIN1_LOW;
      }
#endif
    }
#endif /* PWR_WAKEUP_PIN1 */
#ifdef PWR_WAKEUP_PIN2
    if (p == SYS_WKUP2) {
      wkup_pin = PWR_WAKEUP_PIN2;
#ifdef PWR_WAKEUP_PIN2_HIGH
      if (mode != RISING) {
        wkup_pin = PWR_WAKEUP_PIN2_LOW;
      }
#endif
    }
#endif /* PWR_WAKEUP_PIN2 */
#ifdef PWR_WAKEUP_PIN3
    if (p == SYS_WKUP3) {
      wkup_pin = PWR_WAKEUP_PIN3;
#ifdef PWR_WAKEUP_PIN3_HIGH
      if (mode != RISING) {
        wkup_pin = PWR_WAKEUP_PIN3_LOW;
      }
#endif
    }
#endif /* PWR_WAKEUP_PIN3 */
#ifdef PWR_WAKEUP_PIN4
    if (p == SYS_WKUP4) {
      wkup_pin = PWR_WAKEUP_PIN4;
#ifdef PWR_WAKEUP_PIN4_HIGH
      if (mode != RISING) {
        wkup_pin = PWR_WAKEUP_PIN4_LOW;
      }
#endif
    }
#endif /* PWR_WAKEUP_PIN4 */
#ifdef PWR_WAKEUP_PIN5
    if (p == SYS_WKUP5) {
      wkup_pin = PWR_WAKEUP_PIN5;
#ifdef PWR_WAKEUP_PIN5_HIGH
      if (mode != RISING) {
        wkup_pin = PWR_WAKEUP_PIN5_LOW;
      }
#endif
    }
#endif /* PWR_WAKEUP_PIN5 */
#ifdef PWR_WAKEUP_PIN6
    if (p == SYS_WKUP6) {
      wkup_pin = PWR_WAKEUP_PIN6;
#ifdef PWR_WAKEUP_PIN6_HIGH
      if (mode != RISING) {
        wkup_pin = PWR_WAKEUP_PIN6_LOW;
      }
#endif
    }
#endif /* PWR_WAKEUP_PIN6 */
#ifdef PWR_WAKEUP_PIN7
    if (p == SYS_WKUP7) {
      wkup_pin = PWR_WAKEUP_PIN7;
#ifdef PWR_WAKEUP_PIN7_HIGH
      if (mode != RISING) {
        wkup_pin = PWR_WAKEUP_PIN7_LOW;
      }
#endif
    }
#endif /* PWR_WAKEUP_PIN7 */
#ifdef PWR_WAKEUP_PIN8
    if (p == SYS_WKUP8) {
      wkup_pin = PWR_WAKEUP_PIN8;
#ifdef PWR_WAKEUP_PIN8_HIGH
      if (mode != RISING) {
        wkup_pin = PWR_WAKEUP_PIN8_LOW;
      }
#endif
    }
#endif /* PWR_WAKEUP_PIN8 */
    if (IS_PWR_WAKEUP_PIN(wkup_pin)) {
      HAL_PWR_EnableWakeUpPin(wkup_pin);
    }
  }
#endif /* !PWR_WAKEUP_SELECT_0 && !PWR_WAKEUP1_SOURCE_SELECTION_0 */
}

#if defined(PWR_CSR_REGLPF)
/**
  * @brief  For STM32L0 and STM32L1, running in LowPower Sleep requires
  *         to slow down frequency to MSI range1.
  * @retval None
  */
void SystemClock_Decrease(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
    Error_Handler();
  }
}

#elif defined(STM32L4xx) || defined(STM32L5xx) || defined(STM32WBxx) || defined(STM32WLxx)
/**
  * @brief  For STM32L4, STM32L5, STM32WB and STM32WL
  *         running in LowPower Sleep requires to slow down frequency to 2MHz max.
  * @retval None
  */
void SystemClock_Decrease(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {};

  /** Configure the main internal regulator output voltage
  */

#if (defined(STM32WBxx) && defined(PWR_CR1_VOS)) || !defined(STM32WBxx)
#if defined(STM32L4xx) || defined(STM32WBxx)
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
#elif defined(STM32L5xx) || defined(STM32WLxx)
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2) != HAL_OK)
#endif
  {
    Error_Handler();
  }
#endif

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU and buses clocks
  */
#if defined(STM32WBxx)
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4 | RCC_CLOCKTYPE_HCLK2
                                | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;
#elif defined(STM32WLxx)
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3 | RCC_CLOCKTYPE_HCLK
                                | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1
                                | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;
#elif defined(STM32L4xx)
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
#endif
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
    Error_Handler();
  }
}

#elif defined(STM32G0xx) || defined(STM32G4xx) || defined(STM32U0xx)
/**
  * @brief  For STM32G0 and STM32G4
  *         running in LowPower Sleep requires to slow down frequency to 2MHz max.
  * @retval None
  */
void SystemClock_Decrease(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
#if defined(STM32G0xx)
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
#endif
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
#if defined(STM32G4xx)
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
#elif defined(STM32G0xx) || defined(STM32U0xx)
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1;
#endif
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV8;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
    Error_Handler();
  }
}
#endif

/**
  * @brief  Enable the sleep mode.
  * @param  None
  * @retval None
  */
void LowPower_sleep(uint32_t regulator)
{

#if defined(PWR_LOWPOWERREGULATOR_ON) && \
    (defined(PWR_CSR_REGLPF) || defined(PWR_SR2_REGLPF))
  // When LowPower regulator sleep mode is used, it is necessary to decrease CPU Frequency
  if (regulator == PWR_LOWPOWERREGULATOR_ON) {
    SystemClock_Decrease();
  }
#endif

  /*
   * Suspend Tick increment to prevent wakeup by Systick interrupt.
   * Otherwise the Systick interrupt will wake up the device within
   * 1ms (HAL time base)
   */
  HAL_SuspendTick();

  /* Enter Sleep Mode , wake up is done once User push-button is pressed */
#if defined(PWR_SLEEPENTRY_WFI)
  HAL_PWR_EnterSLEEPMode(regulator, PWR_SLEEPENTRY_WFI);
#else
  (void)regulator;
  HAL_PWR_EnterSLEEPMode();
#endif
#if defined(PWR_LOWPOWERREGULATOR_ON) && \
    (defined(PWR_CSR_REGLPF) || defined(PWR_SR2_REGLPF))
  // In case of LowPower Regulator used for sleep, restore Main regulator on exit
  if (regulator == PWR_LOWPOWERREGULATOR_ON) {
#if defined(__HAL_RCC_PWR_CLK_ENABLE)
    __HAL_RCC_PWR_CLK_ENABLE();
#endif
    HAL_PWREx_DisableLowPowerRunMode();

    // Restore systemClock which has been decreased by SystemClock_Decrease()
    SystemClock_Config();
  }
#endif

  /* Resume Tick interrupt if disabled prior to SLEEP mode entry */
  HAL_ResumeTick();

  if (WakeUpUartCb != NULL) {
    WakeUpUartCb();
  }
}

/**
  * @brief  Enable the stop mode.
  * @param  obj : pointer to serial_t structure
  * @retval None
  */
void LowPower_stop(serial_t *obj)
{
  __disable_irq();

#if defined(UART_WKUP_SUPPORT)
  if (WakeUpUart != NULL) {
    HAL_UARTEx_EnableStopMode(WakeUpUart);
  }
#endif
#if defined(STM32WB0x)
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_DEEPSTOPF);
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
  PWR_DEEPSTOPTypeDef configDEEPSTOP;

  configDEEPSTOP.deepStopMode = PWR_DEEPSTOP_WITH_SLOW_CLOCK_ON;
  /* Enter the Deepstop mode */
  HAL_PWR_ConfigDEEPSTOP(&configDEEPSTOP);
  HAL_PWR_EnterDEEPSTOPMode();
#else
#if defined(PWR_CR_ULP)
  /* Enable Ultra low power mode */
  HAL_PWREx_EnableUltraLowPower();
#endif
#if defined(PWR_CR1_ULPMEN) || defined(PWR_CR3_ULPMEN) || \
    (defined(PWR_CR3_ENULP) && defined(STM32U0xx))
  /* Enable Ultra low power mode */
  HAL_PWREx_EnableUltraLowPowerMode();
#endif
#if defined(PWR_CR_FWU)
  /* Enable the fast wake up from Ultra low power mode */
  HAL_PWREx_EnableFastWakeUp();
#endif
#ifdef __HAL_RCC_WAKEUPSTOP_CLK_CONFIG
  /* Select MSI or HSI as system clock source after Wake Up from Stop mode */
#ifdef RCC_STOP_WAKEUPCLOCK_MSI
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
#else
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI);
#endif
#endif
#if defined(STM32WBxx)
  /* Set low-power mode of CPU2 */
  /* Note: Typically, action performed by CPU2 on a dual core application.
           Since this example is single core, perform it by CPU1. */
  /* Note: On STM32WB, both CPU1 and CPU2 must be in low-power mode
           to set the entire System in low-power mode, corresponding to
           the deepest low-power mode possible.
           For example, CPU1 in Stop2 mode and CPU2 in Shutdown mode
           will make system enter in Stop2 mode. */
#if defined(LL_PWR_MODE_STOP2)
  LL_C2_PWR_SetPowerMode(LL_PWR_MODE_STOP2);
#else
  LL_C2_PWR_SetPowerMode(LL_PWR_MODE_SHUTDOWN);
#endif
#endif
  /* Enter Stop mode */
#if defined(UART_WKUP_SUPPORT) && (defined(PWR_CPUCR_RETDS_CD) || \
    defined(LL_PWR_MODE_STOP2))
  if ((WakeUpUart == NULL)
      || (WakeUpUart->Instance == (USART_TypeDef *)LPUART1_BASE)
#ifdef LPUART2_BASE
      || (WakeUpUart->Instance == (USART_TypeDef *)LPUART2_BASE)
#endif
#ifdef LPUART3_BASE
      || (WakeUpUart->Instance == (USART_TypeDef *)LPUART3_BASE)
#endif
     ) {
#if defined(PWR_CR1_RRSTP)
    // STM32L4+ must keep SRAM3 content when entering STOP2 lowpower mode
    HAL_PWREx_EnableSRAM3ContentRetention();
#endif /* PWR_CR1_RRSTP */
#if defined(STM32H7xx)
#if defined(PWR_LOWPOWERREGULATOR_ON)
    HAL_PWREx_EnterSTOP2Mode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
#else
    HAL_PWREx_EnterSTOP2Mode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);
#endif
#elif defined(STM32U3xx)
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERMODE_STOP2, PWR_STOPENTRY_WFI);
#else
    HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
#endif
  } else
#endif
  {
#if defined(PWR_LOWPOWERREGULATOR_ON)
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
#else
    HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);
#endif
  }
#endif /* STM32WB0x */
  /* Exit Stop mode reset clocks */
  SystemClock_ConfigFromStop();
#if defined(UART_WKUP_SUPPORT)
  if (WakeUpUart != NULL) {
    /* In case of WakeUp from UART, reset its clock source to HSI */
    uart_config_lowpower(obj);
    HAL_UARTEx_DisableStopMode(WakeUpUart);
  }
#else
  UNUSED(obj);
#endif
  __enable_irq();

  HAL_Delay(10);

  if (WakeUpUartCb != NULL) {
    WakeUpUartCb();
  }
}


/**
  * @brief  Enable the standby mode. The board reset when leaves this mode.
  * @param  None
  * @retval None
  */
void LowPower_standby()
{
#if !defined(STM32WB0x)
  __disable_irq();

  /* Clear wakeup flags */
#if defined(PWR_FLAG_WU)
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
#elif defined(PWR_CPUCR_CSSF)
  __HAL_PWR_CLEAR_FLAG(PWR_CPUCR_CSSF);
#elif defined(PWR_MPUCR_CSSF)
  __HAL_PWR_CLEAR_FLAG(PWR_MPUCR_CSSF);
#endif

#if defined(PWR_CR_ULP)
  /* Enable Ultra low power mode */
  HAL_PWREx_EnableUltraLowPower();
#endif
#if defined(PWR_CR_FWU)
  /* Enable the fast wake up from Ultra low power mode */
  HAL_PWREx_EnableFastWakeUp();
#endif
#if defined(STM32WBxx)
  /* Set low-power mode of CPU2 */
  /* Note: Typically, action performed by CPU2 on a dual core application.
           Since this example is single core, perform it by CPU1. */
  /* Note: On STM32WB, both CPU1 and CPU2 must be in low-power mode
           to set the entire System in low-power mode, corresponding to
           the deepest low-power mode possible.
           For example, CPU1 in Stop2 mode and CPU2 in Shutdown mode
           will make system enter in Stop2 mode. */
  LL_C2_PWR_SetPowerMode(LL_PWR_MODE_STANDBY);
#endif
  HAL_PWR_EnterSTANDBYMode();
#endif /* !STM32WB0x */
}

/**
  * @brief  Enable the shutdown mode.The board reset when leaves this mode.
  *         If shutdown mode not available, use standby mode instead.
  * @param  boolean true if RTC is configured, in that case LSE is required
  * @retval None
  */
void LowPower_shutdown(bool isRTC)
{
  __disable_irq();

  /* Clear wakeup flags */
#if defined(PWR_FLAG_WU)
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
#elif defined(PWR_CPUCR_CSSF)
  __HAL_PWR_CLEAR_FLAG(PWR_CPUCR_CSSF);
#elif defined(PWR_MPUCR_CSSF)
  __HAL_PWR_CLEAR_FLAG(PWR_MPUCR_CSSF);
#endif
#if !defined(STM32WB0x)
#if defined(STM32WBxx)
  /* Set low-power mode of CPU2 */
  /* Note: Typically, action performed by CPU2 on a dual core application.
           Since this example is single core, perform it by CPU1. */
  /* Note: On STM32WB, both CPU1 and CPU2 must be in low-power mode
           to set the entire System in low-power mode, corresponding to
           the deepest low-power mode possible.
           For example, CPU1 in Stop2 mode and CPU2 in Shutdown mode
           will make system enter in Stop2 mode. */
  LL_C2_PWR_SetPowerMode(LL_PWR_MODE_SHUTDOWN);
#endif
#if defined(LL_PWR_SHUTDOWN_MODE) || defined(LL_PWR_MODE_SHUTDOWN)
  /* LSE must be on to use shutdown mode within RTC else fallback to standby */
  if ((!isRTC) || (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == SET)) {
#if defined(STM32U0xx)
    HAL_PWR_EnterSHUTDOWNMode();
#else
    HAL_PWREx_EnterSHUTDOWNMode();
#endif
  } else
#else
  UNUSED(isRTC);
#endif
  {
    LowPower_standby();
  }
#else
  (void)isRTC; // Avoid unused variable warning
  PWR_SHUTDOWNTypeDef ConfigSHUTDOWN;
  HAL_PWR_ConfigSHUTDOWN(&ConfigSHUTDOWN);
  LL_PWR_DisableDEEPSTOP2();
  LL_PWR_SetPowerMode(LL_PWR_MODE_SHUTDOWN);
  LL_LPM_EnableDeepSleep();
  __WFI();
#endif /* !STM32WB0x */
}

/**
  * @brief  Configure the UART as a wakeup source. A callback can be called when
  *         the chip leaves the low power mode. See board datasheet to check
  *         with which low power mode the UART is compatible.
  * Warning This function will change UART clock source to HSI
  * @param  serial: pointer to serial
  * @param  FuncPtr: pointer to callback
  * @retval None
  */
void LowPower_EnableWakeUpUart(serial_t *serial, void (*FuncPtr)(void))
{
#if defined(UART_WKUP_SUPPORT)
#ifdef IS_UART_WAKEUP_SELECTION
  UART_WakeUpTypeDef WakeUpSelection;
  if (serial == NULL) {
    return;
  }
  /* Save Uart handler and Serial object */
  WakeUpUart = &(serial->handle);

  /* make sure that no UART transfer is on-going */
  while (__HAL_UART_GET_FLAG(WakeUpUart, USART_ISR_BUSY) == SET);
  /* make sure that UART is ready to receive
   * (test carried out again later in HAL_UARTEx_StopModeWakeUpSourceConfig) */
  while (__HAL_UART_GET_FLAG(WakeUpUart, USART_ISR_REACK) == RESET);

  /* set the wake-up event:
   * specify wake-up on RXNE flag
   */
  WakeUpSelection.WakeUpEvent = UART_WAKEUP_ON_READDATA_NONEMPTY;
  HAL_UARTEx_StopModeWakeUpSourceConfig(WakeUpUart, WakeUpSelection);
#endif
#if defined(UART_IT_WUF)
  /* Enable the UART Wake UP from STOPx mode Interrupt */
  __HAL_UART_ENABLE_IT(WakeUpUart, UART_IT_WUF);
#endif
#else
  UNUSED(serial);
#endif
  /* Save callback */
  WakeUpUartCb = FuncPtr;
}

/**
  * @brief  Configures system clock and system IP clocks after wake-up from STOP
  * @note   Weaked function which can be redefined by user at the sketch level.
  *         By default, call 'SystemClock_Config()'.
  * @param  None
  * @retval None
  */
WEAK void SystemClock_ConfigFromStop(void)
{
  configIPClock();
  SystemClock_Config();
}

#ifdef __cplusplus
}
#endif

#endif /* HAL_PWR_MODULE_ENABLED  && !HAL_PWR_MODULE_ONLY */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
