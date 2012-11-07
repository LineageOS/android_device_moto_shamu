/*
Copyright (c) 2011-2012, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of Code Aurora Forum, Inc. nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "mm_qcamera_dbg.h"
#include "mm_qcamera_app.h"

/* This callback is received once the complete JPEG encoding is done */
static void jpeg_encode_cb(jpeg_job_status_t status,
                           uint8_t thumbnailDroppedFlag,
                           uint32_t client_hdl,
                           uint32_t jobId,
                           uint8_t* out_data,
                           uint32_t data_size,
                           void *userData)
{
    int i = 0;
    mm_camera_test_obj_t *pme = NULL;
    CDBG("%s: BEGIN\n", __func__);

    pme = (mm_camera_test_obj_t *)userData;
    if (pme->jpeg_hdl != client_hdl ||
        jobId != pme->current_job_id ||
        !pme->current_job_frames) {
        CDBG_ERROR("%s: NULL current job frames or not matching job ID (%d, %d)",
                   __func__, jobId, pme->current_job_id);
        return;
    }

    /* dump jpeg img */
    CDBG_ERROR("%s: job %d, status=%d, thumbnail_dropped=%d",
               __func__, jobId, status, thumbnailDroppedFlag);
    if (status == JPEG_JOB_STATUS_DONE) {
        mm_app_dump_jpeg_frame(out_data, data_size, "jpeg_dump", "jpg", jobId);
    }

    /* buf done current encoding frames */
    pme->current_job_id = 0;
    for (i = 0; i < pme->current_job_frames->num_bufs; i++) {
        if (MM_CAMERA_OK != pme->cam->ops->qbuf(pme->current_job_frames->camera_handle,
                                                pme->current_job_frames->ch_id,
                                                pme->current_job_frames->bufs[i])) {
            CDBG_ERROR("%s: Failed in Qbuf\n", __func__);
        }
    }
    free(pme->current_job_frames);
    pme->current_job_frames = NULL;

    /* signal snapshot is done */
    mm_camera_app_done();
}

