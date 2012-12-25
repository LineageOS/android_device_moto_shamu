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

#include <pthread.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <semaphore.h>

#include "mm_camera_dbg.h"
#include "mm_camera_sock.h"
#include "mm_camera_interface.h"
#include "mm_camera.h"

#define SET_PARM_BIT32(parm, parm_arr) \
    (parm_arr[parm/32] |= (1<<(parm%32)))

#define GET_PARM_BIT32(parm, parm_arr) \
    ((parm_arr[parm/32]>>(parm%32))& 0x1)

/* internal function declare */
int32_t mm_camera_evt_sub(mm_camera_obj_t * my_obj,
                          uint8_t reg_flag);
int32_t mm_camera_enqueue_evt(mm_camera_obj_t *my_obj,
                              mm_camera_event_t *event);

mm_channel_t * mm_camera_util_get_channel_by_handler(
                                    mm_camera_obj_t * cam_obj,
                                    uint32_t handler)
{
    int i;
    mm_channel_t *ch_obj = NULL;
    for(i = 0; i < MM_CAMERA_CHANNEL_MAX; i++) {
        if (handler == cam_obj->ch[i].my_hdl) {
            ch_obj = &cam_obj->ch[i];
            break;
        }
    }
    return ch_obj;
}

static void mm_camera_dispatch_app_event(mm_camera_cmdcb_t *cmd_cb,
                                         void* user_data)
{
    int i;
    mm_camera_event_t *event = &cmd_cb->u.evt;
    mm_camera_obj_t * my_obj = (mm_camera_obj_t *)user_data;
    if (NULL != my_obj) {
        pthread_mutex_lock(&my_obj->cb_lock);
        for(i = 0; i < MM_CAMERA_EVT_ENTRY_MAX; i++) {
            if(my_obj->evt.evt[i].evt_cb) {
                my_obj->evt.evt[i].evt_cb(
                    my_obj->my_hdl,
                    event,
                    my_obj->evt.evt[i].user_data);
            }
        }
        pthread_mutex_unlock(&my_obj->cb_lock);
    }
}

static void mm_camera_event_notify(void* user_data)
{
    struct v4l2_event ev;
    struct msm_v4l2_event_data *msm_evt = NULL;
    int rc;
    mm_camera_cmdcb_t *node = NULL;

    mm_camera_obj_t *my_obj = (mm_camera_obj_t*)user_data;
    if (NULL != my_obj) {
        /* read evt */
        memset(&ev, 0, sizeof(ev));
        rc = ioctl(my_obj->ctrl_fd, VIDIOC_DQEVENT, &ev);

        if (rc >= 0 && ev.id == MSM_CAMERA_MSM_NOTIFY) {
            msm_evt = (struct msm_v4l2_event_data *)ev.u.data;
            switch (msm_evt->command) {
            case CAM_EVENT_TYPE_MAP_UNMAP_DONE:
                pthread_mutex_lock(&my_obj->evt_lock);
                my_obj->evt_rcvd.server_event_type = msm_evt->command;
                my_obj->evt_rcvd.status = msm_evt->status;
                pthread_cond_signal(&my_obj->evt_cond);
                pthread_mutex_unlock(&my_obj->evt_lock);
                break;
            case CAM_EVENT_TYPE_AUTO_FOCUS_DONE:
            case CAM_EVENT_TYPE_ZOOM_DONE:
                {
                    node = (mm_camera_cmdcb_t *)malloc(sizeof(mm_camera_cmdcb_t));
                    if (NULL != node) {
                        memset(node, 0, sizeof(mm_camera_cmdcb_t));
                        node->cmd_type = MM_CAMERA_CMD_TYPE_EVT_CB;
                        node->u.evt.server_event_type = msm_evt->command;
                        if (msm_evt->status == MSM_CAMERA_STATUS_SUCCESS) {
                            node->u.evt.status = CAM_STATUS_SUCCESS;
                        } else {
                            node->u.evt.status = CAM_STATUS_FAILED;
                        }
                    }
                }
                break;
            default:
                break;
            }
            if (NULL != node) {
                /* enqueue to evt cmd thread */
                cam_queue_enq(&(my_obj->evt_thread.cmd_queue), node);
                /* wake up evt cmd thread */
                sem_post(&(my_obj->evt_thread.cmd_sem));
            }
        }
    }
}

