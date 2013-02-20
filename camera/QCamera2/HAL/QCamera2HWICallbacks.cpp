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

#include <fcntl.h>
#include <utils/Log.h>
#include <utils/Errors.h>
#include <utils/Timers.h>
#include "QCamera2HWI.h"

namespace qcamera {

/*===========================================================================
 * FUNCTION   : zsl_channel_cb
 *
 * DESCRIPTION: helper function to handle ZSL superbuf callback directly from
 *              mm-camera-interface
 *
 * PARAMETERS :
 *   @recvd_frame : received super buffer
 *   @userdata    : user data ptr
 *
 * RETURN    : None
 *
 * NOTE      : recvd_frame will be released after this call by caller, so if
 *             async operation needed for recvd_frame, it's our responsibility
 *             to save a copy for this variable to be used later.
 *==========================================================================*/
void QCamera2HardwareInterface::zsl_channel_cb(mm_camera_super_buf_t *recvd_frame,
                                               void *userdata)
{
    ALOGV("%s: E",__func__);
    QCamera2HardwareInterface *pme = (QCamera2HardwareInterface *)userdata;
    if (pme == NULL ||
        pme->mCameraHandle == NULL ||
        pme->mCameraHandle->camera_handle != recvd_frame->camera_handle){
       ALOGE("%s: camera obj not valid", __func__);
       return;
    }

    QCameraChannel *pChannel = pme->m_channels[QCAMERA_CH_TYPE_ZSL];
    if (pChannel == NULL ||
        pChannel->getMyHandle() != recvd_frame->ch_id) {
        ALOGE("%s: ZSL channel doesn't exist, return here", __func__);
        return;
    }

    // save a copy for the superbuf
    mm_camera_super_buf_t* frame =
               (mm_camera_super_buf_t *)malloc(sizeof(mm_camera_super_buf_t));
    if (frame == NULL) {
        ALOGE("%s: Error allocating memory to save received_frame structure.", __func__);
        pChannel->bufDone(recvd_frame);
        return;
    }
    *frame = *recvd_frame;

    // send to postprocessor
    pme->m_postprocessor.processData(frame);

    ALOGV("%s: X", __func__);
}

/*===========================================================================
 * FUNCTION   : capture_channel_cb_routine
 *
 * DESCRIPTION: helper function to handle snapshot superbuf callback directly from
 *              mm-camera-interface
 *
 * PARAMETERS :
 *   @recvd_frame : received super buffer
 *   @userdata    : user data ptr
 *
 * RETURN    : None
 *
 * NOTE      : recvd_frame will be released after this call by caller, so if
 *             async operation needed for recvd_frame, it's our responsibility
 *             to save a copy for this variable to be used later.
*==========================================================================*/
void QCamera2HardwareInterface::capture_channel_cb_routine(mm_camera_super_buf_t *recvd_frame,
                                                           void *userdata)
{
    ALOGD("%s: E", __func__);
    QCamera2HardwareInterface *pme = (QCamera2HardwareInterface *)userdata;
    if (pme == NULL ||
        pme->mCameraHandle == NULL ||
        pme->mCameraHandle->camera_handle != recvd_frame->camera_handle){
        ALOGE("%s: camera obj not valid", __func__);
        // simply free super frame
        free(recvd_frame);
        return;
    }

    QCameraChannel *pChannel = pme->m_channels[QCAMERA_CH_TYPE_CAPTURE];
    if (pChannel == NULL ||
        pChannel->getMyHandle() != recvd_frame->ch_id) {
        ALOGE("%s: Capture channel doesn't exist, return here", __func__);
        return;
    }

    // save a copy for the superbuf
    mm_camera_super_buf_t* frame =
               (mm_camera_super_buf_t *)malloc(sizeof(mm_camera_super_buf_t));
    if (frame == NULL) {
        ALOGE("%s: Error allocating memory to save received_frame structure.", __func__);
        pChannel->bufDone(recvd_frame);
        return;
    }
    *frame = *recvd_frame;

    // send to postprocessor
    pme->m_postprocessor.processData(frame);

    ALOGD("%s: X", __func__);
}

/*===========================================================================
 * FUNCTION   : preview_stream_cb_routine
 *
 * DESCRIPTION: helper function to handle preview frame from preview stream in
 *              normal case with display.
 *
 * PARAMETERS :
 *   @super_frame : received super buffer
 *   @stream      : stream object
 *   @userdata    : user data ptr
 *
 * RETURN    : None
 *
 * NOTE      : caller passes the ownership of super_frame, it's our
 *             responsibility to free super_frame once it's done. The new
 *             preview frame will be sent to display, and an older frame
 *             will be dequeued from display and needs to be returned back
 *             to kernel for future use.
 *==========================================================================*/
void QCamera2HardwareInterface::preview_stream_cb_routine(mm_camera_super_buf_t *super_frame,
                                                          QCameraStream * stream,
                                                          void *userdata)
{
    ALOGD("%s : BEGIN", __func__);
    int err = NO_ERROR;
    QCamera2HardwareInterface *pme = (QCamera2HardwareInterface *)userdata;
    QCameraGrallocMemory *memory = (QCameraGrallocMemory *)super_frame->bufs[0]->mem_info;

    if (pme == NULL) {
        ALOGE("%s: Invalid hardware object", __func__);
        free(super_frame);
        return;
    }
    if (memory == NULL) {
        ALOGE("%s: Invalid memory object", __func__);
        free(super_frame);
        return;
    }

    mm_camera_buf_def_t *frame = super_frame->bufs[0];
    if (NULL == frame) {
        ALOGE("%s: preview frame is NLUL", __func__);
        free(super_frame);
        return;
    }

    if (!pme->needProcessPreviewFrame()) {
        ALOGE("%s: preview is not running, no need to process", __func__);
        stream->bufDone(frame->buf_idx);
        free(super_frame);
        return;
    }

    if (pme->needDebugFps()) {
        pme->debugShowPreviewFPS();
    }

    int idx = frame->buf_idx;
    memory->cleanCache(idx);
    pme->dumpFrameToFile(frame->buffer, frame->frame_len,
                         frame->buf_idx, QCAMERA_DUMP_FRM_PREVIEW);

    // Display the buffer.
    int dequeuedIdx = memory->displayBuffer(idx);
    if (dequeuedIdx < 0 || dequeuedIdx >= memory->getCnt()) {
        ALOGD("%s: Invalid dequeued buffer index %d from display",
              __func__, dequeuedIdx);
    } else {
        // Return dequeued buffer back to driver
        err = stream->bufDone(dequeuedIdx);
        if ( err < 0) {
            ALOGE("stream bufDone failed %d", err);
        }
    }

    // Handle preview data callback
    if (pme->mDataCb != NULL && pme->msgTypeEnabled(CAMERA_MSG_PREVIEW_FRAME) > 0) {
        camera_memory_t *previewMem = NULL;
        camera_memory_t *data = NULL;
        int previewBufSize;
        cam_dimension_t preview_dim;
        cam_format_t previewFmt;
        stream->getFrameDimension(preview_dim);
        stream->getFormat(previewFmt);

        /* The preview buffer size in the callback should be (width*height*bytes_per_pixel)
         * As all preview formats we support, use 12 bits per pixel, buffer size = previewWidth * previewHeight * 3/2.
         * We need to put a check if some other formats are supported in future. */
        if ((previewFmt == CAM_FORMAT_YUV_420_NV21) ||
            (previewFmt == CAM_FORMAT_YUV_420_NV12) ||
            (previewFmt == CAM_FORMAT_YUV_420_YV12)) {
            if(previewFmt == CAM_FORMAT_YUV_420_YV12) {
                previewBufSize = ((preview_dim.width+15)/16) * 16 * preview_dim.height +
                                 ((preview_dim.width/2+15)/16) * 16* preview_dim.height;
                } else {
                    previewBufSize = preview_dim.width * preview_dim.height * 3/2;
                }
            if(previewBufSize != memory->getSize(idx)) {
                previewMem = pme->mGetMemory(memory->getFd(idx),
                           previewBufSize, 1, pme->mCallbackCookie);
                if (!previewMem || !previewMem->data) {
                    ALOGE("%s: mGetMemory failed.\n", __func__);
                } else {
                    data = previewMem;
                }
            } else
                data = memory->getMemory(idx, false);
        } else {
            data = memory->getMemory(idx, false);
            ALOGE("%s: Invalid preview format, buffer size in preview callback may be wrong.", __func__);
        }
        pme->mDataCb(CAMERA_MSG_PREVIEW_FRAME, data, 0, NULL, pme->mCallbackCookie);
        if (previewMem) {
            previewMem->release(previewMem);
        }
        ALOGD("end of cb");
    }

    free(super_frame);
    ALOGV("%s : END", __func__);
    return;
}

/*===========================================================================
 * FUNCTION   : nodisplay_preview_stream_cb_routine
 *
 * DESCRIPTION: helper function to handle preview frame from preview stream in
 *              no-display case
 *
 * PARAMETERS :
 *   @super_frame : received super buffer
 *   @stream      : stream object
 *   @userdata    : user data ptr
 *
 * RETURN    : None
 *
 * NOTE      : caller passes the ownership of super_frame, it's our
 *             responsibility to free super_frame once it's done.
 *==========================================================================*/
void QCamera2HardwareInterface::nodisplay_preview_stream_cb_routine(
                                                          mm_camera_super_buf_t *super_frame,
                                                          QCameraStream *stream,
                                                          void * userdata)
{
    ALOGV("%s",__func__);
    QCamera2HardwareInterface *pme = (QCamera2HardwareInterface *)userdata;
    if (pme == NULL ||
        pme->mCameraHandle == NULL ||
        pme->mCameraHandle->camera_handle != super_frame->camera_handle){
        ALOGE("%s: camera obj not valid", __func__);
        // simply free super frame
        free(super_frame);
        return;
    }
    mm_camera_buf_def_t *frame = super_frame->bufs[0];
    if (NULL == frame) {
        ALOGE("%s: preview frame is NLUL", __func__);
        free(super_frame);
        return;
    }

    if (!pme->needProcessPreviewFrame()) {
        ALOGD("%s: preview is not running, no need to process", __func__);
        stream->bufDone(frame->buf_idx);
        free(super_frame);
        return;
    }

    if (pme->needDebugFps()) {
        pme->debugShowPreviewFPS();
    }

    QCameraMemory *previewMemObj = (QCameraMemory *)frame->mem_info;
    camera_memory_t *preview_mem =
        previewMemObj->getMemory(frame->buf_idx, false);
    if (NULL != previewMemObj && NULL != preview_mem) {
        previewMemObj->cleanCache(frame->buf_idx);

        pme->dumpFrameToFile(frame->buffer, frame->frame_len,
                             frame->buf_idx, QCAMERA_DUMP_FRM_PREVIEW);

        if (pme->needProcessPreviewFrame() &&
            pme->mDataCb != NULL &&
            pme->msgTypeEnabled(CAMERA_MSG_PREVIEW_FRAME) > 0 ) {
            //Sending preview callback if corresponding Msgs are enabled
            pme->mDataCb(CAMERA_MSG_PREVIEW_FRAME, preview_mem,
                         0, NULL, pme->mCallbackCookie);
        }

        stream->bufDone(frame->buf_idx);
    }

    free(super_frame);
}

/*===========================================================================
 * FUNCTION   : postview_stream_cb_routine
 *
 * DESCRIPTION: helper function to handle post frame from postview stream
 *
 * PARAMETERS :
 *   @super_frame : received super buffer
 *   @stream      : stream object
 *   @userdata    : user data ptr
 *
 * RETURN    : None
 *
 * NOTE      : caller passes the ownership of super_frame, it's our
 *             responsibility to free super_frame once it's done.
 *==========================================================================*/
void QCamera2HardwareInterface::postview_stream_cb_routine(mm_camera_super_buf_t *super_frame,
                                                           QCameraStream *stream,
                                                           void *userdata)
{
    int err = NO_ERROR;
    QCamera2HardwareInterface *pme = (QCamera2HardwareInterface *)userdata;
    QCameraGrallocMemory *memory = (QCameraGrallocMemory *)super_frame->bufs[0]->mem_info;

    if (pme == NULL) {
        ALOGE("%s: Invalid hardware object", __func__);
        free(super_frame);
        return;
    }
    if (memory == NULL) {
        ALOGE("%s: Invalid memory object", __func__);
        free(super_frame);
        return;
    }

    ALOGD("%s : BEGIN", __func__);

    mm_camera_buf_def_t *frame = super_frame->bufs[0];
    if (NULL == frame) {
        ALOGE("%s: preview frame is NLUL", __func__);
        free(super_frame);
        return;
    }

    QCameraMemory *memObj = (QCameraMemory *)frame->mem_info;
    if (NULL != memObj) {
        memObj->cleanCache(frame->buf_idx);

        pme->dumpFrameToFile(frame->buffer, frame->frame_len,
                             frame->buf_idx, QCAMERA_DUMP_FRM_THUMBNAIL);
    }

    // Display the buffer.
    int dequeuedIdx = memory->displayBuffer(frame->buf_idx);
    if (dequeuedIdx < 0 || dequeuedIdx >= memory->getCnt()) {
        ALOGD("%s: Invalid dequeued buffer index %d",
              __func__, dequeuedIdx);
        free(super_frame);
        return;
    }

    // Return dequeued buffer back to driver
    err = stream->bufDone(dequeuedIdx);
    if ( err < 0) {
        ALOGE("stream bufDone failed %d", err);
    }

    free(super_frame);
    ALOGD("%s : END", __func__);
    return;
}

/*===========================================================================
 * FUNCTION   : video_stream_cb_routine
 *
 * DESCRIPTION: helper function to handle video frame from video stream
 *
 * PARAMETERS :
 *   @super_frame : received super buffer
 *   @stream      : stream object
 *   @userdata    : user data ptr
 *
 * RETURN    : None
 *
 * NOTE      : caller passes the ownership of super_frame, it's our
 *             responsibility to free super_frame once it's done. video
 *             frame will be sent to video encoder. Once video encoder is
 *             done with the video frame, it will call another API
 *             (release_recording_frame) to return the frame back
 *==========================================================================*/
void QCamera2HardwareInterface::video_stream_cb_routine(mm_camera_super_buf_t *super_frame,
                                                        QCameraStream */*stream*/,
                                                        void *userdata)
{
    ALOGV("%s : BEGIN", __func__);
    QCamera2HardwareInterface *pme = (QCamera2HardwareInterface *)userdata;
    if (pme == NULL ||
        pme->mCameraHandle == NULL ||
        pme->mCameraHandle->camera_handle != super_frame->camera_handle){
        ALOGE("%s: camera obj not valid", __func__);
        // simply free super frame
        free(super_frame);
        return;
    }
    mm_camera_buf_def_t *frame = super_frame->bufs[0];

    if (pme->needDebugFps()) {
        pme->debugShowVideoFPS();
    }

    ALOGE("%s: Stream(%d), Timestamp: %ld %ld",
          __func__,
          frame->stream_id,
          frame->ts.tv_sec,
          frame->ts.tv_nsec);

    nsecs_t timeStamp = nsecs_t(frame->ts.tv_sec) * 1000000000LL + frame->ts.tv_nsec;
    ALOGE("Send Video frame to services/encoder TimeStamp : %lld", timeStamp);
    QCameraMemory *videoMemObj = (QCameraMemory *)frame->mem_info;
    camera_memory_t *video_mem =
        videoMemObj->getMemory(frame->buf_idx, (pme->mStoreMetaDataInFrame > 0)? true : false);
    if (NULL != videoMemObj && NULL != video_mem) {
        videoMemObj->cleanCache(frame->buf_idx);
        pme->dumpFrameToFile(frame->buffer, frame->frame_len,
                             frame->buf_idx, QCAMERA_DUMP_FRM_VIDEO);
        if ((pme->mDataCbTimestamp != NULL) &&
            pme->msgTypeEnabled(CAMERA_MSG_VIDEO_FRAME) > 0) {
            pme->mDataCbTimestamp(timeStamp,
                                  CAMERA_MSG_VIDEO_FRAME,
                                  video_mem,
                                  0,
                                  pme->mCallbackCookie);
        }
    }
    free(super_frame);
    ALOGV("%s : END", __func__);
}

/*===========================================================================
 * FUNCTION   : snapshot_stream_cb_routine
 *
 * DESCRIPTION: helper function to handle snapshot frame from snapshot stream
 *
 * PARAMETERS :
 *   @super_frame : received super buffer
 *   @stream      : stream object
 *   @userdata    : user data ptr
 *
 * RETURN    : None
 *
 * NOTE      : caller passes the ownership of super_frame, it's our
 *             responsibility to free super_frame once it's done. For
 *             snapshot, it need to send to postprocessor for jpeg
 *             encoding, therefore the ownership of super_frame will be
 *             hand to postprocessor.
 *==========================================================================*/
void QCamera2HardwareInterface::snapshot_stream_cb_routine(mm_camera_super_buf_t *super_frame,
                                                           QCameraStream * /*stream*/,
                                                           void *userdata)
{
    ALOGD("%s: E", __func__);
    QCamera2HardwareInterface *pme = (QCamera2HardwareInterface *)userdata;
    if (pme == NULL ||
        pme->mCameraHandle == NULL ||
        pme->mCameraHandle->camera_handle != super_frame->camera_handle){
        ALOGE("%s: camera obj not valid", __func__);
        // simply free super frame
        free(super_frame);
        return;
    }

    pme->m_postprocessor.processData(super_frame);

    ALOGD("%s: X", __func__);
}

/*===========================================================================
 * FUNCTION   : raw_stream_cb_routine
 *
 * DESCRIPTION: helper function to handle raw dump frame from raw stream
 *
 * PARAMETERS :
 *   @super_frame : received super buffer
 *   @stream      : stream object
 *   @userdata    : user data ptr
 *
 * RETURN    : None
 *
 * NOTE      : caller passes the ownership of super_frame, it's our
 *             responsibility to free super_frame once it's done. For raw
 *             frame, there is no need to send to postprocessor for jpeg
 *             encoding. this function will play shutter and send the data
 *             callback to upper layer. Raw frame buffer will be returned
 *             back to kernel, and frame will be free after use.
 *==========================================================================*/
void QCamera2HardwareInterface::raw_stream_cb_routine(mm_camera_super_buf_t * super_frame,
                                                      QCameraStream * /*stream*/,
                                                      void * userdata)
{
    ALOGV("%s : BEGIN", __func__);
    QCamera2HardwareInterface *pme = (QCamera2HardwareInterface *)userdata;
    if (pme == NULL ||
        pme->mCameraHandle == NULL ||
        pme->mCameraHandle->camera_handle != super_frame->camera_handle){
        ALOGE("%s: camera obj not valid", __func__);
        // simply free super frame
        free(super_frame);
        return;
    }

    pme->m_postprocessor.processRawData(super_frame);
    ALOGV("%s : END", __func__);
}

/*===========================================================================
 * FUNCTION   : metadata_stream_cb_routine
 *
 * DESCRIPTION: helper function to handle metadata frame from metadata stream
 *
 * PARAMETERS :
 *   @super_frame : received super buffer
 *   @stream      : stream object
 *   @userdata    : user data ptr
 *
 * RETURN    : None
 *
 * NOTE      : caller passes the ownership of super_frame, it's our
 *             responsibility to free super_frame once it's done. Metadata
 *             could have valid entries for face detection result or
 *             histogram statistics information.
 *==========================================================================*/
void QCamera2HardwareInterface::metadata_stream_cb_routine(mm_camera_super_buf_t * super_frame,
                                                           QCameraStream * stream,
                                                           void * userdata)
{
    ALOGD("%s : BEGIN", __func__);
    QCamera2HardwareInterface *pme = (QCamera2HardwareInterface *)userdata;
    if (pme == NULL ||
        pme->mCameraHandle == NULL ||
        pme->mCameraHandle->camera_handle != super_frame->camera_handle){
        ALOGE("%s: camera obj not valid", __func__);
        // simply free super frame
        free(super_frame);
        return;
    }

    mm_camera_buf_def_t *frame = super_frame->bufs[0];
    QCameraMemory *metaMemObj = (QCameraMemory *)frame->mem_info;
    camera_memory_t *meta_mem = metaMemObj->getMemory(frame->buf_idx, false);
    cam_metadata_info_t *pMetaData = (cam_metadata_info_t *)meta_mem->data;

    if (pMetaData->is_faces_valid) {
        // process face detection result
        pme->processFaceDetectionResult(&pMetaData->faces_data);
    }

    if (pMetaData->is_stats_valid) {
        // process histogram statistics info
        pme->processHistogramStats(pMetaData->stats_data);
    }

    if (pMetaData->is_focus_valid) {
        // process focus info
        qcamera_sm_internal_evt_payload_t *payload =
            (qcamera_sm_internal_evt_payload_t *)malloc(sizeof(qcamera_sm_internal_evt_payload_t));
        if (NULL != payload) {
            memset(payload, 0, sizeof(qcamera_sm_internal_evt_payload_t));
            payload->evt_type = QCAMERA_INTERNAL_EVT_FOCUS_UPDATE;
            payload->focus_data = pMetaData->focus_data;
            int32_t rc = pme->processEvt(QCAMERA_SM_EVT_EVT_INTERNAL, payload);
            if (rc != NO_ERROR) {
                ALOGE("%s: processEVt failed", __func__);
                free(payload);
                payload = NULL;

            }
        } else {
            ALOGE("%s: No memory for qcamera_sm_internal_evt_payload_t", __func__);
        }
    }

    stream->bufDone(frame->buf_idx);
    free(super_frame);

    ALOGD("%s : END", __func__);
}

/*===========================================================================
 * FUNCTION   : reprocess_stream_cb_routine
 *
 * DESCRIPTION: helper function to handle reprocess frame from reprocess stream
                (after reprocess, e.g., ZSL snapshot frame after WNR if
 *              WNR is enabled)
 *
 * PARAMETERS :
 *   @super_frame : received super buffer
 *   @stream      : stream object
 *   @userdata    : user data ptr
 *
 * RETURN    : None
 *
 * NOTE      : caller passes the ownership of super_frame, it's our
 *             responsibility to free super_frame once it's done. In this
 *             case, reprocessed frame need to be passed to postprocessor
 *             for jpeg encoding.
 *==========================================================================*/
void QCamera2HardwareInterface::reprocess_stream_cb_routine(mm_camera_super_buf_t * super_frame,
                                                            QCameraStream * /*stream*/,
                                                            void * userdata)
{
    ALOGV("%s: E", __func__);
    QCamera2HardwareInterface *pme = (QCamera2HardwareInterface *)userdata;
    if (pme == NULL ||
        pme->mCameraHandle == NULL ||
        pme->mCameraHandle->camera_handle != super_frame->camera_handle){
        ALOGE("%s: camera obj not valid", __func__);
        // simply free super frame
        free(super_frame);
        return;
    }

    pme->m_postprocessor.processPPData(super_frame);

    ALOGV("%s: X", __func__);
}

/*===========================================================================
 * FUNCTION   : dumpFrameToFile
 *
 * DESCRIPTION: helper function to dump frame into file for debug purpose.
 *
 * PARAMETERS :
 *    @data : data ptr
 *    @size : length of data buffer
 *    @index : identifier for data
 *    @dump_type : type of the frame to be dumped. Only such
 *                 dump type is enabled, the frame will be
 *                 dumped into a file.
 *
 * RETURN     : None
 *==========================================================================*/
void QCamera2HardwareInterface::dumpFrameToFile(const void *data,
                                                uint32_t size,
                                                int index,
                                                int dump_type)
{
    if ((mParameters.getEnabledFileDumpMask() & dump_type) == 0) {
        ALOGV("dumping frame to file not enabled");
        return;
    }

    char buf[32];
    switch (dump_type) {
    case QCAMERA_DUMP_FRM_PREVIEW:
        snprintf(buf, sizeof(buf), "/data/%s_%d.%s", "preview", index, "yuv");
        break;
    case QCAMERA_DUMP_FRM_THUMBNAIL:
        snprintf(buf, sizeof(buf), "/data/%s_%d.%s", "thumbnail", index, "yuv");
        break;
    case QCAMERA_DUMP_FRM_SNAPSHOT:
        snprintf(buf, sizeof(buf), "/data/%s_%d.%s", "main", index, "yuv");
        break;
    case QCAMERA_DUMP_FRM_VIDEO:
        snprintf(buf, sizeof(buf), "/data/%s_%d.%s", "video", index, "yuv");
        break;
    case QCAMERA_DUMP_FRM_RAW:
        snprintf(buf, sizeof(buf), "/data/%s_%d.%s", "raw", index, "raw");
        break;
    case QCAMERA_DUMP_FRM_JPEG:
        snprintf(buf, sizeof(buf), "/data/%s_%d.%s", "jpeg", index, "jpg");
        break;
    default:
        ALOGE("%s: Not supported for dumping stream type %d", __func__, dump_type);
        return;
    }

    ALOGD("dump %s size =%d, data = %p", buf, size, data);
    int file_fd = open(buf, O_RDWR | O_CREAT, 0777);
    int written_len = write(file_fd, data, size);
    ALOGD("%s: written number of bytes %d\n", __func__, written_len);
    close(file_fd);
}

/*===========================================================================
 * FUNCTION   : debugShowVideoFPS
 *
 * DESCRIPTION: helper function to log video frame FPS for debug purpose.
 *
 * PARAMETERS : None
 *
 * RETURN     : None
 *==========================================================================*/
void QCamera2HardwareInterface::debugShowVideoFPS()
{
    static int n_vFrameCount = 0;
    static int n_vLastFrameCount = 0;
    static nsecs_t n_vLastFpsTime = 0;
    static float n_vFps = 0;
    n_vFrameCount++;
    nsecs_t now = systemTime();
    nsecs_t diff = now - n_vLastFpsTime;
    if (diff > ms2ns(250)) {
        n_vFps =  ((n_vFrameCount - n_vLastFrameCount) * float(s2ns(1))) / diff;
        ALOGE("Video Frames Per Second: %.4f", n_vFps);
        n_vLastFpsTime = now;
        n_vLastFrameCount = n_vFrameCount;
    }
}

/*===========================================================================
 * FUNCTION   : debugShowPreviewFPS
 *
 * DESCRIPTION: helper function to log preview frame FPS for debug purpose.
 *
 * PARAMETERS : None
 *
 * RETURN     : None
 *==========================================================================*/
void QCamera2HardwareInterface::debugShowPreviewFPS()
{
    static int n_pFrameCount = 0;
    static int n_pLastFrameCount = 0;
    static nsecs_t n_pLastFpsTime = 0;
    static float n_pFps = 0;
    n_pFrameCount++;
    nsecs_t now = systemTime();
    nsecs_t diff = now - n_pLastFpsTime;
    if (diff > ms2ns(250)) {
        n_pFps =  ((n_pFrameCount - n_pLastFrameCount) * float(s2ns(1))) / diff;
        ALOGE("Preview Frames Per Second: %.4f", n_pFps);
        n_pLastFpsTime = now;
        n_pLastFrameCount = n_pFrameCount;
    }
}

}; // namespace qcamera
