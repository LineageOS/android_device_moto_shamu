/*
 * Copyright (C) 2011-2012 Motorola Mobility, Inc.
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

#define LOG_TAG "sensorhub"

#include <cutils/log.h>

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/android_alarm.h>
#include <linux/input.h>
#include <linux/stm401.h>

#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/types.h>

#include <hardware/mot_sensorhub_stm401.h>

/* paths to the driver fds */
#define DRIVER_CONTROL_PATH "/dev/stm401"
#define DRIVER_DATA_NAME "/dev/stm401_ms"

/* Constants and helper macro for accessing sensor hub event data. */
#define MOVE_VALUE 0

#define ALGO_PAST       0
#define ALGO_CONFIDENCE 0
#define ALGO_ID         0
#define ALGO_TIME       1
#define ALGO_OLDSTATE   1
#define ALGO_NEWSTATE   3
#define ALGO_DISTANCE   3
#define ALGO_MS         5
#define ALGO_ALGO       7

#define GENERIC_INT_OFFSET 0

#define STMLE16TOH(p) (int16_t) le16toh(*((uint16_t *) (p)))

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static uint8_t num_parts[SENSORHUB_NUM_ALGOS] = {
    SENSORHUB_NUM_MODALITIES, SENSORHUB_NUM_ORIENTATIONS, SENSORHUB_NUM_STOWED,
    SENSORHUB_NUM_ACCUM_REQS, 0, 0 };

static uint16_t algo_bits[SENSORHUB_NUM_ALGOS] = {
    M_ALGO_MODALITY, M_ALGO_ORIENTATION, M_ALGO_STOWED,
    M_ALGO_ACCUM_MODALITY, M_ALGO_ACCUM_MVMT, 0 };

struct sensorhub_context_t {
    struct sensorhub_device_t device;
    int control_fd;
    struct pollfd data_pollfd;
    uint16_t active_algos;
    uint32_t active_parts[SENSORHUB_NUM_ALGOS];
};

static int64_t get_wall_clock()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (int64_t)((time.tv_sec*(int64_t)1000) + (time.tv_usec/(int64_t)1000));
}

static int64_t get_elapsed_realtime()
{
    struct timespec ts;
    int fd, result;

    fd = open("/dev/alarm", O_RDONLY);
    if (fd < 0)
        return fd;

   result = ioctl(fd, ANDROID_ALARM_GET_TIME(ANDROID_ALARM_ELAPSED_REALTIME), &ts);
   close(fd);

    if (result == 0)
        return (int64_t)((ts.tv_sec*(int64_t)1000) + (ts.tv_nsec/(int64_t)1000000));
    return -1;
}

static int sensorhub_enable(struct sensorhub_device_t* device, struct sensorhub_algo_t* algo)
{
    struct sensorhub_context_t* context = (struct sensorhub_context_t*)device;
    int error = 0;
    unsigned int data;

    pthread_mutex_lock(&g_lock);

    switch (algo->type) {
    case SENSORHUB_ALGO_MOVEMENT:
        if (algo->enable) {
            data = algo->parameter[0];
            if (ioctl(context->control_fd, STM401_IOCTL_SET_MOTION_DUR, &data) < 0) {
                ALOGE("STM401_IOCTL_SET_MOTION_DUR error (%s)", strerror(errno));
                error = -errno;
            }
            data = algo->parameter[1];
            if (ioctl(context->control_fd, STM401_IOCTL_SET_ZRMOTION_DUR, &data) < 0) {
                ALOGE("STM401_IOCTL_SET_ZRMOTION_DUR error (%s)", strerror(errno));
                error = -errno;
            }
            data = context->active_algos | (M_MMOVEME | M_NOMMOVE);
        } else {
            data = context->active_algos & ~(M_MMOVEME | M_NOMMOVE);
        }
        break;
    }

    if (!error) {
        if (ioctl(context->control_fd, STM401_IOCTL_SET_ALGOS, &data) < 0) {
            ALOGE("STM401_IOCTL_SET_ALGOS error (%s)", strerror(errno));
            error = -errno;
        }
    }
    if (!error) {
        context->active_algos = data;
    }

    pthread_mutex_unlock(&g_lock);
    return error;
}

