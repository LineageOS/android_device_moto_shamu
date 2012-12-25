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

#ifndef __MM_CAMERA_INTERFACE_H__
#define __MM_CAMERA_INTERFACE_H__
#include <linux/msm_ion.h>
#include <linux/videodev2.h>
#include <media/msmb_camera.h>
#include "cam_intf.h"
#include "cam_queue.h"

#define MM_CAMERA_MAX_NUM_FRAMES 16
/* num of channels allowed in a camera obj */
#define MM_CAMERA_CHANNEL_MAX 16

typedef struct {
    uint32_t stream_id; /* stream handler */
    int8_t buf_idx;     /* buf idx within the stream bufs */

    struct timespec ts; /* time stamp, to be filled when DQBUF*/
    uint32_t frame_idx; /* frame sequence num, to be filled when DQBUF */

    int8_t num_planes;  /* num of planes, to be filled during mem allocation */
    struct v4l2_plane planes[VIDEO_MAX_PLANES]; /* plane info, to be filled during mem allocation*/
    int fd;             /* fd of the frame, to be filled during mem allocation */
    void *buffer;       /* ptr to real frame buffer, to be filled during mem allocation */
    uint32_t frame_len; /* len of the whole frame, to be filled during mem allocation */
    void *mem_info;     /* reserved for pointing to additional mem info if needed */
} mm_camera_buf_def_t;

typedef struct {
    uint32_t camera_handle;
    uint32_t ch_id;
    uint8_t num_bufs;
    mm_camera_buf_def_t* bufs[MAX_STRAEM_NUM_IN_BUNDLE];
} mm_camera_super_buf_t;

typedef struct {
    cam_event_type_t server_event_type;
    uint32_t status;
} mm_camera_event_t;

typedef void (*mm_camera_event_notify_t)(uint32_t camera_handle,
                                         mm_camera_event_t *evt,
                                         void *user_data);

typedef void (*mm_camera_buf_notify_t) (mm_camera_super_buf_t *bufs,
                                        void *user_data);

typedef int32_t (*map_stream_buf_op_t) (uint32_t frame_idx,
                                        int fd,
                                        uint32_t size,
                                        void *userdata);

typedef int32_t (*unmap_stream_buf_op_t) (uint32_t frame_idx,
                                          void *userdata);

typedef struct {
    map_stream_buf_op_t map_ops;
    unmap_stream_buf_op_t unmap_ops;
    void *userdata;
} mm_camera_map_unmap_ops_tbl_t;

/* mem ops tbl for alloc/dealloc stream buffers */
typedef struct {
  void *user_data;
  int32_t (*get_bufs) (uint32_t camera_handle,
                       uint32_t ch_id,
                       uint32_t stream_id,
                       uint8_t num_bufs,
                       uint8_t *initial_reg_flag,
                       mm_camera_buf_def_t *bufs,
                       mm_camera_map_unmap_ops_tbl_t *ops_tbl,
                       void *user_data);
  int32_t (*put_bufs) (uint32_t camera_handle,
                       uint32_t ch_id,
                       uint32_t stream_id,
                       uint8_t num_bufs,
                       mm_camera_buf_def_t *bufs,
                       mm_camera_map_unmap_ops_tbl_t *ops_tbl,
                       void *user_data);
} mm_camera_stream_mem_vtbl_t;

typedef struct {
    /* stream info*/
    cam_stream_info_t *stream_info;

    /* num of buffers needed */
    uint8_t num_of_bufs;

    /* memory ops table */
    mm_camera_stream_mem_vtbl_t mem_vtbl;

    /* callback func for stream data notify */
    mm_camera_buf_notify_t stream_cb;
    void *userdata;
} mm_camera_stream_config_t;

typedef enum {
    /* ZSL use case: get burst of frames */
    MM_CAMERA_SUPER_BUF_NOTIFY_BURST = 0,

    /* get continuous frames: when the super buf is
     * ready dispatch it to HAL */
    MM_CAMERA_SUPER_BUF_NOTIFY_CONTINUOUS,

    MM_CAMERA_SUPER_BUF_NOTIFY_MAX
} mm_camera_super_buf_notify_mode_t;

typedef enum {
    /* save the frame. No matter focused or not */
    MM_CAMERA_SUPER_BUF_PRIORITY_NORMAL = 0,

    /* only queue the frame that is focused. Will enable
     * meta data header to carry focus info*/
    MM_CAMERA_SUPER_BUF_PRIORITY_FOCUS,

    /* after shutter, only queue matched exposure index */
    MM_CAMERA_SUPER_BUF_PRIORITY_EXPOSURE_BRACKETING,

    MM_CAMERA_SUPER_BUF_PRIORITY_MAX
} mm_camera_super_buf_priority_t;

typedef struct {
    /* notify mode: burst or continuous */
    mm_camera_super_buf_notify_mode_t notify_mode;

    /* queue depth. Only for burst mode */
    uint8_t water_mark;

    /* look back how many frames from last buf */
    uint8_t look_back;

    /* after send first frame to HAL, how many frames
     * needing to be skipped for next delivery? */
    uint8_t post_frame_skip;

    /* save matched priority frames only */
    mm_camera_super_buf_priority_t priority;
} mm_camera_channel_attr_t;

