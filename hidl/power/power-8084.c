/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Copyright (C) 2018-2019 The LineageOS Project
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
#include <time.h>

#define LOG_TAG "Shamu PowerHAL"
#include <hardware/hardware.h>
#include <hardware/power.h>
#include <log/log.h>

#include "hint-data.h"
#include "metadata-defs.h"
#include "performance.h"
#include "power-common.h"
#include "utils.h"

static int first_display_off_hint;

static int current_power_profile = PROFILE_BALANCED;

// clang-format off
/* power save mode: max 2 CPUs, max 1.2 GHz */
static int profile_power_save[] = {
    0x0A03,
    CPUS_ONLINE_MAX_LIMIT_2,
    CPU0_MAX_FREQ_NONTURBO_MAX + 1,
    CPU1_MAX_FREQ_NONTURBO_MAX + 1,
    CPU2_MAX_FREQ_NONTURBO_MAX + 1,
    CPU3_MAX_FREQ_NONTURBO_MAX + 1
};

/* efficiency mode: max 2 CPUs, max 2.4 GHz */
static int profile_bias_power[] = {
    0x0A03,
    CPUS_ONLINE_MAX_LIMIT_2,
    CPU0_MAX_FREQ_NONTURBO_MAX + 14,
    CPU1_MAX_FREQ_NONTURBO_MAX + 14,
    CPU2_MAX_FREQ_NONTURBO_MAX + 14,
    CPU3_MAX_FREQ_NONTURBO_MAX + 14,
};

/* quick mode: min 3 CPUs */
static int profile_bias_performance[] = {
    CPUS_ONLINE_MIN_3
};

/* high performance mode: min 4 CPUs */
static int profile_high_performance[] = {
    0x0901,
    CPUS_ONLINE_MIN_4
};
// clang-format on

#ifdef INTERACTION_BOOST
int get_number_of_profiles() {
    return 5;
}
#endif

static int set_power_profile(void* data) {
    int profile = data ? *((int*)data) : 0;
    int ret = -EINVAL;
    const char* profile_name = NULL;

    if (profile == current_power_profile) return 0;

    ALOGV("%s: Profile=%d", __func__, profile);

    if (current_power_profile != PROFILE_BALANCED) {
        undo_hint_action(DEFAULT_PROFILE_HINT_ID);
        ALOGV("%s: Hint undone", __func__);
        current_power_profile = PROFILE_BALANCED;
    }

    if (profile == PROFILE_POWER_SAVE) {
        ret = perform_hint_action(DEFAULT_PROFILE_HINT_ID, profile_power_save,
                                  ARRAY_SIZE(profile_power_save));
        profile_name = "powersave";

    } else if (profile == PROFILE_HIGH_PERFORMANCE) {
        ret = perform_hint_action(DEFAULT_PROFILE_HINT_ID, profile_high_performance,
                                  ARRAY_SIZE(profile_high_performance));
        profile_name = "performance";

    } else if (profile == PROFILE_BIAS_POWER) {
        ret = perform_hint_action(DEFAULT_PROFILE_HINT_ID, profile_bias_power,
                                  ARRAY_SIZE(profile_bias_power));
        profile_name = "bias power";

    } else if (profile == PROFILE_BIAS_PERFORMANCE) {
        ret = perform_hint_action(DEFAULT_PROFILE_HINT_ID, profile_bias_performance,
                                  ARRAY_SIZE(profile_bias_performance));
        profile_name = "bias perf";
    } else if (profile == PROFILE_BALANCED) {
        ret = 0;
        profile_name = "balanced";
    }

    if (ret == 0) {
        current_power_profile = profile;
        ALOGD("%s: Set %s mode", __func__, profile_name);
    }
    return ret;
}

static int process_video_encode_hint(void* metadata) {
    char governor[80];
    struct video_encode_metadata_t video_encode_metadata;

    if (!metadata) return HINT_NONE;

    if (get_scaling_governor(governor, sizeof(governor)) == -1) {
        ALOGE("Can't obtain scaling governor.");
        return HINT_NONE;
    }

    /* Initialize encode metadata struct fields */
    memset(&video_encode_metadata, 0, sizeof(struct video_encode_metadata_t));
    video_encode_metadata.state = -1;
    video_encode_metadata.hint_id = DEFAULT_VIDEO_ENCODE_HINT_ID;

    if (parse_video_encode_metadata((char*)metadata, &video_encode_metadata) == -1) {
        ALOGE("Error occurred while parsing metadata.");
        return HINT_NONE;
    }

    if (video_encode_metadata.state == 1) {
        if (is_interactive_governor(governor)) {
            int resource_values[] = {TR_MS_30, HISPEED_LOAD_90, HS_FREQ_1026,
                                     THREAD_MIGRATION_SYNC_OFF, INTERACTIVE_IO_BUSY_OFF};
            perform_hint_action(video_encode_metadata.hint_id, resource_values,
                                ARRAY_SIZE(resource_values));
            return HINT_HANDLED;
        }
    } else if (video_encode_metadata.state == 0) {
        if (is_interactive_governor(governor)) {
            undo_hint_action(video_encode_metadata.hint_id);
            return HINT_HANDLED;
        }
    }
    return HINT_NONE;
}

