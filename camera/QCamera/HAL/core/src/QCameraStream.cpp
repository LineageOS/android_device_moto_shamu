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

#define ALOG_NDEBUG 0
#define ALOG_NIDEBUG 0
#define LOG_TAG __FILE__
#include <utils/Log.h>

#include "QCameraHWI.h"
#include "QCameraStream.h"

/* QCameraStream class implementation goes here*/
/* following code implement the control logic of this class*/

namespace android {

// ---------------------------------------------------------------------------
// QCameraStream
// ---------------------------------------------------------------------------

void stream_cb_routine(mm_camera_super_buf_t *bufs,
                       void *userdata)
{
    ALOGE("%s E ", __func__);
    QCameraStream *p_obj=(QCameraStream*) userdata;
    ALOGE("DEBUG4:ExtMode:%d,streamid:%d",p_obj->mExtImgMode,bufs->bufs[0]->stream_id);
    switch(p_obj->mExtImgMode)
    {
    case MM_CAMERA_PREVIEW:
                ALOGE("%s : callback for MM_CAMERA_PREVIEW", __func__);
                ((QCameraStream_preview *)p_obj)->processPreviewFrame(bufs);
                break;
        case MM_CAMERA_VIDEO:
                ((QCameraStream_record *)p_obj)->processRecordFrame(bufs);
                break;
    case MM_CAMERA_SNAPSHOT_MAIN:
                if(p_obj->mHalCamCtrl->getHDRMode()) {
                    ALOGE("%s: Skipping Q Buf for HDR mode",__func__);
                    break;
                }

                ALOGE("%s : callback for MM_CAMERA_SNAPSHOT_MAIN", __func__);
                break;
         case MM_CAMERA_SNAPSHOT_THUMBNAIL:
                break;
         default:
                break;

    }
    ALOGE("%s X ", __func__);
}

void QCameraStream::dataCallback(mm_camera_super_buf_t *bufs)
{
}


void QCameraStream::setResolution(mm_camera_dimension_t *res)
{
    mWidth = res->width;
    mHeight = res->height;
}
bool QCameraStream::isResolutionSame(mm_camera_dimension_t *res)
{
    if (mWidth != res->width || mHeight != res->height)
        return false;
    else
        return true;
}
void QCameraStream::getResolution(mm_camera_dimension_t *res)
{
    res->width = mWidth;
    res->height = mHeight;
}
int32_t QCameraStream::streamOn()
{
   status_t rc=NO_ERROR;
   mm_camera_stream_config_t stream_config;
   ALOGE("%s: mActive = %d, streamid = %d, image_mode = %d",__func__, mActive, mStreamId, mExtImgMode);
   if(mActive){
       ALOGE("%s: Stream:%d is already active",
            __func__,mStreamId);
       return rc;
   }
   if (mInit == true) {
       /* this is the restart case */
       memset(&stream_config, 0, sizeof(mm_camera_stream_config_t));
       stream_config.fmt.fmt=(cam_format_t)mFormat;
       stream_config.fmt.meta_header=MM_CAMEAR_META_DATA_TYPE_DEF;
       stream_config.fmt.width=mWidth;
       stream_config.fmt.height=mHeight;
       stream_config.fmt.rotation = 0;
       ALOGE("<DEBUG>::%s: Width :%d Height:%d Format:%d",__func__,mWidth,mHeight,mFormat);
       stream_config.num_of_bufs=mNumBuffers;
       stream_config.need_stream_on=true;
       rc=p_mm_ops->ops->config_stream(mCameraHandle,
                                 mChannelId,
                                 mStreamId,
                                 &stream_config);
       ALOGE("%s: config_stream, rc = %d", __func__, rc);
   }
   rc = p_mm_ops->ops->start_streams(mCameraHandle,
                              mChannelId,
                              1,
                              &mStreamId);
   if(rc==NO_ERROR)
       mActive = true;
   ALOGE("%s: X, mActive = %d, mInit = %d, streamid = %d, image_mode = %d",
         __func__, mActive, mInit, mStreamId, mExtImgMode);
   return rc;
}

int32_t QCameraStream::streamOff(bool isAsyncCmd)
{
    status_t rc=NO_ERROR;
    ALOGE("%s: mActive = %d, streamid = %d, image_mode = %d",__func__, mActive, mStreamId, mExtImgMode);
    if(!mActive) {
        ALOGE("%s: Stream:%d is not active",
              __func__,mStreamId);
        return rc;
    }

    rc = p_mm_ops->ops->stop_streams(mCameraHandle,
                              mChannelId,
                              1,
                              &mStreamId);

    mActive=false;
    ALOGE("%s: X, mActive = %d, mInit = %d, streamid = %d, image_mode = %d",
          __func__, mActive, mInit, mStreamId, mExtImgMode);
    return rc;
}

/* initialize a streaming channel*/
status_t QCameraStream::initStream(int no_stream_cb)
{

    int rc = MM_CAMERA_OK;
    status_t ret = NO_ERROR;
    mm_camera_op_mode_type_t op_mode=MM_CAMERA_OP_MODE_VIDEO;
    int i;
    int setJpegRotation = 0;
    int rotation = mHalCamCtrl->getJpegRotation();
    mm_camera_stream_config_t stream_config;

    ALOGE("%s: E, mActive = %d, mInit = %d, streamid = %d, image_mode = %d",
          __func__, mActive, mInit, mStreamId, mExtImgMode);

    if(mInit == true) {
        ALOGE("%s: alraedy initted, mActive = %d, mInit = %d, streamid = %d, image_mode = %d",
              __func__, mActive, mInit, mStreamId, mExtImgMode);
        return rc;
    }
    /***********Allocate Stream**************/
    if (no_stream_cb > 0) {
        rc=p_mm_ops->ops->add_stream(mCameraHandle,
                            mChannelId,
                            NULL,
                            NULL,
                            mExtImgMode,
                            0/*sensor_idx*/);
    } else {
        rc=p_mm_ops->ops->add_stream(mCameraHandle,
                            mChannelId,
                            stream_cb_routine,
                            (void *)this,
                            mExtImgMode,
                            0/*sensor_idx*/);
    }

    if (rc < 0) {
        ALOGE("%s: err in add_stream, mActive = %d, mInit = %d, streamid = %d, image_mode = %d",
              __func__, mActive, mInit, mStreamId, mExtImgMode);
       goto error1;
    }

    mStreamId=rc;
    ALOGE("%s: add_stream done, mActive = %d, streamid = %d, image_mode = %d",__func__, mActive, mStreamId, mExtImgMode);

    /***********Config Stream*****************/
    switch(mExtImgMode)
    {
	case MM_CAMERA_PREVIEW:
            //Get mFormat
            rc= p_mm_ops->ops->get_parm(p_mm_ops->camera_handle,
                                MM_CAMERA_PARM_PREVIEW_FORMAT,
                                &mFormat);
            if (MM_CAMERA_OK != rc) {
                ALOGE("%s: error - can't get preview format!", __func__);
                ALOGE("%s: X", __func__);
                goto error2;
            }
            break;
        case MM_CAMERA_VIDEO:
            break;
        case MM_CAMERA_SNAPSHOT_MAIN:
            setJpegRotation = 1;
            break;
        case MM_CAMERA_SNAPSHOT_THUMBNAIL:
            setJpegRotation = 1;
            break;
        default:
            break;
    }

    memset(&stream_config, 0, sizeof(mm_camera_stream_config_t));
    stream_config.fmt.fmt=(cam_format_t)mFormat;
    stream_config.fmt.meta_header=MM_CAMEAR_META_DATA_TYPE_DEF;
    stream_config.fmt.width=mWidth;
    stream_config.fmt.height=mHeight;
    if(setJpegRotation){
       ALOGE("%s : setting the stream_config rotation for main and thumbnail", __func__);
       stream_config.fmt.rotation = rotation;
    }
    ALOGE("%s: imageMode = %d Width :%d Height:%d Format:%d",__func__,mExtImgMode,mWidth,mHeight,mFormat);
    stream_config.num_of_bufs=mNumBuffers;
    stream_config.need_stream_on=true;
    rc=p_mm_ops->ops->config_stream(mCameraHandle,
                              mChannelId,
                              mStreamId,
                              &stream_config);
    if(MM_CAMERA_OK != rc) {
        ALOGE("%s: err in config_stream, mActive = %d, streamid = %d, image_mode = %d",__func__, mActive, mStreamId, mExtImgMode);
        goto error2;
    }

    goto end;


error2:
    p_mm_ops->ops->del_stream(mCameraHandle,mChannelId,
                              mStreamId);

error1:
    return BAD_VALUE;
end:
    ALOGE("Setting mInit to true");
    mInit=true;
    ALOGE("%s: X, mActive = %d, streamid = %d, image_mode = %d",__func__, mActive, mStreamId, mExtImgMode);
    return NO_ERROR;

}

status_t QCameraStream::deinitStream()
{

    int rc = MM_CAMERA_OK;

    ALOGE("%s: E, mActive = %d, mInit = %d, streamid = %d, image_mode = %d",__func__, mActive, mInit, mStreamId, mExtImgMode);

    if(mInit == false) {
        /* stream has not been initted. nop */
        if (mStreamId > 0) {
            ALOGE("%s: bug. mStreamId = %d, mInit = %d", __func__, mStreamId, mInit);
            rc = -1;
        }
        return rc;
    }
    rc= p_mm_ops->ops->del_stream(mCameraHandle,mChannelId,
                              mStreamId);

    ALOGV("%s: X, Stream = %d\n", __func__, mStreamId);
    mInit = false;
    mStreamId = 0;
    ALOGE("%s: X, mActive = %d, mInit = %d, streamid = %d, image_mode = %d",
          __func__, mActive, mInit, mStreamId, mExtImgMode);
    return NO_ERROR;
}

status_t QCameraStream::setMode(int enable) {
  ALOGE("%s: E, mActive = %d, streamid = %d, image_mode = %d",__func__, mActive, mStreamId, mExtImgMode);
  if (enable) {
      myMode = (camera_mode_t)(myMode | CAMERA_ZSL_MODE);
  } else {
      myMode = (camera_mode_t)(myMode & ~CAMERA_ZSL_MODE);
  }
  return NO_ERROR;
}

status_t QCameraStream::setFormat()
{
    ALOGE("%s: E, mActive = %d, streamid = %d, image_mode = %d",__func__, mActive, mStreamId, mExtImgMode);

    char mDeviceName[PROPERTY_VALUE_MAX];
    property_get("ro.product.device",mDeviceName," ");

    ALOGE("%s: X",__func__);
    return NO_ERROR;
}

QCameraStream::QCameraStream (){
    mInit = false;
    mActive = false;
    /* memset*/
    memset(&mCrop, 0, sizeof(mm_camera_rect_t));
}

QCameraStream::QCameraStream(uint32_t CameraHandle,
                        uint32_t ChannelId,
                        uint32_t Width,
                        uint32_t Height,
                        uint32_t Format,
                        uint8_t NumBuffers,
                        mm_camera_vtbl_t *mm_ops,
                        mm_camera_img_mode imgmode,
                        camera_mode_t mode)
              :myMode(mode)
{
    mInit = false;
    mActive = false;

    mCameraHandle=CameraHandle;
    mChannelId=ChannelId;
    mWidth=Width;
    mHeight=Height;
    mFormat=Format;
    mNumBuffers=NumBuffers;
    p_mm_ops=mm_ops;
    mExtImgMode=imgmode;

    /* memset*/
    memset(&mCrop, 0, sizeof(mm_camera_rect_t));
}

QCameraStream::~QCameraStream () {;}

void QCameraStream::release() {
    return;
}

void QCameraStream::setHALCameraControl(QCameraHardwareInterface* ctrl) {

    /* provide a frame data user,
    for the  queue monitor thread to call the busy queue is not empty*/
    mHalCamCtrl = ctrl;
}

}; // namespace android
