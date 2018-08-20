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

#define LOG_TAG "NfcNciHal"

#include "_OverrideLog.h"
#include "config.h"
#include "nfc_hal_int.h"
#include "userial.h"
extern "C" {
#include "nfc_hal_post_reset.h"
}
#include <cutils/properties.h>
#include <inttypes.h>
#include <malloc.h>
#include <string>
#include "StartupConfig.h"
#include "spdhelper.h"

#define FW_PRE_PATCH "FW_PRE_PATCH"
#define FW_PATCH "FW_PATCH"
#define MAX_RF_DATA_CREDITS "MAX_RF_DATA_CREDITS"

#define MAX_BUFFER (512)
static char sPrePatchFn[MAX_BUFFER + 1];
static char sPatchFn[MAX_BUFFER + 1];
static void* sPrmBuf = NULL;
static void* sI2cFixPrmBuf = NULL;

#define CONFIG_MAX_LEN 256
static uint8_t sConfig[CONFIG_MAX_LEN];
static StartupConfig sStartupConfig;
static StartupConfig sLptdConfig;
static StartupConfig sPreDiscoveryConfig;
static StartupConfig sXtalCustomParam;
extern uint8_t* p_nfc_hal_dm_start_up_cfg;  // defined in the HAL
static uint8_t nfa_dm_start_up_vsc_cfg[CONFIG_MAX_LEN];
extern uint8_t* p_nfc_hal_dm_start_up_vsc_cfg;  // defined in the HAL
extern uint8_t* p_nfc_hal_dm_lptd_cfg;          // defined in the HAL
static uint8_t sDontSendLptd[] = {0};
extern uint8_t* p_nfc_hal_pre_discover_cfg;    // defined in the HAL
extern uint8_t* p_nfc_hal_dm_xtal_params_cfg;  // defined in HAL

extern tSNOOZE_MODE_CONFIG gSnoozeModeCfg;
extern tNFC_HAL_CFG* p_nfc_hal_cfg;
static void mayDisableSecureElement(StartupConfig& config);

/* Default patchfile (in NCD format) */
#ifndef NFA_APP_DEFAULT_PATCHFILE_NAME
#define NFA_APP_DEFAULT_PATCHFILE_NAME "\0"
#endif

/* Default patchfile (in NCD format) */
#ifndef NFA_APP_DEFAULT_I2C_PATCHFILE_NAME
#define NFA_APP_DEFAULT_I2C_PATCHFILE_NAME "\0"
#endif

tNFC_POST_RESET_CB nfc_post_reset_cb = {
    /* Default Patch & Pre-Patch */
    NFA_APP_DEFAULT_PATCHFILE_NAME,
    NULL,
    NFA_APP_DEFAULT_I2C_PATCHFILE_NAME,
    NULL,

    /* Default UART baud rate */
    NFC_HAL_DEFAULT_BAUD,

    /* Default tNFC_HAL_DEV_INIT_CFG (flags, num_xtal_cfg, {brcm_hw_id,
       xtal-freq, xtal-index} ) */
    {2, /* number of valid entries */
     {
         {0x43341000, 37400,
          NFC_HAL_XTAL_INDEX_37400},  // All revisions of 43341 use 37,400
         {0x20795000, 26000, NFC_HAL_XTAL_INDEX_26000},
         {0, 0, 0},
         {0, 0, 0},
         {0, 0, 0},
     }},

    /* Default low power mode settings */
    NFC_HAL_LP_SNOOZE_MODE_NONE,    /* Snooze Mode          */
    NFC_HAL_LP_IDLE_THRESHOLD_HOST, /* Idle Threshold Host  */
    NFC_HAL_LP_IDLE_THRESHOLD_HC,   /* Idle Threshold HC    */
    NFC_HAL_LP_ACTIVE_LOW,          /* NFC_WAKE Active Mode */
    NFC_HAL_LP_ACTIVE_HIGH,         /* DH_WAKE Active Mode  */

    NFA_APP_MAX_NUM_REINIT, /* max retry to get NVM type */
    0,                      /* current retry count */
    TRUE,                   /* debug mode for downloading patchram */
    FALSE /* skip downloading patchram after reinit because of patch download
             failure */
};

