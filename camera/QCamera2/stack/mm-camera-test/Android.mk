OLD_LOCAL_PATH := $(LOCAL_PATH)
LOCAL_PATH := $(call my-dir)

# Build command line test app: mm-qcamera-app
include $(LOCAL_PATH)/../../../common.mk
include $(CLEAR_VARS)

LOCAL_CFLAGS := \
    -DAMSS_VERSION=$(AMSS_VERSION) \
    $(mmcamera_debug_defines) \
    $(mmcamera_debug_cflags) \
    $(USE_SERVER_TREE) \
    -DUSE_ION \
    -D_ANDROID_

# mm_qcamera_main_menu.c has implicit conversion from enum to enum.
LOCAL_CLANG_CFLAGS += \
    -Wno-error=enum-conversion

LOCAL_SRC_FILES:= \
    src/mm_qcamera_main_menu.c \
    src/mm_qcamera_app.c \
    src/mm_qcamera_unit_test.c \
    src/mm_qcamera_video.c \
    src/mm_qcamera_preview.c \
    src/mm_qcamera_snapshot.c \
    src/mm_qcamera_rdi.c \
    src/mm_qcamera_reprocess.c\
    src/mm_qcamera_queue.c \
    src/mm_qcamera_socket.c \
    src/mm_qcamera_commands.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += \
    frameworks/native/include/media/openmax \
    $(LOCAL_PATH)/../common \
    $(LOCAL_PATH)/../../../mm-image-codec/qexif \
    $(LOCAL_PATH)/../../../mm-image-codec/qomx_core

LOCAL_C_INCLUDES+= $(kernel_includes)
LOCAL_ADDITIONAL_DEPENDENCIES := $(common_deps)

LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_IOMMU_HEAP_ID
LOCAL_CFLAGS += -DCAMERA_GRALLOC_HEAP_ID=GRALLOC_USAGE_PRIVATE_IOMMU_HEAP
LOCAL_CFLAGS += -DCAMERA_GRALLOC_FALLBACK_HEAP_ID=GRALLOC_USAGE_PRIVATE_IOMMU_HEAP
LOCAL_CFLAGS += -DCAMERA_ION_FALLBACK_HEAP_ID=ION_IOMMU_HEAP_ID
LOCAL_CFLAGS += -DCAMERA_GRALLOC_CACHING_ID=0
LOCAL_CFLAGS += -DNUM_RECORDING_BUFFERS=9

LOCAL_CFLAGS += \
    -Wall \
    -Werror

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libdl \
    libmmcamera_interface

LOCAL_MODULE_TAGS := optional

LOCAL_32_BIT_ONLY := true

LOCAL_MODULE:= mm-qcamera-app

include $(BUILD_EXECUTABLE)

# Build tuning library
include $(CLEAR_VARS)

LOCAL_CFLAGS := \
    -DAMSS_VERSION=$(AMSS_VERSION) \
    $(mmcamera_debug_defines) \
    $(mmcamera_debug_cflags) \
    $(USE_SERVER_TREE) \
    -DUSE_ION \
    -D_ANDROID_

# mm_qcamera_main_menu.c has implicit conversion from enum to enum.
LOCAL_CLANG_CFLAGS += \
    -Wno-error=enum-conversion

LOCAL_SRC_FILES := \
    src/mm_qcamera_main_menu.c \
    src/mm_qcamera_app.c \
    src/mm_qcamera_unit_test.c \
    src/mm_qcamera_video.c \
    src/mm_qcamera_preview.c \
    src/mm_qcamera_snapshot.c \
    src/mm_qcamera_rdi.c \
    src/mm_qcamera_reprocess.c\
    src/mm_qcamera_queue.c \
    src/mm_qcamera_socket.c \
    src/mm_qcamera_commands.c

LOCAL_C_INCLUDES:=$(LOCAL_PATH)/inc
LOCAL_C_INCLUDES+= \
        frameworks/native/include/media/openmax \
        $(LOCAL_PATH)/../common \
        $(LOCAL_PATH)/../../../mm-image-codec/qexif \
        $(LOCAL_PATH)/../../../mm-image-codec/qomx_core

LOCAL_C_INCLUDES+= $(kernel_includes)
LOCAL_ADDITIONAL_DEPENDENCIES := $(common_deps)

LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_IOMMU_HEAP_ID
LOCAL_CFLAGS += -DCAMERA_GRALLOC_HEAP_ID=GRALLOC_USAGE_PRIVATE_IOMMU_HEAP
LOCAL_CFLAGS += -DCAMERA_GRALLOC_FALLBACK_HEAP_ID=GRALLOC_USAGE_PRIVATE_IOMMU_HEAP
LOCAL_CFLAGS += -DCAMERA_ION_FALLBACK_HEAP_ID=ION_IOMMU_HEAP_ID
LOCAL_CFLAGS += -DCAMERA_GRALLOC_CACHING_ID=0
LOCAL_CFLAGS += -DNUM_RECORDING_BUFFERS=9

LOCAL_CFLAGS += \
    -Wall \
    -Werror

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libdl \
    libmmcamera_interface

LOCAL_MODULE_TAGS := optional

LOCAL_32_BIT_ONLY := true

LOCAL_MODULE := libmm-qcamera
include $(BUILD_SHARED_LIBRARY)
