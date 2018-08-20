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
#ifndef BT_TRACE_H
#define BT_TRACE_H

#ifdef __cplusplus
extern "C" {
#endif

/* External declaration for appl_trace_level here to avoid to add the
 * declaration in all the files using APPL_TRACExxx macros */
extern uint8_t appl_trace_level;

/* Prototype for message logging function. */
extern void LogMsg(uint32_t trace_set_mask, const char* fmt_str, ...);
extern void LogMsg_0(uint32_t trace_set_mask, const char* p_str);
extern void LogMsg_1(uint32_t trace_set_mask, const char* fmt_str,
                     uintptr_t p1);
extern void LogMsg_2(uint32_t trace_set_mask, const char* fmt_str, uintptr_t p1,
                     uintptr_t p2);
extern void LogMsg_3(uint32_t trace_set_mask, const char* fmt_str, uintptr_t p1,
                     uintptr_t p2, uintptr_t p3);
extern void LogMsg_4(uint32_t trace_set_mask, const char* fmt_str, uintptr_t p1,
                     uintptr_t p2, uintptr_t p3, uintptr_t p4);
extern void LogMsg_5(uint32_t trace_set_mask, const char* fmt_str, uintptr_t p1,
                     uintptr_t p2, uintptr_t p3, uintptr_t p4, uintptr_t p5);
extern void LogMsg_6(uint32_t trace_set_mask, const char* fmt_str, uintptr_t p1,
                     uintptr_t p2, uintptr_t p3, uintptr_t p4, uintptr_t p5,
                     uintptr_t p6);

#ifdef __cplusplus
}
#endif

/******************************************************************************
**
** Trace configurable parameters
**
******************************************************************************/

/* Enables or disables verbose trace information. */
#ifndef BT_TRACE_VERBOSE
#define BT_TRACE_VERBOSE FALSE
#endif

/* Enables or disables protocol trace information. */
#ifndef BT_TRACE_PROTOCOL
#define BT_TRACE_PROTOCOL TRUE /* Android requires TRUE */
#endif

/******************************************************************************
**
** Trace Levels
**
** The following values may be used for different levels:
**      BT_TRACE_LEVEL_NONE    0        * No trace messages to be generated
**      BT_TRACE_LEVEL_ERROR   1        * Error condition trace messages
**      BT_TRACE_LEVEL_WARNING 2        * Warning condition trace messages
**      BT_TRACE_LEVEL_API     3        * API traces
**      BT_TRACE_LEVEL_EVENT   4        * Debug messages for events
**      BT_TRACE_LEVEL_DEBUG   5        * Debug messages (general)
******************************************************************************/

/* Core Stack default trace levels */
#ifndef HCI_INITIAL_TRACE_LEVEL
#define HCI_INITIAL_TRACE_LEVEL BT_TRACE_LEVEL_DEBUG
#endif

#ifndef LLCP_INITIAL_TRACE_LEVEL
#define LLCP_INITIAL_TRACE_LEVEL BT_TRACE_LEVEL_DEBUG
#endif

#ifndef APPL_INITIAL_TRACE_LEVEL
#define APPL_INITIAL_TRACE_LEVEL BT_TRACE_LEVEL_DEBUG
#endif

#ifndef NFC_INITIAL_TRACE_LEVEL
#define NFC_INITIAL_TRACE_LEVEL BT_TRACE_LEVEL_DEBUG
#endif

#ifndef SMP_INITIAL_TRACE_LEVEL
#define SMP_INITIAL_TRACE_LEVEL BT_TRACE_LEVEL_DEBUG
#endif

#define BT_TRACE_0(l, t, m) \
  LogMsg_0((TRACE_CTRL_GENERAL | (l) | TRACE_ORG_STACK | (t)), (m))
#define BT_TRACE_1(l, t, m, p1)                                   \
  LogMsg_1(TRACE_CTRL_GENERAL | (l) | TRACE_ORG_STACK | (t), (m), \
           (uintptr_t)(p1))
#define BT_TRACE_2(l, t, m, p1, p2)                               \
  LogMsg_2(TRACE_CTRL_GENERAL | (l) | TRACE_ORG_STACK | (t), (m), \
           (uintptr_t)(p1), (uintptr_t)(p2))
#define BT_TRACE_3(l, t, m, p1, p2, p3)                           \
  LogMsg_3(TRACE_CTRL_GENERAL | (l) | TRACE_ORG_STACK | (t), (m), \
           (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3))
#define BT_TRACE_4(l, t, m, p1, p2, p3, p4)                       \
  LogMsg_4(TRACE_CTRL_GENERAL | (l) | TRACE_ORG_STACK | (t), (m), \
           (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3), (uintptr_t)(p4))
#define BT_TRACE_5(l, t, m, p1, p2, p3, p4, p5)                                \
  LogMsg_5(TRACE_CTRL_GENERAL | (l) | TRACE_ORG_STACK | (t), (m),              \
           (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3), (uintptr_t)(p4), \
           (uintptr_t)(p5))
#define BT_TRACE_6(l, t, m, p1, p2, p3, p4, p5, p6)                            \
  LogMsg_6(TRACE_CTRL_GENERAL | (l) | TRACE_ORG_STACK | (t), (m),              \
           (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3), (uintptr_t)(p4), \
           (uintptr_t)(p5), (uintptr_t)(p6))

#define BT_ERROR_TRACE_0(l, m) \
  LogMsg_0(TRACE_CTRL_GENERAL | (l) | TRACE_ORG_STACK | TRACE_TYPE_ERROR, (m))
#define BT_ERROR_TRACE_1(l, m, p1)                                             \
  LogMsg_1(TRACE_CTRL_GENERAL | (l) | TRACE_ORG_STACK | TRACE_TYPE_ERROR, (m), \
           (uintptr_t)(p1))
#define BT_ERROR_TRACE_2(l, m, p1, p2)                                         \
  LogMsg_2(TRACE_CTRL_GENERAL | (l) | TRACE_ORG_STACK | TRACE_TYPE_ERROR, (m), \
           (uintptr_t)(p1), (uintptr_t)(p2))
#define BT_ERROR_TRACE_3(l, m, p1, p2, p3)                                     \
  LogMsg_3(TRACE_CTRL_GENERAL | (l) | TRACE_ORG_STACK | TRACE_TYPE_ERROR, (m), \
           (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3))

/* Define tracing for the LLCP unit
*/
#define LLCP_TRACE_ERROR0(m)                             \
  {                                                      \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_ERROR)     \
      BT_TRACE_0(TRACE_LAYER_LLCP, TRACE_TYPE_ERROR, m); \
  }
#define LLCP_TRACE_ERROR1(m, p1)                             \
  {                                                          \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_ERROR)         \
      BT_TRACE_1(TRACE_LAYER_LLCP, TRACE_TYPE_ERROR, m, p1); \
  }
#define LLCP_TRACE_ERROR2(m, p1, p2)                             \
  {                                                              \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_ERROR)             \
      BT_TRACE_2(TRACE_LAYER_LLCP, TRACE_TYPE_ERROR, m, p1, p2); \
  }
#define LLCP_TRACE_ERROR3(m, p1, p2, p3)                             \
  {                                                                  \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                 \
      BT_TRACE_3(TRACE_LAYER_LLCP, TRACE_TYPE_ERROR, m, p1, p2, p3); \
  }
#define LLCP_TRACE_ERROR4(m, p1, p2, p3, p4)                             \
  {                                                                      \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                     \
      BT_TRACE_4(TRACE_LAYER_LLCP, TRACE_TYPE_ERROR, m, p1, p2, p3, p4); \
  }
#define LLCP_TRACE_ERROR5(m, p1, p2, p3, p4, p5)                             \
  {                                                                          \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                         \
      BT_TRACE_5(TRACE_LAYER_LLCP, TRACE_TYPE_ERROR, m, p1, p2, p3, p4, p5); \
  }
#define LLCP_TRACE_ERROR6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                         \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                        \
      BT_TRACE_6(TRACE_LAYER_LLCP, TRACE_TYPE_ERROR, m, p1, p2, p3, p4, p5, \
                 p6);                                                       \
  }

#define LLCP_TRACE_WARNING0(m)                             \
  {                                                        \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_WARNING)     \
      BT_TRACE_0(TRACE_LAYER_LLCP, TRACE_TYPE_WARNING, m); \
  }
#define LLCP_TRACE_WARNING1(m, p1)                             \
  {                                                            \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_WARNING)         \
      BT_TRACE_1(TRACE_LAYER_LLCP, TRACE_TYPE_WARNING, m, p1); \
  }
#define LLCP_TRACE_WARNING2(m, p1, p2)                             \
  {                                                                \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_WARNING)             \
      BT_TRACE_2(TRACE_LAYER_LLCP, TRACE_TYPE_WARNING, m, p1, p2); \
  }
#define LLCP_TRACE_WARNING3(m, p1, p2, p3)                             \
  {                                                                    \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                 \
      BT_TRACE_3(TRACE_LAYER_LLCP, TRACE_TYPE_WARNING, m, p1, p2, p3); \
  }
#define LLCP_TRACE_WARNING4(m, p1, p2, p3, p4)                             \
  {                                                                        \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                     \
      BT_TRACE_4(TRACE_LAYER_LLCP, TRACE_TYPE_WARNING, m, p1, p2, p3, p4); \
  }
#define LLCP_TRACE_WARNING5(m, p1, p2, p3, p4, p5)                             \
  {                                                                            \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                         \
      BT_TRACE_5(TRACE_LAYER_LLCP, TRACE_TYPE_WARNING, m, p1, p2, p3, p4, p5); \
  }
#define LLCP_TRACE_WARNING6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                           \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                        \
      BT_TRACE_6(TRACE_LAYER_LLCP, TRACE_TYPE_WARNING, m, p1, p2, p3, p4, p5, \
                 p6);                                                         \
  }

#define LLCP_TRACE_API0(m)                             \
  {                                                    \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_API)     \
      BT_TRACE_0(TRACE_LAYER_LLCP, TRACE_TYPE_API, m); \
  }
#define LLCP_TRACE_API1(m, p1)                             \
  {                                                        \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_API)         \
      BT_TRACE_1(TRACE_LAYER_LLCP, TRACE_TYPE_API, m, p1); \
  }
