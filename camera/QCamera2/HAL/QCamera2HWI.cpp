/* Copyright (c) 2012-2013, The Linux Foundataion. All rights reserved.
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

#define LOG_TAG "QCamera2HWI"

#include <cutils/properties.h>
#include <hardware/camera.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <utils/Errors.h>
#include <gralloc_priv.h>

#include "QCamera2HWI.h"
#include "QCameraMem.h"

#define MAP_TO_DRIVER_COORDINATE(val, base, scale, offset) (val * scale / base + offset)

namespace qcamera {

cam_capability_t *gCamCapability[MM_CAMERA_MAX_NUM_SENSORS];

camera_device_ops_t QCamera2HardwareInterface::mCameraOps = {
    set_preview_window:         QCamera2HardwareInterface::set_preview_window,
    set_callbacks:              QCamera2HardwareInterface::set_CallBacks,
    enable_msg_type:            QCamera2HardwareInterface::enable_msg_type,
    disable_msg_type:           QCamera2HardwareInterface::disable_msg_type,
    msg_type_enabled:           QCamera2HardwareInterface::msg_type_enabled,

    start_preview:              QCamera2HardwareInterface::start_preview,
    stop_preview:               QCamera2HardwareInterface::stop_preview,
    preview_enabled:            QCamera2HardwareInterface::preview_enabled,
    store_meta_data_in_buffers: QCamera2HardwareInterface::store_meta_data_in_buffers,

    start_recording:            QCamera2HardwareInterface::start_recording,
    stop_recording:             QCamera2HardwareInterface::stop_recording,
    recording_enabled:          QCamera2HardwareInterface::recording_enabled,
    release_recording_frame:    QCamera2HardwareInterface::release_recording_frame,

    auto_focus:                 QCamera2HardwareInterface::auto_focus,
    cancel_auto_focus:          QCamera2HardwareInterface::cancel_auto_focus,

    take_picture:               QCamera2HardwareInterface::take_picture,
    cancel_picture:             QCamera2HardwareInterface::cancel_picture,

    set_parameters:             QCamera2HardwareInterface::set_parameters,
    get_parameters:             QCamera2HardwareInterface::get_parameters,
    put_parameters:             QCamera2HardwareInterface::put_parameters,
    send_command:               QCamera2HardwareInterface::send_command,

    release:                    QCamera2HardwareInterface::release,
    dump:                       QCamera2HardwareInterface::dump,
};

int QCamera2HardwareInterface::set_preview_window(struct camera_device *device,
        struct preview_stream_ops *window)
{
    int rc = NO_ERROR;
    ALOGI("%s: %p",__func__, window);
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("%s: NULL camera device", __func__);
        return BAD_VALUE;
    }

    hw->lockAPI();
    rc = hw->processAPI(QCAMERA_SM_EVT_SET_PREVIEW_WINDOW, (void *)window);
    if (rc == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_SET_PREVIEW_WINDOW);
        rc = hw->m_apiResult.status;
    }
    hw->unlockAPI();

    return rc;
}

void QCamera2HardwareInterface::set_CallBacks(struct camera_device *device,
        camera_notify_callback notify_cb,
        camera_data_callback data_cb,
        camera_data_timestamp_callback data_cb_timestamp,
        camera_request_memory get_memory,
        void *user)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return;
    }

    qcamera_sm_evt_setcb_payload_t payload;
    payload.notify_cb = notify_cb;
    payload.data_cb = data_cb;
    payload.data_cb_timestamp = data_cb_timestamp;
    payload.get_memory = get_memory;
    payload.user = user;

    hw->lockAPI();
    int32_t rc = hw->processAPI(QCAMERA_SM_EVT_SET_CALLBACKS, (void *)&payload);
    if (rc == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_SET_CALLBACKS);
    }
    hw->unlockAPI();
}

void QCamera2HardwareInterface::enable_msg_type(struct camera_device *device, int32_t msg_type)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return;
    }
    hw->lockAPI();
    int32_t rc = hw->processAPI(QCAMERA_SM_EVT_ENABLE_MSG_TYPE, (void *)msg_type);
    if (rc == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_ENABLE_MSG_TYPE);
    }
    hw->unlockAPI();
}

void QCamera2HardwareInterface::disable_msg_type(struct camera_device *device, int32_t msg_type)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return;
    }
    hw->lockAPI();
    int32_t rc = hw->processAPI(QCAMERA_SM_EVT_DISABLE_MSG_TYPE, (void *)msg_type);
    if (rc == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_DISABLE_MSG_TYPE);
    }
    hw->unlockAPI();
}

int QCamera2HardwareInterface::msg_type_enabled(struct camera_device *device, int32_t msg_type)
{
    int ret = NO_ERROR;
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    hw->lockAPI();
    ret = hw->processAPI(QCAMERA_SM_EVT_MSG_TYPE_ENABLED, (void *)msg_type);
    if (ret == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_MSG_TYPE_ENABLED);
        ret = hw->m_apiResult.enabled;
    }
    hw->unlockAPI();

   return ret;
}

int QCamera2HardwareInterface::start_preview(struct camera_device *device)
{
    int ret = NO_ERROR;
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }

    hw->lockAPI();
    qcamera_sm_evt_enum_t evt = QCAMERA_SM_EVT_START_PREVIEW;
    if (hw->isNoDisplayMode()) {
        evt = QCAMERA_SM_EVT_START_NODISPLAY_PREVIEW;
    }
    ret = hw->processAPI(evt, NULL);
    if (ret == NO_ERROR) {
        hw->waitAPIResult(evt);
        ret = hw->m_apiResult.status;
    }
    hw->unlockAPI();
    return ret;
}

void QCamera2HardwareInterface::stop_preview(struct camera_device *device)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return;
    }
    hw->lockAPI();
    int32_t ret = hw->processAPI(QCAMERA_SM_EVT_STOP_PREVIEW, NULL);
    if (ret == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_STOP_PREVIEW);
    }
    hw->unlockAPI();
}

int QCamera2HardwareInterface::preview_enabled(struct camera_device *device)
{
    int ret = NO_ERROR;
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }

    hw->lockAPI();
    ret = hw->processAPI(QCAMERA_SM_EVT_PREVIEW_ENABLED, NULL);
    if (ret == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_PREVIEW_ENABLED);
        ret = hw->m_apiResult.enabled;
    }
    hw->unlockAPI();

    return ret;
}

int QCamera2HardwareInterface::store_meta_data_in_buffers(
                struct camera_device *device, int enable)
{
    int ret = NO_ERROR;
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }

    hw->lockAPI();
    ret = hw->processAPI(QCAMERA_SM_EVT_STORE_METADATA_IN_BUFS, (void *)enable);
    if (ret == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_STORE_METADATA_IN_BUFS);
        ret = hw->m_apiResult.status;
    }
    hw->unlockAPI();

    return ret;
}

int QCamera2HardwareInterface::start_recording(struct camera_device *device)
{
    int ret = NO_ERROR;
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    hw->lockAPI();
    ret = hw->processAPI(QCAMERA_SM_EVT_START_RECORDING, NULL);
    if (ret == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_START_RECORDING);
        ret = hw->m_apiResult.status;
    }
    hw->unlockAPI();

    return ret;
}

void QCamera2HardwareInterface::stop_recording(struct camera_device *device)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return;
    }
    hw->lockAPI();
    int32_t ret = hw->processAPI(QCAMERA_SM_EVT_STOP_RECORDING, NULL);
    if (ret == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_STOP_RECORDING);
    }
    hw->unlockAPI();
}

int QCamera2HardwareInterface::recording_enabled(struct camera_device *device)
{
    int ret = NO_ERROR;
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    hw->lockAPI();
    ret = hw->processAPI(QCAMERA_SM_EVT_RECORDING_ENABLED, NULL);
    if (ret == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_RECORDING_ENABLED);
        ret = hw->m_apiResult.enabled;
    }
    hw->unlockAPI();

    return ret;
}

void QCamera2HardwareInterface::release_recording_frame(
            struct camera_device *device, const void *opaque)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return;
    }
    hw->lockAPI();
    int32_t ret = hw->processAPI(QCAMERA_SM_EVT_RELEASE_RECORIDNG_FRAME, (void *)opaque);
    if (ret == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_RELEASE_RECORIDNG_FRAME);
    }
    hw->unlockAPI();
}

int QCamera2HardwareInterface::auto_focus(struct camera_device *device)
{
    int ret = NO_ERROR;
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    hw->lockAPI();
    ret = hw->processAPI(QCAMERA_SM_EVT_START_AUTO_FOCUS, NULL);
    if (ret == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_START_AUTO_FOCUS);
        ret = hw->m_apiResult.status;
    }
    hw->unlockAPI();

    return ret;
}

int QCamera2HardwareInterface::cancel_auto_focus(struct camera_device *device)
{
    int ret = NO_ERROR;
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    hw->lockAPI();
    ret = hw->processAPI(QCAMERA_SM_EVT_STOP_AUTO_FOCUS, NULL);
    if (ret == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_STOP_AUTO_FOCUS);
        ret = hw->m_apiResult.status;
    }
    hw->unlockAPI();

    return ret;
}

int QCamera2HardwareInterface::take_picture(struct camera_device *device)
{
    int ret = NO_ERROR;
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    hw->lockAPI();
    ret = hw->processAPI(QCAMERA_SM_EVT_TAKE_PICTURE, NULL);
    if (ret == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_TAKE_PICTURE);
        ret = hw->m_apiResult.status;
    }
    hw->unlockAPI();

    return ret;
}

int QCamera2HardwareInterface::cancel_picture(struct camera_device *device)
{
    int ret = NO_ERROR;
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    hw->lockAPI();
    ret = hw->processAPI(QCAMERA_SM_EVT_CANCEL_PICTURE, NULL);
    if (ret == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_CANCEL_PICTURE);
        ret = hw->m_apiResult.status;
    }
    hw->unlockAPI();

    return ret;
}

int QCamera2HardwareInterface::set_parameters(struct camera_device *device,
                                              const char *parms)
{
    int ret = NO_ERROR;
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    hw->lockAPI();
    ret = hw->processAPI(QCAMERA_SM_EVT_SET_PARAMS, (void *)parms);
    if (ret == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_SET_PARAMS);
        ret = hw->m_apiResult.status;
    }
    hw->unlockAPI();

    return ret;
}

char* QCamera2HardwareInterface::get_parameters(struct camera_device *device)
{
    char *ret = NULL;
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return NULL;
    }
    hw->lockAPI();
    int32_t rc = hw->processAPI(QCAMERA_SM_EVT_GET_PARAMS, NULL);
    if (rc == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_GET_PARAMS);
        ret = hw->m_apiResult.params;
    }
    hw->unlockAPI();

    return ret;
}

void QCamera2HardwareInterface::put_parameters(struct camera_device *device,
                                               char *parm)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return;
    }
    hw->lockAPI();
    int32_t ret = hw->processAPI(QCAMERA_SM_EVT_PUT_PARAMS, (void *)parm);
    if (ret == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_PUT_PARAMS);
    }
    hw->unlockAPI();
}

int QCamera2HardwareInterface::send_command(struct camera_device *device,
                                            int32_t cmd,
                                            int32_t arg1,
                                            int32_t arg2)
{
    int ret = NO_ERROR;
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }

    qcamera_sm_evt_command_payload_t payload;
    memset(&payload, 0, sizeof(qcamera_sm_evt_command_payload_t));
    payload.cmd = cmd;
    payload.arg1 = arg1;
    payload.arg2 = arg2;
    hw->lockAPI();
    ret = hw->processAPI(QCAMERA_SM_EVT_SEND_COMMAND, (void *)&payload);
    if (ret == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_SEND_COMMAND);
        ret = hw->m_apiResult.status;
    }
    hw->unlockAPI();

    return ret;
}

void QCamera2HardwareInterface::release(struct camera_device *device)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return;
    }
    hw->lockAPI();
    int32_t ret = hw->processAPI(QCAMERA_SM_EVT_RELEASE, NULL);
    if (ret == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_RELEASE);
    }
    hw->unlockAPI();
}

int QCamera2HardwareInterface::dump(struct camera_device *device, int fd)
{
    int ret = NO_ERROR;
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    hw->lockAPI();
    ret = hw->processAPI(QCAMERA_SM_EVT_DUMP, (void *)fd);
    if (ret == NO_ERROR) {
        hw->waitAPIResult(QCAMERA_SM_EVT_DUMP);
        ret = hw->m_apiResult.status;
    }
    hw->unlockAPI();

    return ret;
}

int QCamera2HardwareInterface::close_camera_device(hw_device_t *hw_dev)
{
    int ret = NO_ERROR;
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(
            reinterpret_cast<camera_device_t *>(hw_dev)->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    delete hw;
    return ret;
}

QCamera2HardwareInterface::QCamera2HardwareInterface(int cameraId)
    : mCameraId(cameraId),
      mCameraHandle(NULL),
      mCameraOpened(false),
      mPreviewWindow(NULL),
      mMsgEnabled(0),
      mStoreMetaDataInFrame(0),
      m_stateMachine(this),
      m_postprocessor(this),
      m_bShutterSoundPlayed(false),
      m_bAutoFocusRunning(false),
      m_pHistBuf(NULL)
{
    mCameraDevice.common.tag = HARDWARE_DEVICE_TAG;
    mCameraDevice.common.version = HARDWARE_DEVICE_API_VERSION(1, 0);
    mCameraDevice.common.close = close_camera_device;
    mCameraDevice.ops = &mCameraOps;
    mCameraDevice.priv = this;

    pthread_mutex_init(&m_lock, NULL);
    pthread_cond_init(&m_cond, NULL);
    memset(&m_apiResult, 0, sizeof(qcamera_api_result_t));

    memset(m_channels, 0, sizeof(m_channels));
    memset(mFaces, 0, sizeof(mFaces));
    memset(&mRoiData, 0, sizeof(mRoiData));
}

QCamera2HardwareInterface::~QCamera2HardwareInterface()
{
    closeCamera();
    pthread_mutex_destroy(&m_lock);
    pthread_cond_destroy(&m_cond);

    if (m_pHistBuf != NULL) {
        m_pHistBuf->release(m_pHistBuf);
        m_pHistBuf = NULL;
    }
}

int QCamera2HardwareInterface::openCamera(struct hw_device_t **hw_device)
{
    int rc = NO_ERROR;
    if (mCameraOpened) {
        *hw_device = NULL;
        return PERMISSION_DENIED;
    }

    rc = openCamera();
    if (rc == NO_ERROR)
        *hw_device = &mCameraDevice.common;
    else
        *hw_device = NULL;
    return rc;
}

int QCamera2HardwareInterface::openCamera()
{
    if (mCameraHandle) {
        ALOGE("Failure: Camera already opened");
        return ALREADY_EXISTS;
    }
    mCameraHandle = camera_open(mCameraId);
    if (!mCameraHandle) {
        ALOGE("camera_open failed.");
        return UNKNOWN_ERROR;
    }

    m_evtNotifyTh.launch(evtNotifyRoutine, this);
    mCameraHandle->ops->register_event_notify(mCameraHandle->camera_handle,
                                              evtHandle,
                                              (void *) this);

    int32_t rc = m_postprocessor.init(jpegEvtHandle, this);
    if (rc != 0) {
        ALOGE("Init Postprocessor failed");
        return UNKNOWN_ERROR;
    }

    mParameters.init(gCamCapability[mCameraId], mCameraHandle);
    mCameraOpened = true;

    return NO_ERROR;
}

int QCamera2HardwareInterface::closeCamera()
{
    int rc = NO_ERROR;
    int i;

    // deinit Parameters
    mParameters.deinit();

    // stop and deinit postprocessor
    m_postprocessor.stop();
    m_postprocessor.deinit();

    // delete all channels if not already deleted
    for (i = 0; i < QCAMERA_CH_TYPE_MAX; i++) {
        if (m_channels[i] != NULL) {
            m_channels[i]->stop();
            delete m_channels[i];
            m_channels[i] = NULL;
        }
    }

    rc = mCameraHandle->ops->close_camera(mCameraHandle->camera_handle);
    mCameraHandle = NULL;

    m_evtNotifyTh.exit();
    mCameraOpened = false;

    return rc;
}

#define DATA_PTR(MEM_OBJ,INDEX) MEM_OBJ->getPtr( INDEX )

int QCamera2HardwareInterface::initCapabilities(int cameraId)
{
    int rc = NO_ERROR;
    mm_camera_vtbl_t *cameraHandle = NULL;
    QCameraHeapMemory *capabilityHeap = NULL;

    cameraHandle = camera_open(cameraId);
    if (!cameraHandle) {
        ALOGE("%s: camera_open failed", __func__);
        rc = UNKNOWN_ERROR;
        goto open_failed;
    }

    /* Allocate memory for capability buffer */
    capabilityHeap = new QCameraHeapMemory();
    rc = capabilityHeap->allocate(1, sizeof(cam_capability_t));
    if(rc != OK) {
        ALOGE("%s: No memory for cappability", __func__);
        goto allocate_failed;
    }

    /* Map memory for capability buffer */
    memset(DATA_PTR(capabilityHeap,0), 0, sizeof(cam_capability_t));
    rc = cameraHandle->ops->map_buf(cameraHandle->camera_handle,
                                CAM_MAPPING_BUF_TYPE_CAPABILITY,
                                capabilityHeap->getFd(0),
                                sizeof(cam_capability_t));
    if(rc < 0) {
        ALOGE("%s: failed to map capability buffer", __func__);
        goto map_failed;
    }

    /* Query Capability */
    rc = cameraHandle->ops->query_capability(cameraHandle->camera_handle);
    if(rc < 0) {
        ALOGE("%s: failed to query capability",__func__);
        goto query_failed;
    }
    gCamCapability[cameraId] = (cam_capability_t *)malloc(sizeof(cam_capability_t));
    if (!gCamCapability[cameraId]) {
        ALOGE("%s: out of memory", __func__);
        goto query_failed;
    }
    memcpy(gCamCapability[cameraId], DATA_PTR(capabilityHeap,0),
                                        sizeof(cam_capability_t));
    rc = NO_ERROR;

