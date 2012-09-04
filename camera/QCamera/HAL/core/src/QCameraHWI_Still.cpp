/*
** Copyright (c) 2012 Code Aurora Forum. All rights reserved.
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
    // for hdr
    memset(&mHdrInfo, 0, sizeof(snap_hdr_record_t));

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

void QCameraStream_SnapshotMain::receiveCompleteJpegPicture(camera_jpeg_data_t *jpeg_data)
{
   ALOGE("%s: E", __func__);
   int msg_type = CAMERA_MSG_COMPRESSED_IMAGE;
   camera_data_callback jpg_data_cb = NULL;

   if (jpeg_data->src_frame != NULL) {
       ALOGD("%s: qbuf num_buf=%d", __func__, jpeg_data->src_frame->num_bufs);
       for(int i = 0; i< jpeg_data->src_frame->num_bufs; i++) {
           if(MM_CAMERA_OK != p_mm_ops->ops->qbuf(jpeg_data->src_frame->camera_handle,
                                                  jpeg_data->src_frame->ch_id,
                                                  jpeg_data->src_frame->bufs[i])){
               ALOGE("%s : Buf done failed for buffer[%d]", __func__, i);
           }

       }
       free(jpeg_data->src_frame);
       jpeg_data->src_frame = NULL;
   }

   if(jpeg_data->status == JPEG_JOB_STATUS_ERROR) {
       if (NULL != jpeg_data->out_data) {
           free(jpeg_data->out_data);
           jpeg_data->out_data = NULL;
       }
       jpegErrorHandler(jpeg_data->status);
       ALOGE("Error event handled from jpeg");
       return;
   }

   if(jpeg_data->thumbnailDroppedFlag) {
       ALOGE("%s : Error in thumbnail encoding", __func__);
       return;
   }

   msg_type = CAMERA_MSG_COMPRESSED_IMAGE;
   if(mHalCamCtrl->mDataCb && (mHalCamCtrl->mMsgEnabled & msg_type)){
       jpg_data_cb = mHalCamCtrl->mDataCb;
   } else {
       ALOGE("%s: JPEG callback was cancelled", __func__);
   }
   mHalCamCtrl->deinitExifData();

   mHalCamCtrl->dumpFrameToFile(jpeg_data->out_data,
                                jpeg_data->data_size,
                                (char *)"debug",
                                (char *)"jpg",
                                jpeg_data->jobId);

   ALOGE("%s: jpeg_size=%d, jpeg_mem size=%d",
         __func__, jpeg_data->data_size, mHalCamCtrl->mJpegMemory.size);
   camera_memory_t *encodedMem = mHalCamCtrl->mGetMemory(mHalCamCtrl->mJpegMemory.fd[0],
                                                         jpeg_data->data_size,
                                                         1, mHalCamCtrl);
   if (encodedMem == NULL) {
       ALOGE("%s: no mem for encodedMem", __func__);
       free(jpeg_data->out_data);
       jpeg_data->out_data = NULL;
       return;
   }

   memcpy(encodedMem->data, jpeg_data->out_data, jpeg_data->data_size);
   free(jpeg_data->out_data);
   jpeg_data->out_data = NULL;

   if(jpg_data_cb != NULL){
      ALOGE("%s : Calling upperlayer callback to store JPEG image", __func__);
      jpg_data_cb(msg_type, encodedMem, 0, NULL, mHalCamCtrl->mCallbackCookie);
   } else {
      ALOGE("%s : jpg_data_cb == NULL", __func__);
   }
   encodedMem->release( encodedMem);
   jpg_data_cb = NULL;

   ALOGE("%s: X", __func__);
}

void QCameraStream_SnapshotMain::jpegErrorHandler(jpeg_job_status_t status){
   ALOGE("%s:  E", __func__);
   if(mHalCamCtrl->mDataCb != NULL){
       mHalCamCtrl->mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, NULL, 0, NULL, mHalCamCtrl->mCallbackCookie);
   }
   ALOGE("%s:  X", __func__);
}

status_t QCameraStream_SnapshotMain::receiveRawPicture(mm_camera_super_buf_t* recvd_frame, uint32_t *job_id){
    ALOGE("%s : E", __func__);
    status_t rc = NO_ERROR;

    rc = encodeData(recvd_frame, job_id);

     //play shutter sound
     if(!mHalCamCtrl->mShutterSoundPlayed){
         notifyShutter(true);
     }
     notifyShutter(false);
     mHalCamCtrl->mShutterSoundPlayed = false;

     ALOGE("%s : X", __func__);
     return rc;
}

void QCameraStream_SnapshotMain::notifyShutter(bool play_shutter_sound){
     ALOGV("%s : E", __func__);
     if(mHalCamCtrl->mNotifyCb){
         mHalCamCtrl->mNotifyCb(CAMERA_MSG_SHUTTER, 0, play_shutter_sound, mHalCamCtrl->mCallbackCookie);
     }
     ALOGV("%s : X", __func__);
}

status_t QCameraStream_SnapshotMain::encodeData(mm_camera_super_buf_t* recvd_frame, uint32_t *jobId){
    ALOGV("%s : E", __func__);
    status_t ret = NO_ERROR;
    mm_jpeg_job jpg_job;
    mm_camera_buf_def_t *main_frame = NULL;
    mm_camera_buf_def_t *thumb_frame = NULL;
    src_image_buffer_info* main_buf_info = NULL;
    src_image_buffer_info* thumb_buf_info = NULL;
    uint8_t src_img_num = recvd_frame->num_bufs;
    int i;

    *jobId = 0;

    QCameraStream *main_stream = this;
    for (i = 0; i < recvd_frame->num_bufs; i++) {
        if (main_stream->mStreamId == recvd_frame->bufs[i]->stream_id) {
            main_frame = recvd_frame->bufs[i];
            break;
        }
    }
    if(main_frame == NULL){
       ALOGE("%s : Main frame is NULL", __func__);
       return ret;
    }

    camera_jpeg_encode_cookie_t *cookie =
        (camera_jpeg_encode_cookie_t *)malloc(sizeof(camera_jpeg_encode_cookie_t));
    if (NULL == cookie) {
        ALOGE("%s : no mem for cookie", __func__);
        return ret;
    }
    cookie->src_frame = recvd_frame;
    cookie->userdata = mHalCamCtrl;

    mHalCamCtrl->dumpFrameToFile(main_frame, HAL_DUMP_FRM_MAIN);

    QCameraStream *thumb_stream = NULL;
    if (recvd_frame->num_bufs > 1) {
        /* has thumbnail */
        if(!isZSLMode()) {
            thumb_stream = mHalCamCtrl->mStreamSnapThumb;
        } else {
            thumb_stream = mHalCamCtrl->mStreamDisplay;
        }
        for (i = 0; i < recvd_frame->num_bufs; i++) {
            if (thumb_stream->mStreamId == recvd_frame->bufs[i]->stream_id) {
                thumb_frame = recvd_frame->bufs[i];
                break;
            }
        }
    }  else if(mHalCamCtrl->mStreamSnapThumb->mWidth &&
              mHalCamCtrl->mStreamSnapThumb->mHeight) {
        /*thumbnail is required, not YUV thumbnail, borrow main image*/
        thumb_stream = main_stream;
        thumb_frame = main_frame;
        src_img_num++;
    }

    if (thumb_stream) {
        mHalCamCtrl->dumpFrameToFile(thumb_frame, HAL_DUMP_FRM_THUMBNAIL);
    }

    int jpeg_quality = mHalCamCtrl->getJpegQuality();
    if (jpeg_quality <= 0) {
        jpeg_quality = 85;
    }

    memset(&jpg_job, 0, sizeof(mm_jpeg_job));
    jpg_job.job_type = JPEG_JOB_TYPE_ENCODE;
    jpg_job.encode_job.userdata = cookie;
    jpg_job.encode_job.jpeg_cb = QCameraHardwareInterface::snapshot_jpeg_cb;
    jpg_job.encode_job.encode_parm.exif_data = mHalCamCtrl->getExifData();
    jpg_job.encode_job.encode_parm.exif_numEntries = mHalCamCtrl->getExifTableNumEntries();
    jpg_job.encode_job.encode_parm.rotation = mHalCamCtrl->getJpegRotation();
    ALOGV("%s: jpeg rotation is set to %d", __func__, jpg_job.encode_job.encode_parm.rotation);
    jpg_job.encode_job.encode_parm.buf_info.src_imgs.src_img_num = src_img_num;

    // fill in the src_img info
    //main img
    main_buf_info = &jpg_job.encode_job.encode_parm.buf_info.src_imgs.src_img[JPEG_SRC_IMAGE_TYPE_MAIN];
    main_buf_info->type = JPEG_SRC_IMAGE_TYPE_MAIN;
    main_buf_info->color_format = mHalCamCtrl->getColorfmtFromImgFmt(main_stream->mFormat);
    main_buf_info->quality = jpeg_quality;
    main_buf_info->src_image[0].fd = main_frame->fd;
    main_buf_info->src_image[0].buf_vaddr = (uint8_t*) main_frame->buffer;
    main_buf_info->src_dim.width = main_stream->mWidth;
    main_buf_info->src_dim.height = main_stream->mHeight;
    main_buf_info->out_dim.width = mHalCamCtrl->mPictureWidth;
    main_buf_info->out_dim.height = mHalCamCtrl->mPictureHeight;
    memcpy(&main_buf_info->crop, &main_stream->mCrop, sizeof(image_crop_t));
    if (main_buf_info->crop.width == 0 || main_buf_info->crop.height == 0) {
        main_buf_info->crop.width = main_stream->mWidth;
        main_buf_info->crop.height = main_stream->mHeight;
    }
    ALOGD("%s : Main Image :Input Dimension %d x %d output Dimension = %d X %d",
          __func__, main_buf_info->src_dim.width, main_buf_info->src_dim.height,
          main_buf_info->out_dim.width, main_buf_info->out_dim.height);
    main_buf_info->img_fmt = JPEG_SRC_IMAGE_FMT_YUV;
    main_buf_info->num_bufs = 1;
    main_buf_info->src_image[0].offset = mFrameOffsetInfo;
    ALOGD("%s : setting main image offset info, len = %d, offset = %d",
          __func__, mFrameOffsetInfo.mp[0].len, mFrameOffsetInfo.mp[0].offset);

    if (thumb_frame && thumb_stream) {
        /* fill in thumbnail src img encode param */
        thumb_buf_info = &jpg_job.encode_job.encode_parm.buf_info.src_imgs.src_img[JPEG_SRC_IMAGE_TYPE_THUMB];
        thumb_buf_info->type = JPEG_SRC_IMAGE_TYPE_THUMB;
        thumb_buf_info->color_format = mHalCamCtrl->getColorfmtFromImgFmt(thumb_stream->mFormat);
        thumb_buf_info->quality = jpeg_quality;
        thumb_buf_info->src_dim.width = thumb_stream->mWidth;
        thumb_buf_info->src_dim.height = thumb_stream->mHeight;
        thumb_buf_info->out_dim.width = mHalCamCtrl->thumbnailWidth;
        thumb_buf_info->out_dim.height = mHalCamCtrl->thumbnailHeight;
        memcpy(&thumb_buf_info->crop, &thumb_stream->mCrop, sizeof(image_crop_t));
        if (thumb_buf_info->crop.width == 0 || thumb_buf_info->crop.height == 0) {
            thumb_buf_info->crop.width = thumb_stream->mWidth;
            thumb_buf_info->crop.height = thumb_stream->mHeight;
        }
        ALOGD("%s : Thumanail :Input Dimension %d x %d output Dimension = %d X %d",
          __func__, thumb_buf_info->src_dim.width, thumb_buf_info->src_dim.height,
              thumb_buf_info->out_dim.width,thumb_buf_info->out_dim.height);
        thumb_buf_info->img_fmt = JPEG_SRC_IMAGE_FMT_YUV;
        thumb_buf_info->num_bufs = 1;
        thumb_buf_info->src_image[0].fd = thumb_frame->fd;
        thumb_buf_info->src_image[0].buf_vaddr = (uint8_t*) thumb_frame->buffer;
        thumb_buf_info->src_image[0].offset = thumb_stream->mFrameOffsetInfo;
        ALOGD("%s : setting thumb image offset info, len = %d, offset = %d",
              __func__, thumb_stream->mFrameOffsetInfo.mp[0].len, thumb_stream->mFrameOffsetInfo.mp[0].offset);
    }

    //fill in the sink img info
    jpg_job.encode_job.encode_parm.buf_info.sink_img.buf_len = mHalCamCtrl->mJpegMemory.size;
    jpg_job.encode_job.encode_parm.buf_info.sink_img.buf_vaddr =
        (uint8_t *)malloc(mHalCamCtrl->mJpegMemory.size);
    if (NULL == jpg_job.encode_job.encode_parm.buf_info.sink_img.buf_vaddr) {
        ALOGE("%s: ERROR: no memory for sink_img buf", __func__);
        free(cookie);
        cookie = NULL;
        return -1;
    }

    if (mHalCamCtrl->mJpegClientHandle > 0) {
        ret = mHalCamCtrl->mJpegHandle.start_job(mHalCamCtrl->mJpegClientHandle, &jpg_job, jobId);
    } else {
        ALOGE("%s: Error: bug here, mJpegClientHandle is 0", __func__);
        free(cookie);
        cookie = NULL;
        return -1;
    }

    ALOGV("%s : X", __func__);
    return ret;

}

