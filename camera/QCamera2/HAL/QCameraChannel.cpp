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

#define LOG_TAG "QCameraChannel"

#include <utils/Log.h>
#include <utils/Errors.h>
#include "QCameraParameters.h"
#include "QCameraChannel.h"

using namespace android;

namespace qcamera {

/*===========================================================================
 * FUNCTION   : QCameraChannel
 *
 * DESCRIPTION: constrcutor of QCameraChannel
 *
 * PARAMETERS :
 *   @cam_handle : camera handle
 *   @cam_ops    : ptr to camera ops table
 *
 * RETURN     : none
 *==========================================================================*/
QCameraChannel::QCameraChannel(uint32_t cam_handle,
                               mm_camera_ops_t *cam_ops)
{
    m_camHandle = cam_handle;
    m_camOps = cam_ops;

    m_handle = 0;
    m_numStreams = 0;
    memset(mStreams, 0, sizeof(mStreams));
}

/*===========================================================================
 * FUNCTION   : QCameraChannel
 *
 * DESCRIPTION: default constrcutor of QCameraChannel
 *
 * PARAMETERS : none
 *
 * RETURN     : none
 *==========================================================================*/
QCameraChannel::QCameraChannel()
{
    m_camHandle = 0;
    m_camOps = NULL;

    m_handle = 0;
    m_numStreams = 0;
    memset(mStreams, 0, sizeof(mStreams));
}

/*===========================================================================
 * FUNCTION   : ~QCameraChannel
 *
 * DESCRIPTION: destructor of QCameraChannel
 *
 * PARAMETERS : none
 *
 * RETURN     : none
 *==========================================================================*/
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

/*===========================================================================
 * FUNCTION   : init
 *
 * DESCRIPTION: initialization of channel
 *
 * PARAMETERS :
 *   @attr    : channel bundle attribute setting
 *   @dataCB  : data notify callback
 *   @userData: user data ptr
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
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

/*===========================================================================
 * FUNCTION   : addStream
 *
 * DESCRIPTION: add a stream into channel
 *
 * PARAMETERS :
 *   @allocator    : stream related buffer allocator
 *   @stream_type  : type of stream
 *   @paddingInfo  : padding information
 *   @stream_cb    : stream data notify callback
 *   @userdata     : user data ptr
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
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

/*===========================================================================
 * FUNCTION   : start
 *
 * DESCRIPTION: start channel, which will start all streams belong to this channel
 *
 * PARAMETERS :
 *   @param   : parameter obj, needed to pass channel bundle information to backend.
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
int32_t QCameraChannel::start(QCameraParameters &param)
{
    int32_t rc = NO_ERROR;

    if (m_numStreams > 1) {
        // there is more than one stream in the channel
        // we need to notify mctl that all streams in this channel need to be bundled
        cam_bundle_config_t bundleInfo;
        memset(&bundleInfo, 0, sizeof(bundleInfo));
        rc = m_camOps->get_bundle_info(m_camHandle, m_handle, &bundleInfo);
        if (rc != NO_ERROR) {
            ALOGE("%s: get_bundle_info failed", __func__);
            return rc;
        }
        if (bundleInfo.num_of_streams > 1) {
            rc = param.setBundleInfo(bundleInfo);
            if (rc != NO_ERROR) {
                return rc;
            }
        }
    }

    for (int i = 0; i < m_numStreams; i++) {
        if (mStreams[i] != NULL) {
            mStreams[i]->start();
        }
    }
    rc = m_camOps->start_channel(m_camHandle, m_handle);

    if (rc != NO_ERROR) {
        for (int i = 0; i < m_numStreams; i++) {
            if (mStreams[i] != NULL) {
                mStreams[i]->stop();
            }
        }
    }

    return rc;
}

/*===========================================================================
 * FUNCTION   : stop
 *
 * DESCRIPTION: stop a channel, which will stop all streams belong to this channel
 *
 * PARAMETERS : none
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
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

/*===========================================================================
 * FUNCTION   : bufDone
 *
 * DESCRIPTION: return a stream buf back to kernel
 *
 * PARAMETERS :
 *   @recvd_frame  : stream buf frame to be returned
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
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

/*===========================================================================
 * FUNCTION   : processZoomDone
 *
 * DESCRIPTION: process zoom done event
 *
 * PARAMETERS :
 *   @previewWindoe : ptr to preview window ops table, needed to set preview
 *                    crop information
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
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

/*===========================================================================
 * FUNCTION   : getStreamByHandle
 *
 * DESCRIPTION: return stream object by stream handle
 *
 * PARAMETERS :
 *   @streamHandle : stream handle
 *
 * RETURN     : stream object. NULL if not found
 *==========================================================================*/
QCameraStream *QCameraChannel::getStreamByHandle(uint32_t streamHandle)
{
    for (int i = 0; i < m_numStreams; i++) {
        if (mStreams[i] != NULL && mStreams[i]->getMyHandle() == streamHandle) {
            return mStreams[i];
        }
    }
    return NULL;
}

/*===========================================================================
 * FUNCTION   : QCameraPicChannel
 *
 * DESCRIPTION: constructor of QCameraPicChannel
 *
 * PARAMETERS :
 *   @cam_handle : camera handle
 *   @cam_ops    : ptr to camera ops table
 *
 * RETURN     : none
 *==========================================================================*/
QCameraPicChannel::QCameraPicChannel(uint32_t cam_handle,
                                     mm_camera_ops_t *cam_ops) :
    QCameraChannel(cam_handle, cam_ops)
{
}

/*===========================================================================
 * FUNCTION   : QCameraPicChannel
 *
 * DESCRIPTION: default constructor of QCameraPicChannel
 *
 * PARAMETERS : none
 *
 * RETURN     : none
 *==========================================================================*/
QCameraPicChannel::QCameraPicChannel()
{
}

/*===========================================================================
 * FUNCTION   : ~QCameraPicChannel
 *
 * DESCRIPTION: destructor of QCameraPicChannel
 *
 * PARAMETERS : none
 *
 * RETURN     : none
 *==========================================================================*/
QCameraPicChannel::~QCameraPicChannel()
{
}

/*===========================================================================
 * FUNCTION   : takePicture
 *
 * DESCRIPTION: send request for queued snapshot frames
 *
 * PARAMETERS :
 *   @num_of_snapshot : number of snapshot frames requested
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
int32_t QCameraPicChannel::takePicture(uint8_t num_of_snapshot)
{
    int32_t rc = m_camOps->request_super_buf(m_camHandle,
                                             m_handle,
                                             num_of_snapshot);
    return rc;
}

/*===========================================================================
 * FUNCTION   : cancelPicture
 *
 * DESCRIPTION: cancel request for queued snapshot frames
 *
 * PARAMETERS : none
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
int32_t QCameraPicChannel::cancelPicture()
{
    int32_t rc = m_camOps->cancel_super_buf_request(m_camHandle, m_handle);
    return rc;
}

/*===========================================================================
 * FUNCTION   : QCameraVideoChannel
 *
 * DESCRIPTION: constructor of QCameraVideoChannel
 *
 * PARAMETERS :
 *   @cam_handle : camera handle
 *   @cam_ops    : ptr to camera ops table
 *
 * RETURN     : none
 *==========================================================================*/
QCameraVideoChannel::QCameraVideoChannel(uint32_t cam_handle,
                                         mm_camera_ops_t *cam_ops) :
    QCameraChannel(cam_handle, cam_ops)
{
}

/*===========================================================================
 * FUNCTION   : QCameraVideoChannel
 *
 * DESCRIPTION: default constructor of QCameraVideoChannel
 *
 * PARAMETERS : none
 *
 * RETURN     : none
 *==========================================================================*/
QCameraVideoChannel::QCameraVideoChannel()
{
}

/*===========================================================================
 * FUNCTION   : ~QCameraVideoChannel
 *
 * DESCRIPTION: destructor of QCameraVideoChannel
 *
 * PARAMETERS : none
 *
 * RETURN     : none
 *==========================================================================*/
QCameraVideoChannel::~QCameraVideoChannel()
{
}

/*===========================================================================
 * FUNCTION   : releaseFrame
 *
 * DESCRIPTION: return video frame from app
 *
 * PARAMETERS :
 *   @opaque     : ptr to video frame to be returned
 *   @isMetaData : if frame is a metadata or real frame
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
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

/*===========================================================================
 * FUNCTION   : QCameraReprocessChannel
 *
 * DESCRIPTION: constructor of QCameraReprocessChannel
 *
 * PARAMETERS :
 *   @cam_handle : camera handle
 *   @cam_ops    : ptr to camera ops table
 *
 * RETURN     : none
 *==========================================================================*/
QCameraReprocessChannel::QCameraReprocessChannel(uint32_t cam_handle,
                                                 mm_camera_ops_t *cam_ops) :
    QCameraChannel(cam_handle, cam_ops)
{
}

/*===========================================================================
 * FUNCTION   : QCameraReprocessChannel
 *
 * DESCRIPTION: default constructor of QCameraReprocessChannel
 *
 * PARAMETERS : none
 *
 * RETURN     : none
 *==========================================================================*/
QCameraReprocessChannel::QCameraReprocessChannel()
{
}

/*===========================================================================
 * FUNCTION   : ~QCameraReprocessChannel
 *
 * DESCRIPTION: destructor of QCameraReprocessChannel
 *
 * PARAMETERS : none
 *
 * RETURN     : none
 *==========================================================================*/
QCameraReprocessChannel::~QCameraReprocessChannel()
{
}

/*===========================================================================
 * FUNCTION   : doReprocess
 *
 * DESCRIPTION: request to do a reprocess on the frame
 *
 * PARAMETERS :
 *   @frame   : frame to be performed a reprocess
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
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
