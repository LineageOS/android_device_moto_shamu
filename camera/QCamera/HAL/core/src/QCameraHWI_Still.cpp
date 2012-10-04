/*
** Copyright (c) 2012 The Linux Foundation. All rights reserved.
**
** Not a Contribution, Apache license notifications and license are retained
** for attribution purposes only.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

/*#error uncomment this for compiler test!*/

#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0
#define LOG_NIDEBUG 0
#define LOG_TAG "QCameraHWI_Still"
#include <utils/Log.h>
#include <utils/threads.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <media/mediarecorder.h>
#include <math.h>
#include "QCameraHAL.h"
#include "QCameraHWI.h"
#include "mm_jpeg_interface.h"


/* following code implement the still image capture & encoding logic of this class*/
namespace android {

QCameraStream* QCameraStream_SnapshotMain::createInstance(
                        uint32_t CameraHandle,
                        uint32_t ChannelId,
                        uint32_t Width,
                        uint32_t Height,
                        uint32_t Format,
                        uint8_t NumBuffers,
                        mm_camera_vtbl_t *mm_ops,
                        mm_camera_img_mode imgmode,
                        camera_mode_t mode)
{
    ALOGE("%s : E", __func__);
    QCameraStream* pme = new QCameraStream_SnapshotMain(
                       CameraHandle,
                       ChannelId,
                       Width,
                       Height,
                       Format,
                       NumBuffers,
                       mm_ops,
                       imgmode,
                       mode);
    return pme;
    ALOGE("%s : X", __func__);
}

QCameraStream_SnapshotMain::
QCameraStream_SnapshotMain(uint32_t CameraHandle,
                       uint32_t ChannelId,
                       uint32_t Width,
                       uint32_t Height,
                       uint32_t Format,
                       uint8_t NumBuffers,
                       mm_camera_vtbl_t *mm_ops,
                       mm_camera_img_mode imgmode,
                       camera_mode_t mode)
  :QCameraStream(CameraHandle,
                 ChannelId,
                 Width,
                 Height,
                 Format,
                 NumBuffers,
                 mm_ops,
                 imgmode,
                 mode)
{
    ALOGE("%s: E", __func__);
    ALOGE("%s : X", __func__);
}

QCameraStream_SnapshotMain::~QCameraStream_SnapshotMain()
{
    release();
}

void QCameraStream_SnapshotMain::deleteInstance(QCameraStream *p)
{
  if (p){
    p->release();
    delete p;
    p = NULL;
  }
}

void QCameraStream_SnapshotMain::release()
{
    streamOff(0);
    deinitStream();

}

bool QCameraStream_SnapshotMain::isZSLMode()
{
   return (myMode & CAMERA_ZSL_MODE);
}

status_t QCameraStream_SnapshotMain::initStream(uint8_t no_cb_needed, uint8_t stream_on)
{
    status_t ret = NO_ERROR;

    if(isZSLMode()) {
        mNumBuffers = mHalCamCtrl->getZSLQueueDepth() + 3;
    } else if (!mHalCamCtrl->mHdrInfo.hdr_on) {
        mNumBuffers = 1;
    }
    ret = QCameraStream::initStream(no_cb_needed, stream_on);
    return ret;;
}


status_t QCameraStream_SnapshotMain::initMainBuffers()
{
    ALOGE("%s : E", __func__);

    if ((mNumBuffers == 0) || (mNumBuffers > MM_CAMERA_MAX_NUM_FRAMES)) {
        ALOGE("%s: Invalid number of buffers (=%d) requested!",
             __func__, mNumBuffers);
        return BAD_VALUE;
    }

    memset(mSnapshotStreamBuf, 0, sizeof(mSnapshotStreamBuf));
    if (mHalCamCtrl->initHeapMem(&mHalCamCtrl->mSnapshotMemory,
                                 mNumBuffers,
                                 mFrameOffsetInfo.frame_len,
                                 MSM_PMEM_MAINIMG,
                                 &mFrameOffsetInfo,
                                 mSnapshotStreamBuf) < 0) {
        return NO_MEMORY;
    }

    /* If we have reached here successfully, we have allocated buffer.
       Set state machine.*/
    ALOGD("%s: X", __func__);
    return NO_ERROR;

}

void QCameraStream_SnapshotMain::deInitMainBuffers()
{
    ALOGE("%s: Release Snapshot main Memory",__func__);

    mHalCamCtrl->releaseHeapMem(&mHalCamCtrl->mSnapshotMemory);
}

void QCameraStream_SnapshotThumbnail::deInitThumbnailBuffers()
{
     ALOGE("%s: Release Snapshot thumbnail Memory",__func__);
     mHalCamCtrl->releaseHeapMem(&mHalCamCtrl->mThumbnailMemory);
}

QCameraStream_SnapshotThumbnail::
QCameraStream_SnapshotThumbnail(uint32_t CameraHandle,
                       uint32_t ChannelId,
                       uint32_t Width,
                       uint32_t Height,
                       uint32_t Format,
                       uint8_t NumBuffers,
                       mm_camera_vtbl_t *mm_ops,
                       mm_camera_img_mode imgmode,
                       camera_mode_t mode)
  :QCameraStream(CameraHandle,
                 ChannelId,
                 Width,
                 Height,
                 Format,
                 NumBuffers,
                 mm_ops,
                 imgmode,
                 mode)
{

}
QCameraStream* QCameraStream_SnapshotThumbnail::createInstance(
                        uint32_t CameraHandle,
                        uint32_t ChannelId,
                        uint32_t Width,
                        uint32_t Height,
                        uint32_t Format,
                        uint8_t NumBuffers,
                        mm_camera_vtbl_t *mm_ops,
                        mm_camera_img_mode imgmode,
                        camera_mode_t mode)
{
    QCameraStream* pme = new QCameraStream_SnapshotThumbnail(
                       CameraHandle,
                       ChannelId,
                       Width,
                       Height,
                       Format,
                       NumBuffers,
                       mm_ops,
                       imgmode,
                       mode);
    return pme;
}


QCameraStream_SnapshotThumbnail::~QCameraStream_SnapshotThumbnail()
{
     release();
}

void QCameraStream_SnapshotThumbnail::deleteInstance(QCameraStream *p)
{
  if (p){
    p->release();
    delete p;
    p = NULL;
  }
}

void QCameraStream_SnapshotThumbnail::release()
{
    streamOff(0);
    deinitStream();

}

status_t QCameraStream_SnapshotThumbnail::initThumbnailBuffers()
{
    if ((mNumBuffers == 0) || (mNumBuffers > MM_CAMERA_MAX_NUM_FRAMES)) {
        ALOGE("%s: Invalid number of buffers (=%d) requested!",
             __func__, mNumBuffers);
        return BAD_VALUE;
    }

    memset(mPostviewStreamBuf, 0, sizeof(mPostviewStreamBuf));
    if (mHalCamCtrl->initHeapMem(
                         &mHalCamCtrl->mThumbnailMemory,
                         mNumBuffers,
                         mFrameOffsetInfo.frame_len,
                         MSM_PMEM_THUMBNAIL,
                         &mFrameOffsetInfo,
                         mPostviewStreamBuf) < 0) {
    	return NO_MEMORY;
    }

    return NO_ERROR;
}


}; // namespace android

