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

#include <hardware/sensors.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>

#include <poll.h>
#include <pthread.h>

#include <linux/input.h>

#include <cutils/atomic.h>
#include <cutils/log.h>

#include <sys/select.h>

#include "nusensors.h"
#include "stm401_hal.h"

/*****************************************************************************/

struct sensors_poll_context_t {
    struct sensors_poll_device_t device; // must be first

        sensors_poll_context_t();
        ~sensors_poll_context_t();
    int activate(int handle, int enabled);
    int setDelay(int handle, int64_t ns);
    int pollEvents(sensors_event_t* data, int count);

private:
    enum {
        accelgyromag    = 0,
        numSensorDrivers,
    };

    SensorBase* mSensors[numSensorDrivers];
    struct pollfd mPollFds[numSensorDrivers];

    int handleToDriver(int handle) const {
        switch (handle) {
            case ID_A:
            case ID_G:
            case ID_PR:
            case ID_M:
            case ID_O:
            case ID_T:
            case ID_L:
            case ID_LA:
            case ID_Q:
            case ID_GR:
            case ID_DR:
            case ID_DB:
            case ID_D:
            case ID_P:
            case ID_FU:
            case ID_FD:
            case ID_S:
            case ID_CA:
            case ID_NFC:
            case ID_IR_GESTURE:
            case ID_IR_RAW:
            case ID_SIM:
            case ID_STEP_DETECTOR:
            case ID_STEP_COUNTER:
            case ID_UNCALIB_GYRO:
            case ID_UNCALIB_MAG:
                return accelgyromag;
        }
        return -EINVAL;
    }
};

/*****************************************************************************/

sensors_poll_context_t::sensors_poll_context_t()
{
    mSensors[accelgyromag] = new HubSensor();
    mPollFds[accelgyromag].fd = mSensors[accelgyromag]->getFd();
    mPollFds[accelgyromag].events = POLLIN;
    mPollFds[accelgyromag].revents = 0;
}

sensors_poll_context_t::~sensors_poll_context_t() {
    for (int i=0 ; i<numSensorDrivers ; i++) {
        delete mSensors[i];
    }
}

int sensors_poll_context_t::activate(int handle, int enabled) {
    int index = handleToDriver(handle);
    if (index < 0) return index;
    return mSensors[index]->enable(handle, enabled);
}

int sensors_poll_context_t::setDelay(int handle, int64_t ns) {
    int index = handleToDriver(handle);
    if (index < 0) return index;
    return mSensors[index]->setDelay(handle, ns);
}

int sensors_poll_context_t::pollEvents(sensors_event_t* data, int count)
{
    int nbEvents = 0;
    int ret;

    ret = poll(mPollFds, numSensorDrivers, nbEvents ? 0 : -1);
    if (ret < 0) {
        ALOGE("poll() failed (%s)", strerror(errno));
        return -errno;
    }

    if(mPollFds[accelgyromag].revents & POLLIN) {
        SensorBase* const sensor(mSensors[accelgyromag]);
        int nb = sensor->readEvents(data, count);
        count -= nb;
        nbEvents += nb;
        data += nb;
        mPollFds[accelgyromag].revents = 0;
    }

    return nbEvents;
}

/*****************************************************************************/

static int poll__close(struct hw_device_t *dev)
{
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    if (ctx) {
        delete ctx;
    }
    return 0;
}

static int poll__activate(struct sensors_poll_device_t *dev,
        int handle, int enabled) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->activate(handle, enabled);
}

static int poll__setDelay(struct sensors_poll_device_t *dev,
        int handle, int64_t ns) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->setDelay(handle, ns);
}

static int poll__poll(struct sensors_poll_device_t *dev,
        sensors_event_t* data, int count) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->pollEvents(data, count);
}

/*****************************************************************************/

int init_nusensors(hw_module_t const* module, hw_device_t** device)
{
    int status = -EINVAL;

    sensors_poll_context_t *dev = new sensors_poll_context_t();
    memset(&dev->device, 0, sizeof(sensors_poll_device_t));

    dev->device.common.tag = HARDWARE_DEVICE_TAG;
    dev->device.common.version  = 0;
    dev->device.common.module   = const_cast<hw_module_t*>(module);
    dev->device.common.close    = poll__close;
    dev->device.activate        = poll__activate;
    dev->device.setDelay        = poll__setDelay;
    dev->device.poll            = poll__poll;

    *device = &dev->device.common;
    status = 0;
    return status;
}
