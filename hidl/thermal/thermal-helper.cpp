/*
 * Copyright (C) 2018 The LineageOS Project
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

#define LOG_TAG "thermal-helper"

#include <cctype>
#include <cerrno>
#include <cinttypes>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/stringprintf.h>

#include "thermal-helper.h"

namespace android {
namespace hardware {
namespace thermal {
namespace V1_1 {
namespace implementation {

using ::android::hardware::thermal::V1_0::TemperatureType;

/**
 * Reads device temperature.
 *
 * @param sensor_num Number of sensor file with temperature.
 * @param type Device temperature type.
 * @param name Device temperature name.
 * @param mult Multiplier used to translate temperature to Celsius.
 * @param throttling_threshold Throttling threshold for the temperature.
 * @param shutdown_threshold Shutdown threshold for the temperature.
 * @param out Pointer to temperature_t structure that will be filled with current
 *     values.
 *
 * @return 0 on success or negative value -errno on error.
 */
static ssize_t readTemperature(int sensor_num, TemperatureType type, const char *name, float mult,
                                float throttling_threshold, float shutdown_threshold,
                                Temperature *out) {
    FILE *file;
    char file_name[PATH_MAX];
    float temp;

    sprintf(file_name, kTemperatureFileFormat, sensor_num);
    file = fopen(file_name, "r");
    if (file == NULL) {
        PLOG(ERROR) << "readTemperature: failed to open file (" << file_name << ")";
        return -errno;
    }
    if (1 != fscanf(file, "%f", &temp)) {
        fclose(file);
        PLOG(ERROR) << "readTemperature: failed to read a float";
        return errno ? -errno : -EIO;
    }

    fclose(file);

    (*out).type = type;
    (*out).name = name;
    (*out).currentValue = temp * mult;
    (*out).throttlingThreshold = throttling_threshold;
    (*out).shutdownThreshold = shutdown_threshold;
    (*out).vrThrottlingThreshold = UNKNOWN_TEMPERATURE;

    LOG(DEBUG) << android::base::StringPrintf(
        "readTemperature: %d, %d, %s, %g, %g, %g",
        sensor_num, type, name, temp * mult, throttling_threshold, shutdown_threshold);

    return 0;
}

static ssize_t getCpuTemperatures(hidl_vec<Temperature> *temperatures) {
    size_t cpu;

    for (cpu = 0; cpu < kCpuNum; cpu++) {
        if (cpu >= temperatures->size()) {
            break;
        }
        // tsens_tz_sensor[5-8]: temperature in Celsius
        ssize_t result = readTemperature(cpu + kCpuSensorNum, TemperatureType::CPU, kCpuLabel[cpu],
                                          1, kCpuThrottlingThreshold, kCpuShutdownThreshold,
                                          &(*temperatures)[cpu]);
        if (result != 0) {
            return result;
        }
    }
    return cpu;
}

ssize_t fillTemperatures(hidl_vec<Temperature> *temperatures) {
    ssize_t result = 0;
    size_t current_index = 0;

    if (temperatures == NULL || temperatures->size() < kTemperatureNum) {
        LOG(ERROR) << "fillTemperatures: incorrect buffer";
        return -EINVAL;
    }

    result = getCpuTemperatures(temperatures);
    if (result < 0) {
        return result;
    }
    current_index += result;

    // Battery temperature.
    if (current_index < temperatures->size()) {
        // battery: hwmon sensor: temperature in millidegrees Celsius
        result = readTemperature(kBatterySensorNum, TemperatureType::BATTERY, kBatteryLabel,
                                  0.001, UNKNOWN_TEMPERATURE, kBatteryShutdownThreshold,
                                  &(*temperatures)[current_index]);
        if (result < 0) {
            return result;
        }
        current_index++;
    }

    // GPU temperature.
    if (current_index < temperatures->size()) {
        // GPU_therm: tsens_tz_sensor11: temperature in Celsius.
        result = readTemperature(kGpuSensorNum, TemperatureType::GPU, kGpuLabel,
                                  1, UNKNOWN_TEMPERATURE, UNKNOWN_TEMPERATURE,
                                  &(*temperatures)[current_index]);
        if (result < 0) {
            return result;
        }
        current_index++;
    }

    // Skin temperature.
    if (current_index < temperatures->size()) {
        // skin: quiet_therm: temperature in Celsius
        result = readTemperature(kSkinSensorNum, TemperatureType::SKIN, kSkinLabel,
                                  1, kSkinTrottlingThreshold, UNKNOWN_TEMPERATURE,
                                  &(*temperatures)[current_index]);
        if (result < 0) {
            return result;
        }
        current_index++;
    }

    return kTemperatureNum;
}

