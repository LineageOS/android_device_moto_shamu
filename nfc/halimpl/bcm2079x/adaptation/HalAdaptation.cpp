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

/******************************************************************************
 *
 *  HAL Adaptation Interface (HAI). This interface regulates the interaction
 *  between standard Android HAL and Broadcom-specific HAL.  It adapts
 *  Broadcom-specific features to the Android framework.
 *
 ******************************************************************************/
#define LOG_TAG "NfcNciHal"
#include "HalAdaptation.h"
#include <cutils/properties.h>
#include <errno.h>
#include <pthread.h>
#include "SyncEvent.h"
#include "_OverrideLog.h"
#include "android_logmsg.h"
#include "buildcfg.h"
#include "config.h"
#include "nfc_hal_int.h"
#include "nfc_hal_post_reset.h"
extern void delete_hal_non_volatile_store(bool forceDelete);
extern void verify_hal_non_volatile_store();
extern void resetConfig();
extern "C" {
#include "userial.h"
}

extern void configureCrystalFrequency();

///////////////////////////////////////
// private declaration, definition

static nfc_stack_callback_t* gAndroidHalCallback = NULL;
static nfc_stack_data_callback_t* gAndroidHalDataCallback = NULL;
static SyncEvent gOpenCompletedEvent;
static SyncEvent gPostInitCompletedEvent;
static SyncEvent gCloseCompletedEvent;

uint32_t ScrProtocolTraceFlag = SCR_PROTO_TRACE_ALL;  // 0x017F00;

static void BroadcomHalCallback(uint8_t event, tHAL_NFC_STATUS status);
static void BroadcomHalDataCallback(uint16_t data_len, uint8_t* p_data);

static bool isColdBoot = true;

extern tNFC_HAL_CFG* p_nfc_hal_cfg;
extern const uint8_t nfca_version_string[];
extern const uint8_t nfa_version_string[];

tNFC_HAL_DM_PRE_SET_MEM nfc_hal_pre_set_mem_20795a1[] = {
    {0x0016403c, 0x00000008},
    {0x0016403c, 0x00000000},
    {0x0014008c, 0x00000001},
    {0, 0}};

extern tNFC_HAL_DM_PRE_SET_MEM* p_nfc_hal_dm_pre_set_mem;

///////////////////////////////////////

