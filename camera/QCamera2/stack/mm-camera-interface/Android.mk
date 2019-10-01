OLD_LOCAL_PATH := $(LOCAL_PATH)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# cam_intf.c has type conversion discarding qualifiers.
# mm_camera_interface.c has incomplete field initializer.
LOCAL_CLANG_CFLAGS += \
    -Wno-error=incompatible-pointer-types-discards-qualifiers \
    -Wno-error=missing-field-initializers

MM_CAM_FILES := \
    src/mm_camera_interface.c \
    src/mm_camera.c \
    src/mm_camera_channel.c \
    src/mm_camera_stream.c \
    src/mm_camera_thread.c \
    src/mm_camera_sock.c \
    src/cam_intf.c

LOCAL_CFLAGS += \
    -DUSE_ION \
    -D_ANDROID_ \
    -DCAMERA_ION_HEAP_ID=ION_IOMMU_HEAP_ID \
    -DVENUS_PRESENT \
    -Wall \
    -Werror

LOCAL_COPY_HEADERS_TO := mm-camera-interface
LOCAL_COPY_HEADERS += ../common/cam_intf.h
LOCAL_COPY_HEADERS += ../common/cam_types.h

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/../common \
    system/media/camera/include

LOCAL_HEADER_LIBRARIES := generated_kernel_headers

LOCAL_C_INCLUDES += hardware/qcom/media/$(TARGET_BOARD_PLATFORM)/mm-core/inc

LOCAL_SRC_FILES := $(MM_CAM_FILES)

LOCAL_MODULE := libmmcamera_interface

LOCAL_SHARED_LIBRARIES := \
    libdl \
    libcutils \
    liblog

LOCAL_MODULE_TAGS := optional

LOCAL_32_BIT_ONLY := true
include $(BUILD_SHARED_LIBRARY)

LOCAL_PATH := $(OLD_LOCAL_PATH)
