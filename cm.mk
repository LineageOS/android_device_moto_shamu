# Boot animation
TARGET_SCREEN_HEIGHT := 1440
TARGET_SCREEN_WIDTH := 2560

# Inherit some common CM stuff.
$(call inherit-product, vendor/cm/config/common_full_phone.mk)

# Enhanced NFC
$(call inherit-product, vendor/cm/config/nfc_enhanced.mk)

# Inherit device configuration
$(call inherit-product, device/moto/shamu/aosp_shamu.mk)

DEVICE_PACKAGE_OVERLAYS += device/moto/shamu/overlay-cm

## Device identifier. This must come after all inclusions
PRODUCT_NAME := cm_shamu
PRODUCT_BRAND := google
PRODUCT_MODEL := Nexus 6

TARGET_VENDOR := moto

PRODUCT_BUILD_PROP_OVERRIDES += \
    PRODUCT_NAME=shamu \
    BUILD_FINGERPRINT=google/shamu/shamu:5.1.1/LYZ28E/1914015:user/release-keys \
    PRIVATE_BUILD_DESC="shamu-user 5.1.1 LYZ28E 1914015 release-keys"

PRODUCT_PROPERTY_OVERRIDES += \
	persist.rcs.supported=0 \
	persist.radio.sib16_support=1 \
	persist.data.qmi.adb_logmask=0 \
	persist.data.iwlan.enable=true \
	persist.radio.ignore_ims_wlan=1 \
	persist.radio.data_con_rprt=1 \
	keyguard.no_require_sim=true