int32_t mm_camera_enqueue_evt(mm_camera_obj_t *my_obj,
                              mm_camera_event_t *event)
{
    int32_t rc = 0;
    mm_camera_cmdcb_t *node = NULL;

    node = (mm_camera_cmdcb_t *)malloc(sizeof(mm_camera_cmdcb_t));
    if (NULL != node) {
        memset(node, 0, sizeof(mm_camera_cmdcb_t));
        node->cmd_type = MM_CAMERA_CMD_TYPE_EVT_CB;
        memcpy(&node->u.evt, event, sizeof(mm_camera_event_t));

        /* enqueue to evt cmd thread */
        cam_queue_enq(&(my_obj->evt_thread.cmd_queue), node);
        /* wake up evt cmd thread */
        sem_post(&(my_obj->evt_thread.cmd_sem));
    } else {
        CDBG_ERROR("%s: No memory for mm_camera_node_t", __func__);
        rc = -1;
    }

    return rc;
}

int32_t mm_camera_open(mm_camera_obj_t *my_obj)
{
    char dev_name[MM_CAMERA_DEV_NAME_LEN];
    int32_t rc = 0;
    int8_t n_try=MM_CAMERA_DEV_OPEN_TRIES;
    uint8_t sleep_msec=MM_CAMERA_DEV_OPEN_RETRY_SLEEP;
    unsigned int cam_idx = 0;

    CDBG("%s:  begin\n", __func__);

    snprintf(dev_name, sizeof(dev_name), "/dev/%s",
             mm_camera_util_get_dev_name(my_obj->my_hdl));
    sscanf(dev_name, "/dev/video%u", &cam_idx);
    CDBG_ERROR("%s: dev name = %s, cam_idx = %d", __func__, dev_name, cam_idx);

    do{
        n_try--;
        my_obj->ctrl_fd = open(dev_name, O_RDWR | O_NONBLOCK);
        CDBG("%s:  ctrl_fd = %d, errno == %d", __func__, my_obj->ctrl_fd, errno);
        if((my_obj->ctrl_fd > 0) || (errno != EIO) || (n_try <= 0 )) {
            CDBG_ERROR("%s:  opened, break out while loop", __func__);
            break;
        }
        CDBG("%s:failed with I/O error retrying after %d milli-seconds",
             __func__, sleep_msec);
        usleep(sleep_msec*1000);
    }while(n_try>0);

    if (my_obj->ctrl_fd <= 0) {
        CDBG_ERROR("%s: cannot open control fd of '%s' (%s)\n",
                 __func__, dev_name, strerror(errno));
        rc = -1;
        goto on_error;
    }

    /* open domain socket*/
    n_try = MM_CAMERA_DEV_OPEN_TRIES;
    do {
        n_try--;
        my_obj->ds_fd = mm_camera_socket_create(cam_idx, MM_CAMERA_SOCK_TYPE_UDP);
        CDBG("%s:  ds_fd = %d, errno = %d", __func__, my_obj->ds_fd, errno);
        if((my_obj->ds_fd > 0) || (n_try <= 0 )) {
            CDBG("%s:  opened, break out while loop", __func__);
            break;
        }
        CDBG("%s:failed with I/O error retrying after %d milli-seconds",
             __func__, sleep_msec);
        usleep(sleep_msec * 1000);
    } while (n_try > 0);

    if (my_obj->ds_fd <= 0) {
        CDBG_ERROR("%s: cannot open domain socket fd of '%s'(%s)\n",
                 __func__, dev_name, strerror(errno));
        rc = -1;
        goto on_error;
    }

    pthread_mutex_init(&my_obj->cb_lock, NULL);
    pthread_mutex_init(&my_obj->evt_lock, NULL);
    pthread_cond_init(&my_obj->evt_cond, NULL);

    CDBG("%s : Launch evt Thread in Cam Open",__func__);
    mm_camera_cmd_thread_launch(&my_obj->evt_thread,
                                mm_camera_dispatch_app_event,
                                (void *)my_obj);

    /* launch event poll thread
     * we will add evt fd into event poll thread upon user first register for evt */
    CDBG("%s : Launch evt Poll Thread in Cam Open", __func__);
    mm_camera_poll_thread_launch(&my_obj->evt_poll_thread,
                                 MM_CAMERA_POLL_TYPE_EVT);
    mm_camera_evt_sub(my_obj, TRUE);

    CDBG("%s:  end (rc = %d)\n", __func__, rc);
    /* we do not need to unlock cam_lock here before return
     * because for open, it's done within intf_lock */
    return rc;

on_error:
    if (my_obj->ctrl_fd > 0) {
        close(my_obj->ctrl_fd);
        my_obj->ctrl_fd = 0;
    }
    if (my_obj->ds_fd > 0) {
        mm_camera_socket_close(my_obj->ds_fd);
       my_obj->ds_fd = 0;
    }

    /* we do not need to unlock cam_lock here before return
     * because for open, it's done within intf_lock */
    return rc;
}

