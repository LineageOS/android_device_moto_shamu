LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libdl \
    libbase \
    libutils \
    android.hardware.power-ndk_platform \
    vendor.lineage.power-ndk_platform \
    libbinder_ndk

LOCAL_HEADER_LIBRARIES := \
    libhardware_headers

LOCAL_SRC_FILES := \
    power-common.c \
    metadata-parser.c \
    utils.c \
    list.c \
    hint-data.c \
    Power.cpp \
    LineagePower.cpp \
    main.cpp

LOCAL_CFLAGS += -Wall -Wextra -Werror

# Include target-specific files.
LOCAL_SRC_FILES += power-8084.c

ifneq ($(TARGET_TAP_TO_WAKE_NODE),)
    LOCAL_CFLAGS += -DTAP_TO_WAKE_NODE=\"$(TARGET_TAP_TO_WAKE_NODE)\"
endif

ifeq ($(TARGET_USES_INTERACTION_BOOST),true)
    LOCAL_CFLAGS += -DINTERACTION_BOOST
endif

LOCAL_MODULE := android.hardware.power-service.shamu
LOCAL_INIT_RC := android.hardware.power-service.shamu.rc
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -Wno-unused-parameter -Wno-unused-variable
LOCAL_VENDOR_MODULE := true
LOCAL_VINTF_FRAGMENTS := power.xml

include $(BUILD_EXECUTABLE)