#define LLCP_TRACE_API2(m, p1, p2)                             \
  {                                                            \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_API)             \
      BT_TRACE_2(TRACE_LAYER_LLCP, TRACE_TYPE_API, m, p1, p2); \
  }
#define LLCP_TRACE_API3(m, p1, p2, p3)                             \
  {                                                                \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_API)                 \
      BT_TRACE_3(TRACE_LAYER_LLCP, TRACE_TYPE_API, m, p1, p2, p3); \
  }
#define LLCP_TRACE_API4(m, p1, p2, p3, p4)                             \
  {                                                                    \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_API)                     \
      BT_TRACE_4(TRACE_LAYER_LLCP, TRACE_TYPE_API, m, p1, p2, p3, p4); \
  }
#define LLCP_TRACE_API5(m, p1, p2, p3, p4, p5)                             \
  {                                                                        \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_API)                         \
      BT_TRACE_5(TRACE_LAYER_LLCP, TRACE_TYPE_API, m, p1, p2, p3, p4, p5); \
  }
#define LLCP_TRACE_API6(m, p1, p2, p3, p4, p5, p6)                             \
  {                                                                            \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_API)                             \
      BT_TRACE_6(TRACE_LAYER_LLCP, TRACE_TYPE_API, m, p1, p2, p3, p4, p5, p6); \
  }

#define LLCP_TRACE_EVENT0(m)                             \
  {                                                      \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_EVENT)     \
      BT_TRACE_0(TRACE_LAYER_LLCP, TRACE_TYPE_EVENT, m); \
  }
#define LLCP_TRACE_EVENT1(m, p1)                             \
  {                                                          \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_EVENT)         \
      BT_TRACE_1(TRACE_LAYER_LLCP, TRACE_TYPE_EVENT, m, p1); \
  }
#define LLCP_TRACE_EVENT2(m, p1, p2)                             \
  {                                                              \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_EVENT)             \
      BT_TRACE_2(TRACE_LAYER_LLCP, TRACE_TYPE_EVENT, m, p1, p2); \
  }
#define LLCP_TRACE_EVENT3(m, p1, p2, p3)                             \
  {                                                                  \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                 \
      BT_TRACE_3(TRACE_LAYER_LLCP, TRACE_TYPE_EVENT, m, p1, p2, p3); \
  }
#define LLCP_TRACE_EVENT4(m, p1, p2, p3, p4)                             \
  {                                                                      \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                     \
      BT_TRACE_4(TRACE_LAYER_LLCP, TRACE_TYPE_EVENT, m, p1, p2, p3, p4); \
  }
#define LLCP_TRACE_EVENT5(m, p1, p2, p3, p4, p5)                             \
  {                                                                          \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                         \
      BT_TRACE_5(TRACE_LAYER_LLCP, TRACE_TYPE_EVENT, m, p1, p2, p3, p4, p5); \
  }
#define LLCP_TRACE_EVENT6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                         \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                        \
      BT_TRACE_6(TRACE_LAYER_LLCP, TRACE_TYPE_EVENT, m, p1, p2, p3, p4, p5, \
                 p6);                                                       \
  }

#define LLCP_TRACE_DEBUG0(m)                             \
  {                                                      \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)     \
      BT_TRACE_0(TRACE_LAYER_LLCP, TRACE_TYPE_DEBUG, m); \
  }
#define LLCP_TRACE_DEBUG1(m, p1)                             \
  {                                                          \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)         \
      BT_TRACE_1(TRACE_LAYER_LLCP, TRACE_TYPE_DEBUG, m, p1); \
  }
#define LLCP_TRACE_DEBUG2(m, p1, p2)                             \
  {                                                              \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)             \
      BT_TRACE_2(TRACE_LAYER_LLCP, TRACE_TYPE_DEBUG, m, p1, p2); \
  }
#define LLCP_TRACE_DEBUG3(m, p1, p2, p3)                             \
  {                                                                  \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                 \
      BT_TRACE_3(TRACE_LAYER_LLCP, TRACE_TYPE_DEBUG, m, p1, p2, p3); \
  }
#define LLCP_TRACE_DEBUG4(m, p1, p2, p3, p4)                             \
  {                                                                      \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                     \
      BT_TRACE_4(TRACE_LAYER_LLCP, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4); \
  }
#define LLCP_TRACE_DEBUG5(m, p1, p2, p3, p4, p5)                             \
  {                                                                          \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                         \
      BT_TRACE_5(TRACE_LAYER_LLCP, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4, p5); \
  }
#define LLCP_TRACE_DEBUG6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                         \
    if (llcp_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                        \
      BT_TRACE_6(TRACE_LAYER_LLCP, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4, p5, \
                 p6);                                                       \
  }

/* Define tracing for the NFC unit
*/
#define NFC_TRACE_ERROR0(m)                             \
  {                                                     \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_ERROR)     \
      BT_TRACE_0(TRACE_LAYER_NFC, TRACE_TYPE_ERROR, m); \
  }
#define NFC_TRACE_ERROR1(m, p1)                             \
  {                                                         \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_ERROR)         \
      BT_TRACE_1(TRACE_LAYER_NFC, TRACE_TYPE_ERROR, m, p1); \
  }
#define NFC_TRACE_ERROR2(m, p1, p2)                             \
  {                                                             \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_ERROR)             \
      BT_TRACE_2(TRACE_LAYER_NFC, TRACE_TYPE_ERROR, m, p1, p2); \
  }
#define NFC_TRACE_ERROR3(m, p1, p2, p3)                             \
  {                                                                 \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                 \
      BT_TRACE_3(TRACE_LAYER_NFC, TRACE_TYPE_ERROR, m, p1, p2, p3); \
  }
#define NFC_TRACE_ERROR4(m, p1, p2, p3, p4)                             \
  {                                                                     \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                     \
      BT_TRACE_4(TRACE_LAYER_NFC, TRACE_TYPE_ERROR, m, p1, p2, p3, p4); \
  }
#define NFC_TRACE_ERROR5(m, p1, p2, p3, p4, p5)                             \
  {                                                                         \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                         \
      BT_TRACE_5(TRACE_LAYER_NFC, TRACE_TYPE_ERROR, m, p1, p2, p3, p4, p5); \
  }
#define NFC_TRACE_ERROR6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                        \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                        \
      BT_TRACE_6(TRACE_LAYER_NFC, TRACE_TYPE_ERROR, m, p1, p2, p3, p4, p5, \
                 p6);                                                      \
  }

#define NFC_TRACE_WARNING0(m)                             \
  {                                                       \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_WARNING)     \
      BT_TRACE_0(TRACE_LAYER_NFC, TRACE_TYPE_WARNING, m); \
  }
#define NFC_TRACE_WARNING1(m, p1)                             \
  {                                                           \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_WARNING)         \
      BT_TRACE_1(TRACE_LAYER_NFC, TRACE_TYPE_WARNING, m, p1); \
  }
#define NFC_TRACE_WARNING2(m, p1, p2)                             \
  {                                                               \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_WARNING)             \
      BT_TRACE_2(TRACE_LAYER_NFC, TRACE_TYPE_WARNING, m, p1, p2); \
  }
#define NFC_TRACE_WARNING3(m, p1, p2, p3)                             \
  {                                                                   \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                 \
      BT_TRACE_3(TRACE_LAYER_NFC, TRACE_TYPE_WARNING, m, p1, p2, p3); \
  }
#define NFC_TRACE_WARNING4(m, p1, p2, p3, p4)                             \
  {                                                                       \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                     \
      BT_TRACE_4(TRACE_LAYER_NFC, TRACE_TYPE_WARNING, m, p1, p2, p3, p4); \
  }
#define NFC_TRACE_WARNING5(m, p1, p2, p3, p4, p5)                             \
  {                                                                           \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                         \
      BT_TRACE_5(TRACE_LAYER_NFC, TRACE_TYPE_WARNING, m, p1, p2, p3, p4, p5); \
  }
#define NFC_TRACE_WARNING6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                          \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                        \
      BT_TRACE_6(TRACE_LAYER_NFC, TRACE_TYPE_WARNING, m, p1, p2, p3, p4, p5, \
                 p6);                                                        \
  }

#define NFC_TRACE_API0(m)                             \
  {                                                   \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_API)     \
      BT_TRACE_0(TRACE_LAYER_NFC, TRACE_TYPE_API, m); \
  }
#define NFC_TRACE_API1(m, p1)                             \
  {                                                       \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_API)         \
      BT_TRACE_1(TRACE_LAYER_NFC, TRACE_TYPE_API, m, p1); \
  }
#define NFC_TRACE_API2(m, p1, p2)                             \
  {                                                           \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_API)             \
      BT_TRACE_2(TRACE_LAYER_NFC, TRACE_TYPE_API, m, p1, p2); \
  }
#define NFC_TRACE_API3(m, p1, p2, p3)                             \
  {                                                               \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_API)                 \
      BT_TRACE_3(TRACE_LAYER_NFC, TRACE_TYPE_API, m, p1, p2, p3); \
  }
#define NFC_TRACE_API4(m, p1, p2, p3, p4)                             \
  {                                                                   \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_API)                     \
      BT_TRACE_4(TRACE_LAYER_NFC, TRACE_TYPE_API, m, p1, p2, p3, p4); \
  }
#define NFC_TRACE_API5(m, p1, p2, p3, p4, p5)                             \
  {                                                                       \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_API)                         \
      BT_TRACE_5(TRACE_LAYER_NFC, TRACE_TYPE_API, m, p1, p2, p3, p4, p5); \
  }
#define NFC_TRACE_API6(m, p1, p2, p3, p4, p5, p6)                             \
  {                                                                           \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_API)                             \
      BT_TRACE_6(TRACE_LAYER_NFC, TRACE_TYPE_API, m, p1, p2, p3, p4, p5, p6); \
  }

#define NFC_TRACE_EVENT0(m)                             \
  {                                                     \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_EVENT)     \
      BT_TRACE_0(TRACE_LAYER_NFC, TRACE_TYPE_EVENT, m); \
  }
#define NFC_TRACE_EVENT1(m, p1)                             \
  {                                                         \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_EVENT)         \
      BT_TRACE_1(TRACE_LAYER_NFC, TRACE_TYPE_EVENT, m, p1); \
  }
