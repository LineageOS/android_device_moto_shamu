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

#include <utils/Log.h>
#include <utils/Errors.h>
#include "QCameraQueue.h"

namespace android {

QCameraQueue::QCameraQueue()
{
    pthread_mutex_init(&m_lock, NULL);
    cam_list_init(&m_head.list);
    m_size = 0;
    m_dataFn = NULL;
    m_userData = NULL;
}

QCameraQueue::QCameraQueue(release_data_fn data_rel_fn, void *user_data)
{
    pthread_mutex_init(&m_lock, NULL);
    cam_list_init(&m_head.list);
    m_size = 0;
    m_dataFn = data_rel_fn;
    m_userData = user_data;
}

QCameraQueue::~QCameraQueue()
{
    flush();
    pthread_mutex_destroy(&m_lock);
}

bool QCameraQueue::isEmpty()
{
    bool flag = true;
    pthread_mutex_lock(&m_lock);
    if (m_size > 0) {
        flag = false;
    }
    pthread_mutex_unlock(&m_lock);
    return flag;
}

bool QCameraQueue::enqueue(void *data)
{
    camera_q_node *node =
        (camera_q_node *)malloc(sizeof(camera_q_node));
    if (NULL == node) {
        ALOGE("%s: No memory for camera_q_node", __func__);
        return false;
    }

    memset(node, 0, sizeof(camera_q_node));
    node->data = data;

    pthread_mutex_lock(&m_lock);
    cam_list_add_tail_node(&node->list, &m_head.list);
    m_size++;
    pthread_mutex_unlock(&m_lock);
    return true;
}

/* priority queue, will insert into the head of the queue */
bool QCameraQueue::enqueueWithPriority(void *data)
{
    camera_q_node *node =
        (camera_q_node *)malloc(sizeof(camera_q_node));
    if (NULL == node) {
        ALOGE("%s: No memory for camera_q_node", __func__);
        return false;
    }

    memset(node, 0, sizeof(camera_q_node));
    node->data = data;

    pthread_mutex_lock(&m_lock);
    struct cam_list *p_next = m_head.list.next;

    m_head.list.next = &node->list;
    p_next->prev = &node->list;
    node->list.next = p_next;
    node->list.prev = &m_head.list;

    m_size++;
    pthread_mutex_unlock(&m_lock);
    return true;
}

void* QCameraQueue::dequeue()
{
    camera_q_node* node = NULL;
    void* data = NULL;
    struct cam_list *head = NULL;
    struct cam_list *pos = NULL;

    pthread_mutex_lock(&m_lock);
    head = &m_head.list;
    pos = head->next;
    if (pos != head) {
        node = member_of(pos, camera_q_node, list);
        cam_list_del_node(&node->list);
        m_size--;
    }
    pthread_mutex_unlock(&m_lock);

    if (NULL != node) {
        data = node->data;
        free(node);
    }

    return data;
}

void QCameraQueue::flush(){
    camera_q_node* node = NULL;
    struct cam_list *head = NULL;
    struct cam_list *pos = NULL;

    pthread_mutex_lock(&m_lock);
    head = &m_head.list;
    pos = head->next;

    while(pos != head) {
        node = member_of(pos, camera_q_node, list);
        pos = pos->next;
        cam_list_del_node(&node->list);
        m_size--;

        if (NULL != node->data) {
            if (m_dataFn) {
                m_dataFn(node->data, m_userData);
            }
            free(node->data);
        }
        free(node);

    }
    m_size = 0;
    pthread_mutex_unlock(&m_lock);
}

}; // namespace android
