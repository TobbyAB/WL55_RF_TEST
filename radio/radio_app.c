/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-13     Rick       the first version
 */
#include "rtthread.h"
#include "main.h"
#include "radio.h"
#include "radio_app.h"

#define DBG_TAG "RADIO_APP"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

CRC_HandleTypeDef hcrc;
RadioEvents_t RadioEvents;
rt_thread_t subghz_t = RT_NULL;

static const unsigned char BitReverseTable256[] = { 0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50,
        0xD0, 0x30, 0xB0, 0x70, 0xF0, 0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38,
        0xB8, 0x78, 0xF8, 0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74,
        0xF4, 0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 0x02,
        0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2, 0x0A, 0x8A, 0x4A,
        0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA, 0x06, 0x86, 0x46, 0xC6, 0x26,
        0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6, 0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E,
        0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE, 0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11,
        0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1, 0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59,
        0xD9, 0x39, 0xB9, 0x79, 0xF9, 0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35,
        0xB5, 0x75, 0xF5, 0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D,
        0xFD, 0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3, 0x0B,
        0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB, 0x07, 0x87, 0x47,
        0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 0x0F, 0x8F, 0x4F, 0xCF, 0x2F,
        0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF };
static void OnTxDone(void)
{
    LOG_D("OnTxDone\r\n");

    Radio.SetMaxPayloadLength(MODEM_FSK, MAX_APP_BUFFER_SIZE);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.Rx(0);
}
void Payload_Convert(uint8_t *dest_payload, uint8_t *src_payload, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++)
    {
        dest_payload[i] = BitReverseTable256[src_payload[i]];
    }
}
uint32_t Calculate_Send_CRC(uint8_t *payload, uint16_t size)
{
    uint32_t packet_size = size + 1; //add Len,CRC_L,CRC_H
    uint8_t *temp_payload = rt_malloc(packet_size); //add CRC_L,CRC_H
    for (uint16_t i = 0; i < size; i++) //just data
    {
        temp_payload[i + 1] = payload[i];
    }
    temp_payload[0] = size + 1; //add Len
    uint32_t calc_crc = HAL_CRC_Calculate(&hcrc, (uint32_t *) temp_payload, size + 1) ^ 0xffff;
    rt_free(temp_payload);
    return calc_crc;
}
void RF_Send(uint8_t *payload, uint16_t size)
{
//    uint32_t packet_size = size + 3;//add Len,CRC_L,CRC_H
//    uint8_t *dest_payload = rt_malloc(packet_size);//add CRC_L,CRC_H
//    uint32_t calc_crc = Calculate_Send_CRC(payload,size);
//    Payload_Convert(&dest_payload[1],payload,size);
//    dest_payload[0] = BitReverseTable256[size + 1];//add length
//    dest_payload[packet_size - 1] = BitReverseTable256[calc_crc >> 8];
//    dest_payload[packet_size - 2] = BitReverseTable256[calc_crc&0x00ff];
//    LOG_D("Send:size %d,payload is %s\r\n",size,payload);
//    Radio.Send(&dest_payload[1], BitReverseTable256[size+1]);
//    rt_free(dest_payload);

    Radio.Send(payload, size);
}

char hex_data[] = { 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19 };

//char str_data[128];

