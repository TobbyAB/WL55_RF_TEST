/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-14     Rick       the first version
 */
#ifndef RADIO_BOARD_IF_H_
#define RADIO_BOARD_IF_H_

#include "stm32wlxx.h"

typedef enum
{
  RADIO_SWITCH_OFF    = 0,
  RADIO_SWITCH_RX     = 1,
  RADIO_SWITCH_RFO_LP = 2,
  RADIO_SWITCH_RFO_HP = 3,
}BSP_RADIO_Switch_TypeDef;

typedef enum
{
  RADIO_RFO_LP_MAXPOWER = 0,
  RADIO_RFO_HP_MAXPOWER,
} BSP_RADIO_RFOMaxPowerConfig_TypeDef;

typedef enum
{
  RBI_SWITCH_OFF    = RADIO_SWITCH_OFF,
  RBI_SWITCH_RX     = RADIO_SWITCH_RX,
  RBI_SWITCH_RFO_LP = RADIO_SWITCH_RFO_LP,
  RBI_SWITCH_RFO_HP = RADIO_SWITCH_RFO_HP,
} RBI_Switch_TypeDef;

typedef enum
{
  RBI_RFO_LP_MAXPOWER = RADIO_RFO_LP_MAXPOWER,
  RBI_RFO_HP_MAXPOWER = RADIO_RFO_HP_MAXPOWER,
} RBI_RFOMaxPowerConfig_TypeDef;

#define RADIO_CONF_RFO_LP_HP                     0U
#define RADIO_CONF_RFO_LP                        1U
#define RADIO_CONF_RFO_HP                        2U

#define RBI_CONF_RFO_LP_HP                       RADIO_CONF_RFO_LP_HP
#define RBI_CONF_RFO_LP                          RADIO_CONF_RFO_LP
#define RBI_CONF_RFO_HP                          RADIO_CONF_RFO_HP

#define RADIO_CONF_TCXO_NOT_SUPPORTED            0U
#define RADIO_CONF_TCXO_SUPPORTED                1U

#define RADIO_CONF_DCDC_NOT_SUPPORTED            0U
#define RADIO_CONF_DCDC_SUPPORTED                1U

#define RADIO_CONF_RFO_HP_MAX_22_dBm             ((int32_t) 22)
#define RADIO_CONF_RFO_HP_MAX_20_dBm             ((int32_t) 20)
#define RADIO_CONF_RFO_HP_MAX_17_dBm             ((int32_t) 17)
#define RADIO_CONF_RFO_HP_MAX_14_dBm             ((int32_t) 14)
#define RADIO_CONF_RFO_LP_MAX_15_dBm             ((int32_t) 15)
#define RADIO_CONF_RFO_LP_MAX_14_dBm             ((int32_t) 14)
#define RADIO_CONF_RFO_LP_MAX_10_dBm             ((int32_t) 10)

#define RF_SW_CTRL3_PIN                          GPIO_PIN_3
#define RF_SW_CTRL3_GPIO_PORT                    GPIOC
#define RF_SW_CTRL3_GPIO_CLK_ENABLE()            __HAL_RCC_GPIOC_CLK_ENABLE()
#define RF_SW_CTRL3_GPIO_CLK_DISABLE()           __HAL_RCC_GPIOC_CLK_DISABLE()

#define RF_SW_CTRL1_PIN                          GPIO_PIN_4
#define RF_SW_CTRL1_GPIO_PORT                    GPIOC
#define RF_SW_CTRL1_GPIO_CLK_ENABLE()            __HAL_RCC_GPIOC_CLK_ENABLE()
#define RF_SW_RX_GPIO_CLK_DISABLE()              __HAL_RCC_GPIOC_CLK_DISABLE()

#define RF_SW_CTRL2_PIN                          GPIO_PIN_5
#define RF_SW_CTRL2_GPIO_PORT                    GPIOC
#define RF_SW_CTRL2_GPIO_CLK_ENABLE()            __HAL_RCC_GPIOC_CLK_ENABLE()
#define RF_SW_CTRL2_GPIO_CLK_DISABLE()           __HAL_RCC_GPIOC_CLK_DISABLE()

#define RF_TCXO_VCC_PIN                          GPIO_PIN_0
#define RF_TCXO_VCC_GPIO_PORT                    GPIOB
#define RF_TCXO_VCC_CLK_ENABLE()                 __HAL_RCC_GPIOB_CLK_ENABLE()
#define RF_TCXO_VCC_CLK_DISABLE()                __HAL_RCC_GPIOB_CLK_DISABLE()

#endif /* RADIO_BOARD_IF_H_ */
