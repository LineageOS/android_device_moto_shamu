/*
 * Copyright (C) 2014 The Android Open Source Project
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
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/un.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdbool.h>

#define LOG_TAG "PowerHAL"
#include <utils/Log.h>

#include <hardware/hardware.h>
#include <hardware/power.h>

#define TOUCH_INTERACTIVE_PATH "/sys/bus/i2c/devices/1-004a/tsp"

static int sysfs_write(const char *path, char *s)
{
    char buf[80];
    int len;
    int fd = open(path, O_WRONLY);

    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);
        return -1;
    }

    len = write(fd, s, strlen(s));
    if (len < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error writing to %s: %s\n", path, buf);
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

static void power_init(__attribute__((unused)) struct power_module *module)
{
    ALOGI("%s", __func__);
}

static void power_set_interactive(__attribute__((unused)) struct power_module *module, int on)
{
    ALOGV("%s %s", __func__, (on ? "ON" : "OFF"));
    sysfs_write(TOUCH_INTERACTIVE_PATH, on ? "AUTO" : "ON");
}

static void power_hint( __attribute__((unused)) struct power_module *module,
                        __attribute__((unused)) power_hint_t hint,
                        __attribute__((unused)) void *data)
{
}

static struct hw_module_methods_t power_module_methods = {
    .open = NULL,
};

struct power_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = POWER_MODULE_API_VERSION_0_2,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = POWER_HARDWARE_MODULE_ID,
        .name = "Shamu Power HAL",
        .author = "The Android Open Source Project",
        .methods = &power_module_methods,
    },

    .init = power_init,
    .setInteractive = power_set_interactive,
    .powerHint = power_hint,
};