#define NFC_TRACE_EVENT2(m, p1, p2)                             \
  {                                                             \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_EVENT)             \
      BT_TRACE_2(TRACE_LAYER_NFC, TRACE_TYPE_EVENT, m, p1, p2); \
  }
#define NFC_TRACE_EVENT3(m, p1, p2, p3)                             \
  {                                                                 \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                 \
      BT_TRACE_3(TRACE_LAYER_NFC, TRACE_TYPE_EVENT, m, p1, p2, p3); \
  }
#define NFC_TRACE_EVENT4(m, p1, p2, p3, p4)                             \
  {                                                                     \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                     \
      BT_TRACE_4(TRACE_LAYER_NFC, TRACE_TYPE_EVENT, m, p1, p2, p3, p4); \
  }
#define NFC_TRACE_EVENT5(m, p1, p2, p3, p4, p5)                             \
  {                                                                         \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                         \
      BT_TRACE_5(TRACE_LAYER_NFC, TRACE_TYPE_EVENT, m, p1, p2, p3, p4, p5); \
  }
#define NFC_TRACE_EVENT6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                        \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                        \
      BT_TRACE_6(TRACE_LAYER_NFC, TRACE_TYPE_EVENT, m, p1, p2, p3, p4, p5, \
                 p6);                                                      \
  }

#define NFC_TRACE_DEBUG0(m)                             \
  {                                                     \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)     \
      BT_TRACE_0(TRACE_LAYER_NFC, TRACE_TYPE_DEBUG, m); \
  }
#define NFC_TRACE_DEBUG1(m, p1)                             \
  {                                                         \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)         \
      BT_TRACE_1(TRACE_LAYER_NFC, TRACE_TYPE_DEBUG, m, p1); \
  }
#define NFC_TRACE_DEBUG2(m, p1, p2)                             \
  {                                                             \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)             \
      BT_TRACE_2(TRACE_LAYER_NFC, TRACE_TYPE_DEBUG, m, p1, p2); \
  }
#define NFC_TRACE_DEBUG3(m, p1, p2, p3)                             \
  {                                                                 \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                 \
      BT_TRACE_3(TRACE_LAYER_NFC, TRACE_TYPE_DEBUG, m, p1, p2, p3); \
  }
#define NFC_TRACE_DEBUG4(m, p1, p2, p3, p4)                             \
  {                                                                     \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                     \
      BT_TRACE_4(TRACE_LAYER_NFC, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4); \
  }
#define NFC_TRACE_DEBUG5(m, p1, p2, p3, p4, p5)                             \
  {                                                                         \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                         \
      BT_TRACE_5(TRACE_LAYER_NFC, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4, p5); \
  }
#define NFC_TRACE_DEBUG6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                        \
    if (nfc_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                        \
      BT_TRACE_6(TRACE_LAYER_NFC, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4, p5, \
                 p6);                                                      \
  }

#define NCI_TRACE_ERROR0(m)                             \
  {                                                     \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_ERROR)    \
      BT_TRACE_0(TRACE_LAYER_NCI, TRACE_TYPE_ERROR, m); \
  }
#define NCI_TRACE_ERROR1(m, p1)                             \
  {                                                         \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_ERROR)        \
      BT_TRACE_1(TRACE_LAYER_NCI, TRACE_TYPE_ERROR, m, p1); \
  }
#define NCI_TRACE_ERROR2(m, p1, p2)                             \
  {                                                             \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_ERROR)            \
      BT_TRACE_2(TRACE_LAYER_NCI, TRACE_TYPE_ERROR, m, p1, p2); \
  }
#define NCI_TRACE_ERROR3(m, p1, p2, p3)                             \
  {                                                                 \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                \
      BT_TRACE_3(TRACE_LAYER_NCI, TRACE_TYPE_ERROR, m, p1, p2, p3); \
  }
#define NCI_TRACE_ERROR4(m, p1, p2, p3, p4)                             \
  {                                                                     \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                    \
      BT_TRACE_4(TRACE_LAYER_NCI, TRACE_TYPE_ERROR, m, p1, p2, p3, p4); \
  }
#define NCI_TRACE_ERROR5(m, p1, p2, p3, p4, p5)                             \
  {                                                                         \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                        \
      BT_TRACE_5(TRACE_LAYER_NCI, TRACE_TYPE_ERROR, m, p1, p2, p3, p4, p5); \
  }
#define NCI_TRACE_ERROR6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                        \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                       \
      BT_TRACE_6(TRACE_LAYER_NCI, TRACE_TYPE_ERROR, m, p1, p2, p3, p4, p5, \
                 p6);                                                      \
  }

#define NCI_TRACE_WARNING0(m)                             \
  {                                                       \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_WARNING)    \
      BT_TRACE_0(TRACE_LAYER_NCI, TRACE_TYPE_WARNING, m); \
  }
#define NCI_TRACE_WARNING1(m, p1)                             \
  {                                                           \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_WARNING)        \
      BT_TRACE_1(TRACE_LAYER_NCI, TRACE_TYPE_WARNING, m, p1); \
  }
#define NCI_TRACE_WARNING2(m, p1, p2)                             \
  {                                                               \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_WARNING)            \
      BT_TRACE_2(TRACE_LAYER_NCI, TRACE_TYPE_WARNING, m, p1, p2); \
  }
#define NCI_TRACE_WARNING3(m, p1, p2, p3)                             \
  {                                                                   \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                \
      BT_TRACE_3(TRACE_LAYER_NCI, TRACE_TYPE_WARNING, m, p1, p2, p3); \
  }
#define NCI_TRACE_WARNING4(m, p1, p2, p3, p4)                             \
  {                                                                       \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                    \
      BT_TRACE_4(TRACE_LAYER_NCI, TRACE_TYPE_WARNING, m, p1, p2, p3, p4); \
  }
#define NCI_TRACE_WARNING5(m, p1, p2, p3, p4, p5)                             \
  {                                                                           \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                        \
      BT_TRACE_5(TRACE_LAYER_NCI, TRACE_TYPE_WARNING, m, p1, p2, p3, p4, p5); \
  }
#define NCI_TRACE_WARNING6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                          \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                       \
      BT_TRACE_6(TRACE_LAYER_NCI, TRACE_TYPE_WARNING, m, p1, p2, p3, p4, p5, \
                 p6);                                                        \
  }

#define NCI_TRACE_API0(m)                             \
  {                                                   \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_API)    \
      BT_TRACE_0(TRACE_LAYER_NCI, TRACE_TYPE_API, m); \
  }
#define NCI_TRACE_API1(m, p1)                             \
  {                                                       \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_API)        \
      BT_TRACE_1(TRACE_LAYER_NCI, TRACE_TYPE_API, m, p1); \
  }
#define NCI_TRACE_API2(m, p1, p2)                             \
  {                                                           \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_API)            \
      BT_TRACE_2(TRACE_LAYER_NCI, TRACE_TYPE_API, m, p1, p2); \
  }
#define NCI_TRACE_API3(m, p1, p2, p3)                             \
  {                                                               \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_API)                \
      BT_TRACE_3(TRACE_LAYER_NCI, TRACE_TYPE_API, m, p1, p2, p3); \
  }
#define NCI_TRACE_API4(m, p1, p2, p3, p4)                             \
  {                                                                   \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_API)                    \
      BT_TRACE_4(TRACE_LAYER_NCI, TRACE_TYPE_API, m, p1, p2, p3, p4); \
  }
#define NCI_TRACE_API5(m, p1, p2, p3, p4, p5)                             \
  {                                                                       \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_API)                        \
      BT_TRACE_5(TRACE_LAYER_NCI, TRACE_TYPE_API, m, p1, p2, p3, p4, p5); \
  }
#define NCI_TRACE_API6(m, p1, p2, p3, p4, p5, p6)                             \
  {                                                                           \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_API)                            \
      BT_TRACE_6(TRACE_LAYER_NCI, TRACE_TYPE_API, m, p1, p2, p3, p4, p5, p6); \
  }

#define NCI_TRACE_EVENT0(m)                             \
  {                                                     \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_EVENT)    \
      BT_TRACE_0(TRACE_LAYER_NCI, TRACE_TYPE_EVENT, m); \
  }
#define NCI_TRACE_EVENT1(m, p1)                             \
  {                                                         \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_EVENT)        \
      BT_TRACE_1(TRACE_LAYER_NCI, TRACE_TYPE_EVENT, m, p1); \
  }
#define NCI_TRACE_EVENT2(m, p1, p2)                             \
  {                                                             \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_EVENT)            \
      BT_TRACE_2(TRACE_LAYER_NCI, TRACE_TYPE_EVENT, m, p1, p2); \
  }
#define NCI_TRACE_EVENT3(m, p1, p2, p3)                             \
  {                                                                 \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                \
      BT_TRACE_3(TRACE_LAYER_NCI, TRACE_TYPE_EVENT, m, p1, p2, p3); \
  }
#define NCI_TRACE_EVENT4(m, p1, p2, p3, p4)                             \
  {                                                                     \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                    \
      BT_TRACE_4(TRACE_LAYER_NCI, TRACE_TYPE_EVENT, m, p1, p2, p3, p4); \
  }
#define NCI_TRACE_EVENT5(m, p1, p2, p3, p4, p5)                             \
  {                                                                         \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                        \
      BT_TRACE_5(TRACE_LAYER_NCI, TRACE_TYPE_EVENT, m, p1, p2, p3, p4, p5); \
  }
#define NCI_TRACE_EVENT6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                        \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                       \
      BT_TRACE_6(TRACE_LAYER_NCI, TRACE_TYPE_EVENT, m, p1, p2, p3, p4, p5, \
                 p6);                                                      \
  }

#define NCI_TRACE_DEBUG0(m)                             \
  {                                                     \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)    \
      BT_TRACE_0(TRACE_LAYER_NCI, TRACE_TYPE_DEBUG, m); \
  }
#define NCI_TRACE_DEBUG1(m, p1)                             \
  {                                                         \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)        \
      BT_TRACE_1(TRACE_LAYER_NCI, TRACE_TYPE_DEBUG, m, p1); \
  }
#define NCI_TRACE_DEBUG2(m, p1, p2)                             \
  {                                                             \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)            \
      BT_TRACE_2(TRACE_LAYER_NCI, TRACE_TYPE_DEBUG, m, p1, p2); \
  }
