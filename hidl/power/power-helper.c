/*
 * Copyright (c) 2012-2017, The Linux Foundation. All rights reserved.
 * Copyright (C) 2017 The Android Open Source Project
 * Copyright (C) 2017-2018 The LineageOS Project
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
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>

#define LOG_TAG "QCOM PowerHAL"
#include <log/log.h>
#include <hardware/power.h>

#include "utils.h"
#include "metadata-defs.h"
#include "hint-data.h"
#include "performance.h"
#include "power-common.h"
#include "power-feature.h"
#include "power-helper.h"

#define USINSEC 1000000L
#define NSINUS 1000L

#ifndef RPM_STAT
#define RPM_STAT "/d/rpm_stats"
#endif

#ifndef RPM_MASTER_STAT
#define RPM_MASTER_STAT "/d/rpm_master_stats"
#endif

#ifndef RPM_SYSTEM_STAT
#define RPM_SYSTEM_STAT "/d/system_stats"
#endif

/*
   Set with TARGET_WLAN_POWER_STAT in BoardConfig.mk
   Defaults to QCACLD3 path
   Path for QCACLD3: /d/wlan0/power_stats
   Path for QCACLD2 and Prima: /d/wlan_wcnss/power_stats
 */

#ifndef NO_WLAN_STATS
#ifndef WLAN_POWER_STAT
#define WLAN_POWER_STAT "/d/wlan0/power_stats"
#endif
#endif

#define LINE_SIZE 128

#ifdef LEGACY_STATS
/* Use these stats on pre-nougat qualcomm kernels */
static const char *rpm_param_names[] = {
    "vlow_count",
    "accumulated_vlow_time",
    "vmin_count",
    "accumulated_vmin_time"
};

static const char *rpm_master_param_names[] = {
    "xo_accumulated_duration",
    "xo_count",
    "xo_accumulated_duration",
    "xo_count",
    "xo_accumulated_duration",
    "xo_count",
    "xo_accumulated_duration",
    "xo_count"
};

#ifndef NO_WLAN_STATS
static const char *wlan_param_names[] = {
    "cumulative_sleep_time_ms",
    "cumulative_total_on_time_ms",
    "deep_sleep_enter_counter",
    "last_deep_sleep_enter_tstamp_ms"
};
#endif
#else
/* Use these stats on nougat kernels and forward */
const char *rpm_stat_params[MAX_RPM_PARAMS] = {
    "count",
    "actual last sleep(msec)",
};

const char *master_stat_params[MAX_RPM_PARAMS] = {
    "Accumulated XO duration",
    "XO Count",
};

struct stat_pair rpm_stat_map[] = {
    { RPM_MODE_XO,   "RPM Mode:vlow", rpm_stat_params, ARRAY_SIZE(rpm_stat_params) },
    { RPM_MODE_VMIN, "RPM Mode:vmin", rpm_stat_params, ARRAY_SIZE(rpm_stat_params) },
    { VOTER_APSS,    "APSS",    master_stat_params, ARRAY_SIZE(master_stat_params) },
    { VOTER_MPSS,    "MPSS",    master_stat_params, ARRAY_SIZE(master_stat_params) },
    { VOTER_ADSP,    "ADSP",    master_stat_params, ARRAY_SIZE(master_stat_params) },
    { VOTER_SLPI,    "SLPI",    master_stat_params, ARRAY_SIZE(master_stat_params) },
};
#endif

#ifndef NO_WLAN_STATS
const char *wlan_power_stat_params[] = {
    "cumulative_sleep_time_ms",
    "cumulative_total_on_time_ms",
    "deep_sleep_enter_counter",
    "last_deep_sleep_enter_tstamp_ms"
};

struct stat_pair wlan_stat_map[] = {
    { WLAN_POWER_DEBUG_STATS, "POWER DEBUG STATS", wlan_power_stat_params, ARRAY_SIZE(wlan_power_stat_params) },
};
#endif

static int display_hint_sent;

