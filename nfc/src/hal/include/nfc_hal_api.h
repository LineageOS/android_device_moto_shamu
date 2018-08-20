/******************************************************************************
 *
 *  Copyright (C) 2012-2014 Broadcom Corporation
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
 *  NFC Hardware Abstraction Layer API
 *
 ******************************************************************************/
#ifndef NFC_HAL_API_H
#define NFC_HAL_API_H
#include <hardware/nfc.h>
#include "data_types.h"
#include "nfc_hal_target.h"

/*******************************************************************************
** tHAL_HCI_NETWK_CMD Definitions
*******************************************************************************/
#define HAL_NFC_HCI_NO_UICC_HOST 0x00
#define HAL_NFC_HCI_UICC0_HOST 0x01
#define HAL_NFC_HCI_UICC1_HOST 0x02
#define HAL_NFC_HCI_UICC2_HOST 0x04

typedef uint8_t tHAL_NFC_STATUS;
typedef void(tHAL_NFC_STATUS_CBACK)(tHAL_NFC_STATUS status);
typedef void(tHAL_NFC_CBACK)(uint8_t event, tHAL_NFC_STATUS status);
typedef void(tHAL_NFC_DATA_CBACK)(uint16_t data_len, uint8_t* p_data);

/*******************************************************************************
** tHAL_NFC_ENTRY HAL entry-point lookup table
*******************************************************************************/

typedef void(tHAL_API_INITIALIZE)(void);
typedef void(tHAL_API_TERMINATE)(void);
typedef void(tHAL_API_OPEN)(tHAL_NFC_CBACK* p_hal_cback,
                            tHAL_NFC_DATA_CBACK* p_data_cback);
typedef void(tHAL_API_CLOSE)(void);
typedef void(tHAL_API_CORE_INITIALIZED)(uint16_t data_len,
                                        uint8_t* p_core_init_rsp_params);
typedef void(tHAL_API_WRITE)(uint16_t data_len, uint8_t* p_data);
typedef bool(tHAL_API_PREDISCOVER)(void);
typedef void(tHAL_API_CONTROL_GRANTED)(void);
typedef void(tHAL_API_POWER_CYCLE)(void);
typedef uint8_t(tHAL_API_GET_MAX_NFCEE)(void);

#define NFC_HAL_DM_PRE_SET_MEM_LEN 5
typedef struct {
  uint32_t addr;
  uint32_t data;
} tNFC_HAL_DM_PRE_SET_MEM;

/* data members for NFC_HAL-HCI */
typedef struct {
  bool nfc_hal_prm_nvm_required; /* set nfc_hal_prm_nvm_required to TRUE, if the
                                    platform wants to abort PRM process without
                                    NVM */
  uint16_t nfc_hal_nfcc_enable_timeout; /* max time to wait for RESET NTF after
                                           setting REG_PU to high */
  uint16_t nfc_hal_post_xtal_timeout;   /* max time to wait for RESET NTF after
                                           setting Xtal frequency */
#if (NFC_HAL_HCI_INCLUDED == TRUE)
  bool nfc_hal_first_boot; /* set nfc_hal_first_boot to TRUE, if platform
                              enables NFC for the first time after bootup */
  uint8_t nfc_hal_hci_uicc_support; /* set nfc_hal_hci_uicc_support to Zero, if
                                       no UICC is supported otherwise set
                                       corresponding bit(s) for every supported
                                       UICC(s) */
#endif
} tNFC_HAL_CFG;

typedef struct {
  tHAL_API_INITIALIZE* initialize;
  tHAL_API_TERMINATE* terminate;
  tHAL_API_OPEN* open;
  tHAL_API_CLOSE* close;
  tHAL_API_CORE_INITIALIZED* core_initialized;
  tHAL_API_WRITE* write;
  tHAL_API_PREDISCOVER* prediscover;
  tHAL_API_CONTROL_GRANTED* control_granted;
  tHAL_API_POWER_CYCLE* power_cycle;
  tHAL_API_GET_MAX_NFCEE* get_max_ee;

} tHAL_NFC_ENTRY;