int32_t mm_camera_close(mm_camera_obj_t *my_obj)
{
    CDBG("%s : unsubscribe evt", __func__);
    mm_camera_evt_sub(my_obj, FALSE);

    CDBG("%s : Close evt Poll Thread in Cam Close",__func__);
    mm_camera_poll_thread_release(&my_obj->evt_poll_thread);

    CDBG("%s : Close evt cmd Thread in Cam Close",__func__);
    mm_camera_cmd_thread_release(&my_obj->evt_thread);

    if(my_obj->ctrl_fd > 0) {
        close(my_obj->ctrl_fd);
        my_obj->ctrl_fd = -1;
    }
    if(my_obj->ds_fd > 0) {
        mm_camera_socket_close(my_obj->ds_fd);
        my_obj->ds_fd = -1;
    }

    pthread_mutex_destroy(&my_obj->cb_lock);
    pthread_mutex_destroy(&my_obj->evt_lock);
    pthread_cond_destroy(&my_obj->evt_cond);

    pthread_mutex_unlock(&my_obj->cam_lock);
    return 0;
}

int32_t mm_camera_register_event_notify_internal(mm_camera_obj_t *my_obj,
                                                 mm_camera_event_notify_t evt_cb,
                                                 void * user_data)
{
    int i;
    int rc = -1;
    mm_camera_evt_obj_t *evt_array = NULL;

    pthread_mutex_lock(&my_obj->cb_lock);
    evt_array = &my_obj->evt;
    if(evt_cb) {
        /* this is reg case */
        for(i = 0; i < MM_CAMERA_EVT_ENTRY_MAX; i++) {
            if(evt_array->evt[i].user_data == NULL) {
                evt_array->evt[i].evt_cb = evt_cb;
                evt_array->evt[i].user_data = user_data;
                evt_array->reg_count++;
                rc = 0;
                break;
            }
        }
    } else {
        /* this is unreg case */
        for(i = 0; i < MM_CAMERA_EVT_ENTRY_MAX; i++) {
            if(evt_array->evt[i].user_data == user_data) {
                evt_array->evt[i].evt_cb = NULL;
                evt_array->evt[i].user_data = NULL;
                evt_array->reg_count--;
                rc = 0;
                break;
            }
        }
    }

    pthread_mutex_unlock(&my_obj->cb_lock);
    return rc;
}

int32_t mm_camera_register_event_notify(mm_camera_obj_t *my_obj,
                                        mm_camera_event_notify_t evt_cb,
                                        void * user_data)
{
    int rc = -1;
    rc = mm_camera_register_event_notify_internal(my_obj,
                                                  evt_cb,
                                                  user_data);
    pthread_mutex_unlock(&my_obj->cam_lock);
    return rc;
}

