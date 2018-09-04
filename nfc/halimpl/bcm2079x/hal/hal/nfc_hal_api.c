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
 *  NFC Hardware Abstraction Layer API: Implementation for Broadcom NFC
 *  controllers
 *
 ******************************************************************************/
#include "nfc_hal_api.h"
#include <string.h>
#include "gki.h"
#include "nfc_hal_int.h"
#include "nfc_hal_target.h"

/*******************************************************************************
** NFC_HAL_TASK declarations
*******************************************************************************/
#define NFC_HAL_TASK_STR ((int8_t*)"NFC_HAL_TASK")
#define NFC_HAL_TASK_STACK_SIZE 0x400
uint32_t nfc_hal_task_stack[(NFC_HAL_TASK_STACK_SIZE + 3) / 4];

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
void HAL_NfcInitialize(void) {
  /* Initialize HAL control block */
  nfc_hal_main_init();

  HAL_TRACE_API1("HAL_NfcInitialize (): NFC_HAL_TASK id=%i", NFC_HAL_TASK);

#ifndef NFC_HAL_SHARED_GKI
  /* Initialize GKI (not needed if using shared NFC/HAL GKI resources) */
  GKI_init();
  GKI_enable();
#endif

  /* Create the NCI transport task */
  GKI_create_task(
      (TASKPTR)nfc_hal_main_task, NFC_HAL_TASK, NFC_HAL_TASK_STR,
      (uint16_t*)((uint8_t*)nfc_hal_task_stack + NFC_HAL_TASK_STACK_SIZE),
      sizeof(nfc_hal_task_stack), NULL, NULL);

#ifndef NFC_HAL_SHARED_GKI
  /* Start GKI scheduler (not needed if using shared NFC/HAL GKI resources) */
  GKI_run(0);
#endif
}

/*******************************************************************************
**
** Function         HAL_NfcTerminate
**
** Description      Called to terminate NFC HAL
**
** Returns          void
**
*******************************************************************************/
void HAL_NfcTerminate(void) { HAL_TRACE_API0("HAL_NfcTerminate ()"); }

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
                 tHAL_NFC_DATA_CBACK* p_data_cback) {
  HAL_TRACE_API0("HAL_NfcOpen ()");

  /* Only handle if HAL is not opened (stack cback is NULL) */
  if (p_hal_cback) {
    nfc_hal_dm_init();
    nfc_hal_cb.p_stack_cback = p_hal_cback;
    nfc_hal_cb.p_data_cback = p_data_cback;

    /* Send startup event to NFC_HAL_TASK */
    GKI_send_event(NFC_HAL_TASK, NFC_HAL_TASK_EVT_INITIALIZE);
  }
}

/*******************************************************************************
**
** Function         HAL_NfcClose
**
** Description      Prepare for shutdown. A HAL_CLOSE_DONE_EVENT will be
**                  reported when complete.
**
** Returns          void
**
*******************************************************************************/
void HAL_NfcClose(void) {
  HAL_TRACE_API0("HAL_NfcClose ()");

  /* Only handle if HAL is opened (stack cback is not-NULL) */
  if (nfc_hal_cb.p_stack_cback) {
    /* Send shutdown event to NFC_HAL_TASK */
    GKI_send_event(NFC_HAL_TASK, NFC_HAL_TASK_EVT_TERMINATE);
  }
}

/*******************************************************************************
**
** Function         HAL_NfcCoreInitialized
**
** Description      Called after the CORE_INIT_RSP is received from the NFCC.
**                  At this time, the HAL can do any chip-specific
*configuration,
**                  and when finished signal the libnfc-nci with event
**                  HAL_POST_INIT_DONE.
**
** Returns          void
**
*******************************************************************************/
void HAL_NfcCoreInitialized(uint16_t data_len,
                            uint8_t* p_core_init_rsp_params) {
  NFC_HDR* p_msg;
  uint16_t size;

  HAL_TRACE_API0("HAL_NfcCoreInitialized ()");

  /* NCI payload len + NCI header size */
  size = p_core_init_rsp_params[2] + NCI_MSG_HDR_SIZE;

  /* Send message to NFC_HAL_TASK */
  p_msg = (NFC_HDR*)GKI_getbuf((uint16_t)(size + NFC_HDR_SIZE));
  if (p_msg != NULL) {
    p_msg->event = NFC_HAL_EVT_POST_CORE_RESET;
    p_msg->offset = 0;
    p_msg->len = size;
    p_msg->layer_specific = 0;
    memcpy((uint8_t*)(p_msg + 1) + p_msg->offset, p_core_init_rsp_params, size);

    GKI_send_msg(NFC_HAL_TASK, NFC_HAL_TASK_MBOX, p_msg);
  }
}

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
void HAL_NfcWrite(uint16_t data_len, uint8_t* p_data) {
  NFC_HDR* p_msg;
  uint8_t mt;

  HAL_TRACE_API0("HAL_NfcWrite ()");

  if (data_len > (NCI_MAX_CTRL_SIZE + NCI_MSG_HDR_SIZE)) {
    HAL_TRACE_ERROR1("HAL_NfcWrite (): too many bytes (%d)", data_len);
    return;
  }

  /* Send message to NFC_HAL_TASK */
  p_msg = (NFC_HDR*)GKI_getpoolbuf(NFC_HAL_NCI_POOL_ID);
  if (p_msg != NULL) {
    p_msg->event = NFC_HAL_EVT_TO_NFC_NCI;
    p_msg->offset = NFC_HAL_NCI_MSG_OFFSET_SIZE;
    p_msg->len = data_len;
    memcpy((uint8_t*)(p_msg + 1) + p_msg->offset, p_data, data_len);

    /* Check if message is a command or data */
    mt = (*(p_data)&NCI_MT_MASK) >> NCI_MT_SHIFT;
    p_msg->layer_specific = (mt == NCI_MT_CMD) ? NFC_HAL_WAIT_RSP_CMD : 0;

    GKI_send_msg(NFC_HAL_TASK, NFC_HAL_TASK_MBOX, p_msg);
  }
}

