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

#ifndef ANDROID_HARDWARE_QCAMERA_STREAM_H
#define ANDROID_HARDWARE_QCAMERA_STREAM_H


#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <pthread.h>
#include <semaphore.h>


#include "QCameraHWI.h"
#include "QCameraHWI_Mem.h"
#include "mm_camera_interface.h"
#include "mm_jpeg_interface.h"

extern "C" {
#define DEFAULT_STREAM_WIDTH 320
#define DEFAULT_STREAM_HEIGHT 240
#define DEFAULT_LIVESHOT_WIDTH 2592
#define DEFAULT_LIVESHOT_HEIGHT 1944

#define MM_CAMERA_CH_PREVIEW_MASK    (0x01 << MM_CAMERA_CH_PREVIEW)
#define MM_CAMERA_CH_VIDEO_MASK      (0x01 << MM_CAMERA_CH_VIDEO)
#define MM_CAMERA_CH_SNAPSHOT_MASK   (0x01 << MM_CAMERA_CH_SNAPSHOT)

} /* extern C*/

typedef struct snap_hdr_record_t_ {
    bool hdr_on;
    int num_frame;
    int num_raw_received;
    /*in terms of 2^*(n/6), e.g 6 means (1/2)x, whole 12 means 4x*/
    int exp[MAX_HDR_EXP_FRAME_NUM];
    mm_camera_super_buf_t *recvd_frame[MAX_HDR_EXP_FRAME_NUM];
} snap_hdr_record_t;

typedef struct {
    jpeg_job_status_t status;
    uint8_t thumbnailDroppedFlag;
    uint32_t client_hdl;
    uint32_t jobId;
    uint8_t* out_data;
    uint32_t data_size;
    mm_camera_super_buf_t* src_frame;
} camera_jpeg_data_t;

typedef struct {
    mm_camera_super_buf_t* src_frame;
    void* userdata;
} camera_jpeg_encode_cookie_t;


namespace android {

class QCameraHardwareInterface;

class QCameraStream { //: public virtual RefBase

public:
    bool mInit;
    bool mActive;

    uint32_t mCameraHandle;
    uint32_t mChannelId;
    uint32_t mStreamId;
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mFormat;
    uint8_t mNumBuffers;
    mm_camera_frame_len_offset mFrameOffsetInfo;
    mm_camera_vtbl_t *p_mm_ops;
    mm_camera_img_mode mExtImgMode;
    void setResolution(mm_camera_dimension_t *res);
    bool isResolutionSame(mm_camera_dimension_t *res);
    void getResolution(mm_camera_dimension_t *res);
    virtual void        release();

    status_t setFormat();
    status_t setMode(int enable);

    virtual void        setHALCameraControl(QCameraHardwareInterface* ctrl);

    //static status_t     openChannel(mm_camera_t *, mm_camera_channel_type_t ch_type);
    void dataCallback(mm_camera_super_buf_t *bufs);
    virtual int32_t streamOn();
    virtual int32_t streamOff(bool aSync);
    virtual status_t    initStream(int no_cb_needed);
    virtual status_t    deinitStream();
    virtual void releaseRecordingFrame(const void *opaque)
    {
      ;
    }
    virtual void prepareHardware()
    {
      ;
    }
    virtual sp<IMemoryHeap> getHeap() const{return NULL;}
    virtual status_t    initDisplayBuffers(){return NO_ERROR;}
    virtual status_t initPreviewOnlyBuffers(){return NO_ERROR;}
    virtual sp<IMemoryHeap> getRawHeap() const {return NULL;}
    virtual void *getLastQueuedFrame(void){return NULL;}
    virtual status_t takePictureZSL(void){return NO_ERROR;}
    virtual status_t takeLiveSnapshot(){return NO_ERROR;}
    virtual void setModeLiveSnapshot(bool){;}
    virtual status_t initSnapshotBuffers(cam_ctrl_dimension_t *dim,
                                 int num_of_buf){return NO_ERROR;}

