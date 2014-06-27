/*
 * Copyright (C) 2011- 2012 Motorola, Inc.
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

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <dlfcn.h>

#include <linux/akm8975.h>
#include <linux/stm401.h>

#include <cutils/log.h>

#include <hardware/mot_sensorhub_stm401.h>

#include "stm401_hal.h"

/*****************************************************************************/

HubSensor::HubSensor()
: SensorBase(SENSORHUB_DEVICE_NAME, SENSORHUB_AS_DATA_NAME),
      mEnabled(0),
      mWakeEnabled(0),
      mPendingMask(0)
{
    // read the actual value of all sensors if they're enabled already
    struct input_absinfo absinfo;
    short flags = 0;
    FILE *fp;
    int i;
    int err = 0;

    memset(mMagCal, 0, sizeof(mMagCal));

    open_device();

    if (!ioctl(dev_fd, STM401_IOCTL_GET_SENSORS, &flags))  {
        mEnabled = flags;
    }

    if (!ioctl(dev_fd, STM401_IOCTL_GET_WAKESENSORS, &flags))  {
        mWakeEnabled = flags;
    }

    if ((fp = fopen(MAG_CAL_FILE, "r")) != NULL) {
        for (i=0; i<STM401_MAG_CAL_SIZE; i++) {
            mMagCal[i] = fgetc(fp);
        }
        fclose(fp);
        err = ioctl(dev_fd, STM401_IOCTL_SET_MAG_CAL, &mMagCal);
        if (err < 0) {
           ALOGE("Can't send Mag Cal data");
        }
    }
}

HubSensor::~HubSensor()
{
}