#define NCI_TRACE_DEBUG3(m, p1, p2, p3)                             \
  {                                                                 \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                \
      BT_TRACE_3(TRACE_LAYER_NCI, TRACE_TYPE_DEBUG, m, p1, p2, p3); \
  }
#define NCI_TRACE_DEBUG4(m, p1, p2, p3, p4)                             \
  {                                                                     \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                    \
      BT_TRACE_4(TRACE_LAYER_NCI, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4); \
  }
#define NCI_TRACE_DEBUG5(m, p1, p2, p3, p4, p5)                             \
  {                                                                         \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                        \
      BT_TRACE_5(TRACE_LAYER_NCI, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4, p5); \
  }
#define NCI_TRACE_DEBUG6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                        \
    if (ncit_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                       \
      BT_TRACE_6(TRACE_LAYER_NCI, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4, p5, \
                 p6);                                                      \
  }

#define RW_TRACE_ERROR0(m)                             \
  {                                                    \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_ERROR)     \
      BT_TRACE_0(TRACE_LAYER_RW, TRACE_TYPE_ERROR, m); \
  }
#define RW_TRACE_ERROR1(m, p1)                             \
  {                                                        \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_ERROR)         \
      BT_TRACE_1(TRACE_LAYER_RW, TRACE_TYPE_ERROR, m, p1); \
  }
#define RW_TRACE_ERROR2(m, p1, p2)                             \
  {                                                            \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_ERROR)             \
      BT_TRACE_2(TRACE_LAYER_RW, TRACE_TYPE_ERROR, m, p1, p2); \
  }
#define RW_TRACE_ERROR3(m, p1, p2, p3)                             \
  {                                                                \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                 \
      BT_TRACE_3(TRACE_LAYER_RW, TRACE_TYPE_ERROR, m, p1, p2, p3); \
  }
#define RW_TRACE_ERROR4(m, p1, p2, p3, p4)                             \
  {                                                                    \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                     \
      BT_TRACE_4(TRACE_LAYER_RW, TRACE_TYPE_ERROR, m, p1, p2, p3, p4); \
  }
#define RW_TRACE_ERROR5(m, p1, p2, p3, p4, p5)                             \
  {                                                                        \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                         \
      BT_TRACE_5(TRACE_LAYER_RW, TRACE_TYPE_ERROR, m, p1, p2, p3, p4, p5); \
  }
#define RW_TRACE_ERROR6(m, p1, p2, p3, p4, p5, p6)                             \
  {                                                                            \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                             \
      BT_TRACE_6(TRACE_LAYER_RW, TRACE_TYPE_ERROR, m, p1, p2, p3, p4, p5, p6); \
  }

#define RW_TRACE_WARNING0(m)                             \
  {                                                      \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_WARNING)     \
      BT_TRACE_0(TRACE_LAYER_RW, TRACE_TYPE_WARNING, m); \
  }
#define RW_TRACE_WARNING1(m, p1)                             \
  {                                                          \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_WARNING)         \
      BT_TRACE_1(TRACE_LAYER_RW, TRACE_TYPE_WARNING, m, p1); \
  }
#define RW_TRACE_WARNING2(m, p1, p2)                             \
  {                                                              \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_WARNING)             \
      BT_TRACE_2(TRACE_LAYER_RW, TRACE_TYPE_WARNING, m, p1, p2); \
  }
#define RW_TRACE_WARNING3(m, p1, p2, p3)                             \
  {                                                                  \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                 \
      BT_TRACE_3(TRACE_LAYER_RW, TRACE_TYPE_WARNING, m, p1, p2, p3); \
  }
#define RW_TRACE_WARNING4(m, p1, p2, p3, p4)                             \
  {                                                                      \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                     \
      BT_TRACE_4(TRACE_LAYER_RW, TRACE_TYPE_WARNING, m, p1, p2, p3, p4); \
  }
#define RW_TRACE_WARNING5(m, p1, p2, p3, p4, p5)                             \
  {                                                                          \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                         \
      BT_TRACE_5(TRACE_LAYER_RW, TRACE_TYPE_WARNING, m, p1, p2, p3, p4, p5); \
  }
#define RW_TRACE_WARNING6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                         \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                        \
      BT_TRACE_6(TRACE_LAYER_RW, TRACE_TYPE_WARNING, m, p1, p2, p3, p4, p5, \
                 p6);                                                       \
  }

#define RW_TRACE_API0(m)                             \
  {                                                  \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_API)     \
      BT_TRACE_0(TRACE_LAYER_RW, TRACE_TYPE_API, m); \
  }
#define RW_TRACE_API1(m, p1)                             \
  {                                                      \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_API)         \
      BT_TRACE_1(TRACE_LAYER_RW, TRACE_TYPE_API, m, p1); \
  }
#define RW_TRACE_API2(m, p1, p2)                             \
  {                                                          \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_API)             \
      BT_TRACE_2(TRACE_LAYER_RW, TRACE_TYPE_API, m, p1, p2); \
  }
#define RW_TRACE_API3(m, p1, p2, p3)                             \
  {                                                              \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_API)                 \
      BT_TRACE_3(TRACE_LAYER_RW, TRACE_TYPE_API, m, p1, p2, p3); \
  }
#define RW_TRACE_API4(m, p1, p2, p3, p4)                             \
  {                                                                  \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_API)                     \
      BT_TRACE_4(TRACE_LAYER_RW, TRACE_TYPE_API, m, p1, p2, p3, p4); \
  }
#define RW_TRACE_API5(m, p1, p2, p3, p4, p5)                             \
  {                                                                      \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_API)                         \
      BT_TRACE_5(TRACE_LAYER_RW, TRACE_TYPE_API, m, p1, p2, p3, p4, p5); \
  }
#define RW_TRACE_API6(m, p1, p2, p3, p4, p5, p6)                             \
  {                                                                          \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_API)                             \
      BT_TRACE_6(TRACE_LAYER_RW, TRACE_TYPE_API, m, p1, p2, p3, p4, p5, p6); \
  }

#define RW_TRACE_EVENT0(m)                             \
  {                                                    \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_EVENT)     \
      BT_TRACE_0(TRACE_LAYER_RW, TRACE_TYPE_EVENT, m); \
  }
#define RW_TRACE_EVENT1(m, p1)                             \
  {                                                        \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_EVENT)         \
      BT_TRACE_1(TRACE_LAYER_RW, TRACE_TYPE_EVENT, m, p1); \
  }
#define RW_TRACE_EVENT2(m, p1, p2)                             \
  {                                                            \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_EVENT)             \
      BT_TRACE_2(TRACE_LAYER_RW, TRACE_TYPE_EVENT, m, p1, p2); \
  }
#define RW_TRACE_EVENT3(m, p1, p2, p3)                             \
  {                                                                \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                 \
      BT_TRACE_3(TRACE_LAYER_RW, TRACE_TYPE_EVENT, m, p1, p2, p3); \
  }
#define RW_TRACE_EVENT4(m, p1, p2, p3, p4)                             \
  {                                                                    \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                     \
      BT_TRACE_4(TRACE_LAYER_RW, TRACE_TYPE_EVENT, m, p1, p2, p3, p4); \
  }
#define RW_TRACE_EVENT5(m, p1, p2, p3, p4, p5)                             \
  {                                                                        \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                         \
      BT_TRACE_5(TRACE_LAYER_RW, TRACE_TYPE_EVENT, m, p1, p2, p3, p4, p5); \
  }
#define RW_TRACE_EVENT6(m, p1, p2, p3, p4, p5, p6)                             \
  {                                                                            \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                             \
      BT_TRACE_6(TRACE_LAYER_RW, TRACE_TYPE_EVENT, m, p1, p2, p3, p4, p5, p6); \
  }

#define RW_TRACE_DEBUG0(m)                             \
  {                                                    \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)     \
      BT_TRACE_0(TRACE_LAYER_RW, TRACE_TYPE_DEBUG, m); \
  }
#define RW_TRACE_DEBUG1(m, p1)                             \
  {                                                        \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)         \
      BT_TRACE_1(TRACE_LAYER_RW, TRACE_TYPE_DEBUG, m, p1); \
  }
#define RW_TRACE_DEBUG2(m, p1, p2)                             \
  {                                                            \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)             \
      BT_TRACE_2(TRACE_LAYER_RW, TRACE_TYPE_DEBUG, m, p1, p2); \
  }
#define RW_TRACE_DEBUG3(m, p1, p2, p3)                             \
  {                                                                \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                 \
      BT_TRACE_3(TRACE_LAYER_RW, TRACE_TYPE_DEBUG, m, p1, p2, p3); \
  }
#define RW_TRACE_DEBUG4(m, p1, p2, p3, p4)                             \
  {                                                                    \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                     \
      BT_TRACE_4(TRACE_LAYER_RW, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4); \
  }
#define RW_TRACE_DEBUG5(m, p1, p2, p3, p4, p5)                             \
  {                                                                        \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                         \
      BT_TRACE_5(TRACE_LAYER_RW, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4, p5); \
  }
#define RW_TRACE_DEBUG6(m, p1, p2, p3, p4, p5, p6)                             \
  {                                                                            \
    if (rw_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                             \
      BT_TRACE_6(TRACE_LAYER_RW, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4, p5, p6); \
  }

#define CE_TRACE_ERROR0(m)                             \
  {                                                    \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_ERROR)     \
      BT_TRACE_0(TRACE_LAYER_CE, TRACE_TYPE_ERROR, m); \
  }
#define CE_TRACE_ERROR1(m, p1)                             \
  {                                                        \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_ERROR)         \
      BT_TRACE_1(TRACE_LAYER_CE, TRACE_TYPE_ERROR, m, p1); \
  }
#define CE_TRACE_ERROR2(m, p1, p2)                             \
  {                                                            \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_ERROR)             \
      BT_TRACE_2(TRACE_LAYER_CE, TRACE_TYPE_ERROR, m, p1, p2); \
  }
#define CE_TRACE_ERROR3(m, p1, p2, p3)                             \
  {                                                                \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                 \
      BT_TRACE_3(TRACE_LAYER_CE, TRACE_TYPE_ERROR, m, p1, p2, p3); \
  }
#define CE_TRACE_ERROR4(m, p1, p2, p3, p4)                             \
  {                                                                    \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                     \
      BT_TRACE_4(TRACE_LAYER_CE, TRACE_TYPE_ERROR, m, p1, p2, p3, p4); \
  }