int32_t mm_camera_qbuf(mm_camera_obj_t *my_obj,
                       uint32_t ch_id,
                       mm_camera_buf_def_t *buf)
{
    int rc = -1;
    mm_channel_t * ch_obj = NULL;
    ch_obj = mm_camera_util_get_channel_by_handler(my_obj, ch_id);

    pthread_mutex_unlock(&my_obj->cam_lock);

    /* we always assume qbuf will be done before channel/stream is fully stopped
     * because qbuf is done within dataCB context
     * in order to avoid deadlock, we are not locking ch_lock for qbuf */
    if (NULL != ch_obj) {
        rc = mm_channel_qbuf(ch_obj, buf);
    }

    return rc;
}

int32_t mm_camera_query_capability(mm_camera_obj_t *my_obj)
{
    int32_t rc = 0;
    struct v4l2_capability cap;

    /* get camera capabilities */
    memset(&cap, 0, sizeof(cap));
    rc = ioctl(my_obj->ctrl_fd, VIDIOC_QUERYCAP, &cap);
    if (rc != 0) {
        CDBG_ERROR("%s: cannot get camera capabilities, rc = %d\n", __func__, rc);
    }

    pthread_mutex_unlock(&my_obj->cam_lock);
    return rc;

}

int32_t mm_camera_set_parms(mm_camera_obj_t *my_obj,
                            set_parm_buffer_t *parms)
{
    int32_t rc = -1;
    if (parms !=  NULL) {
        rc = mm_camera_util_s_ctrl(my_obj->ctrl_fd, 0, 0);
    }
    pthread_mutex_unlock(&my_obj->cam_lock);
    return rc;
}

int32_t mm_camera_get_parms(mm_camera_obj_t *my_obj,
                            get_parm_buffer_t *parms)
{
    int32_t rc = -1;
    if (parms != NULL) {
        rc = mm_camera_util_g_ctrl(my_obj->ctrl_fd, 0, 0);
    }
    pthread_mutex_unlock(&my_obj->cam_lock);
    return rc;
}

int32_t mm_camera_do_action(mm_camera_obj_t *my_obj,
                            set_parm_buffer_t *actions)
{
    int32_t rc = -1;
    if (actions != NULL) {
        rc = mm_camera_util_s_ctrl(my_obj->ctrl_fd, 0, 0);
    }
    pthread_mutex_unlock(&my_obj->cam_lock);
    return rc;
}

uint32_t mm_camera_add_channel(mm_camera_obj_t *my_obj,
                               mm_camera_channel_attr_t *attr,
                               mm_camera_buf_notify_t channel_cb,
                               void *userdata)
{
    mm_channel_t *ch_obj = NULL;
    uint8_t ch_idx = 0;
    uint32_t ch_hdl = 0;

    for(ch_idx = 0; ch_idx < MM_CAMERA_CHANNEL_MAX; ch_idx++) {
        if (MM_CHANNEL_STATE_NOTUSED == my_obj->ch[ch_idx].state) {
            ch_obj = &my_obj->ch[ch_idx];
            break;
        }
    }

    if (NULL != ch_obj) {
        /* initialize channel obj */
        memset(ch_obj, 0, sizeof(mm_channel_t));
        ch_hdl = mm_camera_util_generate_handler(ch_idx);
        ch_obj->my_hdl = ch_hdl;
        ch_obj->state = MM_CHANNEL_STATE_STOPPED;
        ch_obj->cam_obj = my_obj;
        pthread_mutex_init(&ch_obj->ch_lock, NULL);
    }

    mm_channel_init(ch_obj, attr, channel_cb, userdata);
    pthread_mutex_unlock(&my_obj->cam_lock);

    return ch_hdl;
}