query_failed:
    cameraHandle->ops->unmap_buf(cameraHandle->camera_handle,
                            CAM_MAPPING_BUF_TYPE_CAPABILITY);
map_failed:
    capabilityHeap->deallocate();
    delete capabilityHeap;
allocate_failed:
    cameraHandle->ops->close_camera(cameraHandle->camera_handle);
    cameraHandle = NULL;
open_failed:
    return rc;
}

int QCamera2HardwareInterface::getCapabilities(int cameraId,
                                    struct camera_info *info)
{
    int rc = NO_ERROR;

    if (NULL == gCamCapability[cameraId]) {
        rc = initCapabilities(cameraId);
        if (rc < 0)
            return rc;
    }

    switch(gCamCapability[cameraId]->position) {
    case CAM_POSITION_BACK:
        info->facing = CAMERA_FACING_BACK;
        break;

    case CAM_POSITION_FRONT:
        info->facing = CAMERA_FACING_FRONT;
        break;

    default:
        ALOGE("%s:Unknown position type for camera id:%d", __func__, cameraId);
        rc = BAD_VALUE;
        break;
    }

    info->orientation=gCamCapability[cameraId]->sensor_mount_angle;
    return rc;
}

QCameraMemory *QCamera2HardwareInterface::allocateStreamBuf(
    cam_stream_type_t stream_type, int size)
{
    int rc = NO_ERROR;
    QCameraMemory *mem = NULL;
    int bufferCnt = 0;
    const int minStreamingBuffers = 3;
    const int minCaptureBuffers = mParameters.getNumOfSnapshots();
    const int minVideoBuffers = 9;
    int zslQueueDepth = mParameters.getZSLQueueDepth();

    // Get buffer count for the particular stream type
    switch (stream_type) {
    case CAM_STREAM_TYPE_PREVIEW:
        bufferCnt = minStreamingBuffers;
        if (m_channels[QCAMERA_CH_TYPE_ZSL])
            bufferCnt += zslQueueDepth;
        break;
    case CAM_STREAM_TYPE_POSTVIEW:
        bufferCnt = minCaptureBuffers;
        break;
    case CAM_STREAM_TYPE_SNAPSHOT:
        if (m_channels[QCAMERA_CH_TYPE_ZSL])
            bufferCnt = minStreamingBuffers + zslQueueDepth;
        else
            bufferCnt = minCaptureBuffers;
        break;
    case CAM_STREAM_TYPE_VIDEO:
        bufferCnt = minVideoBuffers;
        break;
    case CAM_STREAM_TYPE_RAW:
        bufferCnt = minCaptureBuffers;
        break;
    case CAM_STREAM_TYPE_METADATA:
        if (m_channels[QCAMERA_CH_TYPE_ZSL])
            bufferCnt = minStreamingBuffers + zslQueueDepth;
        else if (m_channels[QCAMERA_CH_TYPE_SNAPSHOT])
            bufferCnt = minCaptureBuffers;
        else
            bufferCnt = minStreamingBuffers;
        break;
    case CAM_STREAM_TYPE_OFFLINE_PROC:
        bufferCnt = minCaptureBuffers;
        break;
    case CAM_STREAM_TYPE_DEFAULT:
    case CAM_STREAM_TYPE_MAX:
    default:
        bufferCnt = 0;
        break;
    }

    if (bufferCnt == 0)
        return NULL;
    // Allocate stream buffer memory object
    switch (stream_type) {
    case CAM_STREAM_TYPE_PREVIEW:
    case CAM_STREAM_TYPE_POSTVIEW: {
        cam_dimension_t dim;
        QCameraGrallocMemory *grallocMemory = new QCameraGrallocMemory(mGetMemory);

        mParameters.getStreamDimension(stream_type, dim);
        if (grallocMemory)
            grallocMemory->setWindowInfo(mPreviewWindow, dim.width, dim.height,
                    mParameters.getPreviewHalPixelFormat());
        mem = grallocMemory;
        }
        break;
    case CAM_STREAM_TYPE_SNAPSHOT:
    case CAM_STREAM_TYPE_RAW:
    case CAM_STREAM_TYPE_METADATA:
    case CAM_STREAM_TYPE_OFFLINE_PROC:
        mem = new QCameraStreamMemory(mGetMemory);
        break;
    case CAM_STREAM_TYPE_VIDEO:
        mem = new QCameraVideoMemory(mGetMemory);
        break;
    case CAM_STREAM_TYPE_DEFAULT:
    case CAM_STREAM_TYPE_MAX:
        break;
    }
    if (!mem)
        return NULL;

    rc = mem->allocate(bufferCnt, size);
    if (rc < 0) {
        delete mem;
        return NULL;
    }
    return mem;
}