#define CE_TRACE_ERROR5(m, p1, p2, p3, p4, p5)                             \
  {                                                                        \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                         \
      BT_TRACE_5(TRACE_LAYER_CE, TRACE_TYPE_ERROR, m, p1, p2, p3, p4, p5); \
  }
#define CE_TRACE_ERROR6(m, p1, p2, p3, p4, p5, p6)                             \
  {                                                                            \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                             \
      BT_TRACE_6(TRACE_LAYER_CE, TRACE_TYPE_ERROR, m, p1, p2, p3, p4, p5, p6); \
  }

#define CE_TRACE_WARNING0(m)                             \
  {                                                      \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_WARNING)     \
      BT_TRACE_0(TRACE_LAYER_CE, TRACE_TYPE_WARNING, m); \
  }
#define CE_TRACE_WARNING1(m, p1)                             \
  {                                                          \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_WARNING)         \
      BT_TRACE_1(TRACE_LAYER_CE, TRACE_TYPE_WARNING, m, p1); \
  }
#define CE_TRACE_WARNING2(m, p1, p2)                             \
  {                                                              \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_WARNING)             \
      BT_TRACE_2(TRACE_LAYER_CE, TRACE_TYPE_WARNING, m, p1, p2); \
  }
#define CE_TRACE_WARNING3(m, p1, p2, p3)                             \
  {                                                                  \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                 \
      BT_TRACE_3(TRACE_LAYER_CE, TRACE_TYPE_WARNING, m, p1, p2, p3); \
  }
#define CE_TRACE_WARNING4(m, p1, p2, p3, p4)                             \
  {                                                                      \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                     \
      BT_TRACE_4(TRACE_LAYER_CE, TRACE_TYPE_WARNING, m, p1, p2, p3, p4); \
  }
#define CE_TRACE_WARNING5(m, p1, p2, p3, p4, p5)                             \
  {                                                                          \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                         \
      BT_TRACE_5(TRACE_LAYER_CE, TRACE_TYPE_WARNING, m, p1, p2, p3, p4, p5); \
  }
#define CE_TRACE_WARNING6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                         \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                        \
      BT_TRACE_6(TRACE_LAYER_CE, TRACE_TYPE_WARNING, m, p1, p2, p3, p4, p5, \
                 p6);                                                       \
  }

#define CE_TRACE_API0(m)                             \
  {                                                  \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_API)     \
      BT_TRACE_0(TRACE_LAYER_CE, TRACE_TYPE_API, m); \
  }
#define CE_TRACE_API1(m, p1)                             \
  {                                                      \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_API)         \
      BT_TRACE_1(TRACE_LAYER_CE, TRACE_TYPE_API, m, p1); \
  }
#define CE_TRACE_API2(m, p1, p2)                             \
  {                                                          \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_API)             \
      BT_TRACE_2(TRACE_LAYER_CE, TRACE_TYPE_API, m, p1, p2); \
  }
#define CE_TRACE_API3(m, p1, p2, p3)                             \
  {                                                              \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_API)                 \
      BT_TRACE_3(TRACE_LAYER_CE, TRACE_TYPE_API, m, p1, p2, p3); \
  }
#define CE_TRACE_API4(m, p1, p2, p3, p4)                             \
  {                                                                  \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_API)                     \
      BT_TRACE_4(TRACE_LAYER_CE, TRACE_TYPE_API, m, p1, p2, p3, p4); \
  }
#define CE_TRACE_API5(m, p1, p2, p3, p4, p5)                             \
  {                                                                      \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_API)                         \
      BT_TRACE_5(TRACE_LAYER_CE, TRACE_TYPE_API, m, p1, p2, p3, p4, p5); \
  }
#define CE_TRACE_API6(m, p1, p2, p3, p4, p5, p6)                             \
  {                                                                          \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_API)                             \
      BT_TRACE_6(TRACE_LAYER_CE, TRACE_TYPE_API, m, p1, p2, p3, p4, p5, p6); \
  }

#define CE_TRACE_EVENT0(m)                             \
  {                                                    \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_EVENT)     \
      BT_TRACE_0(TRACE_LAYER_CE, TRACE_TYPE_EVENT, m); \
  }
#define CE_TRACE_EVENT1(m, p1)                             \
  {                                                        \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_EVENT)         \
      BT_TRACE_1(TRACE_LAYER_CE, TRACE_TYPE_EVENT, m, p1); \
  }
#define CE_TRACE_EVENT2(m, p1, p2)                             \
  {                                                            \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_EVENT)             \
      BT_TRACE_2(TRACE_LAYER_CE, TRACE_TYPE_EVENT, m, p1, p2); \
  }
#define CE_TRACE_EVENT3(m, p1, p2, p3)                             \
  {                                                                \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                 \
      BT_TRACE_3(TRACE_LAYER_CE, TRACE_TYPE_EVENT, m, p1, p2, p3); \
  }
#define CE_TRACE_EVENT4(m, p1, p2, p3, p4)                             \
  {                                                                    \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                     \
      BT_TRACE_4(TRACE_LAYER_CE, TRACE_TYPE_EVENT, m, p1, p2, p3, p4); \
  }
#define CE_TRACE_EVENT5(m, p1, p2, p3, p4, p5)                             \
  {                                                                        \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                         \
      BT_TRACE_5(TRACE_LAYER_CE, TRACE_TYPE_EVENT, m, p1, p2, p3, p4, p5); \
  }
#define CE_TRACE_EVENT6(m, p1, p2, p3, p4, p5, p6)                             \
  {                                                                            \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                             \
      BT_TRACE_6(TRACE_LAYER_CE, TRACE_TYPE_EVENT, m, p1, p2, p3, p4, p5, p6); \
  }

#define CE_TRACE_DEBUG0(m)                             \
  {                                                    \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)     \
      BT_TRACE_0(TRACE_LAYER_CE, TRACE_TYPE_DEBUG, m); \
  }
#define CE_TRACE_DEBUG1(m, p1)                             \
  {                                                        \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)         \
      BT_TRACE_1(TRACE_LAYER_CE, TRACE_TYPE_DEBUG, m, p1); \
  }
#define CE_TRACE_DEBUG2(m, p1, p2)                             \
  {                                                            \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)             \
      BT_TRACE_2(TRACE_LAYER_CE, TRACE_TYPE_DEBUG, m, p1, p2); \
  }
#define CE_TRACE_DEBUG3(m, p1, p2, p3)                             \
  {                                                                \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                 \
      BT_TRACE_3(TRACE_LAYER_CE, TRACE_TYPE_DEBUG, m, p1, p2, p3); \
  }
#define CE_TRACE_DEBUG4(m, p1, p2, p3, p4)                             \
  {                                                                    \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                     \
      BT_TRACE_4(TRACE_LAYER_CE, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4); \
  }
#define CE_TRACE_DEBUG5(m, p1, p2, p3, p4, p5)                             \
  {                                                                        \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                         \
      BT_TRACE_5(TRACE_LAYER_CE, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4, p5); \
  }
#define CE_TRACE_DEBUG6(m, p1, p2, p3, p4, p5, p6)                             \
  {                                                                            \
    if (ce_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                             \
      BT_TRACE_6(TRACE_LAYER_CE, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4, p5, p6); \
  }

#define NDEF_TRACE_ERROR0(m)                             \
  {                                                      \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_ERROR)     \
      BT_TRACE_0(TRACE_LAYER_NDEF, TRACE_TYPE_ERROR, m); \
  }
#define NDEF_TRACE_ERROR1(m, p1)                             \
  {                                                          \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_ERROR)         \
      BT_TRACE_1(TRACE_LAYER_NDEF, TRACE_TYPE_ERROR, m, p1); \
  }
#define NDEF_TRACE_ERROR2(m, p1, p2)                             \
  {                                                              \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_ERROR)             \
      BT_TRACE_2(TRACE_LAYER_NDEF, TRACE_TYPE_ERROR, m, p1, p2); \
  }
#define NDEF_TRACE_ERROR3(m, p1, p2, p3)                             \
  {                                                                  \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                 \
      BT_TRACE_3(TRACE_LAYER_NDEF, TRACE_TYPE_ERROR, m, p1, p2, p3); \
  }
#define NDEF_TRACE_ERROR4(m, p1, p2, p3, p4)                             \
  {                                                                      \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                     \
      BT_TRACE_4(TRACE_LAYER_NDEF, TRACE_TYPE_ERROR, m, p1, p2, p3, p4); \
  }
#define NDEF_TRACE_ERROR5(m, p1, p2, p3, p4, p5)                             \
  {                                                                          \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                         \
      BT_TRACE_5(TRACE_LAYER_NDEF, TRACE_TYPE_ERROR, m, p1, p2, p3, p4, p5); \
  }
#define NDEF_TRACE_ERROR6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                         \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                        \
      BT_TRACE_6(TRACE_LAYER_NDEF, TRACE_TYPE_ERROR, m, p1, p2, p3, p4, p5, \
                 p6);                                                       \
  }

#define NDEF_TRACE_WARNING0(m)                             \
  {                                                        \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_WARNING)     \
      BT_TRACE_0(TRACE_LAYER_NDEF, TRACE_TYPE_WARNING, m); \
  }
#define NDEF_TRACE_WARNING1(m, p1)                             \
  {                                                            \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_WARNING)         \
      BT_TRACE_1(TRACE_LAYER_NDEF, TRACE_TYPE_WARNING, m, p1); \
  }
#define NDEF_TRACE_WARNING2(m, p1, p2)                             \
  {                                                                \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_WARNING)             \
      BT_TRACE_2(TRACE_LAYER_NDEF, TRACE_TYPE_WARNING, m, p1, p2); \
  }
#define NDEF_TRACE_WARNING3(m, p1, p2, p3)                             \
  {                                                                    \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                 \
      BT_TRACE_3(TRACE_LAYER_NDEF, TRACE_TYPE_WARNING, m, p1, p2, p3); \
  }
#define NDEF_TRACE_WARNING4(m, p1, p2, p3, p4)                             \
  {                                                                        \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                     \
      BT_TRACE_4(TRACE_LAYER_NDEF, TRACE_TYPE_WARNING, m, p1, p2, p3, p4); \
  }
