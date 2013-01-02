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

#define LOG_TAG "QCameraChannel"

#include <utils/Log.h>
#include <utils/Errors.h>
#include "QCameraChannel.h"

using namespace android;

namespace qcamera {

QCameraChannel::QCameraChannel(uint32_t cam_handle,
                               mm_camera_ops_t *cam_ops)
{
    m_camHandle = cam_handle;
    m_camOps = cam_ops;

    m_handle = 0;
    m_numStreams = 0;
    memset(mStreams, 0, sizeof(mStreams));
}

QCameraChannel::QCameraChannel()
{
    m_camHandle = 0;
    m_camOps = NULL;

    m_handle = 0;
    m_numStreams = 0;
    memset(mStreams, 0, sizeof(mStreams));
}

QCameraChannel::~QCameraChannel()
{
    for (int i = 0; i < m_numStreams; i++) {
        if (mStreams[i] != NULL) {
            delete mStreams[i];
            mStreams[i] = 0;
        }
    }
    m_numStreams = 0;
    m_camOps->delete_channel(m_camHandle, m_handle);
    m_handle = 0;
}

int32_t QCameraChannel::init(mm_camera_channel_attr_t *attr,
                             mm_camera_buf_notify_t dataCB,
                             void *userData)
{
    m_handle = m_camOps->add_channel(m_camHandle,
                                      attr,
                                      dataCB,
                                      userData);
    if (m_handle == 0) {
        ALOGE("%s: Add channel failed", __func__);
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}

int32_t QCameraChannel::addStream(QCameraAllocator &allocator,
                                  cam_stream_type_t stream_type,
                                  cam_padding_info_t *paddingInfo,
                                  stream_cb_routine stream_cb,
                                  void *userdata)
{
    int32_t rc = NO_ERROR;
    if (m_numStreams >= MAX_STREAM_NUM_IN_BUNDLE) {
        ALOGE("%s: stream number (%d) exceeds max limit (%d)",
              __func__, m_numStreams, MAX_STREAM_NUM_IN_BUNDLE);
        return BAD_VALUE;
    }
    QCameraStream *pStream = new QCameraStream(allocator,
                                               m_camHandle,
                                               m_handle,
                                               m_camOps,
                                               paddingInfo);
    if (pStream == NULL) {
        ALOGE("%s: No mem for Stream", __func__);
        return NO_MEMORY;
    }

    rc = pStream->init(stream_type, stream_cb, userdata);
    if (rc == 0) {
        mStreams[m_numStreams] = pStream;
        m_numStreams++;
    } else {
        delete pStream;
    }
    return rc;
}

int32_t QCameraChannel::start()
{
    int32_t rc = NO_ERROR;
    for (int i = 0; i < m_numStreams; i++) {
        if (mStreams[i] != NULL) {
            rc = mStreams[i]->start();
        }
    }

    rc = m_camOps->start_channel(m_camHandle, m_handle);

    return rc;
}

int32_t QCameraChannel::stop()
{
    int32_t rc = NO_ERROR;
    rc = m_camOps->stop_channel(m_camHandle, m_handle);

    for (int i = 0; i < m_numStreams; i++) {
        if (mStreams[i] != NULL) {
            mStreams[i]->stop();
        }
    }

    return rc;
}

int32_t QCameraChannel::bufDone(mm_camera_super_buf_t *recvd_frame)
{
    int32_t rc = NO_ERROR;
    for (int i = 0; i < recvd_frame->num_bufs; i++) {
         if (recvd_frame->bufs[i] != NULL) {
             for (int j = 0; j < m_numStreams; j++) {
                 if (mStreams[j] != NULL &&
                     mStreams[j]->getMyHandle() == recvd_frame->bufs[i]->stream_id) {
                     rc = mStreams[j]->bufDone(recvd_frame->bufs[j]->buf_idx);
                     break; // break loop j
                 }
             }
         }
    }

    return rc;
}

int32_t QCameraChannel::processZoomDone(preview_stream_ops_t *previewWindow)
{
    int32_t rc = NO_ERROR;
    for (int i = 0; i < m_numStreams; i++) {
        if (mStreams[i] != NULL) {
            rc = mStreams[i]->processZoomDone(previewWindow);
        }
    }
    return rc;
}

QCameraStream *QCameraChannel::getStreamByHandle(uint32_t streamHandle)
{
    for (int i = 0; i < m_numStreams; i++) {
        if (mStreams[i] != NULL && mStreams[i]->getMyHandle() == streamHandle) {
            return mStreams[i];
        }
    }
    return NULL;
}

QCameraPicChannel::QCameraPicChannel(uint32_t cam_handle,
                                     mm_camera_ops_t *cam_ops) :
    QCameraChannel(cam_handle, cam_ops)
{
}

QCameraPicChannel::QCameraPicChannel()
{
}

QCameraPicChannel::~QCameraPicChannel()
{
}

int32_t QCameraPicChannel::takePicture(uint8_t num_of_snapshot)
{
    int32_t rc = m_camOps->request_super_buf(m_camHandle,
                                             m_handle,
                                             num_of_snapshot);
    return rc;
}

int32_t QCameraPicChannel::cancelPicture()
{
    int32_t rc = m_camOps->cancel_super_buf_request(m_camHandle, m_handle);
    return rc;
}

QCameraVideoChannel::QCameraVideoChannel(uint32_t cam_handle,
                                         mm_camera_ops_t *cam_ops) :
    QCameraChannel(cam_handle, cam_ops)
{
}

QCameraVideoChannel::QCameraVideoChannel()
{
}

QCameraVideoChannel::~QCameraVideoChannel()
{
}

int32_t QCameraVideoChannel::releaseFrame(const void * opaque, bool isMetaData)
{
    QCameraStream *pVideoStream = NULL;
    for (int i = 0; i < m_numStreams; i++) {
        if (mStreams[i] != NULL && mStreams[i]->isTypeOf(CAM_STREAM_TYPE_VIDEO)) {
            pVideoStream = mStreams[i];
            break;
        }
    }

    if (NULL == pVideoStream) {
        ALOGE("%s: No video stream in the channel", __func__);
        return BAD_VALUE;
    }

    int32_t rc = pVideoStream->bufDone(opaque, isMetaData);
    return rc;
}

QCameraReprocessChannel::QCameraReprocessChannel(uint32_t cam_handle,
                                                 mm_camera_ops_t *cam_ops) :
    QCameraChannel(cam_handle, cam_ops)
{
}

QCameraReprocessChannel::QCameraReprocessChannel()
{
}

QCameraReprocessChannel::~QCameraReprocessChannel()
{
}

int32_t QCameraReprocessChannel::doReprocess(mm_camera_super_buf_t *frame)
{
    int32_t rc = 0;
    if (m_numStreams < 1) {
        ALOGE("%s: No reprocess stream is created", __func__);
        return -1;
    }

    rc = m_camOps->map_stream_buf(m_camHandle,
                                  m_handle,
                                  mStreams[0]->getMyHandle(),
                                  CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF,
                                  0,
                                  -1,
                                  frame->bufs[0]->fd,
                                  frame->bufs[0]->frame_len);
    return rc;
}

}; // namespace qcamera
