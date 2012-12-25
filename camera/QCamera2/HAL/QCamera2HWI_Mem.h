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

#ifndef __QCAMERA2HWI_MEM_H__
#define __QCAMERA2HWI_MEM_H__

#include <hardware/camera.h>

extern "C" {
#include <sys/types.h>
#include <linux/msm_ion.h>
#include <mm_camera_interface.h>
}

namespace android {

// Base class for Heap Memory and Gralloc Memory
class QCamera2IonMemory {

public:
    int CleanCache(int index) {return CacheOps(index, ION_IOC_CLEAN_CACHES);}
    int InvalidateCache(int index) {return CacheOps(index, ION_IOC_INV_CACHES);}
    int CleanInvalidateCache(int index) {return CacheOps(index, ION_IOC_CLEAN_INV_CACHES);}

protected:
    struct QCamera2MemInfo {
        int fd;
        int main_ion_fd;
        struct ion_handle *handle;
        uint32_t size;
        camera_memory_t *camera_memory;
    };

    QCamera2IonMemory(camera_request_memory getMemory);
    virtual ~QCamera2IonMemory();
    //void GetBufDef(const mm_camera_frame_len_offset &offset,
     //           mm_camera_buf_def_t &bufDef, int index) const;

    int mBufferCount;
    struct QCamera2MemInfo mMemInfo[MM_CAMERA_MAX_NUM_FRAMES];
    camera_request_memory mGetMemory;

private:
    int CacheOps(int index, unsigned int cmd);
};

// Heap Memory is used to allocate from /dev/ion directly.
class QCamera2HeapMemory : public QCamera2IonMemory {
public:
    QCamera2HeapMemory(camera_request_memory getMemory);
    virtual ~QCamera2HeapMemory();

    int Allocate(int count, int size, int heap_id);
    void Deallocate();
private:
    int AllocateIonMemory(struct QCamera2MemInfo &memInfo, int heap_id, int size);
    void DeallocateIonMemory(struct QCamera2MemInfo &memInfo);
};

// Gralloc Memory is acquired from preview window
class QCamera2GrallocMemory : public QCamera2IonMemory {
public:
    QCamera2GrallocMemory(camera_request_memory getMemory);
    virtual ~QCamera2GrallocMemory();

    int Allocate(preview_stream_ops_t *window, int count, int size, int heap_id);
    void Deallocate();
private:
    buffer_handle_t *mBufferHandle[MM_CAMERA_MAX_NUM_FRAMES];
    int mLocalFlag[MM_CAMERA_MAX_NUM_FRAMES];
    struct private_handle_t *mPrivateHandle[MM_CAMERA_MAX_NUM_FRAMES];
    preview_stream_ops_t *mWindow;
};

}; // namespace android

#endif /* __QCAMERA2HWI_MEM_H__ */