#define NDEF_TRACE_WARNING5(m, p1, p2, p3, p4, p5)                             \
  {                                                                            \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                         \
      BT_TRACE_5(TRACE_LAYER_NDEF, TRACE_TYPE_WARNING, m, p1, p2, p3, p4, p5); \
  }
#define NDEF_TRACE_WARNING6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                           \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                        \
      BT_TRACE_6(TRACE_LAYER_NDEF, TRACE_TYPE_WARNING, m, p1, p2, p3, p4, p5, \
                 p6);                                                         \
  }

#define NDEF_TRACE_API0(m)                             \
  {                                                    \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_API)     \
      BT_TRACE_0(TRACE_LAYER_NDEF, TRACE_TYPE_API, m); \
  }
#define NDEF_TRACE_API1(m, p1)                             \
  {                                                        \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_API)         \
      BT_TRACE_1(TRACE_LAYER_NDEF, TRACE_TYPE_API, m, p1); \
  }
#define NDEF_TRACE_API2(m, p1, p2)                             \
  {                                                            \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_API)             \
      BT_TRACE_2(TRACE_LAYER_NDEF, TRACE_TYPE_API, m, p1, p2); \
  }
#define NDEF_TRACE_API3(m, p1, p2, p3)                             \
  {                                                                \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_API)                 \
      BT_TRACE_3(TRACE_LAYER_NDEF, TRACE_TYPE_API, m, p1, p2, p3); \
  }
#define NDEF_TRACE_API4(m, p1, p2, p3, p4)                             \
  {                                                                    \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_API)                     \
      BT_TRACE_4(TRACE_LAYER_NDEF, TRACE_TYPE_API, m, p1, p2, p3, p4); \
  }
#define NDEF_TRACE_API5(m, p1, p2, p3, p4, p5)                             \
  {                                                                        \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_API)                         \
      BT_TRACE_5(TRACE_LAYER_NDEF, TRACE_TYPE_API, m, p1, p2, p3, p4, p5); \
  }
#define NDEF_TRACE_API6(m, p1, p2, p3, p4, p5, p6)                             \
  {                                                                            \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_API)                             \
      BT_TRACE_6(TRACE_LAYER_NDEF, TRACE_TYPE_API, m, p1, p2, p3, p4, p5, p6); \
  }

#define NDEF_TRACE_EVENT0(m)                             \
  {                                                      \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_EVENT)     \
      BT_TRACE_0(TRACE_LAYER_NDEF, TRACE_TYPE_EVENT, m); \
  }
#define NDEF_TRACE_EVENT1(m, p1)                             \
  {                                                          \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_EVENT)         \
      BT_TRACE_1(TRACE_LAYER_NDEF, TRACE_TYPE_EVENT, m, p1); \
  }
#define NDEF_TRACE_EVENT2(m, p1, p2)                             \
  {                                                              \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_EVENT)             \
      BT_TRACE_2(TRACE_LAYER_NDEF, TRACE_TYPE_EVENT, m, p1, p2); \
  }
#define NDEF_TRACE_EVENT3(m, p1, p2, p3)                             \
  {                                                                  \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                 \
      BT_TRACE_3(TRACE_LAYER_NDEF, TRACE_TYPE_EVENT, m, p1, p2, p3); \
  }
#define NDEF_TRACE_EVENT4(m, p1, p2, p3, p4)                             \
  {                                                                      \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                     \
      BT_TRACE_4(TRACE_LAYER_NDEF, TRACE_TYPE_EVENT, m, p1, p2, p3, p4); \
  }
#define NDEF_TRACE_EVENT5(m, p1, p2, p3, p4, p5)                             \
  {                                                                          \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                         \
      BT_TRACE_5(TRACE_LAYER_NDEF, TRACE_TYPE_EVENT, m, p1, p2, p3, p4, p5); \
  }
#define NDEF_TRACE_EVENT6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                         \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                        \
      BT_TRACE_6(TRACE_LAYER_NDEF, TRACE_TYPE_EVENT, m, p1, p2, p3, p4, p5, \
                 p6);                                                       \
  }

#define NDEF_TRACE_DEBUG0(m)                             \
  {                                                      \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)     \
      BT_TRACE_0(TRACE_LAYER_NDEF, TRACE_TYPE_DEBUG, m); \
  }
#define NDEF_TRACE_DEBUG1(m, p1)                             \
  {                                                          \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)         \
      BT_TRACE_1(TRACE_LAYER_NDEF, TRACE_TYPE_DEBUG, m, p1); \
  }
#define NDEF_TRACE_DEBUG2(m, p1, p2)                             \
  {                                                              \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)             \
      BT_TRACE_2(TRACE_LAYER_NDEF, TRACE_TYPE_DEBUG, m, p1, p2); \
  }
#define NDEF_TRACE_DEBUG3(m, p1, p2, p3)                             \
  {                                                                  \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                 \
      BT_TRACE_3(TRACE_LAYER_NDEF, TRACE_TYPE_DEBUG, m, p1, p2, p3); \
  }
#define NDEF_TRACE_DEBUG4(m, p1, p2, p3, p4)                             \
  {                                                                      \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                     \
      BT_TRACE_4(TRACE_LAYER_NDEF, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4); \
  }
#define NDEF_TRACE_DEBUG5(m, p1, p2, p3, p4, p5)                             \
  {                                                                          \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                         \
      BT_TRACE_5(TRACE_LAYER_NDEF, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4, p5); \
  }
#define NDEF_TRACE_DEBUG6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                         \
    if (ndef_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                        \
      BT_TRACE_6(TRACE_LAYER_NDEF, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4, p5, \
                 p6);                                                       \
  }

/* Define tracing for the NFA unit
*/
#define NFA_TRACE_ERROR0(m)                             \
  {                                                     \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_ERROR) \
      BT_TRACE_0(TRACE_LAYER_NFA, TRACE_TYPE_ERROR, m); \
  }
#define NFA_TRACE_ERROR1(m, p1)                             \
  {                                                         \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_ERROR)     \
      BT_TRACE_1(TRACE_LAYER_NFA, TRACE_TYPE_ERROR, m, p1); \
  }
#define NFA_TRACE_ERROR2(m, p1, p2)                             \
  {                                                             \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_ERROR)         \
      BT_TRACE_2(TRACE_LAYER_NFA, TRACE_TYPE_ERROR, m, p1, p2); \
  }
#define NFA_TRACE_ERROR3(m, p1, p2, p3)                             \
  {                                                                 \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_ERROR)             \
      BT_TRACE_3(TRACE_LAYER_NFA, TRACE_TYPE_ERROR, m, p1, p2, p3); \
  }
#define NFA_TRACE_ERROR4(m, p1, p2, p3, p4)                             \
  {                                                                     \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                 \
      BT_TRACE_4(TRACE_LAYER_NFA, TRACE_TYPE_ERROR, m, p1, p2, p3, p4); \
  }
#define NFA_TRACE_ERROR5(m, p1, p2, p3, p4, p5)                             \
  {                                                                         \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                     \
      BT_TRACE_5(TRACE_LAYER_NFA, TRACE_TYPE_ERROR, m, p1, p2, p3, p4, p5); \
  }
#define NFA_TRACE_ERROR6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                        \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                    \
      BT_TRACE_6(TRACE_LAYER_NFA, TRACE_TYPE_ERROR, m, p1, p2, p3, p4, p5, \
                 p6);                                                      \
  }

#define NFA_TRACE_WARNING0(m)                             \
  {                                                       \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_WARNING) \
      BT_TRACE_0(TRACE_LAYER_NFA, TRACE_TYPE_WARNING, m); \
  }
#define NFA_TRACE_WARNING1(m, p1)                             \
  {                                                           \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_WARNING)     \
      BT_TRACE_1(TRACE_LAYER_NFA, TRACE_TYPE_WARNING, m, p1); \
  }
#define NFA_TRACE_WARNING2(m, p1, p2)                             \
  {                                                               \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_WARNING)         \
      BT_TRACE_2(TRACE_LAYER_NFA, TRACE_TYPE_WARNING, m, p1, p2); \
  }
#define NFA_TRACE_WARNING3(m, p1, p2, p3)                             \
  {                                                                   \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_WARNING)             \
      BT_TRACE_3(TRACE_LAYER_NFA, TRACE_TYPE_WARNING, m, p1, p2, p3); \
  }
#define NFA_TRACE_WARNING4(m, p1, p2, p3, p4)                             \
  {                                                                       \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                 \
      BT_TRACE_4(TRACE_LAYER_NFA, TRACE_TYPE_WARNING, m, p1, p2, p3, p4); \
  }
#define NFA_TRACE_WARNING5(m, p1, p2, p3, p4, p5)                             \
  {                                                                           \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                     \
      BT_TRACE_5(TRACE_LAYER_NFA, TRACE_TYPE_WARNING, m, p1, p2, p3, p4, p5); \
  }
#define NFA_TRACE_WARNING6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                          \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                    \
      BT_TRACE_6(TRACE_LAYER_NFA, TRACE_TYPE_WARNING, m, p1, p2, p3, p4, p5, \
                 p6);                                                        \
  }

#define NFA_TRACE_API0(m)                             \
  {                                                   \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_API) \
      BT_TRACE_0(TRACE_LAYER_NFA, TRACE_TYPE_API, m); \
  }
#define NFA_TRACE_API1(m, p1)                             \
  {                                                       \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_API)     \
      BT_TRACE_1(TRACE_LAYER_NFA, TRACE_TYPE_API, m, p1); \
  }
#define NFA_TRACE_API2(m, p1, p2)                             \
  {                                                           \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_API)         \
      BT_TRACE_2(TRACE_LAYER_NFA, TRACE_TYPE_API, m, p1, p2); \
  }
#define NFA_TRACE_API3(m, p1, p2, p3)                             \
  {                                                               \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_API)             \
      BT_TRACE_3(TRACE_LAYER_NFA, TRACE_TYPE_API, m, p1, p2, p3); \
  }
#define NFA_TRACE_API4(m, p1, p2, p3, p4)                             \
  {                                                                   \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_API)                 \
      BT_TRACE_4(TRACE_LAYER_NFA, TRACE_TYPE_API, m, p1, p2, p3, p4); \
  }