static int process_video_decode_hint(void* metadata) {
    char governor[80];
    struct video_decode_metadata_t video_decode_metadata;

    if (!metadata) return HINT_NONE;

    if (get_scaling_governor(governor, sizeof(governor)) == -1) {
        ALOGE("Can't obtain scaling governor.");
        return HINT_NONE;
    }

    /* Initialize decode metadata struct fields */
    memset(&video_decode_metadata, 0, sizeof(struct video_decode_metadata_t));
    video_decode_metadata.state = -1;
    video_decode_metadata.hint_id = DEFAULT_VIDEO_DECODE_HINT_ID;

    if (parse_video_decode_metadata((char*)metadata, &video_decode_metadata) == -1) {
        ALOGE("Error occurred while parsing metadata.");
        return HINT_NONE;
    }

    if (video_decode_metadata.state == 1) {
        if (is_interactive_governor(governor)) {
            int resource_values[] = {TR_MS_30, HISPEED_LOAD_90, HS_FREQ_1026,
                                     THREAD_MIGRATION_SYNC_OFF};
            perform_hint_action(video_decode_metadata.hint_id, resource_values,
                                ARRAY_SIZE(resource_values));
            return HINT_HANDLED;
        }
    } else if (video_decode_metadata.state == 0) {
        if (is_interactive_governor(governor)) {
            undo_hint_action(video_decode_metadata.hint_id);
            return HINT_HANDLED;
        }
    }
    return HINT_NONE;
}

// clang-format off
/* fling boost: min 3 CPUs, min 1.3 GHz */
static int resources_interaction_fling_boost[] = {
    CPUS_ONLINE_MIN_3,
    CPU0_MIN_FREQ_NONTURBO_MAX + 3,
    CPU1_MIN_FREQ_NONTURBO_MAX + 3,
    CPU2_MIN_FREQ_NONTURBO_MAX + 3,
    CPU3_MIN_FREQ_NONTURBO_MAX + 3
};

/* interactive boost: min 2 CPUs, min 1.2 GHz */
static int resources_interaction_boost[] = {
    CPUS_ONLINE_MIN_2,
    CPU0_MIN_FREQ_NONTURBO_MAX + 2,
    CPU1_MIN_FREQ_NONTURBO_MAX + 2,
    CPU2_MIN_FREQ_NONTURBO_MAX + 2,
    CPU3_MIN_FREQ_NONTURBO_MAX + 2
};

/* fling boost: min 3 CPUs, min 1.5 GHz */
static int resources_interaction_fling_boost_perf[] = {
    CPUS_ONLINE_MIN_3,
    CPU0_MIN_FREQ_NONTURBO_MAX + 5,
    CPU1_MIN_FREQ_NONTURBO_MAX + 5,
    CPU2_MIN_FREQ_NONTURBO_MAX + 5,
    CPU3_MIN_FREQ_NONTURBO_MAX + 5
};

static int resources_launch[] = {
    CPUS_ONLINE_MIN_3,
    CPU0_MIN_FREQ_TURBO_MAX,
    CPU1_MIN_FREQ_TURBO_MAX,
    CPU2_MIN_FREQ_TURBO_MAX,
    CPU3_MIN_FREQ_TURBO_MAX
};
// clang-format on

/* interactive boost: min 3 CPUs, min 1.5 GHz */
static int resources_interaction_boost_perf[] = {
    CPUS_ONLINE_MIN_3,
    CPU0_MIN_FREQ_NONTURBO_MAX + 5,
    CPU1_MIN_FREQ_NONTURBO_MAX + 5,
    CPU2_MIN_FREQ_NONTURBO_MAX + 5,
    CPU3_MIN_FREQ_NONTURBO_MAX + 5
};
// clang-format on

const int kDefaultInteractiveDuration =  200; /* ms */
const int kPerfInteractiveDuration =  500;    /* ms */
const int kMinFlingDuration = 1500;           /* ms */
const int kMaxInteractiveDuration = 5000;     /* ms */
const int kMaxLaunchDuration = 5000;          /* ms */