    virtual void setFullSizeLiveshot(bool){};
    /* Set the ANativeWindow */
    virtual int setPreviewWindow(preview_stream_ops_t* window) {return NO_ERROR;}
    virtual void notifyROIEvent(fd_roi_t roi) {;}
    virtual void notifyWDenoiseEvent(cam_ctrl_status_t status, void * cookie) {;}
    virtual void resetSnapshotCounters(void ){};
    virtual void initHdrInfoForSnapshot(bool HDR_on, int number_frames, int *exp){};
    virtual void notifyHdrEvent(cam_ctrl_status_t status, void * cookie){};
    virtual status_t receiveRawPicture(mm_camera_super_buf_t* recvd_frame, uint32_t *jobId){return NO_ERROR;};
    virtual status_t encodeData(mm_camera_super_buf_t* recvd_frame, uint32_t *jobId){return NO_ERROR;};
    virtual void receiveCompleteJpegPicture(uint32_t jobId, uint8_t* out_data, uint32_t data_size){};
    QCameraStream();
    QCameraStream(uint32_t CameraHandle,
                        uint32_t ChannelId,
                        uint32_t Width,
                        uint32_t Height,
                        uint32_t Format,
                        uint8_t NumBuffers,
                        mm_camera_vtbl_t *mm_ops,
                        mm_camera_img_mode imgmode,
                        camera_mode_t mode);
    virtual             ~QCameraStream();
    QCameraHardwareInterface*  mHalCamCtrl;
    image_crop_t mCrop;

    camera_mode_t myMode;

    mutable Mutex mStopCallbackLock;
public:
//     friend void liveshot_callback(mm_camera_ch_data_buf_t *frame,void *user_data);
};
/*
*   Record Class
*/
class QCameraStream_record : public QCameraStream {
public:
  void        release() ;

  static QCameraStream* createInstance(uint32_t CameraHandle,
                        uint32_t ChannelId,
                        uint32_t Width,
                        uint32_t Height,
                        uint32_t Format,
                        uint8_t NumBuffers,
                        mm_camera_vtbl_t *mm_ops,
                        mm_camera_img_mode imgmode,
                        camera_mode_t mode);
  static void            deleteInstance(QCameraStream *p);

  QCameraStream_record() {};
  virtual             ~QCameraStream_record();

  status_t processRecordFrame(mm_camera_super_buf_t *data);
  status_t initEncodeBuffers();
  status_t getBufferInfo(sp<IMemory>& Frame, size_t *alignedSize);

  void releaseRecordingFrame(const void *opaque);
  void debugShowVideoFPS() const;

  status_t takeLiveSnapshot();
  mm_camera_buf_def_t              mRecordBuf[2*VIDEO_BUFFER_COUNT];
  void releaseEncodeBuffer();
private:
  QCameraStream_record(uint32_t CameraHandle,
                        uint32_t ChannelId,
                        uint32_t Width,
                        uint32_t Height,
                        uint32_t Format,
                        uint8_t NumBuffers,
                        mm_camera_vtbl_t *mm_ops,
                        mm_camera_img_mode imgmode,
                        camera_mode_t mode);

  bool mDebugFps;
  mm_camera_super_buf_t          mRecordedFrames[MM_CAMERA_MAX_NUM_FRAMES];
  int mJpegMaxSize;
  QCameraStream *mStreamSnap;

};

class QCameraStream_preview : public QCameraStream {
public:
    void        release() ;

//    static QCameraStream*  createInstance(int, camera_mode_t);
    static QCameraStream* createInstance(uint32_t CameraHandle,
                        uint32_t ChannelId,
                        uint32_t Width,
                        uint32_t Height,
                        uint32_t Format,
                        uint8_t NumBuffers,
                        mm_camera_vtbl_t *mm_ops,
                        mm_camera_img_mode imgmode,
                        camera_mode_t mode);
    static void            deleteInstance(QCameraStream *p);

    QCameraStream_preview() {};
    virtual             ~QCameraStream_preview();
    void *getLastQueuedFrame(void);
    /*init preview buffers with display case*/
    status_t initDisplayBuffers();
    /*init preview buffers without display case*/
    status_t initPreviewOnlyBuffers();
    status_t initStream(int no_cb_needed);
    status_t processPreviewFrame(mm_camera_super_buf_t *frame);
    /*init preview buffers with display case*/
    status_t processPreviewFrameWithDisplay(mm_camera_super_buf_t *frame);
    /*init preview buffers without display case*/
    status_t processPreviewFrameWithOutDisplay(mm_camera_super_buf_t *frame);

    int setPreviewWindow(preview_stream_ops_t* window);
    void notifyROIEvent(fd_roi_t roi);
    friend class QCameraHardwareInterface;

private:
    QCameraStream_preview(uint32_t CameraHandle,
                        uint32_t ChannelId,
                        uint32_t Width,
                        uint32_t Height,
                        uint32_t Format,
                        uint8_t NumBuffers,
                        mm_camera_vtbl_t *mm_ops,
                        mm_camera_img_mode imgmode,
                        camera_mode_t mode);
    /*allocate and free buffers with display case*/
    status_t                 getBufferFromSurface();
    status_t                 putBufferToSurface();

