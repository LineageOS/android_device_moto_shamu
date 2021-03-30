/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021, The LineageOS Project. All rights reserved.
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

#include "LineagePower.h"
#include "Power.h"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

using aidl::android::hardware::power::impl::Power;
using LineagePower = aidl::vendor::lineage::power::impl::Power;

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);
    std::shared_ptr<Power> vib = ndk::SharedRefBase::make<Power>();
    const std::string instance = std::string() + Power::descriptor + "/default";
    LOG(INFO) << "Instance " << instance;
    if (vib) {
        binder_status_t status =
                AServiceManager_addService(vib->asBinder().get(), instance.c_str());
        LOG(INFO) << "Status " << status;
        if (status != STATUS_OK) {
            LOG(ERROR) << "Could not register" << instance;
        }
    }

    std::shared_ptr<LineagePower> lineage_vib = ndk::SharedRefBase::make<LineagePower>();
    const std::string lineage_instance = std::string() + LineagePower::descriptor + "/default";
    LOG(INFO) << "Instance " << lineage_instance;
    if (lineage_vib) {
        binder_status_t status =
                AServiceManager_addService(lineage_vib->asBinder().get(), lineage_instance.c_str());
        LOG(INFO) << "Status " << status;
        if (status != STATUS_OK) {
            LOG(ERROR) << "Could not register" << instance;
        }
    }

    ABinderProcess_joinThreadPool();
    return 1;  // should not reach
}