/*******************************************************************************
**
** Function         getFileLength
**
** Description      return the size of a file
**
** Returns          file size in number of bytes
**
*******************************************************************************/
static long getFileLength(FILE* fp) {
  long sz;
  fseek(fp, 0L, SEEK_END);
  sz = ftell(fp);
  fseek(fp, 0L, SEEK_SET);

  return (sz > 0) ? sz : 0;
}

/*******************************************************************************
**
** Function         isFileExist
**
** Description      Check if file name exists (android does not support fexists)
**
** Returns          TRUE if file exists
**
*******************************************************************************/
static bool isFileExist(const char* pFilename) {
  FILE* pf;

  pf = fopen(pFilename, "r");
  if (pf != NULL) {
    fclose(pf);
    return TRUE;
  }
  return FALSE;
}

/*******************************************************************************
**
** Function         findPatchramFile
**
** Description      Find the patchram file name specified in the .conf
**
** Returns          pointer to the file name
**
*******************************************************************************/
static const char* findPatchramFile(const char* pConfigName, char* pBuffer,
                                    int bufferLen) {
  ALOGD("%s: config=%s", __func__, pConfigName);

  if (pConfigName == NULL) {
    ALOGD("%s No patchfile defined\n", __func__);
    return NULL;
  }

  if (GetStrValue(pConfigName, &pBuffer[0], bufferLen)) {
    ALOGD("%s found patchfile %s\n", __func__, pBuffer);
    return (pBuffer[0] == '\0') ? NULL : pBuffer;
  }

  ALOGD("%s Cannot find patchfile '%s'\n", __func__, pConfigName);
  return NULL;
}

/*******************************************************************************
**
** Function:    continueAfterSetSnoozeMode
**
** Description: Called after Snooze Mode is enabled.
**
** Returns:     none
**
*******************************************************************************/
static void continueAfterSetSnoozeMode(tHAL_NFC_STATUS status) {
  ALOGD("%s: status=%u", __func__, status);
  // let stack download firmware during next initialization
  nfc_post_reset_cb.spd_skip_on_power_cycle = FALSE;
  if (status == NCI_STATUS_OK)
    HAL_NfcPreInitDone(HAL_NFC_STATUS_OK);
  else
    HAL_NfcPreInitDone(HAL_NFC_STATUS_FAILED);
}

/*******************************************************************************
**
** Function:    postDownloadPatchram
**
** Description: Called after patch download
**
** Returns:     none
**
*******************************************************************************/
static void postDownloadPatchram(tHAL_NFC_STATUS status) {
  ALOGD("%s: status=%i", __func__, status);
  GetStrValue(NAME_SNOOZE_MODE_CFG, (char*)&gSnoozeModeCfg,
              sizeof(gSnoozeModeCfg));
  if (status != HAL_NFC_STATUS_OK) {
    ALOGE("%s: Patch download failed", __func__);
    if (status == HAL_NFC_STATUS_REFUSED) {
      SpdHelper::setPatchAsBad();
    } else
      SpdHelper::incErrorCount();

    /* If in SPD Debug mode, fail immediately and obviously */
    if (SpdHelper::isSpdDebug())
      HAL_NfcPreInitDone(HAL_NFC_STATUS_FAILED);
    else {
      /* otherwise, power cycle the chip and let the stack startup normally */
      ALOGD("%s: re-init; don't download firmware", __func__);
      // stop stack from downloading firmware during next initialization
      nfc_post_reset_cb.spd_skip_on_power_cycle = TRUE;
      USERIAL_PowerupDevice(0);
      HAL_NfcReInit();
    }
  }
  /* Set snooze mode here */
  else if (gSnoozeModeCfg.snooze_mode != NFC_HAL_LP_SNOOZE_MODE_NONE) {
    status = HAL_NfcSetSnoozeMode(
        gSnoozeModeCfg.snooze_mode, gSnoozeModeCfg.idle_threshold_dh,
        gSnoozeModeCfg.idle_threshold_nfcc, gSnoozeModeCfg.nfc_wake_active_mode,
        gSnoozeModeCfg.dh_wake_active_mode, continueAfterSetSnoozeMode);
    if (status != NCI_STATUS_OK) {
      ALOGE("%s: Setting snooze mode failed, status=%i", __func__, status);
      HAL_NfcPreInitDone(HAL_NFC_STATUS_FAILED);
    }
  } else {
    ALOGD("%s: Not using Snooze Mode", __func__);
    HAL_NfcPreInitDone(HAL_NFC_STATUS_OK);
  }
}

