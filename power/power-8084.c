/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
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

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdlib.h>

#define LOG_TAG "QCOM PowerHAL"
#include <log/log.h>
#include <hardware/hardware.h>
#include <hardware/power.h>

#include "utils.h"
#include "metadata-defs.h"
#include "hint-data.h"
#include "performance.h"
#include "power-common.h"

static int first_display_off_hint;

int set_interactive_override(int on)
{
    char governor[80];

    if (get_scaling_governor(governor, sizeof(governor)) == -1) {
        ALOGE("Can't obtain scaling governor.");

        return HINT_NONE;
    }

    if (!on) {
        /* Display off. */
        /*
         * We need to be able to identify the first display off hint
         * and release the current lock holder
         */
        if (!first_display_off_hint) {
            undo_initial_hint_action();
            first_display_off_hint = 1;
        }
        /* Used for all subsequent toggles to the display */
        undo_hint_action(DISPLAY_STATE_HINT_ID_2);

        if (is_ondemand_governor(governor)) {
            int resource_values[] = {MS_500, SYNC_FREQ_600, OPTIMAL_FREQ_600, THREAD_MIGRATION_SYNC_OFF};

            perform_hint_action(DISPLAY_STATE_HINT_ID,
                    resource_values, ARRAY_SIZE(resource_values));

            return HINT_HANDLED;
        }
    } else {
        /* Display on */
        int resource_values2[] = { CPUS_ONLINE_MIN_2 };
        perform_hint_action(DISPLAY_STATE_HINT_ID_2,
                resource_values2, ARRAY_SIZE(resource_values2));

        if (is_ondemand_governor(governor)) {
            undo_hint_action(DISPLAY_STATE_HINT_ID);

            return HINT_HANDLED;
        }
    }

    return HINT_NONE;
}
