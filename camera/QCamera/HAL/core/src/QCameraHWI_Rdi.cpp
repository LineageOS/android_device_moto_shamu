/*
** Copyright (c) 2011-2012 The Linux Foundation. All rights reserved.
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

#define LOG_TAG "QCameraHWI_Rdi"
#include <utils/Log.h>
#include <utils/threads.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "QCameraHAL.h"
#include "QCameraHWI.h"
#include <gralloc_priv.h>
#include <genlock.h>

#define UNLIKELY(exp) __builtin_expect(!!(exp), 0)

/* QCameraHWI_Raw class implementation goes here*/
/* following code implement the RDI logic of this class*/

namespace android {

status_t   QCameraStream_Rdi::freeBufferRdi()
{
    int err = 0;
    status_t ret = NO_ERROR;

    ALOGE(" %s : E ", __FUNCTION__);
    mHalCamCtrl->mRdiMemoryLock.lock();
    mHalCamCtrl->releaseHeapMem(&mHalCamCtrl->mRdiMemory);
    memset(&mHalCamCtrl->mRdiMemory, 0, sizeof(mHalCamCtrl->mRdiMemory));
    mHalCamCtrl->mRdiMemoryLock.unlock();

    ALOGI(" %s : X ",__FUNCTION__);
    return NO_ERROR;
}

status_t QCameraStream_Rdi::initRdiBuffers()
{
    status_t ret = NO_ERROR;
    int width = 0;  /* width of channel  */
    int height = 0; /* height of channel */
    const char *pmem_region;
    uint8_t num_planes = 0;
    mm_camera_frame_len_offset offset;
    uint32_t planes[VIDEO_MAX_PLANES];
    int i,  frame_len, y_off, cbcr_off;

    ALOGE("%s:BEGIN",__func__);

    width = mHalCamCtrl->mRdiWidth;
    height = mHalCamCtrl->mRdiHeight;

    mHalCamCtrl->mRdiMemoryLock.lock();
    memset(&mHalCamCtrl->mRdiMemory, 0, sizeof(mHalCamCtrl->mRdiMemory));
    mHalCamCtrl->mRdiMemory.buffer_count = kRdiBufferCount;
    if(mHalCamCtrl->isZSLMode()) {
    if(mHalCamCtrl->getZSLQueueDepth() > kRdiBufferCount - 3)
      mHalCamCtrl->mRdiMemory.buffer_count =
      mHalCamCtrl->getZSLQueueDepth() + 3;
    }

    frame_len = mFrameOffsetInfo.frame_len;
    num_planes = mFrameOffsetInfo.num_planes;

    for ( i = 0; i < num_planes; i++) {
    planes[i] = mFrameOffsetInfo.mp[i].len;
    }
    y_off = 0;
    cbcr_off = planes[0];

    if (mHalCamCtrl->initHeapMem(&mHalCamCtrl->mRdiMemory,
     mHalCamCtrl->mRdiMemory.buffer_count,
     frame_len, y_off, cbcr_off, MSM_PMEM_MAINIMG,
     NULL,num_planes, planes) < 0) {
              ret = NO_MEMORY;
              return ret;
    };
    mHalCamCtrl->mRdiMemoryLock.unlock();

    for ( i = 0; i < num_planes; i++) {
    planes[i] = mFrameOffsetInfo.mp[i].len;
    }

    ALOGE("DEBUG: buffer count = %d",mHalCamCtrl->mRdiMemory.buffer_count);
    for (i = 0; i < mHalCamCtrl->mRdiMemory.buffer_count ; i++) {
      int j;

      mRdiBuf[i].fd = mHalCamCtrl->mRdiMemory.fd[i]; 
      mRdiBuf[i].frame_len = mHalCamCtrl->mRdiMemory.alloc[i].len;
      mRdiBuf[i].num_planes = num_planes;

      /* Plane 0 needs to be set seperately. Set other planes
      * in a loop. */
      mFrameOffsetInfo.mp[0].len = mHalCamCtrl->mRdiMemory.alloc[i].len;

      mRdiBuf[i].planes[0].length = mFrameOffsetInfo.mp[0].len;
      mRdiBuf[i].planes[0].m.userptr = mRdiBuf[i].fd;
      mRdiBuf[i].planes[0].data_offset = mFrameOffsetInfo.mp[0].offset;
      mRdiBuf[i].planes[0].reserved[0] = 0;
      for (j = 1; j < num_planes; j++) {
          mRdiBuf[i].planes[j].length = mFrameOffsetInfo.mp[j].len;
          mRdiBuf[i].planes[j].m.userptr = mRdiBuf[i].fd;
          mRdiBuf[i].planes[j].data_offset = mFrameOffsetInfo.mp[j].offset;
          mRdiBuf[i].planes[j].reserved[0] =
              mRdiBuf[i].planes[j-1].reserved[0] +
              mRdiBuf[i].planes[j-1].length;
      }
      ALOGE(" %s : Buffer allocated fd = %d, length = %d num_planes = %d,"
            "mRdiBuf[i].planes[0].length = %d,mRdiBuf[i].planes[0].m.userptr = %d, mRdiBuf[i].planes[0].data_offset = %d",
             __func__,mRdiBuf[i].fd,mRdiBuf[i].frame_len,num_planes,mRdiBuf[i].planes[0].length,
            mRdiBuf[i].planes[0].m.userptr,mRdiBuf[i].planes[0].data_offset);
    }

    end:
    if (MM_CAMERA_OK == ret ) {
    ALOGV("%s: X - NO_ERROR ", __func__);
    return NO_ERROR;
    }
    ALOGV("%s: X - BAD_VALUE ", __func__);
    return BAD_VALUE;
}

void QCameraStream_Rdi::dumpFrameToFile(mm_camera_buf_def_t* newFrame)
{
    char buf[32];
    int file_fd;
    int i;
    char *ext = "yuv";
    int w,h,main_422;
    static int count = 0;
    char *name = "rdi";

    w = mHalCamCtrl->mRdiWidth;
    h = mHalCamCtrl->mRdiHeight;
    main_422 = 1;

    if ( newFrame != NULL) {
        char * str;
        snprintf(buf, sizeof(buf), "/data/%s_%d.%s", name,count,ext);
        file_fd = open(buf, O_RDWR | O_CREAT, 0777);
        if (file_fd < 0) {
            ALOGE("%s: cannot open file\n", __func__);
        } else {
            void* y_off = newFrame->buffer + newFrame->planes[0].data_offset;
            void* cbcr_off = newFrame->buffer + newFrame->planes[0].length;

            write(file_fd, (const void *)(y_off), newFrame->planes[0].length);
            write(file_fd, (const void *)(cbcr_off), 
                  (newFrame->planes[1].length * newFrame->num_planes));
            close(file_fd);
        }
        count++;
    }
}

status_t QCameraStream_Rdi::processRdiFrame(
  mm_camera_super_buf_t *frame)
{
    ALOGE("%s",__func__);
    int err = 0;
    int msgType = 0;
    int i;
    camera_memory_t *data = NULL;

    Mutex::Autolock lock(mStopCallbackLock);
    if(!mActive) {
    ALOGE("RDI Streaming Stopped. Returning callback");
    return NO_ERROR;
    }
    if(mHalCamCtrl==NULL) {
    ALOGE("%s: X: HAL control object not set",__func__);
    /*Call buf done*/
    return BAD_VALUE;
    }

    mHalCamCtrl->mRdiMemoryLock.lock();
    mHalCamCtrl->mRdiMemoryLock.unlock();

    mHalCamCtrl->mCallbackLock.lock();
    camera_data_callback pcb = mHalCamCtrl->mDataCb;
    mHalCamCtrl->mCallbackLock.unlock();
    ALOGD("Message enabled = 0x%x", mHalCamCtrl->mMsgEnabled);

    mHalCamCtrl->dumpFrameToFile(frame->bufs[0], HAL_DUMP_FRM_RDI);
    //dumpFrameToFile(frame->bufs[0]);

    if (pcb != NULL) {
      //Sending rdi callback if corresponding Msgs are enabled
      if(mHalCamCtrl->mMsgEnabled & CAMERA_MSG_PREVIEW_FRAME) {
          msgType |=  CAMERA_MSG_PREVIEW_FRAME;
        data = mHalCamCtrl->mRdiMemory.camera_memory[frame->bufs[0]->buf_idx];
      } else {
          data = NULL;
      }

      if(msgType) {
          mStopCallbackLock.unlock();
          if(mActive)
            pcb(msgType, data, 0, NULL, mHalCamCtrl->mCallbackCookie);
      }
      ALOGD("end of cb");
    }
    if (MM_CAMERA_OK != p_mm_ops->ops->qbuf(mCameraHandle, mChannelId,
                                            frame->bufs[0])) {
        ALOGE("%s: Failed in Preview Qbuf\n", __func__);
        return BAD_VALUE;
    }

    return NO_ERROR;
}


// ---------------------------------------------------------------------------
// QCameraStream_Rdi
// ---------------------------------------------------------------------------

QCameraStream_Rdi::
QCameraStream_Rdi(uint32_t CameraHandle,
                        uint32_t ChannelId,
                        uint32_t Width,
                        uint32_t Height,
                        uint32_t Format,
                        uint8_t NumBuffers,
                        mm_camera_vtbl_t *mm_ops,
                        mm_camera_img_mode imgmode,
                        camera_mode_t mode)
  : QCameraStream(CameraHandle,
                 ChannelId,
                 Width,
                 Height,
                 Format,
                 NumBuffers,
                 mm_ops,
                 imgmode,
                 mode),
    mNumFDRcvd(0)
{
    mHalCamCtrl = NULL;
    ALOGE("%s: E", __func__);
    ALOGE("%s: X", __func__);
}
// ---------------------------------------------------------------------------
// QCameraStream_Rdi
// ---------------------------------------------------------------------------

QCameraStream_Rdi::~QCameraStream_Rdi() {
    ALOGV("%s: E", __func__);
    if(mActive) {
        stop();
    }
    if(mInit) {
        release();
    }
    mInit = false;
    mActive = false;
    ALOGV("%s: X", __func__);

}
// ---------------------------------------------------------------------------
// QCameraStream_Rdi
// ---------------------------------------------------------------------------

status_t QCameraStream_Rdi::init() {

    status_t ret = NO_ERROR;
    ALOGV("%s: E", __func__);
    return ret;
}
// ---------------------------------------------------------------------------
// QCameraStream_Rdi
// ---------------------------------------------------------------------------

status_t QCameraStream_Rdi::start()
{
    ALOGV("%s: E", __func__);
    status_t ret = NO_ERROR;
    uint32_t stream_info;
    ALOGE("%s: X", __func__);
    return ret;
}


// ---------------------------------------------------------------------------
// QCameraStream_Rdi
// ---------------------------------------------------------------------------
  void QCameraStream_Rdi::stop() {
    ALOGE("%s: E", __func__);
    int ret=MM_CAMERA_OK;
    uint32_t stream_info;
    uint32_t str[1];
    str[0] = mStreamId;

    ALOGE("%s : E", __func__);

    ret = p_mm_ops->ops->stop_streams(mCameraHandle, mChannelId, 1, str);
    if(ret != MM_CAMERA_OK){
      ALOGE("%s : stop_streams failed, ret = %d", __func__, ret);
    }
    ret= QCameraStream::deinitStream();
    ALOGE(": %s : De init Channel",__func__);
    if(ret != MM_CAMERA_OK) {
        ALOGE("%s:Deinit preview channel failed=%d\n", __func__, ret);
    }

    ALOGE("%s : X", __func__);
  }
// ---------------------------------------------------------------------------
// QCameraStream_Rdi
// ---------------------------------------------------------------------------
  void QCameraStream_Rdi::release() {

    ALOGE("%s : BEGIN",__func__);
    int ret=MM_CAMERA_OK,i;

    if(!mInit)
    {
      ALOGE("%s : Stream not Initalized",__func__);
      return;
    }

    if(mActive) {
      this->stop();
    }
    ALOGE("%s: END", __func__);

  }

QCameraStream*
QCameraStream_Rdi::createInstance(uint32_t CameraHandle,
                        uint32_t ChannelId,
                        uint32_t Width,
                        uint32_t Height,
                        uint32_t Format,
                        uint8_t NumBuffers,
                        mm_camera_vtbl_t *mm_ops,
                        mm_camera_img_mode imgmode,
                        camera_mode_t mode)
{
  QCameraStream* pme = new QCameraStream_Rdi(CameraHandle,
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
// ---------------------------------------------------------------------------
// QCameraStream_Rdi
// ---------------------------------------------------------------------------

void QCameraStream_Rdi::deleteInstance(QCameraStream *p)
{
  if (p){
    ALOGV("%s: BEGIN", __func__);
    p->release();
    delete p;
    p = NULL;
    ALOGV("%s: END", __func__);
  }
}

// ---------------------------------------------------------------------------
// No code beyone this line
// ---------------------------------------------------------------------------
}; // namespace android
