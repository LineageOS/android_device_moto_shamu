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

#define LOG_TAG "QCameraStream"

#include <utils/Log.h>
#include <utils/Errors.h>
#include "QCamera2HWI.h"
#include "QCameraStream.h"

namespace android {

int32_t QCameraStream::get_bufs(
                     cam_frame_len_offset_t *offset,
                     uint8_t *num_bufs,
                     uint8_t **initial_reg_flag,
                     mm_camera_buf_def_t **bufs,
                     mm_camera_map_unmap_ops_tbl_t *ops_tbl,
                     void *user_data)
{
    QCameraStream *stream = reinterpret_cast<QCameraStream *>(user_data);
    if (!stream) {
        ALOGE("getBufs invalid stream pointer");
        return NO_MEMORY;
    }
    return stream->getBufs(offset, num_bufs, initial_reg_flag, bufs, ops_tbl);
}

int32_t QCameraStream::put_bufs(
                     mm_camera_map_unmap_ops_tbl_t *ops_tbl,
                     void *user_data)
{
    QCameraStream *stream = reinterpret_cast<QCameraStream *>(user_data);
    if (!stream) {
        ALOGE("putBufs invalid stream pointer");
        return NO_MEMORY;
    }
    return stream->putBufs(ops_tbl);
}

QCameraStream::QCameraStream(QCameraAllocator &allocator,
            uint32_t camHandle, uint32_t chId, mm_camera_ops_t *camOps) :
        mCamHandle(camHandle),
        mChannel(chId),
        mHandle(0),
        mCamOps(camOps),
        mStreamInfo(NULL),
        mNumBufs(0),
        mDataCB(NULL),
        mStreamInfoBuf(NULL),
        mStreamBufs(NULL),
        mAllocator(allocator)
{
    mMemVtbl.user_data = this;
    mMemVtbl.get_bufs = get_bufs;
    mMemVtbl.put_bufs = put_bufs;
    memset(&mBufDef[0], 0, sizeof(mBufDef));
    memset(&mFrameLenOffset, 0, sizeof(mFrameLenOffset));
}

QCameraStream::~QCameraStream()
{
    int rc = mCamOps->unmap_stream_buf(mCamHandle,
                mChannel, mHandle, CAM_MAPPING_BUF_TYPE_STREAM_INFO, 0);
    if (rc < 0) {
        ALOGE("Failed to map stream info buffer");
    }
    mStreamInfoBuf->deallocate();
    delete mStreamInfoBuf;

    // delete stream
    if (mHandle > 0) {
        mCamOps->delete_stream(mCamHandle, mChannel, mHandle);
        mHandle = 0;
    }
}

int32_t QCameraStream::init(cam_stream_type_t stream_type,
                         stream_cb_routine stream_cb, void *userdata)
{
    int32_t rc = OK;
    mm_camera_stream_config_t stream_config;

    mHandle = mCamOps->add_stream(mCamHandle, mChannel);
    if (!mHandle) {
        ALOGE("add_stream failed");
        rc = UNKNOWN_ERROR;
        goto done;
    }

    // Allocate and map stream info memory
    mStreamInfoBuf = mAllocator.allocateStreamInfoBuf(stream_type);
    if (!mStreamInfoBuf) {
        ALOGE("Failed to allocate stream info object");
        goto err1;
    }
    mStreamInfo = reinterpret_cast<cam_stream_info_t *>(mStreamInfoBuf->getPtr(0));

    rc = mCamOps->map_stream_buf(mCamHandle,
                mChannel, mHandle, CAM_MAPPING_BUF_TYPE_STREAM_INFO,
                0, mStreamInfoBuf->getFd(0), mStreamInfoBuf->getSize(0));
    if (rc < 0) {
        ALOGE("Failed to map stream info buffer");
        goto err2;
    }

    // Configure the stream
    stream_config.stream_info = mStreamInfo;
    stream_config.mem_vtbl = mMemVtbl;
    stream_config.stream_cb = dataNotifyCB;
    stream_config.userdata = this;
    rc = mCamOps->config_stream(mCamHandle,
                mChannel, mHandle, &stream_config);
    if (rc < 0) {
        ALOGE("Failed to config stream, rc = %d", rc);
        goto err3;
    }

    mDataCB = stream_cb;
    mUserData = userdata;
    return 0;

err3:
    mCamOps->unmap_stream_buf(mCamHandle,
                mChannel, mHandle, CAM_MAPPING_BUF_TYPE_STREAM_INFO, 0);

err2:
    mStreamInfoBuf->deallocate();
    delete mStreamInfoBuf;
err1:
    mCamOps->delete_stream(mCamHandle, mChannel, mHandle);
done:
    return rc;
}

int32_t QCameraStream::start()
{
    int32_t rc = 0;
    rc = mProcTh.launch(dataProcRoutine, this);
    return rc;
}

int32_t QCameraStream::stop()
{
    int32_t rc = 0;
    rc = mProcTh.exit();
    return rc;
}

int32_t QCameraStream::processZoomDone(preview_stream_ops_t *previewWindow)
{
    int32_t rc = 0;

    // TODO: get stream param for crop info

    // update preview window crop if it's preview/postview stream
    if ( (previewWindow != NULL) &&
         (mStreamInfo->stream_type == CAM_STREAM_TYPE_PREVIEW ||
          mStreamInfo->stream_type == CAM_STREAM_TYPE_POSTVIEW) ) {
        rc = previewWindow->set_crop(previewWindow,
                                     mStreamInfo->crop.left,
                                     mStreamInfo->crop.top,
                                     mStreamInfo->crop.width,
                                     mStreamInfo->crop.height);
    }

    return rc;
}

int32_t QCameraStream::processDataNotify(mm_camera_super_buf_t *frame)
{
    mDataQ.enqueue((void *)frame);
    return mProcTh.sendCmd(CAMERA_CMD_TYPE_DO_NEXT_JOB, FALSE, FALSE);
}

void QCameraStream::dataNotifyCB(mm_camera_super_buf_t *recvd_frame,
                                 void *userdata)
{
    QCameraStream* stream = (QCameraStream *)userdata;
    if (stream == NULL ||
        recvd_frame == NULL ||
        recvd_frame->bufs[0] == NULL ||
        recvd_frame->bufs[0]->stream_id != stream->getMyHandle()) {
        ALOGE("%s: Not a valid stream to handle buf", __func__);
        return;
    }

    mm_camera_super_buf_t *frame =
        (mm_camera_super_buf_t *)malloc(sizeof(mm_camera_super_buf_t));
    if (frame == NULL) {
        ALOGE("%s: No mem for mm_camera_buf_def_t", __func__);
        stream->bufDone(recvd_frame->bufs[0]->buf_idx);
        return;
    }
    memcpy(frame, recvd_frame, sizeof(mm_camera_super_buf_t));
    stream->processDataNotify(frame);
    return;
}

void *QCameraStream::dataProcRoutine(void *data)
{
    int running = 1;
    int ret;
    QCameraStream *pme = (QCameraStream *)data;
    QCameraCmdThread *cmdThread = &pme->mProcTh;

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

        // we got notified about new cmd avail in cmd queue
        camera_cmd_type_t cmd = cmdThread->getCmd();
        ALOGD("%s: get cmd %d", __func__, cmd);
        switch (cmd) {
        case CAMERA_CMD_TYPE_DO_NEXT_JOB:
            {
                mm_camera_super_buf_t *frame =
                    (mm_camera_super_buf_t *)pme->mDataQ.dequeue();
                if (NULL != frame) {
                    if (pme->mDataCB != NULL) {
                        pme->mDataCB(frame, pme, pme->mUserData);
                    } else {
                        // no data cb routine, return buf here
                        pme->bufDone(frame->bufs[0]->buf_idx);
                        free(frame);
                    }
                }
            }
            break;
        case CAMERA_CMD_TYPE_EXIT:
            /* flush data buf queue */
            pme->mDataQ.flush();
            running = 0;
            break;
        default:
            break;
        }
    } while (running);
    ALOGD("%s: X", __func__);
    return NULL;
}