QCameraHeapMemory *QCamera2HardwareInterface::allocateStreamInfoBuf(
    cam_stream_type_t stream_type)
{
    int rc = NO_ERROR;

    QCameraHeapMemory *streamInfoBuf = new QCameraHeapMemory();
    if (!streamInfoBuf) {
        ALOGE("allocateStreamInfoBuf: Unable to allocate streamInfo object");
        return NULL;
    }

    rc = streamInfoBuf->allocate(1, sizeof(cam_stream_info_t));
    if (rc < 0) {
        ALOGE("allocateStreamInfoBuf: Failed to allocate stream info memory");
        delete streamInfoBuf;
        return NULL;
    }

    cam_stream_info_t *streamInfo = (cam_stream_info_t *)streamInfoBuf->getPtr(0);
    memset(streamInfo, 0, sizeof(cam_stream_info_t));
    streamInfo->stream_type = stream_type;
    rc = mParameters.getStreamFormat(stream_type, streamInfo->fmt);
    rc = mParameters.getStreamDimension(stream_type, streamInfo->dim);

    streamInfo->streaming_mode = CAM_STREAMING_MODE_CONTINUOUS;
    switch (stream_type) {
    case CAM_STREAM_TYPE_SNAPSHOT:
    case CAM_STREAM_TYPE_RAW:
        if (mParameters.isZSLMode()) {
            streamInfo->streaming_mode = CAM_STREAMING_MODE_CONTINUOUS;
        } else {
            streamInfo->streaming_mode = CAM_STREAMING_MODE_BURST;
            streamInfo->num_of_burst = mParameters.getNumOfSnapshots();
        }
        break;
    case CAM_STREAM_TYPE_OFFLINE_PROC:
        // right now offline process is only for WNR in ZSL case
        // use the same format and dimension for input and output
        streamInfo->offline_proc_buf_fmt = streamInfo->fmt;
        streamInfo->offline_proc_buf_dim = streamInfo->dim;
        if (needOfflineReprocess()) {
            streamInfo->feature_mask |= CAM_QCOM_FEATURE_DENOISE2D;
        }
        break;
    default:
        break;
    }

    return streamInfoBuf;
}

int QCamera2HardwareInterface::setPreviewWindow(
        struct preview_stream_ops *window)
{
    mPreviewWindow = window;
    return NO_ERROR;
}

int QCamera2HardwareInterface::setCallBacks(camera_notify_callback notify_cb,
                                            camera_data_callback data_cb,
                                            camera_data_timestamp_callback data_cb_timestamp,
                                            camera_request_memory get_memory,
                                            void *user)
{
    mNotifyCb        = notify_cb;
    mDataCb          = data_cb;
    mDataCbTimestamp = data_cb_timestamp;
    mGetMemory       = get_memory;
    mCallbackCookie  = user;
    return NO_ERROR;
}

int QCamera2HardwareInterface::enableMsgType(int32_t msg_type)
{
    mMsgEnabled |= msg_type;
    return NO_ERROR;
}

int QCamera2HardwareInterface::disableMsgType(int32_t msg_type)
{
    mMsgEnabled &= ~msg_type;
    return NO_ERROR;
}

int QCamera2HardwareInterface::msgTypeEnabled(int32_t msg_type)
{
    return (mMsgEnabled & msg_type);
}

int QCamera2HardwareInterface::startPreview()
{
    int32_t rc = NO_ERROR;

    // allocate memory for histgrame data to be passed to upper layer
    if (m_pHistBuf == NULL) {
        m_pHistBuf = mGetMemory(-1, sizeof(cam_histogram_data_t), 1, mCallbackCookie);
    }
    if (!m_pHistBuf) {
        ALOGE("%s: Unable to allocate m_pHistBuf", __func__);
        return NO_MEMORY;
    }

    // TODO: commented out for now: start meta data stream
    //rc = startChannel(QCAMERA_CH_TYPE_METADATA);

    // start preview stream
    if (mParameters.isZSLMode()) {
        rc = startChannel(QCAMERA_CH_TYPE_ZSL);
    } else {
        rc = startChannel(QCAMERA_CH_TYPE_PREVIEW);
    }

    if (rc != NO_ERROR) {
        m_pHistBuf->release(m_pHistBuf);
        m_pHistBuf = NULL;
    }

    return rc;
}

int QCamera2HardwareInterface::stopPreview()
{
    // stop preview stream
    if (mParameters.isZSLMode()) {
        stopChannel(QCAMERA_CH_TYPE_ZSL);
    } else {
        stopChannel(QCAMERA_CH_TYPE_PREVIEW);
    }

    // TODO: commented out for now: stop meta data stream
    //stopChannel(QCAMERA_CH_TYPE_METADATA);
    if (m_pHistBuf != NULL) {
        m_pHistBuf->release(m_pHistBuf);
        m_pHistBuf = NULL;
    }

    // delete all channels from preparePreview
    unpreparePreview();
    return NO_ERROR;
}