typedef struct {
    /* query camera capabilities */
    int32_t (*query_capability) (uint32_t camera_handle);

    /* register event notify for certain type */
    int32_t (*register_event_notify) (uint32_t camera_handle,
                                      mm_camera_event_notify_t evt_cb,
                                      void *user_data);

    /* close camera by its handler */
    int32_t (*close_camera) (uint32_t camera_handle);

    /* map or unmap buf of camera
     * buf_type: CAM_MAPPING_BUF_TYPE_HIST_BUF
     *           CAM_MAPPING_BUF_TYPE_CAPABILITY
     *           CAM_MAPPING_BUF_TYPE_SETPARM_BUF
     *           CAM_MAPPING_BUF_TYPE_GETPARM_BUF
     */
    int32_t (*map_buf) (uint32_t camera_handle,
                        uint8_t buf_type,
                        int fd,
                        uint32_t size);
    int32_t (*unmap_buf) (uint32_t camera_handle,
                          uint8_t buf_type);

    /* set a parm current value, would assume parm buf is already mapped in backend
     * and according parameters information are filled in the buf before this call */
    int32_t (*set_parms) (uint32_t camera_handle,
                          void *parms); /* TODO: change void * to valid data struct of set_parms */

    /* get a parm current value, would assume parm buf is already mapped in backend
     * and according parameters need to be get are filled in the buf before this call */
    int32_t (*get_parms) (uint32_t camera_handle,
                          void *parms); /* TODO: change void * to valid data struct of get_parms */

    /* perform an action on camera, would assume parm buf is already mapped,
     * and according action information is filled in the buf before this call */
    int32_t (*do_action) (uint32_t camera_handle,
                          void *actions); /* TODO: change void * to valid data struct of actions */

    /* Add a new channel
     * return channel id, zero is invalid ch_id
     * attr, channel_cb, and userdata can be NULL if no superbufCB is needed */
    uint32_t (*add_channel) (uint32_t camera_handle,
                             mm_camera_channel_attr_t *attr,
                             mm_camera_buf_notify_t channel_cb,
                             void *userdata);

    /* delete a channel */
    int32_t (*delete_channel) (uint32_t camera_handle,
                               uint32_t ch_id);

    /* Add a new stream into a channel
     * return stream_id. zero is invalid stream_id */
    uint32_t (*add_stream) (uint32_t camera_handle,
                            uint32_t ch_id);

    /* delete a stream from a channel */
    int32_t (*delete_stream) (uint32_t camera_handle,
                              uint32_t ch_id,
                              uint32_t stream_id);

    /* configure a stream */
    int32_t (*config_stream) (uint32_t camera_handle,
                              uint32_t ch_id,
                              uint32_t stream_id,
                              mm_camera_stream_config_t *config);

    /* map or unmap buf of stream
     * buf_type: CAM_MAPPING_BUF_TYPE_STREAM_BUF
     *           CAM_MAPPING_BUF_TYPE_STREAM_INFO
     * idx: frame index
     */
    int32_t (*map_stream_buf) (uint32_t camera_handle,
                               uint32_t ch_id,
                               uint32_t stream_id,
                               uint8_t buf_type,
                               uint32_t idx,
                               int fd,
                               uint32_t size);
    int32_t (*unmap_stream_buf) (uint32_t camera_handle,
                                 uint32_t ch_id,
                                 uint32_t stream_id,
                                 uint8_t buf_type,
                                 uint32_t idx);

    /* set stream parameters, would assume parm buf is already mapped,
     * and according parameter information is filled in the buf before this call */
    int32_t (*set_stream_parms) (uint32_t camera_handle,
                                 uint32_t ch_id,
                                 uint32_t s_id,
                                 void *parms); /* TODO: change void * to valid data struct of set_parms */

    /* get stream parameters, would assume parm buf is already mapped,
     * and according parameter information is filled in the buf before this call */
    int32_t (*get_stream_parms) (uint32_t camera_handle,
                                 uint32_t ch_id,
                                 uint32_t s_id,
                                 void *parms); /* TODO: change void * to valid data struct of set_parms */

    /* perform an action on stream, would assume parm buf is already mapped,
     * and according action information is filled in the buf before this call */
    int32_t (*do_stream_action) (uint32_t camera_handle,
                                 uint32_t ch_id,
                                 uint32_t s_id,
                                 void *actions); /* TODO: change void * to valid data struct of actions*/

    /* start channel.
     * this call will start all streams belongs to the channel */
    int32_t (*start_channel) (uint32_t camera_handle,
                              uint32_t ch_id);

    /* stop channel.
     * this call will stop all streams belongs to the channel */
    int32_t (*stop_channel) (uint32_t camera_handle,
                             uint32_t ch_id);

    /* queue buf to kernel */
    int32_t (*qbuf) (uint32_t camera_handle,
                     uint32_t ch_id,
                     mm_camera_buf_def_t *buf);

    /* get super bufs. for burst mode only */
    int32_t (*request_super_buf) (uint32_t camera_handle,
                                  uint32_t ch_id,
                                  uint32_t num_buf_requested);

    /* abort the super buf dispatching. for burst mode only  */
    int32_t (*cancel_super_buf_request) (uint32_t camera_handle,
                                         uint32_t ch_id);

} mm_camera_ops_t;

typedef struct {
    uint32_t camera_handle;        /* camera object handle */
    mm_camera_ops_t *ops;          /* API call table */
} mm_camera_vtbl_t;

/* return number of cameras */
uint8_t get_num_of_cameras();

/* return reference pointer of camera vtbl */
mm_camera_vtbl_t * camera_open(uint8_t camera_idx);

#endif /*__MM_CAMERA_INTERFACE_H__*/
