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
#include "android_logmsg.h"
#include <cutils/log.h>
#include "_OverrideLog.h"
#include "buildcfg.h"
#include "nfc_target.h"

extern uint32_t ScrProtocolTraceFlag;
#define MAX_NCI_PACKET_SIZE 259
#define BTE_LOG_BUF_SIZE 1024
#define BTE_LOG_MAX_SIZE (BTE_LOG_BUF_SIZE - 12)
#define MAX_LOGCAT_LINE 4096
#define PRINT(s) __android_log_write(ANDROID_LOG_DEBUG, "BrcmNci", s)
static char log_line[MAX_LOGCAT_LINE];
static const char* sTable = "0123456789abcdef";
static bool sIsUseRaw = FALSE;
static void ToHex(const uint8_t* data, uint16_t len, char* hexString,
                  uint16_t hexStringSize);
static void dumpbin(const char* data, int size, uint32_t trace_layer,
                    uint32_t trace_type);
static inline void word2hex(const char* data, char** hex);
static inline void byte2char(const char* data, char** str);
static inline void byte2hex(const char* data, char** str);

void BTDISP_LOCK_LOG() {}

void BTDISP_UNLOCK_LOG() {}

void BTDISP_INIT_LOCK() {}

void BTDISP_UNINIT_LOCK() {}

void ProtoDispAdapterUseRawOutput(bool isUseRaw) { sIsUseRaw = isUseRaw; }

void ProtoDispAdapterDisplayNciPacket(uint8_t* nciPacket, uint16_t nciPacketLen,
                                      bool is_recv) {
  // Protocol decoder is not available, so decode NCI packet into hex numbers.
  if (!(ScrProtocolTraceFlag & SCR_PROTO_TRACE_NCI)) return;
  char line_buf[(MAX_NCI_PACKET_SIZE * 2) + 1];
  ToHex(nciPacket, nciPacketLen, line_buf, sizeof(line_buf));
  __android_log_write(ANDROID_LOG_DEBUG, (is_recv) ? "BrcmNciR" : "BrcmNciX",
                      line_buf);
}

void ToHex(const uint8_t* data, uint16_t len, char* hexString,
           uint16_t hexStringSize) {
  int i = 0, j = 0;
  for (i = 0, j = 0; i < len && j < hexStringSize - 3; i++) {
    hexString[j++] = sTable[(*data >> 4) & 0xf];
    hexString[j++] = sTable[*data & 0xf];
    data++;
  }
  hexString[j] = '\0';
}

// Protodisp code calls ScrLog() to print decoded texts.
void ScrLog(uint32_t trace_set_mask, const char* fmt_str, ...) {
  static char buffer[BTE_LOG_BUF_SIZE];
  va_list ap;

  va_start(ap, fmt_str);
  vsnprintf(buffer, BTE_LOG_MAX_SIZE, fmt_str, ap);
  va_end(ap);
  __android_log_write(ANDROID_LOG_INFO, "BrcmNci", buffer);
}

uint8_t* scru_dump_hex(uint8_t* p, char* pTitle, uint32_t len, uint32_t layer,
                       uint32_t type) {
  if (pTitle && *pTitle) PRINT(pTitle);
  dumpbin((char*)p, len, layer, type);
  return p;
}

void dumpbin(const char* data, int size, uint32_t trace_layer,
             uint32_t trace_type) {
  char line_buff[256];
  char* line;
  int i, j, addr;
  const int width = 16;
  if (size <= 0) return;
  for (i = 0; i < size / width; i++) {
    line = line_buff;
    // write address:
    addr = i * width;
    word2hex((const char*)&addr, &line);
    *line++ = ':';
    *line++ = ' ';
    // write hex of data
    for (j = 0; j < width; j++) {
      byte2hex(&data[j], &line);
      *line++ = ' ';
    }
    // write char of data
    for (j = 0; j < width; j++) byte2char(data++, &line);
    // wirte the end of line
    *line = 0;
    // output the line
    PRINT(line_buff);
  }
  // last line of left over if any
  int leftover = size % width;
  if (leftover > 0) {
    line = line_buff;
    // write address:
    addr = i * width;
    word2hex((const char*)&addr, &line);
    *line++ = ':';
    *line++ = ' ';
    // write hex of data
    for (j = 0; j < leftover; j++) {
      byte2hex(&data[j], &line);
      *line++ = ' ';
    }
    // write hex padding
    for (; j < width; j++) {
      *line++ = ' ';
      *line++ = ' ';
      *line++ = ' ';
    }
    // write char of data
    for (j = 0; j < leftover; j++) byte2char(data++, &line);
    // write the end of line
    *line = 0;
    // output the line
    PRINT(line_buff);
  }
}

inline void word2hex(const char* data, char** hex) {
  byte2hex(&data[1], hex);
  byte2hex(&data[0], hex);
}

inline void byte2char(const char* data, char** str) {
  **str = *data < ' ' ? '.' : *data > '~' ? '.' : *data;
  ++(*str);
}

inline void byte2hex(const char* data, char** str) {
  **str = sTable[(*data >> 4) & 0xf];
  ++*str;
  **str = sTable[*data & 0xf];
  ++*str;
}

// Decode a few Bluetooth HCI packets into hex numbers.
void DispHciCmd(NFC_HDR* p_buf) {
  uint32_t nBytes = ((NFC_HDR_SIZE + p_buf->offset + p_buf->len) * 2) + 1;
  uint8_t* data = (uint8_t*)p_buf;
  int data_len = NFC_HDR_SIZE + p_buf->offset + p_buf->len;

  if (appl_trace_level < BT_TRACE_LEVEL_DEBUG) return;

  if (nBytes > sizeof(log_line)) return;

  ToHex(data, data_len, log_line, sizeof(log_line));
  __android_log_write(ANDROID_LOG_DEBUG, "BrcmHciX", log_line);
}