int HubSensor::enable(int32_t handle, int en)
{
    int newState  = en ? 1 : 0;
    uint32_t new_enabled;
    int found = 0;
    int err = 0;

    new_enabled = mEnabled;
    switch (handle) {
        case ID_A:
            new_enabled &= ~M_ACCEL;
            if (newState)
                new_enabled |= M_ACCEL;
            found = 1;
            break;
        case ID_G:
            new_enabled &= ~M_GYRO;
            if (newState)
                new_enabled |= M_GYRO;
            found = 1;
            break;
        case ID_PR:
            new_enabled &= ~M_PRESSURE;
            if (newState)
                new_enabled |= M_PRESSURE;
            found = 1;
            break;
        case ID_M:
        case ID_O:
            new_enabled &= ~M_ECOMPASS;
            if (newState)
                new_enabled |= M_ECOMPASS;
            else {
                FILE *fp;
                int i;

                err = ioctl(dev_fd, STM401_IOCTL_GET_MAG_CAL, &mMagCal);
                if (err < 0) {
                    ALOGE("Can't read Mag Cal data");
                } else {
                    if ((fp = fopen(MAG_CAL_FILE, "w")) == NULL) {
                        ALOGE("Can't open Mag Cal file");
                    } else {
                        for (i=0; i<STM401_MAG_CAL_SIZE; i++) {
                            fputc(mMagCal[i], fp);
                        }
                        fclose(fp);
                    }
                }
            }
            found = 1;
            break;
        case ID_T:
            new_enabled &= ~M_TEMPERATURE;
            if (newState)
                new_enabled |= M_TEMPERATURE;
            found = 1;
            break;
        case ID_L:
            new_enabled &= ~M_ALS;
            if (newState)
                new_enabled |= M_ALS;
            found = 1;
            break;
        case ID_LA:
            new_enabled &= ~M_LIN_ACCEL;
            if (newState)
                new_enabled |= M_LIN_ACCEL;
            found = 1;
            break;
        case ID_Q:
            new_enabled &= ~M_QUATERNION;
            if (newState)
                new_enabled |= M_QUATERNION;
            found = 1;
            break;
        case ID_GR:
            new_enabled &= ~M_GRAVITY;
            if (newState)
                new_enabled |= M_GRAVITY;
            found = 1;
            break;
        case ID_DR:
            new_enabled &= ~M_DISP_ROTATE;
            if (newState)
                new_enabled |= M_DISP_ROTATE;
            found = 1;
            break;
        case ID_DB:
            new_enabled &= ~M_DISP_BRIGHTNESS;
            if (newState)
                new_enabled |= M_DISP_BRIGHTNESS;
            found = 1;
            break;
        case ID_UNCALIB_GYRO:
            new_enabled &= ~M_UNCALIB_GYRO;
            if (newState)
                new_enabled |= M_UNCALIB_GYRO;
            found = 1;
            break;
	case ID_UNCALIB_MAG:
            new_enabled &= ~M_UNCALIB_MAG;
            if (newState)
                new_enabled |= M_UNCALIB_MAG;
            found = 1;
            break;
	case ID_STEP_COUNTER:
            new_enabled &= ~M_STEP_COUNTER;
            if (newState)
                new_enabled |= M_STEP_COUNTER;
            found = 1;
            break;
	case ID_STEP_DETECTOR:
            new_enabled &= ~M_STEP_DETECTOR;
            if (newState)
                new_enabled |= M_STEP_DETECTOR;
            found = 1;
            break;
    }

    if (found && (new_enabled != mEnabled)) {
        err = ioctl(dev_fd, STM401_IOCTL_SET_SENSORS, &new_enabled);
        ALOGE_IF(err, "Could not change sensor state (%s)", strerror(-err));
        if (!err) {
            mEnabled = new_enabled;
        }
    }

    new_enabled = mWakeEnabled;
    found = 0;
    switch (handle) {
        case ID_D:
            new_enabled &= ~M_DOCK;
            if (newState)
                new_enabled |= M_DOCK;
            found = 1;
            break;
        case ID_P:
            new_enabled &= ~M_PROXIMITY;
            if (newState)
                new_enabled |= M_PROXIMITY;
            found = 1;
            break;
        case ID_FU:
            new_enabled &= ~M_FLATUP;
            if (newState)
                new_enabled |= M_FLATUP;
            found = 1;
            break;
        case ID_FD:
            new_enabled &= ~M_FLATDOWN;
            if (newState)
                new_enabled |= M_FLATDOWN;
            found = 1;
            break;
        case ID_S:
            new_enabled &= ~M_STOWED;
            if (newState)
                new_enabled |= M_STOWED;
            found = 1;
            break;
        case ID_CA:
            new_enabled &= ~M_CAMERA_ACT;
            if (newState)
                new_enabled |= M_CAMERA_ACT;
            found = 1;
            break;
        case ID_NFC:
            new_enabled &= ~M_NFC;
            if (newState)
                new_enabled |= M_NFC;
            else {
                FILE *fp;
                int i;

                err = ioctl(dev_fd, STM401_IOCTL_GET_MAG_CAL, &mMagCal);
                if (err < 0) {
                    ALOGE("Can't read Mag Cal data");
                } else {
                    if ((fp = fopen(MAG_CAL_FILE, "w")) == NULL) {
                        ALOGE("Can't open Mag Cal file");
                    } else {
                        for (i=0; i<STM401_MAG_CAL_SIZE; i++) {
                            fputc(mMagCal[i], fp);
                        }
                        fclose(fp);
                    }
                }
            }
            found = 1;
            break;
        case ID_SIM:
            new_enabled &= ~M_SIM;
            if (newState)
                new_enabled |= M_SIM;
            found = 1;
            break;
    }

    if (found && (new_enabled != mWakeEnabled)) {
        err = ioctl(dev_fd, STM401_IOCTL_SET_WAKESENSORS, &new_enabled);
        ALOGE_IF(err, "Could not change sensor state (%s)", strerror(-err));
        if (!err) {
            mWakeEnabled = new_enabled;
        }
    }

    return err;
}

int HubSensor::setDelay(int32_t handle, int64_t ns)
{
    int status = -EINVAL;

    if (ns < 0)
        return -EINVAL;

    unsigned short delay = int64_t(ns) / 1000000;
    switch (handle) {
        case ID_A: status = ioctl(dev_fd,  STM401_IOCTL_SET_ACC_DELAY, &delay);   break;
        case ID_G: status = ioctl(dev_fd,  STM401_IOCTL_SET_GYRO_DELAY, &delay);  break;
        case ID_PR: status = ioctl(dev_fd,  STM401_IOCTL_SET_PRES_DELAY, &delay); break;
        case ID_M: /* Mag and orientation get set together */
        case ID_O: status = ioctl(dev_fd,  STM401_IOCTL_SET_MAG_DELAY, &delay);   break;
        case ID_T: status = 0;                                                    break;
        case ID_L: status = 0;                                                    break;
        case ID_LA: status = 0;                                                   break;
        case ID_Q: status = 0;                                                    break;
        case ID_GR: status = 0;                                                   break;
        case ID_DR: status = 0;                                                   break;
        case ID_DB: status = 0;                                                   break;
        case ID_D: status = 0;                                                    break;
        case ID_P: status = 0;                                                    break;
        case ID_FU: status = 0;                                                   break;
        case ID_FD: status = 0;                                                   break;
        case ID_S: status = 0;                                                    break;
        case ID_CA: status = 0;                                                   break;
        case ID_NFC: status = 0;                                                  break;
        case ID_SIM: status = 0;                                                  break;
        case ID_UNCALIB_GYRO: status = ioctl(dev_fd,  STM401_IOCTL_SET_GYRO_DELAY, &delay); break;
        case ID_UNCALIB_MAG: status = ioctl(dev_fd,  STM401_IOCTL_SET_MAG_DELAY, &delay);  break;
        case ID_STEP_COUNTER:
		    delay /= 1000; // convert to seconds for pedometer rate
		    if (delay == 0)
		        delay = 1;
		    status = ioctl(dev_fd,  STM401_IOCTL_SET_STEP_COUNTER_DELAY, &delay);
		    break;
        case ID_STEP_DETECTOR:status = 0;                                         break;
    }
    return status;
}