int32_t mm_camera_del_channel(mm_camera_obj_t *my_obj,
                              uint32_t ch_id)
{
    int32_t rc = -1;
    mm_channel_t * ch_obj =
        mm_camera_util_get_channel_by_handler(my_obj, ch_id);

    if (NULL != ch_obj) {
        pthread_mutex_lock(&ch_obj->ch_lock);
        pthread_mutex_unlock(&my_obj->cam_lock);

        rc = mm_channel_fsm_fn(ch_obj,
                               MM_CHANNEL_EVT_DELETE,
                               NULL,
                               NULL);

        pthread_mutex_destroy(&ch_obj->ch_lock);
        memset(ch_obj, 0, sizeof(mm_channel_t));
    } else {
        pthread_mutex_unlock(&my_obj->cam_lock);
    }
    return rc;
}

uint32_t mm_camera_add_stream(mm_camera_obj_t *my_obj,
                              uint32_t ch_id)
{
    uint32_t s_hdl = 0;
    mm_channel_t * ch_obj =
        mm_camera_util_get_channel_by_handler(my_obj, ch_id);

    if (NULL != ch_obj) {
        pthread_mutex_lock(&ch_obj->ch_lock);
        pthread_mutex_unlock(&my_obj->cam_lock);

        mm_channel_fsm_fn(ch_obj,
                          MM_CHANNEL_EVT_ADD_STREAM,
                          NULL,
                          (void*)&s_hdl);
    } else {
        pthread_mutex_unlock(&my_obj->cam_lock);
    }

    return s_hdl;
}

int32_t mm_camera_del_stream(mm_camera_obj_t *my_obj,
                             uint32_t ch_id,
                             uint32_t stream_id)
{
    int32_t rc = -1;
    mm_channel_t * ch_obj =
        mm_camera_util_get_channel_by_handler(my_obj, ch_id);

    if (NULL != ch_obj) {
        pthread_mutex_lock(&ch_obj->ch_lock);
        pthread_mutex_unlock(&my_obj->cam_lock);

        rc = mm_channel_fsm_fn(ch_obj,
                               MM_CHANNEL_EVT_DEL_STREAM,
                               (void*)stream_id,
                               NULL);
    } else {
        pthread_mutex_unlock(&my_obj->cam_lock);
    }

    return rc;
}

int32_t mm_camera_config_stream(mm_camera_obj_t *my_obj,
                                uint32_t ch_id,
                                uint32_t stream_id,
                                mm_camera_stream_config_t *config)
{
    int32_t rc = -1;
    mm_channel_t * ch_obj =
        mm_camera_util_get_channel_by_handler(my_obj, ch_id);
    mm_evt_paylod_config_stream_t payload;

    if (NULL != ch_obj) {
        pthread_mutex_lock(&ch_obj->ch_lock);
        pthread_mutex_unlock(&my_obj->cam_lock);

        memset(&payload, 0, sizeof(mm_evt_paylod_config_stream_t));
        payload.stream_id = stream_id;
        payload.config = config;
        rc = mm_channel_fsm_fn(ch_obj,
                               MM_CHANNEL_EVT_CONFIG_STREAM,
                               (void*)&payload,
                               NULL);
    } else {
        pthread_mutex_unlock(&my_obj->cam_lock);
    }

    return rc;
}

int32_t mm_camera_start_channel(mm_camera_obj_t *my_obj,
                                uint32_t ch_id)
{
    int32_t rc = -1;
    mm_channel_t * ch_obj =
        mm_camera_util_get_channel_by_handler(my_obj, ch_id);

    if (NULL != ch_obj) {
        pthread_mutex_lock(&ch_obj->ch_lock);
        pthread_mutex_unlock(&my_obj->cam_lock);

        rc = mm_channel_fsm_fn(ch_obj,
                               MM_CHANNEL_EVT_START,
                               NULL,
                               NULL);
    } else {
        pthread_mutex_unlock(&my_obj->cam_lock);
    }

    return rc;
}

int32_t mm_camera_stop_channel(mm_camera_obj_t *my_obj,
                               uint32_t ch_id)
{
    int32_t rc = 0;
    mm_channel_t * ch_obj =
        mm_camera_util_get_channel_by_handler(my_obj, ch_id);

    if (NULL != ch_obj) {
        pthread_mutex_lock(&ch_obj->ch_lock);
        pthread_mutex_unlock(&my_obj->cam_lock);

        rc = mm_channel_fsm_fn(ch_obj,
                               MM_CHANNEL_EVT_STOP,
                               NULL,
                               NULL);
    } else {
        pthread_mutex_unlock(&my_obj->cam_lock);
    }
    return rc;
}

