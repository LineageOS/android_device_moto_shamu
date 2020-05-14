#
# System Properties for shamu
#

# Audio
PRODUCT_PROPERTY_OVERRIDES += \
    af.fast_track_multiplier=1 \
    audio_hal.period_size=240 \
    debug.stagefright.ccodec=0 \
    media.aac_51_output_enabled=true \
    persist.audio.dualmic.config=endfire \
    persist.audio.fluence.speaker=false \
    persist.audio.fluence.voicecall=true \
    persist.audio.fluence.voicecomm=false \
    persist.audio.fluence.voicerec=false \
    ro.audio.flinger_standbytime_ms=300 \
    ro.config.media_vol_steps=25 \
    ro.config.vc_call_vol_steps=7 \
    ro.qc.sdk.audio.fluencetype=fluence

# Bluetooth
PRODUCT_PROPERTY_OVERRIDES += \
    bluetooth.enable_timeout_ms=12000

# Camera
PRODUCT_PROPERTY_OVERRIDES += \
    persist.camera.hal.debug.mask=7 \
    persist.camera.HAL3.enabled=1 \
    persist.camera.ISP.debug.mask=0 \
    persist.camera.mct.debug.mask=1 \
    persist.camera.ois.disable=0 \
    persist.camera.pproc.debug.mask=7 \
    persist.camera.stats.debug.mask=0

# Camera default-properties
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    camera.disable_zsl_mode=0 \
    persist.camera.imglib.logs=1 \
    persist.camera.sensor.debug=0

# Dalvik VM
PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.heapstartsize=16m \
    dalvik.vm.heapgrowthlimit=256m \
    dalvik.vm.heapsize=512m \
    dalvik.vm.heaptargetutilization=0.75 \
    dalvik.vm.heapminfree=2m \
    dalvik.vm.heapmaxfree=8m


# Display
PRODUCT_PROPERTY_OVERRIDES += \
    debug.hwui.use_buffer_age=false \
    debug.sf.enable_gl_backpressure=1 \
    debug.sf.latch_unsignaled=1 \
    persist.hwc.mdpcomp.enable=true \
    ro.opengles.version=196610 \
    ro.sf.lcd_density=560 \
    vidc.debug.level=1

# DRM service
PRODUCT_PROPERTY_OVERRIDES += \
    drm.service.enabled=true

# Face lock
PRODUCT_PROPERTY_OVERRIDES += \
    ro.facelock.black_timeout=700 \
    ro.facelock.det_timeout=2500 \
    ro.facelock.est_max_time=600 \
    ro.facelock.rec_timeout=3500

# Factory reset protection
PRODUCT_PROPERTY_OVERRIDES += \
    ro.frp.pst=/dev/block/platform/msm_sdcc.1/by-name/frp

# Fastbootd
PRODUCT_PROPERTY_OVERRIDES += \
    ro.fastbootd.available=true

# FIFO
PRODUCT_PROPERTY_OVERRIDES += \
    sys.use_fifo_ui=1

# OEM Unlock reporting
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    ro.oem_unlock_supported=1

# Perf
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.extension_library=/vendor/lib/libqc-opt.so

# Radio
PRODUCT_PROPERTY_OVERRIDES += \
    persist.data.qmi.adb_logmask=0 \
    persist.radio.alt_mbn_name=tmo_alt.mbn \
    persist.radio.apm_sim_not_pwdn=1 \
    persist.radio.data_no_toggle=1 \
    persist.radio.fsg_reload_on=1 \
    persist.radio.mcfg_enabled=1 \
    persist.radio.no_wait_for_card=1 \
    persist.radio.oem_socket=false \
    persist.radio.sib16_support=1 \
    persist.qcril_uim_vcc_feature=1 \
    rild.libpath=/system/vendor/lib/libril-qc-qmi-1.so

# Surfaceflinger
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    ro.surface_flinger.force_hwc_copy_for_virtual_displays=true \
    ro.surface_flinger.max_frame_buffer_acquired_buffers=3 \
    ro.surface_flinger.max_virtual_display_dimension=2048

# Telephony
PRODUCT_PROPERTY_OVERRIDES += \
    ro.telephony.default_network=10 \
    ro.telephony.default_cdma_sub=0 \
    ro.telephony.get_imsi_from_sim=true \
    telephony.lteOnCdmaDevice=1

# Tethering
PRODUCT_PROPERTY_OVERRIDES += \
    net.tethering.noprovisioning=true

# Vendor security patch level
PRODUCT_PROPERTY_OVERRIDES += \
    ro.lineage.build.vendor_security_patch=2017-10-01
