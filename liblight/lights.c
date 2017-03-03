/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (C) 2014 The  Linux Foundation. All rights reserved.
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

// #define LOG_NDEBUG 0

#define LOG_TAG "shamu-lights"
#include <cutils/log.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <hardware/lights.h>

/******************************************************************************/

static pthread_once_t g_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static struct light_state_t g_notification;
static struct light_state_t g_battery;

char const*const RED_LED_FILE
        = "/sys/class/leds/red/brightness";

char const*const GREEN_LED_FILE
        = "/sys/class/leds/green/brightness";

char const*const BLUE_LED_FILE
        = "/sys/class/leds/blue/brightness";

char const*const RED_BLINK_LED_FILE
        = "/sys/class/leds/red/blink";

char const*const GREEN_BLINK_LED_FILE
        = "/sys/class/leds/green/blink";

char const*const BLUE_BLINK_LED_FILE
        = "/sys/class/leds/blue/blink";

char const*const LCD_FILE
        = "/sys/class/leds/lcd-backlight/brightness";

typedef struct { int red; int green; int blue;} LedIdentifier;
typedef struct { bool red; bool green; bool blue;} LedIdentifierRGB;

/**
 * device methods
 */

void init_globals(void)
{
    // init the mutex
    pthread_mutex_init(&g_lock, NULL);

    memset(&g_battery, 0, sizeof(g_battery));
    memset(&g_notification, 0, sizeof(g_notification));
}

static int
write_int(char const* path, int value)
{
    int fd;

    fd = open(path, O_RDWR);
    if (fd >= 0) {
        char buffer[20];
        int bytes = sprintf(buffer, "%d\n", value);
        ssize_t amt = write(fd, buffer, (size_t)bytes);
        close(fd);
        return amt == -1 ? -errno : 0;
    } else {
        ALOGE("write_int failed to open %s\n", path);
        return -errno;
    }
}

static int
write_rgb_str(LedIdentifierRGB rgb_leds, char *value)
{
    LedIdentifier fd;

    fd.red = open(RED_BLINK_LED_FILE, O_RDWR);
    fd.green = open(GREEN_BLINK_LED_FILE, O_RDWR);
    fd.blue = open(BLUE_BLINK_LED_FILE, O_RDWR);
    if (fd.red >= 0 && fd.green >= 0 && fd.blue >= 0) {
        LedIdentifier amt;
        const char *nullbytes = "0,0";
        if (rgb_leds.red) {
            amt.red = write(fd.red, value, strlen(value) + 1);
        } else {
            amt.red = write(fd.red, nullbytes, strlen(nullbytes) + 1);
        }
        if (rgb_leds.green) {
            amt.green = write(fd.green, value, strlen(value) + 1);
        } else {
            amt.green = write(fd.green, nullbytes, strlen(nullbytes) + 1);
        }
        if (rgb_leds.blue) {
            amt.blue = write(fd.blue, value, strlen(value) + 1);
        } else {
            amt.blue = write(fd.blue, nullbytes, strlen(nullbytes) + 1);
        }
        close(fd.red);
        close(fd.green);
        close(fd.blue);
        return (amt.red == -1 || amt.green == -1 || amt.blue == -1) ? -errno : 0;
    } else {
        if (fd.red != -1) {
            close(fd.red);
        }
        if (fd.green != -1) {
            close(fd.green);
        }
        if (fd.blue != -1) {
            close(fd.blue);
        }
        ALOGE("write_str failed to open LED Paths");
        return -errno;
    }
}

static int
is_lit(struct light_state_t const* state)
{
    return state->color & 0x00ffffff;
}

static int
rgb_to_brightness(struct light_state_t const* state)
{
    int color = state->color & 0x00ffffff;
    return ((77*((color>>16)&0x00ff))
            + (150*((color>>8)&0x00ff)) + (29*(color&0x00ff))) >> 8;
}

static int
set_light_backlight(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int err = 0;
    int brightness = rgb_to_brightness(state);
    if(!dev) {
        return -1;
    }
    pthread_mutex_lock(&g_lock);
    err = write_int(LCD_FILE, brightness);
    pthread_mutex_unlock(&g_lock);
    return err;
}