// Decode a few Bluetooth HCI packets into hex numbers.
void DispHciEvt(NFC_HDR* p_buf) {
  uint32_t nBytes = ((NFC_HDR_SIZE + p_buf->offset + p_buf->len) * 2) + 1;
  uint8_t* data = (uint8_t*)p_buf;
  int data_len = NFC_HDR_SIZE + p_buf->offset + p_buf->len;

  if (appl_trace_level < BT_TRACE_LEVEL_DEBUG) return;

  if (nBytes > sizeof(log_line)) return;

  ToHex(data, data_len, log_line, sizeof(log_line));
  __android_log_write(ANDROID_LOG_DEBUG, "BrcmHciR", log_line);
}

/*******************************************************************************
**
** Function         DispLLCP
**
** Description      Log LLCP packet as hex-ascii bytes.
**
** Returns          None.
**
*******************************************************************************/
void DispLLCP(NFC_HDR* p_buf, bool is_recv) {
  uint32_t nBytes = ((NFC_HDR_SIZE + p_buf->offset + p_buf->len) * 2) + 1;
  uint8_t* data = (uint8_t*)p_buf;
  int data_len = NFC_HDR_SIZE + p_buf->offset + p_buf->len;

  if (appl_trace_level < BT_TRACE_LEVEL_DEBUG) return;

  if (nBytes > sizeof(log_line)) return;

  ToHex(data, data_len, log_line, sizeof(log_line));
  __android_log_write(ANDROID_LOG_DEBUG, (is_recv) ? "BrcmLlcpR" : "BrcmLlcpX",
                      log_line);
}

/*******************************************************************************
**
** Function         DispHcp
**
** Description      Log raw HCP packet as hex-ascii bytes
**
** Returns          None.
**
*******************************************************************************/
void DispHcp(uint8_t* data, uint16_t len, bool is_recv) {
  uint32_t nBytes = (len * 2) + 1;

  if (appl_trace_level < BT_TRACE_LEVEL_DEBUG) return;

  // Only trace HCP if we're tracing HCI as well
  if (!(ScrProtocolTraceFlag & SCR_PROTO_TRACE_HCI_SUMMARY)) return;

  if (nBytes > sizeof(log_line)) return;

  ToHex(data, len, log_line, sizeof(log_line));
  __android_log_write(ANDROID_LOG_DEBUG, (is_recv) ? "BrcmHcpR" : "BrcmHcpX",
                      log_line);
}

void DispSNEP(uint8_t local_sap, uint8_t remote_sap, NFC_HDR* p_buf,
              bool is_first, bool is_rx) {}
void DispCHO(uint8_t* pMsg, uint32_t MsgLen, bool is_rx) {}
void DispT3TagMessage(NFC_HDR* p_msg, bool is_rx) {}
void DispRWT4Tags(NFC_HDR* p_buf, bool is_rx) {}
void DispCET4Tags(NFC_HDR* p_buf, bool is_rx) {}
void DispRWI93Tag(NFC_HDR* p_buf, bool is_rx, uint8_t command_to_respond) {}
void DispNDEFMsg(uint8_t* pMsg, uint32_t MsgLen, bool is_recv) {}

/*******************************************************************************
**
** Function:        LogMsg
**
** Description:     Print messages from NFC stack.
**
** Returns:         None.
**
*******************************************************************************/
void LogMsg(uint32_t trace_set_mask, const char* fmt_str, ...) {
  static char buffer[BTE_LOG_BUF_SIZE];
  va_list ap;
  uint32_t trace_type =
      trace_set_mask & 0x07;  // lower 3 bits contain trace type
  int android_log_type = ANDROID_LOG_INFO;

  va_start(ap, fmt_str);
  vsnprintf(buffer, BTE_LOG_MAX_SIZE, fmt_str, ap);
  va_end(ap);
  if (trace_type == TRACE_TYPE_ERROR) android_log_type = ANDROID_LOG_ERROR;
  __android_log_write(android_log_type, LOGMSG_TAG_NAME, buffer);
}

void LogMsg_0(uint32_t maskTraceSet, const char* p_str) {
  LogMsg(maskTraceSet, p_str);
}

void LogMsg_1(uint32_t maskTraceSet, const char* fmt_str, uintptr_t p1) {
  LogMsg(maskTraceSet, fmt_str, p1);
}

void LogMsg_2(uint32_t maskTraceSet, const char* fmt_str, uintptr_t p1,
              uintptr_t p2) {
  LogMsg(maskTraceSet, fmt_str, p1, p2);
}

void LogMsg_3(uint32_t maskTraceSet, const char* fmt_str, uintptr_t p1,
              uintptr_t p2, uintptr_t p3) {
  LogMsg(maskTraceSet, fmt_str, p1, p2, p3);
}

void LogMsg_4(uint32_t maskTraceSet, const char* fmt_str, uintptr_t p1,
              uintptr_t p2, uintptr_t p3, uintptr_t p4) {
  LogMsg(maskTraceSet, fmt_str, p1, p2, p3, p4);
}

void LogMsg_5(uint32_t maskTraceSet, const char* fmt_str, uintptr_t p1,
              uintptr_t p2, uintptr_t p3, uintptr_t p4, uintptr_t p5) {
  LogMsg(maskTraceSet, fmt_str, p1, p2, p3, p4, p5);
}

void LogMsg_6(uint32_t maskTraceSet, const char* fmt_str, uintptr_t p1,
              uintptr_t p2, uintptr_t p3, uintptr_t p4, uintptr_t p5,
              uintptr_t p6) {
  LogMsg(maskTraceSet, fmt_str, p1, p2, p3, p4, p5, p6);
}