static void OnRxDone(uint8_t *src_payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{
//    uint32_t packet_size = BitReverseTable256[size];//add CRC_L,CRC_H
//    uint8_t *dest_payload = rt_malloc(packet_size+2);
//    Payload_Convert(&dest_payload[1],src_payload,packet_size+1);//not change buf length
//    uint32_t src_crc = dest_payload[packet_size] | dest_payload[packet_size+1]<<8;
//    dest_payload[0] = packet_size;//remove length,CRC_L,CRC_H
//    uint32_t calc_crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)dest_payload, packet_size) ^ 0xffff;
//    if(calc_crc == src_crc)
//    {
//        LOG_D("rssi %d,cfo %d,recv Size %d,recv payload is %s\r\n",rssi,LoraSnr_FskCfo,packet_size,dest_payload);
//    }
//    else
//    {
//        LOG_E("rssi %d,cfo %d,Calc_CRC is %04X,src_CRC is %04X\r\n",rssi,LoraSnr_FskCfo,calc_crc,src_crc);
//    }
//    rt_free(dest_payload);

    //  uint8_t dest_payload[50] = { 0 };

    uint8_t str_data[100] = { 0 };
    hex_to_str(src_payload, 35, str_data);
    LOG_D("rssi %d,cfo %d,recv Size %d\r\nrecv payload is %s \r\n", rssi, LoraSnr_FskCfo, size, str_data);
    if (rssi < -94)
    {
        //Dis_RED_Rec();
        LOG_D("rssi LOW\r\n");
    }
    else if (rssi >= -94 && rssi < -78)
    {
        //Dis_GREEN_Rec();
        LOG_D("rssi mid\r\n");
    }
    else
    {
        //Dis_Blue_Rec();
        LOG_D("rssi HIGH\r\n");
    }
    memset(str_data, 0, 100);
}

static void OnTxTimeout(void)
{
    LOG_W("OnTxTimeout\r\n");
}
static void OnRxTimeout(void)
{
    LOG_W("OnRxTimeout\r\n");
}
static void OnRxError(void)
{
    LOG_W("OnRxError\r\n");
}
#define TX_RX_FREQUENCE_OFFSET                      0   // 0           TX = RX

#define LORA_TX_OUTPUT_POWER                        22  //14    // dBm

#define LORA_BANDWIDTH                              1         // [0: 125 kHz,
//  1: 250 kHz,
//  2: 500 kHz,
//  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
//  2: 4/6,
//  3: 4/7,
//  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON_DISABLE          false
#define LORA_IQ_INVERSION_ON_DISABLE                false
uint8_t buf[255];
uint8_t g_radio_tx_buf[50] =
        { 0x21, 0x7B, 0x31, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x2c, 0x32, 0x30, 0x30, 0x32, 0x36, 0x37, 0x30,
                0x33, 0x2c, 0x31, 0x30, 0x34, 0x2c, 0x30, 0X36, 0X2C, 0x30, 0x7d, 0x45, 0x39, 0x0d, 0x0a };

uint8_t g_radio_tx_buf1[20] =
        { 0xAA,0x55,0x33,0x44,0x55,0x66};
void SubghzApp_Init(void)
{
    /* CRC initialization */
    hcrc.Instance = CRC;
    hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;
    hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_DISABLE;
    hcrc.Init.GeneratingPolynomial = 32773;
    hcrc.Init.CRCLength = CRC_POLYLENGTH_16B;
    hcrc.Init.InitValue = 0XFFFF;
    hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
    hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
    hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
    if (HAL_CRC_Init(&hcrc) != HAL_OK)
    {
      Error_Handler();
    }
    /* Radio initialization */

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    Radio.Init(&RadioEvents);

    Radio.SetChannel(RF_FREQUENCY);

//    LOG_D("FSK_MODULATION\n\r");
//    LOG_D("FSK_BW=%d Hz\n\r", FSK_BANDWIDTH);
//    LOG_D("FSK_DR=%d bits/s\n\r", FSK_DATARATE);
//
//    Radio.SetTxConfig(MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
//                    FSK_DATARATE, 0,
//                    FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
//                    false ,0, 0, 0, TX_TIMEOUT_VALUE);
//
//    Radio.SetRxConfig(MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
//                    0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
//                    0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, false,
//                    0, 0, false, true);

    Radio.SetPublicNetwork( false);

    Radio.SetTxConfig(MODEM_LORA, LORA_TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON_DISABLE,
    true, 0, 0, LORA_IQ_INVERSION_ON_DISABLE, 5000);

    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON_DISABLE, 0, true, 0, 0, LORA_IQ_INVERSION_ON_DISABLE, true);
    Radio.SetMaxPayloadLength(MODEM_LORA, MAX_APP_BUFFER_SIZE);
    Radio.Rx(0);
}
void subghz_callback(void *parameter)
{
    SubghzApp_Init();
    rt_thread_mdelay(1000);
    memset(buf, 0xF0, 255);

    while (1)
    {
        RF_Send(g_radio_tx_buf, 33);
        rt_thread_mdelay(500);
    }
}
void subghz_init(void)
{
    subghz_t = rt_thread_create("subghz_t", subghz_callback, RT_NULL, 2048, 10, 10);
    if (subghz_t != RT_NULL)
    {
        rt_thread_startup(subghz_t);
    }
}

void hex_to_str(char *hex, int hex_len, char *str)

{
    int i, pos = 0;
    for (i = 0; i < hex_len; i++)
    {
        sprintf(str + pos, "%02x", hex[i]);
        pos += 2;
    }
}
