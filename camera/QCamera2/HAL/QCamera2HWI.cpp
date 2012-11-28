/* Copyright (c) 2012, The Linux Foundataion. All rights reserved.
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

#include "QCamera2HWI.h"
#include "QCameraMem.h"

namespace android {

cam_capapbility_t gCamCapability[MM_CAMERA_MAX_NUM_SENSORS];

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
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }

    hw->lockAPI();
    hw->processAPI(QCAMERA_SM_EVT_SET_PREVIEW_WINDOW, (void *)window);
    hw->waitAPIResult(QCAMERA_SM_EVT_SET_PREVIEW_WINDOW);
    rc = hw->m_apiResult.status;
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
    hw->processAPI(QCAMERA_SM_EVT_SET_CALLBACKS, (void *)&payload);
    hw->waitAPIResult(QCAMERA_SM_EVT_SET_CALLBACKS);
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
    hw->processAPI(QCAMERA_SM_EVT_ENABLE_MSG_TYPE, (void *)msg_type);
    hw->waitAPIResult(QCAMERA_SM_EVT_ENABLE_MSG_TYPE);
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
    hw->processAPI(QCAMERA_SM_EVT_DISABLE_MSG_TYPE, (void *)msg_type);
    hw->waitAPIResult(QCAMERA_SM_EVT_DISABLE_MSG_TYPE);
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
    hw->processAPI(QCAMERA_SM_EVT_MSG_TYPE_ENABLED, (void *)msg_type);
    hw->waitAPIResult(QCAMERA_SM_EVT_MSG_TYPE_ENABLED);
    ret = hw->m_apiResult.enabled;
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
    if (hw->isNoDisplayMode()) {
        hw->processAPI(QCAMERA_SM_EVT_START_NODISPLAY_PREVIEW, NULL);
        hw->waitAPIResult(QCAMERA_SM_EVT_START_NODISPLAY_PREVIEW);
    } else {
        hw->processAPI(QCAMERA_SM_EVT_START_PREVIEW, NULL);
        hw->waitAPIResult(QCAMERA_SM_EVT_START_PREVIEW);
    }
    ret = hw->m_apiResult.status;
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
    hw->processAPI(QCAMERA_SM_EVT_STOP_PREVIEW, NULL);
    hw->waitAPIResult(QCAMERA_SM_EVT_STOP_PREVIEW);
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
    hw->processAPI(QCAMERA_SM_EVT_PREVIEW_ENABLED, NULL);
    hw->waitAPIResult(QCAMERA_SM_EVT_PREVIEW_ENABLED);
    ret = hw->m_apiResult.enabled;
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
    hw->processAPI(QCAMERA_SM_EVT_STORE_METADATA_IN_BUFS, (void *)enable);
    hw->waitAPIResult(QCAMERA_SM_EVT_STORE_METADATA_IN_BUFS);
    ret = hw->m_apiResult.status;
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
    hw->processAPI(QCAMERA_SM_EVT_START_RECORDING, NULL);
    hw->waitAPIResult(QCAMERA_SM_EVT_START_RECORDING);
    ret = hw->m_apiResult.status;
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
    hw->processAPI(QCAMERA_SM_EVT_STOP_RECORDING, NULL);
    hw->waitAPIResult(QCAMERA_SM_EVT_STOP_RECORDING);
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
    hw->processAPI(QCAMERA_SM_EVT_RECORDING_ENABLED, NULL);
    hw->waitAPIResult(QCAMERA_SM_EVT_RECORDING_ENABLED);
    ret = hw->m_apiResult.enabled;
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
    hw->processAPI(QCAMERA_SM_EVT_RELEASE_RECORIDNG_FRAME, (void *)opaque);
    hw->waitAPIResult(QCAMERA_SM_EVT_RELEASE_RECORIDNG_FRAME);
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
    hw->processAPI(QCAMERA_SM_EVT_START_AUTO_FOCUS, NULL);
    hw->waitAPIResult(QCAMERA_SM_EVT_START_AUTO_FOCUS);
    ret = hw->m_apiResult.status;
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
    hw->processAPI(QCAMERA_SM_EVT_STOP_AUTO_FOCUS, NULL);
    hw->waitAPIResult(QCAMERA_SM_EVT_STOP_AUTO_FOCUS);
    ret = hw->m_apiResult.status;
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
    hw->processAPI(QCAMERA_SM_EVT_TAKE_PICTURE, NULL);
    hw->waitAPIResult(QCAMERA_SM_EVT_TAKE_PICTURE);
    ret = hw->m_apiResult.status;
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
    hw->processAPI(QCAMERA_SM_EVT_CANCEL_PICTURE, NULL);
    hw->waitAPIResult(QCAMERA_SM_EVT_CANCEL_PICTURE);
    ret = hw->m_apiResult.status;
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
    hw->processAPI(QCAMERA_SM_EVT_SET_PARAMS, (void *)parms);
    hw->waitAPIResult(QCAMERA_SM_EVT_SET_PARAMS);
    ret = hw->m_apiResult.status;
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
    hw->processAPI(QCAMERA_SM_EVT_GET_PARAMS, NULL);
    hw->waitAPIResult(QCAMERA_SM_EVT_GET_PARAMS);
    ret = hw->m_apiResult.params;
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
    hw->processAPI(QCAMERA_SM_EVT_PUT_PARAMS, (void *)parm);
    hw->waitAPIResult(QCAMERA_SM_EVT_PUT_PARAMS);
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
    hw->processAPI(QCAMERA_SM_EVT_SEND_COMMAND, (void *)&payload);
    hw->waitAPIResult(QCAMERA_SM_EVT_SEND_COMMAND);
    ret = hw->m_apiResult.status;
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
    hw->processAPI(QCAMERA_SM_EVT_RELEASE, NULL);
    hw->waitAPIResult(QCAMERA_SM_EVT_RELEASE);
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
    hw->processAPI(QCAMERA_SM_EVT_DUMP, (void *)fd);
    hw->waitAPIResult(QCAMERA_SM_EVT_DUMP);
    ret = hw->m_apiResult.status;
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
    : m_stateMachine(this),
      m_postprocessor(this),
      mRecordingHint(false),
      mSmoothZoomRunning(false),
      mDenoiseValue(0),
      mDumpFrameEnabled(0),
      mDebugFps(false),
      mShutterSoundPlayed(false),
      mNoDisplayMode(false)
{
    mCameraId = cameraId;
    mCameraDevice.common.tag = HARDWARE_DEVICE_TAG;
    mCameraDevice.common.version = HARDWARE_DEVICE_API_VERSION(1, 0);
    mCameraDevice.common.close = close_camera_device;
    mCameraDevice.ops = &mCameraOps;
    mCameraDevice.priv = this;
    mCameraHandle = NULL;

    pthread_mutex_init(&m_lock, NULL);
    pthread_cond_init(&m_cond, NULL);
    memset(&m_apiResult, 0, sizeof(qcamera_api_result_t));

    char value[32];
    property_get("persist.debug.sf.showfps", value, "0");
    mDebugFps = atoi(value) > 0 ? true : false;
    property_get("persist.camera.dumpimg", value, "0");
    mDumpFrameEnabled = atoi(value);

    memset(m_channels, 0, sizeof(m_channels));
}

QCamera2HardwareInterface::~QCamera2HardwareInterface()
{
    closeCamera();
    pthread_mutex_destroy(&m_lock);
    pthread_cond_destroy(&m_cond);
}

int QCamera2HardwareInterface::openCamera(struct hw_device_t **hw_device)
{
    *hw_device = &mCameraDevice.common;
    return openCamera();
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
    int32_t rc = m_postprocessor.init(jpegEvtHandle, this);
    if (rc != 0) {
        ALOGE("Init Postprocessor failed");
        return UNKNOWN_ERROR;
    }

    return NO_ERROR;
}

int QCamera2HardwareInterface::closeCamera()
{
    int rc = NO_ERROR;
    int i;

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
    return rc;
}

#define DATA_PTR(MEM_OBJ,INDEX) MEM_OBJ->getPtr( INDEX )

int QCamera2HardwareInterface::getCapabilities(struct camera_info *info)
{

    int rc = NO_ERROR;
    mm_camera_vtbl_t *lCameraHandle=NULL;
    QCameraHeapMemory *capabilityHeap = NULL;
    lCameraHandle = camera_open(mCameraId);
    if (!lCameraHandle) {
        ALOGE("%s:camera_open failed",__func__);
        rc = UNKNOWN_ERROR;
        goto GET_CAP_DONE;
    }

    /*Allocate memory for capability buffer*/
    capabilityHeap = new QCameraHeapMemory();
    rc = capabilityHeap->allocate(1, sizeof(cam_capapbility_t));
    if(rc != OK) {
        rc = NO_MEMORY;
        goto GET_CAP_ERROR1;
    }

    /*Map memory for capability buffer*/
    rc = lCameraHandle->ops->map_buf(lCameraHandle->camera_handle,
                                     CAM_MAPPING_BUF_TYPE_CAPABILITY,
                                     capabilityHeap->getFd(0),
                                     sizeof(cam_capapbility_t));
    if(rc < 0) {
        ALOGE("%s:failed to map capability buffer",__func__);
        rc = FAILED_TRANSACTION;
        goto GET_CAP_ERROR2;
    }

    /*Query Capability*/
    rc = lCameraHandle->ops->query_capability(lCameraHandle->camera_handle);
    if(rc < 0) {
        ALOGE("%s:failed to query capability",__func__);
        rc = FAILED_TRANSACTION;
        goto GET_CAP_ERROR3;
    }

    memcpy(&gCamCapability[mCameraId], DATA_PTR(capabilityHeap,0), sizeof(cam_capapbility_t));

    switch(gCamCapability[mCameraId].position) {
        case CAM_POSITION_BACK:
                info->facing = CAMERA_FACING_BACK;
                break;

        case CAM_POSITION_FRONT:
                info->facing = CAMERA_FACING_FRONT;
                break;

        default:
                ALOGE("%s:Unknown position type for camera id:%d",__func__,mCameraId);
                rc = BAD_VALUE;
                break;
    }

    info->orientation=gCamCapability[mCameraId].sensor_mount_angle;