/*******************************************************************************
**
** Function:    prmCallback
**
** Description: Patchram callback (for static patchram mode)
**
** Returns:     none
**
*******************************************************************************/
void prmCallback(uint8_t event) {
  ALOGD("%s: event=0x%x", __func__, event);
  switch (event) {
    case NFC_HAL_PRM_CONTINUE_EVT:
      /* This event does not occur if static patchram buf is used */
      break;

    case NFC_HAL_PRM_COMPLETE_EVT:
      postDownloadPatchram(HAL_NFC_STATUS_OK);
      break;

    case NFC_HAL_PRM_ABORT_EVT:
      postDownloadPatchram(HAL_NFC_STATUS_FAILED);
      break;

    case NFC_HAL_PRM_ABORT_INVALID_PATCH_EVT:
      ALOGD("%s: invalid patch...skipping patch download", __func__);
      postDownloadPatchram(HAL_NFC_STATUS_REFUSED);
      break;

    case NFC_HAL_PRM_ABORT_BAD_SIGNATURE_EVT:
      ALOGD("%s: patch authentication failed", __func__);
      postDownloadPatchram(HAL_NFC_STATUS_REFUSED);
      break;

    case NFC_HAL_PRM_ABORT_NO_NVM_EVT:
      ALOGD("%s: No NVM detected", __func__);
      HAL_NfcPreInitDone(HAL_NFC_STATUS_FAILED);
      break;

    default:
      ALOGD("%s: not handled event=0x%x", __func__, event);
      break;
  }
}

