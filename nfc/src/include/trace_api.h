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

/******************************************************************************
 *
 *  Contains API for BTE Test Tool trace related functions.
 *
 ******************************************************************************/

#ifndef TRACE_API_H
#define TRACE_API_H

#include "bt_types.h"
#include "nfc_hal_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Trace API Function External Declarations */
extern void DispT3TagMessage(NFC_HDR* p_msg, bool is_rx);
extern void DispRWT4Tags(NFC_HDR* p_buf, bool is_rx);
extern void DispCET4Tags(NFC_HDR* p_buf, bool is_rx);
extern void DispRWI93Tag(NFC_HDR* p_buf, bool is_rx,
                         uint8_t command_to_respond);

extern void DispLLCP(NFC_HDR* p_buf, bool is_rx);
extern void DispHcp(uint8_t* p, uint16_t len, bool is_recv, bool is_first_seg);

#ifdef __cplusplus
}
#endif

#endif /* TRACE_API_H */