int32_t mm_camera_request_super_buf(mm_camera_obj_t *my_obj,
                                    uint32_t ch_id,
                                    uint32_t num_buf_requested)
{
    int32_t rc = -1;
    mm_channel_t * ch_obj =
        mm_camera_util_get_channel_by_handler(my_obj, ch_id);

    if (NULL != ch_obj) {
        pthread_mutex_lock(&ch_obj->ch_lock);
        pthread_mutex_unlock(&my_obj->cam_lock);

        rc = mm_channel_fsm_fn(ch_obj,
                               MM_CHANNEL_EVT_REQUEST_SUPER_BUF,
                               (void*)num_buf_requested,
                               NULL);
    } else {
        pthread_mutex_unlock(&my_obj->cam_lock);
    }

    return rc;
}

int32_t mm_camera_cancel_super_buf_request(mm_camera_obj_t *my_obj, uint32_t ch_id)
{
    int32_t rc = -1;
    mm_channel_t * ch_obj =
        mm_camera_util_get_channel_by_handler(my_obj, ch_id);

    if (NULL != ch_obj) {
        pthread_mutex_lock(&ch_obj->ch_lock);
        pthread_mutex_unlock(&my_obj->cam_lock);

        rc = mm_channel_fsm_fn(ch_obj,
                               MM_CHANNEL_EVT_CANCEL_REQUEST_SUPER_BUF,
                               NULL,
                               NULL);
    } else {
        pthread_mutex_unlock(&my_obj->cam_lock);
    }

    return rc;
}

int32_t mm_camera_set_stream_parms(mm_camera_obj_t *my_obj,
                                   uint32_t ch_id,
                                   uint32_t s_id,
                                   void *parms)
{
    int32_t rc = -1;
    mm_evt_paylod_set_get_stream_parms_t payload;
    mm_channel_t * ch_obj =
        mm_camera_util_get_channel_by_handler(my_obj, ch_id);

    if (NULL != ch_obj) {
        pthread_mutex_lock(&ch_obj->ch_lock);
        pthread_mutex_unlock(&my_obj->cam_lock);

        memset(&payload, 0, sizeof(payload));
        payload.stream_id = s_id;
        payload.parms = parms;

        rc = mm_channel_fsm_fn(ch_obj,
                               MM_CHANNEL_EVT_SET_STREAM_PARM,
                               (void *)&payload,
                               NULL);
    } else {
        pthread_mutex_unlock(&my_obj->cam_lock);
    }

    return rc;
}

int32_t mm_camera_get_stream_parms(mm_camera_obj_t *my_obj,
                                   uint32_t ch_id,
                                   uint32_t s_id,
                                   void *parms)
{
    int32_t rc = -1;
    mm_evt_paylod_set_get_stream_parms_t payload;
    mm_channel_t * ch_obj =
        mm_camera_util_get_channel_by_handler(my_obj, ch_id);

    if (NULL != ch_obj) {
        pthread_mutex_lock(&ch_obj->ch_lock);
        pthread_mutex_unlock(&my_obj->cam_lock);

        memset(&payload, 0, sizeof(payload));
        payload.stream_id = s_id;
        payload.parms = parms;

        rc = mm_channel_fsm_fn(ch_obj,
                               MM_CHANNEL_EVT_GET_STREAM_PARM,
                               (void *)&payload,
                               NULL);
    } else {
        pthread_mutex_unlock(&my_obj->cam_lock);
    }

    return rc;
}

