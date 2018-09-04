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
 *  Vendor-specific handler for DM events
 *
 ******************************************************************************/
#include <string.h>
#include "nfc_hal_int.h"
#include "nfc_hal_post_reset.h"
#include "upio.h"
#include "userial.h"

/*****************************************************************************
** Constants and types
*****************************************************************************/

#define NFC_HAL_I93_RW_CFG_LEN (5)
#define NFC_HAL_I93_RW_CFG_PARAM_LEN (3)
#define NFC_HAL_I93_AFI (0)
#define NFC_HAL_I93_ENABLE_SMART_POLL (1)

static uint8_t nfc_hal_dm_i93_rw_cfg[NFC_HAL_I93_RW_CFG_LEN] = {
    NCI_PARAM_ID_I93_DATARATE, NFC_HAL_I93_RW_CFG_PARAM_LEN,
    NFC_HAL_I93_FLAG_DATA_RATE,   /* Bit0:Sub carrier, Bit1:Data rate,
                                     Bit4:Enable/Disable AFI */
    NFC_HAL_I93_AFI,              /* AFI if Bit 4 is set in the flag byte */
    NFC_HAL_I93_ENABLE_SMART_POLL /* Bit0:Enable/Disable smart poll */
};

static uint8_t nfc_hal_dm_set_fw_fsm_cmd[NCI_MSG_HDR_SIZE + 1] = {
    NCI_MTS_CMD | NCI_GID_PROP, NCI_MSG_SET_FWFSM, 0x01, 0x00,
};
#define NCI_SET_FWFSM_OFFSET_ENABLE 3

/* length of parameters in XTAL_INDEX CMD */
#define NCI_PROP_PARAM_SIZE_XTAL_INDEX 3
#ifndef NCI_PROP_PARAM_MAX_SIZE_XTAL_INDEX
#define NCI_PROP_PARAM_MAX_SIZE_XTAL_INDEX 20
#endif

const uint8_t nfc_hal_dm_get_build_info_cmd[NCI_MSG_HDR_SIZE] = {
    NCI_MTS_CMD | NCI_GID_PROP, NCI_MSG_GET_BUILD_INFO, 0x00};
#define NCI_BUILD_INFO_OFFSET_HWID 25 /* HW ID offset in build info RSP */

const uint8_t nfc_hal_dm_get_patch_version_cmd[NCI_MSG_HDR_SIZE] = {
    NCI_MTS_CMD | NCI_GID_PROP, NCI_MSG_GET_PATCH_VERSION, 0x00};
/* Length of patch version string in PATCH_INFO */
#define NCI_PATCH_INFO_VERSION_LEN 16

/*****************************************************************************
** Extern function prototypes
*****************************************************************************/
extern uint8_t* p_nfc_hal_dm_lptd_cfg;
extern uint8_t* p_nfc_hal_dm_pll_325_cfg;
extern uint8_t* p_nfc_hal_dm_start_up_cfg;
extern uint8_t* p_nfc_hal_dm_start_up_vsc_cfg;
extern tNFC_HAL_CFG* p_nfc_hal_cfg;
extern tNFC_HAL_DM_PRE_SET_MEM* p_nfc_hal_dm_pre_set_mem;

/*****************************************************************************
** Local function prototypes
*****************************************************************************/

/*******************************************************************************
**
** Function         nfc_hal_dm_set_config
**
** Description      Send NCI config items to NFCC
**
** Returns          tHAL_NFC_STATUS
**
*******************************************************************************/
tHAL_NFC_STATUS nfc_hal_dm_set_config(uint8_t tlv_size, uint8_t* p_param_tlvs,
                                      tNFC_HAL_NCI_CBACK* p_cback) {
  uint8_t *p_buff, *p;
  uint8_t num_param = 0, param_len, rem_len, *p_tlv;
  uint16_t cmd_len = NCI_MSG_HDR_SIZE + tlv_size + 1;
  tHAL_NFC_STATUS status = HAL_NFC_STATUS_FAILED;

  if ((tlv_size == 0) || (p_param_tlvs == NULL)) {
    return status;
  }

  p_buff = (uint8_t*)GKI_getbuf((uint16_t)(NCI_MSG_HDR_SIZE + tlv_size));
  if (p_buff != NULL) {
    p = p_buff;

    NCI_MSG_BLD_HDR0(p, NCI_MT_CMD, NCI_GID_CORE);
    NCI_MSG_BLD_HDR1(p, NCI_MSG_CORE_SET_CONFIG);
    UINT8_TO_STREAM(p, (uint8_t)(tlv_size + 1));

    rem_len = tlv_size;
    p_tlv = p_param_tlvs;
    while (rem_len > 1) {
      num_param++; /* number of params */

      p_tlv++;              /* param type   */
      param_len = *p_tlv++; /* param length */

      rem_len -= 2; /* param type and length */
      if (rem_len >= param_len) {
        rem_len -= param_len;
        p_tlv += param_len; /* next param_type */

        if (rem_len == 0) {
          status = HAL_NFC_STATUS_OK;
          break;
        }
      } else {
        /* error found */
        break;
      }
    }

    if (status == HAL_NFC_STATUS_OK) {
      UINT8_TO_STREAM(p, num_param);
      ARRAY_TO_STREAM(p, p_param_tlvs, tlv_size);

      nfc_hal_dm_send_nci_cmd(p_buff, cmd_len, p_cback);
    } else {
      HAL_TRACE_ERROR0("nfc_hal_dm_set_config ():Bad TLV");
    }

    GKI_freebuf(p_buff);
  }

  return status;
}

/*******************************************************************************
**
** Function         nfc_hal_dm_set_fw_fsm
**
** Description      Enable or disable FW FSM
**
** Returns          void
**
*******************************************************************************/
void nfc_hal_dm_set_fw_fsm(bool enable, tNFC_HAL_NCI_CBACK* p_cback) {
  if (enable)
    nfc_hal_dm_set_fw_fsm_cmd[NCI_SET_FWFSM_OFFSET_ENABLE] =
        0x01; /* Enable, default is disabled */
  else
    nfc_hal_dm_set_fw_fsm_cmd[NCI_SET_FWFSM_OFFSET_ENABLE] = 0x00; /* Disable */

  nfc_hal_dm_send_nci_cmd(nfc_hal_dm_set_fw_fsm_cmd, NCI_MSG_HDR_SIZE + 1,
                          p_cback);
}

/*******************************************************************************
**
** Function         nfc_hal_dm_config_nfcc_cback
**
** Description      Callback for NCI vendor specific command complete
**
** Returns          void
**
*******************************************************************************/
void nfc_hal_dm_config_nfcc_cback(tNFC_HAL_NCI_EVT event, uint16_t data_len,
                                  uint8_t* p_data) {
  if (nfc_hal_cb.dev_cb.next_dm_config == NFC_HAL_DM_CONFIG_NONE) {
    nfc_hal_hci_enable();
  } else {
    nfc_hal_dm_config_nfcc();
  }
}

