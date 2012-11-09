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

#include <utils/Log.h>
#include <utils/Errors.h>
#include <hardware/camera.h>

#include "QCamera2HWI.h"

namespace android {

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
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    return hw->setPreviewWindow(window);
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
    hw->setCallBacks(notify_cb, data_cb, data_cb_timestamp, get_memory, user);
}

void QCamera2HardwareInterface::enable_msg_type(struct camera_device *device, int32_t msg_type)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return;
    }
    hw->enableMsgType(msg_type);
}

void QCamera2HardwareInterface::disable_msg_type(struct camera_device *device, int32_t msg_type)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return;
    }
    hw->disableMsgType(msg_type);
}

int QCamera2HardwareInterface::msg_type_enabled(struct camera_device *device, int32_t msg_type)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
   return hw->msgTypeEnabled(msg_type);
}

int QCamera2HardwareInterface::start_preview(struct camera_device *device)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    return hw->startPreview();
}

void QCamera2HardwareInterface::stop_preview(struct camera_device *device)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return;
    }
    hw->stopPreview();
}

int QCamera2HardwareInterface::preview_enabled(struct camera_device *device)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }

    return hw->previewEnabled();
}

int QCamera2HardwareInterface::store_meta_data_in_buffers(
                struct camera_device *device, int enable)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    return hw->storeMetaDataInBuffers(enable);
}

int QCamera2HardwareInterface::start_recording(struct camera_device *device)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    return hw->startRecording();
}

void QCamera2HardwareInterface::stop_recording(struct camera_device *device)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return;
    }
    hw->stopRecording();
}

int QCamera2HardwareInterface::recording_enabled(struct camera_device *device)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    return hw->recordingEnabled();
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
    hw->releaseRecordingFrame(opaque);
}

int QCamera2HardwareInterface::auto_focus(struct camera_device *device)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    return hw->autoFocus();
}

int QCamera2HardwareInterface::cancel_auto_focus(struct camera_device *device)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    return hw->cancelAutoFocus();
}

int QCamera2HardwareInterface::take_picture(struct camera_device *device)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    return hw->takePicture();
}

int QCamera2HardwareInterface::cancel_picture(struct camera_device *device)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    return hw->cancelPicture();
}

int QCamera2HardwareInterface::set_parameters(struct camera_device *device,
                                                        const char *parms)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    return hw->setParameters(parms);
}

char* QCamera2HardwareInterface::get_parameters(struct camera_device *device)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return NULL;
    }
    return hw->getParameters();
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
    hw->putParameters(parm);
}

int QCamera2HardwareInterface::send_command(struct camera_device *device,
                                int32_t cmd, int32_t arg1, int32_t arg2)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    return hw->sendCommand(cmd, arg1, arg2);
}

void QCamera2HardwareInterface::release(struct camera_device *device)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return;
    }
    hw->release();
}

int QCamera2HardwareInterface::dump(struct camera_device *device, int fd)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(device->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    return hw->dump(fd);
}

int QCamera2HardwareInterface::close(hw_device_t *hw_dev)
{
    QCamera2HardwareInterface *hw =
        reinterpret_cast<QCamera2HardwareInterface *>(
            reinterpret_cast<camera_device_t *>(hw_dev)->priv);
    if (!hw) {
        ALOGE("NULL camera device");
        return BAD_VALUE;
    }
    return hw->closeCamera();
}

QCamera2HardwareInterface::QCamera2HardwareInterface(int cameraId)
{
    mCameraId = cameraId;
    mCameraDevice.common.tag = HARDWARE_DEVICE_TAG;
    mCameraDevice.common.version = HARDWARE_DEVICE_API_VERSION(1, 0);
    mCameraDevice.common.close = close;
    mCameraDevice.ops = &mCameraOps;
    mCameraDevice.priv = this;
    mCameraHandle = NULL;
}

QCamera2HardwareInterface::~QCamera2HardwareInterface()
{
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
    return OK;
}

int QCamera2HardwareInterface::closeCamera()
{
    int rc = OK;

    rc = mCameraHandle->ops->close_camera(mCameraHandle->camera_handle);
    mCameraHandle = NULL;

    return rc;
}

int QCamera2HardwareInterface::getCapabilities(struct camera_info *info)
{
    if (mCameraId == 0) {
        info->facing = CAMERA_FACING_BACK;
        info->orientation = 0;
    } else {
        info->facing = CAMERA_FACING_FRONT;
        info->orientation = 0;
    }
    return OK;
}

int QCamera2HardwareInterface::setPreviewWindow(
        struct preview_stream_ops * /*window*/)
{
    return OK;
}

void QCamera2HardwareInterface::setCallBacks(
    camera_notify_callback /*notify_cb */,
    camera_data_callback /*data_cb*/,
    camera_data_timestamp_callback /*data_cb_timestamp*/,
    camera_request_memory /*get_memory*/,
    void * /*user*/)
{
}

void QCamera2HardwareInterface::enableMsgType(int32_t /*msg_type*/)
{
}

void QCamera2HardwareInterface::disableMsgType(int32_t /*msg_type*/)
{
}

int QCamera2HardwareInterface::msgTypeEnabled(int32_t /*msg_type*/)
{
    return OK;
}

int QCamera2HardwareInterface::startPreview()
{
    return OK;
}

void QCamera2HardwareInterface::stopPreview()
{
}

int QCamera2HardwareInterface::previewEnabled()
{
    return OK;
}

int QCamera2HardwareInterface::storeMetaDataInBuffers(int /*enable*/)
{
    return OK;
}

int QCamera2HardwareInterface::startRecording()
{
    return OK;
}

void QCamera2HardwareInterface::stopRecording()
{
}

int QCamera2HardwareInterface::recordingEnabled()
{
    return OK;
}

void QCamera2HardwareInterface::releaseRecordingFrame(const void * /*opaque*/)
{
}

int QCamera2HardwareInterface::autoFocus()
{
    return OK;
}

int QCamera2HardwareInterface::cancelAutoFocus()
{
    return OK;
}

int QCamera2HardwareInterface::takePicture()
{
    return OK;
}

int QCamera2HardwareInterface::cancelPicture()
{
    return OK;
}

int QCamera2HardwareInterface::setParameters(const char * /*parms*/)
{
    return OK;
}

char* QCamera2HardwareInterface::getParameters()
{
   return NULL;
}

void QCamera2HardwareInterface::putParameters(char *)
{
}

int QCamera2HardwareInterface::sendCommand(int32_t /*cmd*/, int32_t /*arg1*/, int32_t /*arg2*/)
{
    return OK;
}

void QCamera2HardwareInterface::release()
{
}

int QCamera2HardwareInterface::dump(int /*fd*/)
{
    return OK;
}

}; // namespace android
