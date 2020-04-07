/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 * Copyright (C) 2017-2019 The LineageOS Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
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

#define LOG_TAG "android.hardware.power@1.2-service.shamu"

// #define LOG_NDEBUG 0

#include "Power.h"
#include <android-base/file.h>
#include <log/log.h>
#include "power-common.h"
#include "power-feature.h"

namespace android {
namespace hardware {
namespace power {
namespace V1_2 {
namespace implementation {

using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::power::V1_0::Feature;
using ::android::hardware::power::V1_0::PowerHint;
using ::android::hardware::power::V1_0::PowerStatePlatformSleepState;
using ::android::hardware::power::V1_0::Status;
using ::android::hardware::power::V1_1::PowerStateSubsystem;

Power::Power() {
    power_init();
}

Return<void> Power::setInteractive(bool interactive) {
    set_interactive(interactive ? 1 : 0);
    return Void();
}

Return<void> Power::powerHint(PowerHint_1_0 hint, int32_t data) {
    power_hint(static_cast<power_hint_t>(hint), data ? (&data) : NULL);
    return Void();
}

Return<void> Power::setFeature(Feature feature, bool activate) {
    switch (feature) {
#ifdef TAP_TO_WAKE_NODE
        case Feature::POWER_FEATURE_DOUBLE_TAP_TO_WAKE:
            ::android::base::WriteStringToFile(activate ? "AUTO" : "OFF", TAP_TO_WAKE_NODE, true);
            break;
#endif
        default:
            break;
    }
    set_device_specific_feature(static_cast<feature_t>(feature), activate ? 1 : 0);
    return Void();
}

Return<void> Power::getPlatformLowPowerStats(getPlatformLowPowerStats_cb _hidl_cb) {
    hidl_vec<PowerStatePlatformSleepState> states;
    states.resize(0);

    _hidl_cb(states, Status::SUCCESS);
    return Void();
}

Return<void> Power::getSubsystemLowPowerStats(getSubsystemLowPowerStats_cb _hidl_cb) {
    hidl_vec<PowerStateSubsystem> subsystems;

    _hidl_cb(subsystems, Status::SUCCESS);
    return Void();
}

Return<void> Power::powerHintAsync(PowerHint_1_0 hint, int32_t data) {
    return powerHint(hint, data);
}

Return<void> Power::powerHintAsync_1_2(PowerHint_1_2 hint, int32_t data) {
    return powerHint(static_cast<PowerHint_1_0>(hint), data);
}

Return<int32_t> Power::getFeature(LineageFeature feature) {
    if (feature == LineageFeature::SUPPORTED_PROFILES) {
        return get_number_of_profiles();
    }
    return -1;
}

status_t Power::registerAsSystemService() {
    status_t ret = 0;

    ret = IPower::registerAsService();
    if (ret != 0) {
        ALOGE("Failed to register IPower (%d)", ret);
        goto fail;
    } else {
        ALOGI("Successfully registered IPower");
    }

    ret = ILineagePower::registerAsService();
    if (ret != 0) {
        ALOGE("Failed to register ILineagePower (%d)", ret);
        goto fail;
    } else {
        ALOGI("Successfully registered ILineagePower");
    }

fail:
    return ret;
}

}  // namespace implementation
}  // namespace V1_2
}  // namespace power
}  // namespace hardware
}  // namespace android