int encodeData(mm_camera_test_obj_t *test_obj, mm_camera_super_buf_t* recvd_frame)
{
    cam_capapbility_t *cam_cap = (cam_capapbility_t *)(test_obj->cap_buf.buf.buffer);

    int rc = -MM_CAMERA_E_GENERAL;
    int i;
    mm_jpeg_job job;
    mm_camera_channel_t *channel = NULL;
    mm_camera_stream_t *p_stream = NULL;
    mm_camera_stream_t *m_stream = NULL;
    mm_camera_buf_def_t *p_frame = NULL;
    mm_camera_buf_def_t *m_frame = NULL;
    src_image_buffer_info* p_imgbuf_info = NULL;
    src_image_buffer_info* m_imgbuf_info = NULL;

    /* find channel */
    for (i = 0; i < MM_CHANNEL_TYPE_MAX; i++) {
        if (test_obj->channels[i].ch_id == recvd_frame->ch_id) {
            channel = &test_obj->channels[i];
            break;
        }
    }
    if (NULL == channel) {
        CDBG_ERROR("%s: Wrong channel id (%d)", __func__, recvd_frame->ch_id);
        return rc;
    }

    /* find snapshot stream */
    for (i = 0; i < channel->num_streams; i++) {
        if (channel->streams[i].s_config.stream_info->stream_type == CAM_STREAM_TYPE_SNAPSHOT) {
            m_stream = &channel->streams[i];
            break;
        }
    }
    if (NULL == m_stream) {
        CDBG_ERROR("%s: cannot find snapshot stream", __func__);
        return rc;
    }

    /* find snapshot frame */
    for (i = 0; i < recvd_frame->num_bufs; i++) {
        if (recvd_frame->bufs[i]->stream_id == m_stream->s_id) {
            m_frame = recvd_frame->bufs[i];
            break;
        }
    }
    if (NULL == m_frame) {
        CDBG_ERROR("%s: main frame is NULL", __func__);
        return rc;
    }

    mm_app_dump_frame(m_frame, "main", "yuv", m_frame->frame_idx);

    /* find postview stream */
    for (i = 0; i < channel->num_streams; i++) {
        if (channel->streams[i].s_config.stream_info->stream_type == CAM_STREAM_TYPE_POSTVIEW) {
            p_stream = &channel->streams[i];
            break;
        }
    }
    if (NULL != p_stream) {
        /* find preview frame */
        for (i = 0; i < recvd_frame->num_bufs; i++) {
            if (recvd_frame->bufs[i]->stream_id == p_stream->s_id) {
                p_frame = recvd_frame->bufs[i];
                break;
            }
        }
        if (NULL != p_frame) {
            mm_app_dump_frame(p_frame, "postview", "yuv", p_frame->frame_idx);
        }
    }

    /* remember current frames being encoded */
    test_obj->current_job_frames =
        (mm_camera_super_buf_t *)malloc(sizeof(mm_camera_super_buf_t));
    if (!test_obj->current_job_frames) {
        CDBG_ERROR("%s: No memory for current_job_frames", __func__);
        return rc;
    }
    memcpy(test_obj->current_job_frames, recvd_frame, sizeof(mm_camera_super_buf_t));

    memset(&job, 0, sizeof(job));
    job.job_type = JPEG_JOB_TYPE_ENCODE;
    job.encode_job.jpeg_cb = jpeg_encode_cb;
    job.encode_job.userdata = (void*)test_obj;
    job.encode_job.encode_parm.exif_data = NULL;
    job.encode_job.encode_parm.exif_numEntries = 0;
    job.encode_job.encode_parm.rotation = 0;
    job.encode_job.encode_parm.buf_info.src_imgs.src_img_num = recvd_frame->num_bufs;
    job.encode_job.encode_parm.rotation = 0;
    if (cam_cap->position == CAM_POSITION_BACK) {
        /* back camera, rotate 90 */
        job.encode_job.encode_parm.rotation = 90;
    }

    /* fill in main src img encode param */
    m_imgbuf_info = &job.encode_job.encode_parm.buf_info.src_imgs.src_img[JPEG_SRC_IMAGE_TYPE_MAIN];
    m_imgbuf_info->type = JPEG_SRC_IMAGE_TYPE_MAIN;
    m_imgbuf_info->color_format = MM_JPEG_COLOR_FORMAT_YCRCBLP_H2V2;
    m_imgbuf_info->quality = 85;
    m_imgbuf_info->img_fmt = JPEG_SRC_IMAGE_FMT_YUV;
    m_imgbuf_info->num_bufs = 1;
    m_imgbuf_info->src_image[0].fd = m_frame->fd;
    m_imgbuf_info->src_image[0].buf_vaddr = m_frame->buffer;
    memcpy(&m_imgbuf_info->src_dim,
           &m_stream->s_config.stream_info->dim,
           sizeof(cam_dimension_t));
    memcpy(&m_imgbuf_info->out_dim,
           &m_stream->s_config.stream_info->dim,
           sizeof(cam_dimension_t));
    memcpy(&m_imgbuf_info->crop,
           &m_stream->s_config.stream_info->crop,
           sizeof(cam_rect_t));
    memcpy(&m_imgbuf_info->src_image[0].offset,
           &m_stream->offset,
           sizeof(cam_frame_len_offset_t));
    mm_app_cache_ops((mm_camera_app_meminfo_t *)recvd_frame->bufs[i]->mem_info,
                     ION_IOC_CLEAN_INV_CACHES);

    if (p_frame) {
        /* fill in thumbnail src img encode param */
        p_imgbuf_info = &job.encode_job.encode_parm.buf_info.src_imgs.src_img[JPEG_SRC_IMAGE_TYPE_THUMB];
        p_imgbuf_info->type = JPEG_SRC_IMAGE_TYPE_THUMB;
        p_imgbuf_info->color_format = MM_JPEG_COLOR_FORMAT_YCRCBLP_H2V2;
        p_imgbuf_info->quality = 85;
        p_imgbuf_info->img_fmt = JPEG_SRC_IMAGE_FMT_YUV;
        p_imgbuf_info->num_bufs = 1;
        p_imgbuf_info->src_image[0].fd = p_frame->fd;
        p_imgbuf_info->src_image[0].buf_vaddr = p_frame->buffer;
        memcpy(&p_imgbuf_info->src_dim,
               &p_stream->s_config.stream_info->dim,
               sizeof(cam_dimension_t));
        memcpy(&p_imgbuf_info->out_dim,
               &p_stream->s_config.stream_info->dim,
               sizeof(cam_dimension_t));
        memcpy(&p_imgbuf_info->crop,
               &p_stream->s_config.stream_info->crop,
               sizeof(cam_rect_t));
        memcpy(&p_imgbuf_info->src_image[0].offset,
               &p_stream->offset,
               sizeof(cam_frame_len_offset_t));
    }

    /* fill in sink img param */
    job.encode_job.encode_parm.buf_info.sink_img.buf_len = test_obj->jpeg_buf.buf.frame_len;
    job.encode_job.encode_parm.buf_info.sink_img.buf_vaddr = test_obj->jpeg_buf.buf.buffer;
    job.encode_job.encode_parm.buf_info.sink_img.fd = test_obj->jpeg_buf.buf.fd;

    mm_app_cache_ops((mm_camera_app_meminfo_t *)recvd_frame->bufs[i]->mem_info,
                     ION_IOC_CLEAN_INV_CACHES);

    rc = test_obj->jpeg_ops.start_job(test_obj->jpeg_hdl, &job, &test_obj->current_job_id);
    if ( 0 != rc ) {
        free(test_obj->current_job_frames);
        test_obj->current_job_frames = NULL;
    }

    return rc;
}