/*******************************************************************************
**
** Function         getNfaValues
**
** Description      Get configuration values needed by NFA layer
**
** Returns:         None
**
*******************************************************************************/
static void getNfaValues(uint32_t chipid) {
  unsigned long num = 0;
  int actualLen = 0;

  sStartupConfig.initialize();
  sLptdConfig.initialize();
  sPreDiscoveryConfig.initialize();

  actualLen =
      GetStrValue(NAME_NFA_DM_START_UP_CFG, (char*)sConfig, sizeof(sConfig));
  if (actualLen) sStartupConfig.append(sConfig, actualLen);

  // Set antenna tuning configuration if configured.
  actualLen =
      GetStrValue(NAME_PREINIT_DSP_CFG, (char*)sConfig, sizeof(sConfig));
  if (actualLen) sStartupConfig.append(sConfig, actualLen);

  if (GetStrValue(NAME_NFA_DM_START_UP_VSC_CFG, (char*)nfa_dm_start_up_vsc_cfg,
                  sizeof(nfa_dm_start_up_vsc_cfg))) {
    p_nfc_hal_dm_start_up_vsc_cfg = &nfa_dm_start_up_vsc_cfg[0];
    ALOGD("START_UP_VSC_CFG[0] = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
          nfa_dm_start_up_vsc_cfg[0], nfa_dm_start_up_vsc_cfg[1],
          nfa_dm_start_up_vsc_cfg[2], nfa_dm_start_up_vsc_cfg[3],
          nfa_dm_start_up_vsc_cfg[4], nfa_dm_start_up_vsc_cfg[5],
          nfa_dm_start_up_vsc_cfg[6], nfa_dm_start_up_vsc_cfg[7]);
  }

  actualLen = GetStrValue(NAME_LPTD_CFG, (char*)sConfig, sizeof(sConfig));
  if (actualLen) {
    sLptdConfig.append(sConfig, actualLen);
    p_nfc_hal_dm_lptd_cfg =
        const_cast<uint8_t*>(sLptdConfig.getInternalBuffer());
  } else {
    // Default to not sending any LPTD setting.
    p_nfc_hal_dm_lptd_cfg = sDontSendLptd;
  }

  mayDisableSecureElement(sStartupConfig);
  p_nfc_hal_dm_start_up_cfg =
      const_cast<uint8_t*>(sStartupConfig.getInternalBuffer());

  actualLen = GetStrValue(NAME_NFA_DM_PRE_DISCOVERY_CFG, (char*)sConfig,
                          sizeof(sConfig));
  if (actualLen) {
    sPreDiscoveryConfig.append(sConfig, actualLen);
    mayDisableSecureElement(sPreDiscoveryConfig);
    p_nfc_hal_pre_discover_cfg =
        const_cast<uint8_t*>(sPreDiscoveryConfig.getInternalBuffer());
  }

  // configure how many secure elements are available for each type of chip
  if (p_nfc_hal_cfg->nfc_hal_hci_uicc_support > 0) {
    if ((chipid & BRCM_NFC_GEN_MASK) == BRCM_NFC_20791_GEN) {
      nfc_hal_cb.max_ee = BRCM_NFC_20791_GEN_MAX_EE;
      p_nfc_hal_cfg->nfc_hal_hci_uicc_support =
          HAL_NFC_HCI_UICC0_HOST | HAL_NFC_HCI_UICC1_HOST;
    } else if ((chipid & BRCM_NFC_GEN_MASK) == BRCM_NFC_43341_GEN) {
      nfc_hal_cb.max_ee = BRCM_NFC_43341_GEN_MAX_EE;
      p_nfc_hal_cfg->nfc_hal_hci_uicc_support =
          HAL_NFC_HCI_UICC0_HOST | HAL_NFC_HCI_UICC1_HOST;
    } else if ((chipid & BRCM_NFC_GEN_MASK) == BRCM_NFC_20795_GEN) {
      nfc_hal_cb.max_ee = BRCM_NFC_20795_GEN_MAX_EE;
      p_nfc_hal_cfg->nfc_hal_hci_uicc_support = HAL_NFC_HCI_UICC0_HOST |
                                                HAL_NFC_HCI_UICC1_HOST |
                                                HAL_NFC_HCI_UICC2_HOST;
    }

    // let .conf variable determine how many EE's to discover
    if (GetNumValue(NAME_NFA_MAX_EE_SUPPORTED, &num, sizeof(num)))
      nfc_hal_cb.max_ee = num;
  }
}

