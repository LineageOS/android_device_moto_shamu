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

#define LOG_TAG "QCamera2HWI_Mem"

#include <string.h>
#include <fcntl.h>
#include <utils/Log.h>
#include <utils/Errors.h>
#include "QCamera2HWI_Mem.h"

namespace android {

QCamera2IonMemory::QCamera2IonMemory(camera_request_memory getMemory)
{
    mGetMemory = getMemory;
    mBufferCount = 0;
    for (int i = 0; i < MM_CAMERA_MAX_NUM_FRAMES; i++) {
        mMemInfo[i].fd = 0;
        mMemInfo[i].main_ion_fd = 0;
        mMemInfo[i].handle = NULL;
        mMemInfo[i].size = 0;
        mMemInfo[i].camera_memory = NULL;
    }
}

QCamera2IonMemory::~QCamera2IonMemory()
{
}

int QCamera2IonMemory::CacheOps(int index, unsigned int cmd)
{
    struct ion_flush_data cache_inv_data;
    struct ion_custom_data custom_data;
    int ret = OK;

    if (index >= mBufferCount) {
        ALOGE("index %d out of bound [0, %d)", index, mBufferCount);
        return BAD_INDEX;
    }

    memset(&cache_inv_data, 0, sizeof(cache_inv_data));
    memset(&custom_data, 0, sizeof(custom_data));
    cache_inv_data.vaddr = mMemInfo[index].camera_memory->data;
    cache_inv_data.fd = mMemInfo[index].fd;
    cache_inv_data.handle = mMemInfo[index].handle;
    cache_inv_data.length = mMemInfo[index].size;
    custom_data.cmd = cmd;
    custom_data.arg = (unsigned long)&cache_inv_data;

    ALOGD("addr = %p, fd = %d, handle = %p length = %d, ION Fd = %d",
         cache_inv_data.vaddr, cache_inv_data.fd,
         cache_inv_data.handle, cache_inv_data.length,
         mMemInfo[index].main_ion_fd);
    ret = ioctl(mMemInfo[index].main_ion_fd, ION_IOC_CUSTOM, &custom_data);
    if (ret < 0)
        ALOGE("%s: Cache Invalidate failed: %s\n", __func__, strerror(errno));

    return ret;
}

#if 0
void QCamera2IonMemory::GetBufDef(const mm_camera_frame_len_offset &offset,
        mm_camera_buf_def_t &bufDef, int index) const
{
    if (!mBufferCount) {
        ALOGE("Memory not allocated");
        return;
    }
    bufDef.fd = mMemInfo[index].fd;
    bufDef.frame_len = mMemInfo[index].size;
    bufDef.mem_info = (void *)index;
    bufDef.num_planes = offset.num_planes;

    /* Plane 0 needs to be set separately. Set other planes in a loop */
    bufDef.planes[0].length = offset.mp[0].len;
    bufDef.planes[0].m.userptr = mMemInfo[index].fd;
    bufDef.planes[0].data_offset = offset.mp[0].offset;
    bufDef.planes[0].reserved[0] = 0;
    for (int i = 1; i < bufDef.num_planes; i++) {
         bufDef.planes[i].length = offset.mp[i].len;
         bufDef.planes[i].m.userptr = mMemInfo[i].fd;
         bufDef.planes[i].data_offset = offset.mp[i].offset;
         bufDef.planes[i].reserved[0] =
                 bufDef.planes[i-1].reserved[0] +
                 bufDef.planes[i-1].length;
    }
}
#endif

QCamera2HeapMemory::QCamera2HeapMemory(camera_request_memory getMemory)
    : QCamera2IonMemory(getMemory)
{
}

QCamera2HeapMemory::~QCamera2HeapMemory()
{
}

int QCamera2HeapMemory::Allocate(int count, int size, int heap_id)
{
    int rc = OK;
    if (count > MM_CAMERA_MAX_NUM_FRAMES) {
        ALOGE("Buffer count %d out of bound. Max is %d", count, MM_CAMERA_MAX_NUM_FRAMES);
        return BAD_INDEX;
    }
    if (!mBufferCount) {
        ALOGE("Allocating a already allocated heap memory");
        return INVALID_OPERATION;
    }

    for (int i = 0; i < count; i ++) {
        rc = AllocateIonMemory(mMemInfo[i], heap_id, size);
        if (rc < 0) {
            ALOGE("AllocateIonMemory failed");
            for (int j = i-1; j >= 0; j--)
                DeallocateIonMemory(mMemInfo[j]);
            break;
        }
    }
    return rc;
}

void QCamera2HeapMemory::Deallocate()
{
    for (int i = 0; i < mBufferCount; i++)
        DeallocateIonMemory(mMemInfo[i]);
}

int QCamera2HeapMemory::AllocateIonMemory(QCamera2MemInfo &memInfo, int heap_id, int size)
{
    int rc = OK;
    struct ion_handle_data handle_data;
    struct ion_allocation_data alloc;
    struct ion_fd_data ion_info_fd;
    int main_ion_fd = 0;

    main_ion_fd = open("/dev/ion", O_RDONLY);
    if (main_ion_fd <= 0) {
        ALOGE("Ion dev open failed: %s\n", strerror(errno));
        goto ION_OPEN_FAILED;
    }

    memset(&alloc, 0, sizeof(alloc));
    alloc.len = size;
    /* to make it page size aligned */
    alloc.len = (alloc.len + 4095) & (~4095);
    alloc.align = 4096;
    alloc.flags = ION_FLAG_CACHED;
    alloc.heap_mask = heap_id;
    rc = ioctl(main_ion_fd, ION_IOC_ALLOC, &alloc);
    if (rc < 0) {
        ALOGE("ION allocation failed: %s\n", strerror(errno));
        goto ION_ALLOC_FAILED;
    }

    memset(&ion_info_fd, 0, sizeof(ion_info_fd));
    ion_info_fd.handle = alloc.handle;
    rc = ioctl(main_ion_fd, ION_IOC_SHARE, &ion_info_fd);
    if (rc < 0) {
        ALOGE("ION map failed %s\n", strerror(errno));
        goto ION_MAP_FAILED;
    }

    memInfo.camera_memory = mGetMemory(memInfo.fd, memInfo.size, 1, (void *)this);
    if (!memInfo.camera_memory) {
        ALOGE("mGetMemory failed");
        goto GET_MEMORY_FAILED;
    }
    memInfo.main_ion_fd = main_ion_fd;
    memInfo.fd = ion_info_fd.fd;
    memInfo.handle = ion_info_fd.handle;
    memInfo.size = alloc.len;
    return OK;

GET_MEMORY_FAILED:
ION_MAP_FAILED:
    memset(&handle_data, 0, sizeof(handle_data));
    handle_data.handle = ion_info_fd.handle;
    ioctl(main_ion_fd, ION_IOC_FREE, &handle_data);
ION_ALLOC_FAILED:
    close(main_ion_fd);
ION_OPEN_FAILED:
    return NO_MEMORY;
}

void QCamera2HeapMemory::DeallocateIonMemory(QCamera2MemInfo &memInfo)
{
    struct ion_handle_data handle_data;

    memInfo.camera_memory->release(memInfo.camera_memory);
    memInfo.camera_memory = NULL;

    if (memInfo.fd > 0) {
        close(memInfo.fd);
        memInfo.fd = 0;
    }

    if (memInfo.main_ion_fd > 0) {
        memset(&handle_data, 0, sizeof(handle_data));
        handle_data.handle = memInfo.handle;
        ioctl(memInfo.main_ion_fd, ION_IOC_FREE, &handle_data);
        close(memInfo.main_ion_fd);
        memInfo.main_ion_fd = 0;
    }
    memInfo.handle = NULL;
    memInfo.size = 0;
}

QCamera2GrallocMemory::QCamera2GrallocMemory(camera_request_memory getMemory)
        : QCamera2IonMemory(getMemory)
{
}

QCamera2GrallocMemory::~QCamera2GrallocMemory()
{
}

int QCamera2GrallocMemory::Allocate(preview_stream_ops_t * /*window*/,
        int /*count*/, int /*size*/, int /*heap_id*/)
{
    return OK;
}

void QCamera2GrallocMemory::Deallocate()
{
}

}; //namespace android