int HaiInitializeLibrary(const bcm2079x_dev_t* device) {
  ALOGD("%s: enter", __func__);
  ALOGE("%s: ver=%s nfa=%s", __func__, nfca_version_string, nfa_version_string);
  int retval = EACCES;
  unsigned long freq = 0;
  unsigned long num = 0;
  char temp[120];
  int8_t prop_value;
  uint8_t logLevel = 0;

  logLevel = InitializeGlobalAppLogLevel();

  if (GetNumValue(NAME_GLOBAL_RESET, &num, sizeof(num))) {
    if (num == 1) {
      // Send commands to disable boc
      p_nfc_hal_dm_pre_set_mem = nfc_hal_pre_set_mem_20795a1;
    }
  }

  configureCrystalFrequency();
  verify_hal_non_volatile_store();
  if (GetNumValue(NAME_PRESERVE_STORAGE, (char*)&num, sizeof(num)) &&
      (num == 1))
    ALOGD("%s: preserve HAL NV store", __func__);
  else {
    delete_hal_non_volatile_store(false);
  }

  if (GetNumValue(NAME_USE_RAW_NCI_TRACE, &num, sizeof(num))) {
    if (num == 1) {
      // display protocol traces in raw format
      ProtoDispAdapterUseRawOutput(TRUE);
    }
  }

  // Initialize protocol logging level
  InitializeProtocolLogLevel();

  tUSERIAL_OPEN_CFG cfg;
  struct tUART_CONFIG uart;

  if (GetStrValue(NAME_UART_PARITY, temp, sizeof(temp))) {
    if (strcmp(temp, "even") == 0)
      uart.m_iParity = USERIAL_PARITY_EVEN;
    else if (strcmp(temp, "odd") == 0)
      uart.m_iParity = USERIAL_PARITY_ODD;
    else if (strcmp(temp, "none") == 0)
      uart.m_iParity = USERIAL_PARITY_NONE;
  } else
    uart.m_iParity = USERIAL_PARITY_NONE;

  if (GetStrValue(NAME_UART_STOPBITS, temp, sizeof(temp))) {
    if (strcmp(temp, "1") == 0)
      uart.m_iStopbits = USERIAL_STOPBITS_1;
    else if (strcmp(temp, "2") == 0)
      uart.m_iStopbits = USERIAL_STOPBITS_2;
    else if (strcmp(temp, "1.5") == 0)
      uart.m_iStopbits = USERIAL_STOPBITS_1_5;
  } else if (GetNumValue(NAME_UART_STOPBITS, &num, sizeof(num))) {
    if (num == 1)
      uart.m_iStopbits = USERIAL_STOPBITS_1;
    else if (num == 2)
      uart.m_iStopbits = USERIAL_STOPBITS_2;
  } else
    uart.m_iStopbits = USERIAL_STOPBITS_1;

  if (GetNumValue(NAME_UART_DATABITS, &num, sizeof(num))) {
    if (5 <= num && num <= 8) uart.m_iDatabits = (1 << (num + 1));
  } else
    uart.m_iDatabits = USERIAL_DATABITS_8;

  if (GetNumValue(NAME_UART_BAUD, &num, sizeof(num))) {
    if (num == 300)
      uart.m_iBaudrate = USERIAL_BAUD_300;
    else if (num == 600)
      uart.m_iBaudrate = USERIAL_BAUD_600;
    else if (num == 1200)
      uart.m_iBaudrate = USERIAL_BAUD_1200;
    else if (num == 2400)
      uart.m_iBaudrate = USERIAL_BAUD_2400;
    else if (num == 9600)
      uart.m_iBaudrate = USERIAL_BAUD_9600;
    else if (num == 19200)
      uart.m_iBaudrate = USERIAL_BAUD_19200;
    else if (num == 57600)
      uart.m_iBaudrate = USERIAL_BAUD_57600;
    else if (num == 115200)
      uart.m_iBaudrate = USERIAL_BAUD_115200;
    else if (num == 230400)
      uart.m_iBaudrate = USERIAL_BAUD_230400;
    else if (num == 460800)
      uart.m_iBaudrate = USERIAL_BAUD_460800;
    else if (num == 921600)
      uart.m_iBaudrate = USERIAL_BAUD_921600;
  } else if (GetStrValue(NAME_UART_BAUD, temp, sizeof(temp))) {
    if (strcmp(temp, "auto") == 0) uart.m_iBaudrate = USERIAL_BAUD_AUTO;
  } else
    uart.m_iBaudrate = USERIAL_BAUD_115200;

  memset(&cfg, 0, sizeof(tUSERIAL_OPEN_CFG));
  cfg.fmt = uart.m_iDatabits | uart.m_iParity | uart.m_iStopbits;
  cfg.baud = uart.m_iBaudrate;

  ALOGD("%s: uart config=0x%04x, %d\n", __func__, cfg.fmt, cfg.baud);
  USERIAL_Init(&cfg);

  if (GetNumValue(NAME_NFCC_ENABLE_TIMEOUT, &num, sizeof(num))) {
    p_nfc_hal_cfg->nfc_hal_nfcc_enable_timeout = num;
  }

  if (GetNumValue(NAME_NFA_MAX_EE_SUPPORTED, &num, sizeof(num)) && num == 0) {
    // Since NFA_MAX_EE_SUPPORTED is explicetly set to 0, no UICC support is
    // needed.
    p_nfc_hal_cfg->nfc_hal_hci_uicc_support = 0;
  }

  prop_value = property_get_bool("nfc.bcm2079x.isColdboot", 0);
  if (prop_value) {
    isColdBoot = true;
    property_set("nfc.bcm2079x.isColdboot", "0");
  }
  // Set 'first boot' flag based on static variable that will get set to false
  // after the stack has first initialized the EE.
  p_nfc_hal_cfg->nfc_hal_first_boot = isColdBoot ? TRUE : FALSE;

  HAL_NfcInitialize();
  HAL_NfcSetTraceLevel(logLevel);  // Initialize HAL's logging level

  retval = 0;
  ALOGD("%s: exit %d", __func__, retval);
  return retval;
}

int HaiTerminateLibrary() {
  int retval = EACCES;
  ALOGD("%s: enter", __func__);

  HAL_NfcTerminate();
  gAndroidHalCallback = NULL;
  gAndroidHalDataCallback = NULL;
  GKI_shutdown();
  resetConfig();
  retval = 0;
  ALOGD("%s: exit %d", __func__, retval);
  return retval;
}

int HaiOpen(const bcm2079x_dev_t* device, nfc_stack_callback_t* halCallbackFunc,
            nfc_stack_data_callback_t* halDataCallbackFunc) {
  ALOGD("%s: enter", __func__);
  int retval = EACCES;

  gAndroidHalCallback = halCallbackFunc;
  gAndroidHalDataCallback = halDataCallbackFunc;

  SyncEventGuard guard(gOpenCompletedEvent);
  HAL_NfcOpen(BroadcomHalCallback, BroadcomHalDataCallback);
  gOpenCompletedEvent.wait();

  retval = 0;
  ALOGD("%s: exit %d", __func__, retval);
  return retval;
}