/*******************************************************************************
**
** Function         StartPatchDownload
**
** Description      Reads configuration settings, and begins the download
**                  process if patch files are configured.
**
** Returns:         None
**
*******************************************************************************/
static void StartPatchDownload(uint32_t chipid) {
  ALOGD("%s: chipid=%" PRIX32, __func__, chipid);

  char chipID[30];
  sprintf(chipID, "%" PRIx32, chipid);
  ALOGD("%s: chidId=%s", __func__, chipID);

  readOptionalConfig(chipID);  // Read optional chip specific settings
  readOptionalConfig("fime");  // Read optional FIME specific settings
  getNfaValues(chipid);        // Get NFA configuration values into variables

  findPatchramFile(FW_PATCH, sPatchFn, sizeof(sPatchFn));
  findPatchramFile(FW_PRE_PATCH, sPrePatchFn, sizeof(sPatchFn));

  {
    FILE* fd;
    /* If an I2C fix patch file was specified, then tell the stack about it */
    if (sPrePatchFn[0] != '\0') {
      fd = fopen(sPrePatchFn, "rb");
      if (fd != NULL) {
        uint32_t lenPrmBuffer = getFileLength(fd);

        sI2cFixPrmBuf = malloc(lenPrmBuffer);
        if (sI2cFixPrmBuf != NULL) {
          size_t actualLen = fread(sI2cFixPrmBuf, 1, lenPrmBuffer, fd);
          if (actualLen == lenPrmBuffer) {
            ALOGD("%s Setting I2C fix to %s (size: %" PRIu32 ")", __func__,
                  sPrePatchFn, lenPrmBuffer);
            HAL_NfcPrmSetI2cPatch((uint8_t*)sI2cFixPrmBuf,
                                  (uint16_t)lenPrmBuffer, 0);
          } else
            ALOGE(
                "%s fail reading i2c fix; actual len=%zu; expected len="
                "%" PRIu32,
                __func__, actualLen, lenPrmBuffer);
        } else {
          ALOGE("%s Unable to get buffer to i2c fix (%" PRIu32 " bytes)",
                __func__, lenPrmBuffer);
        }

        fclose(fd);
      } else {
        ALOGE("%s Unable to open i2c fix patchfile %s", __func__, sPrePatchFn);
      }
    }
  }

  {
    FILE* fd;

    /* If a patch file was specified, then download it now */
    if (sPatchFn[0] != '\0') {
      uint32_t bDownloadStarted = false;

      /* open patchfile, read it into a buffer */
      fd = fopen(sPatchFn, "rb");
      if (fd != NULL) {
        uint32_t lenPrmBuffer = getFileLength(fd);
        ALOGD("%s Downloading patchfile %s (size: %" PRIu32 ") format=%u",
              __func__, sPatchFn, lenPrmBuffer, NFC_HAL_PRM_FORMAT_NCD);
        sPrmBuf = malloc(lenPrmBuffer);
        if (sPrmBuf != NULL) {
          size_t actualLen = fread(sPrmBuf, 1, lenPrmBuffer, fd);
          if (actualLen == lenPrmBuffer) {
            if (!SpdHelper::isPatchBad((uint8_t*)sPrmBuf, lenPrmBuffer)) {
              /* Download patch using static memeory mode */
              HAL_NfcPrmDownloadStart(NFC_HAL_PRM_FORMAT_NCD, 0,
                                      (uint8_t*)sPrmBuf, lenPrmBuffer, 0,
                                      prmCallback);
              bDownloadStarted = true;
            }
          } else
            ALOGE("%s fail reading patchram", __func__);
        } else
          ALOGE("%s Unable to buffer to hold patchram (%" PRIu32 " bytes)",
                __func__, lenPrmBuffer);

        fclose(fd);
      } else
        ALOGE("%s Unable to open patchfile %s", __func__, sPatchFn);

      /* If the download never got started */
      if (!bDownloadStarted) {
        /* If debug mode, fail in an obvious way, otherwise try to start stack
         */
        postDownloadPatchram(SpdHelper::isSpdDebug() ? HAL_NFC_STATUS_FAILED
                                                     : HAL_NFC_STATUS_OK);
      }
    } else {
      ALOGE(
          "%s: No patchfile specified or disabled. Proceeding to post-download "
          "procedure...",
          __func__);
      postDownloadPatchram(HAL_NFC_STATUS_OK);
    }
  }

  ALOGD("%s: exit", __func__);
}

/*******************************************************************************
**
** Function:    nfc_hal_post_reset_init
**
** Description: Called by the NFC HAL after controller has been reset.
**              Begin to download firmware patch files.
**
** Returns:     none
**
*******************************************************************************/
void nfc_hal_post_reset_init(uint32_t brcm_hw_id, uint8_t nvm_type) {
  ALOGD("%s: brcm_hw_id=0x%" PRIX32 ", nvm_type=%d", __func__, brcm_hw_id,
        nvm_type);
  tHAL_NFC_STATUS stat = HAL_NFC_STATUS_FAILED;
  uint8_t max_credits = 1, allow_no_nvm = 0;

  p_nfc_hal_cfg->nfc_hal_prm_nvm_required =
      TRUE;  // don't download firmware if controller cannot detect EERPOM

  if (nvm_type == NCI_SPD_NVM_TYPE_NONE) {
    GetNumValue(NAME_ALLOW_NO_NVM, &allow_no_nvm, sizeof(allow_no_nvm));
    if (allow_no_nvm == 0) {
      ALOGD("%s: No NVM detected, FAIL the init stage to force a retry",
            __func__);
      USERIAL_PowerupDevice(0);
      stat = HAL_NfcReInit();
      return;
    }

    p_nfc_hal_cfg->nfc_hal_prm_nvm_required =
        FALSE;  // allow download firmware if controller cannot detect EERPOM
  }

  /* Start downloading the patch files */
  StartPatchDownload(brcm_hw_id);

  if (GetNumValue(MAX_RF_DATA_CREDITS, &max_credits, sizeof(max_credits)) &&
      (max_credits > 0)) {
    ALOGD("%s : max_credits=%d", __func__, max_credits);
    HAL_NfcSetMaxRfDataCredits(max_credits);
  }
}