int HubSensor::readEvents(sensors_event_t* data, int count)
{
    int numEventReceived = 0;
    struct stm401_android_sensor_data buff;
    int ret;
    char timeBuf[32];
    struct tm* ptm = NULL;
    struct timeval timeutc;
    static long int sent_bug2go_sec = 0;

    if (count < 1)
        return -EINVAL;
    while (((ret = read(data_fd, &buff, sizeof(struct stm401_android_sensor_data))) != 0)  && count) {
        /* these sensors are not supported, upload a bug2go if its been at least 10mins since previous bug2go*/
        /* remove this if-clause when corruption issue resolved */
        if (buff.type == DT_PRESSURE || buff.type == DT_TEMP || buff.type == DT_LIN_ACCEL ||
            buff.type == DT_GRAVITY || buff.type == DT_DOCK || buff.type == DT_QUATERNION ||
            buff.type == DT_NFC) {
            count--;
            time(&timeutc.tv_sec);
            if ((sent_bug2go_sec == 0) ||
                (timeutc.tv_sec - sent_bug2go_sec > 60*10)) {
                // put timestamp in dropbox file
                ptm = localtime(&(timeutc.tv_sec));
                if (ptm != NULL) {
                    strftime(timeBuf, sizeof(timeBuf), "%m-%d %H:%M:%S", ptm);
                    capture_dump(timeBuf, buff.type, SENSORHUB_DUMPFILE,
                        DROPBOX_FLAG_TEXT | DROPBOX_FLAG_GZIP);
                }
                sent_bug2go_sec = timeutc.tv_sec;
            }
            continue;
        }

        switch (buff.type) {
            case DT_ACCEL:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_A;
                data->type = SENSOR_TYPE_ACCELEROMETER;
                data->acceleration.x = STM16TOH(buff.data+ACCEL_X) * CONVERT_A_X;
                data->acceleration.y = STM16TOH(buff.data+ACCEL_Y) * CONVERT_A_Y;
                data->acceleration.z = STM16TOH(buff.data+ACCEL_Z) * CONVERT_A_Z;
                data->acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_GYRO:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_G;
                data->type = SENSOR_TYPE_GYROSCOPE;
                data->gyro.x = STM16TOH(buff.data + GYRO_X) * CONVERT_G_P;
                data->gyro.y = STM16TOH(buff.data + GYRO_Y) * CONVERT_G_R;
                data->gyro.z = STM16TOH(buff.data + GYRO_Z) * CONVERT_G_Y;
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_UNCALIB_GYRO:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_UNCALIB_GYRO;
                data->type = SENSOR_TYPE_GYROSCOPE_UNCALIBRATED;
                data->uncalibrated_gyro.x_uncalib = STM16TOH(buff.data + UNCALIB_GYRO_X) * CONVERT_G_P;
                data->uncalibrated_gyro.y_uncalib = STM16TOH(buff.data + UNCALIB_GYRO_Y) * CONVERT_G_R;
                data->uncalibrated_gyro.z_uncalib = STM16TOH(buff.data + UNCALIB_GYRO_Z) * CONVERT_G_Y;
                data->uncalibrated_gyro.x_bias = STM16TOH(buff.data + UNCALIB_GYRO_X_BIAS) * CONVERT_BIAS_G_P;
                data->uncalibrated_gyro.y_bias = STM16TOH(buff.data + UNCALIB_GYRO_Y_BIAS) * CONVERT_BIAS_G_R;
                data->uncalibrated_gyro.z_bias = STM16TOH(buff.data + UNCALIB_GYRO_Z_BIAS) * CONVERT_BIAS_G_Y;
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_UNCALIB_MAG:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_UNCALIB_MAG;
                data->type = SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED;
                data->uncalibrated_magnetic.x_uncalib = STM16TOH(buff.data + UNCALIB_MAGNETIC_X) * CONVERT_M_X;
                data->uncalibrated_magnetic.y_uncalib = STM16TOH(buff.data + UNCALIB_MAGNETIC_Y) * CONVERT_M_Y;
                data->uncalibrated_magnetic.z_uncalib = STM16TOH(buff.data + UNCALIB_MAGNETIC_Z) * CONVERT_M_Z;
                data->uncalibrated_magnetic.x_bias = STM16TOH(buff.data + UNCALIB_MAGNETIC_X_BIAS) * CONVERT_BIAS_M_X;
                data->uncalibrated_magnetic.y_bias = STM16TOH(buff.data + UNCALIB_MAGNETIC_Y_BIAS) * CONVERT_BIAS_M_Y;
                data->uncalibrated_magnetic.z_bias = STM16TOH(buff.data + UNCALIB_MAGNETIC_Z_BIAS) * CONVERT_BIAS_M_Z;
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_STEP_COUNTER:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_STEP_COUNTER;
                data->type = SENSOR_TYPE_STEP_COUNTER;
                data->u64.step_counter =  (
                        (((uint64_t)STM16TOH(buff.data + STEP_COUNTER_3)) << 48) |
                        (((uint64_t)STM16TOH(buff.data + STEP_COUNTER_2)) << 32) |
                        (((uint64_t)STM16TOH(buff.data + STEP_COUNTER_1)) << 16) |
                        (((uint64_t)STM16TOH(buff.data + STEP_COUNTER_0))) );
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_STEP_DETECTOR:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_STEP_DETECTOR;
                data->type = SENSOR_TYPE_STEP_DETECTOR;
                data->data[0] = STM16TOH(buff.data + STEP_DETECTOR);
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_PRESSURE:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_PR;
                data->type = SENSOR_TYPE_PRESSURE;
                data->pressure = STM32TOH(buff.data + PRESSURE_PRESSURE) * CONVERT_B;
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_MAG:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_M;
                data->type = SENSOR_TYPE_MAGNETIC_FIELD;
                data->magnetic.x = STM16TOH(buff.data + MAGNETIC_X) * CONVERT_M_X;
                data->magnetic.y = STM16TOH(buff.data + MAGNETIC_Y) * CONVERT_M_Y;
                data->magnetic.z = STM16TOH(buff.data + MAGNETIC_Z) * CONVERT_M_Z;
                data->magnetic.status = buff.status;
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_ORIENT:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_O;
                data->type = SENSOR_TYPE_ORIENTATION;
                data->orientation.azimuth = STM16TOH(buff.data + ORIENTATION_AZIMUTH) * CONVERT_O_Y;
                data->orientation.pitch = STM16TOH(buff.data + ORIENTATION_PITCH) * CONVERT_O_P;
                // Roll value needs to be negated.
                data->orientation.roll = -STM16TOH(buff.data + ORIENTATION_ROLL) * CONVERT_O_R;
                data->orientation.status = buff.status;
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_TEMP:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_T;
                data->type = SENSOR_TYPE_TEMPERATURE;
                data->temperature = STM16TOH(buff.data + TEMPERATURE_TEMPERATURE) * CONVERT_T;
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_ALS:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_L;
                data->type = SENSOR_TYPE_LIGHT;
                data->light = (uint16_t)STM16TOH(buff.data + LIGHT_LIGHT);
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_LIN_ACCEL:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_LA;
                data->type = SENSOR_TYPE_LINEAR_ACCELERATION;
                data->acceleration.x = STM16TOH(buff.data + ACCEL_X) * CONVERT_A_LIN;
                data->acceleration.y = STM16TOH(buff.data + ACCEL_Y) * CONVERT_A_LIN;
                data->acceleration.z = STM16TOH(buff.data + ACCEL_Z) * CONVERT_A_LIN;
                data->acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_QUATERNION:
                break;
            case DT_GRAVITY:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_GR;
                data->type = SENSOR_TYPE_GRAVITY;
                data->acceleration.x = STM16TOH(buff.data + ACCEL_X) * CONVERT_A_GRAV;
                data->acceleration.y = STM16TOH(buff.data + ACCEL_Y) * CONVERT_A_GRAV;
                data->acceleration.z = STM16TOH(buff.data + ACCEL_Z) * CONVERT_A_GRAV;
                data->acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_DISP_ROTATE:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_DR;
                data->type = SENSOR_TYPE_DISPLAY_ROTATE;
                if (buff.data[ROTATE_ROTATE] == DISP_FLAT)
                    data->data[0] = DISP_UNKNOWN;
                else
                    data->data[0] = buff.data[ROTATE_ROTATE];

                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_DISP_BRIGHT:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_DB;
                data->type = SENSOR_TYPE_DISPLAY_BRIGHTNESS;
                data->data[0] = buff.data[BRIGHT_BRIGHT];
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_DOCK:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_D;
                data->type = SENSOR_TYPE_DOCK;
                data->data[0] = buff.data[DOCK_DOCK];
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_PROX:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_P;
                data->type = SENSOR_TYPE_PROXIMITY;
                if (buff.data[PROXIMITY_PROXIMITY] == 0) {
                    data->distance = PROX_UNCOVERED;
                    ALOGE("Proximity uncovered");
		} else if (buff.data[PROXIMITY_PROXIMITY] == 1) {
                    data->distance = PROX_COVERED;
                    ALOGE("Proximity covered 1");
                } else {
                    data->distance = PROX_SATURATED;
                    ALOGE("Proximity covered 2");
                }
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_FLAT_UP:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_FU;
                data->type = SENSOR_TYPE_FLAT_UP;
                if (buff.data[FLAT_FLAT] == 0x01)
                    data->data[0] = FLAT_DETECTED;
                else
                    data->data[0] = FLAT_NOTDETECTED;
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_FLAT_DOWN:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_FD;
                data->type = SENSOR_TYPE_FLAT_DOWN;
                if (buff.data[FLAT_FLAT] == 0x02)
                    data->data[0] = FLAT_DETECTED;
                else
                    data->data[0] = FLAT_NOTDETECTED;
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_STOWED:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_S;
                data->type = SENSOR_TYPE_STOWED;
                data->data[0] = buff.data[STOWED_STOWED];
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_CAMERA_ACT:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_CA;
                data->type = SENSOR_TYPE_CAMERA_ACTIVATE;
                data->data[0] = STM401_CAMERA_DATA;
                data->data[1] = STM16TOH(buff.data + CAMERA_CAMERA);
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_NFC:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_NFC;
                data->type = SENSOR_TYPE_NFC_DETECT;
                data->data[0] = buff.data[NFC_NFC];
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                break;
            case DT_SIM:
                data->version = SENSORS_EVENT_T_SIZE;
                data->sensor = ID_SIM;
                data->type = SENSOR_TYPE_SIGNIFICANT_MOTION;
                data->data[0] = STM16TOH(buff.data + SIM);
                data->timestamp = buff.timestamp;
                data++;
                count--;
                numEventReceived++;
                enable(ID_SIM, 0);
                break;
            case DT_RESET:
                count--;
                // put timestamp in dropbox file
                time(&timeutc.tv_sec);
                ptm = localtime(&(timeutc.tv_sec));
                if (ptm != NULL) {
                    strftime(timeBuf, sizeof(timeBuf), "%m-%d %H:%M:%S", ptm);
                    capture_dump(timeBuf, buff.data[0], SENSORHUB_DUMPFILE,
                         DROPBOX_FLAG_TEXT | DROPBOX_FLAG_GZIP);
                }
                break;
            default:
                break;
        }
    }

    return numEventReceived;
}

