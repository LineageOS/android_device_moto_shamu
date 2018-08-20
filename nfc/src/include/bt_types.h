/******************************************************************************
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#ifndef BT_TYPES_H
#define BT_TYPES_H

#include "data_types.h"
#include "nfc_types.h"

#define SCR_PROTO_TRACE_NCI 0x00004000
#define SCR_PROTO_TRACE_ALL 0x001fffff
#define SCR_PROTO_TRACE_HCI_SUMMARY 0x00000001

#define TRACE_LAYER_NONE 0x00000000
#define TRACE_LAYER_HCI 0x00070000
#define TRACE_LAYER_GKI 0x001a0000
#define TRACE_LAYER_NFC 0x00270000
/*it's overwritten in nfc_types.h*/
#define TRACE_LAYER_NCI 0x00280000
#define TRACE_LAYER_LLCP 0x00290000
#define TRACE_LAYER_NDEF 0x002a0000
#define TRACE_LAYER_RW 0x002b0000
#define TRACE_LAYER_CE 0x002c0000
#define TRACE_LAYER_P2P 0x002d0000
#define TRACE_LAYER_NFA 0x00300000
/*it's overwritten in nfc_types.h*/
#define TRACE_LAYER_HAL 0x00310000
#define TRACE_LAYER_MAX_NUM 0x0032

/* NCI Command, Notification or Data*/
#define BT_EVT_TO_NFC_NCI 0x4000
#define BT_EVT_TO_NFC_NCI_VS 0x4200 /* Vendor specific message */
/* messages between NFC and NCI task */
#define BT_EVT_TO_NFC_MSGS 0x4300

/* start timer */
#define BT_EVT_TO_START_TIMER 0x3c00

/* stop timer */
#define BT_EVT_TO_STOP_TIMER 0x3d00

/* start quick timer */
#define BT_EVT_TO_START_QUICK_TIMER 0x3e00

#define TRACE_ORG_APPL 0x00000500

#define DEV_CLASS_LEN 3
typedef uint8_t DEV_CLASS[DEV_CLASS_LEN]; /* Device class */

#define BD_ADDR_LEN 6                 /* Device address length */
typedef uint8_t BD_ADDR[BD_ADDR_LEN]; /* Device address */
#endif
