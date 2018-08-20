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
 * Construct a buffer that contains multiple Type-Length-Value contents
 * that is used by the HAL in a CORE_SET_CONFIG NCI command.
 ******************************************************************************/

#pragma once
#include <string>
#include "bt_types.h"

class StartupConfig {
 public:
  typedef std::basic_string<uint8_t> uint8_string;
  StartupConfig();

  /*******************************************************************************
  **
  ** Function:        initialize
  **
  ** Description:     Reset all member variables.
  **
  ** Returns:         None
  **
  *******************************************************************************/
  void initialize();

  /*******************************************************************************
  **
  ** Function:        getInternalBuffer
  **
  ** Description:     Get the pointer to buffer that contains multiple
  **                  Type-Length-Value contents.
  **
  ** Returns:         Pointer to buffer.
  **
  *******************************************************************************/
  const uint8_t* getInternalBuffer();

  /*******************************************************************************
  **
  ** Function:        append
  **
  ** Description:     Append new config data to internal buffer.
  **                  newContent: buffer containing new content; newContent[0]
  *is
  **                          payload length; newContent[1..end] is payload.
  **                  newContentLen: total length of newContent.
  **
  ** Returns:         True if ok.
  **
  *******************************************************************************/
  bool append(const uint8_t* newContent, uint8_t newContentLen);

  /*******************************************************************************
  **
  ** Function:        disableSecureElement
  **
  ** Description:     Adjust a TLV to disable secure element(s).  The TLV's type
  *is 0xC2.
  **                  bitmask: 0xC0 = do not detect any secure element.
  **                           0x40 = do not detect secure element in slot 0.
  **                           0x80 = do not detect secure element in slot 1.
  **
  ** Returns:         True if ok.
  **
  *******************************************************************************/
  bool disableSecureElement(uint8_t bitmask);

 private:
  static const uint8_t mMaxLength;
  uint8_string mBuffer;
};
