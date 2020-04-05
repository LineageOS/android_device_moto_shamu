/*
 * Copyright (c) 2012-2019, The Linux Foundation. All rights reserved.
 * Copyright (C) 2017-2019 The LineageOS Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * *    * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_NIDEBUG 0

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define LOG_TAG "QTI PowerHAL"
#include <hardware/hardware.h>
#include <hardware/power.h>
#include <log/log.h>

#include "hint-data.h"
#include "performance.h"
#include "power-common.h"
#include "power-feature.h"
#include "utils.h"

static struct hint_handles handles[NUM_HINTS];

void power_init() {
    ALOGI("Initing");

    for (int i = 0; i < NUM_HINTS; i++) {
        handles[i].handle = 0;
        handles[i].ref_count = 0;
    }
}

int __attribute__((weak)) power_hint_override(power_hint_t UNUSED(hint), void* UNUSED(data)) {
    return HINT_NONE;
}

void power_hint(power_hint_t hint, void* data) {
    /* Check if this hint has been overridden. */
    if (power_hint_override(hint, data) == HINT_HANDLED) {
        /* The power_hint has been handled. We can skip the rest. */
        return;
    }
    switch (hint) {
        case POWER_HINT_VR_MODE:
            ALOGI("VR mode power hint not handled in power_hint_override");
            break;
        // fall through below, hints will fail if not defined in powerhint.xml
        case POWER_HINT_SUSTAINED_PERFORMANCE:
        case POWER_HINT_VIDEO_ENCODE:
            if (data) {
                if (handles[hint].ref_count == 0)
                    handles[hint].handle = perf_hint_enable((AOSP_DELTA + hint), 0);

                if (handles[hint].handle > 0) handles[hint].ref_count++;
            } else {
                if (handles[hint].handle > 0) {
                    if (--handles[hint].ref_count == 0) {
                        release_request(handles[hint].handle);
                        handles[hint].handle = 0;
                    }
                } else {
                    ALOGE("Lock for hint: %X was not acquired, cannot be released", hint);
                }
            }
            break;
        case POWER_HINT_SET_PROFILE:
            ALOGI("set profile power hint not handled in power_hint_override");
            break;
        default:
            break;
    }
}

int __attribute__((weak)) get_number_of_profiles() {
    return 0;
}

int __attribute__((weak)) set_interactive_override(int UNUSED(on)) {
    return HINT_NONE;
}

#ifdef SET_INTERACTIVE_EXT
extern void power_set_interactive_ext(int on);
#endif

void set_interactive(int on) {
    static int display_hint_sent;

    if (!on) {
        /* Send Display OFF hint to perf HAL */
        perf_hint_enable(VENDOR_HINT_DISPLAY_OFF, 0);
    } else {
        /* Send Display ON hint to perf HAL */
        perf_hint_enable(VENDOR_HINT_DISPLAY_ON, 0);
    }

    /**
     * Ignore consecutive display-off hints
     * Consecutive display-on hints are already handled
     */
    if (display_hint_sent && !on) return;

    display_hint_sent = !on;

#ifdef SET_INTERACTIVE_EXT
    power_set_interactive_ext(on);
#endif

    if (set_interactive_override(on) == HINT_HANDLED) {
        return;
    } else {
        ALOGI("Hint not handled in set_interactive_override");
    }
}

void __attribute__((weak))
set_device_specific_feature(feature_t UNUSED(feature), int UNUSED(state)) {}
