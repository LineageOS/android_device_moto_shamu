LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
        QCamera2Factory.cpp QCamera2Hal.cpp QCamera2HWI.cpp QCamera2HWI_Mem.cpp QCamera2DSockParm.cpp

LOCAL_SHARED_LIBRARIES := libcamera_client liblog libhardware
LOCAL_SHARED_LIBRARIES += libmmcamera_interface3

LOCAL_CFLAGS = -Wall -Werror

LOCAL_C_INCLUDES := \
        $(LOCAL_PATH)/../stack/common/

LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE := camera2.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