int32_t QCameraStream::bufDone(int index)
{
    int32_t rc = NO_ERROR;

    if (index >= mNumBufs)
        return BAD_INDEX;

    rc = mCamOps->qbuf(mCamHandle, mChannel, &mBufDef[index]);
    if (rc < 0)
        return rc;

    mStreamBufs->invalidateCache(index);
    return rc;
}

int32_t QCameraStream::bufDone(const void *opaque, bool isMetaData)
{
    int32_t rc = NO_ERROR;

    int index = mStreamBufs->getMatchBufIndex(opaque, isMetaData);
    if (index == -1 || index >= mNumBufs) {
        return BAD_INDEX;
    }

    rc = bufDone(index);
    return rc;
}

int32_t QCameraStream::getBufs(cam_frame_len_offset_t *offset,
                     uint8_t *num_bufs,
                     uint8_t **initial_reg_flag,
                     mm_camera_buf_def_t **bufs,
                     mm_camera_map_unmap_ops_tbl_t *ops_tbl)
{
    int rc = NO_ERROR;
    uint8_t *regFlags;

    if (!ops_tbl) {
        ALOGE("getBufs: ops_tbl is NULL");
        return INVALID_OPERATION;
    }

    memcpy(&mFrameLenOffset, offset, sizeof(cam_frame_len_offset_t));

    //Allocate and map stream info buffer
    mStreamBufs = mAllocator.allocateStreamBuf(mStreamInfo->stream_type,
                                mFrameLenOffset.frame_len);
    if (!mStreamBufs) {
        ALOGE("Failed to allocate stream buffers");
        return NO_MEMORY;
    }

    mNumBufs = mStreamBufs->getCnt();
    for (int i = 0; i < mNumBufs; i++) {
        rc = ops_tbl->map_ops(i, mStreamBufs->getFd(i),
                mStreamBufs->getSize(i), ops_tbl->userdata);
        if (rc < 0) {
            ALOGE("getBufs: map_stream_buf failed: %d", rc);
            mStreamBufs->deallocate();
            delete mStreamBufs;
            return INVALID_OPERATION;
        }
    }

    //regFlags array is allocated by us, but consumed and freed by mm-camera-interface
    regFlags = (uint8_t *)malloc(sizeof(uint8_t) * mNumBufs);
    if (!regFlags) {
        ALOGE("Out of memory");
        mStreamBufs->deallocate();
        delete mStreamBufs;
        return NO_MEMORY;
    }

    for (int i = 0; i < mNumBufs; i++) {
        mStreamBufs->getBufDef(mFrameLenOffset, mBufDef[i], i);
    }
    rc = mStreamBufs->getRegFlags(regFlags);
    if (rc < 0) {
        ALOGE("getBufs: getRegFlags failed %d", rc);
        mStreamBufs->deallocate();
        delete mStreamBufs;
        return INVALID_OPERATION;
    }

    *num_bufs = mNumBufs;
    *initial_reg_flag = regFlags;
    *bufs = &mBufDef[0];
    return NO_ERROR;
}

