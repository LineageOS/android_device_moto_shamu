/*
 * Copyright (C) 2017 The Android Open Source Project
 * Copyright (C) 2017-2018 The LineageOS Project
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

#ifdef V1_0_HAL
#define LOG_TAG "android.hardware.power@1.0-service-qti"
#else
#define LOG_TAG "android.hardware.power@1.1-service-qti"
#endif

// #define LOG_NDEBUG 0

#include <android/log.h>
#include <hidl/HidlTransportSupport.h>
#include <hardware/power.h>
#include "Power.h"

using android::sp;
using android::status_t;
using android::OK;

// libhwbinder:
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

// Generated HIDL files
#ifdef V1_0_HAL
using android::hardware::power::V1_0::implementation::Power;
#else
using android::hardware::power::V1_1::implementation::Power;
#endif

int main() {

    status_t status;
    android::sp<Power> service = nullptr;

#ifdef V1_0_HAL
    ALOGI("Power HAL Service 1.0 for QCOM is starting.");
#else
    ALOGI("Power HAL Service 1.1 for QCOM is starting.");
#endif

    service = new Power();
    if (service == nullptr) {
        ALOGE("Can not create an instance of Power HAL Iface, exiting.");

        goto shutdown;
    }

    configureRpcThreadpool(1, true /*callerWillJoin*/);

    status = service->registerAsSystemService();
    if (status != OK) {
        ALOGE("Could not register service for Power HAL Iface (%d).", status);
        goto shutdown;
    }

    ALOGI("Power Service is ready");
    joinRpcThreadpool();
    //Should not pass this line

shutdown:
    // In normal operation, we don't expect the thread pool to exit

    ALOGE("Power Service is shutting down");
    return 1;
}
