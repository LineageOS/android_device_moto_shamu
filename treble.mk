#
# Copyright (C) 2017 The The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Audio
PRODUCT_PACKAGES += \
    android.hardware.audio@2.0-impl \
    android.hardware.audio.effect@2.0-impl

# Bluetooth
PRODUCT_PACKAGES += \
    android.hardware.bluetooth@1.0-impl

# Camera
PRODUCT_PACKAGES += \
    camera.device@3.2-impl \
    android.hardware.camera.provider@2.4-impl

# DRM
PRODUCT_PACKAGES += \
    android.hardware.drm@1.0-impl

# Dumpstate
PRODUCT_PACKAGES += \
    android.hardware.dumpstate@1.0-service.shamu

# GNSS
PRODUCT_PACKAGES += \
    android.hardware.gnss@1.0-impl \
    android.hardware.gnss@1.0-service

# Graphics
PRODUCT_PACKAGES += \
    android.hardware.configstore@1.0-service \
    android.hardware.graphics.allocator@2.0-impl \
    android.hardware.graphics.allocator@2.0-service \
    android.hardware.graphics.composer@2.1-impl \
    android.hardware.graphics.mapper@2.0-impl \
    android.hardware.memtrack@1.0-impl

# Health
PRODUCT_PACKAGES += \
   android.hardware.health@1.0-convert \
   android.hardware.health@1.0-impl \
   android.hardware.health@1.0-service

# Keymaster
PRODUCT_PACKAGES += \
    android.hardware.keymaster@3.0-impl

# Light
PRODUCT_PACKAGES += \
    android.hardware.light@2.0-service.shamu

# NFC
PRODUCT_PACKAGES += \
    android.hardware.nfc@1.0-impl-bcm \
    android.hardware.nfc@1.0-service

# RenderScript
PRODUCT_PACKAGES += \
    android.hardware.renderscript@1.0-impl

# Sensors
PRODUCT_PACKAGES += \
    android.hardware.sensors@1.0-impl

# Thermal
PRODUCT_PACKAGES += \
    android.hardware.thermal@1.0-impl

# Vibrator
PRODUCT_PACKAGES += \
    android.hardware.vibrator@1.0-service.shamu

# Wi-Fi
PRODUCT_PACKAGES += \
    android.hardware.wifi@1.0-service