int32_t QCameraStream::putBufs(mm_camera_map_unmap_ops_tbl_t *ops_tbl)
{
    int rc = NO_ERROR;
    for (int i = 0; i < mNumBufs; i++) {
        rc = ops_tbl->unmap_ops(i, ops_tbl->userdata);
        if (rc < 0) {
            ALOGE("getBufs: map_stream_buf failed: %d", rc);
        }
    }
    memset(&mBufDef[0], 0, sizeof(mBufDef));
    memset(&mFrameLenOffset, 0, sizeof(mFrameLenOffset));
    mStreamBufs->deallocate();
    delete mStreamBufs;

    return rc;
}

bool QCameraStream::isTypeOf(cam_stream_type_t type)
{
    if (mStreamInfo != NULL && (mStreamInfo->stream_type == type)) {
        return true;
    } else {
        return false;
    }
}

int32_t QCameraStream::getFrameOffset(cam_frame_len_offset_t &offset)
{
    memcpy(&offset, &mFrameLenOffset, sizeof(cam_frame_len_offset_t));
    return 0;
}

int32_t QCameraStream::getCropInfo(cam_rect_t &crop)
{
    if (mStreamInfo != NULL) {
        memcpy(&crop, &mStreamInfo->crop, sizeof(cam_rect_t));
        return 0;
    }
    return -1;
}

int32_t QCameraStream::getFrameDimension(cam_dimension_t &dim)
{
    if (mStreamInfo != NULL) {
        memcpy(&dim, &mStreamInfo->dim, sizeof(cam_dimension_t));
        return 0;
    }
    return -1;
}

int32_t QCameraStream::getFormat(cam_format_t &fmt)
{
    if (mStreamInfo != NULL) {
        fmt = mStreamInfo->fmt;
        return 0;
    }
    return -1;
}

}; // namespace android
