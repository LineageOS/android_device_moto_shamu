/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_HUB_SENSOR_H
#define ANDROID_HUB_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <endian.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <zlib.h>
#include <time.h>
#include <private/android_filesystem_config.h>

#include "linux/stm401.h"

#include "nusensors.h"
#include "SensorBase.h"

/*****************************************************************************/

#define SENSORS_EVENT_T_SIZE sizeof(sensors_event_t);
#define MAG_CAL_FILE "/data/misc/akmd_set.txt"
#define DROPBOX_DIR "/data/system/dropbox-add"
#define DROPBOX_TAG "SENSOR_HUB"
#define SENSORHUB_DUMPFILE  "sensor_hub"
#define DROPBOX_FLAG_TEXT        2
#define DROPBOX_FLAG_GZIP        4
#define COPYSIZE 256

// Defines for offsets into the sensorhub event data.
#define ACCEL_X (0 * sizeof(int16_t))
#define ACCEL_Y (1 * sizeof(int16_t))
#define ACCEL_Z (2 * sizeof(int16_t))

#define GYRO_X (0 * sizeof(int16_t))
#define GYRO_Y (1 * sizeof(int16_t))
#define GYRO_Z (2 * sizeof(int16_t))

#define UNCALIB_GYRO_X (0 * sizeof(int16_t))
#define UNCALIB_GYRO_Y (1 * sizeof(int16_t))
#define UNCALIB_GYRO_Z (2 * sizeof(int16_t))
#define UNCALIB_GYRO_X_BIAS (3 * sizeof(int16_t))
#define UNCALIB_GYRO_Y_BIAS (4 * sizeof(int16_t))
#define UNCALIB_GYRO_Z_BIAS (5 * sizeof(int16_t))

#define PRESSURE_PRESSURE (0 * sizeof(int32_t))

#define MAGNETIC_X (0 * sizeof(int16_t))
#define MAGNETIC_Y (1 * sizeof(int16_t))
#define MAGNETIC_Z (2 * sizeof(int16_t))

#define UNCALIB_MAGNETIC_X (0 * sizeof(int16_t))
#define UNCALIB_MAGNETIC_Y (1 * sizeof(int16_t))
#define UNCALIB_MAGNETIC_Z (2 * sizeof(int16_t))
#define UNCALIB_MAGNETIC_X_BIAS (3 * sizeof(int16_t))
#define UNCALIB_MAGNETIC_Y_BIAS (4 * sizeof(int16_t))
#define UNCALIB_MAGNETIC_Z_BIAS (5 * sizeof(int16_t))

#define STEP_COUNTER_0 (0 * sizeof(int16_t))
#define STEP_COUNTER_1 (1 * sizeof(int16_t))
#define STEP_COUNTER_2 (2 * sizeof(int16_t))
#define STEP_COUNTER_3 (3 * sizeof(int16_t))

#define STEP_DETECTOR (0 * sizeof(int16_t))

#define SIM (0 * sizeof(int16_t))

#define ORIENTATION_AZIMUTH (0 * sizeof(int16_t))
#define ORIENTATION_PITCH   (1 * sizeof(int16_t))
#define ORIENTATION_ROLL    (2 * sizeof(int16_t))

#define TEMPERATURE_TEMPERATURE (0 * sizeof(int16_t))

#define LIGHT_LIGHT (0 * sizeof(int16_t))

#define ROTATE_ROTATE (0 * sizeof(int8_t))

#define BRIGHT_BRIGHT (0 * sizeof(int8_t))

#define DOCK_DOCK (0 * sizeof(int8_t))

#define PROXIMITY_PROXIMITY (0 * sizeof(int8_t))

#define FLAT_FLAT (0 * sizeof(int8_t))

#define STOWED_STOWED (0 * sizeof(int8_t))

#define CAMERA_CAMERA (0 * sizeof(int16_t))

#define NFC_NFC (0 * sizeof(int8_t))

#define IR_EVENT      (0 * sizeof(int8_t))
#define IR_GESTURE    (1 * sizeof(int8_t))
#define IR_DIRECTION  (2 * sizeof(int8_t))
#define IR_MAGNITUDE  (2 * sizeof(int8_t)) // Same offset as direction.
#define IR_CONFIDENCE (3 * sizeof(int8_t))
#define IR_TR_H       (0 * sizeof(int16_t))
#define IR_BL_H       (1 * sizeof(int16_t))
#define IR_BR_H       (2 * sizeof(int16_t))
#define IR_BB_H       (3 * sizeof(int16_t))
#define IR_TR_L       (4 * sizeof(int16_t))
#define IR_BL_L       (5 * sizeof(int16_t))
#define IR_BR_L       (6 * sizeof(int16_t))
#define IR_BB_L       (7 * sizeof(int16_t))
#define IR_AMBIENT_H  (8 * sizeof(int16_t))
#define IR_AMBIENT_L  (9 * sizeof(int16_t))

#define STM16TOH(p) (int16_t) be16toh(*((uint16_t *) (p)))
#define STM32TOH(p) (int32_t) be32toh(*((uint32_t *) (p)))

struct input_event;

class HubSensor : public SensorBase {
public:
            HubSensor();
    virtual ~HubSensor();

    virtual int setDelay(int32_t handle, int64_t ns);
    virtual int enable(int32_t handle, int enabled);
    virtual int readEvents(sensors_event_t* data, int count);

private:
    int update_delay();
    uint32_t mEnabled;
    uint32_t mWakeEnabled;
    uint32_t mPendingMask;
    uint8_t mMagCal[STM401_MAG_CAL_SIZE];
    gzFile open_dropbox_file(const char* timestamp, const char* dst, const int flags);
    short capture_dump(char* timestamp, const int id, const char* dst, const int flags);
};

/*****************************************************************************/

#endif  // ANDROID_HUB_SENSOR_H