int QCamera2HardwareInterface::storeMetaDataInBuffers(int enable)
{
    mStoreMetaDataInFrame = enable;
    return NO_ERROR;
}

int QCamera2HardwareInterface::startRecording()
{
    int32_t rc = NO_ERROR;
    if (mParameters.getRecordingHintValue() == false) {
        ALOGE("%s: start recording when hint is false, stop preview first", __func__);
        stopChannel(QCAMERA_CH_TYPE_PREVIEW);
        delChannel(QCAMERA_CH_TYPE_PREVIEW);

        // Set local recording hint to TRUE
        mParameters.setRecordingHintValue(true);

        rc = preparePreview();
        if (rc == NO_ERROR) {
            rc = startChannel(QCAMERA_CH_TYPE_PREVIEW);
        }
    }

    if (rc == NO_ERROR) {
        rc = startChannel(QCAMERA_CH_TYPE_VIDEO);
    }

    return rc;
}

int QCamera2HardwareInterface::stopRecording()
{
    return stopChannel(QCAMERA_CH_TYPE_VIDEO);
}

int QCamera2HardwareInterface::releaseRecordingFrame(const void * opaque)
{
    int32_t rc = UNKNOWN_ERROR;
    QCameraVideoChannel *pChannel =
        (QCameraVideoChannel *)m_channels[QCAMERA_CH_TYPE_VIDEO];
    if(pChannel != NULL) {
        rc = pChannel->releaseFrame(opaque, mStoreMetaDataInFrame > 0);
    }
    return rc;
}

int QCamera2HardwareInterface::autoFocus()
{
    int rc = NO_ERROR;
    cam_focus_mode_type focusMode = mParameters.getFocusMode();

    switch (focusMode) {
    case CAM_FOCUS_MODE_AUTO:
    case CAM_FOCUS_MODE_MACRO:
        rc = mCameraHandle->ops->do_auto_focus(mCameraHandle->camera_handle,
                                               CAM_AF_DO_ONE_FULL_SWEEP);
        if (rc == NO_ERROR) {
            m_bAutoFocusRunning = true;
        }
        break;
    case CAM_FOCUS_MODE_CONTINOUS_VIDEO:
        // According to Google API definition, the focus callback will immediately
        // return with a boolean that indicates whether the focus is sharp or not.
        // The focus position is locked after autoFocus call.
        // in this sense, the effect is the same as cancel_auto_focus
        {
            cam_autofocus_state_t state =
                mCameraHandle->ops->cancel_auto_focus(mCameraHandle->camera_handle);
            m_bAutoFocusRunning = false;

            // send evt notify that foucs is done
            rc = sendEvtNotify(CAMERA_MSG_FOCUS,
                               (state == CAM_AF_FOCUSED)? true : false,
                               0);
        }
        break;
    case CAM_FOCUS_MODE_CONTINOUS_PICTURE:
        // According to Google API definition, if the autofocus is in the middle
        // of scanning, the focus callback will return when it completes. If the
        // autofocus is not scanning, focus callback will immediately return with
        // a boolean that indicates whether the focus is sharp or not. The apps
        // can then decide if they want to take a picture immediately or to change
        // the focus mode to auto, and run a full autofocus cycle. The focus position
        // is locked after autoFocus call.
        rc = mCameraHandle->ops->do_auto_focus(mCameraHandle->camera_handle,
                                               CAM_AF_COMPLETE_EXISTING_SWEEP);
        if (rc == NO_ERROR) {
            m_bAutoFocusRunning = true;
        }
        break;
    case CAM_FOCUS_MODE_INFINITY:
    case CAM_FOCUS_MODE_FIXED:
    case CAM_FOCUS_MODE_EDOF:
    default:
        ALOGE("%s: Not supported in focusMode (%d)", __func__, focusMode);
        m_bAutoFocusRunning = false;
        rc = BAD_VALUE;
        break;
    }
    return rc;
}

int QCamera2HardwareInterface::cancelAutoFocus()
{
    int rc = NO_ERROR;
    cam_focus_mode_type focusMode = mParameters.getFocusMode();

    switch (focusMode) {
    case CAM_FOCUS_MODE_AUTO:
    case CAM_FOCUS_MODE_MACRO:
        mCameraHandle->ops->cancel_auto_focus(mCameraHandle->camera_handle);
        m_bAutoFocusRunning = false;
        rc = NO_ERROR;
        break;
    case CAM_FOCUS_MODE_CONTINOUS_VIDEO:
    case CAM_FOCUS_MODE_CONTINOUS_PICTURE:
        // resume CAF
        rc = mCameraHandle->ops->do_auto_focus(mCameraHandle->camera_handle,
                                               CAM_AF_START_CONTINUOUS_SWEEP);
        m_bAutoFocusRunning = false;
        break;
    case CAM_FOCUS_MODE_INFINITY:
    case CAM_FOCUS_MODE_FIXED:
    case CAM_FOCUS_MODE_EDOF:
    default:
        ALOGE("%s: Not supported in focusMode (%d)", __func__, focusMode);
        rc = BAD_VALUE;
        break;
    }
    return rc;
}

int QCamera2HardwareInterface::takePicture()
{
    int rc = NO_ERROR;
    uint8_t numSnapshots = mParameters.getNumOfSnapshots();

    // start postprocessor
    m_postprocessor.start();

    if (mParameters.isZSLMode()) {
        QCameraPicChannel *pZSLChannel =
            (QCameraPicChannel *)m_channels[QCAMERA_CH_TYPE_ZSL];
        if (NULL != pZSLChannel) {
            rc = pZSLChannel->takePicture(numSnapshots);
        } else {
            ALOGE("%s: ZSL channel is NULL", __func__);
            rc = UNKNOWN_ERROR;
        }

        if (rc != NO_ERROR) {
            ALOGE("%s: cannot take ZSL picture", __func__);
            m_postprocessor.stop();
            return rc;
        }
    } else {
        // normal capture case
        // prepare snapshot, e.g LED
        prepareHardwareForSnapshot( );

        // need to stop preview channel
        stopChannel(QCAMERA_CH_TYPE_PREVIEW);
        delChannel(QCAMERA_CH_TYPE_PREVIEW);

        // start snapshot
        rc = addCaptureChannel();
        if (rc == NO_ERROR) {
            rc = startChannel(QCAMERA_CH_TYPE_CAPTURE);
            if (rc != NO_ERROR) {
                ALOGE("%s: cannot start capture channel", __func__);
                delChannel(QCAMERA_CH_TYPE_CAPTURE);
                m_postprocessor.stop();
                return rc;
            }
        }
    }

    return rc;
}

int QCamera2HardwareInterface::cancelPicture()
{
    //stop post processor
    m_postprocessor.stop();

    if (mParameters.isZSLMode()) {
        QCameraPicChannel *pZSLChannel =
            (QCameraPicChannel *)m_channels[QCAMERA_CH_TYPE_ZSL];
        if (NULL != pZSLChannel) {
            pZSLChannel->cancelPicture();
            stopChannel(QCAMERA_CH_TYPE_ZSL);
            delChannel(QCAMERA_CH_TYPE_ZSL);
        }
    } else {
        // normal capture case
        stopChannel(QCAMERA_CH_TYPE_CAPTURE);
        delChannel(QCAMERA_CH_TYPE_CAPTURE);
    }
    return NO_ERROR;
}

int QCamera2HardwareInterface::takeLiveSnapshot()
{
    int rc = NO_ERROR;

    // start post processor
    m_postprocessor.start();

    // start snapshot channel
    rc = startChannel(QCAMERA_CH_TYPE_SNAPSHOT);
    if (rc != NO_ERROR) {
        m_postprocessor.stop();
    }
    return rc;
}

int QCamera2HardwareInterface::cancelLiveSnapshot()
{
    int rc = NO_ERROR;

    //stop post processor
    m_postprocessor.stop();

    // stop snapshot channel
    rc = stopChannel(QCAMERA_CH_TYPE_SNAPSHOT);

    return rc;
}

char* QCamera2HardwareInterface::getParameters()
{
    char* strParams = NULL;
    String8 str;
    str = mParameters.flatten( );
    strParams = (char *)malloc(sizeof(char)*(str.length()+1));
    if(strParams != NULL){
        memset(strParams, 0, sizeof(char)*(str.length()+1));
        strncpy(strParams, str.string(), str.length());
        strParams[str.length()] = 0;
    }
    return strParams;
}

int QCamera2HardwareInterface::putParameters(char *parms)
{
    free(parms);
    return NO_ERROR;
}

int QCamera2HardwareInterface::sendCommand(int32_t command, int32_t /*arg1*/, int32_t /*arg2*/)
{
    int rc = NO_ERROR;

    switch (command) {
    case CAMERA_CMD_HISTOGRAM_ON:
    case CAMERA_CMD_HISTOGRAM_OFF:
        rc = setHistogram(command == CAMERA_CMD_HISTOGRAM_ON? true : false);
        break;
    case CAMERA_CMD_START_FACE_DETECTION:
    case CAMERA_CMD_STOP_FACE_DETECTION:
        rc = setFaceDetection(command == CAMERA_CMD_START_FACE_DETECTION? true : false);
        break;
    case CAMERA_CMD_HISTOGRAM_SEND_DATA:
    default:
        rc = NO_ERROR;
        break;
    }
    return rc;
}

