/******************************************************************************
 *
 *  Copyright (C) 2003-2014 Broadcom Corporation
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
 *  This is the interface file for storing nv data
 *
 ******************************************************************************/
#ifndef NFC_HAL_NV_CO_H
#define NFC_HAL_NV_CO_H

#include "nfc_hal_target.h"

#if (NFC_HAL_HCI_INCLUDED == TRUE)

#include <time.h>

/*****************************************************************************
**  Constants and Data Types
*****************************************************************************/

/**************************
**  Common Definitions
***************************/

/* Status codes returned by call-out functions, or in call-in functions as
 * status */
#define NFC_HAL_NV_CO_OK 0x00
#define NFC_HAL_NV_CO_FAIL 0x01 /* Used to pass all other errors */
#define NFC_HAL_NV_CO_EACCES 0x02
#define NFC_HAL_NV_CO_ENOTEMPTY 0x03
#define NFC_HAL_NV_CO_EOF 0x04
#define NFC_HAL_NV_CO_EODIR 0x05
/* Returned in nfa_nv_ci_open if no room */
#define NFC_HAL_NV_CO_ENOSPACE 0x06
#define NFC_HAL_NV_CO_EIS_DIR 0x07
#define NFC_HAL_NV_CO_RESUME 0x08 /* used in nfa_nv_ci_open, on resume */
/* used in nfa_nv_ci_open, on resume (no file to resume) */
#define NFC_HAL_NV_CO_NONE 0x09

typedef uint8_t tNFC_HAL_NV_CO_STATUS;

#define DH_NV_BLOCK 0x01
#define HC_F3_NV_BLOCK 0x02
#define HC_F4_NV_BLOCK 0x03
#define HC_F2_NV_BLOCK 0x04
#define HC_F5_NV_BLOCK 0x05

/*****************************************************************************
**  Function Declarations
*****************************************************************************/
/**************************
**  Common Functions
***************************/

/*******************************************************************************
**
** Function         nfc_hal_nv_co_read
**
** Description      This function is called by NFA to read in data from the
**                  previously opened file.
**
** Parameters       p_buf   - buffer to read the data into.
**                  nbytes  - number of bytes to read into the buffer.
**
** Returns          void
**
**                  Note: Upon completion of the request, nfa_nv_ci_read () is
**                        called with the buffer of data, along with the number
**                        of bytes read into the buffer, and a status.  The
**                        call-in function should only be called when ALL
**                        requested bytes have been read, the end of file has
**                        been detected, or an error has occurred.
**
*******************************************************************************/
void nfc_hal_nv_co_read(uint8_t* p_buf, uint16_t nbytes, uint8_t block);

/*******************************************************************************
**
** Function         nfc_hal_nv_co_write
**
** Description      This function is called by io to send file data to the
**                  phone.
**
** Parameters       p_buf   - buffer to read the data from.
**                  nbytes  - number of bytes to write out to the file.
**
** Returns          void
**
**                  Note: Upon completion of the request, nfa_nv_ci_write () is
**                        called with the file descriptor and the status. The
**                        call-in function should only be called when ALL
**                        requested bytes have been written, or an error has
**                        been detected,
**
*******************************************************************************/
void nfc_hal_nv_co_write(const uint8_t* p_buf, uint16_t nbytes, uint8_t block);

#endif /* NFC_HAL_HCI_INCLUDED */
#endif /* NFC_HAL_NV_CO_H */
