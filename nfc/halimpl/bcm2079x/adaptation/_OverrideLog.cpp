/******************************************************************************
 *
 *  Copyright (C) 2012 Broadcom Corporation
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

#define LOG_TAG "NfcNciHal"

/******************************************************************************
 *
 *  Override the ALOGD(), ALOGE(), and other logging macros from
 *  /system/core/include/cutils/log.h
 *
 ******************************************************************************/
#include "_OverrideLog.h"
#include <cutils/properties.h>
#include <stdio.h>
#include <string.h>
#include "android_logmsg.h"
#include "config.h"

unsigned char appl_trace_level = BT_TRACE_LEVEL_DEBUG;

/*******************************************************************************
**
** Function:        InitializeGlobalAppLogLevel
**
** Description:     Initialize and get global logging level from
**                  Android property nfc.app_log_level.
**
** Returns:         Global log level:
**                  BT_TRACE_LEVEL_NONE    0        * No trace messages to be
*generated
**                  BT_TRACE_LEVEL_ERROR   1        * Error condition trace
*messages
**                  BT_TRACE_LEVEL_WARNING 2        * Warning condition trace
*messages
**                  BT_TRACE_LEVEL_API     3        * API traces
**                  BT_TRACE_LEVEL_EVENT   4        * Debug messages for events
**                  BT_TRACE_LEVEL_DEBUG   5        * Debug messages (general)
**
*******************************************************************************/
unsigned char InitializeGlobalAppLogLevel() {
  unsigned long num = 0;
  char valueStr[PROPERTY_VALUE_MAX] = {0};

  num = 1;
  if (GetNumValue(NAME_APPL_TRACE_LEVEL, &num, sizeof(num)))
    appl_trace_level = (unsigned char)num;

  int len = property_get("nfc.app_log_level", valueStr, "");
  if (len > 0) {
    // let Android property override default value
    sscanf(valueStr, "%lu", &num);
    appl_trace_level = (unsigned char)num;
  }

  // 0xFF is a special value used by the stack to query the current
  // trace level; it does not change any trace level
  if (appl_trace_level == 0xFF) appl_trace_level = BT_TRACE_LEVEL_DEBUG;
  ALOGD("%s: level=%u", __func__, appl_trace_level);

  if (appl_trace_level < BT_TRACE_LEVEL_DEBUG) {
    // display protocol traces in raw format
    ProtoDispAdapterUseRawOutput(TRUE);
  }
  return appl_trace_level;
}

uint32_t InitializeProtocolLogLevel() {
  uint32_t num = 0;
  char valueStr[PROPERTY_VALUE_MAX] = {0};

  if (GetNumValue(NAME_PROTOCOL_TRACE_LEVEL, &num, sizeof(num)))
    ScrProtocolTraceFlag = num;

  int len = property_get("nfc.enable_protocol_log", valueStr, "");
  if (len > 0) {
    if (strncmp("0", valueStr, 1) == 0) {
      ScrProtocolTraceFlag = 0;
    } else {
      ScrProtocolTraceFlag = ~0;
    }
  }

  return ScrProtocolTraceFlag;
}