int32_t mm_camera_map_stream_buf(mm_camera_obj_t *my_obj,
                                 uint32_t ch_id,
                                 uint32_t stream_id,
                                 uint8_t buf_type,
                                 uint32_t idx,
                                 int fd,
                                 uint32_t size)
{
    int32_t rc = -1;
    mm_evt_paylod_map_stream_buf_t payload;
    mm_channel_t * ch_obj =
        mm_camera_util_get_channel_by_handler(my_obj, ch_id);

    if (NULL != ch_obj) {
        pthread_mutex_lock(&ch_obj->ch_lock);
        pthread_mutex_unlock(&my_obj->cam_lock);

        memset(&payload, 0, sizeof(payload));
        payload.stream_id = stream_id;
        payload.buf_type = buf_type;
        payload.idx = idx;
        payload.fd = fd;
        payload.size = size;
        rc = mm_channel_fsm_fn(ch_obj,
                               MM_CHANNEL_EVT_MAP_STREAM_BUF,
                               (void*)&payload,
                               NULL);
    } else {
        pthread_mutex_unlock(&my_obj->cam_lock);
    }

    return rc;
}

int32_t mm_camera_unmap_stream_buf(mm_camera_obj_t *my_obj,
                                   uint32_t ch_id,
                                   uint32_t stream_id,
                                   uint8_t buf_type,
                                   int idx)
{
    int32_t rc = -1;
    mm_evt_paylod_unmap_stream_buf_t payload;
    mm_channel_t * ch_obj =
        mm_camera_util_get_channel_by_handler(my_obj, ch_id);

    if (NULL != ch_obj) {
        pthread_mutex_lock(&ch_obj->ch_lock);
        pthread_mutex_unlock(&my_obj->cam_lock);

        memset(&payload, 0, sizeof(payload));
        payload.stream_id = stream_id;
        payload.buf_type = buf_type;
        payload.idx = idx;
        rc = mm_channel_fsm_fn(ch_obj,
                               MM_CHANNEL_EVT_UNMAP_STREAM_BUF,
                               (void*)&payload,
                               NULL);
    } else {
        pthread_mutex_unlock(&my_obj->cam_lock);
    }

    return rc;
}

int32_t mm_camera_do_stream_action(mm_camera_obj_t *my_obj,
                                   uint32_t ch_id,
                                   uint32_t stream_id,
                                   void *actions)
{
    int32_t rc = -1;
    mm_evt_paylod_do_stream_action_t payload;
    mm_channel_t * ch_obj =
        mm_camera_util_get_channel_by_handler(my_obj, ch_id);

    if (NULL != ch_obj) {
        pthread_mutex_lock(&ch_obj->ch_lock);
        pthread_mutex_unlock(&my_obj->cam_lock);

        memset(&payload, 0, sizeof(payload));
        payload.stream_id = stream_id;
        payload.actions = actions;

        rc = mm_channel_fsm_fn(ch_obj,
                               MM_CHANNEL_EVT_DO_STREAM_ACTION,
                               (void*)&payload,
                               NULL);
    } else {
        pthread_mutex_unlock(&my_obj->cam_lock);
    }

    return rc;
}