void power_init(void)
{
    ALOGI("QCOM power HAL initing.");
}

static void process_video_decode_hint(void *metadata)
{
    char governor[80];
    struct video_decode_metadata_t video_decode_metadata;

    if (get_scaling_governor(governor, sizeof(governor)) == -1) {
        ALOGE("Can't obtain scaling governor.");

        return;
    }

    if (metadata) {
        ALOGI("Processing video decode hint. Metadata: %s", (char *)metadata);
    }

    /* Initialize encode metadata struct fields. */
    memset(&video_decode_metadata, 0, sizeof(struct video_decode_metadata_t));
    video_decode_metadata.state = -1;
    video_decode_metadata.hint_id = DEFAULT_VIDEO_DECODE_HINT_ID;

    if (metadata) {
        if (parse_video_decode_metadata((char *)metadata, &video_decode_metadata) ==
            -1) {
            ALOGE("Error occurred while parsing metadata.");
            return;
        }
    } else {
        return;
    }

    if (video_decode_metadata.state == 1) {
        if (is_ondemand_governor(governor)) {
            int resource_values[] = {THREAD_MIGRATION_SYNC_OFF};

            perform_hint_action(video_decode_metadata.hint_id,
                    resource_values, ARRAY_SIZE(resource_values));
        } else if (is_interactive_governor(governor)) {
            int resource_values[] = {TR_MS_30, HISPEED_LOAD_90, HS_FREQ_1026, THREAD_MIGRATION_SYNC_OFF};

            perform_hint_action(video_decode_metadata.hint_id,
                    resource_values, ARRAY_SIZE(resource_values));
        }
    } else if (video_decode_metadata.state == 0) {
        if (is_ondemand_governor(governor)) {
            undo_hint_action(video_decode_metadata.hint_id);
        } else if (is_interactive_governor(governor)) {
            undo_hint_action(video_decode_metadata.hint_id);
        }
    }
}

static void process_video_encode_hint(void *metadata)
{
    char governor[80];
    struct video_encode_metadata_t video_encode_metadata;

    if (get_scaling_governor(governor, sizeof(governor)) == -1) {
        ALOGE("Can't obtain scaling governor.");

        return;
    }

    /* Initialize encode metadata struct fields. */
    memset(&video_encode_metadata, 0, sizeof(struct video_encode_metadata_t));
    video_encode_metadata.state = -1;
    video_encode_metadata.hint_id = DEFAULT_VIDEO_ENCODE_HINT_ID;

    if (metadata) {
        if (parse_video_encode_metadata((char *)metadata, &video_encode_metadata) ==
            -1) {
            ALOGE("Error occurred while parsing metadata.");
            return;
        }
    } else {
        return;
    }

    if (video_encode_metadata.state == 1) {
        if (is_ondemand_governor(governor)) {
            int resource_values[] = {IO_BUSY_OFF, SAMPLING_DOWN_FACTOR_1, THREAD_MIGRATION_SYNC_OFF};

            perform_hint_action(video_encode_metadata.hint_id,
                    resource_values, ARRAY_SIZE(resource_values));
        } else if (is_interactive_governor(governor)) {
            int resource_values[] = {TR_MS_30, HISPEED_LOAD_90, HS_FREQ_1026, THREAD_MIGRATION_SYNC_OFF,
                INTERACTIVE_IO_BUSY_OFF};

            perform_hint_action(video_encode_metadata.hint_id,
                    resource_values, ARRAY_SIZE(resource_values));
        }
    } else if (video_encode_metadata.state == 0) {
        if (is_ondemand_governor(governor)) {
            undo_hint_action(video_encode_metadata.hint_id);
        } else if (is_interactive_governor(governor)) {
            undo_hint_action(video_encode_metadata.hint_id);
        }
    }
}

int __attribute__ ((weak)) power_hint_override(power_hint_t UNUSED(hint),
                                               void *UNUSED(data))
{
    return HINT_NONE;
}