#define NFA_TRACE_API5(m, p1, p2, p3, p4, p5)                             \
  {                                                                       \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_API)                     \
      BT_TRACE_5(TRACE_LAYER_NFA, TRACE_TYPE_API, m, p1, p2, p3, p4, p5); \
  }
#define NFA_TRACE_API6(m, p1, p2, p3, p4, p5, p6)                             \
  {                                                                           \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_API)                         \
      BT_TRACE_6(TRACE_LAYER_NFA, TRACE_TYPE_API, m, p1, p2, p3, p4, p5, p6); \
  }

#define NFA_TRACE_EVENT0(m)                             \
  {                                                     \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_EVENT) \
      BT_TRACE_0(TRACE_LAYER_NFA, TRACE_TYPE_EVENT, m); \
  }
#define NFA_TRACE_EVENT1(m, p1)                             \
  {                                                         \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_EVENT)     \
      BT_TRACE_1(TRACE_LAYER_NFA, TRACE_TYPE_EVENT, m, p1); \
  }
#define NFA_TRACE_EVENT2(m, p1, p2)                             \
  {                                                             \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_EVENT)         \
      BT_TRACE_2(TRACE_LAYER_NFA, TRACE_TYPE_EVENT, m, p1, p2); \
  }
#define NFA_TRACE_EVENT3(m, p1, p2, p3)                             \
  {                                                                 \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_EVENT)             \
      BT_TRACE_3(TRACE_LAYER_NFA, TRACE_TYPE_EVENT, m, p1, p2, p3); \
  }
#define NFA_TRACE_EVENT4(m, p1, p2, p3, p4)                             \
  {                                                                     \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                 \
      BT_TRACE_4(TRACE_LAYER_NFA, TRACE_TYPE_EVENT, m, p1, p2, p3, p4); \
  }
#define NFA_TRACE_EVENT5(m, p1, p2, p3, p4, p5)                             \
  {                                                                         \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                     \
      BT_TRACE_5(TRACE_LAYER_NFA, TRACE_TYPE_EVENT, m, p1, p2, p3, p4, p5); \
  }
#define NFA_TRACE_EVENT6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                        \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                    \
      BT_TRACE_6(TRACE_LAYER_NFA, TRACE_TYPE_EVENT, m, p1, p2, p3, p4, p5, \
                 p6);                                                      \
  }

#define NFA_TRACE_DEBUG0(m)                             \
  {                                                     \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_DEBUG) \
      BT_TRACE_0(TRACE_LAYER_NFA, TRACE_TYPE_DEBUG, m); \
  }
#define NFA_TRACE_DEBUG1(m, p1)                             \
  {                                                         \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)     \
      BT_TRACE_1(TRACE_LAYER_NFA, TRACE_TYPE_DEBUG, m, p1); \
  }
#define NFA_TRACE_DEBUG2(m, p1, p2)                             \
  {                                                             \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)         \
      BT_TRACE_2(TRACE_LAYER_NFA, TRACE_TYPE_DEBUG, m, p1, p2); \
  }
#define NFA_TRACE_DEBUG3(m, p1, p2, p3)                             \
  {                                                                 \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)             \
      BT_TRACE_3(TRACE_LAYER_NFA, TRACE_TYPE_DEBUG, m, p1, p2, p3); \
  }
#define NFA_TRACE_DEBUG4(m, p1, p2, p3, p4)                             \
  {                                                                     \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                 \
      BT_TRACE_4(TRACE_LAYER_NFA, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4); \
  }
#define NFA_TRACE_DEBUG5(m, p1, p2, p3, p4, p5)                             \
  {                                                                         \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                     \
      BT_TRACE_5(TRACE_LAYER_NFA, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4, p5); \
  }
#define NFA_TRACE_DEBUG6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                        \
    if (nfa_sys_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                    \
      BT_TRACE_6(TRACE_LAYER_NFA, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4, p5, \
                 p6);                                                      \
  }

/* Define tracing for the NFA P2P unit
*/
#define P2P_TRACE_ERROR0(m)                             \
  {                                                     \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_ERROR) \
      BT_TRACE_0(TRACE_LAYER_P2P, TRACE_TYPE_ERROR, m); \
  }
#define P2P_TRACE_ERROR1(m, p1)                             \
  {                                                         \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_ERROR)     \
      BT_TRACE_1(TRACE_LAYER_P2P, TRACE_TYPE_ERROR, m, p1); \
  }
#define P2P_TRACE_ERROR2(m, p1, p2)                             \
  {                                                             \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_ERROR)         \
      BT_TRACE_2(TRACE_LAYER_P2P, TRACE_TYPE_ERROR, m, p1, p2); \
  }
#define P2P_TRACE_ERROR3(m, p1, p2, p3)                             \
  {                                                                 \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_ERROR)             \
      BT_TRACE_3(TRACE_LAYER_P2P, TRACE_TYPE_ERROR, m, p1, p2, p3); \
  }
#define P2P_TRACE_ERROR4(m, p1, p2, p3, p4)                             \
  {                                                                     \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                 \
      BT_TRACE_4(TRACE_LAYER_P2P, TRACE_TYPE_ERROR, m, p1, p2, p3, p4); \
  }
#define P2P_TRACE_ERROR5(m, p1, p2, p3, p4, p5)                             \
  {                                                                         \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                     \
      BT_TRACE_5(TRACE_LAYER_P2P, TRACE_TYPE_ERROR, m, p1, p2, p3, p4, p5); \
  }
#define P2P_TRACE_ERROR6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                        \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_ERROR)                    \
      BT_TRACE_6(TRACE_LAYER_P2P, TRACE_TYPE_ERROR, m, p1, p2, p3, p4, p5, \
                 p6);                                                      \
  }

#define P2P_TRACE_WARNING0(m)                             \
  {                                                       \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_WARNING) \
      BT_TRACE_0(TRACE_LAYER_P2P, TRACE_TYPE_WARNING, m); \
  }
#define P2P_TRACE_WARNING1(m, p1)                             \
  {                                                           \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_WARNING)     \
      BT_TRACE_1(TRACE_LAYER_P2P, TRACE_TYPE_WARNING, m, p1); \
  }
#define P2P_TRACE_WARNING2(m, p1, p2)                             \
  {                                                               \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_WARNING)         \
      BT_TRACE_2(TRACE_LAYER_P2P, TRACE_TYPE_WARNING, m, p1, p2); \
  }
#define P2P_TRACE_WARNING3(m, p1, p2, p3)                             \
  {                                                                   \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_WARNING)             \
      BT_TRACE_3(TRACE_LAYER_P2P, TRACE_TYPE_WARNING, m, p1, p2, p3); \
  }
#define P2P_TRACE_WARNING4(m, p1, p2, p3, p4)                             \
  {                                                                       \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                 \
      BT_TRACE_4(TRACE_LAYER_P2P, TRACE_TYPE_WARNING, m, p1, p2, p3, p4); \
  }
#define P2P_TRACE_WARNING5(m, p1, p2, p3, p4, p5)                             \
  {                                                                           \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                     \
      BT_TRACE_5(TRACE_LAYER_P2P, TRACE_TYPE_WARNING, m, p1, p2, p3, p4, p5); \
  }
#define P2P_TRACE_WARNING6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                          \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_WARNING)                    \
      BT_TRACE_6(TRACE_LAYER_P2P, TRACE_TYPE_WARNING, m, p1, p2, p3, p4, p5, \
                 p6);                                                        \
  }

#define P2P_TRACE_API0(m)                             \
  {                                                   \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_API) \
      BT_TRACE_0(TRACE_LAYER_P2P, TRACE_TYPE_API, m); \
  }
#define P2P_TRACE_API1(m, p1)                             \
  {                                                       \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_API)     \
      BT_TRACE_1(TRACE_LAYER_P2P, TRACE_TYPE_API, m, p1); \
  }
#define P2P_TRACE_API2(m, p1, p2)                             \
  {                                                           \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_API)         \
      BT_TRACE_2(TRACE_LAYER_P2P, TRACE_TYPE_API, m, p1, p2); \
  }
#define P2P_TRACE_API3(m, p1, p2, p3)                             \
  {                                                               \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_API)             \
      BT_TRACE_3(TRACE_LAYER_P2P, TRACE_TYPE_API, m, p1, p2, p3); \
  }
#define P2P_TRACE_API4(m, p1, p2, p3, p4)                             \
  {                                                                   \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_API)                 \
      BT_TRACE_4(TRACE_LAYER_P2P, TRACE_TYPE_API, m, p1, p2, p3, p4); \
  }
#define P2P_TRACE_API5(m, p1, p2, p3, p4, p5)                             \
  {                                                                       \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_API)                     \
      BT_TRACE_5(TRACE_LAYER_P2P, TRACE_TYPE_API, m, p1, p2, p3, p4, p5); \
  }
#define P2P_TRACE_API6(m, p1, p2, p3, p4, p5, p6)                             \
  {                                                                           \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_API)                         \
      BT_TRACE_6(TRACE_LAYER_P2P, TRACE_TYPE_API, m, p1, p2, p3, p4, p5, p6); \
  }

#define P2P_TRACE_EVENT0(m)                             \
  {                                                     \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_EVENT) \
      BT_TRACE_0(TRACE_LAYER_P2P, TRACE_TYPE_EVENT, m); \
  }
#define P2P_TRACE_EVENT1(m, p1)                             \
  {                                                         \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_EVENT)     \
      BT_TRACE_1(TRACE_LAYER_P2P, TRACE_TYPE_EVENT, m, p1); \
  }
#define P2P_TRACE_EVENT2(m, p1, p2)                             \
  {                                                             \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_EVENT)         \
      BT_TRACE_2(TRACE_LAYER_P2P, TRACE_TYPE_EVENT, m, p1, p2); \
  }
#define P2P_TRACE_EVENT3(m, p1, p2, p3)                             \
  {                                                                 \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_EVENT)             \
      BT_TRACE_3(TRACE_LAYER_P2P, TRACE_TYPE_EVENT, m, p1, p2, p3); \
  }
#define P2P_TRACE_EVENT4(m, p1, p2, p3, p4)                             \
  {                                                                     \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                 \
      BT_TRACE_4(TRACE_LAYER_P2P, TRACE_TYPE_EVENT, m, p1, p2, p3, p4); \
  }