ssize_t fillCpuUsages(hidl_vec<CpuUsage> *cpuUsages) {
    int vals, cpu_num, online;
    ssize_t read;
    uint64_t user, nice, system, idle, active, total;
    char *line = NULL;
    size_t len = 0;
    size_t size = 0;
    char file_name[PATH_MAX];
    FILE *file;
    FILE *cpu_file;

    if (cpuUsages == NULL || cpuUsages->size() < kCpuNum ) {
        LOG(ERROR) << "fillCpuUsages: incorrect buffer";
        return -EINVAL;
    }

    file = fopen(kCpuUsageFile, "r");
    if (file == NULL) {
        PLOG(ERROR) << "fillCpuUsages: failed to open file (" << kCpuUsageFile << ")";
        return -errno;
    }

    while ((read = getline(&line, &len, file)) != -1) {
        // Skip non "cpu[0-9]" lines.
        if (strnlen(line, read) < 4 || strncmp(line, "cpu", 3) != 0 || !isdigit(line[3])) {
            free(line);
            line = NULL;
            len = 0;
            continue;
        }

        vals = sscanf(line, "cpu%d %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64,
                &cpu_num, &user, &nice, &system, &idle);

        free(line);
        line = NULL;
        len = 0;

        if (vals != 5 || size == kCpuNum) {
            if (vals != 5) {
                PLOG(ERROR) << "fillCpuUsages: failed to read CPU information from file ("
                            << kCpuUsageFile << ")";
            } else {
                PLOG(ERROR) << "fillCpuUsages: file has incorrect format ("
                            << kCpuUsageFile << ")";
            }
            fclose(file);
            return errno ? -errno : -EIO;
        }

        active = user + nice + system;
        total = active + idle;

        // Read online CPU information.
        snprintf(file_name, PATH_MAX, kCpuOnlineFileFormat, cpu_num);
        cpu_file = fopen(file_name, "r");
        online = 0;
        if (cpu_file == NULL) {
            PLOG(ERROR) << "fillCpuUsages: failed to open file (" << file_name << ")";
            fclose(file);
            return -errno;
        }
        if (1 != fscanf(cpu_file, "%d", &online)) {
            PLOG(ERROR) << "fillCpuUsages: failed to read CPU online information from file ("
                        << file_name << ")";
            fclose(file);
            fclose(cpu_file);
            return errno ? -errno : -EIO;
        }
        fclose(cpu_file);

        (*cpuUsages)[size].name = kCpuLabel[size];
        (*cpuUsages)[size].active = active;
        (*cpuUsages)[size].total = total;
        (*cpuUsages)[size].isOnline = static_cast<bool>(online);

        LOG(DEBUG) << "fillCpuUsages: "<< kCpuLabel[size] << ": "
                   << active << " " << total << " " <<  online;
        size++;
    }
    fclose(file);

    if (size != kCpuNum) {
        PLOG(ERROR) << "fillCpuUsages: file has incorrect format (" << kCpuUsageFile << ")";
        return -EIO;
    }
    return kCpuNum;
}

}  // namespace implementation
}  // namespace V1_1
}  // namespace thermal
}  // namespace hardware
}  // namespace android
