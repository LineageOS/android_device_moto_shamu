#
# Copyright (C) 2014 The Android Open-Source Project
# Copyright (C) 2018-2019 The LineageOS Project
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

# This file includes all definitions that apply to ALL shamu devices, and
# are also specific to shamu devices
#

# AAPT
PRODUCT_AAPT_CONFIG := normal
PRODUCT_AAPT_PREF_CONFIG := 560dpi
# A list of dpis to select prebuilt apk, in precedence order.
PRODUCT_AAPT_PREBUILT_DPI := xxxhdpi xxhdpi xhdpi hdpi

# APEX
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/ld.config.txt:$(TARGET_COPY_OUT_SYSTEM)/etc/swcodec/ld.config.txt

# Audio
PRODUCT_PACKAGES += \
    audio.primary.msm8084 \
    audio.a2dp.default \
    audio.usb.default \
    audio.r_submix.default \
    libaudio-resampler

USE_XML_AUDIO_POLICY_CONF := 1

# Audio configs
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/audio_effects.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_effects.xml \
    $(LOCAL_PATH)/audio/audio_platform_info.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_platform_info.xml \
    $(LOCAL_PATH)/audio/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration.xml \
    $(LOCAL_PATH)/audio/mixer_paths.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths.xml \
    frameworks/av/services/audiopolicy/config/a2dp_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/a2dp_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/audio_policy_volumes.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_volumes.xml \
    frameworks/av/services/audiopolicy/config/default_volume_tables.xml:$(TARGET_COPY_OUT_VENDOR)/etc/default_volume_tables.xml \
    frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/r_submix_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/usb_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/usb_audio_policy_configuration.xml

# Audio Effects
PRODUCT_PACKAGES += \
    libqcompostprocbundle \
    libqcomvisualizer \
    libqcomvoiceprocessing \
    libqcomvoiceprocessingdescriptors

# Bluetooth
PRODUCT_PACKAGES += \
    bdAddrLoader_shamu \
    libbt-vendor

# Bluetooth firmware
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/bluetooth/BCM4356A2_001.003.015.0077.0214_ORC.hcd:$(TARGET_COPY_OUT_VENDOR)/firmware/bcm4354A2.hcd

# Camera
PRODUCT_PACKAGES += \
    Snap

PRODUCT_PACKAGES += \
    camera.msm8084 \
    libmm-qcamera

# Characteristics
PRODUCT_CHARACTERISTICS := nosdcard

# Data (CAF)
PRODUCT_SOONG_NAMESPACES += \
    vendor/qcom/opensource/dataservices

# Dex-pre-opt exclusions
$(call add-product-dex-preopt-module-config,MotoSignatureApp,disable)

# Display
PRODUCT_PACKAGES += \
    gralloc.msm8084 \
    hwcomposer.msm8084 \
    memtrack.msm8084 \
    libgenlock \
    libqdutils \
    libqdMetaData

# Filesystem
# For android_filesystem_config.h
PRODUCT_PACKAGES += \
   fs_config_files

# GPS
PRODUCT_PACKAGES += \
    gps.msm8084

# GPS config
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/gps.conf:$(TARGET_COPY_OUT_SYSTEM)/etc/gps.conf

# HIDL
$(call inherit-product, $(LOCAL_PATH)/hidl.mk)
PRODUCT_ENFORCE_VINTF_MANIFEST_OVERRIDE := true

# Keylayouts
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/keylayout/gpio-keys.kl:$(TARGET_COPY_OUT_SYSTEM)/usr/keylayout/gpio-keys.kl \
    $(LOCAL_PATH)/keylayout/apq8084-taiko-tfa9890_stereo_co_Button_Jack.kl:$(TARGET_COPY_OUT_SYSTEM)/usr/keylayout/apq8084-taiko-tfa9890_stereo_co_Button_Jack.kl \
    $(LOCAL_PATH)/configs/atmel_mxt_ts.idc:$(TARGET_COPY_OUT_SYSTEM)/usr/idc/atmel_mxt_ts.idc

# NFC
PRODUCT_PACKAGES += \
    com.android.nfc_extras \
    nfc_nci.bcm2079x.default \
    NfcNci \
    Tag