GET_CAP_ERROR3:
    lCameraHandle->ops->unmap_buf(lCameraHandle->camera_handle,
                                  CAM_MAPPING_BUF_TYPE_CAPABILITY);

GET_CAP_ERROR2:
    capabilityHeap->deallocate();

GET_CAP_ERROR1:
    delete capabilityHeap;

    lCameraHandle->ops->close_camera(lCameraHandle->camera_handle);
    lCameraHandle=NULL;

GET_CAP_DONE:
    return rc;
}

QCameraMemory *QCamera2HardwareInterface::allocateStreamBuf(
    cam_stream_type_t stream_type, int size)
{
    int rc = NO_ERROR;
    QCameraMemory *mem = NULL;
    int bufferCnt = 0;
    const int minStreamingBuffers = 3;
    const int minCaptureBuffers = getNumOfSnapshots();
    const int minVideoBuffers = 9;
    int zslQueueDepth = getZSLQueueDepth();

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
        QCameraGrallocMemory *grallocMemory = new QCameraGrallocMemory(mGetMemory);
        // Fill in properly widht, height, and format once parameter class is ready.
        grallocMemory->setWindowInfo(mPreviewWindow, 640, 480, HAL_PIXEL_FORMAT_YCrCb_420_SP);
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
    streamInfo->stream_type = stream_type;
    streamInfo->rotation = 0;
    streamInfo->offline_reproc_mask = 0;
    //Fill out stream info structure based on stream type and format
    switch (stream_type) {
    case CAM_STREAM_TYPE_PREVIEW:
    case CAM_STREAM_TYPE_POSTVIEW:
        streamInfo->fmt = mPreviewFormat;
        streamInfo->dim.width = mPreviewWidth;
        streamInfo->dim.height = mPreviewHeight;
        streamInfo->streaming_mode = CAM_STREAMING_MODE_CONTINUOUS;
        streamInfo->width_padding = CAM_PAD_TO_2;
        streamInfo->height_padding = CAM_PAD_TO_2;
        streamInfo->plane_padding = CAM_PAD_TO_2;
        break;
    case CAM_STREAM_TYPE_SNAPSHOT:
        streamInfo->fmt = CAM_FORMAT_YUV_420_NV21;
        break;
    case CAM_STREAM_TYPE_RAW:
    case CAM_STREAM_TYPE_METADATA:
    case CAM_STREAM_TYPE_OFFLINE_PROC:
        break;
    case CAM_STREAM_TYPE_VIDEO:
        streamInfo->fmt = CAM_FORMAT_YUV_420_NV12;
        break;
    case CAM_STREAM_TYPE_DEFAULT:
    case CAM_STREAM_TYPE_MAX:
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
    if (isZSLMode()) {
        rc = startChannel(QCAMERA_CH_TYPE_ZSL);
    } else {
        rc = startChannel(QCAMERA_CH_TYPE_PREVIEW);
    }

    return rc;
}

int QCamera2HardwareInterface::stopPreview()
{
    if (isZSLMode()) {
        stopChannel(QCAMERA_CH_TYPE_ZSL);
    } else {
        stopChannel(QCAMERA_CH_TYPE_PREVIEW);
    }

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
    if (mRecordingHint == false) {
        ALOGE("%s: start recording when hint is false, stop preview first", __func__);
        stopChannel(QCAMERA_CH_TYPE_PREVIEW);
        delChannel(QCAMERA_CH_TYPE_PREVIEW);

        // Set recording hint to TRUE
        mRecordingHint = true;
        setRecordingHintValue(mRecordingHint);

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
    uint32_t rc = UNKNOWN_ERROR;
    QCameraVideoChannel *pChannel =
        (QCameraVideoChannel *)m_channels[QCAMERA_CH_TYPE_VIDEO];
    if(pChannel != NULL) {
        rc = pChannel->releaseFrame(opaque, mStoreMetaDataInFrame > 0);
    }
    return rc;
}

int QCamera2HardwareInterface::autoFocus()
{
    // TODO
    return 0;
}

int QCamera2HardwareInterface::cancelAutoFocus()
{
    // TODO
    return 0;
}

int QCamera2HardwareInterface::takePicture()
{
    int rc = NO_ERROR;
    uint8_t numSnapshots = getNumOfSnapshots();

    // start postprocessor
    m_postprocessor.start();

    if (isZSLMode()) {
        if (isWNREnabled()) {
            // need to do WNR for ZSL
            // add reprocess channel
            rc = addChannel(QCAMERA_CH_TYPE_REPROCESS);
            if (rc != 0) {
                ALOGE("%s: cannot add reprocess channel", __func__);
                m_postprocessor.stop();
                return rc;
            }
            rc = startChannel(QCAMERA_CH_TYPE_REPROCESS);
            if (rc != NO_ERROR) {
                ALOGE("%s: cannot start reprocess channel", __func__);
                m_postprocessor.stop();
                delChannel(QCAMERA_CH_TYPE_REPROCESS);
                return rc;
            }
        }

        QCameraPicChannel *pZSLChannel =
            (QCameraPicChannel *)m_channels[QCAMERA_CH_TYPE_ZSL];
        if (NULL != pZSLChannel) {
            rc = pZSLChannel->takePicture(numSnapshots);
            // disbale preview msg type
            disableMsgType(CAMERA_MSG_PREVIEW_FRAME);
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
        // need to stop preview channel
        stopChannel(QCAMERA_CH_TYPE_PREVIEW);
        delChannel(QCAMERA_CH_TYPE_PREVIEW);

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

    if (isZSLMode()) {
        if (isWNREnabled()) {
            stopChannel(QCAMERA_CH_TYPE_REPROCESS);
            delChannel(QCAMERA_CH_TYPE_REPROCESS);
        }

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

int QCamera2HardwareInterface::setParameters(const char * parms)
{
    QCameraParameters param;
    String8 str = String8(parms);
    param.unflatten(str);
    return setParameters(param);
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

int QCamera2HardwareInterface::sendCommand(int32_t /*cmd*/, int32_t /*arg1*/, int32_t /*arg2*/)
{
    // TODO
    return 0;
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
    if (obj) {
        obj->processEvt(QCAMERA_SM_EVT_EVT_NOTIFY, evt);
    } else {
        ALOGE("%s: NULL user_data", __func__);
    }
}

void QCamera2HardwareInterface::jpegEvtHandle(jpeg_job_status_t status,
                                              uint8_t thumbnailDroppedFlag,
                                              uint32_t /*client_hdl*/,
                                              uint32_t jobId,
                                              uint8_t* out_data,
                                              uint32_t data_size,
                                              void *userdata)
{
    QCamera2HardwareInterface *obj = (QCamera2HardwareInterface *)userdata;
    if (obj) {
        qcamera_jpeg_evt_payload_t *payload =
            (qcamera_jpeg_evt_payload_t *)malloc(sizeof(qcamera_jpeg_evt_payload_t));
        if (NULL != payload) {
            memset(payload, 0, sizeof(qcamera_jpeg_evt_payload_t));
            payload->status = status;
            payload->jobId = jobId;
            payload->thumbnailDroppedFlag = thumbnailDroppedFlag;
            payload->out_data = out_data;
            payload->data_size = data_size;
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
            ret = sem_wait(&cmdThread->cmd_sem);
            if (ret != 0 && errno != EINVAL) {
                ALOGE("%s: sem_wait error (%s)",
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

int32_t QCamera2HardwareInterface::processAutoFocusEvent(uint32_t status)
{
    int32_t ret = NO_ERROR;

    // TODO: update focus distance

    // send evt notify that foucs is done
    ret = sendEvtNotify(CAMERA_MSG_FOCUS,
                        (status == CAM_STATUS_SUCCESS)? true : false,
                        0);

    return ret;
}

int32_t QCamera2HardwareInterface::processZoomEvent(uint32_t /*status*/)
{
    int32_t ret = NO_ERROR;

    for (int i = 0; i < QCAMERA_CH_TYPE_MAX; i++) {
        if (m_channels[i] != NULL) {
            ret = m_channels[i]->processZoomDone(mSmoothZoomRunning? NULL : mPreviewWindow);
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
    do {
        pthread_cond_wait(&m_cond, &m_lock);
    } while (m_apiResult.request_api != api_evt);
}

void QCamera2HardwareInterface::unlockAPI()
{
    pthread_mutex_unlock(&m_lock);
}

void QCamera2HardwareInterface::signalAPIResult(qcamera_api_result_t *result)
{
    pthread_mutex_lock(&m_lock);
    memcpy(&m_apiResult, result, sizeof(qcamera_api_result_t));
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
                                 nodisplay_preview_stream_cb_routine, this);
    } else {
        rc = pChannel->addStream(*this,
                                 CAM_STREAM_TYPE_PREVIEW,
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
    attr.look_back = getZSLBackLookCount();
    attr.post_frame_skip = getZSLBurstInterval();
    attr.water_mark = getZSLQueueDepth();
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
                                 nodisplay_preview_stream_cb_routine, this);
    } else {
        rc = pChannel->addStream(*this, CAM_STREAM_TYPE_PREVIEW,
                                 preview_stream_cb_routine, this);
    }
    if (rc != NO_ERROR) {
        ALOGE("%s: add preview stream failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    rc = pChannel->addStream(*this, CAM_STREAM_TYPE_SNAPSHOT,
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
                             postview_stream_cb_routine, this);

    if (rc != NO_ERROR) {
        ALOGE("%s: add postview stream failed, ret = %d", __func__, rc);
        delete pChannel;
        return rc;
    }

    rc = pChannel->addStream(*this, CAM_STREAM_TYPE_SNAPSHOT,
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
    if (isZSLMode()) {
        rc = addChannel(QCAMERA_CH_TYPE_ZSL);
    } else {
        rc = addChannel(QCAMERA_CH_TYPE_PREVIEW);
        if (rc != NO_ERROR) {
            return rc;
        }

        if(mRecordingHint == true) {
            rc = addChannel(QCAMERA_CH_TYPE_VIDEO);
            if (rc != NO_ERROR) {
                delChannel(QCAMERA_CH_TYPE_PREVIEW);
                return rc;
            }

            rc = addChannel(QCAMERA_CH_TYPE_SNAPSHOT);
            if (rc != NO_ERROR) {
                delChannel(QCAMERA_CH_TYPE_PREVIEW);
                delChannel(QCAMERA_CH_TYPE_VIDEO);
            }
        }
    }

    return rc;
}

void QCamera2HardwareInterface::unpreparePreview()
{
    if (isZSLMode()) {
        delChannel(QCAMERA_CH_TYPE_ZSL);
    } else {
        delChannel(QCAMERA_CH_TYPE_PREVIEW);
        if(mRecordingHint == true) {
            delChannel(QCAMERA_CH_TYPE_VIDEO);
            delChannel(QCAMERA_CH_TYPE_SNAPSHOT);
        }
    }
}

void QCamera2HardwareInterface::playShutter(){
     ALOGV("%s : E", __func__);
     if (mNotifyCb == NULL ||
         msgTypeEnabled(CAMERA_MSG_SHUTTER) == 0){
         ALOGV("%s: shutter msg not enabled or NULL cb", __func__);
         return;
     }

     if(!mShutterSoundPlayed){
         mNotifyCb(CAMERA_MSG_SHUTTER, 0, true, mCallbackCookie);
     }
     mNotifyCb(CAMERA_MSG_SHUTTER, 0, false, mCallbackCookie);
     mShutterSoundPlayed = false;
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

int QCamera2HardwareInterface::getZSLBurstInterval()
{
    // TODO
    int val = 1;
    return val;
}


int QCamera2HardwareInterface::getZSLQueueDepth()
{
    //TODO
    int val = 2;
    return val;
}

int QCamera2HardwareInterface::getZSLBackLookCount()
{
    int look_back = 2;
    return look_back;
}

bool QCamera2HardwareInterface::isZSLMode()
{
    //TODO
    return false;
}

int QCamera2HardwareInterface::setRecordingHintValue(bool /*value*/)
{
    // TODO
    return 0;
}

uint8_t QCamera2HardwareInterface::getNumOfSnapshots()
{
    // TODO
    return 1;
}

int QCamera2HardwareInterface::setParameters(const QCameraParameters& /*params*/)
{
    // TODO
    return 0;
}

bool QCamera2HardwareInterface::isNoDisplayMode()
{
    return mNoDisplayMode;
}

bool QCamera2HardwareInterface::isWNREnabled()
{
    // TODO
    return (mDenoiseValue != 0);
}

bool QCamera2HardwareInterface::needDebugFps()
{
    return mDebugFps;
}

bool QCamera2HardwareInterface::needOfflineReprocess()
{
    if (isZSLMode() && isWNREnabled()) {
        return true;
    } else {
        return false;
    }
}

void QCamera2HardwareInterface::getPictureSize(int *picture_width,
                                               int *picture_height)
{
    if (NULL == picture_width || NULL == picture_height) {
        ALOGE("%s: invalid parameters", __func__);
        return;
    }
    // TODO: from local viarable?
    mParameters.getPictureSize(picture_width, picture_height);
}

void QCamera2HardwareInterface::getThumbnailSize(int *width,
                                                int *height)
{
    if (NULL == width || NULL == height) {
        ALOGE("%s: invalid parameters", __func__);
        return;
    }
    // TODO: from local viarable?
}

int QCamera2HardwareInterface::getJpegQuality()
{
    // TODO
    return 85;
}

int QCamera2HardwareInterface::getJpegRotation() {
    // TODO
    return 0;
}

int QCamera2HardwareInterface::getExifData(QEXIF_INFO_DATA **exif_data,
                                           int *num_exif_entries)
{
    if (NULL == exif_data || NULL == num_exif_entries) {
        ALOGE("%s: invalid parameters", __func__);
        return BAD_VALUE;
    }
    // TODO
    return NO_ERROR;
}

}; // namespace android