gzFile HubSensor::open_dropbox_file(const char* timestamp, const char* dst, const int flags)
{
    char dropbox_path[128];
    pid_t pid = getpid();

    snprintf(dropbox_path, sizeof(dropbox_path), "%s/%s:%d:%u-%s",
             DROPBOX_DIR, DROPBOX_TAG, flags, pid, timestamp);
    ALOGD("stm401 - dumping to dropbox file[%s]...\n", dropbox_path);

    return gzopen(dropbox_path, "wb");
}

short HubSensor::capture_dump(char* timestamp, const int id, const char* dst, const int flags)
{
    char buffer[COPYSIZE] = {0};
    int rc = 0;
    gzFile dropbox_file = NULL;

    dropbox_file = open_dropbox_file(timestamp, dst, flags);
    if(dropbox_file == NULL) {
        ALOGE("ERROR! unable to open dropbox file[errno:%d(%s)]\n", errno, strerror(errno));
    } else {
        // put timestamp in dropbox file
        rc = snprintf(buffer, COPYSIZE, "timestamp:%s\n", timestamp);
        gzwrite(dropbox_file, buffer, rc);
        rc = snprintf(buffer, COPYSIZE, "reason:%02d\n", id);
        gzwrite(dropbox_file, buffer, rc);
        gzclose(dropbox_file);
        // to commit buffer cache to disk
        sync();
    }

    return 0;
}