void power_hint(power_hint_t hint, void *data)
{
    /* Check if this hint has been overridden. */
    if (power_hint_override(hint, data) == HINT_HANDLED) {
        /* The power_hint has been handled. We can skip the rest. */
        return;
    }

    switch(hint) {
        case POWER_HINT_SUSTAINED_PERFORMANCE:
            ALOGI("Sustained perf power hint not handled in power_hint_override");
        break;
        case POWER_HINT_VR_MODE:
            ALOGI("VR mode power hint not handled in power_hint_override");
        break;
        case POWER_HINT_VIDEO_ENCODE:
            process_video_encode_hint(data);
        break;
        case POWER_HINT_VIDEO_DECODE:
            process_video_decode_hint(data);
        break;
        case POWER_HINT_SET_PROFILE:
            ALOGI("set profile power hint not handled in power_hint_override");
        break;
        default:
        break;
    }
}

int get_number_of_profiles()
{
    return 0;
}

int __attribute__ ((weak)) set_interactive_override(int UNUSED(on))
{
    return HINT_NONE;
}

#ifdef SET_INTERACTIVE_EXT
extern void power_set_interactive_ext(int on);
#endif

void power_set_interactive(int on)
{
    char governor[80];

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
    if (display_hint_sent && !on)
        return;

    display_hint_sent = !on;

#ifdef SET_INTERACTIVE_EXT
    power_set_interactive_ext(on);
#endif

    if (set_interactive_override(on) == HINT_HANDLED) {
        return;
    }

    ALOGI("Got set_interactive hint");

    if (get_scaling_governor(governor, sizeof(governor)) == -1) {
        ALOGE("Can't obtain scaling governor.");

        return;
    }

    if (!on) {
        /* Display off. */
        if (is_ondemand_governor(governor)) {
            int resource_values[] = { MS_500, THREAD_MIGRATION_SYNC_OFF };

            perform_hint_action(DISPLAY_STATE_HINT_ID,
                    resource_values, ARRAY_SIZE(resource_values));
        } else if (is_interactive_governor(governor)) {
            int resource_values[] = {TR_MS_50, THREAD_MIGRATION_SYNC_OFF};

            perform_hint_action(DISPLAY_STATE_HINT_ID,
                    resource_values, ARRAY_SIZE(resource_values));
        }
    } else {
        /* Display on. */
        if (is_ondemand_governor(governor)) {
            undo_hint_action(DISPLAY_STATE_HINT_ID);
        } else if (is_interactive_governor(governor)) {
            undo_hint_action(DISPLAY_STATE_HINT_ID);
        }
    }
}

void __attribute__((weak)) set_device_specific_feature(feature_t UNUSED(feature), int UNUSED(state))
{
}

void set_feature(feature_t feature, int state)
{
    switch (feature) {
#ifdef TAP_TO_WAKE_NODE
        case POWER_FEATURE_DOUBLE_TAP_TO_WAKE:
            sysfs_write(TAP_TO_WAKE_NODE, state ? "AUTO" : "OFF");
            break;
#endif
        default:
            break;
    }
    set_device_specific_feature(feature, state);
}

#ifdef LEGACY_STATS
static int extract_stats(uint64_t *list, char *file, const char**param_names,
                         unsigned int num_parameters, int isHex) {
    FILE *fp;
    ssize_t read;
    size_t len;
    size_t index = 0;
    char *line;
    int ret;

    fp = fopen(file, "r");
    if (fp == NULL) {
        ret = -errno;
        ALOGE("%s: failed to open: %s Error = %s", __func__, file, strerror(errno));
        return ret;
    }

    for (line = NULL, len = 0;
         ((read = getline(&line, &len, fp) != -1) && (index < num_parameters));
         free(line), line = NULL, len = 0) {
        uint64_t value;
        char* offset;

        size_t begin = strspn(line, " \t");
        if (strncmp(line + begin, param_names[index], strlen(param_names[index]))) {
            continue;
        }

        offset = memchr(line, ':', len);
        if (!offset) {
            continue;
        }

        if (isHex) {
            sscanf(offset, ":%" SCNx64, &value);
        } else {
            sscanf(offset, ":%" SCNu64, &value);
        }
        list[index] = value;
        index++;
    }

    free(line);
    fclose(fp);

    return 0;
}

