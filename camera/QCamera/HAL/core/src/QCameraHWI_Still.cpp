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
    status_t ret = NO_ERROR;
    uint32_t frame_len, y_off, cbcr_off;
    uint8_t num_planes=mFrameOffsetInfo.num_planes;
    int num_of_buf=mNumBuffers;
    uint32_t planes[VIDEO_MAX_PLANES];
    int rotation = 0;

    memset(mSnapshotStreamBuf, 0, MM_CAMERA_MAX_NUM_FRAMES *sizeof(mm_camera_buf_def_t));

    if ((num_of_buf == 0) || (num_of_buf > MM_CAMERA_MAX_NUM_FRAMES)) {
        ALOGE("%s: Invalid number of buffers (=%d) requested!",
             __func__, num_of_buf);
        return BAD_VALUE;
    }

    ALOGD("%s: Mode: %d Num_of_buf: %d ImageSizes: main: %dx%d",
         __func__, myMode, num_of_buf,
         mWidth, mHeight);

    /* Number of buffers to be set*/
    /* Set the JPEG Rotation here since get_buffer_offset needs
     * the value of rotation.*/
    num_planes = 2;
    planes[0] = mFrameOffsetInfo.mp[0].len;
    planes[1] = mFrameOffsetInfo.mp[1].len;
    frame_len = mFrameOffsetInfo.frame_len;
    y_off = mFrameOffsetInfo.mp[0].offset;
    cbcr_off = mFrameOffsetInfo.mp[1].offset;
    if (mHalCamCtrl->initHeapMem(&mHalCamCtrl->mSnapshotMemory,
        num_of_buf,frame_len, y_off, cbcr_off,
        MSM_PMEM_MAINIMG, mSnapshotStreamBuf,
        num_planes, planes) < 0) {
        ret = NO_MEMORY;
    	return ret;
    };
    for(int i=0;i<num_of_buf;i++) {
       this->mSnapshotStreamBuf[i].stream_id=mStreamId;
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
    status_t ret = NO_ERROR;
    uint32_t frame_len, y_off, cbcr_off;
    uint8_t num_planes=mFrameOffsetInfo.num_planes;
    int num_of_buf=mNumBuffers;
    uint32_t planes[VIDEO_MAX_PLANES];
    int rotation = 0;


    num_planes = mFrameOffsetInfo.num_planes;
    planes[0] = mFrameOffsetInfo.mp[0].len;
    planes[1] = mFrameOffsetInfo.mp[1].len;
    frame_len = mFrameOffsetInfo.frame_len;
    y_off = mFrameOffsetInfo.mp[0].offset;
    cbcr_off = mFrameOffsetInfo.mp[1].offset;

    if (mHalCamCtrl->initHeapMem(
                         &mHalCamCtrl->mThumbnailMemory,
                         num_of_buf,
                         frame_len,
                         y_off,
                         cbcr_off,
                         MSM_PMEM_THUMBNAIL,
                         mPostviewStreamBuf,
    		         num_planes,
                         planes) < 0) {
    	return NO_MEMORY;
    }


    return NO_ERROR;
}


}; // namespace android