status_t QCameraStream_SnapshotMain::initStream(int no_cb_needed)
{
    status_t ret = NO_ERROR;

    if(isZSLMode()) {
        mNumBuffers = mHalCamCtrl->getZSLQueueDepth() + 3;
    }else{
        mNumBuffers = 1;
    }
    ret = QCameraStream::initStream(no_cb_needed);
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

    ALOGD("%s: E", __func__);
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
    //TO-DO for now initHeapMem for jpeg too
    for(int i=0;i<num_of_buf;i++)
       this->mSnapshotStreamBuf[i].stream_id=mStreamId;

    if (mHalCamCtrl->initHeapMem(&mHalCamCtrl->mJpegMemory,
        1,frame_len, 0, cbcr_off,
        MSM_PMEM_MAX, NULL,
        num_planes, planes) < 0) {
        ALOGE("%s : initHeapMem for jpeg, ret = NO_MEMORY", __func__);
        ret = NO_MEMORY;
       // mHalCamCtrl->releaseHeapMem(&mHalCamCtrl->mJpegMemory);
    	return ret;
    };
    /* If we have reached here successfully, we have allocated buffer.
       Set state machine.*/
    ALOGD("%s: X", __func__);
    return NO_ERROR;

}

void QCameraStream_SnapshotMain::deInitMainBuffers()
{
    ALOGE("%s: Release Snapshot main Memory",__func__);

    mHalCamCtrl->releaseHeapMem(&mHalCamCtrl->mSnapshotMemory);
    mHalCamCtrl->releaseHeapMem(&mHalCamCtrl->mJpegMemory);

}

