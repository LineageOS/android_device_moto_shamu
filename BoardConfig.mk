#
# Copyright (C) 2014 The Android Open-Source Project
# Copyright (C) 2018 The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Architecture
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_VARIANT := krait

# Audio
BOARD_HAS_AUDIO_DSP := true
BOARD_USES_ALSA_AUDIO := true

# Binder
TARGET_USES_64_BIT_BINDER := true

# Bluetooth defines
BOARD_HAVE_BLUETOOTH_BCM := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/moto/shamu/bluetooth
BOARD_CUSTOM_BT_CONFIG := device/moto/shamu/bluetooth/vnd_shamu.txt

# Board
TARGET_NO_BOOTLOADER := true
TARGET_NO_RADIOIMAGE := true
TARGET_BOOTLOADER_BOARD_NAME := shamu
TARGET_BOARD_PLATFORM := msm8084

# Camera
TARGET_PROCESS_SDK_VERSION_OVERRIDE += \
    /system/bin/mm-qcamera-daemon=25
USE_DEVICE_SPECIFIC_CAMERA:= true

# Charger
BACKLIGHT_PATH := /sys/class/leds/lcd-backlight/brightness
BOARD_CHARGER_ENABLE_SUSPEND := true

# Dexpreopt
# Enable dex-preoptimization to speed up first boot sequence
ifeq ($(HOST_OS),linux)
  ifneq ($(TARGET_BUILD_VARIANT),eng)
    ifeq ($(WITH_DEXPREOPT),)
      DONT_DEXPREOPT_PREBUILTS := true
      WITH_DEXPREOPT_BOOT_IMG_AND_SYSTEM_SERVER_ONLY := false
      WITH_DEXPREOPT := true
    endif
  endif
endif

# EGL
# Maximum GLES shader cache size for each app to store the compiled shader
# binaries. Decrease the size if RAM or Flash Storage size is a limitation
# of the device.
MAX_EGL_CACHE_SIZE := 2048*1024
# Shader cache config options
# Maximum size of the  GLES Shaders that can be cached for reuse.
# Increase the size if shaders of size greater than 12KB are used.
MAX_EGL_CACHE_KEY_SIZE := 12*1024

# Encryption
TARGET_KEYMASTER_SKIP_WAITING_FOR_QSEE := true

# Filesystem
BOARD_BOOTIMAGE_PARTITION_SIZE := 16777216
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_CACHEIMAGE_PARTITION_SIZE := 268435456
BOARD_FLASH_BLOCK_SIZE := 131072
BOARD_OEMIMAGE_PARTITION_SIZE := 67108864
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 16793600
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 2147483648
BOARD_USERDATAIMAGE_PARTITION_SIZE := 25253773312
TARGET_USERIMAGES_USE_EXT4 := true
TARGET_USERIMAGES_USE_F2FS := true
TARGET_FS_CONFIG_GEN += device/moto/shamu/config.fs

# Fonts (reduce system image size)
EXCLUDE_SERIF_FONTS := true

# GPS
BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE := $(TARGET_BOARD_PLATFORM)
BOARD_VENDOR_QCOM_LOC_PDK_FEATURE_SET := true
TARGET_NO_RPC := true

# Gralloc
TARGET_ADDITIONAL_GRALLOC_10_USAGE_BITS := 0x02000000U

# Hardware Composer
# Maximum dimension (width or height) of a virtual display that will be
# handled by the hardware composer
MAX_VIRTUAL_DISPLAY_DIMENSION := 2048

# ION
TARGET_USES_ION := true

# Kernel
BOARD_KERNEL_BASE := 0x00000000
BOARD_KERNEL_CMDLINE := console=ttyHSL0,115200,n8 androidboot.console=ttyHSL0
BOARD_KERNEL_CMDLINE += androidboot.hardware=shamu ehci-hcd.park=3
BOARD_KERNEL_CMDLINE += utags.blkdev=/dev/block/platform/msm_sdcc.1/by-name/utags
BOARD_KERNEL_CMDLINE += utags.backup=/dev/block/platform/msm_sdcc.1/by-name/utagsBackup
BOARD_KERNEL_CMDLINE += coherent_pool=8M vmalloc=384M
BOARD_KERNEL_IMAGE_NAME := zImage-dtb
BOARD_KERNEL_PAGESIZE :=  2048
BOARD_KERNEL_TAGS_OFFSET := 0x01E00000
BOARD_RAMDISK_OFFSET     := 0x02000000
# Needs to be defined after the above flags
BOARD_MKBOOTIMG_ARGS := --ramdisk_offset $(BOARD_RAMDISK_OFFSET) --tags_offset $(BOARD_KERNEL_TAGS_OFFSET)
TARGET_KERNEL_CONFIG := shamu_defconfig
TARGET_KERNEL_SOURCE := kernel/moto/shamu

# Manifests
DEVICE_FRAMEWORK_MANIFEST_FILE += system/libhidl/vintfdata/manifest_healthd_exclude.xml
DEVICE_MANIFEST_FILE := device/moto/shamu/manifest.xml
DEVICE_MATRIX_FILE := device/moto/shamu/compatibility_matrix.xml

# Power
TARGET_HAS_LEGACY_POWER_STATS := true
TARGET_HAS_NO_WLAN_STATS := true
TARGET_USES_INTERACTION_BOOST := true
TARGET_TAP_TO_WAKE_NODE := "/sys/bus/i2c/devices/1-004a/tsp"

# Recovery
LZMA_RAMDISK_TARGETS := recovery
TARGET_RECOVERY_FSTAB = device/moto/shamu/rootdir/etc/fstab.shamu

# Render
NUM_FRAMEBUFFER_SURFACE_BUFFERS := 3
OVERRIDE_RS_DRIVER := libRSDriver_adreno.so
TARGET_FORCE_HWC_FOR_VIRTUAL_DISPLAYS := true

# RIL
# Support Native Layer RF cutback
BOARD_USES_CUTBACK_IN_RILD := true

# Selinux
BOARD_SEPOLICY_DIRS += device/moto/shamu/sepolicy

# Wi-Fi
BOARD_WLAN_DEVICE           := bcmdhd
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
BOARD_HOSTAPD_DRIVER        := NL80211
BOARD_HOSTAPD_PRIVATE_LIB   := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
WIFI_BUS := PCIE
WIFI_DRIVER_FW_PATH_AP      := "/vendor/firmware/fw_bcmdhd_apsta.bin"
WIFI_DRIVER_FW_PATH_PARAM   := "/sys/module/bcmdhd/parameters/firmware_path"
WIFI_DRIVER_FW_PATH_STA     := "/vendor/firmware/fw_bcmdhd.bin"
WPA_SUPPLICANT_VERSION      := VER_0_8_X

-include vendor/motorola/shamu/BoardConfigVendor.mk