    /*allocate and free buffers without display case*/
    status_t                 getBufferNoDisplay();
    status_t                 freeBufferNoDisplay();

    void                     dumpFrameToFile(mm_camera_buf_def_t* newFrame);
    bool                     mFirstFrameRcvd;

    int8_t                   my_id;
    mm_camera_op_mode_type_t op_mode;
    cam_ctrl_dimension_t     dim;
    mm_camera_buf_def_t      *mLastQueuedFrame;
    mm_camera_buf_def_t      mDisplayBuf[2*PREVIEW_BUFFER_COUNT];
    mm_camera_buf_def_t      mRdiBuf;
    Mutex                   mDisplayLock;
    preview_stream_ops_t   *mPreviewWindow;
    static const int        kPreviewBufferCount = PREVIEW_BUFFER_COUNT;
    mm_camera_super_buf_t mNotifyBuffer[16];
    int8_t                  mNumFDRcvd;
    int                     mVFEOutputs;
    int                     mHFRFrameCnt;
    int                     mHFRFrameSkip;
};

class QCameraStream_SnapshotMain : public QCameraStream {
public:
    void        release();
    status_t    initStream(int no_cb_needed);
    status_t    initMainBuffers();
    bool        isZSLMode();
    void        jpegErrorHandler(jpeg_job_status_t event);
    void        deInitMainBuffers();
    void        initHdrInfoForSnapshot(bool HDR_on, int number_frames, int *exp);
    void        notifyHdrEvent(cam_ctrl_status_t status, void * cookie);
    static void            deleteInstance(QCameraStream *p);
    status_t receiveRawPicture(mm_camera_super_buf_t* recvd_frame, uint32_t *jobId);
    void receiveCompleteJpegPicture(camera_jpeg_data_t* jpeg_data);
    mm_camera_buf_def_t mSnapshotStreamBuf[MM_CAMERA_MAX_NUM_FRAMES];
    static QCameraStream* createInstance(uint32_t CameraHandle,
                        uint32_t ChannelId,
                        uint32_t Width,
                        uint32_t Height,
                        uint32_t Format,
                        uint8_t NumBuffers,
                        mm_camera_vtbl_t *mm_ops,
                        mm_camera_img_mode imgmode,
                        camera_mode_t mode);
    QCameraStream_SnapshotMain(uint32_t CameraHandle,
                        uint32_t ChannelId,
                        uint32_t Width,
                        uint32_t Height,
                        uint32_t Format,
                        uint8_t NumBuffers,
                        mm_camera_vtbl_t *mm_ops,
                        mm_camera_img_mode imgmode,
                        camera_mode_t mode);
    ~QCameraStream_SnapshotMain();

private:
    status_t doHdrProcessing();
    status_t encodeData(mm_camera_super_buf_t* recvd_frame, uint32_t *jobId);
    void notifyShutter(bool play_shutter_sound);

    /*Member variables*/
    snap_hdr_record_t    mHdrInfo;
    int mSnapshotState;
};

class QCameraStream_SnapshotThumbnail : public QCameraStream {
public:
    void        release();
    status_t    initThumbnailBuffers();
	void        deInitThumbnailBuffers();
    static QCameraStream* createInstance(uint32_t CameraHandle,
                        uint32_t ChannelId,
                        uint32_t Width,
                        uint32_t Height,
                        uint32_t Format,
                        uint8_t NumBuffers,
                        mm_camera_vtbl_t *mm_ops,
                        mm_camera_img_mode imgmode,
                        camera_mode_t mode);
    QCameraStream_SnapshotThumbnail(uint32_t CameraHandle,
                        uint32_t ChannelId,
                        uint32_t Width,
                        uint32_t Height,
                        uint32_t Format,
                        uint8_t NumBuffers,
                        mm_camera_vtbl_t *mm_ops,
                        mm_camera_img_mode imgmode,
                        camera_mode_t mode);
    ~QCameraStream_SnapshotThumbnail();
    static void            deleteInstance(QCameraStream *p);
    mm_camera_buf_def_t mPostviewStreamBuf[MM_CAMERA_MAX_NUM_FRAMES];
};

}; // namespace android

#endif