/*******************************************************************************
**
** Function         HAL_NfcPreDiscover
**
** Description      Perform any vendor-specific pre-discovery actions (if
*needed)
**                  If any actions were performed TRUE will be returned, and
**                  HAL_PRE_DISCOVER_DONE_EVENT will notify when actions are
**                  completed.
**
** Returns          TRUE if vendor-specific pre-discovery actions initialized
**                  FALSE if no vendor-specific pre-discovery actions are
*needed.
**
*******************************************************************************/
bool HAL_NfcPreDiscover(void) {
  bool status = false;

  NFC_HDR* p_msg;

  HAL_TRACE_API0("HAL_NfcPreDiscover ()");
  if (nfc_hal_cb.pre_discover_done == false) {
    nfc_hal_cb.pre_discover_done = true;
    if (p_nfc_hal_pre_discover_cfg && *p_nfc_hal_pre_discover_cfg) {
      status = true;
      /* Send message to NFC_HAL_TASK */
      p_msg = (NFC_HDR*)GKI_getpoolbuf(NFC_HAL_NCI_POOL_ID);
      if (p_msg != NULL) {
        p_msg->event = NFC_HAL_EVT_PRE_DISCOVER;
        GKI_send_msg(NFC_HAL_TASK, NFC_HAL_TASK_MBOX, p_msg);
      }
    }
  }

  HAL_TRACE_API1("HAL_NfcPreDiscover status:%d", status);
  return status;
}

/*******************************************************************************
**
** Function         HAL_NfcControlGranted
**
** Description      Grant control to HAL control for sending NCI commands.
**
**                  Call in response to HAL_REQUEST_CONTROL_EVENT.
**
**                  Must only be called when there are no NCI commands pending.
**
**                  HAL_RELEASE_CONTROL_EVENT will notify when HAL no longer
**                  needs control of NCI.
**
**
** Returns          void
**
*******************************************************************************/
void HAL_NfcControlGranted(void) {
  NFC_HDR* p_msg;
  HAL_TRACE_API0("HAL_NfcControlGranted ()");

  /* Send message to NFC_HAL_TASK */
  p_msg = (NFC_HDR*)GKI_getpoolbuf(NFC_HAL_NCI_POOL_ID);
  if (p_msg != NULL) {
    p_msg->event = NFC_HAL_EVT_CONTROL_GRANTED;
    GKI_send_msg(NFC_HAL_TASK, NFC_HAL_TASK_MBOX, p_msg);
  }
}

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
void HAL_NfcPowerCycle(void) {
  HAL_TRACE_API0("HAL_NfcPowerCycle ()");

  /* Only handle if HAL is opened (stack cback is not-NULL) */
  if (nfc_hal_cb.p_stack_cback) {
    /* Send power cycle event to NFC_HAL_TASK */
    GKI_send_event(NFC_HAL_TASK, NFC_HAL_TASK_EVT_POWER_CYCLE);
  }
}

/*******************************************************************************
**
** Function         HAL_NfcGetMaxNfcee
**
** Description      Retrieve the maximum number of NFCEEs supported by NFCC
**
** Returns          the maximum number of NFCEEs supported by NFCC
**
*******************************************************************************/
uint8_t HAL_NfcGetMaxNfcee(void) {
  HAL_TRACE_API1("HAL_NfcGetMaxNfcee: %d", nfc_hal_cb.max_ee);
  return nfc_hal_cb.max_ee;
}