static void process_interaction_hint(void* data) {
    static struct timespec s_previous_boost_timespec;
    static int s_previous_duration = 0;

    struct timespec cur_boost_timespec;
    long long elapsed_time;
    int duration;
    if (current_power_profile == PROFILE_BIAS_POWER ||
        current_power_profile == PROFILE_HIGH_PERFORMANCE) {
        duration = kPerfInteractiveDuration;
    } else {
        duration = kDefaultInteractiveDuration;
    }

    if (data) {
        int input_duration = *((int*)data);
        if (input_duration > duration) {
            duration = (input_duration > kMaxInteractiveDuration) ? kMaxInteractiveDuration
                                                                  : input_duration;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &cur_boost_timespec);

    elapsed_time = calc_timespan_us(s_previous_boost_timespec, cur_boost_timespec);
    // don't hint if previous hint's duration covers this hint's duration
    if ((s_previous_duration * 1000) > (elapsed_time + duration * 1000)) {
        return;
    }
    s_previous_boost_timespec = cur_boost_timespec;
    s_previous_duration = duration;

    if (current_power_profile == PROFILE_BIAS_POWER) {
        if (duration >= kMinFlingDuration) {
            interaction(duration, ARRAY_SIZE(resources_interaction_fling_boost_perf),
                    resources_interaction_fling_boost_perf);
        } else {
            interaction(duration, ARRAY_SIZE(resources_interaction_boost_perf),
                    resources_interaction_boost_perf);
        }
    } else {
        if (duration >= kMinFlingDuration) {
            interaction(duration, ARRAY_SIZE(resources_interaction_fling_boost),
                    resources_interaction_fling_boost);
        } else {
            interaction(duration, ARRAY_SIZE(resources_interaction_boost),
                    resources_interaction_boost);
        }
    }
}

static int process_activity_launch_hint(void* data) {
    static int launch_handle = -1;
    static int launch_mode = 0;

    // release lock early if launch has finished
    if (!data) {
        if (CHECK_HANDLE(launch_handle)) {
            release_request(launch_handle);
            launch_handle = -1;
        }
        launch_mode = 0;
        return HINT_HANDLED;
    }

    if (!launch_mode) {
        launch_handle = interaction_with_handle(launch_handle, kMaxLaunchDuration,
                                                ARRAY_SIZE(resources_launch), resources_launch);
        if (!CHECK_HANDLE(launch_handle)) {
            ALOGE("Failed to perform launch boost");
            return HINT_NONE;
        }
        launch_mode = 1;
    }
    return HINT_HANDLED;
}

int power_hint_override(power_hint_t hint, void* data) {
    int ret_val = HINT_NONE;

    if (hint == POWER_HINT_SET_PROFILE) {
        if (set_power_profile(data) < 0)
            ALOGE("Setting power profile failed. mpdecision not started?");
        return HINT_HANDLED;
    }

    // Skip other hints in high/low power modes
    if (current_power_profile == PROFILE_POWER_SAVE) {
        return HINT_HANDLED;
    }

    switch (hint) {
        case POWER_HINT_VIDEO_ENCODE:
            ret_val = process_video_encode_hint(data);
            break;
        case POWER_HINT_VIDEO_DECODE:
            ret_val = process_video_decode_hint(data);
            break;
        case POWER_HINT_INTERACTION:
            process_interaction_hint(data);
            ret_val = HINT_HANDLED;
            break;
        case POWER_HINT_LAUNCH:
            ret_val = process_activity_launch_hint(data);
            break;
        default:
            break;
    }
    return ret_val;
}

int set_interactive_override(int on) {
    char governor[80];

    if (get_scaling_governor(governor, sizeof(governor)) == -1) {
        ALOGE("Can't obtain scaling governor.");
        return HINT_NONE;
    }

    if (!on) {
        /* Display off */
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
        if (is_interactive_governor(governor)) {
            int resource_values[] = {TR_MS_50, THREAD_MIGRATION_SYNC_OFF};
            perform_hint_action(DISPLAY_STATE_HINT_ID, resource_values,
                                ARRAY_SIZE(resource_values));
        }
    } else {
        /* Display on */
        int resource_values2[] = {CPUS_ONLINE_MIN_2};
        perform_hint_action(DISPLAY_STATE_HINT_ID_2, resource_values2,
                            ARRAY_SIZE(resource_values2));
        if (is_interactive_governor(governor)) {
            undo_hint_action(DISPLAY_STATE_HINT_ID);
        }
    }
    return HINT_HANDLED;
}