/*******************************************************************************
**
** Function:        mayDisableSecureElement
**
** Description:     Optionally adjust a TLV to disable secure element.  This
*feature
**                  is enabled by setting the system property
**                  nfc.disable_secure_element to a bit mask represented by a
*hex
**                  octet: C0 = do not detect any secure element.
**                         40 = do not detect secure element in slot 0.
**                         80 = do not detect secure element in slot 1.
**
**                  config: a sequence of TLV's.
**
*******************************************************************************/
void mayDisableSecureElement(StartupConfig& config) {
  unsigned int bitmask = 0;
  char valueStr[PROPERTY_VALUE_MAX] = {0};
  int len = property_get("nfc.disable_secure_element", valueStr, "");
  if (len > 0) {
    sscanf(valueStr, "%x", &bitmask);  // read system property as a hex octet
    ALOGD("%s: disable 0x%02X", __func__, (uint8_t)bitmask);
    config.disableSecureElement((uint8_t)(bitmask & 0xC0));
  }
}

/*******************************************************************************
**
** Function:    configureCrystalFrequency
**
** Description: Configure controller's crystal frequency by reading values from
**              .conf file.  If .conf file does not define any value, then use
**              default values defined in struct nfc_post_reset_cb.
**
** Returns:     none
**
*******************************************************************************/
void configureCrystalFrequency() {
  unsigned long num = 0;
  uint32_t hwId = 0;
  uint16_t xtalFreq = 0;
  uint8_t xtalIndex = 0;
  int actualLen = 0;

  if (GetNumValue(NAME_XTAL_HARDWARE_ID, &num, sizeof(num))) hwId = num;

  if (GetNumValue(NAME_XTAL_FREQUENCY, &num, sizeof(num)))
    xtalFreq = (uint16_t)num;

  if (GetNumValue(NAME_XTAL_FREQ_INDEX, &num, sizeof(num)))
    xtalIndex = (uint8_t)num;

  actualLen =
      GetStrValue(NAME_XTAL_PARAMS_CFG, (char*)sConfig, sizeof(sConfig));
  if (actualLen &&
      (xtalIndex ==
       NFC_HAL_XTAL_INDEX_SPECIAL))  // whether to use custom crystal frequency
  {
    sXtalCustomParam.append(sConfig, actualLen);
    p_nfc_hal_dm_xtal_params_cfg =
        const_cast<uint8_t*>(sXtalCustomParam.getInternalBuffer());
  }

  if ((hwId == 0) && (xtalFreq == 0) && (xtalIndex == 0)) return;

  ALOGD("%s: hwId=0x%" PRIX32 "; freq=%u; index=%u", __func__, hwId, xtalFreq,
        xtalIndex);
  nfc_post_reset_cb.dev_init_config.xtal_cfg[0].brcm_hw_id =
      (hwId & BRCM_NFC_GEN_MASK);
  nfc_post_reset_cb.dev_init_config.xtal_cfg[0].xtal_freq = xtalFreq;
  nfc_post_reset_cb.dev_init_config.xtal_cfg[0].xtal_index = xtalIndex;
  nfc_post_reset_cb.dev_init_config.num_xtal_cfg = 1;
}
