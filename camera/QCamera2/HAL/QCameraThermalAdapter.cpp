/* Copyright (c) 2013, The Linux Foundataion. All rights reserved.
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
 *
 */

#define LOG_TAG "QCameraThermalAdapter"

#include <dlfcn.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <utils/Errors.h>

#include "QCameraThermalAdapter.h"

using namespace android;

namespace qcamera {


QCameraThermalAdapter& QCameraThermalAdapter::getInstance()
{
    static QCameraThermalAdapter instance;
    return instance;
}

QCameraThermalAdapter::QCameraThermalAdapter() :
                                        mCallback(NULL),
                                        mHandle(NULL),
                                        mRegister(NULL),
                                        mUnregister(NULL)
{
}

int QCameraThermalAdapter::init(QCameraThermalCallback *thermalCb)
{
    const char *error = NULL;
    int rc;

    ALOGV("%s E", __func__);
    mHandle = dlopen("/system/lib/libthermalclient.so", RTLD_NOW);
    if (!mHandle) {
        error = dlerror();
        ALOGE("%s: dlopen failed with error %s",
                    __func__, error ? error : "");
        rc = UNKNOWN_ERROR;
        goto error;
    }
    *(void **)&mRegister = dlsym(mHandle, "thermal_register_callback");
    if (!mRegister) {
        error = dlerror();
        ALOGE("%s: dlsym failed with error code %s",
                    __func__, error ? error: "");
        rc = UNKNOWN_ERROR;
        goto error2;
    }
    *(void **)&mUnregister = dlsym(mHandle, "thermal_unregister_callback");
    if (!mUnregister) {
        error = dlerror();
        ALOGE("%s: dlsym failed with error code %s",
                    __func__, error ? error: "");
        rc = UNKNOWN_ERROR;
        goto error2;
    }

    // Register camera and camcorder callbacks
    rc = mRegister(mStrCamera, thermalCallback, NULL);
    if (rc < 0) {
        ALOGE("%s: thermal_register_callback failed %d", __func__, rc);
        goto error2;
    }
    rc = mRegister(mStrCamcorder, thermalCallback, NULL);
    if (rc < 0) {
        ALOGE("%s: thermal_register_callback failed %d", __func__, rc);
        goto error3;
    }

    mCallback = thermalCb;
    ALOGV("%s X", __func__);
    return rc;

error3:
    mUnregister(mStrCamera);
error2:
    dlclose(mHandle);
    mHandle = NULL;
error:
    ALOGV("%s X", __func__);
    return rc;
}

void QCameraThermalAdapter::deinit()
{
    ALOGV("%s E", __func__);
    if (mUnregister) {
        mUnregister(mStrCamera);
        mUnregister(mStrCamcorder);
    }
    if (mHandle)
        dlclose(mHandle);

    mHandle = NULL;
    mRegister = NULL;
    mUnregister = NULL;
    mCallback = NULL;
    ALOGV("%s X", __func__);
}

char QCameraThermalAdapter::mStrCamera[] = "camera";
char QCameraThermalAdapter::mStrCamcorder[] = "camcorder";

int QCameraThermalAdapter::thermalCallback(char * name,
                                int threshold, int level)
{
    int rc = 0;
    ALOGV("%s E", __func__);
    QCameraThermalAdapter& instance = getInstance();
    qcamera_thermal_level_enum_t lvl = (qcamera_thermal_level_enum_t) level;
    if (instance.mCallback)
        rc = instance.mCallback->thermalEvtHandle(name, threshold, lvl);
    ALOGV("%s X", __func__);
    return rc;
}

}; //namespace qcamera
