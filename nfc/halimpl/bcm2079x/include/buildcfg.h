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
#pragma once
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include "data_types.h"

#define BTE_APPL_MAX_USERIAL_DEV_NAME (256)

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t* scru_dump_hex(uint8_t* p, char* p_title, uint32_t len,
                              uint32_t trace_layer, uint32_t trace_type);
void DispNci(uint8_t* p, uint16_t len, bool is_recv);
void ProtoDispAdapterDisplayNciPacket(uint8_t* nciPacket, uint16_t nciPacketLen,
                                      bool is_recv);
#define DISP_NCI ProtoDispAdapterDisplayNciPacket
#define LOGMSG_TAG_NAME "NfcNciHal"

#ifdef __cplusplus
};
#endif