int extract_platform_stats(uint64_t *list) {
    int ret;
    //Data is located in two files
    ret = extract_stats(list, RPM_STAT, rpm_param_names, RPM_PARAM_COUNT, false);
    if (ret) {
        for (size_t i=0; i < RPM_PARAM_COUNT; i++)
            list[i] = 0;
    }
    ret = extract_stats(list + RPM_PARAM_COUNT, RPM_MASTER_STAT,
                        rpm_master_param_names, PLATFORM_PARAM_COUNT - RPM_PARAM_COUNT, true);
    if (ret) {
        for (size_t i=RPM_PARAM_COUNT; i < PLATFORM_PARAM_COUNT; i++)
        list[i] = 0;
    }
    return 0;
}

#ifndef NO_WLAN_STATS
int extract_wlan_stats(uint64_t *list) {
    int ret;
    ret = extract_stats(list, WLAN_POWER_STAT, wlan_param_names, WLAN_POWER_PARAMS_COUNT, false);
    if (ret) {
        for (size_t i=0; i < WLAN_POWER_PARAMS_COUNT; i++)
            list[i] = 0;
    }
    return 0;
}
#endif
#else

static int parse_stats(const char **params, size_t params_size,
                       uint64_t *list, FILE *fp) {
    ssize_t nread;
    size_t len = LINE_SIZE;
    char *line;
    size_t params_read = 0;
    size_t i;

    line = malloc(len);
    if (!line) {
        ALOGE("%s: no memory to hold line", __func__);
        return -ENOMEM;
    }

    while ((params_read < params_size) &&
        (nread = getline(&line, &len, fp) > 0)) {
        char *key = line + strspn(line, " \t");
        char *value = strchr(key, ':');
        if (!value || (value > (line + len)))
            continue;
        *value++ = '\0';

        for (i = 0; i < params_size; i++) {
            if (!strcmp(key, params[i])) {
                list[i] = strtoull(value, NULL, 0);
                params_read++;
                break;
            }
        }
    }
    free(line);

    return 0;
}

static int extract_stats(uint64_t *list, char *file,
                         struct stat_pair *map, size_t map_size) {
    FILE *fp;
    ssize_t read;
    size_t len = LINE_SIZE;
    char *line;
    size_t i, stats_read = 0;
    int ret = 0;

    fp = fopen(file, "re");
    if (fp == NULL) {
        ALOGE("%s: failed to open: %s Error = %s", __func__, file, strerror(errno));
        return -errno;
    }

    line = malloc(len);
    if (!line) {
        ALOGE("%s: no memory to hold line", __func__);
        fclose(fp);
        return -ENOMEM;
    }

    while ((stats_read < map_size) && (read = getline(&line, &len, fp) != -1)) {
        size_t begin = strspn(line, " \t");

        for (i = 0; i < map_size; i++) {
            if (!strncmp(line + begin, map[i].label, strlen(map[i].label))) {
                stats_read++;
                break;
            }
        }

        if (i == map_size)
            continue;

        ret = parse_stats(map[i].parameters, map[i].num_parameters,
                          &list[map[i].stat * MAX_RPM_PARAMS], fp);
        if (ret < 0)
            break;
    }
    free(line);
    fclose(fp);

    return ret;
}

int extract_platform_stats(uint64_t *list) {
    return extract_stats(list, RPM_SYSTEM_STAT, rpm_stat_map, ARRAY_SIZE(rpm_stat_map));
}

#ifndef NO_WLAN_STATS
int extract_wlan_stats(uint64_t *list) {
    return extract_stats(list, WLAN_POWER_STAT, wlan_stat_map, ARRAY_SIZE(wlan_stat_map));
}
#endif
#endif
