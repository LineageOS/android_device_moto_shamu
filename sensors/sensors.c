/*
 * Copyright (C) 2009-2012 Motorola, Inc.
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

#include <hardware/sensors.h>
#include <hardware/mot_sensorhub_stm401.h>
#include <float.h>

#include "nusensors.h"

/*****************************************************************************/

/*
 * The SENSORS Module
 */

static const struct sensor_t sSensorList[] = {
    { "MPU6515 3-axis Accelerometer",
                "InvenSense",
                1, SENSORS_HANDLE_BASE+ID_A,
                SENSOR_TYPE_ACCELEROMETER, 16.0f*9.81f, 9.81f/2048.0f, 0.25f, 10000, 0, 0, { 0 } },
    { "MPU6515 Gyroscope sensor",
                "InvenSense",
                1, SENSORS_HANDLE_BASE+ID_G,
                SENSOR_TYPE_GYROSCOPE, 2000.0f, 1.0f, 6.1f, 20000, 0, 0, { 0 } },
    { "AK8963 3-axis Magnetic field sensor",
                "Asahi Kasei",
                1, SENSORS_HANDLE_BASE+ID_M,
                SENSOR_TYPE_MAGNETIC_FIELD, 2000.0f, 1.0f/10.0f, 6.8f, 10000, 0, 0, { 0 } },
    { "AK8963 Orientation sensor",
                "Asahi Kasei",
                1, SENSORS_HANDLE_BASE+ID_O,
                SENSOR_TYPE_ORIENTATION, 360.0f, 1.0f/64.0f, 7.05f, 10000, 0, 0, { 0 } },
    { "CT406 Light sensor",
                "TAOS",
                1, SENSORS_HANDLE_BASE+ID_L,
                SENSOR_TYPE_LIGHT, 27000.0f, 1.0f, 0.175f, 0, 0, 0, { 0 } },
    { "Display Rotation sensor",
                "Motorola",
                1, SENSORS_HANDLE_BASE+ID_DR,
                SENSOR_TYPE_DISPLAY_ROTATE, 4.0f, 1.0f, 0.0f, 0, 0, 0, { 0 } },
    { "Display Brightness sensor",
                "Motorola",
                1, SENSORS_HANDLE_BASE+ID_DB,
                SENSOR_TYPE_DISPLAY_BRIGHTNESS, 255.0f, 1.0f, 0.0f, 0, 0, 0, { 0 } },
    /*{ "Dock",
                "Motorola",
                1, SENSORS_HANDLE_BASE+ID_D,
                SENSOR_TYPE_DOCK, 3.0f, 1.0f, 0.01f, 0, 0, 0, { 0 } },
    */
    { "CT406 Proximity sensor",
                "TAOS",
                1, SENSORS_HANDLE_BASE+ID_P,
                SENSOR_TYPE_PROXIMITY, 100.0f, 100.0f, 3.0f, 0, 0, 0, { 0 } },
    /*
    { "Flat Up",
                "Motorola",
                1, SENSORS_HANDLE_BASE+ID_FU,
                SENSOR_TYPE_FLAT_UP, 1.0f, 1.0f, 0.0f, 0, 0, 0, { 0 } },
    { "Flat Down",
                "Motorola",
                1, SENSORS_HANDLE_BASE+ID_FD,
                SENSOR_TYPE_FLAT_DOWN, 1.0f, 1.0f, 0.0f, 0, 0, 0, { 0 } },
    { "Stowed",
                "Motorola",
                1, SENSORS_HANDLE_BASE+ID_S,
                SENSOR_TYPE_STOWED, 1.0f, 1.0f, 0.0f, 0, 0, 0, { 0 } },

    { "Camera Activation sensor",
                "Motorola",
                1, SENSORS_HANDLE_BASE+ID_CA,
                SENSOR_TYPE_CAMERA_ACTIVATE, 1.0f, 1.0f, 0.0f, 20000, 0, 0, { 0 } },

    { "NFC Detect sensor",
                "Motorola",
                1, SENSORS_HANDLE_BASE+ID_NFC,
                SENSOR_TYPE_NFC_DETECT, 1.0f, 1.0f, 0.0f, 0, 0, 0, { 0 } },
    { "IR Gestures",
                "Motorola",
                1, SENSORS_HANDLE_BASE+ID_IR_GESTURE,
                SENSOR_TYPE_IR_GESTURE, 1.0f, 1.0f, 1.0f, 0, 8, 8, { 0 } },

    { "IR Raw Data",
                "Motorola",
                1, SENSORS_HANDLE_BASE+ID_IR_RAW,
                SENSOR_TYPE_IR_RAW, 4096.0f, 1.0f, 1.0f, 10000, 0, 0, { 0 } },
    { "Significant Motion sensor",
                "Motorola",
                1, SENSORS_HANDLE_BASE+ID_SIM,
                SENSOR_TYPE_SIGNIFICANT_MOTION, 1.0f, 1.0f, 3.0f, -1, 0, 0, { 0 } },
    */
    { "Step Detector sensor",
                "Motorola",
                1, SENSORS_HANDLE_BASE+ID_STEP_DETECTOR,
                SENSOR_TYPE_STEP_DETECTOR, 1.0f, 0, 0, 0, 0, 0, { 0 } },

    { "Step Counter sensor",
                "Motorola",
                1, SENSORS_HANDLE_BASE+ID_STEP_COUNTER,
                SENSOR_TYPE_STEP_COUNTER, FLT_MAX, 0, 0, 0, 0, 0, { 0 } },

    { "Uncalibrated gyro sensor",
                "Motorola",
                1, SENSORS_HANDLE_BASE+ID_UNCALIB_GYRO,
                SENSOR_TYPE_GYROSCOPE_UNCALIBRATED,2000.0f, 1.0f, 6.1f, 20000, 0, 0, { 0 } },

    { "AK8963 3-axis Uncalibrated Magnetic field sensor",
                "Asahi Kasei",
                1, SENSORS_HANDLE_BASE+ID_UNCALIB_MAG,
                SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED, 2000.0f, 1.0f/10.0f, 6.8f, 10000, 0, 0, { 0 } },
};

static int open_sensors(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device);

static int sensors__get_sensors_list(struct sensors_module_t* module,
        struct sensor_t const** list)
{
    *list = sSensorList;
    return ARRAY_SIZE(sSensorList);
}

static struct hw_module_methods_t sensors_module_methods = {
    .open = open_sensors
};

struct sensors_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 2,
        .version_minor = 0,
        .id = SENSORS_HARDWARE_MODULE_ID,
        .name = "Motorola Sensors Module",
        .author = "Motorola",
        .methods = &sensors_module_methods,
    },
    .get_sensors_list = sensors__get_sensors_list
};

/*****************************************************************************/

static int open_sensors(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
    return init_nusensors(module, device);
}