int QCamera2HardwareInterface::release()
{
    // stop and delete all channels
    for (int i = 0; i <QCAMERA_CH_TYPE_MAX ; i++) {
        if (m_channels[i] != NULL) {
            stopChannel((qcamera_ch_type_enum_t)i);
            delChannel((qcamera_ch_type_enum_t)i);
        }
    }

    return NO_ERROR;
}

int QCamera2HardwareInterface::dump(int /*fd*/)
{
    ALOGE("%s: not supported yet", __func__);
    return INVALID_OPERATION;
}

int QCamera2HardwareInterface::processAPI(qcamera_sm_evt_enum_t api, void *api_payload)
{
    return m_stateMachine.procAPI(api, api_payload);
}

int QCamera2HardwareInterface::processEvt(qcamera_sm_evt_enum_t evt, void *evt_payload)
{
    return m_stateMachine.procEvt(evt, evt_payload);
}

void QCamera2HardwareInterface::evtHandle(uint32_t /*camera_handle*/,
                                          mm_camera_event_t *evt,
                                          void *user_data)
{
    QCamera2HardwareInterface *obj = (QCamera2HardwareInterface *)user_data;
    if (obj && evt) {
        mm_camera_event_t *payload =
            (mm_camera_event_t *)malloc(sizeof(mm_camera_event_t));
        if (NULL != payload) {
            *payload = *evt;
            obj->processEvt(QCAMERA_SM_EVT_EVT_NOTIFY, payload);
        }
    } else {
        ALOGE("%s: NULL user_data", __func__);
    }
}

void QCamera2HardwareInterface::jpegEvtHandle(jpeg_job_status_t status,
                                              uint32_t /*client_hdl*/,
                                              uint32_t jobId,
                                              mm_jpeg_buf_t *p_buf,
                                              void *userdata)
{
    QCamera2HardwareInterface *obj = (QCamera2HardwareInterface *)userdata;
    if (obj) {
        qcamera_jpeg_evt_payload_t *payload =
            (qcamera_jpeg_evt_payload_t *)malloc(sizeof(qcamera_jpeg_evt_payload_t));
        if (NULL != payload && (JPEG_JOB_STATUS_DONE == status)) {
            memset(payload, 0, sizeof(qcamera_jpeg_evt_payload_t));
            payload->status = status;
            payload->jobId = jobId;
            payload->out_data = p_buf->buf_vaddr;
            payload->data_size = p_buf->buf_size;
            obj->processEvt(QCAMERA_SM_EVT_JPEG_EVT_NOTIFY, payload);
        }
    } else {
        ALOGE("%s: NULL user_data", __func__);
    }
}

void *QCamera2HardwareInterface::evtNotifyRoutine(void *data)
{
    int running = 1;
    int ret;
    QCamera2HardwareInterface *pme = (QCamera2HardwareInterface *)data;
    QCameraCmdThread *cmdThread = &pme->m_evtNotifyTh;

    ALOGD("%s: E", __func__);
    do {
        do {
            ret = cam_sem_wait(&cmdThread->cmd_sem);
            if (ret != 0 && errno != EINVAL) {
                ALOGE("%s: cam_sem_wait error (%s)",
                           __func__, strerror(errno));
                return NULL;
            }
        } while (ret != 0);

        /* we got notified about new cmd avail in cmd queue */
        camera_cmd_type_t cmd = cmdThread->getCmd();
        ALOGD("%s: get cmd %d", __func__, cmd);
        switch (cmd) {
        case CAMERA_CMD_TYPE_DO_NEXT_JOB:
            {
                qcamera_evt_argm_t *evt_cb =
                    (qcamera_evt_argm_t *)pme->m_evtNotifyQ.dequeue();
                if (NULL != evt_cb) {
                    /* send notify to upper layer */
                    if (NULL != pme->mNotifyCb &&
                        pme->msgTypeEnabled(evt_cb->msg_type) > 0) {
                        pme->mNotifyCb(evt_cb->msg_type,
                                       evt_cb->ext1,
                                       evt_cb->ext2,
                                       pme->mCallbackCookie);
                    }
                    /* free evt_cb */
                    free(evt_cb);
                }
            }
            break;
        case CAMERA_CMD_TYPE_EXIT:
            {
                /* flush evt notify queue */
                pme->m_evtNotifyQ.flush();
                running = 0;
            }
            break;
        default:
            break;
        }
    } while (running);
    ALOGD("%s: X", __func__);
    return NULL;
}