static int sensorhub_algo_req(struct sensorhub_device_t* device, uint16_t algo,
        uint32_t active_parts, struct sensorhub_req_t* req_info)
{
    struct sensorhub_context_t* context = (struct sensorhub_context_t*)device;
    int i, error = 0;
    uint8_t req_len;
    uint16_t algos;

    if (algo == SENSORHUB_ALGO_ACCUM_MVMT) {
        req_len = sizeof(sensorhub_accum_mvmt_req_t);
    } else
    if (algo == SENSORHUB_ALGO_ACCUM_MODALITY) {
        req_len = (sizeof(sensorhub_accum_state_req_t) * num_parts[algo]);
    } else {
        req_len = sizeof(active_parts) + (sizeof(req_info[0].durations) * num_parts[algo]);
    }

    unsigned char bytes[sizeof(algo) + sizeof(req_len) + req_len];

    pthread_mutex_lock(&g_lock);
    ALOGD("sensorhub_algo_req(): algo: %d, active_parts: %d, num_parts: %d, req_len: %d, bytes: %d",
        algo, active_parts, num_parts[algo], req_len, sizeof(bytes));

    algos = context->active_algos;
    if (!context->active_parts[algo] && active_parts) {
        algos |= algo_bits[algo];
    } else
    if (context->active_parts[algo] && !active_parts) {
        algos &= ~algo_bits[algo];
    }

    ALOGD("sensorhub_algo_req(): algos: %d", algos);
    // ioctl set algo req = active_parts + req_info for each part
    unsigned char* p = bytes;
    memcpy(p, &algo, sizeof(algo));
    p += sizeof(algo);
    memcpy(p, &req_len, sizeof(req_len));
    p += sizeof(req_len);

    if (algo == SENSORHUB_ALGO_ACCUM_MVMT) {
        memcpy(p, &req_info->am_req, sizeof(req_info->am_req));
    } else
    if (algo == SENSORHUB_ALGO_ACCUM_MODALITY) {
        ALOGD("sensorhub_algo_accum_modality(): about to copy req_info size: %d", req_len);
        for (i = 0; i < num_parts[algo]; i++) {
            memcpy(p, &req_info[i].as_req, sizeof(req_info[i].as_req));
            p += sizeof(req_info[i].as_req);
        }
    } else {
        memcpy(p, &active_parts, sizeof(active_parts));
        p += sizeof(active_parts);
        for (i = 0; i < num_parts[algo]; i++) {
            memcpy(p, req_info[i].durations, sizeof(req_info[i].durations));
            p += sizeof(req_info[i].durations);
        }
    }

    if (ioctl(context->control_fd, STM401_IOCTL_SET_ALGO_REQ, bytes) < 0) {
        ALOGE("STM401_IOCTL_SET_ALGO_REQ error (%s)", strerror(errno));
        error = -errno;
    } else {
        context->active_parts[algo] = active_parts;

        // ioctl set algos
        if (ioctl(context->control_fd, STM401_IOCTL_SET_ALGOS, &algos) < 0) {
            ALOGE("STM401_IOCTL_SET_ALGOS error (%s)", strerror(errno));
            error = -errno;
        } else {
            context->active_algos = algos;
        }
    }
    pthread_mutex_unlock(&g_lock);
    return error;
}

static int sensorhub_algo_query(struct sensorhub_device_t* device, uint16_t algo,
    struct sensorhub_event_t* output)
{
    struct sensorhub_context_t* context = (struct sensorhub_context_t*)device;
    int i, evt_reg_size, error = 0;

    if (algo == SENSORHUB_ALGO_ACCUM_MVMT) {
        evt_reg_size = STM401_EVT_SZ_ACCUM_MVMT;
    } else {
        evt_reg_size = STM401_EVT_SZ_TRANSITION;
    }

    unsigned char bytes[sizeof(algo) + evt_reg_size];

    pthread_mutex_lock(&g_lock);
    ALOGD("sensorhub_algo_query(): algo: %d", algo);

    memcpy(bytes, &algo, sizeof(algo));

    if (ioctl(context->control_fd, STM401_IOCTL_GET_ALGO_EVT, bytes) < 0) {
        ALOGE("STM401_IOCTL_GET_ALGO_EVT error (%s)", strerror(errno));
        error = -errno;
    } else {
        // just clear time for query output
        output->time = 0;
        output->ertime = 0;
        unsigned char* p_evt = bytes + sizeof(algo);

        if (algo == SENSORHUB_ALGO_ACCUM_MVMT)
        {
            output->type = SENSORHUB_EVENT_ACCUM_MVMT;
            output->time_s = (p_evt[1] << 8) | p_evt[0];
            output->distance = (p_evt[3] << 8) | p_evt[2];
        } else {
            output->type = SENSORHUB_EVENT_TRANSITION;
            output->algo = algo;
            output->past = (p_evt[0] & 0x80) > 0;
            output->confidence = p_evt[0] & 0x7F;
            output->old_state = (p_evt[2] << 8) | p_evt[1];
            output->new_state = (p_evt[4] << 8) | p_evt[3];
        }
    }
    pthread_mutex_unlock(&g_lock);
    return error;
}

