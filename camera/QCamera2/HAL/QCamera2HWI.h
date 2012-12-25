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

#ifndef __QCAMERA2HARDWAREINTERFACE_H__
#define __QCAMERA2HARDWAREINTERFACE_H__

#include <hardware/camera.h>
#include <camera/QCameraParameters.h>

#include "QCameraQueue.h"
#include "QCameraCmdThread.h"
#include "QCameraChannel.h"
#include "QCameraStream.h"
#include "QCameraStateMachine.h"
#include "QCameraAllocator.h"
#include "QCameraPostProc.h"

extern "C" {
#include <mm_camera_interface.h>
#include <mm_jpeg_interface.h>
}

namespace android {

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef enum {
    QCAMERA_CH_TYPE_ZSL,
    QCAMERA_CH_TYPE_CAPTURE,
    QCAMERA_CH_TYPE_PREVIEW,
    QCAMERA_CH_TYPE_VIDEO,
    QCAMERA_CH_TYPE_SNAPSHOT,
    QCAMERA_CH_TYPE_RAW,
    QCAMERA_CH_TYPE_METADATA,
    QCAMERA_CH_TYPE_REPROCESS,
    QCAMERA_CH_TYPE_MAX
} qcamera_ch_type_enum_t;

typedef struct {
    int32_t msg_type;
    int32_t ext1;
    int32_t ext2;
} qcamera_evt_argm_t;

#define QCAMERA_DUMP_FRM_PREVIEW    1
#define QCAMERA_DUMP_FRM_VIDEO      1<<1
#define QCAMERA_DUMP_FRM_SNAPSHOT   1<<2
#define QCAMERA_DUMP_FRM_THUMBNAIL  1<<3
#define QCAMERA_DUMP_FRM_RAW        1<<4
#define QCAMERA_DUMP_FRM_JPEG       1<<5

class QCamera2HardwareInterface : public QCameraAllocator
{
public:
    /* static variable and functions accessed by camera service */
    static camera_device_ops_t mCameraOps;

    static int set_preview_window(struct camera_device *,
        struct preview_stream_ops *window);
    static void set_CallBacks(struct camera_device *,
        camera_notify_callback notify_cb,
        camera_data_callback data_cb,
        camera_data_timestamp_callback data_cb_timestamp,
        camera_request_memory get_memory,
        void *user);
    static void enable_msg_type(struct camera_device *, int32_t msg_type);
    static void disable_msg_type(struct camera_device *, int32_t msg_type);
    static int msg_type_enabled(struct camera_device *, int32_t msg_type);
    static int start_preview(struct camera_device *);
    static void stop_preview(struct camera_device *);
    static int preview_enabled(struct camera_device *);
    static int store_meta_data_in_buffers(struct camera_device *, int enable);
    static int start_recording(struct camera_device *);
    static void stop_recording(struct camera_device *);
    static int recording_enabled(struct camera_device *);
    static void release_recording_frame(struct camera_device *, const void *opaque);
    static int auto_focus(struct camera_device *);
    static int cancel_auto_focus(struct camera_device *);
    static int take_picture(struct camera_device *);
    static int cancel_picture(struct camera_device *);
    static int set_parameters(struct camera_device *, const char *parms);
    static char* get_parameters(struct camera_device *);
    static void put_parameters(struct camera_device *, char *);
    static int send_command(struct camera_device *,
              int32_t cmd, int32_t arg1, int32_t arg2);
    static void release(struct camera_device *);
    static int dump(struct camera_device *, int fd);
    static int close_camera_device(hw_device_t *);

public:
    QCamera2HardwareInterface(int cameraId);
    virtual ~QCamera2HardwareInterface();
    int openCamera(struct hw_device_t **hw_device);
    int getCapabilities(struct camera_info *info);

    // Implementation of QCameraAllocator
    virtual QCameraMemory *allocateStreamBuf(cam_stream_type_t stream_type, int size);
    virtual QCameraHeapMemory *allocateStreamInfoBuf(cam_stream_type_t stream_type);

    friend class QCameraStateMachine;
    friend class QCameraPostProcessor;

private:
    int setPreviewWindow(struct preview_stream_ops *window);
    int setCallBacks(
        camera_notify_callback notify_cb,
        camera_data_callback data_cb,
        camera_data_timestamp_callback data_cb_timestamp,
        camera_request_memory get_memory,
        void *user);
    int enableMsgType(int32_t msg_type);
    int disableMsgType(int32_t msg_type);
    int msgTypeEnabled(int32_t msg_type);
    int startPreview();
    int stopPreview();
    int storeMetaDataInBuffers(int enable);
    int startRecording();
    int stopRecording();
    int releaseRecordingFrame(const void *opaque);
    int autoFocus();
    int cancelAutoFocus();
    int takePicture();
    int cancelPicture();
    int takeLiveSnapshot();
    int cancelLiveSnapshot();
    int setParameters(const char *parms);
    char* getParameters();
    int putParameters(char *);
    int sendCommand(int32_t cmd, int32_t arg1, int32_t arg2);
    int release();
    int dump(int fd);

    int openCamera();
    int closeCamera();

