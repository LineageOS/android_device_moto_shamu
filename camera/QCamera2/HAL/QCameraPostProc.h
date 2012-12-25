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

#ifndef __QCAMERA_POSTPROC_H__
#define __QCAMERA_POSTPROC_H__

extern "C" {
#include <mm_camera_interface.h>
#include <mm_jpeg_interface.h>
}
#include "QCamera2HWI.h"

namespace android {

typedef struct {
    int num_exif_entries;
    exif_tags_info_t *exif_entries;
} qcamera_exif_data_t;

typedef struct {
    uint32_t jobId;
    uint32_t client_hdl;
    uint8_t* out_data;
    qcamera_exif_data_t *exif_data;
    mm_camera_super_buf_t *src_frame;
} qcamera_jpeg_data_t;

typedef struct {
    uint32_t jobId;
    mm_camera_super_buf_t *src_frame;
} qcamera_pp_data_t;

typedef struct {
    mm_camera_super_buf_t *frame;
} qcamera_pp_request_t;

typedef struct {
    uint32_t jobId;
    jpeg_job_status_t status;
    uint8_t thumbnailDroppedFlag;
    uint8_t* out_data;
    uint32_t data_size;
} qcamera_jpeg_evt_payload_t;

typedef struct {
    int32_t                  msg_type;
    camera_memory_t *        data;
    unsigned int             index;
    camera_frame_metadata_t *metadata;
    QCameraHeapMemory *      jpeg_mem; // jpeg heap mem for release after CB
} qcamera_data_argm_t;

class QCameraPostProcessor
{
public:
    QCameraPostProcessor(QCamera2HardwareInterface *cam_ctrl);
    virtual ~QCameraPostProcessor();

    int32_t init(jpeg_encode_callback_t jpeg_cb, void *user_data);
    int32_t deinit();
    int32_t start();
    int32_t stop();
    int32_t processData(mm_camera_super_buf_t *frame);
    int32_t processPPData(mm_camera_super_buf_t *frame);
    int32_t processJpegEvt(qcamera_jpeg_evt_payload_t *evt);

private:
    int32_t sendEvtNotify(int32_t msg_type, int32_t ext1, int32_t ext2);
    int32_t sendDataNotify(int32_t msg_type,
                           camera_memory_t *data,
                           uint8_t index,
                           camera_frame_metadata_t *metadata,
                           QCameraHeapMemory *jpeg_mem);
    qcamera_jpeg_data_t *findJpegJobByJobId(uint32_t jobId);
    mm_jpeg_color_format getColorfmtFromImgFmt(cam_format_t img_fmt);
    jpeg_enc_src_img_fmt_t getJpegImgTypeFromImgFmt(cam_format_t img_fmt);
    int32_t encodeData(mm_camera_super_buf_t *recvd_frame,
                       qcamera_jpeg_data_t *jpeg_job_data);
    int32_t fillImgInfo(QCameraStream *stream,
                        mm_camera_buf_def_t *frame,
                        src_image_buffer_info *buf_info,
                        jpeg_enc_src_img_type_t img_type,
                        uint32_t jpeg_quality);
    void releaseSuperBuf(mm_camera_super_buf_t *super_buf);
    void releaseNotifyData(qcamera_data_argm_t *app_cb);
    void releaseJpegJobData(qcamera_jpeg_data_t *job);
    int32_t releaseExifData(qcamera_exif_data_t *exif_data);

    static void releaseOutputData(void *data, void *user_data);
    static void releasePPInputData(void *data, void *user_data);
    static void releaseJpegInputData(void *data, void *user_data);
    static void releaseOngoingJpegData(void *data, void *user_data);
    static void releaseOngoingPPData(void *data, void *user_data);

    static void *dataProcessRoutine(void *data);
    static void *dataNotifyRoutine(void *data);

private:
    QCamera2HardwareInterface *m_parent;
    jpeg_encode_callback_t     mJpegCB;
    void *                     mJpegUserData;
    mm_jpeg_ops_t              mJpegHandle;
    uint32_t                   mJpegClientHandle;

    QCameraReprocessChannel *  m_pReprocessChannel;

    QCameraQueue m_inputPPQ;            // input queue for postproc
    QCameraQueue m_ongoingPPQ;          // ongoing postproc queue
    QCameraQueue m_inputJpegQ;          // input jpeg job queue
    QCameraQueue m_ongoingJpegQ;        // ongoing jpeg job queue
    QCameraCmdThread m_dataProcTh;      // thread for data processing
    QCameraQueue m_dataNotifyQ;         // data notify queue
    QCameraCmdThread m_dataNotifyTh;    // thread handling data notify to service layer
};

}; // namespace android

#endif /* __QCAMERA_POSTPROC_H__ */