/*******************************************************************************
**
** Function         nfc_hal_dm_send_startup_vsc
**
** Description      Send VS command before NFA start-up
**
** Returns          None
**
*******************************************************************************/
void nfc_hal_dm_send_startup_vsc(void) {
  uint8_t *p, *p_end;
  uint16_t len;

  HAL_TRACE_DEBUG0("nfc_hal_dm_send_startup_vsc ()");

  /* VSC must have NCI header at least */
  if (nfc_hal_cb.dev_cb.next_startup_vsc + NCI_MSG_HDR_SIZE - 1 <=
      *p_nfc_hal_dm_start_up_vsc_cfg) {
    p = p_nfc_hal_dm_start_up_vsc_cfg + nfc_hal_cb.dev_cb.next_startup_vsc;
    len = *(p + 2);
    p_end = p + NCI_MSG_HDR_SIZE - 1 + len;

    if (p_end <=
        p_nfc_hal_dm_start_up_vsc_cfg + *p_nfc_hal_dm_start_up_vsc_cfg) {
      /* move to next VSC */
      nfc_hal_cb.dev_cb.next_startup_vsc += NCI_MSG_HDR_SIZE + len;

      /* if this is last VSC */
      if (p_end ==
          p_nfc_hal_dm_start_up_vsc_cfg + *p_nfc_hal_dm_start_up_vsc_cfg)
        nfc_hal_cb.dev_cb.next_dm_config = NFC_HAL_DM_CONFIG_NONE;

      nfc_hal_dm_send_nci_cmd(p, (uint16_t)(NCI_MSG_HDR_SIZE + len),
                              nfc_hal_dm_config_nfcc_cback);
      return;
    }
  }

  HAL_TRACE_ERROR0("nfc_hal_dm_send_startup_vsc (): Bad start-up VSC");

  NFC_HAL_SET_INIT_STATE(NFC_HAL_INIT_STATE_IDLE);
  nfc_hal_cb.p_stack_cback(HAL_NFC_POST_INIT_CPLT_EVT, HAL_NFC_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         nfc_hal_dm_config_nfcc
**
** Description      Send VS config before NFA start-up
**
** Returns          void
**
*******************************************************************************/
void nfc_hal_dm_config_nfcc(void) {
  HAL_TRACE_DEBUG1("nfc_hal_dm_config_nfcc (): next_dm_config = %d",
                   nfc_hal_cb.dev_cb.next_dm_config);

  if ((p_nfc_hal_dm_lptd_cfg[0]) &&
      (nfc_hal_cb.dev_cb.next_dm_config <= NFC_HAL_DM_CONFIG_LPTD)) {
    nfc_hal_cb.dev_cb.next_dm_config = NFC_HAL_DM_CONFIG_PLL_325;

    if (nfc_hal_dm_set_config(
            p_nfc_hal_dm_lptd_cfg[0], &p_nfc_hal_dm_lptd_cfg[1],
            nfc_hal_dm_config_nfcc_cback) == HAL_NFC_STATUS_OK) {
      return;
    } else {
      NFC_HAL_SET_INIT_STATE(NFC_HAL_INIT_STATE_IDLE);
      nfc_hal_cb.p_stack_cback(HAL_NFC_POST_INIT_CPLT_EVT,
                               HAL_NFC_STATUS_FAILED);
      return;
    }
  }

  if ((p_nfc_hal_dm_pll_325_cfg) &&
      (nfc_hal_cb.dev_cb.next_dm_config <= NFC_HAL_DM_CONFIG_PLL_325)) {
    nfc_hal_cb.dev_cb.next_dm_config = NFC_HAL_DM_CONFIG_START_UP;

    if (nfc_hal_dm_set_config(
            NFC_HAL_PLL_325_SETCONFIG_PARAM_LEN, p_nfc_hal_dm_pll_325_cfg,
            nfc_hal_dm_config_nfcc_cback) == HAL_NFC_STATUS_OK) {
      return;
    } else {
      NFC_HAL_SET_INIT_STATE(NFC_HAL_INIT_STATE_IDLE);
      nfc_hal_cb.p_stack_cback(HAL_NFC_POST_INIT_CPLT_EVT,
                               HAL_NFC_STATUS_FAILED);
      return;
    }
  }

  if ((p_nfc_hal_dm_start_up_cfg[0]) &&
      (nfc_hal_cb.dev_cb.next_dm_config <= NFC_HAL_DM_CONFIG_START_UP)) {
    nfc_hal_cb.dev_cb.next_dm_config = NFC_HAL_DM_CONFIG_I93_DATA_RATE;
    if (nfc_hal_dm_set_config(
            p_nfc_hal_dm_start_up_cfg[0], &p_nfc_hal_dm_start_up_cfg[1],
            nfc_hal_dm_config_nfcc_cback) == HAL_NFC_STATUS_OK) {
      return;
    } else {
      NFC_HAL_SET_INIT_STATE(NFC_HAL_INIT_STATE_IDLE);
      nfc_hal_cb.p_stack_cback(HAL_NFC_POST_INIT_CPLT_EVT,
                               HAL_NFC_STATUS_FAILED);
      return;
    }
  }

#if (NFC_HAL_I93_FLAG_DATA_RATE == NFC_HAL_I93_FLAG_DATA_RATE_HIGH)
  if (nfc_hal_cb.dev_cb.next_dm_config <= NFC_HAL_DM_CONFIG_I93_DATA_RATE) {
    nfc_hal_cb.dev_cb.next_dm_config = NFC_HAL_DM_CONFIG_FW_FSM;
    if (nfc_hal_dm_set_config(NFC_HAL_I93_RW_CFG_LEN, nfc_hal_dm_i93_rw_cfg,
                              nfc_hal_dm_config_nfcc_cback) ==
        HAL_NFC_STATUS_OK) {
      return;
    } else {
      NFC_HAL_SET_INIT_STATE(NFC_HAL_INIT_STATE_IDLE);
      nfc_hal_cb.p_stack_cback(HAL_NFC_POST_INIT_CPLT_EVT,
                               HAL_NFC_STATUS_FAILED);
      return;
    }
  }
#endif

  /* FW FSM is disabled as default in NFCC */
  if (nfc_hal_cb.dev_cb.next_dm_config <= NFC_HAL_DM_CONFIG_FW_FSM) {
    nfc_hal_cb.dev_cb.next_dm_config = NFC_HAL_DM_CONFIG_START_UP_VSC;
    nfc_hal_dm_set_fw_fsm(NFC_HAL_DM_MULTI_TECH_RESP,
                          nfc_hal_dm_config_nfcc_cback);
    return;
  }

  if (nfc_hal_cb.dev_cb.next_dm_config <= NFC_HAL_DM_CONFIG_START_UP_VSC) {
    if (p_nfc_hal_dm_start_up_vsc_cfg && *p_nfc_hal_dm_start_up_vsc_cfg) {
      nfc_hal_dm_send_startup_vsc();
      return;
    }
  }

  /* nothing to config */
  nfc_hal_cb.dev_cb.next_dm_config = NFC_HAL_DM_CONFIG_NONE;
  nfc_hal_dm_config_nfcc_cback(0, 0, NULL);
}

/*******************************************************************************
**
** Function:    nfc_hal_dm_get_xtal_index
**
** Description: Return Xtal index and frequency
**
** Returns:     tNFC_HAL_XTAL_INDEX
**
*******************************************************************************/
tNFC_HAL_XTAL_INDEX nfc_hal_dm_get_xtal_index(uint32_t brcm_hw_id,
                                              uint16_t* p_xtal_freq) {
  uint8_t xx;

  HAL_TRACE_DEBUG1("nfc_hal_dm_get_xtal_index() brcm_hw_id:0x%x", brcm_hw_id);

  for (xx = 0; xx < nfc_post_reset_cb.dev_init_config.num_xtal_cfg; xx++) {
    if ((brcm_hw_id & BRCM_NFC_GEN_MASK) ==
        nfc_post_reset_cb.dev_init_config.xtal_cfg[xx].brcm_hw_id) {
      *p_xtal_freq = nfc_post_reset_cb.dev_init_config.xtal_cfg[xx].xtal_freq;
      return (nfc_post_reset_cb.dev_init_config.xtal_cfg[xx].xtal_index);
    }
  }

  /* if not found */
  *p_xtal_freq = 0;
  return (NFC_HAL_XTAL_INDEX_MAX);
}

/*******************************************************************************
**
** Function         nfc_hal_dm_set_xtal_freq_index
**
** Description      Set crystal frequency index
**
** Returns          void
**
*******************************************************************************/
void nfc_hal_dm_set_xtal_freq_index(void) {
  uint8_t nci_brcm_xtal_index_cmd[NCI_MSG_HDR_SIZE +
                                  NCI_PROP_PARAM_MAX_SIZE_XTAL_INDEX];
  uint8_t* p;
  tNFC_HAL_XTAL_INDEX xtal_index;
  uint16_t xtal_freq;
  uint8_t cmd_len = NCI_PROP_PARAM_SIZE_XTAL_INDEX;
  extern uint8_t* p_nfc_hal_dm_xtal_params_cfg;

  HAL_TRACE_DEBUG1("nfc_hal_dm_set_xtal_freq_index (): brcm_hw_id = 0x%x",
                   nfc_hal_cb.dev_cb.brcm_hw_id);

  xtal_index =
      nfc_hal_dm_get_xtal_index(nfc_hal_cb.dev_cb.brcm_hw_id, &xtal_freq);
  if ((xtal_index == NFC_HAL_XTAL_INDEX_SPECIAL) &&
      (p_nfc_hal_dm_xtal_params_cfg)) {
    cmd_len +=
        p_nfc_hal_dm_xtal_params_cfg[0]; /* [0] is the length of extra params */
  }

  p = nci_brcm_xtal_index_cmd;
  UINT8_TO_STREAM(p, (NCI_MTS_CMD | NCI_GID_PROP));
  UINT8_TO_STREAM(p, NCI_MSG_GET_XTAL_INDEX_FROM_DH);
  UINT8_TO_STREAM(p, cmd_len);
  UINT8_TO_STREAM(p, xtal_index);
  UINT16_TO_STREAM(p, xtal_freq);
  if (cmd_len > NCI_PROP_PARAM_SIZE_XTAL_INDEX) {
    memcpy(p, &p_nfc_hal_dm_xtal_params_cfg[1],
           p_nfc_hal_dm_xtal_params_cfg[0]);
  }

  NFC_HAL_SET_INIT_STATE(NFC_HAL_INIT_STATE_W4_XTAL_SET);

  nfc_hal_dm_send_nci_cmd(nci_brcm_xtal_index_cmd, NCI_MSG_HDR_SIZE + cmd_len,
                          NULL);
}

/*******************************************************************************
**
** Function         nfc_hal_dm_set_power_level_zero
**
** Description      set power level to 0
**
** Returns          None
**
*******************************************************************************/
void nfc_hal_dm_set_power_level_zero(void) {
  uint8_t
      nci_brcm_set_pwr_level_cmd[NCI_MSG_HDR_SIZE + NCI_PARAM_LEN_POWER_LEVEL];
  uint8_t* p;
  uint8_t cmd_len = NCI_PARAM_LEN_POWER_LEVEL;

  p = nci_brcm_set_pwr_level_cmd;
  UINT8_TO_STREAM(p, (NCI_MTS_CMD | NCI_GID_PROP));
  UINT8_TO_STREAM(p, NCI_MSG_POWER_LEVEL);
  UINT8_TO_STREAM(p, NCI_PARAM_LEN_POWER_LEVEL);
  memset(p, 0, NCI_PARAM_LEN_POWER_LEVEL);

  nfc_hal_dm_send_nci_cmd(nci_brcm_set_pwr_level_cmd,
                          NCI_MSG_HDR_SIZE + cmd_len,
                          nfc_hal_main_exit_op_done);
}

/*******************************************************************************
**
** Function         nfc_hal_dm_send_get_build_info_cmd
**
** Description      Send NCI_MSG_GET_BUILD_INFO CMD
**
** Returns          void
**
*******************************************************************************/
void nfc_hal_dm_send_get_build_info_cmd(void) {
  NFC_HAL_SET_INIT_STATE(NFC_HAL_INIT_STATE_W4_BUILD_INFO);

  /* get build information to find out HW */
  nfc_hal_dm_send_nci_cmd(nfc_hal_dm_get_build_info_cmd, NCI_MSG_HDR_SIZE,
                          NULL);
}
/*******************************************************************************
**
** Function:    nfc_hal_dm_adjust_hw_id
**
** Description: The hw_id of certain chips are shifted by 8 bits.
**              Adjust the hw_id before processing.
**
** Returns:     Nothing
**
*******************************************************************************/
static uint32_t nfc_hal_dm_adjust_hw_id(uint32_t hw_id) {
  if ((hw_id & 0xF0000000) == 0)
    hw_id <<= 4; /* shift hw_id by 4 bits to align w the format of most chips */
  return hw_id;
}

/*******************************************************************************
**
** Function         nfc_hal_dm_check_xtal
**
** Description      check if need to send xtal command.
**                  If not, proceed to next step get_patch_version.
**
** Returns          void
**
*******************************************************************************/
static void nfc_hal_dm_check_xtal(void) {
  uint16_t xtal_freq;
  tNFC_HAL_XTAL_INDEX xtal_index;

  /* if NFCC needs to set Xtal frequency before getting patch version */
  xtal_index =
      nfc_hal_dm_get_xtal_index(nfc_hal_cb.dev_cb.brcm_hw_id, &xtal_freq);
  if ((xtal_index < NFC_HAL_XTAL_INDEX_MAX) ||
      (xtal_index == NFC_HAL_XTAL_INDEX_SPECIAL)) {
    {
      /* set Xtal index before getting patch version */
      nfc_hal_dm_set_xtal_freq_index();
      return;
    }
  }

  NFC_HAL_SET_INIT_STATE(NFC_HAL_INIT_STATE_W4_PATCH_INFO);

  nfc_hal_dm_send_nci_cmd(nfc_hal_dm_get_patch_version_cmd, NCI_MSG_HDR_SIZE,
                          NULL);
}

/*******************************************************************************
**
** Function         nfc_hal_dm_pre_set_mem_cback
**
** Description      This is pre-set mem complete callback.
**
** Returns          void
**
*******************************************************************************/
static void nfc_hal_dm_pre_set_mem_cback(tNFC_HAL_BTVSC_CPLT* pData) {
  uint8_t status = pData->p_param_buf[0];

  HAL_TRACE_DEBUG1("nfc_hal_dm_pre_set_mem_cback: %d", status);
  /* if it is completed */
  if (status == HCI_SUCCESS) {
    if (!nfc_hal_dm_check_pre_set_mem()) {
      return;
    }
  }
  nfc_hal_dm_check_xtal();
}

/*******************************************************************************
**
** Function         nfc_hal_dm_check_pre_set_mem
**
** Description      Check if need to send the command.
**
** Returns          TRUE if done.
**
*******************************************************************************/
bool nfc_hal_dm_check_pre_set_mem(void) {
  uint8_t cmd[NFC_HAL_BT_HCI_CMD_HDR_SIZE + HCI_BRCM_PRE_SET_MEM_LENGTH];
  uint8_t* p;
  uint32_t addr = 0;

  if (p_nfc_hal_dm_pre_set_mem)
    addr = p_nfc_hal_dm_pre_set_mem[nfc_hal_cb.pre_set_mem_idx].addr;
  HAL_TRACE_DEBUG2("nfc_hal_dm_check_pre_set_mem: %d/0x%x",
                   nfc_hal_cb.pre_set_mem_idx, addr);
  if (addr == 0) {
    return true;
  }
  p = cmd;

  /* Add the command */
  UINT16_TO_STREAM(p, HCI_BRCM_PRE_SET_MEM);
  UINT8_TO_STREAM(p, HCI_BRCM_PRE_SET_MEM_LENGTH);

  UINT8_TO_STREAM(p, HCI_BRCM_PRE_SET_MEM_TYPE);
  UINT32_TO_STREAM(p, addr);
  UINT8_TO_STREAM(p, 0);
  UINT32_TO_STREAM(p,
                   p_nfc_hal_dm_pre_set_mem[nfc_hal_cb.pre_set_mem_idx].data);
  nfc_hal_cb.pre_set_mem_idx++;

  nfc_hal_dm_send_bt_cmd(
      cmd, NFC_HAL_BT_HCI_CMD_HDR_SIZE + HCI_BRCM_PRE_SET_MEM_LENGTH,
      nfc_hal_dm_pre_set_mem_cback);
  return false;
}

/*******************************************************************************
**
** Function         nfc_hal_dm_got_vs_rsp
**
** Description      Received VS RSP. Clean up control block to allow next NCI
*cmd
**
** Returns          void
**
*******************************************************************************/
tNFC_HAL_NCI_CBACK* nfc_hal_dm_got_vs_rsp(void) {
  tNFC_HAL_NCI_CBACK* p_cback = NULL;
  nfc_hal_cb.ncit_cb.nci_wait_rsp = NFC_HAL_WAIT_RSP_NONE;
  p_cback = (tNFC_HAL_NCI_CBACK*)nfc_hal_cb.ncit_cb.p_vsc_cback;
  nfc_hal_cb.ncit_cb.p_vsc_cback = NULL;
  nfc_hal_main_stop_quick_timer(&nfc_hal_cb.ncit_cb.nci_wait_rsp_timer);
  return p_cback;
}

/*******************************************************************************
**
** Function         nfc_hal_dm_proc_msg_during_init
**
** Description      Process NCI message while initializing NFCC
**
** Returns          void
**
*******************************************************************************/
void nfc_hal_dm_proc_msg_during_init(NFC_HDR* p_msg) {
  uint8_t* p;
  uint8_t reset_reason, reset_type;
  uint8_t mt, pbf, gid, op_code;
  uint8_t *p_old, old_gid, old_oid, old_mt;
  uint8_t u8;
  tNFC_HAL_NCI_CBACK* p_cback = NULL;
  uint8_t chipverlen;
  uint8_t chipverstr[NCI_SPD_HEADER_CHIPVER_LEN];
  uint32_t hw_id = 0;

  HAL_TRACE_DEBUG1("nfc_hal_dm_proc_msg_during_init(): init state:%d",
                   nfc_hal_cb.dev_cb.initializing_state);

  p = (uint8_t*)(p_msg + 1) + p_msg->offset;

  NCI_MSG_PRS_HDR0(p, mt, pbf, gid);
  NCI_MSG_PRS_HDR1(p, op_code);

  /* check if waiting for this response */
  if ((nfc_hal_cb.ncit_cb.nci_wait_rsp == NFC_HAL_WAIT_RSP_CMD) ||
      (nfc_hal_cb.ncit_cb.nci_wait_rsp == NFC_HAL_WAIT_RSP_VSC)) {
    if (mt == NCI_MT_RSP) {
      p_old = nfc_hal_cb.ncit_cb.last_hdr;
      NCI_MSG_PRS_HDR0(p_old, old_mt, pbf, old_gid);
      old_oid = ((*p_old) & NCI_OID_MASK);
      /* make sure this is the RSP we are waiting for before updating the
       * command window */
      if ((old_gid == gid) && (old_oid == op_code)) {
        nfc_hal_cb.ncit_cb.nci_wait_rsp = NFC_HAL_WAIT_RSP_NONE;
        p_cback = (tNFC_HAL_NCI_CBACK*)nfc_hal_cb.ncit_cb.p_vsc_cback;
        nfc_hal_cb.ncit_cb.p_vsc_cback = NULL;
        nfc_hal_main_stop_quick_timer(&nfc_hal_cb.ncit_cb.nci_wait_rsp_timer);
      }
    }
  }

  if (gid == NCI_GID_CORE) {
    if (op_code == NCI_MSG_CORE_RESET) {
      if (mt == NCI_MT_NTF) {
        if ((nfc_hal_cb.dev_cb.initializing_state ==
             NFC_HAL_INIT_STATE_W4_NFCC_ENABLE) ||
            (nfc_hal_cb.dev_cb.initializing_state ==
             NFC_HAL_INIT_STATE_POST_XTAL_SET)) {
          /*
          ** Core reset ntf in the following cases;
          ** 1) after power up (raising REG_PU)
          ** 2) after setting xtal index
          ** Start pre-initializing NFCC
          */
          nfc_hal_main_stop_quick_timer(&nfc_hal_cb.timer);
          nfc_hal_dm_pre_init_nfcc();
        } else {
          /* Core reset ntf after post-patch download, Call reset notification
           * callback */
          p++; /* Skip over param len */
          STREAM_TO_UINT8(reset_reason, p);
          STREAM_TO_UINT8(reset_type, p);
          nfc_hal_prm_spd_reset_ntf(reset_reason, reset_type);
        }
      }
    } else if (p_cback) {
      (*p_cback)((tNFC_HAL_NCI_EVT)(op_code), p_msg->len,
                 (uint8_t*)(p_msg + 1) + p_msg->offset);
    }
  } else if (gid == NCI_GID_PROP) /* this is for download patch */
  {
    if (mt == NCI_MT_NTF)
      op_code |= NCI_NTF_BIT;
    else
      op_code |= NCI_RSP_BIT;

    if (nfc_hal_cb.dev_cb.initializing_state ==
        NFC_HAL_INIT_STATE_W4_XTAL_SET) {
      if (op_code == (NCI_RSP_BIT | NCI_MSG_GET_XTAL_INDEX_FROM_DH)) {
        /* start timer in case that NFCC doesn't send RESET NTF after loading
         * patch from NVM */
        NFC_HAL_SET_INIT_STATE(NFC_HAL_INIT_STATE_POST_XTAL_SET);

        nfc_hal_main_start_quick_timer(
            &nfc_hal_cb.timer, NFC_HAL_TTYPE_NFCC_ENABLE,
            ((p_nfc_hal_cfg->nfc_hal_post_xtal_timeout) *
             QUICK_TIMER_TICKS_PER_SEC) /
                1000);
      }
    } else if ((op_code == NFC_VS_GET_BUILD_INFO_EVT) &&
               (nfc_hal_cb.dev_cb.initializing_state ==
                NFC_HAL_INIT_STATE_W4_BUILD_INFO)) {
      p += NCI_BUILD_INFO_OFFSET_HWID;

      STREAM_TO_UINT32(hw_id, p);
      nfc_hal_cb.dev_cb.brcm_hw_id = nfc_hal_dm_adjust_hw_id(hw_id);
      HAL_TRACE_DEBUG2("brcm_hw_id: 0x%x -> 0x%x", hw_id,
                       nfc_hal_cb.dev_cb.brcm_hw_id);

      STREAM_TO_UINT8(chipverlen, p);
      memset(chipverstr, 0, NCI_SPD_HEADER_CHIPVER_LEN);

      STREAM_TO_ARRAY(chipverstr, p, chipverlen);

      /* If chip is not 20791 and 43341, set flag to send the "Disable" VSC */
      if (((nfc_hal_cb.dev_cb.brcm_hw_id & BRCM_NFC_GEN_MASK) !=
           BRCM_NFC_20791_GEN) &&
          ((nfc_hal_cb.dev_cb.brcm_hw_id & BRCM_NFC_GEN_MASK) !=
           BRCM_NFC_43341_GEN)) {
        nfc_hal_cb.hal_flags |= NFC_HAL_FLAGS_NEED_DISABLE_VSC;
      }

      nfc_hal_hci_handle_build_info(chipverlen, chipverstr);
      nfc_hal_cb.pre_set_mem_idx = 0;
      if (!nfc_hal_dm_check_pre_set_mem()) {
        /* pre-set mem started */
        return;
      }
      nfc_hal_dm_check_xtal();
    } else if ((op_code == NFC_VS_GET_PATCH_VERSION_EVT) &&
               (nfc_hal_cb.dev_cb.initializing_state ==
                NFC_HAL_INIT_STATE_W4_PATCH_INFO)) {
      /* Store NVM info to control block */

      /* Skip over rsp len */
      p++;

      /* Get project id */
      STREAM_TO_UINT16(nfc_hal_cb.nvm_cb.project_id, p);

      /* RFU */
      p++;

      /* Get chip version string */
      STREAM_TO_UINT8(u8, p);
      if (u8 > NFC_HAL_PRM_MAX_CHIP_VER_LEN) u8 = NFC_HAL_PRM_MAX_CHIP_VER_LEN;
      memcpy(nfc_hal_cb.nvm_cb.chip_ver, p, u8);
      p += NCI_PATCH_INFO_VERSION_LEN;

      /* Get major/minor version */
      STREAM_TO_UINT16(nfc_hal_cb.nvm_cb.ver_major, p);
      STREAM_TO_UINT16(nfc_hal_cb.nvm_cb.ver_minor, p);

      /* Skip over max_size and patch_max_size */
      p += 4;

      /* Get current lpm patch size */
      STREAM_TO_UINT16(nfc_hal_cb.nvm_cb.lpm_size, p);
      STREAM_TO_UINT16(nfc_hal_cb.nvm_cb.fpm_size, p);

      /* clear all flags which may be set during previous initialization */
      nfc_hal_cb.nvm_cb.flags = 0;

      /* Set patch present flag */
      if ((nfc_hal_cb.nvm_cb.fpm_size) || (nfc_hal_cb.nvm_cb.lpm_size))
        nfc_hal_cb.nvm_cb.flags |= NFC_HAL_NVM_FLAGS_PATCH_PRESENT;

      /* LPMPatchCodeHasBadCRC (if not bad crc, then indicate LPM patch is
       * present in nvm) */
      STREAM_TO_UINT8(u8, p);
      if (u8) {
        /* LPM patch in NVM fails CRC check */
        nfc_hal_cb.nvm_cb.flags |= NFC_HAL_NVM_FLAGS_LPM_BAD;
      }

      /* FPMPatchCodeHasBadCRC (if not bad crc, then indicate LPM patch is
       * present in nvm) */
      STREAM_TO_UINT8(u8, p);
      if (u8) {
        /* FPM patch in NVM fails CRC check */
        nfc_hal_cb.nvm_cb.flags |= NFC_HAL_NVM_FLAGS_FPM_BAD;
      }

      /* Check if downloading patch to RAM only (no NVM) */
      STREAM_TO_UINT8(nfc_hal_cb.nvm_cb.nvm_type, p);
      if (nfc_hal_cb.nvm_cb.nvm_type == NCI_SPD_NVM_TYPE_NONE) {
        nfc_hal_cb.nvm_cb.flags |= NFC_HAL_NVM_FLAGS_NO_NVM;
      }

      /* let platform update baudrate or download patch */
      NFC_HAL_SET_INIT_STATE(NFC_HAL_INIT_STATE_W4_APP_COMPLETE);
      nfc_hal_post_reset_init(nfc_hal_cb.dev_cb.brcm_hw_id,
                              nfc_hal_cb.nvm_cb.nvm_type);
    } else if (p_cback) {
      (*p_cback)((tNFC_HAL_NCI_EVT)(op_code), p_msg->len,
                 (uint8_t*)(p_msg + 1) + p_msg->offset);
    } else if (op_code == NFC_VS_SEC_PATCH_AUTH_EVT) {
      HAL_TRACE_DEBUG0("signature!!");
      nfc_hal_prm_nci_command_complete_cback(
          (tNFC_HAL_NCI_EVT)(op_code), p_msg->len,
          (uint8_t*)(p_msg + 1) + p_msg->offset);
    }
  }
}

/*******************************************************************************
**
** Function         nfc_hal_dm_proc_msg_during_exit
**
** Description      Process NCI message while shutting down NFCC
**
** Returns          void
**
*******************************************************************************/
void nfc_hal_dm_proc_msg_during_exit(NFC_HDR* p_msg) {
  uint8_t* p;
  uint8_t mt, pbf, gid, op_code;
  uint8_t *p_old, old_gid, old_oid, old_mt;
  uint8_t u8;
  tNFC_HAL_NCI_CBACK* p_cback = NULL;

  HAL_TRACE_DEBUG1("nfc_hal_dm_proc_msg_during_exit(): state:%d",
                   nfc_hal_cb.dev_cb.initializing_state);

  p = (uint8_t*)(p_msg + 1) + p_msg->offset;

  NCI_MSG_PRS_HDR0(p, mt, pbf, gid);
  NCI_MSG_PRS_HDR1(p, op_code);
  u8 = *p;

  /* check if waiting for this response */
  if ((nfc_hal_cb.ncit_cb.nci_wait_rsp == NFC_HAL_WAIT_RSP_CMD) ||
      (nfc_hal_cb.ncit_cb.nci_wait_rsp == NFC_HAL_WAIT_RSP_VSC)) {
    if (mt == NCI_MT_RSP) {
      p_old = nfc_hal_cb.ncit_cb.last_hdr;
      NCI_MSG_PRS_HDR0(p_old, old_mt, pbf, old_gid);
      old_oid = ((*p_old) & NCI_OID_MASK);
      /* make sure this is the RSP we are waiting for before updating the
       * command window */
      if ((old_gid == gid) && (old_oid == op_code)) {
        p_cback = nfc_hal_dm_got_vs_rsp();
        if (p_cback) {
          if (gid == NCI_GID_PROP) {
            if (mt == NCI_MT_NTF)
              op_code |= NCI_NTF_BIT;
            else
              op_code |= NCI_RSP_BIT;

            if (op_code == NFC_VS_POWER_LEVEL_RSP) {
              (*p_cback)((tNFC_HAL_NCI_EVT)(op_code), p_msg->len,
                         (uint8_t*)(p_msg + 1) + p_msg->offset);
            }
          }
        }
      }
    }
  }
}

/*******************************************************************************
**
** Function         nfc_hal_dm_send_nci_cmd
**
** Description      Send NCI command to NFCC while initializing BRCM NFCC
**
** Returns          void
**
*******************************************************************************/
void nfc_hal_dm_send_nci_cmd(const uint8_t* p_data, uint16_t len,
                             tNFC_HAL_NCI_CBACK* p_cback) {
  NFC_HDR* p_buf;
  uint8_t* ps;

  HAL_TRACE_DEBUG1("nfc_hal_dm_send_nci_cmd (): nci_wait_rsp = 0x%x",
                   nfc_hal_cb.ncit_cb.nci_wait_rsp);

  if (nfc_hal_cb.ncit_cb.nci_wait_rsp != NFC_HAL_WAIT_RSP_NONE) {
    HAL_TRACE_ERROR0("nfc_hal_dm_send_nci_cmd(): no command window");
    return;
  }

  p_buf = (NFC_HDR*)GKI_getpoolbuf(NFC_HAL_NCI_POOL_ID);
  if (p_buf != NULL) {
    nfc_hal_cb.ncit_cb.nci_wait_rsp = NFC_HAL_WAIT_RSP_VSC;

    p_buf->offset = NFC_HAL_NCI_MSG_OFFSET_SIZE;
    p_buf->event = NFC_HAL_EVT_TO_NFC_NCI;
    p_buf->len = len;

    memcpy((uint8_t*)(p_buf + 1) + p_buf->offset, p_data, len);

    /* Keep a copy of the command and send to NCI transport */

    /* save the message header to double check the response */
    ps = (uint8_t*)(p_buf + 1) + p_buf->offset;
    memcpy(nfc_hal_cb.ncit_cb.last_hdr, ps, NFC_HAL_SAVED_HDR_SIZE);
    memcpy(nfc_hal_cb.ncit_cb.last_cmd, ps + NCI_MSG_HDR_SIZE,
           NFC_HAL_SAVED_CMD_SIZE);

    /* save the callback for NCI VSCs */
    nfc_hal_cb.ncit_cb.p_vsc_cback = (void*)p_cback;

    nfc_hal_nci_send_cmd(p_buf);

    /* start NFC command-timeout timer */
    nfc_hal_main_start_quick_timer(
        &nfc_hal_cb.ncit_cb.nci_wait_rsp_timer,
        (uint16_t)(NFC_HAL_TTYPE_NCI_WAIT_RSP),
        ((uint32_t)NFC_HAL_CMD_TOUT) * QUICK_TIMER_TICKS_PER_SEC / 1000);
  }
}

/*******************************************************************************
**
** Function         nfc_hal_dm_send_pend_cmd
**
** Description      Send a command to NFCC
**
** Returns          void
**
*******************************************************************************/
void nfc_hal_dm_send_pend_cmd(void) {
  NFC_HDR* p_buf = nfc_hal_cb.ncit_cb.p_pend_cmd;
  uint8_t* p;

  if (p_buf == NULL) return;

  /* check low power mode state */
  if (!nfc_hal_dm_power_mode_execute(NFC_HAL_LP_TX_DATA_EVT)) {
    return;
  }

  if (nfc_hal_cb.ncit_cb.nci_wait_rsp == NFC_HAL_WAIT_RSP_PROP) {
#if (NFC_HAL_TRACE_PROTOCOL == TRUE)
    DispHciCmd(p_buf);
#endif

    /* save the message header to double check the response */
    p = (uint8_t*)(p_buf + 1) + p_buf->offset;
    memcpy(nfc_hal_cb.ncit_cb.last_hdr, p, NFC_HAL_SAVED_HDR_SIZE);

    /* add packet type for BT message */
    p_buf->offset--;
    p_buf->len++;

    p = (uint8_t*)(p_buf + 1) + p_buf->offset;
    *p = HCIT_TYPE_COMMAND;

    USERIAL_Write(USERIAL_NFC_PORT, p, p_buf->len);

    GKI_freebuf(p_buf);
    nfc_hal_cb.ncit_cb.p_pend_cmd = NULL;

    /* start NFC command-timeout timer */
    nfc_hal_main_start_quick_timer(
        &nfc_hal_cb.ncit_cb.nci_wait_rsp_timer,
        (uint16_t)(NFC_HAL_TTYPE_NCI_WAIT_RSP),
        ((uint32_t)NFC_HAL_CMD_TOUT) * QUICK_TIMER_TICKS_PER_SEC / 1000);
  }
}

/*******************************************************************************
**
** Function         nfc_hal_dm_send_bt_cmd
**
** Description      Send BT message to NFCC while initializing BRCM NFCC
**
** Returns          void
**
*******************************************************************************/
void nfc_hal_dm_send_bt_cmd(const uint8_t* p_data, uint16_t len,
                            tNFC_HAL_BTVSC_CPLT_CBACK* p_cback) {
  NFC_HDR* p_buf;
  char buff[300];
  char tmp[4];
  buff[0] = 0;
  int i;

  HAL_TRACE_DEBUG1("nfc_hal_dm_send_bt_cmd (): nci_wait_rsp = 0x%x",
                   nfc_hal_cb.ncit_cb.nci_wait_rsp);

  for (i = 0; i < len; i++) {
    sprintf(tmp, "%02x ", p_data[i]);
    strcat(buff, tmp);
  }
  HAL_TRACE_DEBUG2("nfc_hal_dm_send_bt_cmd (): HCI Write (%d bytes): %s", len,
                   buff);

  if (nfc_hal_cb.ncit_cb.nci_wait_rsp != NFC_HAL_WAIT_RSP_NONE) {
    HAL_TRACE_ERROR0("nfc_hal_dm_send_bt_cmd(): no command window");
    return;
  }

  p_buf = (NFC_HDR*)GKI_getpoolbuf(NFC_HAL_NCI_POOL_ID);
  if (p_buf != NULL) {
    nfc_hal_cb.ncit_cb.nci_wait_rsp = NFC_HAL_WAIT_RSP_PROP;

    p_buf->offset = NFC_HAL_NCI_MSG_OFFSET_SIZE;
    p_buf->len = len;

    memcpy((uint8_t*)(p_buf + 1) + p_buf->offset, p_data, len);

    /* save the callback for NCI VSCs)  */
    nfc_hal_cb.ncit_cb.p_vsc_cback = (void*)p_cback;

    nfc_hal_cb.ncit_cb.p_pend_cmd = p_buf;
    if (nfc_hal_cb.dev_cb.initializing_state == NFC_HAL_INIT_STATE_IDLE) {
      NFC_HAL_SET_INIT_STATE(NFC_HAL_INIT_STATE_W4_CONTROL_DONE);
      nfc_hal_cb.p_stack_cback(HAL_NFC_REQUEST_CONTROL_EVT, HAL_NFC_STATUS_OK);
      return;
    }

    nfc_hal_dm_send_pend_cmd();
  }
}

/*******************************************************************************
**
** Function         nfc_hal_dm_set_nfc_wake
**
** Description      Set NFC_WAKE line
**
** Returns          void
**
*******************************************************************************/
void nfc_hal_dm_set_nfc_wake(uint8_t cmd) {
  HAL_TRACE_DEBUG1("nfc_hal_dm_set_nfc_wake () %s",
                   (cmd == NFC_HAL_ASSERT_NFC_WAKE ? "ASSERT" : "DEASSERT"));

  /*
  **  nfc_wake_active_mode             cmd              result of voltage on
  *NFC_WAKE
  **
  **  NFC_HAL_LP_ACTIVE_LOW (0)    NFC_HAL_ASSERT_NFC_WAKE (0)    pull down
  *NFC_WAKE (GND)
  **  NFC_HAL_LP_ACTIVE_LOW (0)    NFC_HAL_DEASSERT_NFC_WAKE (1)  pull up
  *NFC_WAKE (VCC)
  **  NFC_HAL_LP_ACTIVE_HIGH (1)   NFC_HAL_ASSERT_NFC_WAKE (0)    pull up
  *NFC_WAKE (VCC)
  **  NFC_HAL_LP_ACTIVE_HIGH (1)   NFC_HAL_DEASSERT_NFC_WAKE (1)  pull down
  *NFC_WAKE (GND)
  */

  if (cmd == nfc_hal_cb.dev_cb.nfc_wake_active_mode)
    UPIO_Set(UPIO_GENERAL, NFC_HAL_LP_NFC_WAKE_GPIO,
             UPIO_OFF); /* pull down NFC_WAKE */
  else
    UPIO_Set(UPIO_GENERAL, NFC_HAL_LP_NFC_WAKE_GPIO,
             UPIO_ON); /* pull up NFC_WAKE */
}

/*******************************************************************************
**
** Function         nfc_hal_dm_power_mode_execute
**
** Description      If snooze mode is enabled in full power mode,
**                     Assert NFC_WAKE before sending data
**                     Deassert NFC_WAKE when idle timer expires
**
** Returns          TRUE if DH can send data to NFCC
**
*******************************************************************************/
bool nfc_hal_dm_power_mode_execute(tNFC_HAL_LP_EVT event) {
  bool send_to_nfcc = false;

  HAL_TRACE_DEBUG1("nfc_hal_dm_power_mode_execute () event = %d", event);

  if (nfc_hal_cb.dev_cb.power_mode == NFC_HAL_POWER_MODE_FULL) {
    if (nfc_hal_cb.dev_cb.snooze_mode != NFC_HAL_LP_SNOOZE_MODE_NONE) {
      /* if any transport activity */
      if ((event == NFC_HAL_LP_TX_DATA_EVT) ||
          (event == NFC_HAL_LP_RX_DATA_EVT)) {
        /* if idle timer is not running */
        if (nfc_hal_cb.dev_cb.lp_timer.in_use == false) {
          nfc_hal_dm_set_nfc_wake(NFC_HAL_ASSERT_NFC_WAKE);
        }

        /* start or extend idle timer */
        nfc_hal_main_start_quick_timer(&nfc_hal_cb.dev_cb.lp_timer, 0x00,
                                       ((uint32_t)NFC_HAL_LP_IDLE_TIMEOUT) *
                                           QUICK_TIMER_TICKS_PER_SEC / 1000);
      } else if (event == NFC_HAL_LP_TIMEOUT_EVT) {
        /* let NFCC go to snooze mode */
        nfc_hal_dm_set_nfc_wake(NFC_HAL_DEASSERT_NFC_WAKE);
      }
    }

    send_to_nfcc = true;
  }

  return (send_to_nfcc);
}

/*******************************************************************************
**
** Function         nci_brcm_lp_timeout_cback
**
** Description      callback function for low power timeout
**
** Returns          void
**
*******************************************************************************/
static void nci_brcm_lp_timeout_cback(void* p_tle) {
  HAL_TRACE_DEBUG0("nci_brcm_lp_timeout_cback ()");

  nfc_hal_dm_power_mode_execute(NFC_HAL_LP_TIMEOUT_EVT);
}

/*******************************************************************************
**
** Function         nfc_hal_dm_pre_init_nfcc
**
** Description      This function initializes Broadcom specific control blocks
*for
**                  NCI transport
**
** Returns          void
**
*******************************************************************************/
void nfc_hal_dm_pre_init_nfcc(void) {
  HAL_TRACE_DEBUG0("nfc_hal_dm_pre_init_nfcc ()");

  /* if it was waiting for core reset notification after raising REG_PU */
  if (nfc_hal_cb.dev_cb.initializing_state ==
      NFC_HAL_INIT_STATE_W4_NFCC_ENABLE) {
    nfc_hal_dm_send_get_build_info_cmd();
  }
  /* if it was waiting for core reset notification after setting Xtal */
  else if (nfc_hal_cb.dev_cb.initializing_state ==
           NFC_HAL_INIT_STATE_POST_XTAL_SET) {
    {
      /* Core reset ntf after xtal setting indicating NFCC loaded patch from NVM
       */
      NFC_HAL_SET_INIT_STATE(NFC_HAL_INIT_STATE_W4_PATCH_INFO);

      nfc_hal_dm_send_nci_cmd(nfc_hal_dm_get_patch_version_cmd,
                              NCI_MSG_HDR_SIZE, NULL);
    }
  }
}

/*******************************************************************************
**
** Function         nfc_hal_dm_shutting_down_nfcc
**
** Description      This function initializes Broadcom specific control blocks
*for
**                  NCI transport
**
** Returns          void
**
*******************************************************************************/
void nfc_hal_dm_shutting_down_nfcc(void) {
  HAL_TRACE_DEBUG0("nfc_hal_dm_shutting_down_nfcc ()");

  nfc_hal_cb.dev_cb.initializing_state = NFC_HAL_INIT_STATE_CLOSING;

  /* reset low power mode variables */
  if ((nfc_hal_cb.dev_cb.power_mode == NFC_HAL_POWER_MODE_FULL) &&
      (nfc_hal_cb.dev_cb.snooze_mode != NFC_HAL_LP_SNOOZE_MODE_NONE)) {
    nfc_hal_dm_set_nfc_wake(NFC_HAL_ASSERT_NFC_WAKE);
  }

  nfc_hal_cb.ncit_cb.nci_wait_rsp = NFC_HAL_WAIT_RSP_NONE;

  nfc_hal_cb.dev_cb.power_mode = NFC_HAL_POWER_MODE_FULL;
  nfc_hal_cb.dev_cb.snooze_mode = NFC_HAL_LP_SNOOZE_MODE_NONE;

  /* Stop all timers */
  nfc_hal_main_stop_quick_timer(&nfc_hal_cb.ncit_cb.nci_wait_rsp_timer);
  nfc_hal_main_stop_quick_timer(&nfc_hal_cb.dev_cb.lp_timer);
  nfc_hal_main_stop_quick_timer(&nfc_hal_cb.prm.timer);
#if (NFC_HAL_HCI_INCLUDED == TRUE)
  nfc_hal_cb.hci_cb.hcp_conn_id = 0;
  nfc_hal_main_stop_quick_timer(&nfc_hal_cb.hci_cb.hci_timer);
#endif
  nfc_hal_main_stop_quick_timer(&nfc_hal_cb.timer);
}

/*******************************************************************************
**
** Function         nfc_hal_dm_init
**
** Description      This function initializes Broadcom specific control blocks
*for
**                  NCI transport
**
** Returns          void
**
*******************************************************************************/
void nfc_hal_dm_init(void) {
  HAL_TRACE_DEBUG0("nfc_hal_dm_init ()");

  nfc_hal_cb.dev_cb.lp_timer.p_cback = nci_brcm_lp_timeout_cback;

  nfc_hal_cb.ncit_cb.nci_wait_rsp_timer.p_cback = nfc_hal_nci_cmd_timeout_cback;

#if (NFC_HAL_HCI_INCLUDED == TRUE)
  nfc_hal_cb.hci_cb.hci_timer.p_cback = nfc_hal_hci_timeout_cback;
#endif

  nfc_hal_cb.pre_discover_done = false;

  nfc_post_reset_cb.spd_nvm_detection_cur_count = 0;
  nfc_post_reset_cb.spd_skip_on_power_cycle = false;
}

/*******************************************************************************
**
** Function         HAL_NfcDevInitDone
**
** Description      Notify that pre-initialization of NFCC is complete
**
** Returns          void
**
*******************************************************************************/
void HAL_NfcPreInitDone(tHAL_NFC_STATUS status) {
  HAL_TRACE_DEBUG1("HAL_NfcPreInitDone () status=%d", status);

  if (nfc_hal_cb.dev_cb.initializing_state ==
      NFC_HAL_INIT_STATE_W4_APP_COMPLETE) {
    NFC_HAL_SET_INIT_STATE(NFC_HAL_INIT_STATE_IDLE);

    nfc_hal_main_pre_init_done(status);
  }
}

/*******************************************************************************
**
** Function         HAL_NfcReInit
**
** Description      This function is called to restart initialization after
*REG_PU
**                  toggled because of failure to detect NVM type or download
*patchram.
**
** Note             This function should be called only during the HAL init
*process
**
** Returns          HAL_NFC_STATUS_OK if successfully initiated
**                  HAL_NFC_STATUS_FAILED otherwise
**
*******************************************************************************/
tHAL_NFC_STATUS HAL_NfcReInit(void) {
  tHAL_NFC_STATUS status = HAL_NFC_STATUS_FAILED;

  HAL_TRACE_DEBUG1("HAL_NfcReInit () init st=0x%x",
                   nfc_hal_cb.dev_cb.initializing_state);
  if (nfc_hal_cb.dev_cb.initializing_state ==
      NFC_HAL_INIT_STATE_W4_APP_COMPLETE) {
    {
      /* Wait for NFCC to enable - Core reset notification */
      NFC_HAL_SET_INIT_STATE(NFC_HAL_INIT_STATE_W4_NFCC_ENABLE);

      /* NFCC Enable timeout */
      nfc_hal_main_start_quick_timer(
          &nfc_hal_cb.timer, NFC_HAL_TTYPE_NFCC_ENABLE,
          ((p_nfc_hal_cfg->nfc_hal_nfcc_enable_timeout) *
           QUICK_TIMER_TICKS_PER_SEC) /
              1000);
    }

    status = HAL_NFC_STATUS_OK;
  }
  return status;
}

/*******************************************************************************
**
** Function         nfc_hal_dm_set_snooze_mode_cback
**
** Description      This is snooze update complete callback.
**
** Returns          void
**
*******************************************************************************/
static void nfc_hal_dm_set_snooze_mode_cback(tNFC_HAL_BTVSC_CPLT* pData) {
  uint8_t status = pData->p_param_buf[0];
  tHAL_NFC_STATUS hal_status;
  tHAL_NFC_STATUS_CBACK* p_cback;

  /* if it is completed */
  if (status == HCI_SUCCESS) {
    /* update snooze mode */
    nfc_hal_cb.dev_cb.snooze_mode = nfc_hal_cb.dev_cb.new_snooze_mode;

    nfc_hal_dm_set_nfc_wake(NFC_HAL_ASSERT_NFC_WAKE);

    if (nfc_hal_cb.dev_cb.snooze_mode != NFC_HAL_LP_SNOOZE_MODE_NONE) {
      /* start idle timer */
      nfc_hal_main_start_quick_timer(&nfc_hal_cb.dev_cb.lp_timer, 0x00,
                                     ((uint32_t)NFC_HAL_LP_IDLE_TIMEOUT) *
                                         QUICK_TIMER_TICKS_PER_SEC / 1000);
    } else {
      nfc_hal_main_stop_quick_timer(&nfc_hal_cb.dev_cb.lp_timer);
    }
    hal_status = HAL_NFC_STATUS_OK;
  } else {
    hal_status = HAL_NFC_STATUS_FAILED;
  }

  if (nfc_hal_cb.dev_cb.p_prop_cback) {
    p_cback = nfc_hal_cb.dev_cb.p_prop_cback;
    nfc_hal_cb.dev_cb.p_prop_cback = NULL;
    (*p_cback)(hal_status);
  }
}

/*******************************************************************************
**
** Function         HAL_NfcSetSnoozeMode
**
** Description      Set snooze mode
**                  snooze_mode
**                      NFC_HAL_LP_SNOOZE_MODE_NONE - Snooze mode disabled
**                      NFC_HAL_LP_SNOOZE_MODE_UART - Snooze mode for UART
**                      NFC_HAL_LP_SNOOZE_MODE_SPI_I2C - Snooze mode for SPI/I2C
**
**                  idle_threshold_dh/idle_threshold_nfcc
**                      Idle Threshold Host in 100ms unit
**
**                  nfc_wake_active_mode/dh_wake_active_mode
**                      NFC_HAL_LP_ACTIVE_LOW - high to low voltage is asserting
**                      NFC_HAL_LP_ACTIVE_HIGH - low to high voltage is
*asserting
**
**                  p_snooze_cback
**                      Notify status of operation
**
** Returns          tHAL_NFC_STATUS
**
*******************************************************************************/
tHAL_NFC_STATUS HAL_NfcSetSnoozeMode(uint8_t snooze_mode,
                                     uint8_t idle_threshold_dh,
                                     uint8_t idle_threshold_nfcc,
                                     uint8_t nfc_wake_active_mode,
                                     uint8_t dh_wake_active_mode,
                                     tHAL_NFC_STATUS_CBACK* p_snooze_cback) {
  uint8_t cmd[NFC_HAL_BT_HCI_CMD_HDR_SIZE + HCI_BRCM_WRITE_SLEEP_MODE_LENGTH];
  uint8_t* p;

  HAL_TRACE_API1("HAL_NfcSetSnoozeMode (): snooze_mode = %d", snooze_mode);

  nfc_hal_cb.dev_cb.new_snooze_mode = snooze_mode;
  nfc_hal_cb.dev_cb.nfc_wake_active_mode = nfc_wake_active_mode;
  nfc_hal_cb.dev_cb.p_prop_cback = p_snooze_cback;

  p = cmd;

  /* Add the HCI command */
  UINT16_TO_STREAM(p, HCI_BRCM_WRITE_SLEEP_MODE);
  UINT8_TO_STREAM(p, HCI_BRCM_WRITE_SLEEP_MODE_LENGTH);

  memset(p, 0x00, HCI_BRCM_WRITE_SLEEP_MODE_LENGTH);

  UINT8_TO_STREAM(p, snooze_mode); /* Sleep Mode               */

  UINT8_TO_STREAM(p, idle_threshold_dh);    /* Idle Threshold Host      */
  UINT8_TO_STREAM(p, idle_threshold_nfcc);  /* Idle Threshold HC        */
  UINT8_TO_STREAM(p, nfc_wake_active_mode); /* BT Wake Active Mode      */
  UINT8_TO_STREAM(p, dh_wake_active_mode);  /* Host Wake Active Mode    */

  nfc_hal_dm_send_bt_cmd(
      cmd, NFC_HAL_BT_HCI_CMD_HDR_SIZE + HCI_BRCM_WRITE_SLEEP_MODE_LENGTH,
      nfc_hal_dm_set_snooze_mode_cback);
  return (NCI_STATUS_OK);
}
