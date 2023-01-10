/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-15     Rick       the first version
 */
#ifndef RADIO_RADIO_APP_H_
#define RADIO_RADIO_APP_H_

#define USE_MODEM_LORA  0
#define USE_MODEM_FSK   1
/*
 * Packet Config
 */
#define MAX_APP_BUFFER_SIZE                         255
/*
 * FSK Config
 */
#define FSK_FDEV                                    1660
#define FSK_DATARATE                                4800
#define FSK_BANDWIDTH                               7200
#define FSK_PREAMBLE_LENGTH                         4
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false
#define FSK_AFC_BANDWIDTH                           870
/*
 * Radio Config
 */
#define RF_FREQUENCY                                470000000
#define TX_OUTPUT_POWER                             10
#define RX_TIMEOUT_VALUE                            0
#define TX_TIMEOUT_VALUE                            3000

void SubghzApp_Init(void);

#endif /* RADIO_RADIO_APP_H_ */
