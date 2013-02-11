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

#define LOG_TAG "QCameraPostProc"

#include <stdlib.h>
#include <utils/Log.h>
#include <utils/Errors.h>

#include "QCamera2HWI.h"
#include "QCameraPostProc.h"

namespace qcamera {

/*===========================================================================
 * FUNCTION   : QCameraPostProcessor
 *
 * DESCRIPTION: constructor of QCameraPostProcessor.
 *
 * PARAMETERS :
 *   @cam_ctrl : ptr to HWI object
 *
 * RETURN     : None
 *==========================================================================*/
QCameraPostProcessor::QCameraPostProcessor(QCamera2HardwareInterface *cam_ctrl)
    : m_parent(cam_ctrl),
      mJpegCB(NULL),
      mJpegUserData(NULL),
      mJpegClientHandle(0),
      mJpegSessionId(0),
      m_pJpegOutputMem(NULL),
      m_pJpegExifObj(NULL),
      m_bThumbnailNeeded(TRUE),
      m_inputPPQ(releasePPInputData, this),
      m_ongoingPPQ(releaseOngoingPPData, this),
      m_inputJpegQ(releaseJpegInputData, this),
      m_ongoingJpegQ(releaseOngoingJpegData, this),
      m_inputRawQ(releaseJpegInputData, this),
      m_dataNotifyQ(releaseOutputData, this)
{
    memset(&mJpegHandle, 0, sizeof(mJpegHandle));
}

/*===========================================================================
 * FUNCTION   : ~QCameraPostProcessor
 *
 * DESCRIPTION: deconstructor of QCameraPostProcessor.
 *
 * PARAMETERS : None
 *
 * RETURN     : None
 *==========================================================================*/
QCameraPostProcessor::~QCameraPostProcessor()
{
    if (m_pJpegOutputMem != NULL) {
        m_pJpegOutputMem->deallocate();
        delete m_pJpegOutputMem;
        m_pJpegOutputMem = NULL;
    }
    if (m_pJpegExifObj != NULL) {
        delete m_pJpegExifObj;
        m_pJpegExifObj = NULL;
    }
}

/*===========================================================================
 * FUNCTION   : init
 *
 * DESCRIPTION: initialization of postprocessor
 *
 * PARAMETERS :
 *   @jpeg_cb      : callback to handle jpeg event from mm-camera-interface
 *   @user_data    : user data ptr for jpeg callback
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
int32_t QCameraPostProcessor::init(jpeg_encode_callback_t jpeg_cb, void *user_data)
{
    mJpegCB = jpeg_cb;
    mJpegUserData = user_data;

    mJpegClientHandle = jpeg_open(&mJpegHandle);
    if(!mJpegClientHandle) {
        ALOGE("%s : jpeg_open did not work", __func__);
        return UNKNOWN_ERROR;
    }

    m_dataProcTh.launch(dataProcessRoutine, this);
    m_dataNotifyTh.launch(dataNotifyRoutine, this);

    return NO_ERROR;
}

/*===========================================================================
 * FUNCTION   : deinit
 *
 * DESCRIPTION: de-initialization of postprocessor
 *
 * PARAMETERS : None
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
int32_t QCameraPostProcessor::deinit()
{
    m_dataProcTh.exit();
    m_dataNotifyTh.exit();

    if(mJpegClientHandle > 0) {
        int rc = mJpegHandle.close(mJpegClientHandle);
        ALOGE("%s: Jpeg closed, rc = %d, mJpegClientHandle = %x",
              __func__, rc, mJpegClientHandle);
        mJpegClientHandle = 0;
        memset(&mJpegHandle, 0, sizeof(mJpegHandle));
    }

    return NO_ERROR;
}

/*===========================================================================
 * FUNCTION   : start
 *
 * DESCRIPTION: start postprocessor. Data process thread and data notify thread
 *              will be launched.
 *
 * PARAMETERS : None
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *
 * NOTE       : if any offline reprocess is needed, a reprocess channel/stream
 *              will be started.
 *==========================================================================*/
int32_t QCameraPostProcessor::start()
{
    int32_t rc = NO_ERROR;
    if (m_parent->needOfflineReprocess()) {
        // if offline reprocess is needed, start reprocess channel
        rc = m_parent->addChannel(QCAMERA_CH_TYPE_REPROCESS);
        if (rc != 0) {
            ALOGE("%s: cannot add reprocess channel", __func__);
            return rc;
        }

        rc = m_parent->startChannel(QCAMERA_CH_TYPE_REPROCESS);
        if (rc != 0) {
            ALOGE("%s: cannot start reprocess channel", __func__);
            m_parent->delChannel(QCAMERA_CH_TYPE_REPROCESS);
            return rc;
        }
    }

    m_dataProcTh.sendCmd(CAMERA_CMD_TYPE_START_DATA_PROC, FALSE, FALSE);
    m_dataNotifyTh.sendCmd(CAMERA_CMD_TYPE_START_DATA_PROC, FALSE, FALSE);

    return rc;
}

/*===========================================================================
 * FUNCTION   : stop
 *
 * DESCRIPTION: stop postprocessor. Data process and notify thread will be stopped.
 *
 * PARAMETERS : None
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *
 * NOTE       : reprocess channel will be stopped and deleted if there is any
 *==========================================================================*/
int32_t QCameraPostProcessor::stop()
{
    m_dataNotifyTh.sendCmd(CAMERA_CMD_TYPE_STOP_DATA_PROC, FALSE, TRUE);
    // dataProc Thread need to process "stop" as sync call because abort jpeg job should be a sync call
    m_dataProcTh.sendCmd(CAMERA_CMD_TYPE_STOP_DATA_PROC, TRUE, TRUE);

    if (m_parent->needOfflineReprocess()) {
        m_parent->stopChannel(QCAMERA_CH_TYPE_REPROCESS);
        m_parent->delChannel(QCAMERA_CH_TYPE_REPROCESS);
    }

    return NO_ERROR;
}

/*===========================================================================
 * FUNCTION   : getJpegEncodingConfig
 *
 * DESCRIPTION: function to prepare encoding job information
 *
 * PARAMETERS :
 *   @encode_parm   : param to be filled with encoding configuration
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
int32_t QCameraPostProcessor::getJpegEncodingConfig(mm_jpeg_encode_params_t& encode_parm,
                                                    QCameraStream *main_stream,
                                                    QCameraStream *thumb_stream)
{
    ALOGV("%s : E", __func__);
    int32_t ret = NO_ERROR;
    camera_memory_t *jpeg_mem = NULL;

    encode_parm.jpeg_cb = mJpegCB;
    encode_parm.userdata = mJpegUserData;

    m_bThumbnailNeeded = TRUE; // need encode thumbnail by default
    cam_dimension_t thumbnailSize;
    memset(&thumbnailSize, 0, sizeof(cam_dimension_t));
    m_parent->getThumbnailSize(thumbnailSize);
    if (thumbnailSize.width == 0 && thumbnailSize.height == 0) {
        // (0,0) means no thumbnail
        m_bThumbnailNeeded = FALSE;
    }
    encode_parm.encode_thumbnail = m_bThumbnailNeeded;

    // get color format
    cam_format_t img_fmt = CAM_FORMAT_YUV_420_NV12;
    main_stream->getFormat(img_fmt);
    encode_parm.color_format = getColorfmtFromImgFmt(img_fmt);

    // get jpeg quality
    encode_parm.quality = m_parent->getJpegQuality();
    if (encode_parm.quality <= 0) {
        encode_parm.quality = 85;
    }

    // get exif data
    if (m_pJpegExifObj != NULL) {
        delete m_pJpegExifObj;
        m_pJpegExifObj = NULL;
    }
    m_pJpegExifObj = m_parent->getExifData();
    if (m_pJpegExifObj != NULL) {
        encode_parm.exif_info.exif_data = m_pJpegExifObj->getEntries();
        encode_parm.exif_info.numOfEntries = m_pJpegExifObj->getNumOfEntries();
    }

    cam_frame_len_offset_t offset;
    memset(&offset, 0, sizeof(cam_frame_len_offset_t));
    main_stream->getFrameOffset(offset);

    // src buf config
    QCameraMemory *pStreamMem = main_stream->getStreamBufs();
    if (pStreamMem == NULL) {
        ALOGE("%s: cannot get stream bufs from main stream", __func__);
        ret = BAD_VALUE;
        goto on_error;
    }
    encode_parm.num_src_bufs = pStreamMem->getCnt();
    for (uint32_t i = 0; i < encode_parm.num_src_bufs; i++) {
        camera_memory_t *stream_mem = pStreamMem->getMemory(i, false);
        if (stream_mem != NULL) {
            encode_parm.src_main_buf[i].index = i;
            encode_parm.src_main_buf[i].buf_size = stream_mem->size;
            encode_parm.src_main_buf[i].buf_vaddr = (uint8_t *)stream_mem->data;
            encode_parm.src_main_buf[i].fd = pStreamMem->getFd(i);
            encode_parm.src_main_buf[i].format = MM_JPEG_FMT_YUV;
            encode_parm.src_main_buf[i].offset = offset;
        }
    }

    if (m_bThumbnailNeeded == TRUE) {
        if (thumb_stream == NULL) {
            thumb_stream = main_stream;
        }
        pStreamMem = thumb_stream->getStreamBufs();
        if (pStreamMem == NULL) {
            ALOGE("%s: cannot get stream bufs from thumb stream", __func__);
            ret = BAD_VALUE;
            goto on_error;
        }
        memset(&offset, 0, sizeof(cam_frame_len_offset_t));
        thumb_stream->getFrameOffset(offset);
        for (int i = 0; i < pStreamMem->getCnt(); i++) {
            camera_memory_t *stream_mem = pStreamMem->getMemory(i, false);
            if (stream_mem != NULL) {
                encode_parm.src_thumb_buf[i].index = i;
                encode_parm.src_thumb_buf[i].buf_size = stream_mem->size;
                encode_parm.src_thumb_buf[i].buf_vaddr = (uint8_t *)stream_mem->data;
                encode_parm.src_thumb_buf[i].fd = pStreamMem->getFd(i);
                encode_parm.src_thumb_buf[i].format = MM_JPEG_FMT_YUV;
                encode_parm.src_thumb_buf[i].offset = offset;
            }
        }
    }

    // allocate output buf for jpeg encoding
    if (m_pJpegOutputMem != NULL) {
        m_pJpegOutputMem->deallocate();
        delete m_pJpegOutputMem;
        m_pJpegOutputMem = NULL;
    }
    m_pJpegOutputMem = new QCameraStreamMemory(m_parent->mGetMemory);
    if (NULL == m_pJpegOutputMem) {
        ret = NO_MEMORY;
        ALOGE("%s : No memory for m_pJpegOutputMem", __func__);
        goto on_error;
    }
    ret = m_pJpegOutputMem->allocate(1, offset.frame_len);
    if(ret != OK) {
        ret = NO_MEMORY;
        ALOGE("%s : No memory for m_pJpegOutputMem", __func__);
        goto on_error;
    }
    jpeg_mem = m_pJpegOutputMem->getMemory(0, false);
    if (NULL == jpeg_mem) {
        ret = NO_MEMORY;
        ALOGE("%s : initHeapMem for jpeg, ret = NO_MEMORY", __func__);
        goto on_error;
    }
    encode_parm.num_dst_bufs = 1;
    encode_parm.dest_buf[0].index = 0;
    encode_parm.dest_buf[0].buf_size = jpeg_mem->size;
    encode_parm.dest_buf[0].buf_vaddr = (uint8_t *)jpeg_mem->data;
    encode_parm.dest_buf[0].fd = m_pJpegOutputMem->getFd(0);
    encode_parm.dest_buf[0].format = MM_JPEG_FMT_YUV;
    encode_parm.dest_buf[0].offset = offset;

    ALOGV("%s : X", __func__);
    return NO_ERROR;

on_error:
    if (m_pJpegOutputMem != NULL) {
        m_pJpegOutputMem->deallocate();
        delete m_pJpegOutputMem;
        m_pJpegOutputMem = NULL;
    }
    if (m_pJpegExifObj != NULL) {
        delete m_pJpegExifObj;
        m_pJpegExifObj = NULL;
    }
    ALOGV("%s : X with error %d", __func__, ret);
    return ret;
}

/*===========================================================================
 * FUNCTION   : sendEvtNotify
 *
 * DESCRIPTION: send event notify through notify callback registered by upper layer
 *
 * PARAMETERS :
 *   @msg_type: msg type of notify
 *   @ext1    : extension
 *   @ext2    : extension
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
int32_t QCameraPostProcessor::sendEvtNotify(int32_t msg_type,
                                            int32_t ext1,
                                            int32_t ext2)
{
    return m_parent->sendEvtNotify(msg_type, ext1, ext2);
}

/*===========================================================================
 * FUNCTION   : sendDataNotify
 *
 * DESCRIPTION: enqueue data into dataNotify thread
 *
 * PARAMETERS :
 *   @msg_type: data callback msg type
 *   @data    : ptr to data memory struct
 *   @index   : index to data buffer
 *   @metadata: ptr to meta data buffer if there is any
 *   @jpeg_mem: any tempory heap memory to be released after callback
 *   @release_data : ptr to struct indicating if data need to be released
 *                   after notify
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
int32_t QCameraPostProcessor::sendDataNotify(int32_t msg_type,
                                             camera_memory_t *data,
                                             uint8_t index,
                                             camera_frame_metadata_t *metadata,
                                             qcamera_release_data_t *release_data)
{
    qcamera_data_argm_t *data_cb = (qcamera_data_argm_t *)malloc(sizeof(qcamera_data_argm_t));
    if (NULL == data_cb) {
        ALOGE("%s: no mem for acamera_data_argm_t", __func__);
        return NO_MEMORY;
    }
    memset(data_cb, 0, sizeof(qcamera_data_argm_t));
    data_cb->msg_type = msg_type;
    data_cb->data = data;
    data_cb->index = index;
    data_cb->metadata = metadata;
    if (release_data != NULL) {
        data_cb->release_data = *release_data;
    }

    // enqueue jpeg_data into jpeg data queue
    if (m_dataNotifyQ.enqueue((void *)data_cb)) {
        m_dataNotifyTh.sendCmd(CAMERA_CMD_TYPE_DO_NEXT_JOB, FALSE, FALSE);
    } else {
        ALOGE("%s: Error enqueuing jpeg data into notify queue", __func__);
        free(data_cb);
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}

/*===========================================================================
 * FUNCTION   : processData
 *
 * DESCRIPTION: enqueue data into dataProc thread
 *
 * PARAMETERS :
 *   @frame   : process frame received from mm-camera-interface
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *
 * NOTE       : depends on if offline reprocess is needed, received frame will
 *              be sent to either input queue of postprocess or jpeg encoding
 *==========================================================================*/
int32_t QCameraPostProcessor::processData(mm_camera_super_buf_t *frame)
{
    if (m_parent->needOfflineReprocess()) {
        ALOGD("%s: need offline reprocess", __func__);
        // enqueu to post proc input queue
        m_inputPPQ.enqueue((void *)frame);
    } else {
        ALOGD("%s: no need offline reprocess, sending to jpeg encoding", __func__);
        // enqueu to jpeg input queue
        m_inputJpegQ.enqueue((void *)frame);
    }
    m_dataProcTh.sendCmd(CAMERA_CMD_TYPE_DO_NEXT_JOB, FALSE, FALSE);

    return NO_ERROR;
}

/*===========================================================================
 * FUNCTION   : processRawData
 *
 * DESCRIPTION: enqueue raw data into dataProc thread
 *
 * PARAMETERS :
 *   @frame   : process frame received from mm-camera-interface
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
int32_t QCameraPostProcessor::processRawData(mm_camera_super_buf_t *frame)
{
    // enqueu to raw input queue
    m_inputRawQ.enqueue((void *)frame);
    m_dataProcTh.sendCmd(CAMERA_CMD_TYPE_DO_NEXT_JOB, FALSE, FALSE);
    return NO_ERROR;
}

/*===========================================================================
 * FUNCTION   : processJpegEvt
 *
 * DESCRIPTION: process jpeg event from mm-jpeg-interface.
 *
 * PARAMETERS :
 *   @evt     : payload of jpeg event, including information about jpeg encoding
 *              status, jpeg size and so on.
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *
 * NOTE       : This event will also trigger DataProc thread to move to next job
 *              processing (i.e., send a new jpeg encoding job to mm-jpeg-interface
 *              if there is any pending job in jpeg input queue)
 *==========================================================================*/
int32_t QCameraPostProcessor::processJpegEvt(qcamera_jpeg_evt_payload_t *evt)
{
    int32_t rc = NO_ERROR;
    camera_memory_t *jpeg_mem = NULL;

    // find job by jobId
    qcamera_jpeg_data_t *job = findJpegJobByJobId(evt->jobId);

    if (job == NULL) {
        ALOGE("%s: Cannot find jpeg job by jobId(%d)", __func__, evt->jobId);
        rc = BAD_VALUE;
        goto end;
    }

    if (m_parent->mDataCb == NULL ||
        m_parent->msgTypeEnabled(CAMERA_MSG_COMPRESSED_IMAGE) == 0 ) {
        ALOGD("%s: No dataCB or CAMERA_MSG_COMPRESSED_IMAGE not enabled",
              __func__);
        rc = NO_ERROR;
        goto end;
    }

    if(evt->status == JPEG_JOB_STATUS_ERROR) {
        ALOGE("%s: Error event handled from jpeg, status = %d",
              __func__, evt->status);
        rc = FAILED_TRANSACTION;
        goto end;
    }

    m_parent->dumpFrameToFile(evt->out_data.buf_vaddr,
                              evt->out_data.buf_filled_len,
                              evt->jobId,
                              QCAMERA_DUMP_FRM_JPEG);
    ALOGD("%s: Dump jpeg_size=%d", __func__, evt->out_data.buf_filled_len);

    // alloc jpeg memory to pass to upper layer
    jpeg_mem = m_parent->mGetMemory(-1, evt->out_data.buf_filled_len, 1, m_parent->mCallbackCookie);
    if (NULL == jpeg_mem) {
        rc = NO_MEMORY;
        ALOGE("%s : getMemory for jpeg, ret = NO_MEMORY", __func__);
        goto end;
    }
    memcpy(jpeg_mem->data, evt->out_data.buf_vaddr, evt->out_data.buf_filled_len);

    ALOGE("%s : Calling upperlayer callback to store JPEG image", __func__);
    qcamera_release_data_t release_data;
    memset(&release_data, 0, sizeof(qcamera_release_data_t));
    release_data.data = jpeg_mem;
    rc = sendDataNotify(CAMERA_MSG_COMPRESSED_IMAGE,
                        jpeg_mem,
                        0,
                        NULL,
                        &release_data);

end:
    if (rc != NO_ERROR) {
        // send error msg to upper layer
        sendDataNotify(CAMERA_MSG_COMPRESSED_IMAGE,
                       NULL,
                       0,
                       NULL,
                       NULL);

        if (NULL != jpeg_mem) {
            jpeg_mem->release(jpeg_mem);
            jpeg_mem = NULL;
        }
    }

    // release internal data for jpeg job
    if (job != NULL) {
        releaseJpegJobData(job);
        free(job);
    }

    // wait up data proc thread to do next job,
    // if previous request is blocked due to ongoing jpeg job
    m_dataProcTh.sendCmd(CAMERA_CMD_TYPE_DO_NEXT_JOB, FALSE, FALSE);

    return rc;
}

/*===========================================================================
 * FUNCTION   : processPPData
 *
 * DESCRIPTION: process received frame after reprocess.
 *
 * PARAMETERS :
 *   @frame   : received frame from reprocess channel.
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *
 * NOTE       : The frame after reprocess need to send to jpeg encoding.
 *==========================================================================*/
int32_t QCameraPostProcessor::processPPData(mm_camera_super_buf_t *frame)
{
    qcamera_pp_data_t *job = (qcamera_pp_data_t *)m_ongoingPPQ.dequeue();

    if (job == NULL) {
        ALOGE("%s: Cannot find reprocess job", __func__);
        return -1;
    }

    // got reprocess result back, no need for source frame
    releaseSuperBuf(job->src_frame);
    free(job->src_frame);
    job->src_frame = NULL;
    free(job);

    // enqueu reprocessed frame to jpeg input queue
    m_inputJpegQ.enqueue((void *)frame);

    // wait up data proc thread
    m_dataProcTh.sendCmd(CAMERA_CMD_TYPE_DO_NEXT_JOB, FALSE, FALSE);

    return 0;
}

/*===========================================================================
 * FUNCTION   : findJpegJobByJobId
 *
 * DESCRIPTION: find a jpeg job from ongoing Jpeg queue by its job ID
 *
 * PARAMETERS :
 *   @jobId   : job Id of the job
 *
 * RETURN     : ptr to a jpeg job struct. NULL if not found.
 *
 * NOTE       : Currently only one job is sending to mm-jpeg-interface for jpeg
 *              encoding. Therefore simply dequeue from the ongoing Jpeg Queue
 *              will serve the purpose to find the jpeg job.
 *==========================================================================*/
qcamera_jpeg_data_t *QCameraPostProcessor::findJpegJobByJobId(uint32_t jobId)
{
    qcamera_jpeg_data_t * job = NULL;
    if (jobId == 0) {
        ALOGE("%s: not a valid jpeg jobId", __func__);
        return NULL;
    }

    // currely only one jpeg job ongoing, so simply dequeue the head
    job = (qcamera_jpeg_data_t *)m_ongoingJpegQ.dequeue();
    return job;
}

/*===========================================================================
 * FUNCTION   : releaseOutputData
 *
 * DESCRIPTION: callback function to release notify data node
 *
 * PARAMETERS :
 *   @data      : ptr to notify data
 *   @user_data : user data ptr (QCameraReprocessor)
 *
 * RETURN     : None
 *==========================================================================*/
void QCameraPostProcessor::releaseOutputData(void *data, void *user_data)
{
    QCameraPostProcessor *pme = (QCameraPostProcessor *)user_data;
    if (NULL != pme) {
        pme->releaseNotifyData((qcamera_data_argm_t *)data);
    }
}

/*===========================================================================
 * FUNCTION   : releaseOutputData
 *
 * DESCRIPTION: callback function to release jpeg input data node
 *
 * PARAMETERS :
 *   @data      : ptr to jpeg input data
 *   @user_data : user data ptr (QCameraReprocessor)
 *
 * RETURN     : None
 *==========================================================================*/
void QCameraPostProcessor::releaseJpegInputData(void *data, void *user_data)
{
    QCameraPostProcessor *pme = (QCameraPostProcessor *)user_data;
    if (NULL != pme) {
        pme->releaseSuperBuf((mm_camera_super_buf_t *)data);
    }
}

/*===========================================================================
 * FUNCTION   : releasePPInputData
 *
 * DESCRIPTION: callback function to release post process input data node
 *
 * PARAMETERS :
 *   @data      : ptr to post process input data
 *   @user_data : user data ptr (QCameraReprocessor)
 *
 * RETURN     : None
 *==========================================================================*/
void QCameraPostProcessor::releasePPInputData(void *data, void *user_data)
{
    QCameraPostProcessor *pme = (QCameraPostProcessor *)user_data;
    qcamera_pp_request_t *request = (qcamera_pp_request_t *)data;
    if (NULL != pme) {
        if (request->frame != NULL) {
            pme->releaseSuperBuf(request->frame);
            free(request->frame);
            request->frame = NULL;
        }
    }
}

/*===========================================================================
 * FUNCTION   : releaseOngoingJpegData
 *
 * DESCRIPTION: callback function to release ongoing jpeg job node
 *
 * PARAMETERS :
 *   @data      : ptr to ongoing jpeg job data
 *   @user_data : user data ptr (QCameraReprocessor)
 *
 * RETURN     : None
 *==========================================================================*/
void QCameraPostProcessor::releaseOngoingJpegData(void *data, void *user_data)
{
    QCameraPostProcessor *pme = (QCameraPostProcessor *)user_data;
    if (NULL != pme) {
        pme->releaseJpegJobData((qcamera_jpeg_data_t *)data);
    }
}

/*===========================================================================
 * FUNCTION   : releaseOngoingPPData
 *
 * DESCRIPTION: callback function to release ongoing postprocess job node
 *
 * PARAMETERS :
 *   @data      : ptr to onging postprocess job
 *   @user_data : user data ptr (QCameraReprocessor)
 *
 * RETURN     : None
 *==========================================================================*/
void QCameraPostProcessor::releaseOngoingPPData(void *data, void *user_data)
{
    QCameraPostProcessor *pme = (QCameraPostProcessor *)user_data;
    if (NULL != pme) {
        qcamera_pp_data_t *pp_job = (qcamera_pp_data_t *)data;
        if (NULL != pp_job->src_frame) {
            pme->releaseSuperBuf(pp_job->src_frame);
            free(pp_job->src_frame);
            pp_job->src_frame = NULL;
        }
    }
}

/*===========================================================================
 * FUNCTION   : releaseNotifyData
 *
 * DESCRIPTION: function to release internal resources in notify data struct
 *
 * PARAMETERS :
 *   @app_cb  : ptr to data notify struct
 *
 * RETURN     : None
 *
 * NOTE       : deallocate jpeg heap memory if it's not NULL
 *==========================================================================*/
void QCameraPostProcessor::releaseNotifyData(qcamera_data_argm_t *app_cb)
{
    if (app_cb && NULL != app_cb->release_data.data) {
        app_cb->release_data.data->release(app_cb->release_data.data);
        app_cb->release_data.data = NULL;
    }
    if (app_cb && NULL != app_cb->release_data.frame) {
        releaseSuperBuf(app_cb->release_data.frame);
        free(app_cb->release_data.frame);
        app_cb->release_data.frame = NULL;
    }
}

/*===========================================================================
 * FUNCTION   : releaseSuperBuf
 *
 * DESCRIPTION: function to release a superbuf frame by returning back to kernel
 *
 * PARAMETERS :
 *   @super_buf : ptr to the superbuf frame
 *
 * RETURN     : None
 *==========================================================================*/
void QCameraPostProcessor::releaseSuperBuf(mm_camera_super_buf_t *super_buf)
{
    if (NULL != super_buf) {
        QCameraChannel *pChannel = m_parent->getChannelByHandle(super_buf->ch_id);
        if (pChannel != NULL) {
            pChannel->bufDone(super_buf);
        }
    }
}

/*===========================================================================
 * FUNCTION   : releaseJpegJobData
 *
 * DESCRIPTION: function to release internal resources in jpeg job struct
 *
 * PARAMETERS :
 *   @job     : ptr to jpeg job struct
 *
 * RETURN     : None
 *
 * NOTE       : original source frame need to be queued back to kernel for
 *              future use. Output buf of jpeg job need to be released since
 *              it's allocated for each job. Exif object need to be deleted.
 *==========================================================================*/
void QCameraPostProcessor::releaseJpegJobData(qcamera_jpeg_data_t *job)
{
    ALOGV("%s: E", __func__);
    if (NULL != job) {
        if (NULL != job->src_frame) {
            releaseSuperBuf(job->src_frame);
            free(job->src_frame);
            job->src_frame = NULL;
        }
    }
    ALOGV("%s: X", __func__);
}

/*===========================================================================
 * FUNCTION   : getColorfmtFromImgFmt
 *
 * DESCRIPTION: function to return jpeg color format based on its image format
 *
 * PARAMETERS :
 *   @img_fmt : image format
 *
 * RETURN     : jpeg color format that can be understandable by omx lib
 *==========================================================================*/
mm_jpeg_color_format QCameraPostProcessor::getColorfmtFromImgFmt(cam_format_t img_fmt)
{
    switch (img_fmt) {
    case CAM_FORMAT_YUV_420_NV21:
        return MM_JPEG_COLOR_FORMAT_YCRCBLP_H2V2;
    case CAM_FORMAT_YUV_420_NV21_ADRENO:
        return MM_JPEG_COLOR_FORMAT_YCRCBLP_H2V2;
    case CAM_FORMAT_YUV_420_NV12:
        return MM_JPEG_COLOR_FORMAT_YCBCRLP_H2V2;
    case CAM_FORMAT_YUV_420_YV12:
        return MM_JPEG_COLOR_FORMAT_YCBCRLP_H2V2;
    case CAM_FORMAT_YUV_422_NV61:
        return MM_JPEG_COLOR_FORMAT_YCRCBLP_H2V1;
    case CAM_FORMAT_YUV_422_NV16:
        return MM_JPEG_COLOR_FORMAT_YCBCRLP_H2V1;
    default:
        return MM_JPEG_COLOR_FORMAT_YCRCBLP_H2V2;
    }
}

/*===========================================================================
 * FUNCTION   : getJpegImgTypeFromImgFmt
 *
 * DESCRIPTION: function to return jpeg encode image type based on its image format
 *
 * PARAMETERS :
 *   @img_fmt : image format
 *
 * RETURN     : return jpeg source image format (YUV or Bitstream)
 *==========================================================================*/
mm_jpeg_format_t QCameraPostProcessor::getJpegImgTypeFromImgFmt(cam_format_t img_fmt)
{
    switch (img_fmt) {
    case CAM_FORMAT_YUV_420_NV21:
    case CAM_FORMAT_YUV_420_NV21_ADRENO:
    case CAM_FORMAT_YUV_420_NV12:
    case CAM_FORMAT_YUV_420_YV12:
    case CAM_FORMAT_YUV_422_NV61:
    case CAM_FORMAT_YUV_422_NV16:
        return MM_JPEG_FMT_YUV;
    default:
        return MM_JPEG_FMT_YUV;
    }
}

/*===========================================================================
 * FUNCTION   : encodeData
 *
 * DESCRIPTION: function to prepare encoding job information and send to
 *              mm-jpeg-interface to do the encoding job
 *
 * PARAMETERS :
 *   @recvd_frame   : frame to be encoded
 *   @jpeg_job_data : ptr to a struct saving job related information
 *   @needNewSess   : flag to indicate if a new jpeg encoding session need
 *                    to be created. After creation, this flag will be toggled
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
int32_t QCameraPostProcessor::encodeData(mm_camera_super_buf_t *recvd_frame,
                                         qcamera_jpeg_data_t *jpeg_job_data,
                                         uint8_t &needNewSess)
{
    ALOGV("%s : E", __func__);
    int32_t ret = NO_ERROR;
    mm_jpeg_job_t jpg_job;
    uint32_t jobId = 0;
    QCameraStream *main_stream = NULL;
    mm_camera_buf_def_t *main_frame = NULL;
    QCameraStream *thumb_stream = NULL;
    mm_camera_buf_def_t *thumb_frame = NULL;

    // find channel
    QCameraChannel *pChannel = m_parent->getChannelByHandle(recvd_frame->ch_id);
    if (pChannel == NULL) {
        ALOGE("%s: No corresponding channel (ch_id = %d) exist, return here",
              __func__, recvd_frame->ch_id);
        return BAD_VALUE;
    }

    // find snapshot frame
    for (int i = 0; i < recvd_frame->num_bufs; i++) {
        QCameraStream *pStream =
            pChannel->getStreamByHandle(recvd_frame->bufs[i]->stream_id);
        if (pStream != NULL) {
            if (pStream->isTypeOf(CAM_STREAM_TYPE_SNAPSHOT) ||
                pStream->isTypeOf(CAM_STREAM_TYPE_OFFLINE_PROC)) {
                main_stream = pStream;
                main_frame = recvd_frame->bufs[i];
            } else if (pStream->isTypeOf(CAM_STREAM_TYPE_PREVIEW) ||
                       pStream->isTypeOf(CAM_STREAM_TYPE_POSTVIEW)) {
                thumb_stream = pStream;
                thumb_frame = recvd_frame->bufs[i];
            }
        }
    }

    if(NULL == main_frame){
       ALOGE("%s : Main frame is NULL", __func__);
       return BAD_VALUE;
    }

    QCameraMemory *memObj = (QCameraMemory *)main_frame->mem_info;
    if (NULL == memObj) {
        ALOGE("%s : Memeory Obj of main frame is NULL", __func__);
        return NO_MEMORY;
    }

    // clean and invalidate cache ops through mem obj of the frame
    memObj->cleanInvalidateCache(main_frame->buf_idx);

    // dump snapshot frame if enabled
    m_parent->dumpFrameToFile(main_frame->buffer, main_frame->frame_len,
                              main_frame->frame_idx, QCAMERA_DUMP_FRM_SNAPSHOT);

    // send upperlayer callback for raw image
    camera_memory_t *mem = memObj->getMemory(main_frame->buf_idx, false);
    if (NULL != m_parent->mDataCb &&
        m_parent->msgTypeEnabled(CAMERA_MSG_RAW_IMAGE) > 0) {
        m_parent->mDataCb(CAMERA_MSG_RAW_IMAGE,
                          mem, 1, NULL,
                          m_parent->mCallbackCookie);
    }
    if (NULL != m_parent->mNotifyCb &&
        m_parent->msgTypeEnabled(CAMERA_MSG_RAW_IMAGE_NOTIFY) > 0) {
        m_parent->mNotifyCb(CAMERA_MSG_RAW_IMAGE_NOTIFY,
                            0, 0, m_parent->mCallbackCookie);
    }

    if (thumb_frame != NULL) {
        QCameraMemory *thumb_memObj = (QCameraMemory *)thumb_frame->mem_info;
        if (NULL != thumb_memObj) {
            // clean and invalidate cache ops through mem obj of the frame
            thumb_memObj->cleanInvalidateCache(thumb_frame->buf_idx);
        }

        // dump thumbnail frame if enabled
        m_parent->dumpFrameToFile(thumb_frame->buffer, thumb_frame->frame_len,
                                  thumb_frame->frame_idx, QCAMERA_DUMP_FRM_THUMBNAIL);
    }

    if (mJpegClientHandle <= 0) {
        ALOGE("%s: Error: bug here, mJpegClientHandle is 0", __func__);
        return UNKNOWN_ERROR;
    }

    if (needNewSess) {
        // create jpeg encoding session
        mm_jpeg_encode_params_t encodeParam;
        memset(&encodeParam, 0, sizeof(mm_jpeg_encode_params_t));
        getJpegEncodingConfig(encodeParam, main_stream, thumb_stream);
        ret = mJpegHandle.create_session(mJpegClientHandle, &encodeParam, &mJpegSessionId);
        if (ret != NO_ERROR) {
            ALOGE("%s: error creating a new jpeg encoding session", __func__);
            return ret;
        }
        needNewSess = FALSE;
    }

    // Fill in new job
    memset(&jpg_job, 0, sizeof(mm_jpeg_job_t));
    jpg_job.job_type = JPEG_JOB_TYPE_ENCODE;
    jpg_job.encode_job.session_id = mJpegSessionId;
    jpg_job.encode_job.src_index = main_frame->buf_idx;
    jpg_job.encode_job.dst_index = 0;

    cam_rect_t crop;
    memset(&crop, 0, sizeof(cam_rect_t));
    main_stream->getCropInfo(crop);

    cam_dimension_t src_dim;
    memset(&src_dim, 0, sizeof(cam_dimension_t));
    main_stream->getFrameDimension(src_dim);

    // main dim
    jpg_job.encode_job.main_dim.src_dim = src_dim;
    jpg_job.encode_job.main_dim.dst_dim = src_dim;
    jpg_job.encode_job.main_dim.crop = crop;

    // thumbnail dim
    if (m_bThumbnailNeeded == TRUE) {
        if (thumb_stream == NULL) {
            // need jpeg thumbnail, but no postview/preview stream exists
            // we use the main stream/frame to encode thumbnail
            thumb_stream = main_stream;
        }
        memset(&crop, 0, sizeof(cam_rect_t));
        thumb_stream->getCropInfo(crop);
        memset(&src_dim, 0, sizeof(cam_dimension_t));
        thumb_stream->getFrameDimension(src_dim);
        jpg_job.encode_job.thumb_dim.src_dim = src_dim;
        m_parent->getThumbnailSize(jpg_job.encode_job.thumb_dim.dst_dim);
        jpg_job.encode_job.thumb_dim.crop = crop;
    }

    // rotation
    jpg_job.encode_job.rotation = m_parent->getJpegRotation();
    ALOGV("%s: jpeg rotation is set to %d", __func__, jpg_job.encode_job.rotation);

    ret = mJpegHandle.start_job(&jpg_job, &jobId);
    if (ret == NO_ERROR) {
        // remember job info
        jpeg_job_data->jobId = jobId;
        jpeg_job_data->src_frame = recvd_frame;
    }

    ALOGD("%s : X", __func__);
    return ret;
}

/*===========================================================================
 * FUNCTION   : processRawImageImpl
 *
 * DESCRIPTION: function to send raw image to upper layer
 *
 * PARAMETERS :
 *   @recvd_frame   : frame to be encoded
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
int32_t QCameraPostProcessor::processRawImageImpl(mm_camera_super_buf_t *recvd_frame)
{
    int32_t rc = NO_ERROR;

    mm_camera_buf_def_t *frame = recvd_frame->bufs[0];
    QCameraMemory *rawMemObj = (QCameraMemory *)frame->mem_info;
    camera_memory_t *raw_mem = rawMemObj->getMemory(frame->buf_idx, false);

    if (NULL != rawMemObj && NULL != raw_mem) {
        // send data callback for COMPRESSED_IMAGE
        rawMemObj->cleanCache(frame->buf_idx);

        // dump frame into file
        m_parent->dumpFrameToFile(frame->buffer, frame->frame_len,
                                  frame->buf_idx, QCAMERA_DUMP_FRM_RAW);

        // send data callback / notify for RAW_IMAGE
        if (NULL != m_parent->mDataCb &&
            m_parent->msgTypeEnabled(CAMERA_MSG_RAW_IMAGE) > 0) {
            m_parent->mDataCb(CAMERA_MSG_RAW_IMAGE,
                         raw_mem,
                         0,
                         NULL,
                         m_parent->mCallbackCookie);
        }
        if (NULL != m_parent->mNotifyCb &&
            m_parent->msgTypeEnabled(CAMERA_MSG_RAW_IMAGE_NOTIFY) > 0) {
            m_parent->mNotifyCb(CAMERA_MSG_RAW_IMAGE_NOTIFY,
                                0, 0, m_parent->mCallbackCookie);
        }

        if ((m_parent->mDataCb != NULL) &&
            m_parent->msgTypeEnabled(CAMERA_MSG_COMPRESSED_IMAGE) > 0) {
            qcamera_release_data_t release_data;
            memset(&release_data, 0, sizeof(qcamera_release_data_t));
            release_data.frame = recvd_frame;
            sendDataNotify(CAMERA_MSG_COMPRESSED_IMAGE,
                           raw_mem,
                           0,
                           NULL,
                           &release_data);
        }
    } else {
        ALOGE("%s: Cannot get raw mem", __func__);
        rc = UNKNOWN_ERROR;
    }

    return rc;
}

/*===========================================================================
 * FUNCTION   : dataNotifyRoutine
 *
 * DESCRIPTION: data notify routine that will send data to upper layer through
 *              registered data callback
 *
 * PARAMETERS :
 *   @data    : user data ptr (QCameraPostProcessor)
 *
 * RETURN     : None
 *==========================================================================*/
void *QCameraPostProcessor::dataNotifyRoutine(void *data)
{
    int running = 1;
    int ret;
    QCameraPostProcessor *pme = (QCameraPostProcessor *)data;
    QCameraCmdThread *cmdThread = &pme->m_dataNotifyTh;
    uint8_t isActive = FALSE;
    uint32_t numOfSnapshotExpected = 0;
    uint32_t numOfSnapshotRcvd = 0;

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

        // we got notified about new cmd avail in cmd queue
        camera_cmd_type_t cmd = cmdThread->getCmd();
        ALOGD("%s: get cmd %d", __func__, cmd);
        switch (cmd) {
        case CAMERA_CMD_TYPE_START_DATA_PROC:
            isActive = TRUE;
            // init flag to FALSE
            numOfSnapshotExpected = pme->m_parent->numOfSnapshotsExpected();
            numOfSnapshotRcvd = 0;
            break;
        case CAMERA_CMD_TYPE_STOP_DATA_PROC:
            // flush jpeg data queue
            pme->m_dataNotifyQ.flush();

            // set flag to FALSE
            isActive = FALSE;

            numOfSnapshotExpected = 0;
            numOfSnapshotRcvd = 0;
            break;
        case CAMERA_CMD_TYPE_DO_NEXT_JOB:
            {
                if (TRUE == isActive) {
                    // first check if there is any pending jpeg notify
                    qcamera_data_argm_t *app_cb =
                        (qcamera_data_argm_t *)pme->m_dataNotifyQ.dequeue();
                    if (NULL != app_cb) {
                        ALOGE("%s: data notify cb", __func__);
                        if (pme->m_parent->msgTypeEnabled(app_cb->msg_type)) {
                            if (pme->m_parent->mDataCb) {
                                pme->m_parent->mDataCb(app_cb->msg_type,
                                                       app_cb->data,
                                                       app_cb->index,
                                                       app_cb->metadata,
                                                       pme->m_parent->mCallbackCookie);
                            }
                            numOfSnapshotRcvd++;
                            if (numOfSnapshotExpected > 0 &&
                                numOfSnapshotExpected == numOfSnapshotRcvd) {
                                // notify HWI that snapshot is done
                                pme->m_parent->processEvt(QCAMERA_SM_EVT_SNAPSHOT_DONE, NULL);
                            }
                        }
                        // free app_cb
                        pme->releaseNotifyData(app_cb);
                        free(app_cb);
                    }
                } else {
                    // do no op if not active
                    qcamera_data_argm_t *app_cb =
                        (qcamera_data_argm_t *)pme->m_dataNotifyQ.dequeue();
                    if (NULL != app_cb) {
                        // free app_cb
                        free(app_cb);
                    }
                }
            }
            break;
        case CAMERA_CMD_TYPE_EXIT:
            {
                // flush jpeg data queue
                pme->m_dataNotifyQ.flush();
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

/*===========================================================================
 * FUNCTION   : dataProcessRoutine
 *
 * DESCRIPTION: data process routine that handles input data either from input
 *              Jpeg Queue to do jpeg encoding, or from input PP Queue to do
 *              reprocess.
 *
 * PARAMETERS :
 *   @data    : user data ptr (QCameraPostProcessor)
 *
 * RETURN     : None
 *==========================================================================*/
void *QCameraPostProcessor::dataProcessRoutine(void *data)
{
    int running = 1;
    int ret;
    uint8_t is_active = FALSE;
    uint8_t needNewSess = TRUE;
    QCameraPostProcessor *pme = (QCameraPostProcessor *)data;
    QCameraCmdThread *cmdThread = &pme->m_dataProcTh;

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

        // we got notified about new cmd avail in cmd queue
        camera_cmd_type_t cmd = cmdThread->getCmd();
        switch (cmd) {
        case CAMERA_CMD_TYPE_START_DATA_PROC:
            ALOGD("%s: start data proc", __func__);
            is_active = TRUE;
            needNewSess = TRUE;
            break;
        case CAMERA_CMD_TYPE_STOP_DATA_PROC:
            {
                ALOGD("%s: stop data proc", __func__);
                is_active = FALSE;

                // cancel all ongoing jpeg jobs
                qcamera_jpeg_data_t *jpeg_job =
                    (qcamera_jpeg_data_t *)pme->m_ongoingJpegQ.dequeue();
                while (jpeg_job != NULL) {
                    pme->mJpegHandle.abort_job(jpeg_job->jobId);

                    pme->releaseJpegJobData(jpeg_job);
                    free(jpeg_job);

                    jpeg_job = (qcamera_jpeg_data_t *)pme->m_ongoingJpegQ.dequeue();
                }

                // destroy jpeg encoding session
                pme->mJpegHandle.destroy_session(pme->mJpegSessionId);
                pme->mJpegSessionId = 0;

                // free jpeg out buf and exif obj
                if (pme->m_pJpegOutputMem != NULL) {
                    pme->m_pJpegOutputMem->deallocate();
                    delete pme->m_pJpegOutputMem;
                    pme->m_pJpegOutputMem = NULL;
                }
                if (pme->m_pJpegExifObj != NULL) {
                    delete pme->m_pJpegExifObj;
                    pme->m_pJpegExifObj = NULL;
                }
                needNewSess = TRUE;

                // flush ongoing postproc Queue
                pme->m_ongoingPPQ.flush();

                // flush input jpeg Queue
                pme->m_inputJpegQ.flush();

                // flush input Postproc Queue
                pme->m_inputPPQ.flush();

                // flush input raw Queue
                pme->m_inputRawQ.flush();

                // signal cmd is completed
                cam_sem_post(&cmdThread->sync_sem);
            }
            break;
        case CAMERA_CMD_TYPE_DO_NEXT_JOB:
            {
                ALOGD("%s: Do next job, active is %d", __func__, is_active);
                if (is_active == TRUE) {
                    // check if there is any ongoing jpeg jobs
                    if (pme->m_ongoingJpegQ.isEmpty()) {
                        // no ongoing jpeg job, we are fine to send jpeg encoding job
                        mm_camera_super_buf_t *super_buf =
                            (mm_camera_super_buf_t *)pme->m_inputJpegQ.dequeue();

                        if (NULL != super_buf) {
                            //play shutter sound
                            pme->m_parent->playShutter();

                            qcamera_jpeg_data_t *jpeg_job =
                                (qcamera_jpeg_data_t *)malloc(sizeof(qcamera_jpeg_data_t));
                            if (jpeg_job != NULL) {
                                memset(jpeg_job, 0, sizeof(qcamera_jpeg_data_t));
                                ret = pme->encodeData(super_buf, jpeg_job, needNewSess);
                            } else {
                                ALOGE("%s: no mem for qcamera_jpeg_data_t", __func__);
                                ret = NO_MEMORY;
                            }
                            if (NO_ERROR != ret) {
                                pme->releaseSuperBuf(super_buf);
                                free(super_buf);
                                if (jpeg_job != NULL) {
                                    free(jpeg_job);
                                }
                                pme->sendDataNotify(CAMERA_MSG_COMPRESSED_IMAGE,
                                                    NULL,
                                                    0,
                                                    NULL,
                                                    NULL);
                            } else {
                                // add into ongoing jpeg job Q
                                pme->m_ongoingJpegQ.enqueue((void *)jpeg_job);
                            }
                        }
                    }

                    // process raw data if any
                    mm_camera_super_buf_t *super_buf =
                        (mm_camera_super_buf_t *)pme->m_inputRawQ.dequeue();

                    if (NULL != super_buf) {
                        //play shutter sound
                        pme->m_parent->playShutter();
                        ret = pme->processRawImageImpl(super_buf);
                        if (NO_ERROR != ret) {
                            pme->releaseSuperBuf(super_buf);
                            free(super_buf);
                            pme->sendDataNotify(CAMERA_MSG_COMPRESSED_IMAGE,
                                                NULL,
                                                0,
                                                NULL,
                                                NULL);
                        }
                    }

                    qcamera_pp_request_t *request =
                        (qcamera_pp_request_t *)pme->m_inputPPQ.dequeue();
                    if (NULL != request) {
                        qcamera_pp_data_t *pp_job =
                            (qcamera_pp_data_t *)malloc(sizeof(qcamera_pp_data_t));
                        if (pp_job != NULL) {
                            memset(pp_job, 0, sizeof(qcamera_pp_data_t));
                            QCameraReprocessChannel *reproc_channel =
                                (QCameraReprocessChannel *)pme->m_parent->m_channels[QCAMERA_CH_TYPE_REPROCESS];
                            if (reproc_channel != NULL) {
                                ret = reproc_channel->doReprocess(request->frame);
                                if (ret == 0) {
                                    pp_job->src_frame = request->frame;
                                }
                            } else {
                                ALOGE("%s: Reprocess channel is NULL", __func__);
                                ret = -1;
                            }
                        } else {
                            ALOGE("%s: no mem for qcamera_pp_data_t", __func__);
                            ret = -1;
                        }

                        if (0 != ret) {
                            // free frame
                            if (request->frame != NULL) {
                                pme->releaseSuperBuf(request->frame);
                                free(request->frame);
                            }
                            // free request buf
                            free(request);
                            // send error notify
                            pme->sendDataNotify(CAMERA_MSG_COMPRESSED_IMAGE,
                                                NULL,
                                                0,
                                                NULL,
                                                NULL);
                        } else {
                            // free request buf
                            free(request);

                            // add into ongoing jpeg job Q
                            pme->m_ongoingPPQ.enqueue((void *)pp_job);
                        }
                    }
                } else {
                    // not active, simply return buf and do no op
                    mm_camera_super_buf_t *super_buf =
                        (mm_camera_super_buf_t *)pme->m_inputJpegQ.dequeue();
                    if (NULL != super_buf) {
                        pme->releaseSuperBuf(super_buf);
                        free(super_buf);
                    }
                    super_buf = (mm_camera_super_buf_t *)pme->m_inputRawQ.dequeue();
                    if (NULL != super_buf) {
                        pme->releaseSuperBuf(super_buf);
                        free(super_buf);
                    }
                    qcamera_pp_request_t *request =
                        (qcamera_pp_request_t *)pme->m_inputPPQ.dequeue();
                    if (NULL != request) {
                        if (request->frame != NULL) {
                            pme->releaseSuperBuf(request->frame);
                        }
                        free(request);
                    }
                }
            }
            break;
        case CAMERA_CMD_TYPE_EXIT:
            running = 0;
            break;
        default:
            break;
        }
    } while (running);
    ALOGD("%s: X", __func__);
    return NULL;
}

/*===========================================================================
 * FUNCTION   : getJpegPaddingReq
 *
 * DESCRIPTION: function to add an entry to exif data
 *
 * PARAMETERS :
 *   @padding_info : jpeg specific padding requirement
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
int32_t QCameraPostProcessor::getJpegPaddingReq(cam_padding_info_t &padding_info)
{
    // TODO: hardcode for now, needs to query from mm-jpeg-interface
    padding_info.width_padding  = CAM_PAD_NONE;
    padding_info.height_padding  = CAM_PAD_TO_16;
    padding_info.plane_padding  = CAM_PAD_TO_WORD;
    return NO_ERROR;
}

/*===========================================================================
 * FUNCTION   : QCameraExif
 *
 * DESCRIPTION: constructor of QCameraExif
 *
 * PARAMETERS : None
 *
 * RETURN     : None
 *==========================================================================*/
QCameraExif::QCameraExif()
    : m_nNumEntries(0)
{
    memset(m_Entries, 0, sizeof(m_Entries));
}

/*===========================================================================
 * FUNCTION   : ~QCameraExif
 *
 * DESCRIPTION: deconstructor of QCameraExif. Will release internal memory ptr.
 *
 * PARAMETERS : None
 *
 * RETURN     : None
 *==========================================================================*/
QCameraExif::~QCameraExif()
{
    for (uint32_t i = 0; i < m_nNumEntries; i++) {
        switch (m_Entries[i].tag_entry.type) {
        case EXIF_BYTE:
            {
                if (m_Entries[i].tag_entry.count > 1 &&
                    m_Entries[i].tag_entry.data._bytes != NULL) {
                    free(m_Entries[i].tag_entry.data._bytes);
                    m_Entries[i].tag_entry.data._bytes = NULL;
                }
            }
            break;
        case EXIF_ASCII:
            {
                if (m_Entries[i].tag_entry.data._ascii != NULL) {
                    free(m_Entries[i].tag_entry.data._ascii);
                    m_Entries[i].tag_entry.data._ascii = NULL;
                }
            }
            break;
        case EXIF_SHORT:
            {
                if (m_Entries[i].tag_entry.count > 1 &&
                    m_Entries[i].tag_entry.data._shorts != NULL) {
                    free(m_Entries[i].tag_entry.data._shorts);
                    m_Entries[i].tag_entry.data._shorts = NULL;
                }
            }
            break;
        case EXIF_LONG:
            {
                if (m_Entries[i].tag_entry.count > 1 &&
                    m_Entries[i].tag_entry.data._longs != NULL) {
                    free(m_Entries[i].tag_entry.data._longs);
                    m_Entries[i].tag_entry.data._longs = NULL;
                }
            }
            break;
        case EXIF_RATIONAL:
            {
                if (m_Entries[i].tag_entry.count > 1 &&
                    m_Entries[i].tag_entry.data._rats != NULL) {
                    free(m_Entries[i].tag_entry.data._rats);
                    m_Entries[i].tag_entry.data._rats = NULL;
                }
            }
            break;
        case EXIF_UNDEFINED:
            {
                if (m_Entries[i].tag_entry.data._undefined != NULL) {
                    free(m_Entries[i].tag_entry.data._undefined);
                    m_Entries[i].tag_entry.data._undefined = NULL;
                }
            }
            break;
        case EXIF_SLONG:
            {
                if (m_Entries[i].tag_entry.count > 1 &&
                    m_Entries[i].tag_entry.data._slongs != NULL) {
                    free(m_Entries[i].tag_entry.data._slongs);
                    m_Entries[i].tag_entry.data._slongs = NULL;
                }
            }
            break;
        case EXIF_SRATIONAL:
            {
                if (m_Entries[i].tag_entry.count > 1 &&
                    m_Entries[i].tag_entry.data._srats != NULL) {
                    free(m_Entries[i].tag_entry.data._srats);
                    m_Entries[i].tag_entry.data._srats = NULL;
                }
            }
            break;
        }
    }
}

/*===========================================================================
 * FUNCTION   : addEntry
 *
 * DESCRIPTION: function to add an entry to exif data
 *
 * PARAMETERS :
 *   @tagid   : exif tag ID
 *   @type    : data type
 *   @count   : number of data in uint of its type
 *   @data    : input data ptr
 *
 * RETURN     : int32_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
int32_t QCameraExif::addEntry(exif_tag_id_t tagid,
                              exif_tag_type_t type,
                              uint32_t count,
                              void *data)
{
    int32_t rc = NO_ERROR;
    if(m_nNumEntries >= MAX_EXIF_TABLE_ENTRIES) {
        ALOGE("%s: Number of entries exceeded limit", __func__);
        return NO_MEMORY;
    }

    m_Entries[m_nNumEntries].tag_id = tagid;
    m_Entries[m_nNumEntries].tag_entry.type = type;
    m_Entries[m_nNumEntries].tag_entry.count = count;
    m_Entries[m_nNumEntries].tag_entry.copy = 1;
    switch (type) {
    case EXIF_BYTE:
        {
            if (count > 1) {
                uint8_t *values = (uint8_t *)malloc(count);
                if (values == NULL) {
                    ALOGE("%s: No memory for byte array", __func__);
                    rc = NO_MEMORY;
                } else {
                    memcpy(values, data, count);
                    m_Entries[m_nNumEntries].tag_entry.data._bytes = values;
                }
            } else {
                m_Entries[m_nNumEntries].tag_entry.data._byte = *(uint8_t *)data;
            }
        }
        break;
    case EXIF_ASCII:
        {
            char *str = NULL;
            str = (char *)malloc(count + 1);
            if (str == NULL) {
                ALOGE("%s: No memory for ascii string", __func__);
                rc = NO_MEMORY;
            } else {
                memset(str, 0, count + 1);
                memcpy(str, data, count);
                m_Entries[m_nNumEntries].tag_entry.data._ascii = str;
            }
        }
        break;
    case EXIF_SHORT:
        {
            if (count > 1) {
                uint16_t *values = (uint16_t *)malloc(count * sizeof(uint16_t));
                if (values == NULL) {
                    ALOGE("%s: No memory for short array", __func__);
                    rc = NO_MEMORY;
                } else {
                    memcpy(values, data, count * sizeof(uint16_t));
                    m_Entries[m_nNumEntries].tag_entry.data._shorts = values;
                }
            } else {
                m_Entries[m_nNumEntries].tag_entry.data._short = *(uint16_t *)data;
            }
        }
        break;
    case EXIF_LONG:
        {
            if (count > 1) {
                uint32_t *values = (uint32_t *)malloc(count * sizeof(uint32_t));
                if (values == NULL) {
                    ALOGE("%s: No memory for long array", __func__);
                    rc = NO_MEMORY;
                } else {
                    memcpy(values, data, count * sizeof(uint32_t));
                    m_Entries[m_nNumEntries].tag_entry.data._longs = values;
                }
            } else {
                m_Entries[m_nNumEntries].tag_entry.data._long = *(uint32_t *)data;
            }
        }
        break;
    case EXIF_RATIONAL:
        {
            if (count > 1) {
                rat_t *values = (rat_t *)malloc(count * sizeof(rat_t));
                if (values == NULL) {
                    ALOGE("%s: No memory for rational array", __func__);
                    rc = NO_MEMORY;
                } else {
                    memcpy(values, data, count * sizeof(rat_t));
                    m_Entries[m_nNumEntries].tag_entry.data._rats = values;
                }
            } else {
                m_Entries[m_nNumEntries].tag_entry.data._rat = *(rat_t *)data;
            }
        }
        break;
    case EXIF_UNDEFINED:
        {
            uint8_t *values = (uint8_t *)malloc(count);
            if (values == NULL) {
                ALOGE("%s: No memory for undefined array", __func__);
                rc = NO_MEMORY;
            } else {
                memcpy(values, data, count);
                m_Entries[m_nNumEntries].tag_entry.data._undefined = values;
            }
        }
        break;
    case EXIF_SLONG:
        {
            if (count > 1) {
                int32_t *values = (int32_t *)malloc(count * sizeof(int32_t));
                if (values == NULL) {
                    ALOGE("%s: No memory for signed long array", __func__);
                    rc = NO_MEMORY;
                } else {
                    memcpy(values, data, count * sizeof(int32_t));
                    m_Entries[m_nNumEntries].tag_entry.data._slongs = values;
                }
            } else {
                m_Entries[m_nNumEntries].tag_entry.data._slong = *(int32_t *)data;
            }
        }
        break;
    case EXIF_SRATIONAL:
        {
            if (count > 1) {
                srat_t *values = (srat_t *)malloc(count * sizeof(srat_t));
                if (values == NULL) {
                    ALOGE("%s: No memory for signed rational array", __func__);
                    rc = NO_MEMORY;
                } else {
                    memcpy(values, data, count * sizeof(srat_t));
                    m_Entries[m_nNumEntries].tag_entry.data._srats = values;
                }
            } else {
                m_Entries[m_nNumEntries].tag_entry.data._srat = *(srat_t *)data;
            }
        }
        break;
    }

    // Increase number of entries
    m_nNumEntries++;
    return rc;
}

}; // namespace qcamera