/*******************************************************************************
** HAL API Function Prototypes
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
**
** Function         HAL_NfcInitialize
**
** Description      Called when HAL library is loaded.
**
**                  Initialize GKI and start the HCIT task
**
** Returns          void
**
*******************************************************************************/
void HAL_NfcInitialize(void);

/*******************************************************************************
**
** Function         HAL_NfcTerminate
**
** Description      Called to terminate NFC HAL
**
** Returns          void
**
*******************************************************************************/
void HAL_NfcTerminate(void);

/*******************************************************************************
**
** Function         HAL_NfcOpen
**
** Description      Open transport and intialize the NFCC, and
**                  Register callback for HAL event notifications,
**
**                  HAL_OPEN_CPLT_EVT will notify when operation is complete.
**
** Returns          void
**
*******************************************************************************/
void HAL_NfcOpen(tHAL_NFC_CBACK* p_hal_cback,
                 tHAL_NFC_DATA_CBACK* p_data_cback);

/*******************************************************************************
**
** Function         HAL_NfcClose
**
** Description      Prepare for shutdown. A HAL_CLOSE_CPLT_EVT will be
**                  reported when complete.
**
** Returns          void
**
*******************************************************************************/
void HAL_NfcClose(void);

/*******************************************************************************
**
** Function         HAL_NfcCoreInitialized
**
** Description      Called after the CORE_INIT_RSP is received from the NFCC.
**                  At this time, the HAL can do any chip-specific
**                  configuration, and when finished signal the libnfc-nci with
**                  event HAL_POST_INIT_CPLT_EVT.
**
** Returns          void
**
*******************************************************************************/
void HAL_NfcCoreInitialized(uint16_t data_len, uint8_t* p_core_init_rsp_params);

/*******************************************************************************
**
** Function         HAL_NfcWrite
**
** Description      Send an NCI control message or data packet to the
**                  transport. If an NCI command message exceeds the transport
**                  size, HAL is responsible for fragmenting it, Data packets
**                  must be of the correct size.
**
** Returns          void
**
*******************************************************************************/
void HAL_NfcWrite(uint16_t data_len, uint8_t* p_data);

/*******************************************************************************
**
** Function         HAL_NfcPreDiscover
**
** Description      Perform any vendor-specific pre-discovery actions (if
**                  needed). If any actions were performed TRUE will be
**                  returned, and HAL_PRE_DISCOVER_CPLT_EVT will notify when
**                  actions are completed.
**
** Returns          TRUE if vendor-specific pre-discovery actions initialized
**                  FALSE if no vendor-specific pre-discovery actions are
**                  needed.
**
*******************************************************************************/
bool HAL_NfcPreDiscover(void);

/*******************************************************************************
**
** Function         HAL_NfcControlGranted
**
** Description      Grant control to HAL control for sending NCI commands.
**
**                  Call in response to HAL_REQUEST_CONTROL_EVT.
**
**                  Must only be called when there are no NCI commands pending.
**
**                  HAL_RELEASE_CONTROL_EVT will notify when HAL no longer
**                  needs control of NCI.
**
**
** Returns          void
**
*******************************************************************************/
void HAL_NfcControlGranted(void);

/*******************************************************************************
**
** Function         HAL_NfcPowerCycle
**
** Description      Restart NFCC by power cyle
**
**                  HAL_OPEN_CPLT_EVT will notify when operation is complete.
**
** Returns          void
**
*******************************************************************************/
void HAL_NfcPowerCycle(void);

/*******************************************************************************
**
** Function         HAL_NfcGetMaxNfcee
**
** Description      Retrieve the maximum number of NFCEEs supported by NFCC
**
** Returns          the maximum number of NFCEEs supported by NFCC
**
*******************************************************************************/
uint8_t HAL_NfcGetMaxNfcee(void);

#ifdef __cplusplus
}
#endif

#endif /* NFC_HAL_API_H  */
