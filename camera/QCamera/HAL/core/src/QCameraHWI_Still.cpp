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
    memset(&mJpegHandle, 0, sizeof(mJpegHandle));
    mJpegClientHandle = jpeg_open(&mJpegHandle);
    if(!mJpegClientHandle) {
        ALOGE("%s : jpeg_open did not work", __func__);
        //error
    }

    ALOGE("%s : X", __func__);

}

QCameraStream_SnapshotMain::~QCameraStream_SnapshotMain()
{
    int rc = 0;
    if(mJpegClientHandle > 0) {
        rc = mJpegHandle.close(mJpegClientHandle);
        ALOGE("%s: Jpeg closed, rc = %d, mJpegClientHandle = %x",
              __func__, rc, mJpegClientHandle);
        mJpegClientHandle = 0;
        memset(&mJpegHandle, 0, sizeof(mJpegHandle));
    }
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
#if 0
status_t QCameraStream_SnapshotMain::init()
{

    ALOGE("%s : E", __func__);
    status_t ret;
    mm_camera_op_mode_type_t op_mode=MM_CAMERA_OP_MODE_CAPTURE;
    ret = p_mm_ops->ops->set_parm (mCameraHandle, MM_CAMERA_PARM_OP_MODE, &op_mode);
    ALOGE("OP Mode Set");

    if(MM_CAMERA_OK != ret) {
        ALOGE("%s: X :set mode MM_CAMERA_OP_MODE_VIDEO err=%d\n", __func__, ret);
        return BAD_VALUE;
    }
    ret = QCameraStream::initStream(1);
    if (NO_ERROR!=ret) {
        ALOGE("%s E: can't init native camera snapshot main ch\n",__func__);
        return ret;
    }

    ALOGE("%s : X", __func__);
    return NO_ERROR;
}
#endif

#if 0
status_t QCameraStream_SnapshotMain::start()
{

    ALOGE("%s : E", __func__);
    return NO_ERROR;

    ALOGE("%s : X", __func__);
}
#endif

#if 0
void QCameraStream_SnapshotMain::stop()
{

    ALOGE("%s : E", __func__);
    status_t ret;
    //TODO - call snapshot stream off
    /*ret = streamOff(0);
    if(ret != MM_CAMERA_OK){
      ALOGE("%s : streamOff failed, ret = %d", __func__, ret);
    }*/
    ret = p_mm_ops->ops->stop_streams(mCameraHandle, mChannelId, 1, &mStreamId);
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
#endif

void QCameraStream_SnapshotMain::release()
{
    streamOff(0);
    deinitStream();

}

bool QCameraStream_SnapshotMain::isZSLMode()
{
   return (myMode & CAMERA_ZSL_MODE);
}

void QCameraStream_SnapshotMain::receiveCompleteJpegPicture(uint8_t* out_data, uint32_t data_size)
{
   ALOGE("%s: E", __func__);
   int msg_type = CAMERA_MSG_COMPRESSED_IMAGE;
   //camera_memory_t *encodedMem = NULL;
   camera_data_callback jpg_data_cb = NULL;
   if(mCurrentFrameEncoded != NULL){
       int i;
       ALOGE("%s: Calling buf done for snapshot buffer", __func__);
       //cam_evt_buf_done(mHalCamCtrl->mCameraId, mCurrentFrameEncoded);
       for(i = 0; i< mCurrentFrameEncoded->num_bufs; i++) {
           if(MM_CAMERA_OK != p_mm_ops->ops->qbuf(mCameraHandle, mChannelId, mCurrentFrameEncoded->bufs[i])){
               ALOGE("%s : Buf done failed for buffer[%d]", __func__, i);
           }

       }
   }
   msg_type = CAMERA_MSG_COMPRESSED_IMAGE;
   if(mHalCamCtrl->mDataCb && (mHalCamCtrl->mMsgEnabled & msg_type)){
       jpg_data_cb = mHalCamCtrl->mDataCb;
   } else {
       ALOGE("%s: JPEG callback was cancelled", __func__);
   }
   mHalCamCtrl->deinitExifData();

   if(mCurrentFrameEncoded){
       ALOGE("Free internal memory used for snapshot");
       free(mCurrentFrameEncoded);
       mCurrentFrameEncoded = NULL;
   }
   camera_memory_t *encodedMem = mHalCamCtrl->mGetMemory(mHalCamCtrl->mJpegMemory.fd[0], mHalCamCtrl->mJpegMemory.size, 1, mHalCamCtrl);
   memcpy(encodedMem->data, mHalCamCtrl->mJpegMemory.camera_memory[0]->data, data_size);
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
   if(mCurrentFrameEncoded){
	free(mCurrentFrameEncoded);
        mCurrentFrameEncoded = NULL;
   }
   //setSnapshotState(SNAPSHOT_STATE_ERROR); //TODO
   if(mHalCamCtrl->mDataCb != NULL){
	mHalCamCtrl->mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, NULL, 0, NULL, mHalCamCtrl->mCallbackCookie);
   }
   ALOGE("%s:  X", __func__);

}
static void snapshot_jpeg_cb(jpeg_job_status_t status, uint8_t thumbnailDroppedFlag, uint32_t client_hdl, uint32_t jobId, uint8_t* out_data, uint32_t data_size, void *userdata)
{
    ALOGE("%s: E", __func__);
    QCameraStream_SnapshotMain *pme = (QCameraStream_SnapshotMain *)userdata;
    if(pme == NULL){
       ALOGE("%s: pme is null", __func__);
    }
    if(status == JPEG_JOB_STATUS_ERROR) {
        if(pme != NULL) {
           pme->jpegErrorHandler(status);
        }
        ALOGE("Error event handled from jpeg");
        return;
    }
    if(thumbnailDroppedFlag) {
        ALOGE("%s : Error in thumbnail encoding", __func__);
        return;
    }
    if(pme != NULL) {
        //pme->mHalCamCtrl->mCancelPictureLock.lock();
        ALOGE("Completed issuing jpeg callback");
        /*if(!pme->isZSLMode()) {
            pme->mHalCamCtrl->cancelPicture();
            //pme->stop();
        }*/
        //pme->mHalCamCtrl->mCancelPictureLock.unlock();
        pme->receiveCompleteJpegPicture(out_data,data_size);
        if (!pme->isZSLMode()) {
            pme->mHalCamCtrl->cancelPicture();
        }
    }
    ALOGE("%s: X", __func__);
}

/*
* To-do: Gets the raw picture and sends to the jpeg interface
*/
status_t QCameraStream_SnapshotMain::receiveRawPicture(mm_camera_super_buf_t* recvd_frame){
    ALOGE("%s : E", __func__);
    int buf_index = 0;
    //common_crop_t crop;
    status_t rc = NO_ERROR;
    camera_notify_callback notifyCb;
    camera_data_callback dataCb, jpegDataCb;
    // for the jpeg
     mm_camera_super_buf_t* frame =
            (mm_camera_super_buf_t *)malloc(sizeof(mm_camera_super_buf_t));
     if (frame == NULL) {
         ALOGE("%s: Error allocating memory to save received_frame structure.", __func__);
         //cam_evt_buf_done(mHalCamCtrl->mCameraId, recvd_frame);
         p_mm_ops->ops->qbuf(mHalCamCtrl->mCameraId, mChannelId, mCurrentFrameEncoded->bufs[0]);
         p_mm_ops->ops->qbuf(mHalCamCtrl->mCameraId, mChannelId, mCurrentFrameEncoded->bufs[1]);
         return BAD_VALUE;
     }
     memcpy(frame, recvd_frame, sizeof(mm_camera_super_buf_t));
     rc = encodeData(frame);
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

status_t QCameraStream_SnapshotMain::encodeData(mm_camera_super_buf_t* recvd_frame){
    ALOGV("%s : E", __func__);
    status_t ret = NO_ERROR;
    mm_jpeg_job jpg_job;
    cam_ctrl_dimension_t dimension;
    mCurrentFrameEncoded = recvd_frame;
    mm_camera_buf_def_t *main_frame = NULL;
    mm_camera_buf_def_t *thumb_frame = NULL;
    src_image_buffer_info* main_buf_info = NULL;
    src_image_buffer_info* thumb_buf_info = NULL;
    p_mm_ops->ops->get_parm(p_mm_ops->camera_handle, MM_CAMERA_PARM_DIMENSION, &dimension);

    int i;

    main_frame = (mStreamId == recvd_frame->bufs[0]->stream_id)?recvd_frame->bufs[0]:recvd_frame->bufs[1];
    thumb_frame = (mHalCamCtrl->mStreamSnapThumb->mStreamId == recvd_frame->bufs[0]->stream_id) \
                            ?recvd_frame->bufs[0]:recvd_frame->bufs[1];

    if(main_frame == NULL){
       ALOGE("%s : Main frame is NULL", __func__);
       return ret;
    }

    int jpeg_quality = mHalCamCtrl->getJpegQuality();
    if (jpeg_quality <= 0) {
        jpeg_quality = 85;
    }

    memset(&jpg_job, 0, sizeof(mm_jpeg_job));
    jpg_job.job_type = JPEG_JOB_TYPE_ENCODE;
    jpg_job.encode_job.userdata = this;
    jpg_job.encode_job.jpeg_cb = snapshot_jpeg_cb;
    jpg_job.encode_job.encode_parm.exif_data = mHalCamCtrl->getExifData();
    jpg_job.encode_job.encode_parm.exif_numEntries = mHalCamCtrl->getExifTableNumEntries();
    jpg_job.encode_job.encode_parm.rotation = mHalCamCtrl->getJpegRotation();
    ALOGV("%s: jpeg rotation is set to %d", __func__, jpg_job.encode_job.encode_parm.rotation);
    jpg_job.encode_job.encode_parm.buf_info.src_imgs.src_img_num = recvd_frame->num_bufs;

    // fill in the src_img info
    //main img
    main_buf_info = &jpg_job.encode_job.encode_parm.buf_info.src_imgs.src_img[JPEG_SRC_IMAGE_TYPE_MAIN];
    main_buf_info->type = JPEG_SRC_IMAGE_TYPE_MAIN;
    main_buf_info->color_format = MM_JPEG_COLOR_FORMAT_YCRCBLP_H2V2;
    main_buf_info->quality = jpeg_quality;
    main_buf_info->src_image[0].fd = main_frame->fd;
    main_buf_info->src_image[0].buf_vaddr = (uint8_t*) main_frame->buffer;
    main_buf_info->src_dim.width = mWidth;
    main_buf_info->src_dim.height = mHeight;
    main_buf_info->out_dim.width = mHalCamCtrl->mPictureWidth;
    main_buf_info->out_dim.height = mHalCamCtrl->mPictureHeight;
    ALOGE("%s : Main Image :Input Dimension %d x %d output Dimension = %d X %d",
          __func__, mWidth, mHeight,mHalCamCtrl->mPictureWidth,mHalCamCtrl->mPictureHeight);

    main_buf_info->crop.width = mWidth;
    main_buf_info->crop.height = mHeight;
    main_buf_info->crop.offset_x = 0;
    main_buf_info->crop.offset_y = 0;
    main_buf_info->img_fmt = JPEG_SRC_IMAGE_FMT_YUV;
    main_buf_info->num_bufs = 1;
    main_buf_info->src_image[0].offset = mFrameOffsetInfo;
    ALOGE("%s : setting main image offset info, len = %d, offset = %d", __func__, mFrameOffsetInfo.mp[0].len, mFrameOffsetInfo.mp[0].offset);

    if (thumb_frame) {
        /* fill in thumbnail src img encode param */
        thumb_buf_info = &jpg_job.encode_job.encode_parm.buf_info.src_imgs.src_img[JPEG_SRC_IMAGE_TYPE_THUMB];
        thumb_buf_info->type = JPEG_SRC_IMAGE_TYPE_THUMB;
        thumb_buf_info->color_format = MM_JPEG_COLOR_FORMAT_YCRCBLP_H2V2; //TODO
        thumb_buf_info->quality = jpeg_quality;
       /* thumb_buf_info->src_dim.width = dimension.ui_thumbnail_width;
        thumb_buf_info->src_dim.height = dimension.ui_thumbnail_height;
        thumb_buf_info->out_dim.width = dimension.ui_thumbnail_width;
        thumb_buf_info->out_dim.height = dimension.ui_thumbnail_height;
        thumb_buf_info->crop.width = dimension.ui_thumbnail_width;
        thumb_buf_info->crop.height = dimension.ui_thumbnail_height;
        ALOGE("%s : the thumbnail info from dimension is %d x %d", __func__, dimension.ui_thumbnail_width, dimension.ui_thumbnail_height); */
        if(!isZSLMode()) {
            thumb_buf_info->src_dim.width = mHalCamCtrl->mStreamSnapThumb->mWidth;
            thumb_buf_info->src_dim.height = mHalCamCtrl->mStreamSnapThumb->mHeight;
            thumb_buf_info->crop.width = mHalCamCtrl->mStreamSnapThumb->mWidth;
            thumb_buf_info->crop.height = mHalCamCtrl->mStreamSnapThumb->mHeight;
        }else{
            thumb_buf_info->src_dim.width = mHalCamCtrl->mStreamDisplay->mWidth;
            thumb_buf_info->src_dim.height = mHalCamCtrl->mStreamDisplay->mHeight;
            thumb_buf_info->crop.width = mHalCamCtrl->mStreamDisplay->mWidth;
            thumb_buf_info->crop.height = mHalCamCtrl->mStreamDisplay->mHeight;

        }

        thumb_buf_info->out_dim.width = mHalCamCtrl->thumbnailWidth;
        thumb_buf_info->out_dim.height = mHalCamCtrl->thumbnailHeight;

        ALOGE("%s : Thumanail :Input Dimension %d x %d output Dimension = %d X %d",
          __func__, thumb_buf_info->src_dim.width, thumb_buf_info->src_dim.height,
              thumb_buf_info->out_dim.width,thumb_buf_info->out_dim.height);

        thumb_buf_info->crop.offset_x = 0;
        thumb_buf_info->crop.offset_y = 0;
        thumb_buf_info->img_fmt = JPEG_SRC_IMAGE_FMT_YUV;
        thumb_buf_info->num_bufs = 1;
        thumb_buf_info->src_image[0].fd = thumb_frame->fd;
        thumb_buf_info->src_image[0].buf_vaddr = (uint8_t*) thumb_frame->buffer;
        if(!isZSLMode()) {
            thumb_buf_info->src_image[0].offset = mHalCamCtrl->mStreamSnapThumb->mFrameOffsetInfo;
        }else{
            thumb_buf_info->src_image[0].offset = mHalCamCtrl->mStreamDisplay->mFrameOffsetInfo;
        }
        //thumb_buf_info->src_image[0].offset = mFrameOffsetInfo;
         ALOGE("%s : setting thumb image offset info, len = %d, offset = %d", __func__, mHalCamCtrl->mStreamSnapThumb->mFrameOffsetInfo.mp[0].len, mHalCamCtrl->mStreamSnapThumb->mFrameOffsetInfo.mp[0].offset);
    } else if(mHalCamCtrl->mStreamSnapThumb->mWidth &&
              mHalCamCtrl->mStreamSnapThumb->mHeight) { /*thumbnail is required, not YUV thumbnail, borrow main image*/
        jpg_job.encode_job.encode_parm.buf_info.src_imgs.src_img_num += 1;

        // fill in the src_img info
        //Tb img
        thumb_buf_info = &jpg_job.encode_job.encode_parm.buf_info.src_imgs.src_img[JPEG_SRC_IMAGE_TYPE_THUMB];

        thumb_buf_info->type = JPEG_SRC_IMAGE_TYPE_THUMB;
        thumb_buf_info->color_format = MM_JPEG_COLOR_FORMAT_YCRCBLP_H2V2;
        thumb_buf_info->quality = jpeg_quality;
        thumb_buf_info->src_image[0].fd = main_frame->fd;
        thumb_buf_info->src_image[0].buf_vaddr = (uint8_t*) main_frame->buffer;
        thumb_buf_info->src_dim.width = dimension.picture_width;
        thumb_buf_info->src_dim.height = dimension.picture_height;
        thumb_buf_info->out_dim.width = mHalCamCtrl->mStreamSnapThumb->mWidth;
        thumb_buf_info->out_dim.height = mHalCamCtrl->mStreamSnapThumb->mHeight;
        thumb_buf_info->crop.width = dimension.picture_width; /*revisit: shall use crop info*/
        thumb_buf_info->crop.height = dimension.picture_height;
        thumb_buf_info->crop.offset_x = 0;
        thumb_buf_info->crop.offset_y = 0;
        thumb_buf_info->img_fmt = JPEG_SRC_IMAGE_FMT_YUV;
        thumb_buf_info->num_bufs = 1;
        thumb_buf_info->src_image[0].offset = mFrameOffsetInfo;
    }

    if(mHalCamCtrl->mStreamSnapThumb->mWidth == 0 ||
              mHalCamCtrl->mStreamSnapThumb->mHeight == 0) {
         /*no thumbnail required*/
        jpg_job.encode_job.encode_parm.buf_info.src_imgs.src_img_num = 1;
    }

   //fill in the sink img info
    jpg_job.encode_job.encode_parm.buf_info.sink_img.fd = mHalCamCtrl->mJpegMemory.fd[0];
    jpg_job.encode_job.encode_parm.buf_info.sink_img.buf_len = mHalCamCtrl->mJpegMemory.size;
    jpg_job.encode_job.encode_parm.buf_info.sink_img.buf_vaddr = (uint8_t*) mHalCamCtrl->mJpegMemory.camera_memory[0]->data;

    if (mJpegClientHandle > 0) {
        ret = mJpegHandle.start_job(mJpegClientHandle, &jpg_job, &jpeg_jobId);
    } else {
        ALOGE("%s: Error: bug here, mJpegClientHandle is 0", __func__);
    }
  #if 0
    status_t ret = NO_ERROR;
    cam_ctrl_dimension_t dimension;
    cam_point_t main_crop_offset;
    cam_point_t thumb_crop_offset;
    int width, height;
    uint8_t *thumbnail_buf;
    uint32_t thumbnail_fd;
    uint8_t hw_encode = true;
    int mNuberOfVFEOutputs = 0;
    mm_jpeg_encode_params encode_params;


 else {  /*not busy and new buffer (first job)*/
        ALOGD("%s: JPG Idle and  first frame.", __func__);
        // For full-size live shot, use mainimage to generate thumbnail
        if (isFullSizeLiveshot()){
            postviewframe = recvd_frame->snapshot.main.frame;
        } else {
            postviewframe = recvd_frame->snapshot.thumbnail.frame;
        }
        mainframe = recvd_frame->snapshot.main.frame;
        cam_config_get_parm(mHalCamCtrl->mCameraId, MM_CAMERA_PARM_DIMENSION, &dimension);
        ALOGD("%s: main_fmt =%d, tb_fmt =%d", __func__, dimension.main_img_format, dimension.thumb_format);
        dimension.orig_picture_dx = mPictureWidth;
        dimension.orig_picture_dy = mPictureHeight;

        if(!mDropThumbnail) {
            if(isZSLMode()) {
                ALOGI("Setting input thumbnail size to previewWidth= %d   previewheight= %d in ZSL mode",
                      mHalCamCtrl->mPreviewWidth, mHalCamCtrl->mPreviewHeight);
                dimension.thumbnail_width = width = mHalCamCtrl->mPreviewWidth;
                dimension.thumbnail_height = height = mHalCamCtrl->mPreviewHeight;
            } else {
                dimension.thumbnail_width = width = mThumbnailWidth;
                dimension.thumbnail_height = height = mThumbnailHeight;
            }
        } else {
            dimension.thumbnail_width = width = 0;
            dimension.thumbnail_height = height = 0;
        }
        dimension.main_img_format = mPictureFormat;
        dimension.thumb_format = mThumbnailFormat;

        /*TBD: Move JPEG handling to the mm-camera library */
        ALOGD("Setting callbacks, initializing encoder and start encoding.");
        ALOGD(" Passing my obj: %x", (unsigned int) this);
        set_callbacks(snapshot_jpeg_fragment_cb, snapshot_jpeg_cb, this,
                      mHalCamCtrl->mJpegMemory.camera_memory[0]->data, &mJpegOffset);

        if (isLiveSnapshot() || isFullSizeLiveshot()) {
            /* determine the target type */
            ret = cam_config_get_parm(mCameraId,MM_CAMERA_PARM_VFE_OUTPUT_ENABLE,
                                      &mNuberOfVFEOutputs);
            if (ret != MM_CAMERA_OK) {
                ALOGE("get parm MM_CAMERA_PARM_VFE_OUTPUT_ENABLE  failed");
                ret = BAD_VALUE;
            }
            /* VFE 2x has hardware limitation:
              * It can't support concurrent
    * video encoding and jpeg encoding
     * So switch to software for liveshot
    */
            if (mNuberOfVFEOutputs == 1)
                hw_encode = false;
        }
        ALOGD("%s: hw_encode: %d\n",__func__, hw_encode);

        if(omxJpegStart(hw_encode) != NO_ERROR){
            ALOGE("Error In omxJpegStart!!! Return");
            ret = FAILED_TRANSACTION;
            goto end;
        }
        if (mHalCamCtrl->getJpegQuality())
            mm_jpeg_encoder_setMainImageQuality(mHalCamCtrl->getJpegQuality());
        else
            mm_jpeg_encoder_setMainImageQuality(85);
        ALOGE("%s: Dimension to encode: main: %dx%d thumbnail: %dx%d", __func__,
              dimension.orig_picture_dx, dimension.orig_picture_dy,
              dimension.thumbnail_width, dimension.thumbnail_height);
        /*TBD: Pass 0 as cropinfo for now as v4l2 doesn't provide
         cropinfo. It'll be changed later.*/
        memset(&crop,0,sizeof(common_crop_t));
        memset(&main_crop_offset,0,sizeof(cam_point_t));
        memset(&thumb_crop_offset,0,sizeof(cam_point_t));
        /* Setting crop info */
        /*Main image*/
        crop.in2_w=mCrop.snapshot.main_crop.width;// dimension.picture_width
        crop.in2_h=mCrop.snapshot.main_crop.height;// dimension.picture_height;
        if (!mJpegDownscaling) {
            crop.out2_w = mPictureWidth;
            crop.out2_h = mPictureHeight;
        } else {
            crop.out2_w = mActualPictureWidth;
            crop.out2_h = mActualPictureHeight;
            if (!crop.in2_w || !crop.in2_h) {
                crop.in2_w = mPictureWidth;
                crop.in2_h = mPictureHeight;
            }
        }
        main_crop_offset.x=mCrop.snapshot.main_crop.left;
        main_crop_offset.y=mCrop.snapshot.main_crop.top;
        /*Thumbnail image*/
        crop.in1_w=mCrop.snapshot.thumbnail_crop.width; //dimension.thumbnail_width;
        crop.in1_h=mCrop.snapshot.thumbnail_crop.height; // dimension.thumbnail_height;
        if(isLiveSnapshot() || isFullSizeLiveshot()) {
            crop.out1_w= mHalCamCtrl->thumbnailWidth;
            crop.out1_h=  mHalCamCtrl->thumbnailHeight;
            ALOGD("Thumbnail width= %d  height= %d for livesnapshot", crop.out1_w, crop.out1_h);
        } else {
            crop.out1_w = width;
            crop.out1_h = height;
        }
        thumb_crop_offset.x=mCrop.snapshot.thumbnail_crop.left;
        thumb_crop_offset.y=mCrop.snapshot.thumbnail_crop.top;
        //update exif parameters in HAL
        mHalCamCtrl->initExifData();
        /*Fill in the encode parameters*/
        encode_params.dimension = (const cam_ctrl_dimension_t *)&dimension;
        //if (!isFullSizeLiveshot()) {
        encode_params.thumbnail_buf = (uint8_t *)postviewframe->buffer;
        encode_params.thumbnail_fd = postviewframe->fd;
        encode_params.thumbnail_offset = postviewframe->phy_offset;
        encode_params.thumb_crop_offset = &thumb_crop_offset;
        //}
        encode_params.snapshot_buf = (uint8_t *)mainframe->buffer;
        encode_params.snapshot_fd = mainframe->fd;
        encode_params.snapshot_offset = mainframe->phy_offset;
        encode_params.scaling_params = &crop;
        encode_params.exif_data = mHalCamCtrl->getExifData();
        encode_params.exif_numEntries = mHalCamCtrl->getExifTableNumEntries();
        if (isLiveSnapshot() && !isFullSizeLiveshot())
            encode_params.a_cbcroffset = mainframe->cbcr_off;
        else
            encode_params.a_cbcroffset = -1;
        encode_params.main_crop_offset = &main_crop_offset;
        if (mDropThumbnail)
            encode_params.hasThumbnail = 0;
        else
            encode_params.hasThumbnail = 1;
        encode_params.thumb_crop_offset = &thumb_crop_offset;
        encode_params.main_format = dimension.main_img_format;
        encode_params.thumbnail_format = dimension.thumb_format;
        if (!omxJpegEncode(&encode_params)){
            ALOGE("%s: Failure! JPEG encoder returned error.", __func__);
            ret = FAILED_TRANSACTION;
            goto end;
        }

    /* Save the pointer to the frame sent for encoding. we'll need it to
    tell kernel that we are done with the frame.*/
    mCurrentFrameEncoded = recvd_frame;
    setSnapshotState(SNAPSHOT_STATE_JPEG_ENCODING);
    }

    end:
    ALOGD("%s: X", __func__);
#endif
    ALOGV("%s : X", __func__);
    return ret;

}

status_t QCameraStream_SnapshotMain::initStream(int no_cb_needed)
{
    status_t ret = NO_ERROR;

    if(isZSLMode()) {
        mNumBuffers = mHalCamCtrl->mStreamDisplay->mNumBuffers;
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
#if 0
    mHalCamCtrl->setJpegRotation(isZSLMode());
    if(!isZSLMode())
        rotation = mHalCamCtrl->getJpegRotation();
    else
        rotation = 0;
    if(rotation != dim->rotation) {
        dim->rotation = rotation;
    }
    if(isLiveSnapshot()) {
        ret = p_mm_ops->ops->set_parm(mCameraHandle, MM_CAMERA_PARM_DIMENSION, dim);
    }
#endif
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
       // mHalCamCtrl->releaseHeapMem(&mHalCamCtrl->mJpegMemory);
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
////Thumbnail
#if 0
        num_planes = 2;
        planes[0] = dim->thumb_frame_offset.mp[0].len;
        planes[1] = dim->thumb_frame_offset.mp[1].len;
        frame_len = planes[0] + planes[1];
        if (!isFullSizeLiveshot()) {
    	    y_off = dim->thumb_frame_offset.mp[0].offset;
                cbcr_off = dim->thumb_frame_offset.mp[1].offset;
    	    ALOGE("%s: thumbnail: rotation = %d, yoff = %d, cbcroff = %d, size = %d, width = %d, height = %d",
    		__func__, dim->rotation, y_off, cbcr_off, frame_len,
    		dim->thumbnail_width, dim->thumbnail_height);

    	    if (mHalCamCtrl->initHeapMem(&mHalCamCtrl->mThumbnailMemory, num_of_buf,
    		    frame_len, y_off, cbcr_off, MSM_PMEM_THUMBNAIL, &mPostviewStreamBuf,
    		    &reg_buf.snapshot.thumbnail, num_planes, planes) < 0) {
    	        ret = NO_MEMORY;
                    mHalCamCtrl->releaseHeapMem(&mHalCamCtrl->mSnapshotMemory);
                    mHalCamCtrl->releaseHeapMem(&mHalCamCtrl->mJpegMemory);
    	        goto end;
    	    }
        }
#endif
        /* register the streaming buffers for the channel*/


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
// to notify output buffer and other buffers.
/*status_t QCameraStream_SnapshotMain::doHdrProcessing()
{
    status_t rc = NO_ERROR;
    cam_sock_packet_t packet;
    int i;
    memset(&packet, 0, sizeof(cam_sock_packet_t));
    packet.msg_type = CAM_SOCK_MSG_TYPE_HDR_START;
    packet.payload.hdr_pkg.cookie = (unisgned long) this;
    packet.payload.hdr_pkg.num_hdr_frames = mHdrInfo.num_frame;
    ALOGI("%s num frames = %d ", __func__, mHdrInfo.num_frame);
    for (i = 0; i < mHdrInfo.num_frame; i++) {
        packet.payload.hdr_pkg.hdr_main_idx[i] =mHdrInfo.recvd_frame[i]->snapshot.main.idx;
        packet.payload.hdr_pkg.hdr_thm_idx[i] = mHdrInfo.recvd_frame[i]->snapshot.thumbnail.idx;
        packet.payload.hdr_pkg.exp[i] = mHdrInfo.exp[i];
        ALOGI("%s Adding buffer M %d T %d Exp %d into hdr pkg ", __func__,
              packet.payload.hdr_pkg.hdr_main_idx[i],
              packet.payload.hdr_pkg.hdr_thm_idx[i],
              packet.payload.hdr_pkg.exp[i]);
    }
    if (p_mm_ops->ops->cam_ops_sendmsg(mCameraId, &packet, sizeof(packet), 0) <= 0) {
        LOGE("%s: sending start hdr msg failed", __func__);
        rc= FAILED_TRANSACTION;
    }
    return rc;

}*/

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
     /*for (i =0; i< 2; i++) {
        if (rc[i] != NO_ERROR)
        {
            ALOGE("%s: Error while encoding/displaying/saving image", __func__);
            if (frame) {
                qbuf(mCameraId,  mHdrInfo.recvd_frame[i]);
            }
            if (dataCb) {
                dataCb(CAMERA_MSG_RAW_IMAGE, mHalCamCtrl->mSnapshotMemory.camera_memory[0],
                       1, NULL, mHalCamCtrl->mCallbackCookie);
            }
            if (notifyCb) {
                notifyCb(CAMERA_MSG_RAW_IMAGE_NOTIFY, 0, 0, mHalCamCtrl->mCallbackCookie);
            }
            if (jpgDataCb) {
                jpgDataCb(CAMERA_MSG_COMPRESSED_IMAGE,
                          NULL, 0, NULL,
                          mHalCamCtrl->mCallbackCookie);
            }
            if ( mHdrInfo.recvd_frame[i] != NULL) {
                free( mHdrInfo.recvd_frame[i]);
                mHdrInfo.recvd_frame[i] = NULL;
            }
        }
     }
     for (i = 2; i <mHdrInfo.num_raw_received; i++ ) {
         if (mHdrInfo.recvd_frame[i]) {
             qbuf(mHalCamCtrl->mCameraId,  mHdrInfo.recvd_frame[i]);
             free( mHdrInfo.recvd_frame[i]);
             mHdrInfo.recvd_frame[i] = NULL;
         }
     } */
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

#if 0
status_t QCameraStream_SnapshotThumbnail::init()
{
    status_t ret;
    ret = QCameraStream::initStream(1);
    if (NO_ERROR!=ret) {
        ALOGE("%s E: can't init native camera snapshot main ch\n",__func__);
        return ret;
    }
    return NO_ERROR;
}
#endif

//status_t QCameraStream_SnapshotThumbnail::start()
//{
//   return NO_ERROR;
//}

#if 0
void QCameraStream_SnapshotThumbnail::stop()
{
    status_t ret;
    /*ret = streamOff(0);
    if(ret != MM_CAMERA_OK){
      ALOGE("%s : streamOff failed, ret = %d", __func__, ret);
    }*/
    ret = p_mm_ops->ops->stop_streams(mCameraHandle, mChannelId, 1, &mStreamId);
    if(ret != MM_CAMERA_OK){
      ALOGE("%s : stop_streams failed, ret = %d", __func__, ret);
    }
    ret= QCameraStream::deinitStream();
    ALOGE(": %s : De init Channel",__func__);
    if(ret != MM_CAMERA_OK) {
        ALOGE("%s:Deinit preview channel failed=%d\n", __func__, ret);
    }
}
#endif
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