#define P2P_TRACE_EVENT5(m, p1, p2, p3, p4, p5)                             \
  {                                                                         \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                     \
      BT_TRACE_5(TRACE_LAYER_P2P, TRACE_TYPE_EVENT, m, p1, p2, p3, p4, p5); \
  }
#define P2P_TRACE_EVENT6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                        \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_EVENT)                    \
      BT_TRACE_6(TRACE_LAYER_P2P, TRACE_TYPE_EVENT, m, p1, p2, p3, p4, p5, \
                 p6);                                                      \
  }

#define P2P_TRACE_DEBUG0(m)                             \
  {                                                     \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_DEBUG) \
      BT_TRACE_0(TRACE_LAYER_P2P, TRACE_TYPE_DEBUG, m); \
  }
#define P2P_TRACE_DEBUG1(m, p1)                             \
  {                                                         \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)     \
      BT_TRACE_1(TRACE_LAYER_P2P, TRACE_TYPE_DEBUG, m, p1); \
  }
#define P2P_TRACE_DEBUG2(m, p1, p2)                             \
  {                                                             \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)         \
      BT_TRACE_2(TRACE_LAYER_P2P, TRACE_TYPE_DEBUG, m, p1, p2); \
  }
#define P2P_TRACE_DEBUG3(m, p1, p2, p3)                             \
  {                                                                 \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)             \
      BT_TRACE_3(TRACE_LAYER_P2P, TRACE_TYPE_DEBUG, m, p1, p2, p3); \
  }
#define P2P_TRACE_DEBUG4(m, p1, p2, p3, p4)                             \
  {                                                                     \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                 \
      BT_TRACE_4(TRACE_LAYER_P2P, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4); \
  }
#define P2P_TRACE_DEBUG5(m, p1, p2, p3, p4, p5)                             \
  {                                                                         \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                     \
      BT_TRACE_5(TRACE_LAYER_P2P, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4, p5); \
  }
#define P2P_TRACE_DEBUG6(m, p1, p2, p3, p4, p5, p6)                        \
  {                                                                        \
    if (nfa_p2p_cb.trace_level >= BT_TRACE_LEVEL_DEBUG)                    \
      BT_TRACE_6(TRACE_LAYER_P2P, TRACE_TYPE_DEBUG, m, p1, p2, p3, p4, p5, \
                 p6);                                                      \
  }

/* define traces for application */
#define APPL_TRACE_ERROR0(m)                                            \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_ERROR)                       \
      LogMsg_0(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_ERROR,                                    \
               (m));                                                    \
  }
#define APPL_TRACE_ERROR1(m, p1)                                        \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_ERROR)                       \
      LogMsg_1(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_ERROR,                                    \
               (m), (uintptr_t)(p1));                                   \
  }
#define APPL_TRACE_ERROR2(m, p1, p2)                                    \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_ERROR)                       \
      LogMsg_2(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_ERROR,                                    \
               (m), (uintptr_t)(p1), (uintptr_t)(p2));                  \
  }
#define APPL_TRACE_ERROR3(m, p1, p2, p3)                                \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_ERROR)                       \
      LogMsg_3(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_ERROR,                                    \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3)); \
  }
#define APPL_TRACE_ERROR4(m, p1, p2, p3, p4)                            \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_ERROR)                       \
      LogMsg_4(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_ERROR,                                    \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3),  \
               (uintptr_t)(p4));                                        \
  }
#define APPL_TRACE_ERROR5(m, p1, p2, p3, p4, p5)                        \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_ERROR)                       \
      LogMsg_5(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_ERROR,                                    \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3),  \
               (uintptr_t)(p4), (uintptr_t)(p5));                       \
  }
#define APPL_TRACE_ERROR6(m, p1, p2, p3, p4, p5, p6)                    \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_ERROR)                       \
      LogMsg_6(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_ERROR,                                    \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3),  \
               (uintptr_t)(p4), (uintptr_t)(p5), (uintptr_t)(p6));      \
  }

#define APPL_TRACE_WARNING0(m)                                          \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_WARNING)                     \
      LogMsg_0(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_WARNING,                                  \
               (m));                                                    \
  }
#define APPL_TRACE_WARNING1(m, p1)                                      \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_WARNING)                     \
      LogMsg_1(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_WARNING,                                  \
               (m), (uintptr_t)(p1));                                   \
  }
#define APPL_TRACE_WARNING2(m, p1, p2)                                  \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_WARNING)                     \
      LogMsg_2(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_WARNING,                                  \
               (m), (uintptr_t)(p1), (uintptr_t)(p2));                  \
  }
#define APPL_TRACE_WARNING3(m, p1, p2, p3)                              \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_WARNING)                     \
      LogMsg_3(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_WARNING,                                  \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3)); \
  }
#define APPL_TRACE_WARNING4(m, p1, p2, p3, p4)                          \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_WARNING)                     \
      LogMsg_4(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_WARNING,                                  \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3),  \
               (uintptr_t)(p4));                                        \
  }
#define APPL_TRACE_WARNING5(m, p1, p2, p3, p4, p5)                      \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_WARNING)                     \
      LogMsg_5(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_WARNING,                                  \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3),  \
               (uintptr_t)(p4), (uintptr_t)(p5));                       \
  }
#define APPL_TRACE_WARNING6(m, p1, p2, p3, p4, p5, p6)                  \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_WARNING)                     \
      LogMsg_6(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_WARNING,                                  \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3),  \
               (uintptr_t)(p4), (uintptr_t)(p5), (uintptr_t)(p6));      \
  }

#define APPL_TRACE_API0(m)                                              \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_API)                         \
      LogMsg_0(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_API,                                      \
               (m));                                                    \
  }
#define APPL_TRACE_API1(m, p1)                                          \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_API)                         \
      LogMsg_1(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_API,                                      \
               (m), (uintptr_t)(p1));                                   \
  }
#define APPL_TRACE_API2(m, p1, p2)                                      \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_API)                         \
      LogMsg_2(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_API,                                      \
               (m), (uintptr_t)(p1), (uintptr_t)(p2));                  \
  }
#define APPL_TRACE_API3(m, p1, p2, p3)                                  \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_API)                         \
      LogMsg_3(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_API,                                      \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3)); \
  }
#define APPL_TRACE_API4(m, p1, p2, p3, p4)                              \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_API)                         \
      LogMsg_4(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_API,                                      \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3),  \
               (uintptr_t)(p4));                                        \
  }
#define APPL_TRACE_API5(m, p1, p2, p3, p4, p5)                          \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_API)                         \
      LogMsg_5(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_API,                                      \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3),  \
               (uintptr_t)(p4), (uintptr_t)(p5));                       \
  }
#define APPL_TRACE_API6(m, p1, p2, p3, p4, p5, p6)                      \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_API)                         \
      LogMsg_6(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_API,                                      \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3),  \
               (uintptr_t)(p4), (uintptr_t)(p5), (uintptr_t)(p6));      \
  }

#define APPL_TRACE_EVENT0(m)                                            \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_EVENT)                       \
      LogMsg_0(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_EVENT,                                    \
               (m));                                                    \
  }
#define APPL_TRACE_EVENT1(m, p1)                                        \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_EVENT)                       \
      LogMsg_1(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_EVENT,                                    \
               (m), (uintptr_t)(p1));                                   \
  }
#define APPL_TRACE_EVENT2(m, p1, p2)                                    \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_EVENT)                       \
      LogMsg_2(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_EVENT,                                    \
               (m), (uintptr_t)(p1), (uintptr_t)(p2));                  \
  }
#define APPL_TRACE_EVENT3(m, p1, p2, p3)                                \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_EVENT)                       \
      LogMsg_3(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_EVENT,                                    \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3)); \
  }
#define APPL_TRACE_EVENT4(m, p1, p2, p3, p4)                            \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_EVENT)                       \
      LogMsg_4(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_EVENT,                                    \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3),  \
               (uintptr_t)(p4));                                        \
  }
#define APPL_TRACE_EVENT5(m, p1, p2, p3, p4, p5)                        \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_EVENT)                       \
      LogMsg_5(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_EVENT,                                    \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3),  \
               (uintptr_t)(p4), (uintptr_t)(p5));                       \
  }
#define APPL_TRACE_EVENT6(m, p1, p2, p3, p4, p5, p6)                    \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_EVENT)                       \
      LogMsg_6(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_EVENT,                                    \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3),  \
               (uintptr_t)(p4), (uintptr_t)(p5), (uintptr_t)(p6));      \
  }

#define APPL_TRACE_DEBUG0(m)                                            \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_DEBUG)                       \
      LogMsg_0(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_DEBUG,                                    \
               (m));                                                    \
  }
#define APPL_TRACE_DEBUG1(m, p1)                                        \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_DEBUG)                       \
      LogMsg_1(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_DEBUG,                                    \
               (m), (uintptr_t)(p1));                                   \
  }
#define APPL_TRACE_DEBUG2(m, p1, p2)                                    \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_DEBUG)                       \
      LogMsg_2(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_DEBUG,                                    \
               (m), (uintptr_t)(p1), (uintptr_t)(p2));                  \
  }
#define APPL_TRACE_DEBUG3(m, p1, p2, p3)                                \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_DEBUG)                       \
      LogMsg_3(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_DEBUG,                                    \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3)); \
  }
#define APPL_TRACE_DEBUG4(m, p1, p2, p3, p4)                            \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_DEBUG)                       \
      LogMsg_4(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_DEBUG,                                    \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3),  \
               (uintptr_t)(p4));                                        \
  }
#define APPL_TRACE_DEBUG5(m, p1, p2, p3, p4, p5)                        \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_DEBUG)                       \
      LogMsg_5(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_DEBUG,                                    \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3),  \
               (uintptr_t)(p4), (uintptr_t)(p5));                       \
  }
#define APPL_TRACE_DEBUG6(m, p1, p2, p3, p4, p5, p6)                    \
  {                                                                     \
    if (appl_trace_level >= BT_TRACE_LEVEL_DEBUG)                       \
      LogMsg_6(TRACE_CTRL_GENERAL | TRACE_LAYER_NONE | TRACE_ORG_APPL | \
                   TRACE_TYPE_DEBUG,                                    \
               (m), (uintptr_t)(p1), (uintptr_t)(p2), (uintptr_t)(p3),  \
               (uintptr_t)(p4), (uintptr_t)(p5), (uintptr_t)(p6));      \
  }

#endif /* BT_TRACE_H */
