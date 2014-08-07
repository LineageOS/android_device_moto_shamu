# Copyright 2014 The Android Open Source Project
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

# Motorola blob(s) necessary for Shamu hardware
PRODUCT_COPY_FILES := \
    vendor/moto/shamu/proprietary/Bluetooth_cal.acdb:system/etc/Bluetooth_cal.acdb:moto \
    vendor/moto/shamu/proprietary/FIT_V12.cfg:system/etc/diag/FIT_V12.cfg:moto \
    vendor/moto/shamu/proprietary/General_cal.acdb:system/etc/General_cal.acdb:moto \
    vendor/moto/shamu/proprietary/Global_cal.acdb:system/etc/Global_cal.acdb:moto \
    vendor/moto/shamu/proprietary/Handset_cal.acdb:system/etc/Handset_cal.acdb:moto \
    vendor/moto/shamu/proprietary/Hdmi_cal.acdb:system/etc/Hdmi_cal.acdb:moto \
    vendor/moto/shamu/proprietary/Headset_cal.acdb:system/etc/Headset_cal.acdb:moto \
    vendor/moto/shamu/proprietary/Speaker_cal.acdb:system/etc/Speaker_cal.acdb:moto \
    vendor/moto/shamu/proprietary/left.boost.config:system/vendor/firmware/left.boost.config:moto \
    vendor/moto/shamu/proprietary/left.boost.eq:system/vendor/firmware/left.boost.eq:moto \
    vendor/moto/shamu/proprietary/left.boost_music_table.preset:system/vendor/firmware/left.boost_music_table.preset:moto \
    vendor/moto/shamu/proprietary/left.boost_n1b12.patch:system/vendor/firmware/left.boost_n1b12.patch:moto \
    vendor/moto/shamu/proprietary/left.boost_n1c2.patch:system/vendor/firmware/left.boost_n1c2.patch:moto \
    vendor/moto/shamu/proprietary/left.boost_ringtone_table.preset:system/vendor/firmware/left.boost_ringtone_table.preset:moto \
    vendor/moto/shamu/proprietary/left.boost.speaker:system/vendor/firmware/left.boost.speaker:moto \
    vendor/moto/shamu/proprietary/left.boost_voice_table.preset:system/vendor/firmware/left.boost_voice_table.preset:moto \
    vendor/moto/shamu/proprietary/right.boost.config:system/vendor/firmware/right.boost.config:moto \
    vendor/moto/shamu/proprietary/right.boost.eq:system/vendor/firmware/right.boost.eq:moto \
    vendor/moto/shamu/proprietary/right.boost_music_table.preset:system/vendor/firmware/right.boost_music_table.preset:moto \
    vendor/moto/shamu/proprietary/right.boost_n1b12.patch:system/vendor/firmware/right.boost_n1b12.patch:moto \
    vendor/moto/shamu/proprietary/right.boost_n1c2.patch:system/vendor/firmware/right.boost_n1c2.patch:moto \
    vendor/moto/shamu/proprietary/right.boost_ringtone_table.preset:system/vendor/firmware/right.boost_ringtone_table.preset:moto \
    vendor/moto/shamu/proprietary/right.boost.speaker:system/vendor/firmware/right.boost.speaker:moto \
    vendor/moto/shamu/proprietary/right.boost_voice_table.preset:system/vendor/firmware/right.boost_voice_table.preset:moto \