int32_t QCamera2HardwareInterface::sendEvtNotify(int32_t msg_type,
                                                 int32_t ext1,
                                                 int32_t ext2)
{
    qcamera_evt_argm_t *evt_cb = (qcamera_evt_argm_t *)malloc(sizeof(qcamera_evt_argm_t));
    if (NULL == evt_cb) {
        ALOGE("%s: no mem for qcamera_evt_argm_t", __func__);
        return NO_MEMORY;
    }
    memset(evt_cb, 0, sizeof(qcamera_evt_argm_t));
    evt_cb->msg_type = msg_type;
    evt_cb->ext1 = ext1;
    evt_cb->ext2 = ext2;

    /* enqueue evt into notify queue */
    if (m_evtNotifyQ.enqueue((void *)evt_cb)) {
        m_evtNotifyTh.sendCmd(CAMERA_CMD_TYPE_DO_NEXT_JOB, FALSE, FALSE);
    } else {
        ALOGE("%s: Error enqueue into event notify queue", __func__);
        free(evt_cb);
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}

int32_t QCamera2HardwareInterface::processAutoFocusEvent(cam_auto_focus_data_t &focus_data)
{
    int32_t ret = NO_ERROR;

    cam_focus_mode_type focusMode = mParameters.getFocusMode();
    switch (focusMode) {
    case CAM_FOCUS_MODE_AUTO:
    case CAM_FOCUS_MODE_MACRO:
        if (m_bAutoFocusRunning) {
            // update focus distance
            mParameters.updateFocusDistances(&focus_data.focus_dist);
            ret = sendEvtNotify(CAMERA_MSG_FOCUS,
                                (focus_data.focus_state == CAM_AF_FOCUSED)? true : false,
                                0);
        }
        break;
    case CAM_FOCUS_MODE_CONTINOUS_VIDEO:
    case CAM_FOCUS_MODE_CONTINOUS_PICTURE:
        if (m_bAutoFocusRunning) {
            // update focus distance
            mParameters.updateFocusDistances(&focus_data.focus_dist);
            ret = sendEvtNotify(CAMERA_MSG_FOCUS,
                                (focus_data.focus_state == CAM_AF_FOCUSED)? true : false,
                                0);
        } else {
            if (focus_data.focus_state == CAM_AF_FOCUSED ||
                focus_data.focus_state == CAM_AF_NOT_FOCUSED) {
                // update focus distance
                mParameters.updateFocusDistances(&focus_data.focus_dist);
            }
            ret = sendEvtNotify(CAMERA_MSG_FOCUS_MOVE,
                                (focus_data.focus_state == CAM_AF_SCANNING)? true : false,
                                0);
        }
        break;
    case CAM_FOCUS_MODE_INFINITY:
    case CAM_FOCUS_MODE_FIXED:
    case CAM_FOCUS_MODE_EDOF:
    default:
        ALOGD("%s: no ops for autofocus event in focusmode %d", __func__, focusMode);
        break;
    }

    return ret;
}

int32_t QCamera2HardwareInterface::processZoomEvent(uint32_t /*status*/)
{
    int32_t ret = NO_ERROR;

    for (int i = 0; i < QCAMERA_CH_TYPE_MAX; i++) {
        if (m_channels[i] != NULL) {
            ret = m_channels[i]->processZoomDone(mPreviewWindow);
        }
    }
    return ret;
}

int32_t QCamera2HardwareInterface::processJpegNotify(qcamera_jpeg_evt_payload_t *jpeg_evt)
{
    return m_postprocessor.processJpegEvt(jpeg_evt);
}

void QCamera2HardwareInterface::lockAPI()
{
    pthread_mutex_lock(&m_lock);
}

void QCamera2HardwareInterface::waitAPIResult(qcamera_sm_evt_enum_t api_evt)
{
    ALOGD("%s: wait for API result of evt (%d)", __func__, api_evt);
    memset(&m_apiResult, 0, sizeof(qcamera_api_result_t));
    while (m_apiResult.request_api != api_evt) {
        pthread_cond_wait(&m_cond, &m_lock);
    }
    ALOGD("%s: return from API result wait for evt (%d)", __func__, api_evt);
}

void QCamera2HardwareInterface::unlockAPI()
{
    pthread_mutex_unlock(&m_lock);
}

void QCamera2HardwareInterface::signalAPIResult(qcamera_api_result_t *result)
{
    pthread_mutex_lock(&m_lock);
    m_apiResult = *result;
    pthread_cond_signal(&m_cond);
    pthread_mutex_unlock(&m_lock);
}

int32_t QCamera2HardwareInterface::addPreviewChannel()
{
    int32_t rc = NO_ERROR;
    QCameraChannel *pChannel = NULL;

    if (m_channels[QCAMERA_CH_TYPE_PREVIEW] != NULL) {
        // if we had preview channel before, delete it first
        delete m_channels[QCAMERA_CH_TYPE_PREVIEW];
        m_channels[QCAMERA_CH_TYPE_PREVIEW] = NULL;
    }

    pChannel = new QCameraChannel(mCameraHandle->camera_handle,
                                  mCameraHandle->ops);
    if (NULL == pChannel) {
        ALOGE("%s: no mem for preview channel", __func__);
        return NO_MEMORY;
    }

    // preview only channel, don't need bundle attr and cb
    rc = pChannel->init(NULL, NULL, NULL);
    if (rc != NO_ERROR) {
        ALOGE("%s: init preview channel failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    if (isNoDisplayMode()) {
        rc = pChannel->addStream(*this,
                                 CAM_STREAM_TYPE_PREVIEW,
                                 &gCamCapability[mCameraId]->padding_info,
                                 nodisplay_preview_stream_cb_routine, this);
    } else {
        rc = pChannel->addStream(*this,
                                 CAM_STREAM_TYPE_PREVIEW,
                                 &gCamCapability[mCameraId]->padding_info,
                                 preview_stream_cb_routine, this);
    }
    if (rc != NO_ERROR) {
        ALOGE("%s: add preview stream failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    m_channels[QCAMERA_CH_TYPE_PREVIEW] = pChannel;
    return rc;
}

int32_t QCamera2HardwareInterface::addVideoChannel()
{
    int32_t rc = NO_ERROR;
    QCameraVideoChannel *pChannel = NULL;

    if (m_channels[QCAMERA_CH_TYPE_VIDEO] != NULL) {
        // if we had video channel before, delete it first
        delete m_channels[QCAMERA_CH_TYPE_VIDEO];
        m_channels[QCAMERA_CH_TYPE_VIDEO] = NULL;
    }

    pChannel = new QCameraVideoChannel(mCameraHandle->camera_handle,
                                       mCameraHandle->ops);
    if (NULL == pChannel) {
        ALOGE("%s: no mem for video channel", __func__);
        return NO_MEMORY;
    }

    // preview only channel, don't need bundle attr and cb
    rc = pChannel->init(NULL, NULL, NULL);
    if (rc != 0) {
        ALOGE("%s: init video channel failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    rc = pChannel->addStream(*this, CAM_STREAM_TYPE_VIDEO,
                             &gCamCapability[mCameraId]->padding_info,
                             video_stream_cb_routine, this);
    if (rc != NO_ERROR) {
        ALOGE("%s: add video stream failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    m_channels[QCAMERA_CH_TYPE_VIDEO] = pChannel;
    return rc;
}

int32_t QCamera2HardwareInterface::addSnapshotChannel()
{
    int32_t rc = NO_ERROR;
    QCameraChannel *pChannel = NULL;

    if (m_channels[QCAMERA_CH_TYPE_SNAPSHOT] != NULL) {
        // if we had ZSL channel before, delete it first
        delete m_channels[QCAMERA_CH_TYPE_SNAPSHOT];
        m_channels[QCAMERA_CH_TYPE_SNAPSHOT] = NULL;
    }

    pChannel = new QCameraChannel(mCameraHandle->camera_handle,
                                  mCameraHandle->ops);
    if (NULL == pChannel) {
        ALOGE("%s: no mem for snapshot channel", __func__);
        return NO_MEMORY;
    }

    rc = pChannel->init(NULL, NULL, NULL);
    if (rc != NO_ERROR) {
        ALOGE("%s: init snapshot channel failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    rc = pChannel->addStream(*this, CAM_STREAM_TYPE_SNAPSHOT,
                             &gCamCapability[mCameraId]->padding_info,
                             snapshot_stream_cb_routine, this);

    if (rc != NO_ERROR) {
        ALOGE("%s: add snapshot stream failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    m_channels[QCAMERA_CH_TYPE_SNAPSHOT] = pChannel;
    return rc;
}

int32_t QCamera2HardwareInterface::addRawChannel()
{
    int32_t rc = NO_ERROR;
    QCameraChannel *pChannel = NULL;

    if (m_channels[QCAMERA_CH_TYPE_RAW] != NULL) {
        // if we had raw channel before, delete it first
        delete m_channels[QCAMERA_CH_TYPE_RAW];
        m_channels[QCAMERA_CH_TYPE_RAW] = NULL;
    }

    pChannel = new QCameraChannel(mCameraHandle->camera_handle,
                                  mCameraHandle->ops);
    if (NULL == pChannel) {
        ALOGE("%s: no mem for raw channel", __func__);
        return NO_MEMORY;
    }

    rc = pChannel->init(NULL, NULL, NULL);
    if (rc != NO_ERROR) {
        ALOGE("%s: init raw channel failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    rc = pChannel->addStream(*this, CAM_STREAM_TYPE_RAW,
                             &gCamCapability[mCameraId]->padding_info,
                             raw_stream_cb_routine, this);

    if (rc != NO_ERROR) {
        ALOGE("%s: add snapshot stream failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    m_channels[QCAMERA_CH_TYPE_RAW] = pChannel;
    return rc;
}

int32_t QCamera2HardwareInterface::addZSLChannel()
{
    int32_t rc = NO_ERROR;
    QCameraPicChannel *pChannel = NULL;

    if (m_channels[QCAMERA_CH_TYPE_ZSL] != NULL) {
        // if we had ZSL channel before, delete it first
        delete m_channels[QCAMERA_CH_TYPE_ZSL];
        m_channels[QCAMERA_CH_TYPE_ZSL] = NULL;
    }

    pChannel = new QCameraPicChannel(mCameraHandle->camera_handle,
                                     mCameraHandle->ops);
    if (NULL == pChannel) {
        ALOGE("%s: no mem for ZSL channel", __func__);
        return NO_MEMORY;
    }

    // ZSL channel, init with bundle attr and cb
    mm_camera_channel_attr_t attr;
    memset(&attr, 0, sizeof(mm_camera_channel_attr_t));
    attr.notify_mode = MM_CAMERA_SUPER_BUF_NOTIFY_BURST;
    attr.look_back = mParameters.getZSLBackLookCount();
    attr.post_frame_skip = mParameters.getZSLBurstInterval();
    attr.water_mark = mParameters.getZSLQueueDepth();
    rc = pChannel->init(&attr,
                        zsl_channel_cb,
                        this);
    if (rc != 0) {
        ALOGE("%s: init ZSL channel failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    if (isNoDisplayMode()) {
        rc = pChannel->addStream(*this, CAM_STREAM_TYPE_PREVIEW,
                                 &gCamCapability[mCameraId]->padding_info,
                                 nodisplay_preview_stream_cb_routine, this);
    } else {
        rc = pChannel->addStream(*this, CAM_STREAM_TYPE_PREVIEW,
                                 &gCamCapability[mCameraId]->padding_info,
                                 preview_stream_cb_routine, this);
    }
    if (rc != NO_ERROR) {
        ALOGE("%s: add preview stream failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    rc = pChannel->addStream(*this, CAM_STREAM_TYPE_SNAPSHOT,
                             &gCamCapability[mCameraId]->padding_info,
                             NULL, this);
    if (rc != NO_ERROR) {
        ALOGE("%s: add snapshot stream failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    m_channels[QCAMERA_CH_TYPE_ZSL] = pChannel;
    return rc;
}

int32_t QCamera2HardwareInterface::addCaptureChannel()
{
    int32_t rc = NO_ERROR;
    QCameraChannel *pChannel = NULL;

    if (m_channels[QCAMERA_CH_TYPE_CAPTURE] != NULL) {
        delete m_channels[QCAMERA_CH_TYPE_CAPTURE];
        m_channels[QCAMERA_CH_TYPE_CAPTURE] = NULL;
    }

    pChannel = new QCameraChannel(mCameraHandle->camera_handle,
                                  mCameraHandle->ops);
    if (NULL == pChannel) {
        ALOGE("%s: no mem for capture channel", __func__);
        return NO_MEMORY;
    }

    // Capture channel, only need snapshot and postview streams start together
    rc = pChannel->init(NULL,
                        NULL,
                        NULL);
    if (rc != NO_ERROR) {
        ALOGE("%s: init capture channel failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    rc = pChannel->addStream(*this, CAM_STREAM_TYPE_POSTVIEW,
                             &gCamCapability[mCameraId]->padding_info,
                             postview_stream_cb_routine, this);

    if (rc != NO_ERROR) {
        ALOGE("%s: add postview stream failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    rc = pChannel->addStream(*this, CAM_STREAM_TYPE_SNAPSHOT,
                             &gCamCapability[mCameraId]->padding_info,
                             snapshot_stream_cb_routine, this);
    if (rc != NO_ERROR) {
        ALOGE("%s: add snapshot stream failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    m_channels[QCAMERA_CH_TYPE_CAPTURE] = pChannel;
    return rc;
}

int32_t QCamera2HardwareInterface::addMetaDataChannel()
{
    int32_t rc = NO_ERROR;
    QCameraChannel *pChannel = NULL;

    if (m_channels[QCAMERA_CH_TYPE_METADATA] != NULL) {
        delete m_channels[QCAMERA_CH_TYPE_METADATA];
        m_channels[QCAMERA_CH_TYPE_METADATA] = NULL;
    }

    pChannel = new QCameraChannel(mCameraHandle->camera_handle,
                                  mCameraHandle->ops);
    if (NULL == pChannel) {
        ALOGE("%s: no mem for metadata channel", __func__);
        return NO_MEMORY;
    }

    rc = pChannel->init(NULL,
                        NULL,
                        NULL);
    if (rc != NO_ERROR) {
        ALOGE("%s: init metadata channel failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    rc = pChannel->addStream(*this, CAM_STREAM_TYPE_METADATA,
                             &gCamCapability[mCameraId]->padding_info,
                             metadata_stream_cb_routine, this);

    if (rc != NO_ERROR) {
        ALOGE("%s: add metadata stream failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    m_channels[QCAMERA_CH_TYPE_METADATA] = pChannel;
    return rc;
}

int32_t QCamera2HardwareInterface::addReprocessChannel()
{
    int32_t rc = NO_ERROR;
    QCameraChannel *pChannel = NULL;

    if (m_channels[QCAMERA_CH_TYPE_REPROCESS] != NULL) {
        delete m_channels[QCAMERA_CH_TYPE_REPROCESS];
        m_channels[QCAMERA_CH_TYPE_REPROCESS] = NULL;
    }

    pChannel = new QCameraChannel(mCameraHandle->camera_handle,
                                  mCameraHandle->ops);
    if (NULL == pChannel) {
        ALOGE("%s: no mem for reprocess channel", __func__);
        return NO_MEMORY;
    }

    rc = pChannel->init(NULL, NULL, NULL);
    if (rc != NO_ERROR) {
        ALOGE("%s: init reprocess channel failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    rc = pChannel->addStream(*this, CAM_STREAM_TYPE_OFFLINE_PROC,
                             &gCamCapability[mCameraId]->padding_info,
                             reprocess_stream_cb_routine, this);
    if (rc != NO_ERROR) {
        ALOGE("%s: add reprocess stream failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    m_channels[QCAMERA_CH_TYPE_REPROCESS] = pChannel;
    return rc;
}

int32_t QCamera2HardwareInterface::addChannel(qcamera_ch_type_enum_t ch_type)
{
    int32_t rc = UNKNOWN_ERROR;
    switch (ch_type) {
    case QCAMERA_CH_TYPE_ZSL:
        rc = addZSLChannel();
        break;
    case QCAMERA_CH_TYPE_CAPTURE:
        rc = addCaptureChannel();
        break;
    case QCAMERA_CH_TYPE_PREVIEW:
        rc = addPreviewChannel();
        break;
    case QCAMERA_CH_TYPE_VIDEO:
        rc = addVideoChannel();
        break;
    case QCAMERA_CH_TYPE_SNAPSHOT:
        rc = addSnapshotChannel();
        break;
    case QCAMERA_CH_TYPE_RAW:
        rc = addRawChannel();
        break;
    case QCAMERA_CH_TYPE_METADATA:
        rc = addMetaDataChannel();
        break;
    case QCAMERA_CH_TYPE_REPROCESS:
        rc = addReprocessChannel();
        break;
    default:
        break;
    }
    return rc;
}

int32_t QCamera2HardwareInterface::delChannel(qcamera_ch_type_enum_t ch_type)
{
    if (m_channels[ch_type] != NULL) {
        delete m_channels[ch_type];
        m_channels[ch_type] = NULL;
    }

    return NO_ERROR;
}

int32_t QCamera2HardwareInterface::startChannel(qcamera_ch_type_enum_t ch_type)
{
    int32_t rc = UNKNOWN_ERROR;
    if (m_channels[ch_type] != NULL) {
        rc = m_channels[ch_type]->start();
    }

    return rc;
}

int32_t QCamera2HardwareInterface::stopChannel(qcamera_ch_type_enum_t ch_type)
{
    int32_t rc = UNKNOWN_ERROR;
    if (m_channels[ch_type] != NULL) {
        rc = m_channels[ch_type]->stop();
    }

    return rc;
}

int32_t QCamera2HardwareInterface::preparePreview()
{
    int32_t rc = NO_ERROR;
    // TODO: commented out for now
    //rc = addChannel(QCAMERA_CH_TYPE_METADATA);
    if (rc != NO_ERROR) {
        return rc;
    }

    if (mParameters.isZSLMode()) {
        rc = addChannel(QCAMERA_CH_TYPE_ZSL);
        if (rc != NO_ERROR) {
            delChannel(QCAMERA_CH_TYPE_METADATA);
            return rc;
        }
    } else {
        rc = addChannel(QCAMERA_CH_TYPE_PREVIEW);
        if (rc != NO_ERROR) {
            delChannel(QCAMERA_CH_TYPE_METADATA);
            return rc;
        }

        if(mParameters.getRecordingHintValue() == true) {
            rc = addChannel(QCAMERA_CH_TYPE_VIDEO);
            if (rc != NO_ERROR) {
                delChannel(QCAMERA_CH_TYPE_METADATA);
                delChannel(QCAMERA_CH_TYPE_PREVIEW);
                return rc;
            }

            rc = addChannel(QCAMERA_CH_TYPE_SNAPSHOT);
            if (rc != NO_ERROR) {
                delChannel(QCAMERA_CH_TYPE_METADATA);
                delChannel(QCAMERA_CH_TYPE_PREVIEW);
                delChannel(QCAMERA_CH_TYPE_VIDEO);
            }
        }
    }

    return rc;
}

void QCamera2HardwareInterface::unpreparePreview()
{
    if (mParameters.isZSLMode()) {
        delChannel(QCAMERA_CH_TYPE_ZSL);
    } else {
        delChannel(QCAMERA_CH_TYPE_PREVIEW);
        if(mParameters.getRecordingHintValue() == true) {
            delChannel(QCAMERA_CH_TYPE_VIDEO);
            delChannel(QCAMERA_CH_TYPE_SNAPSHOT);
        }
    }

    delChannel(QCAMERA_CH_TYPE_METADATA);
}

void QCamera2HardwareInterface::playShutter(){
     ALOGV("%s : E", __func__);
     if (mNotifyCb == NULL ||
         msgTypeEnabled(CAMERA_MSG_SHUTTER) == 0){
         ALOGV("%s: shutter msg not enabled or NULL cb", __func__);
         return;
     }

     if(!m_bShutterSoundPlayed){
         mNotifyCb(CAMERA_MSG_SHUTTER, 0, true, mCallbackCookie);
     }
     mNotifyCb(CAMERA_MSG_SHUTTER, 0, false, mCallbackCookie);
     m_bShutterSoundPlayed = false;
     ALOGV("%s : X", __func__);
}

QCameraChannel *QCamera2HardwareInterface::getChannelByHandle(uint32_t channelHandle)
{
    for(int i = 0; i < QCAMERA_CH_TYPE_MAX; i++) {
        if (m_channels[i] != NULL &&
            m_channels[i]->getMyHandle() == channelHandle) {
            return m_channels[i];
        }
    }

    return NULL;
}

int32_t QCamera2HardwareInterface::processFaceDetectionReuslt(cam_face_detection_data_t *fd_data)
{
    if (!mParameters.isFaceDetectionEnabled()) {
        ALOGD("%s: FaceDetection not enabled, no ops here", __func__);
        return NO_ERROR;
    }

    if ((NULL == mDataCb) || (msgTypeEnabled(CAMERA_MSG_PREVIEW_METADATA) == 0)) {
        ALOGD("%s: prevew metadata msgtype not enabled, no ops here", __func__);
        return NO_ERROR;
    }

    cam_dimension_t display_dim;
    mParameters.getStreamDimension(CAM_STREAM_TYPE_PREVIEW, display_dim);
    if (display_dim.width <= 0 || display_dim.height <= 0) {
        ALOGE("%s: Invalid preview width or height (%d x %d)",
              __func__, display_dim.width, display_dim.height);
        return UNKNOWN_ERROR;
    }

    // process face detection result
    memset(&mRoiData, 0, sizeof(mRoiData));
    memset(mFaces, 0, sizeof(mFaces));
    mRoiData.number_of_faces = fd_data->num_faces_detected;
    mRoiData.faces = mFaces;
    if (mRoiData.number_of_faces > 0) {
        for (int i = 0; i < mRoiData.number_of_faces; i++) {
            mFaces[i].id = fd_data->faces[i].face_id;
            mFaces[i].score = fd_data->faces[i].score;

            // left
            mFaces[i].rect[0] =
                MAP_TO_DRIVER_COORDINATE(fd_data->faces[i].face_boundary.left, display_dim.width, 2000, -1000);

            // top
            mFaces[i].rect[1] =
                MAP_TO_DRIVER_COORDINATE(fd_data->faces[i].face_boundary.top, display_dim.height, 2000, -1000);

            // right
            mFaces[i].rect[2] = mFaces[i].rect[0] +
                MAP_TO_DRIVER_COORDINATE(fd_data->faces[i].face_boundary.width, display_dim.width, 2000, 0);

             // bottom
            mFaces[i].rect[2] = mFaces[i].rect[1] +
                MAP_TO_DRIVER_COORDINATE(fd_data->faces[i].face_boundary.height, display_dim.height, 2000, 0);

            // Center of left eye
            mFaces[i].left_eye[0] =
                MAP_TO_DRIVER_COORDINATE(fd_data->faces[i].left_eye_center.x, display_dim.width, 2000, -1000);
            mFaces[i].left_eye[0] =
                MAP_TO_DRIVER_COORDINATE(fd_data->faces[i].left_eye_center.y, display_dim.height, 2000, -1000);

            // Center of right eye
            mFaces[i].right_eye[0] =
                MAP_TO_DRIVER_COORDINATE(fd_data->faces[i].right_eye_center.x, display_dim.width, 2000, -1000);
            mFaces[i].right_eye[0] =
                MAP_TO_DRIVER_COORDINATE(fd_data->faces[i].right_eye_center.y, display_dim.height, 2000, -1000);

            // Center of mouth
            mFaces[i].mouth[0] =
                MAP_TO_DRIVER_COORDINATE(fd_data->faces[i].mouth_center.x, display_dim.width, 2000, -1000);
            mFaces[i].mouth[0] =
                MAP_TO_DRIVER_COORDINATE(fd_data->faces[i].mouth_center.y, display_dim.height, 2000, -1000);


#if 0 //Disable for now before these fields are added in camera_face_t in frameworks
            mFaces[i].smile_degree = fd_data->faces[i].smile_degree;
            mFaces[i].smile_score = fd_data->faces[i].smile_confidence;
            mFaces[i].blink_detected = fd_data->faces[i].blink_detected;
            mFaces[i].face_recognised = fd_data->faces[i].face_recognised;
            mFaces[i].gaze_angle = fd_data->faces[i].gaze_angle;

            // upscale by 2 to recover from demaen downscaling
            mFaces[i].updown_dir = fd_data->faces[i].updown_dir * 2;
            mFaces[i].leftright_dir = fd_data->faces[i].leftright_dir * 2;
            mFaces[i].roll_dir = fd_data->faces[i].roll_dir * 2;

            mFaces[i].leye_blink = fd_data->faces[i].left_blink;
            mFaces[i].reye_blink = fd_data->faces[i].right_blink;
            mFaces[i].left_right_gaze = fd_data->faces[i].left_right_gaze;
            mFaces[i].top_bottom_gaze = fd_data->faces[i].top_bottom_gaze;
#endif
        }
    }

    mDataCb(CAMERA_MSG_PREVIEW_METADATA, NULL, 0, &mRoiData, mCallbackCookie);
    return NO_ERROR;
}

int32_t QCamera2HardwareInterface::processHistogramStats(cam_histogram_data_t *hist_data)
{
    if (!mParameters.isHistogramEnabled()) {
        ALOGD("%s: Histogram not enabled, no ops here", __func__);
        return NO_ERROR;
    }

    if (m_pHistBuf == NULL) {
        ALOGE("%s: m_pHistBuf is NULL", __func__);
        return UNKNOWN_ERROR;
    }

    cam_histogram_data_t *pHistData = (cam_histogram_data_t *)m_pHistBuf->data;
    if (pHistData == NULL) {
        ALOGE("%s: memory data ptr is NULL", __func__);
        return UNKNOWN_ERROR;
    }

    *pHistData = *hist_data;

    if ((NULL != mDataCb) && (msgTypeEnabled(CAMERA_MSG_STATS_DATA) > 0)) {
        mDataCb(CAMERA_MSG_STATS_DATA, m_pHistBuf, 0, NULL, mCallbackCookie);
    }

    return NO_ERROR;
}

int QCamera2HardwareInterface::updateParameters(const char *parms, bool &needRestart)
{
    String8 str = String8(parms);
    QCameraParameters param(str);
    return mParameters.updateParameters(param, needRestart);
}

int QCamera2HardwareInterface::commitParameterChanges()
{
    return mParameters.commitParameters();
}

bool QCamera2HardwareInterface::needDebugFps()
{
    return mParameters.isFpsDebugEnabled();
}

bool QCamera2HardwareInterface::needOfflineReprocess()
{
    if (mParameters.isZSLMode() && mParameters.isWNREnabled()) {
        return true;
    } else {
        return false;
    }
}

void QCamera2HardwareInterface::getThumbnailSize(cam_dimension_t &dim)
{
    mParameters.getThumbnailSize(&dim.width, &dim.height);
}

int QCamera2HardwareInterface::getJpegQuality()
{
    return mParameters.getJpegQuality();
}

int QCamera2HardwareInterface::getJpegRotation() {
    return mParameters.getJpegRotation();
}

QCameraExif *QCamera2HardwareInterface::getExifData()
{
    QCameraExif *exif = new QCameraExif();
    if (exif == NULL) {
        ALOGE("%s: No memory for QCameraExif", __func__);
        return NULL;
    }

    int32_t rc = NO_ERROR;
    uint32_t count = 0;

    // add exif entries
    char dateTime[20];
    memset(dateTime, 0, sizeof(dateTime));
    count = 20;
    rc = mParameters.getExifDateTime(dateTime, count);
    if(rc == NO_ERROR) {
        exif->addEntry(EXIFTAGID_EXIF_DATE_TIME_ORIGINAL,
                       EXIF_ASCII,
                       count,
                       (void *)dateTime);
    }

    rat_t focalLength;
    rc = mParameters.getExifFocalLength(&focalLength);
    if (rc == NO_ERROR) {
        exif->addEntry(EXIFTAGID_FOCAL_LENGTH,
                       EXIF_RATIONAL,
                       1,
                       (void *)&(focalLength));
    } else {
        ALOGE("%s: getExifFocalLength failed", __func__);
    }

    uint16_t isoSpeed = mParameters.getExifIsoSpeed();
    exif->addEntry(EXIFTAGID_ISO_SPEED_RATING,
                   EXIF_SHORT,
                   1,
                   (void *)&(isoSpeed));

    char gpsProcessingMethod[EXIF_ASCII_PREFIX_SIZE + GPS_PROCESSING_METHOD_SIZE];
    count = 0;
    rc = mParameters.getExifGpsProcessingMethod(gpsProcessingMethod, count);
    if(rc == NO_ERROR) {
        exif->addEntry(EXIFTAGID_GPS_PROCESSINGMETHOD,
                       EXIF_ASCII,
                       count,
                       (void *)gpsProcessingMethod);
    }

    rat_t latitude[3];
    char latRef[2];
    rc = mParameters.getExifLatitude(latitude, latRef);
    if(rc == NO_ERROR) {
        exif->addEntry(EXIFTAGID_GPS_LATITUDE,
                       EXIF_RATIONAL,
                       3,
                       (void *)latitude);
        exif->addEntry(EXIFTAGID_GPS_LATITUDE_REF,
                       EXIF_ASCII,
                       2,
                       (void *)latRef);
    } else {
        ALOGE("%s: getExifLatitude failed", __func__);
    }

    rat_t longitude[3];
    char lonRef[2];
    rc = mParameters.getExifLongitude(longitude, lonRef);
    if(rc == NO_ERROR) {
        exif->addEntry(EXIFTAGID_GPS_LONGITUDE,
                       EXIF_RATIONAL,
                       3,
                       (void *)longitude);

        exif->addEntry(EXIFTAGID_GPS_LONGITUDE_REF,
                       EXIF_ASCII,
                       2,
                       (void *)lonRef);
    } else {
        ALOGE("%s: getExifLongitude failed", __func__);
    }

    rat_t altitude;
    char altRef;
    rc = mParameters.getExifAltitude(&altitude, &altRef);
    if(rc == NO_ERROR) {
        exif->addEntry(EXIFTAGID_GPS_ALTITUDE,
                       EXIF_RATIONAL,
                       1,
                       (void *)&(altitude));

        exif->addEntry(EXIFTAGID_GPS_ALTITUDE_REF,
                       EXIF_BYTE,
                       1,
                       (void *)&altRef);
    } else {
        ALOGE("%s: getExifAltitude failed", __func__);
    }

    char gpsDateStamp[20];
    rat_t gpsTimeStamp[3];
    rc = mParameters.getExifGpsDateTimeStamp(gpsDateStamp, 20, gpsTimeStamp);
    if(rc == NO_ERROR) {
        exif->addEntry(EXIFTAGID_GPS_DATESTAMP,
                       EXIF_ASCII,
                       strlen(gpsDateStamp) + 1,
                       (void *)gpsDateStamp);

        exif->addEntry(EXIFTAGID_GPS_TIMESTAMP,
                       EXIF_RATIONAL,
                       3,
                       (void *)gpsTimeStamp);
    } else {
        ALOGE("%s: getExifGpsDataTimeStamp failed", __func__);
    }

    return exif;
}

int32_t QCamera2HardwareInterface::setHistogram(bool histogram_en)
{
    return mParameters.setHistogram(histogram_en);
}

int32_t QCamera2HardwareInterface::setFaceDetection(bool enabled)
{
    return mParameters.setFaceDetection(enabled);
}

int32_t QCamera2HardwareInterface::prepareHardwareForSnapshot()
{
    return mCameraHandle->ops->prepare_snapshot(mCameraHandle->camera_handle);
}

}; // namespace qcamera