/**
 * There is a sperate gpio per led. Each with a maximum brightness of 20.
 * The normal rgb values are computed on a scale up to 255 so we divide them
 * by 12.75 so that varied brightness per led is spread out evenly per color.
 * The led is only optimal on absolute red, green or blue, and produces artifacts
 * of two different colors appearing at once when more than one is active, but
 * this is very limiting so ultimately we allow the end user full control.
 **/
static int
set_speaker_light_locked(struct light_device_t* dev,
        struct light_state_t const* state)
{
    unsigned int colorRGB = state->color;
    LedIdentifier rgb_leds;
    unsigned long onMS, offMS;
    char blink_string[PAGE_SIZE];

    rgb_leds.red = (((colorRGB >> 16) & 0xFF) / 12.75);
    rgb_leds.green = (((colorRGB >> 8) & 0xFF) / 12.75);
    rgb_leds.blue = ((colorRGB & 0xFF) / 12.75);

    write_int(RED_LED_FILE, rgb_leds.red);
    write_int(GREEN_LED_FILE, rgb_leds.green);
    write_int(BLUE_LED_FILE, rgb_leds.blue);

    LedIdentifierRGB rgb_leds_val = {rgb_leds.red > 5, rgb_leds.green > 5, rgb_leds.blue > 5};

    switch (state->flashMode) {
        case LIGHT_FLASH_TIMED:
            onMS = state->flashOnMS;
            offMS = state->flashOffMS;
            break;
        case LIGHT_FLASH_NONE:
        default:
            onMS = 0;
            offMS = 0;
            break;
    }

    if (!(onMS == 1 && offMS == 0)) {
        // We can only blink at full brightness when someone doesn't want a blinking LED
        // don't write the blink code, so all colors can be used.
        sprintf(blink_string, "%lu,%lu", onMS, offMS);
        write_rgb_str(rgb_leds_val, blink_string);
    }
    return 0;
}

static void
handle_speaker_battery_locked(struct light_device_t* dev)
{
    if (is_lit(&g_notification)) {
        set_speaker_light_locked(dev, &g_notification);
    } else {
        set_speaker_light_locked(dev, &g_battery);
    }
}

static int
set_light_notifications(struct light_device_t* dev,
        struct light_state_t const* state)
{
    pthread_mutex_lock(&g_lock);
    g_notification = *state;
    handle_speaker_battery_locked(dev);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

static int
set_light_battery(struct light_device_t* dev,
        struct light_state_t const* state)
{
    pthread_mutex_lock(&g_lock);
    g_battery = *state;
    handle_speaker_battery_locked(dev);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

/** Close the lights device */
static int
close_lights(struct light_device_t *dev)
{
    if (dev) {
        free(dev);
    }
    return 0;
}


/******************************************************************************/

/**
 * module methods
 */

/** Open a new instance of a lights device using name */
static int open_lights(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
    int (*set_light)(struct light_device_t* dev,
            struct light_state_t const* state);

    if (0 == strcmp(LIGHT_ID_BACKLIGHT, name))
        set_light = set_light_backlight;
    else if (0 == strcmp(LIGHT_ID_NOTIFICATIONS, name))
        set_light = set_light_notifications;
    else if (0 == strcmp(LIGHT_ID_BATTERY, name))
        set_light = set_light_battery;
    else
        return -EINVAL;

    pthread_once(&g_init, init_globals);

    struct light_device_t *dev = malloc(sizeof(struct light_device_t));
    memset(dev, 0, sizeof(*dev));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = (int (*)(struct hw_device_t*))close_lights;
    dev->set_light = set_light;

    *device = (struct hw_device_t*)dev;
    return 0;
}

static struct hw_module_methods_t lights_module_methods = {
    .open =  open_lights,
};

/*
 * The lights Module
 */
struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = LIGHTS_HARDWARE_MODULE_ID,
    .name = "shamu lights module",
    .author = "Google, Inc.",
    .methods = &lights_module_methods,
};
