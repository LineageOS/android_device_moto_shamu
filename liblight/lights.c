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

#include <cutils/log.h>

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <hardware/lights.h>

/******************************************************************************/

static pthread_once_t g_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * for now we are only interested in
 * battery and notification events
 */
static struct light_state_t g_notification;
static struct light_state_t g_battery;

char const*const RED_LED_FILE
        = "/sys/class/leds/red/brightness";

char const*const GREEN_LED_FILE
        = "/sys/class/leds/green/brightness";

char const*const BLUE_LED_FILE
        = "/sys/class/leds/blue/brightness";

char const*const LCD_FILE
        = "/sys/class/leds/lcd-backlight/brightness";

enum {
  LED_RED = 1,
  LED_GREEN = 2,
  LED_BLUE = 3,
  LED_BLANK = 0
};

/**
 * device methods
 */

void init_globals(void)
{
    // init the mutex
    pthread_mutex_init(&g_lock, NULL);
}

static int
write_int(char const* path, int value)
{
    int fd;
    static int already_warned = 0;

    fd = open(path, O_RDWR);
    if (fd >= 0) {
        char buffer[20];
        int bytes = sprintf(buffer, "%d\n", value);
        ssize_t amt = write(fd, buffer, (size_t)bytes);
        close(fd);
        return amt == -1 ? -errno : 0;
    } else {
        if (already_warned == 0) {
            ALOGE("write_int failed to open %s\n", path);
            already_warned = 1;
        }
        return -errno;
    }
}

static int
is_lit(struct light_state_t const* state)
{
    return state->color & 0x00ffffff;
}

/**
 * battery can only occupy one color at a time
 * this leaves the two other colors open for
 * notifications while charging
 */
static int
get_battery_color(struct light_state_t const* state)
{
  unsigned int colorRGB = state->color & 0xFFFFFF;

  // the highest valued shade is always
  // our battery color
  int red = (colorRGB >> 16) & 0xFF;
  int green = (colorRGB >> 8) & 0xFF;
  int blue = colorRGB & 0xFF;
  int color = 0;
  int ret = LED_BLANK;

  if(red > green) {
    color = red;
    ret = LED_RED;
  } else {
    color = green;
    ret = LED_GREEN;
  }
  if(blue > color) {
    ret = LED_BLUE;
  }
  return ret;
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

static void
set_speaker_light_locked_notification(struct light_device_t* dev,
        struct light_state_t const* bstate, struct light_state_t const* nstate)
{
  unsigned int colorRGB = nstate->color & 0xFFFFFF;
  int red = LED_BLANK;
  int green = LED_BLANK;
  int blue = LED_BLANK;

  if ((colorRGB >> 16) & 0xFF) red = LED_RED;
  if ((colorRGB >> 8) & 0xFF) green = LED_GREEN;
  if (colorRGB & 0xFF) blue = LED_BLUE;

  // notification came in and battery light is off
  // free to write all values
  if (!is_lit(bstate)) {
    write_int (RED_LED_FILE, red);
    write_int (GREEN_LED_FILE, green);
    write_int (BLUE_LED_FILE, blue);
  } else {
    // battery light is active. Be careful not
    // to turn it off
    int bcolor = get_battery_color(bstate);
    if (bcolor != LED_RED) {
      write_int (RED_LED_FILE, red);
    }
    if (bcolor != LED_GREEN) {
      write_int (GREEN_LED_FILE, green);
    }
    if (bcolor != LED_BLUE) {
      write_int (BLUE_LED_FILE, blue);
    }
  }
}

static void
set_speaker_light_locked_battery(struct light_device_t* dev,
        struct light_state_t const* bstate, struct light_state_t const* nstate)
{
  int bcolor = LED_BLANK;
  int n_red = LED_BLANK;
  int n_green = LED_BLANK;
  int n_blue = LED_BLANK;

  unsigned int n_colorRGB = nstate->color & 0xFFFFFF;

  if ((n_colorRGB >> 16) & 0xFF) n_red = LED_RED;
  if ((n_colorRGB >> 8) & 0xFF) n_green = LED_GREEN;
  if (n_colorRGB & 0xFF) n_blue = LED_BLUE;

  // a battery event came in and battery light is visible
  // we must be careful as it is possible to change from one
  // visible battery state to another. so we write the visible
  // color and clear remaining lights if they are not in use
  // from a notification
  if (is_lit(bstate)) {
    bcolor = get_battery_color(bstate);
      switch (bcolor) {
        case LED_RED:
          write_int (RED_LED_FILE, LED_RED);
          if(n_green == LED_BLANK) {
            write_int (GREEN_LED_FILE, LED_BLANK);
          }
          if(n_blue == LED_BLANK) {
            write_int (BLUE_LED_FILE, LED_BLANK);
          }
          break;
        case LED_GREEN:
          write_int (GREEN_LED_FILE, LED_GREEN);
          if(n_blue == LED_BLANK) {
            write_int (BLUE_LED_FILE, LED_BLANK);
          }
          if(n_red == LED_BLANK) {
            write_int (RED_LED_FILE, LED_BLANK);
          }
          break;
        case LED_BLUE:
          write_int (BLUE_LED_FILE, LED_BLUE);
          if(n_red == LED_BLANK) {
            write_int (RED_LED_FILE, LED_BLANK);
          }
          if(n_green == LED_BLANK) {
            write_int (GREEN_LED_FILE, LED_BLANK);
          }
          break;
        default:
          ALOGE("set_led_state (dual) unexpected color: bcolorRGB=%08x\n", bcolor);
      }
  } else {
    // device is not charging. clear all states
    // preserving any notification lights
    if(n_red == LED_BLANK) {
      write_int (RED_LED_FILE, LED_BLANK);
    }
    if(n_green == LED_BLANK) {
      write_int (GREEN_LED_FILE, LED_BLANK);
    }
    if(n_blue == LED_BLANK) {
      write_int (BLUE_LED_FILE, LED_BLANK);
    }
  }
}

static int
set_light_notifications(struct light_device_t* dev,
        struct light_state_t const* state)
{
    pthread_mutex_lock(&g_lock);
    g_notification = *state;
    set_speaker_light_locked_notification(dev, &g_battery, &g_notification);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

static int
set_light_battery(struct light_device_t* dev,
        struct light_state_t const* state)
{
    pthread_mutex_lock(&g_lock);
    g_battery = *state;
    set_speaker_light_locked_battery(dev, &g_battery, &g_notification);
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