int32_t mm_camera_evt_sub(mm_camera_obj_t * my_obj,
                      uint8_t reg_flag)
{
    int32_t rc = 0;
    struct v4l2_event_subscription sub;

    memset(&sub, 0, sizeof(sub));
    sub.type = MSM_CAMERA_V4L2_EVENT_TYPE;
    sub.id = MSM_CAMERA_MSM_NOTIFY;
    if(FALSE == reg_flag) {
        /* unsubscribe */
        rc = ioctl(my_obj->ctrl_fd, VIDIOC_UNSUBSCRIBE_EVENT, &sub);
        if (rc < 0) {
            CDBG_ERROR("%s: unsubscribe event rc = %d", __func__, rc);
            return rc;
        }
        /* remove evt fd from the polling thraed when unreg the last event */
        rc = mm_camera_poll_thread_del_poll_fd(&my_obj->evt_poll_thread,
                                               my_obj->my_hdl);
    } else {
        rc = ioctl(my_obj->ctrl_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
        if (rc < 0) {
            CDBG_ERROR("%s: subscribe event rc = %d", __func__, rc);
            return rc;
        }
        /* add evt fd to polling thread when subscribe the first event */
        rc = mm_camera_poll_thread_add_poll_fd(&my_obj->evt_poll_thread,
                                               my_obj->my_hdl,
                                               my_obj->ctrl_fd,
                                               mm_camera_event_notify,
                                               (void*)my_obj);
    }
    return rc;
}

void mm_camera_util_wait_for_event(mm_camera_obj_t *my_obj,
                                   uint32_t evt_mask,
                                   int32_t *status)
{
    pthread_mutex_lock(&my_obj->evt_lock);
    while (!(my_obj->evt_rcvd.server_event_type & evt_mask)) {
        pthread_cond_wait(&my_obj->evt_cond, &my_obj->evt_lock);
    }
    *status = my_obj->evt_rcvd.status;
    /* reset local storage for recieved event for next event */
    memset(&my_obj->evt_rcvd, 0, sizeof(mm_camera_event_t));
    pthread_mutex_unlock(&my_obj->evt_lock);
}

int32_t mm_camera_util_sendmsg(mm_camera_obj_t *my_obj,
                               void *msg,
                               uint32_t buf_size,
                               int sendfd)
{
    int32_t rc = -1;
    int32_t status;
    if(mm_camera_socket_sendmsg(my_obj->ds_fd, msg, buf_size, sendfd) > 0) {
        /* wait for event that mapping/unmapping is done */
        mm_camera_util_wait_for_event(my_obj, CAM_EVENT_TYPE_MAP_UNMAP_DONE, &status);
        if (MSM_CAMERA_STATUS_SUCCESS == status) {
            rc = 0;
        }
    }
    return rc;
}

int32_t mm_camera_map_buf(mm_camera_obj_t *my_obj,
                          uint8_t buf_type,
                          int fd,
                          uint32_t size)
{
    int32_t rc = 0;
    cam_sock_packet_t packet;
    memset(&packet, 0, sizeof(cam_sock_packet_t));
    packet.msg_type = CAM_MAPPING_TYPE_FD_MAPPING;
    packet.payload.buf_map.type = buf_type;
    packet.payload.buf_map.fd = fd;
    packet.payload.buf_map.size = size;
    rc = mm_camera_util_sendmsg(my_obj,
                                &packet,
                                sizeof(cam_sock_packet_t),
                                fd);
    pthread_mutex_unlock(&my_obj->cam_lock);
    return rc;
}

int32_t mm_camera_unmap_buf(mm_camera_obj_t *my_obj,
                            uint8_t buf_type)
{
    int32_t rc = 0;
    cam_sock_packet_t packet;
    memset(&packet, 0, sizeof(cam_sock_packet_t));
    packet.msg_type = CAM_MAPPING_TYPE_FD_UNMAPPING;
    packet.payload.buf_unmap.type = buf_type;
    rc = mm_camera_util_sendmsg(my_obj,
                                &packet,
                                sizeof(cam_sock_packet_t),
                                0);
    pthread_mutex_unlock(&my_obj->cam_lock);
    return rc;
}

int32_t mm_camera_util_s_ctrl(int32_t fd,  uint32_t id, int32_t value)
{
    int rc = 0;
    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = id;
    control.value = value;
    rc = ioctl(fd, VIDIOC_S_CTRL, &control);

    CDBG("%s: fd=%d, S_CTRL, id=0x%x, value = 0x%x, rc = %d\n",
         __func__, fd, id, (uint32_t)value, rc);
    return (rc >= 0)? 0 : -1;
}

int32_t mm_camera_util_g_ctrl( int32_t fd, uint32_t id, int32_t value)
{
    int rc = 0;
    struct v4l2_control control;

    memset(&control, 0, sizeof(control));
    control.id = id;
    control.value = value;
    rc = ioctl(fd, VIDIOC_G_CTRL, &control);
    CDBG("%s: fd=%d, G_CTRL, id=0x%x, rc = %d\n", __func__, fd, id, rc);
    return (rc >= 0)? 0 : -1;
}