static int sensorhub_poll(struct sensorhub_device_t* device, struct sensorhub_event_t* event)
{
    struct sensorhub_context_t* context = (struct sensorhub_context_t*)device;
    struct stm401_moto_sensor_data buff;
    int64_t elapsed_ms;
    uint16_t algo;
    int ret;

    ALOGD("sensorhub_poll() polling...");
    ret = poll(&(context->data_pollfd), 1, -1);
    if (ret < 0) {
        ALOGE("poll() failed (%s)", strerror(errno));
        return -errno;
    }

    ret = read(context->data_pollfd.fd, &buff, sizeof(struct stm401_moto_sensor_data));
    if (ret == 0)
        return 0;

    switch (buff.type) {
        case DT_MMMOVE:
            event->type = SENSORHUB_EVENT_START_MOVEMENT;
            event->value[2] = buff.data[MOVE_VALUE] >> 4;
            break;
        case DT_NOMOVE:
            event->type = SENSORHUB_EVENT_END_MOVEMENT;
            event->value[2] = buff.data[MOVE_VALUE] >> 4;
            break;
        case DT_ALGO_EVT:
            // These events are sent little endian and are different from
            // all other sensorhub data which are sent big endian.
            algo = buff.data[ALGO_ALGO];
            event->time = get_wall_clock();
            elapsed_ms = get_elapsed_realtime();
            event->ertime = elapsed_ms > 0 ? elapsed_ms : 0;
            if (algo == SENSORHUB_ALGO_ACCUM_MVMT) {
                event->type = SENSORHUB_EVENT_ACCUM_MVMT;
                event->time_s = STMLE16TOH(buff.data + ALGO_TIME);
                event->distance = STMLE16TOH(buff.data + ALGO_DISTANCE);
                //ALOGD("sensorhub_poll(): accum mvmt: time_s: %d, distance: %d",
                //    event->time_s, event->distance);
            } else
            if (algo == SENSORHUB_ALGO_ACCUM_MODALITY) {
                event->type = SENSORHUB_EVENT_ACCUM_STATE;
                event->accum_algo = algo;
                event->id = STMLE16TOH(buff.data + ALGO_ID);
            } else {
                event->type = SENSORHUB_EVENT_TRANSITION;
                elapsed_ms = STMLE16TOH(buff.data + ALGO_MS) * 1000;
                event->time -= elapsed_ms;
                if (event->ertime > 0)
                    event->ertime -= elapsed_ms;
                event->algo = algo;
                event->past = (buff.data[ALGO_PAST] & 0x80) > 0;
                event->confidence = buff.data[ALGO_CONFIDENCE] & 0x7F;
                event->old_state = STMLE16TOH(buff.data + ALGO_OLDSTATE);
                event->new_state = STMLE16TOH(buff.data + ALGO_NEWSTATE);
                //ALOGD("sensorhub_poll(): tran: algo: %d, elapsed: %lld, t: %lld, ert: %lld",
                //    event->algo, elapsed_ms, event->time, event->ertime);
            }
            break;
        case DT_GENERIC_INT:
#if 0
            // packaging irq3_status into ertime field
            // of the event
            event->type = SENSORHUB_EVENT_GENERIC_CB;
            event->time = get_wall_clock();
            event->ertime = buff.data[GENERIC_INT_OFFSET];
#endif
            context->data_pollfd.revents = 0;
            return 0;
            break;
        case DT_RESET:
            event->type = SENSORHUB_EVENT_RESET;
            event->time = get_wall_clock();
            break;
    }
    context->data_pollfd.revents = 0;
    return 1;
}

static int sensorhub_close(struct hw_device_t* device)
{
    struct sensorhub_context_t* context = (struct sensorhub_context_t*)device;
    close(context->control_fd);
    close(context->data_pollfd.fd);
    free(context);
    pthread_mutex_destroy(&g_lock);
    return 0;
}

static int sensorhub_open(const struct hw_module_t* module, char const* name, struct hw_device_t** device)
{
    struct sensorhub_context_t* context = calloc(1, sizeof(struct sensorhub_context_t));
    int fd;

    if (!context) {
        ALOGE("%s: Couldn't allocate context.", __func__);
        return -ENOMEM;
    }

    pthread_mutex_init(&g_lock, NULL);

    context->device.common.tag = HARDWARE_DEVICE_TAG;
    context->device.common.version = 0;
    context->device.common.module = (struct hw_module_t*)module;
    context->device.common.close = sensorhub_close;
    context->device.enable = sensorhub_enable;
    context->device.algo_req = sensorhub_algo_req;
    context->device.algo_query = sensorhub_algo_query;
    context->device.poll = sensorhub_poll;

    fd = open(DRIVER_CONTROL_PATH, O_RDWR);
    if (fd < 0) {
        ALOGE("%s: Couldn't open control '%s' (%s)",
                __func__, DRIVER_CONTROL_PATH, strerror(errno));
        free(context);
        return -errno;
    }
    context->control_fd = fd;

    fd = open(DRIVER_DATA_NAME, O_RDONLY);
    if (fd < 0) {
        ALOGE("%s: Couldn't open data '%s' (%s)",
                __func__, DRIVER_DATA_NAME, strerror(errno));
        close(context->control_fd);
        free(context);
        return -errno;
    }
    context->data_pollfd.fd = fd;
    context->data_pollfd.events = POLLIN;

    context->active_algos = 0;

    *device = (struct hw_device_t*)context;
    return 0;
}

static struct hw_module_methods_t sensorhub_module_methods = {
    .open = sensorhub_open,
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 3,
    .version_minor = 0,
    .id = SENSORHUB_HARDWARE_MODULE_ID,
    .name = "Motorola Mobility Smart Fusion module",
    .author = "Motorola Mobility, Inc.",
    .methods = &sensorhub_module_methods,
};