static void mm_app_snapshot_notify_cb(mm_camera_super_buf_t *bufs,
                                      void *user_data)
{

    int rc;
    int i = 0;
    mm_camera_test_obj_t *pme = (mm_camera_test_obj_t *)user_data;

    CDBG("%s: BEGIN\n", __func__);

    /* start jpeg encoding job */
    rc = encodeData(pme, bufs);

    /* buf done rcvd frames in error case */
    if ( 0 != rc ) {
        for (i=0; i<bufs->num_bufs; i++) {
            if (MM_CAMERA_OK != pme->cam->ops->qbuf(bufs->camera_handle,
                                                    bufs->ch_id,
                                                    bufs->bufs[i])) {
                CDBG_ERROR("%s: Failed in Qbuf\n", __func__);
            }
            mm_app_cache_ops((mm_camera_app_meminfo_t *)bufs->bufs[i]->mem_info,
                             ION_IOC_INV_CACHES);
        }
    }

    CDBG("%s: END\n", __func__);
}

mm_camera_channel_t * mm_app_add_snapshot_channel(mm_camera_test_obj_t *test_obj)
{
    mm_camera_channel_t *channel = NULL;
    mm_camera_stream_t *stream = NULL;

    channel = mm_app_add_channel(test_obj,
                                 MM_CHANNEL_TYPE_SNAPSHOT,
                                 NULL,
                                 NULL,
                                 NULL);
    if (NULL == channel) {
        CDBG_ERROR("%s: add channel failed", __func__);
        return NULL;
    }

    stream = mm_app_add_snapshot_stream(test_obj,
                                        channel,
                                        mm_app_snapshot_notify_cb,
                                        (void *)test_obj,
                                        1,
                                        1);
    if (NULL == stream) {
        CDBG_ERROR("%s: add snapshot stream failed\n", __func__);
        mm_app_del_channel(test_obj, channel);
        return NULL;
    }

    return channel;
}