void BroadcomHalCallback(uint8_t event, tHAL_NFC_STATUS status) {
  ALOGD("%s: enter; event=0x%X", __func__, event);
  switch (event) {
    case HAL_NFC_OPEN_CPLT_EVT: {
      ALOGD("%s: HAL_NFC_OPEN_CPLT_EVT; status=0x%X", __func__, status);
      SyncEventGuard guard(gOpenCompletedEvent);
      gOpenCompletedEvent.notifyOne();
      break;
    }

    case HAL_NFC_POST_INIT_CPLT_EVT: {
      ALOGD("%s: HAL_NFC_POST_INIT_CPLT_EVT", __func__);
      SyncEventGuard guard(gPostInitCompletedEvent);
      gPostInitCompletedEvent.notifyOne();
      break;
    }

    case HAL_NFC_CLOSE_CPLT_EVT: {
      ALOGD("%s: HAL_NFC_CLOSE_CPLT_EVT", __func__);
      SyncEventGuard guard(gCloseCompletedEvent);
      gCloseCompletedEvent.notifyOne();
      break;
    }

    case HAL_NFC_ERROR_EVT: {
      ALOGD("%s: HAL_NFC_ERROR_EVT", __func__);
      {
        SyncEventGuard guard(gOpenCompletedEvent);
        gOpenCompletedEvent.notifyOne();
      }
      {
        SyncEventGuard guard(gPostInitCompletedEvent);
        gPostInitCompletedEvent.notifyOne();
      }
      {
        SyncEventGuard guard(gCloseCompletedEvent);
        gCloseCompletedEvent.notifyOne();
      }
      break;
    }
  }
  gAndroidHalCallback(event, status);
  ALOGD("%s: exit; event=0x%X", __func__, event);
}

void BroadcomHalDataCallback(uint16_t data_len, uint8_t* p_data) {
  ALOGD("%s: enter; len=%u", __func__, data_len);
  gAndroidHalDataCallback(data_len, p_data);
}

int HaiClose(const bcm2079x_dev_t* device) {
  ALOGD("%s: enter", __func__);
  int retval = EACCES;

  SyncEventGuard guard(gCloseCompletedEvent);
  HAL_NfcClose();
  gCloseCompletedEvent.wait();
  retval = 0;
  ALOGD("%s: exit %d", __func__, retval);
  return retval;
}

int HaiCoreInitialized(const bcm2079x_dev_t* device,
                       uint8_t* coreInitResponseParams) {
  ALOGD("%s: enter", __func__);
  int retval = EACCES;

  SyncEventGuard guard(gPostInitCompletedEvent);
  HAL_NfcCoreInitialized(0, coreInitResponseParams);
  gPostInitCompletedEvent.wait();
  retval = 0;
  ALOGD("%s: exit %d", __func__, retval);
  return retval;
}

int HaiWrite(const bcm2079x_dev_t* dev, uint16_t dataLen, const uint8_t* data) {
  ALOGD("%s: enter; len=%u", __func__, dataLen);
  int retval = EACCES;

  HAL_NfcWrite(dataLen, const_cast<uint8_t*>(data));
  retval = 0;
  ALOGD("%s: exit %d", __func__, retval);
  return retval;
}

int HaiPreDiscover(const bcm2079x_dev_t* device) {
  ALOGD("%s: enter", __func__);
  int retval = EACCES;

  // This function is a clear indication that the stack is initializing
  // EE.  So we can reset the cold-boot flag here.
  isColdBoot = false;
  retval = HAL_NfcPreDiscover() ? 1 : 0;
  ALOGD("%s: exit %d", __func__, retval);
  return retval;
}

int HaiControlGranted(const bcm2079x_dev_t* device) {
  ALOGD("%s: enter", __func__);
  int retval = EACCES;

  HAL_NfcControlGranted();
  retval = 0;
  ALOGD("%s: exit %d", __func__, retval);
  return retval;
}

int HaiPowerCycle(const bcm2079x_dev_t* device) {
  ALOGD("%s: enter", __func__);
  int retval = EACCES;

  HAL_NfcPowerCycle();
  retval = 0;
  ALOGD("%s: exit %d", __func__, retval);
  return retval;
}

int HaiGetMaxNfcee(const bcm2079x_dev_t* device, uint8_t* maxNfcee) {
  ALOGD("%s: enter", __func__);
  int retval = EACCES;

  // This function is a clear indication that the stack is initializing
  // EE.  So we can reset the cold-boot flag here.
  isColdBoot = false;

  if (maxNfcee) {
    *maxNfcee = HAL_NfcGetMaxNfcee();
    ALOGD("%s: max_ee from HAL to use %d", __func__, *maxNfcee);
    retval = 0;
  }
  ALOGD("%s: exit %d", __func__, retval);
  return retval;
}