void QCameraStream_SnapshotThumbnail::deInitThumbnailBuffers()
{
     ALOGE("%s: Release Snapshot thumbnail Memory",__func__);
     mHalCamCtrl->releaseHeapMem(&mHalCamCtrl->mThumbnailMemory);
}

void QCameraStream_SnapshotMain::initHdrInfoForSnapshot(bool Hdr_on, int number_frames, int *exp )
{
    ALOGE("%s E hdr_on = %d", __func__, Hdr_on);
    mHdrInfo.hdr_on = Hdr_on;
    mHdrInfo.num_frame = number_frames;
    mHdrInfo.num_raw_received = 0;
    if(number_frames) {
        memcpy(mHdrInfo.exp, exp, sizeof(int)*number_frames);
    }
    memset(mHdrInfo.recvd_frame, 0,
           sizeof(mm_camera_super_buf_t *)*MAX_HDR_EXP_FRAME_NUM);
    ALOGE("%s X", __func__);
}

void QCameraStream_SnapshotMain::notifyHdrEvent(cam_ctrl_status_t status, void * cookie)
{
     ALOGE("%s E", __func__);
    camera_notify_callback         notifyCb;
    camera_data_callback           dataCb, jpgDataCb;
    int rc[2];
    mm_camera_super_buf_t *frame;
    int i;
    ALOGI("%s: HDR Done status (%d) received",__func__,status);
    Mutex::Autolock lock(mStopCallbackLock);
    for (i =0; i < 2; i++) {
        frame = mHdrInfo.recvd_frame[i];
        //rc[i] = encodeDisplayAndSave(frame, 0);
    }
    // send upperlayer callback for raw image (data or notify, not both)
    if((mHalCamCtrl->mDataCb) && (mHalCamCtrl->mMsgEnabled & CAMERA_MSG_RAW_IMAGE)){
        dataCb = mHalCamCtrl->mDataCb;
    } else {
        dataCb = NULL;
    }
    if((mHalCamCtrl->mNotifyCb) && (mHalCamCtrl->mMsgEnabled & CAMERA_MSG_RAW_IMAGE_NOTIFY)){
        notifyCb = mHalCamCtrl->mNotifyCb;
    } else {
        notifyCb = NULL;
    }
    if(mHalCamCtrl->mDataCb &&
       (mHalCamCtrl->mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE)) {
        //get picture failed. Give jpeg callback with NULL data
        //to the application to restore to preview mode
        jpgDataCb = mHalCamCtrl->mDataCb;
    } else {
        jpgDataCb = NULL;
    }
     mStopCallbackLock.unlock();
      ALOGE("%s X", __func__);
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