mm_camera_stream_t * mm_app_add_postview_stream(mm_camera_test_obj_t *test_obj,
                                                mm_camera_channel_t *channel,
                                                mm_camera_buf_notify_t stream_cb,
                                                void *userdata,
                                                uint8_t num_bufs,
                                                uint8_t num_burst)
{
    int rc = MM_CAMERA_OK;
    mm_camera_stream_t *stream = NULL;

    stream = mm_app_add_stream(test_obj, channel);
    if (NULL == stream) {
        CDBG_ERROR("%s: add stream failed\n", __func__);
        return NULL;
    }

    stream->s_config.mem_vtbl.get_bufs = mm_app_stream_initbuf;
    stream->s_config.mem_vtbl.put_bufs = mm_app_stream_deinitbuf;
    stream->s_config.mem_vtbl.user_data = (void *)stream;
    stream->s_config.stream_cb = stream_cb;
    stream->s_config.userdata = userdata;
    stream->num_of_bufs = num_bufs;

    stream->s_config.stream_info = (cam_stream_info_t *)stream->s_info_buf.buf.buffer;
    memset(stream->s_config.stream_info, 0, sizeof(cam_stream_info_t));
    stream->s_config.stream_info->stream_type = CAM_STREAM_TYPE_POSTVIEW;
    if (num_burst == 0) {
        stream->s_config.stream_info->streaming_mode = CAM_STREAMING_MODE_CONTINUOUS;
    } else {
        stream->s_config.stream_info->streaming_mode = CAM_STREAMING_MODE_BURST;
        stream->s_config.stream_info->num_of_burst = num_burst;
    }
    stream->s_config.stream_info->meta_header = CAM_META_DATA_TYPE_DEF;
    stream->s_config.stream_info->fmt = DEFAULT_PREVIEW_FORMAT;
    stream->s_config.stream_info->dim.width = DEFAULT_PREVIEW_WIDTH;
    stream->s_config.stream_info->dim.height = DEFAULT_PREVIEW_HEIGHT;
    stream->s_config.stream_info->width_padding = CAM_PAD_TO_WORD;
    stream->s_config.stream_info->height_padding = CAM_PAD_TO_WORD;
    stream->s_config.stream_info->plane_padding = DEFAULT_PREVIEW_PADDING;

    rc = mm_app_config_stream(test_obj, channel, stream, &stream->s_config);
    if (MM_CAMERA_OK != rc) {
        CDBG_ERROR("%s:config preview stream err=%d\n", __func__, rc);
        return NULL;
    }

    return stream;
}

int mm_app_start_capture(mm_camera_test_obj_t *test_obj,
                         uint8_t num_snapshots)
{
    int32_t rc = MM_CAMERA_OK;
    mm_camera_channel_t *channel = NULL;
    mm_camera_stream_t *s_main = NULL;
    mm_camera_stream_t *s_postview = NULL;
    mm_camera_channel_attr_t attr;

    memset(&attr, 0, sizeof(mm_camera_channel_attr_t));
    attr.notify_mode = MM_CAMERA_SUPER_BUF_NOTIFY_CONTINUOUS;
    channel = mm_app_add_channel(test_obj,
                                 MM_CHANNEL_TYPE_CAPTURE,
                                 &attr,
                                 mm_app_snapshot_notify_cb,
                                 test_obj);
    if (NULL == channel) {
        CDBG_ERROR("%s: add channel failed", __func__);
        return -MM_CAMERA_E_GENERAL;
    }

    s_postview = mm_app_add_postview_stream(test_obj,
                                            channel,
                                            NULL,
                                            NULL,
                                            num_snapshots,
                                            num_snapshots);
    if (NULL == s_postview) {
        CDBG_ERROR("%s: add preview stream failed\n", __func__);
        mm_app_del_channel(test_obj, channel);
        return rc;
    }

    s_main = mm_app_add_snapshot_stream(test_obj,
                                        channel,
                                        NULL,
                                        NULL,
                                        num_snapshots,
                                        num_snapshots);
    if (NULL == s_main) {
        CDBG_ERROR("%s: add main snapshot stream failed\n", __func__);
        mm_app_del_stream(test_obj, channel, s_postview);
        mm_app_del_channel(test_obj, channel);
        return rc;
    }

    rc = mm_app_start_channel(test_obj, channel);
    if (MM_CAMERA_OK != rc) {
        CDBG_ERROR("%s:start zsl failed rc=%d\n", __func__, rc);
        mm_app_del_stream(test_obj, channel, s_postview);
        mm_app_del_stream(test_obj, channel, s_main);
        mm_app_del_channel(test_obj, channel);
        return rc;
    }

    return rc;
}

int mm_app_stop_capture(mm_camera_test_obj_t *test_obj)
{
    int rc = MM_CAMERA_OK;
    mm_camera_channel_t *ch = NULL;

    ch = mm_app_get_channel_by_type(test_obj, MM_CHANNEL_TYPE_CAPTURE);

    rc = mm_app_stop_channel(test_obj, ch);
    if (MM_CAMERA_OK != rc) {
        CDBG_ERROR("%s:stop recording failed rc=%d\n", __func__, rc);
    }

    return rc;
}
