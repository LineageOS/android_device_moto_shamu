LOCAL_PATH:= $(call my-dir)

HAL      := src/hal
UDRV     := src/udrv
HALIMPL  := halimpl/bcm2079x
D_CFLAGS := \
    -DANDROID -DBUILDCFG=1 \
    -Wno-deprecated-register \
    -Wno-unused-parameter \
    -Wno-missing-field-initializers \
    -Wno-unused-function \
    -Wno-unused-variable \
    -Wno-macro-redefined

include $(CLEAR_VARS)

LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_VENDOR_MODULE := true

LOCAL_MODULE := nfc_nci.bcm2079x.$(TARGET_BOARD_PLATFORM)

LOCAL_SRC_FILES := \
    $(call all-c-files-under, $(HALIMPL)) \
    $(call all-cpp-files-under, $(HALIMPL)) \
    src/adaptation/CrcChecksum.cpp \
    src//nfca_version.c

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libhwbinder

LOCAL_HEADER_LIBRARIES := \
    libhardware_headers \
    libhardware_legacy_headers

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/$(HALIMPL)/include \
    $(LOCAL_PATH)/$(HALIMPL)/gki/ulinux \
    $(LOCAL_PATH)/$(HALIMPL)/gki/common \
    $(LOCAL_PATH)/$(HAL)/include \
    $(LOCAL_PATH)/$(HAL)/int \
    $(LOCAL_PATH)/src/include \
    $(LOCAL_PATH)/$(UDRV)/include

LOCAL_CFLAGS := \
    $(D_CFLAGS) \
    -DNFC_HAL_TARGET=TRUE \
    -DNFC_RW_ONLY=TRUE

include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
