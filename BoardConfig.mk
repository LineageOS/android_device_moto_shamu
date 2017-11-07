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

TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_VARIANT := krait

TARGET_NO_BOOTLOADER := true

# Inline kernel building
TARGET_KERNEL_CONFIG := shamu_defconfig
TARGET_KERNEL_SOURCE := kernel/moto/shamu
BOARD_KERNEL_IMAGE_NAME := zImage-dtb

BOARD_KERNEL_BASE := 0x00000000
BOARD_KERNEL_PAGESIZE :=  2048
BOARD_KERNEL_TAGS_OFFSET := 0x01E00000
BOARD_RAMDISK_OFFSET     := 0x02000000

BOARD_KERNEL_CMDLINE := console=ttyHSL0,115200,n8 androidboot.console=ttyHSL0 androidboot.hardware=shamu msm_rtb.filter=0x37 ehci-hcd.park=3 utags.blkdev=/dev/block/platform/msm_sdcc.1/by-name/utags utags.backup=/dev/block/platform/msm_sdcc.1/by-name/utagsBackup coherent_pool=8M vmalloc=300M

BOARD_MKBOOTIMG_ARGS := --ramdisk_offset $(BOARD_RAMDISK_OFFSET) --tags_offset $(BOARD_KERNEL_TAGS_OFFSET)

# Shader cache config options
# Maximum size of the  GLES Shaders that can be cached for reuse.
# Increase the size if shaders of size greater than 12KB are used.
MAX_EGL_CACHE_KEY_SIZE := 12*1024

# Maximum GLES shader cache size for each app to store the compiled shader
# binaries. Decrease the size if RAM or Flash Storage size is a limitation
# of the device.
MAX_EGL_CACHE_SIZE := 2048*1024

# Maximum dimension (width or height) of a virtual display that will be
# handled by the hardware composer
MAX_VIRTUAL_DISPLAY_DIMENSION := 2048

BOARD_USES_ALSA_AUDIO := true

BOARD_SUPPORTS_SOUND_TRIGGER := true

# Wifi related defines
WPA_SUPPLICANT_VERSION      := VER_0_8_X
BOARD_WLAN_DEVICE           := bcmdhd
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
BOARD_HOSTAPD_DRIVER        := NL80211
BOARD_HOSTAPD_PRIVATE_LIB   := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
WIFI_DRIVER_FW_PATH_PARAM   := "/sys/module/bcmdhd/parameters/firmware_path"
WIFI_DRIVER_FW_PATH_AP      := "/vendor/firmware/fw_bcmdhd_apsta.bin"
WIFI_DRIVER_FW_PATH_STA     := "/vendor/firmware/fw_bcmdhd.bin"
WIFI_BUS := PCIE
#BOARD_USES_SECURE_SERVICES := true

#Bluetooth defines
BOARD_HAVE_BLUETOOTH_BCM := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/moto/shamu/bluetooth
BOARD_CUSTOM_BT_CONFIG := device/moto/shamu/bluetooth/vnd_shamu.txt

TARGET_NO_RADIOIMAGE := true
TARGET_BOARD_PLATFORM := msm8084
TARGET_BOOTLOADER_BOARD_NAME := shamu
TARGET_NO_RPC := true

TARGET_BOARD_INFO_FILE := device/moto/shamu/board-info.txt

TARGET_USES_ION := true
TARGET_USES_64_BIT_BINDER := true
TARGET_HW_DISK_ENCRYPTION := false

TARGET_USERIMAGES_USE_EXT4 := true
BOARD_BOOTIMAGE_PARTITION_SIZE := 16777216
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 16793600
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 2147483648
BOARD_OEMIMAGE_PARTITION_SIZE := 67108864
BOARD_USERDATAIMAGE_PARTITION_SIZE := 25253773312
BOARD_CACHEIMAGE_PARTITION_SIZE := 268435456
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_FLASH_BLOCK_SIZE := 131072

# Charger
BOARD_CHARGER_ENABLE_SUSPEND := true
BOARD_HAL_STATIC_LIBRARIES := libhealthd.shamu
BACKLIGHT_PATH := /sys/class/leds/lcd-backlight/brightness
WITH_LINEAGE_CHARGER := false

TARGET_RECOVERY_FSTAB = device/moto/shamu/fstab.shamu
# Ensure f2fstools are built
TARGET_USERIMAGES_USE_F2FS := true

TARGET_RELEASETOOLS_EXTENSIONS := device/moto/shamu

# Support Native Layer RF cutback
BOARD_USES_CUTBACK_IN_RILD := true

BOARD_SEPOLICY_DIRS += device/moto/shamu/sepolicy

HAVE_ADRENO_SOURCE:= false

TARGET_FORCE_HWC_FOR_VIRTUAL_DISPLAYS := true

BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE := $(TARGET_BOARD_PLATFORM)
BOARD_VENDOR_QCOM_LOC_PDK_FEATURE_SET := true

BOARD_HAS_AUDIO_DSP := true

USE_DEVICE_SPECIFIC_CAMERA:= true

USE_CLANG_PLATFORM_BUILD := true

TARGET_FS_CONFIG_GEN += device/moto/shamu/config.fs

# Render
OVERRIDE_RS_DRIVER := libRSDriver_adreno.so
USE_OPENGL_RENDERER := true
NUM_FRAMEBUFFER_SURFACE_BUFFERS := 3

# Recovery
LZMA_RAMDISK_TARGETS := recovery

# Device manifest
DEVICE_MANIFEST_FILE := device/moto/shamu/manifest.xml

-include vendor/motorola/shamu/BoardConfigVendor.mk
