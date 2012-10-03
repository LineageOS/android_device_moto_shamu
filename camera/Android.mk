MM_STILL_V4L2_DRIVER_LIST := msm7627a
MM_STILL_V4L2_DRIVER_LIST += msm8660
MM_STILL_V4L2_DRIVER_LIST += msm8960
MM_STILL_V4L2_DRIVER_LIST += msm8974

ifeq ($(call is-board-platform-in-list,$(MM_STILL_V4L2_DRIVER_LIST)),true)
  ifneq ($(USE_CAMERA_STUB),true)
    ifneq ($(BUILD_TINY_ANDROID),true)
      include $(call all-subdir-makefiles)
    endif
  endif
else #ifeq ($(call is-board-platform-in-list,$(MM_STILL_V4L2_DRIVER_LIST)),true)
  ifneq ($(USE_CAMERA_STUB),true)
    # When zero we link against libmmcamera; when 1, we dlopen libmmcamera.
    DLOPEN_LIBMMCAMERA:=1
    ifneq ($(BUILD_TINY_ANDROID),true)
      V4L2_BASED_LIBCAM := true

      LOCAL_PATH:= $(call my-dir)
      LOCAL_PATH1:= $(call my-dir)

      include $(CLEAR_VARS)

      LOCAL_CFLAGS:= -DDLOPEN_LIBMMCAMERA=$(DLOPEN_LIBMMCAMERA)

      BUILD_UNIFIED_CODE := false

      ifeq ($(strip $(TARGET_USES_ION)),true)
        LOCAL_CFLAGS += -DUSE_ION
      endif

      LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CP_MM_HEAP_ID # 8660=SMI, Rest=EBI
      LOCAL_CFLAGS += -DCAMERA_ZSL_ION_HEAP_ID=ION_CP_MM_HEAP_ID
      LOCAL_CFLAGS += -DCAMERA_GRALLOC_HEAP_ID=GRALLOC_USAGE_PRIVATE_CAMERA_HEAP
      LOCAL_CFLAGS += -DCAMERA_GRALLOC_FALLBACK_HEAP_ID=GRALLOC_USAGE_PRIVATE_CAMERA_HEAP # Don't Care
      LOCAL_CFLAGS += -DCAMERA_GRALLOC_CACHING_ID=GRALLOC_USAGE_PRIVATE_UNCACHED #uncached
      LOCAL_CFLAGS += -DCAMERA_ION_FALLBACK_HEAP_ID=ION_CAMERA_HEAP_ID
      LOCAL_CFLAGS += -DCAMERA_ZSL_ION_FALLBACK_HEAP_ID=ION_CAMERA_HEAP_ID
      LOCAL_CFLAGS += -DNUM_RECORDING_BUFFERS=5

      LOCAL_HAL_FILES := QualcommCamera.cpp QualcommCameraHardware.cpp QCameraParameters.cpp

      LOCAL_CFLAGS+= -DHW_ENCODE

      LOCAL_C_INCLUDES+= $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
      LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

      # if debug service layer and up , use stub camera!
      LOCAL_C_INCLUDES += \
        frameworks/base/services/camera/libcameraservice #

      LOCAL_SRC_FILES := $(MM_CAM_FILES) $(LOCAL_HAL_FILES)

      LOCAL_CFLAGS+= -DNUM_PREVIEW_BUFFERS=4 -D_ANDROID_

      # To Choose neon/C routines for YV12 conversion
      LOCAL_CFLAGS+= -DUSE_NEON_CONVERSION
      # Uncomment below line to enable smooth zoom
      #LOCAL_CFLAGS+= -DCAMERA_SMOOTH_ZOOM

      LOCAL_C_INCLUDES+= \
        $(TARGET_OUT_HEADERS)/mm-camera \
        $(TARGET_OUT_HEADERS)/mm-camera/common \
        $(TARGET_OUT_HEADERS)/mm-still \
        $(TARGET_OUT_HEADERS)/mm-still/jpeg \

      LOCAL_C_INCLUDES+= hardware/qcom/display/libgralloc
      LOCAL_C_INCLUDES+= hardware/qcom/display/libgenlock
      LOCAL_C_INCLUDES+= hardware/qcom/media/libstagefrighthw

      LOCAL_SHARED_LIBRARIES:= libutils libui libcamera_client liblog libcutils libmmjpeg

      LOCAL_SHARED_LIBRARIES+= libgenlock libbinder libhardware
      ifneq ($(DLOPEN_LIBMMCAMERA),1)
        LOCAL_SHARED_LIBRARIES+= liboemcamera
      else
        LOCAL_SHARED_LIBRARIES+= libdl
      endif

      LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/socket.h

      LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
      LOCAL_MODULE:= camera.$(TARGET_BOARD_PLATFORM)
      LOCAL_MODULE_TAGS := optional
      include $(BUILD_SHARED_LIBRARY)

    endif # BUILD_TINY_ANDROID
  endif # USE_CAMERA_STUB
endif #ifeq ($(call is-board-platform-in-list,$(MM_STILL_V4L2_DRIVER_LIST)),true)
