LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS := -DLOG_TAG=\"MotoSensors\"
LOCAL_SRC_FILES := SensorBase.cpp sensors.c nusensors.cpp stm401_hal.cpp
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := liblog libcutils libz
LOCAL_C_INCLUDES := external/zlib
LOCAL_MODULE := sensors.shamu
include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_SRC_FILES := sensorhub.c
LOCAL_SHARED_LIBRARIES := libcutils libc
LOCAL_MODULE := sensorhub.shamu
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_REQUIRED_MODULES := sensorhub.shamu
LOCAL_REQUIRED_MODULES += sensors.shamu
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := -DLOG_TAG=\"STM401\"
LOCAL_SRC_FILES:= stm401.cpp
LOCAL_MODULE_OWNER := google
LOCAL_MODULE:= stm401
LOCAL_SHARED_LIBRARIES := libcutils libc
include $(BUILD_EXECUTABLE)