# NFC configs
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.nfc.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/permissions/android.hardware.nfc.xml \
    frameworks/native/data/etc/android.hardware.nfc.hce.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/permissions/android.hardware.nfc.hce.xml \
    frameworks/native/data/etc/android.hardware.nfc.hcef.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/permissions/android.hardware.nfc.hcef.xml \
    $(LOCAL_PATH)/configs/nfcee_access.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/nfcee_access.xml \
    $(LOCAL_PATH)/configs/libnfc-nci.conf:$(TARGET_COPY_OUT_SYSTEM)/etc/libnfc-nci.conf \
    $(LOCAL_PATH)/configs/libnfc-nci-20795a10.conf:$(TARGET_COPY_OUT_SYSTEM)/etc/libnfc-nci-20795a10.conf

# Media
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/media/media_profiles_V1_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_profiles_V1_0.xml \
    $(LOCAL_PATH)/media/media_codecs.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml \
    $(LOCAL_PATH)/media/media_codecs_performance.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_telephony.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_telephony.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video.xml

# Offmode charging
PRODUCT_PACKAGES += \
    charger_res_images

# OMX
PRODUCT_PACKAGES += \
    libc2dcolorconvert \
    libstagefrighthw \
    libOmxCore \
    libmm-omxcore \
    libOmxVdec \
    libOmxVdecHevc \
    libOmxVenc

# Overlays
DEVICE_PACKAGE_OVERLAYS := \
    $(LOCAL_PATH)/overlay \
    $(LOCAL_PATH)/overlay-lineage

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/handheld_core_hardware.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/handheld_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.camera.full.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.full.xml \
    frameworks/native/data/etc/android.hardware.camera.raw.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.raw.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.wifi.passpoint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.passpoint.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.barometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.barometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepcounter.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.stepcounter.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepdetector.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.stepdetector.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.audio.low_latency.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.audio.low_latency.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth_le.xml \
    frameworks/native/data/etc/android.hardware.telephony.cdma.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.cdma.xml \
    frameworks/native/data/etc/android.hardware.opengles.aep.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.opengles.aep.xml \
    frameworks/native/data/etc/android.software.midi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.midi.xml

# Privileged permissions whitelist
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/permissions/privapp-permissions_shamu.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/permissions/privapp-permissions_shamu.xml

# Ramdisk
 PRODUCT_PACKAGES += \
    fstab.shamu \
    init.shamu.rc \
    init.shamu.power.rc \
    init.shamu.usb.rc \
    ueventd.shamu.rc

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/bin/init.qcom.devwait.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.qcom.devwait.sh \
    $(LOCAL_PATH)/rootdir/bin/init.qcom.devstart.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.qcom.devstart.sh

# Recorder
PRODUCT_PACKAGES += \
    Recorder

# Recovery
PRODUCT_PACKAGES += \
    fastbootd

# RIL
PRODUCT_PACKAGES += \
    libion \
    librmnetctl \
    libxml2 \
    qmi_motext_hook

# SEC config
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/sec_config:$(TARGET_COPY_OUT_VENDOR)/etc/sec_config

# Shims
PRODUCT_PACKAGES += \
    libcutils_shim

# Shippig API
$(call inherit-product, $(SRC_TARGET_DIR)/product/product_launched_with_l.mk)

# Soong
PRODUCT_SOONG_NAMESPACES += $(LOCAL_PATH)

# System Properties
-include $(LOCAL_PATH)/system_prop.mk

# Thermal config
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/thermal-engine-shamu.conf:$(TARGET_COPY_OUT_VENDOR)/etc/thermal-engine-shamu.conf

# TimeKeep
PRODUCT_PACKAGES += \
    timekeep \
    TimeKeep

# Touch
PRODUCT_PACKAGES += \
    atmel.fw.apq8084

# Touch firmware updater
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/bin/init.mmi.touch.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.mmi.touch.sh

# Verity
# Only include verity on user builds for LineageOS
ifeq ($(TARGET_BUILD_VARIANT),user)
# Setup dm-verity configs.
PRODUCT_SYSTEM_VERITY_PARTITION := /dev/block/platform/msm_sdcc.1/by-name/system
$(call inherit-product, build/target/product/verity.mk)
endif

# Wi-Fi
PRODUCT_PACKAGES += \
    libwpa_client \
    hostapd \
    wifilogd \
    wpa_supplicant \
    wpa_supplicant.conf

# WiFi cal NVRAM file
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/bcmdhd.cal:$(TARGET_COPY_OUT_SYSTEM)/etc/wifi/bcmdhd.cal

# WiFi config
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/wpa_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/wpa_supplicant_overlay.conf

# WiFi firmware
$(call inherit-product-if-exists, hardware/broadcom/wlan/bcmdhd/firmware/bcm4356/device-bcm.mk)