    int processAPI(qcamera_sm_evt_enum_t api, void *api_payload);
    int processEvt(qcamera_sm_evt_enum_t evt, void *evt_payload);
    void lockAPI();
    void waitAPIResult(qcamera_sm_evt_enum_t api_evt);
    void unlockAPI();
    void signalAPIResult(qcamera_api_result_t *result);
    int getZSLBurstInterval();
    int getZSLQueueDepth();
    int getZSLBackLookCount();
    bool isZSLMode();
    bool isNoDisplayMode();
    bool isWNREnabled();
    bool needDebugFps();
    bool needOfflineReprocess();
    int setRecordingHintValue(bool value);
    uint8_t getNumOfSnapshots();
    int setParameters(const QCameraParameters& params);
    void debugShowVideoFPS();
    void debugShowPreviewFPS();
    void dumpFrameToFile(const void *data, uint32_t size,
                         int index, int dump_type);
    void releaseSuperBuf(mm_camera_super_buf_t *super_buf);
    void playShutter();
    void getPictureSize(int *picture_width, int *picture_height);
    void getThumbnailSize(int *width, int *height);
    int getJpegQuality();
    int getJpegRotation();
    int getExifData(exif_tags_info_t **exif_data, int *num_exif_entries);

    int32_t processAutoFocusEvent(uint32_t status);
    int32_t processZoomEvent(uint32_t status);
    int32_t processJpegNotify(qcamera_jpeg_evt_payload_t *jpeg_job);

    int32_t sendEvtNotify(int32_t msg_type, int32_t ext1, int32_t ext2);
    int32_t sendDataNotify(int32_t msg_type,
                           camera_memory_t *data,
                           uint8_t index,
                           camera_frame_metadata_t *metadata);

    int32_t addChannel(qcamera_ch_type_enum_t ch_type);
    int32_t startChannel(qcamera_ch_type_enum_t ch_type);
    int32_t stopChannel(qcamera_ch_type_enum_t ch_type);
    int32_t delChannel(qcamera_ch_type_enum_t ch_type);
    int32_t addPreviewChannel();
    int32_t addSnapshotChannel();
    int32_t addVideoChannel();
    int32_t addZSLChannel();
    int32_t addCaptureChannel();
    int32_t addRawChannel();
    int32_t addMetaDataChannel();
    int32_t addReprocessChannel();
    int32_t preparePreview();
    void unpreparePreview();
    QCameraChannel *getChannelByHandle(uint32_t channelHandle);
    mm_camera_buf_def_t *getSnapshotFrame(mm_camera_super_buf_t *recvd_frame);

    static void evtHandle(uint32_t camera_handle,
                          mm_camera_event_t *evt,
                          void *user_data);
    static void jpegEvtHandle(jpeg_job_status_t status,
                              uint8_t thumbnailDroppedFlag,
                              uint32_t client_hdl,
                              uint32_t jobId,
                              uint8_t* out_data,
                              uint32_t data_size,
                              void *userdata);

    static void *evtNotifyRoutine(void *data);

    // helper functions for different data notify cb
    static void zsl_channel_cb(mm_camera_super_buf_t *recvd_frame, void *userdata);

    static void nodisplay_preview_stream_cb_routine(mm_camera_super_buf_t *frame,
                                                    QCameraStream *stream,
                                                    void *userdata);
    static void preview_stream_cb_routine(mm_camera_super_buf_t *frame,
                                          QCameraStream *stream,
                                          void *userdata);
    static void postview_stream_cb_routine(mm_camera_super_buf_t *frame,
                                           QCameraStream *stream,
                                           void *userdata);
    static void video_stream_cb_routine(mm_camera_super_buf_t *frame,
                                        QCameraStream *stream,
                                        void *userdata);
    static void snapshot_stream_cb_routine(mm_camera_super_buf_t *frame,
                                           QCameraStream *stream,
                                           void *userdata);
    static void raw_stream_cb_routine(mm_camera_super_buf_t *frame,
                                      QCameraStream *stream,
                                      void *userdata);
    static void metadata_stream_cb_routine(mm_camera_super_buf_t *frame,
                                           QCameraStream *stream,
                                           void *userdata);
    static void reprocess_stream_cb_routine(mm_camera_super_buf_t *frame,
                                            QCameraStream *stream,
                                            void *userdata);

    //TODO: This will be removed once Parameters class is ready.
    cam_format_t mPreviewFormat;
    int mPreviewWidth;
    int mPreviewHeight;
private:
    camera_device_t   mCameraDevice;
    uint8_t           mCameraId;
    mm_camera_vtbl_t *mCameraHandle;

    preview_stream_ops_t *mPreviewWindow;
    QCameraParameters     mParameters;
    int32_t               mMsgEnabled;
    int                   mStoreMetaDataInFrame;

    camera_notify_callback         mNotifyCb;
    camera_data_callback           mDataCb;
    camera_data_timestamp_callback mDataCbTimestamp;
    camera_request_memory          mGetMemory;
    void                          *mCallbackCookie;

    QCameraStateMachine m_stateMachine;   // state machine
    QCameraPostProcessor m_postprocessor; // post processor
    pthread_mutex_t m_lock;
    pthread_cond_t m_cond;
    qcamera_api_result_t m_apiResult;

    QCameraChannel *m_channels[QCAMERA_CH_TYPE_MAX]; // array holding channel ptr

    QCameraQueue m_evtNotifyQ;          // evt notify queue
    QCameraCmdThread m_evtNotifyTh;     // thread handling evt notify to service layer

    bool mRecordingHint;
    bool mSmoothZoomRunning;
    int  mDenoiseValue;
    int  mDumpFrameEnabled; // mask for type of dumping
    bool mDebugFps;
    bool mShutterSoundPlayed;
    bool mNoDisplayMode; // if true, running no display preview
};

}; // namespace android

#endif /* __QCAMERA2HARDWAREINTERFACE_H__ */
