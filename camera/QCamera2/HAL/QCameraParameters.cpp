/*
Copyright (c) 2012, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of Code Aurora Forum, Inc. nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#define LOG_TAG "QCameraParameters"

#include <utils/Log.h>
#include <utils/Errors.h>
#include <string.h>
#include <stdlib.h>
#include <gralloc_priv.h>
#include "QCameraParameters.h"

namespace android {
// Parameter keys to communicate between camera application and driver.
const char QCameraParameters::KEY_QC_SUPPORTED_HFR_SIZES[] = "hfr-size-values";
const char QCameraParameters::KEY_QC_PREVIEW_FRAME_RATE_MODE[] = "preview-frame-rate-mode";
const char QCameraParameters::KEY_QC_SUPPORTED_PREVIEW_FRAME_RATE_MODES[] = "preview-frame-rate-modes";
const char QCameraParameters::KEY_QC_PREVIEW_FRAME_RATE_AUTO_MODE[] = "frame-rate-auto";
const char QCameraParameters::KEY_QC_PREVIEW_FRAME_RATE_FIXED_MODE[] = "frame-rate-fixed";
const char QCameraParameters::KEY_QC_TOUCH_AF_AEC[] = "touch-af-aec";
const char QCameraParameters::KEY_QC_SUPPORTED_TOUCH_AF_AEC[] = "touch-af-aec-values";
const char QCameraParameters::KEY_QC_TOUCH_INDEX_AEC[] = "touch-index-aec";
const char QCameraParameters::KEY_QC_TOUCH_INDEX_AF[] = "touch-index-af";
const char QCameraParameters::KEY_QC_SCENE_DETECT[] = "scene-detect";
const char QCameraParameters::KEY_QC_SUPPORTED_SCENE_DETECT[] = "scene-detect-values";
const char QCameraParameters::KEY_QC_ISO_MODE[] = "iso";
const char QCameraParameters::KEY_QC_SUPPORTED_ISO_MODES[] = "iso-values";
const char QCameraParameters::KEY_QC_LENSSHADE[] = "lensshade";
const char QCameraParameters::KEY_QC_SUPPORTED_LENSSHADE_MODES[] = "lensshade-values";
const char QCameraParameters::KEY_QC_AUTO_EXPOSURE[] = "auto-exposure";
const char QCameraParameters::KEY_QC_SUPPORTED_AUTO_EXPOSURE[] = "auto-exposure-values";
const char QCameraParameters::KEY_QC_DENOISE[] = "denoise";
const char QCameraParameters::KEY_QC_SUPPORTED_DENOISE[] = "denoise-values";
const char QCameraParameters::KEY_QC_FOCUS_ALGO[] = "selectable-zone-af";
const char QCameraParameters::KEY_QC_SUPPORTED_FOCUS_ALGOS[] = "selectable-zone-af-values";
const char QCameraParameters::KEY_QC_FACE_DETECTION[] = "face-detection";
const char QCameraParameters::KEY_QC_SUPPORTED_FACE_DETECTION[] = "face-detection-values";
const char QCameraParameters::KEY_QC_MEMORY_COLOR_ENHANCEMENT[] = "mce";
const char QCameraParameters::KEY_QC_SUPPORTED_MEM_COLOR_ENHANCE_MODES[] = "mce-values";
const char QCameraParameters::KEY_QC_VIDEO_HIGH_FRAME_RATE[] = "video-hfr";
const char QCameraParameters::KEY_QC_SUPPORTED_VIDEO_HIGH_FRAME_RATE_MODES[] = "video-hfr-values";
const char QCameraParameters::KEY_QC_REDEYE_REDUCTION[] = "redeye-reduction";
const char QCameraParameters::KEY_QC_SUPPORTED_REDEYE_REDUCTION[] = "redeye-reduction-values";
const char QCameraParameters::KEY_QC_HIGH_DYNAMIC_RANGE_IMAGING[] = "hdr";
const char QCameraParameters::KEY_QC_SUPPORTED_HDR_IMAGING_MODES[] = "hdr-values";
const char QCameraParameters::KEY_QC_ZSL[] = "zsl";
const char QCameraParameters::KEY_QC_SUPPORTED_ZSL_MODES[] = "zsl-values";
const char QCameraParameters::KEY_QC_CAMERA_MODE[] = "camera-mode";
const char QCameraParameters::KEY_QC_AE_BRACKET_HDR[] = "ae-bracket-hdr";
/*only effective when KEY_QC_AE_BRACKET_HDR set to ae_bracketing*/
//const char QCameraParameters::KEY_QC_AE_BRACKET_SETTING_KEY[] = "ae-bracket-setting";

// Values for effect settings.
const char QCameraParameters::EFFECT_EMBOSS[] = "emboss";
const char QCameraParameters::EFFECT_SKETCH[] = "sketch";
const char QCameraParameters::EFFECT_NEON[] = "neon";

// Values for auto exposure settings.
const char QCameraParameters::TOUCH_AF_AEC_OFF[] = "touch-off";
const char QCameraParameters::TOUCH_AF_AEC_ON[] = "touch-on";

// Values for scene mode settings.
const char QCameraParameters::SCENE_MODE_ASD[] = "asd";   // corresponds to CAMERA_BESTSHOT_AUTO in HAL
const char QCameraParameters::SCENE_MODE_BACKLIGHT[] = "backlight";
const char QCameraParameters::SCENE_MODE_FLOWERS[] = "flowers";
const char QCameraParameters::SCENE_MODE_AR[] = "AR";

// Formats for setPreviewFormat and setPictureFormat.
const char QCameraParameters::PIXEL_FORMAT_YUV420SP_ADRENO[] = "yuv420sp-adreno";
const char QCameraParameters::PIXEL_FORMAT_RAW[] = "raw";
const char QCameraParameters::PIXEL_FORMAT_YV12[] = "yuv420p";
const char QCameraParameters::PIXEL_FORMAT_NV12[] = "nv12";

// Values for ISO Settings
const char QCameraParameters::ISO_AUTO[] = "auto";
const char QCameraParameters::ISO_HJR[] = "ISO_HJR";
const char QCameraParameters::ISO_100[] = "ISO100";
const char QCameraParameters::ISO_200[] = "ISO200";
const char QCameraParameters::ISO_400[] = "ISO400";
const char QCameraParameters::ISO_800[] = "ISO800";
const char QCameraParameters::ISO_1600[] = "ISO1600";

// Values for auto exposure settings.
const char QCameraParameters::AUTO_EXPOSURE_FRAME_AVG[] = "frame-average";
const char QCameraParameters::AUTO_EXPOSURE_CENTER_WEIGHTED[] = "center-weighted";
const char QCameraParameters::AUTO_EXPOSURE_SPOT_METERING[] = "spot-metering";
const char QCameraParameters::AUTO_EXPOSURE_SMART_METERING[] = "smart-metering";
const char QCameraParameters::AUTO_EXPOSURE_USER_METERING[] = "user-metering";
const char QCameraParameters::AUTO_EXPOSURE_SPOT_METERING_ADV[] = "spot-metering-adv";
const char QCameraParameters::AUTO_EXPOSURE_CENTER_WEIGHTED_ADV[] = "center-weighted-adv";

const char QCameraParameters::KEY_QC_GPS_LATITUDE_REF[] = "gps-latitude-ref";
const char QCameraParameters::KEY_QC_GPS_LONGITUDE_REF[] = "gps-longitude-ref";
const char QCameraParameters::KEY_QC_GPS_ALTITUDE_REF[] = "gps-altitude-ref";
const char QCameraParameters::KEY_QC_GPS_STATUS[] = "gps-status";
const char QCameraParameters::KEY_QC_EXIF_DATETIME[] = "exif-datetime";

const char QCameraParameters::KEY_QC_HISTOGRAM[] = "histogram";
const char QCameraParameters::KEY_QC_SUPPORTED_HISTOGRAM_MODES[] = "histogram-values";

const char QCameraParameters::VALUE_ENABLE[] = "enable";
const char QCameraParameters::VALUE_DISABLE[] = "disable";
const char QCameraParameters::VALUE_OFF[] = "off";
const char QCameraParameters::VALUE_ON[] = "on";
const char QCameraParameters::VALUE_TRUE[] = "true";
const char QCameraParameters::VALUE_FALSE[] = "false";

const char QCameraParameters::KEY_QC_SHARPNESS[] = "sharpness";
const char QCameraParameters::KEY_QC_MIN_SHARPNESS[] = "min-sharpness";
const char QCameraParameters::KEY_QC_MAX_SHARPNESS[] = "max-sharpness";
const char QCameraParameters::KEY_QC_SHARPNESS_STEP[] = "sharpness-step";
const char QCameraParameters::KEY_QC_CONTRAST[] = "contrast";
const char QCameraParameters::KEY_QC_MIN_CONTRAST[] = "min-contrast";
const char QCameraParameters::KEY_QC_MAX_CONTRAST[] = "max-contrast";
const char QCameraParameters::KEY_QC_CONTRAST_STEP[] = "contrast-step";
const char QCameraParameters::KEY_QC_SATURATION[] = "saturation";
const char QCameraParameters::KEY_QC_MIN_SATURATION[] = "min-saturation";
const char QCameraParameters::KEY_QC_MAX_SATURATION[] = "max-saturation";
const char QCameraParameters::KEY_QC_SATURATION_STEP[] = "saturation-step";
const char QCameraParameters::KEY_QC_BRIGHTNESS[] = "luma-adaptation";
const char QCameraParameters::KEY_QC_MIN_BRIGHTNESS[] = "min-brightness";
const char QCameraParameters::KEY_QC_MAX_BRIGHTNESS[] = "max-brightness";
const char QCameraParameters::KEY_QC_BRIGHTNESS_STEP[] = "brightness-step";
const char QCameraParameters::KEY_QC_SCE_FACTOR[] = "skinToneEnhancement";
const char QCameraParameters::KEY_QC_MIN_SCE_FACTOR[] = "min-sce-factor";
const char QCameraParameters::KEY_QC_MAX_SCE_FACTOR[] = "max-sce-factor";
const char QCameraParameters::KEY_QC_SCE_FACTOR_STEP[] = "sce-factor-step";

const char QCameraParameters::KEY_QC_SUPPORTED_CAMERA_FEATURES[] = "qc-camera-features";
const char QCameraParameters::KEY_QC_MAX_NUM_REQUESTED_FACES[] = "qc-max-num-requested-faces";

//Values for DENOISE
const char QCameraParameters::DENOISE_OFF[] = "denoise-off";
const char QCameraParameters::DENOISE_ON[] = "denoise-on";

// Values for selectable zone af Settings
const char QCameraParameters::FOCUS_ALGO_AUTO[] = "auto";
const char QCameraParameters::FOCUS_ALGO_SPOT_METERING[] = "spot-metering";
const char QCameraParameters::FOCUS_ALGO_CENTER_WEIGHTED[] = "center-weighted";
const char QCameraParameters::FOCUS_ALGO_FRAME_AVERAGE[] = "frame-average";

// Values for HFR settings.
const char QCameraParameters::VIDEO_HFR_OFF[] = "off";
const char QCameraParameters::VIDEO_HFR_2X[] = "60";
const char QCameraParameters::VIDEO_HFR_3X[] = "90";
const char QCameraParameters::VIDEO_HFR_4X[] = "120";
const char QCameraParameters::VIDEO_HFR_5X[] = "150";

// Values for HDR Bracketing settings.
const char QCameraParameters::AE_BRACKET_HDR_OFF[] = "Off";
const char QCameraParameters::AE_BRACKET_HDR[] = "HDR";
const char QCameraParameters::AE_BRACKET[] = "AE-Bracket";

static const char* portrait = "portrait";
static const char* landscape = "landscape";

const cam_dimension_t QCameraParameters::THUMBNAIL_SIZES_MAP[] = {
    { 512, 288 }, //1.777778
    { 480, 288 }, //1.666667
    { 256, 154 }, //1.66233
    { 432, 288 }, //1.5
    { 320, 240 }, //1.33333
    { 176, 144 }, //1.222222
};

const QCameraParameters::QCameraMap QCameraParameters::AUTO_EXPOSURE_MAP[] = {
    { AUTO_EXPOSURE_FRAME_AVG,           CAM_AEC_MODE_FRAME_AVERAGE },
    { AUTO_EXPOSURE_CENTER_WEIGHTED,     CAM_AEC_MODE_CENTER_WEIGHTED },
    { AUTO_EXPOSURE_SPOT_METERING,       CAM_AEC_MODE_SPOT_METERING },
    { AUTO_EXPOSURE_SMART_METERING,      CAM_AEC_MODE_SMART_METERING },
    { AUTO_EXPOSURE_USER_METERING,       CAM_AEC_MODE_USER_METERING },
    { AUTO_EXPOSURE_SPOT_METERING_ADV,   CAM_AEC_MODE_SPOT_METERING_ADV },
    { AUTO_EXPOSURE_CENTER_WEIGHTED_ADV, CAM_AEC_MODE_CENTER_WEIGHTED_ADV },
};

const QCameraParameters::QCameraMap QCameraParameters::PREVIEW_FORMATS_MAP[] = {
    {PIXEL_FORMAT_YUV420SP,        CAM_FORMAT_YUV_420_NV21},
    {PIXEL_FORMAT_YUV420P,         CAM_FORMAT_YUV_420_YV12},
    {PIXEL_FORMAT_YUV420SP_ADRENO, CAM_FORMAT_YUV_420_NV21_ADRENO},
    {PIXEL_FORMAT_YV12,            CAM_FORMAT_YUV_420_YV12},
    {PIXEL_FORMAT_NV12,            CAM_FORMAT_YUV_420_NV12}
};

const QCameraParameters::QCameraMap QCameraParameters::FOCUS_MODES_MAP[] = {
    { FOCUS_MODE_AUTO,               CAM_FOCUS_MODE_AUTO},
    { FOCUS_MODE_INFINITY,           CAM_FOCUS_MODE_INFINITY },
    { FOCUS_MODE_MACRO,              CAM_FOCUS_MODE_MACRO },
    { FOCUS_MODE_FIXED,              CAM_FOCUS_MODE_FIXED },
    { FOCUS_MODE_EDOF,               CAM_FOCUS_MODE_EDOF },
    { FOCUS_MODE_CONTINUOUS_PICTURE, CAM_FOCUS_MODE_CONTINOUS_PICTURE},
    { FOCUS_MODE_CONTINUOUS_VIDEO,   CAM_FOCUS_MODE_CONTINOUS_VIDEO }
};

const QCameraParameters::QCameraMap QCameraParameters::EFFECT_MODES_MAP[] = {
    { EFFECT_NONE,       CAM_EFFECT_MODE_OFF },
    { EFFECT_MONO,       CAM_EFFECT_MODE_MONO },
    { EFFECT_NEGATIVE,   CAM_EFFECT_MODE_NEGATIVE },
    { EFFECT_SOLARIZE,   CAM_EFFECT_MODE_SOLARIZE },
    { EFFECT_SEPIA,      CAM_EFFECT_MODE_SEPIA },
    { EFFECT_POSTERIZE,  CAM_EFFECT_MODE_POSTERIZE },
    { EFFECT_WHITEBOARD, CAM_EFFECT_MODE_WHITEBOARD },
    { EFFECT_BLACKBOARD, CAM_EFFECT_MODE_BLACKBOARD },
    { EFFECT_AQUA,       CAM_EFFECT_MODE_AQUA },
    { EFFECT_EMBOSS,     CAM_EFFECT_MODE_EMBOSS },
    { EFFECT_SKETCH,     CAM_EFFECT_MODE_SKETCH },
    { EFFECT_NEON,       CAM_EFFECT_MODE_NEON }
};

const QCameraParameters::QCameraMap QCameraParameters::SCENE_MODES_MAP[] = {
    { SCENE_MODE_AUTO,           CAM_SCENE_MODE_OFF },
    { SCENE_MODE_ACTION,         CAM_SCENE_MODE_ACTION },
    { SCENE_MODE_PORTRAIT,       CAM_SCENE_MODE_PORTRAIT },
    { SCENE_MODE_LANDSCAPE,      CAM_SCENE_MODE_LANDSCAPE },
    { SCENE_MODE_NIGHT,          CAM_SCENE_MODE_NIGHT },
    { SCENE_MODE_NIGHT_PORTRAIT, CAM_SCENE_MODE_NIGHT_PORTRAIT },
    { SCENE_MODE_THEATRE,        CAM_SCENE_MODE_THEATRE },
    { SCENE_MODE_BEACH,          CAM_SCENE_MODE_BEACH },
    { SCENE_MODE_SNOW,           CAM_SCENE_MODE_SNOW },
    { SCENE_MODE_SUNSET,         CAM_SCENE_MODE_SUNSET },
    { SCENE_MODE_STEADYPHOTO,    CAM_SCENE_MODE_ANTISHAKE },
    { SCENE_MODE_FIREWORKS ,     CAM_SCENE_MODE_FIREWORKS },
    { SCENE_MODE_SPORTS ,        CAM_SCENE_MODE_SPORTS },
    { SCENE_MODE_PARTY,          CAM_SCENE_MODE_PARTY },
    { SCENE_MODE_CANDLELIGHT,    CAM_SCENE_MODE_CANDLELIGHT },
    { SCENE_MODE_ASD,            CAM_SCENE_MODE_AUTO },
    { SCENE_MODE_BACKLIGHT,      CAM_SCENE_MODE_BACKLIGHT },
    { SCENE_MODE_FLOWERS,        CAM_SCENE_MODE_FLOWERS },
    { SCENE_MODE_AR,             CAM_SCENE_MODE_AR },
};

const QCameraParameters::QCameraMap QCameraParameters::FLASH_MODES_MAP[] = {
    { FLASH_MODE_OFF,   CAM_FLASH_MODE_OFF },
    { FLASH_MODE_AUTO,  CAM_FLASH_MODE_AUTO },
    { FLASH_MODE_ON,    CAM_FLASH_MODE_ON },
    { FLASH_MODE_TORCH, CAM_FLASH_MODE_TORCH }
};

const QCameraParameters::QCameraMap QCameraParameters::FOCUS_ALGO_MAP[] = {
    { FOCUS_ALGO_AUTO,            CAM_FOCUS_ALGO_AUTO },
    { FOCUS_ALGO_SPOT_METERING,   CAM_FOCUS_ALGO_SPOT },
    { FOCUS_ALGO_CENTER_WEIGHTED, CAM_FOCUS_ALGO_CENTER_WEIGHTED },
    { FOCUS_ALGO_FRAME_AVERAGE,   CAM_FOCUS_ALGO_AVERAGE }
};

const QCameraParameters::QCameraMap QCameraParameters::WHITE_BALANCE_MODES_MAP[] = {
    { WHITE_BALANCE_AUTO,            CAM_WB_MODE_AUTO },
    { WHITE_BALANCE_INCANDESCENT,    CAM_WB_MODE_INCANDESCENT },
    { WHITE_BALANCE_FLUORESCENT,     CAM_WB_MODE_FLUORESCENT },
    { WHITE_BALANCE_WARM_FLUORESCENT,CAM_WB_MODE_WARM_FLUORESCENT},
    { WHITE_BALANCE_DAYLIGHT,        CAM_WB_MODE_DAYLIGHT },
    { WHITE_BALANCE_CLOUDY_DAYLIGHT, CAM_WB_MODE_CLOUDY_DAYLIGHT },
    { WHITE_BALANCE_TWILIGHT,        CAM_WB_MODE_TWILIGHT },
    { WHITE_BALANCE_SHADE,           CAM_WB_MODE_SHADE }
};

const QCameraParameters::QCameraMap QCameraParameters::ANTIBANDING_MODES_MAP[] = {
    { ANTIBANDING_OFF,  CAM_ANTIBANDING_MODE_OFF },
    { ANTIBANDING_50HZ, CAM_ANTIBANDING_MODE_50HZ },
    { ANTIBANDING_60HZ, CAM_ANTIBANDING_MODE_60HZ },
    { ANTIBANDING_AUTO, CAM_ANTIBANDING_MODE_AUTO }
};

const QCameraParameters::QCameraMap QCameraParameters::ISO_MODES_MAP[] = {
    { ISO_AUTO,  CAM_ISO_MODE_AUTO },
    { ISO_HJR,   CAM_ISO_MODE_DEBLUR },
    { ISO_100,   CAM_ISO_MODE_100 },
    { ISO_200,   CAM_ISO_MODE_200 },
    { ISO_400,   CAM_ISO_MODE_400 },
    { ISO_800,   CAM_ISO_MODE_800 },
    { ISO_1600,  CAM_ISO_MODE_1600 }
};

const QCameraParameters::QCameraMap QCameraParameters::HFR_MODES_MAP[] = {
    { VIDEO_HFR_OFF, CAM_HFR_MODE_OFF },
    { VIDEO_HFR_2X,  CAM_HFR_MODE_60FPS },
    { VIDEO_HFR_3X,  CAM_HFR_MODE_90FPS },
    { VIDEO_HFR_4X,  CAM_HFR_MODE_120FPS },
    { VIDEO_HFR_5X,  CAM_HFR_MODE_150FPS }
};

const QCameraParameters::QCameraMap QCameraParameters::HDR_BRACKETING_MODES_MAP[] = {
    { AE_BRACKET_HDR_OFF, CAM_HDR_BRACKETING_OFF },
    { AE_BRACKET_HDR,     CAM_HDR_MODE },
    { AE_BRACKET,         CAM_EXP_BRACKETING_MODE }
};

const QCameraParameters::QCameraMap QCameraParameters::ON_OFF_MODES_MAP[] = {
    { VALUE_OFF, 0 },
    { VALUE_ON,  1 }
};

const QCameraParameters::QCameraMap QCameraParameters::ENABLE_DISABLE_MODES_MAP[] = {
    { VALUE_ENABLE,  1 },
    { VALUE_DISABLE, 0 }
};

const QCameraParameters::QCameraMap QCameraParameters::DENOISE_ON_OFF_MODES_MAP[] = {
    { DENOISE_OFF, 0 },
    { DENOISE_ON,  1 }
};

const QCameraParameters::QCameraMap QCameraParameters::TRUE_FALSE_MODES_MAP[] = {
    { VALUE_FALSE, 0},
    { VALUE_TRUE,  1}
};

#define DEFAULT_CAMERA_AREA "(0, 0, 0, 0, 0)"

QCameraParameters::QCameraParameters()
    : CameraParameters(),
      m_pCapability(NULL),
      m_pCamOpsTbl(NULL),
      m_pParamHeap(NULL),
      mZslMode(false),
      mRecordingHint(false),
      m_bHistogramEnabled(false),
      m_bFaceDetectionEnabled(false),
      m_bDebugFps(false),
      m_nDumpFrameEnabled(0),
      mFocusMode(CAM_FOCUS_MODE_MAX),
      mPreviewFormat(CAM_FORMAT_YUV_420_NV21)
{
    char value[32];
    // TODO: may move to parameter instead of sysprop
    property_get("persist.debug.sf.showfps", value, "0");
    m_bDebugFps = atoi(value) > 0 ? true : false;
    property_get("persist.camera.dumpimg", value, "0");
    m_nDumpFrameEnabled = atoi(value);
}

QCameraParameters::QCameraParameters(const String8 &params)
    : CameraParameters(params),
      m_pCapability(NULL),
      m_pCamOpsTbl(NULL),
      m_pParamHeap(NULL),
      mRecordingHint(false),
      m_bHistogramEnabled(false),
      m_bFaceDetectionEnabled(false),
      m_bDebugFps(false),
      m_nDumpFrameEnabled(0),
      mFocusMode(CAM_FOCUS_MODE_MAX),
      mPreviewFormat(CAM_FORMAT_YUV_420_NV21)
{
}

QCameraParameters::~QCameraParameters()
{
    deinit();
}

int QCameraParameters::getOrientation() const
{
    const char* orientation = get("orientation");
    if (orientation && !strcmp(orientation, portrait))
        return CAMERA_ORIENTATION_PORTRAIT;
    return CAMERA_ORIENTATION_LANDSCAPE;
}

void QCameraParameters::setOrientation(int orientation)
{
    if (orientation == CAMERA_ORIENTATION_PORTRAIT) {
        set("orientation", portrait);
    } else {
         set("orientation", landscape);
    }
}

String8 QCameraParameters::createSizesString(const cam_dimension_t *sizes, int len)
{
    String8 str;
    char buffer[32];

    if (len > 0) {
        snprintf(buffer, sizeof(buffer), "%dx%d", sizes[0].width, sizes[0].height);
        str.append(buffer);
    }
    for (int i = 1; i < len; i++) {
        snprintf(buffer, sizeof(buffer), ",%dx%d", sizes[i].width, sizes[i].height);
        str.append(buffer);
    }
    return str;
}

String8 QCameraParameters::createValuesString(const int *values,
                                              int len,
                                              const QCameraMap* map,
                                              int map_len)
{
    String8 str;
    int count = 0;

    for (int i = 0; i < len; i++ ) {
        for (int j = 0; j < map_len; j ++)
            if (map[j].val == values[i]) {
                if (NULL != map[j].desc) {
                    if (count > 0) {
                        str.append(",");
                    }
                    str.append(map[j].desc);
                    count++;
                    break; //loop j
                }
            }
    }
    return str;
}

String8 QCameraParameters::createValuesStringFromMap(const QCameraMap* map,
                                                     int map_len)
{
    String8 str;

    for (int i = 0; i < map_len; i++) {
        if (NULL != map[i].desc) {
            if (i > 0) {
                str.append(",");
            }
            str.append(map[i].desc);
        }
    }
    return str;
}

String8 QCameraParameters::createZoomRatioValuesString(int *zoomRatios, int length)
{
    String8 str;
    char buffer[32] = {0};

    if(length > 0){
        snprintf(buffer, sizeof(buffer), "%d", zoomRatios[0]);
        str.append(buffer);
    }

    for (int i =1;i<length;i++){
        memset(buffer, 0, sizeof(buffer));
        snprintf(buffer, sizeof(buffer), ",%d", zoomRatios[i]);
        str.append(buffer);
    }
    return str;
}

String8 QCameraParameters::createHfrValuesString(
                                const cam_hfr_info_t *values,
                                int len,
                                const QCameraMap* map,
                                int map_len)
{
    String8 str;
    int count = 0;

    for (int i = 0; i < len; i++ ) {
        for (int j = 0; j < map_len; j ++)
            if (map[j].val == (int)values[i].mode) {
                if (NULL != map[j].desc) {
                    if (count > 0) {
                        str.append(",");
                    }
                     str.append(map[j].desc);
                     count++;
                     break; //loop j
                }
            }
    }
    return str;
}

String8 QCameraParameters::createHfrSizesString(
                                const cam_hfr_info_t *values,
                                int len)
{
    String8 str;
    char buffer[32];

    if (len > 0) {
        snprintf(buffer, sizeof(buffer), "%dx%d",
                 values[0].dim.width, values[0].dim.height);
        str.append(buffer);
    }
    for (int i = 1; i < len; i++) {
        snprintf(buffer, sizeof(buffer), ",%dx%d",
                 values[i].dim.width, values[i].dim.height);
        str.append(buffer);
    }
    return str;
}

String8 QCameraParameters::createFrameratesString(const cam_fps_range_t *fps, int len)
{
    String8 str;
    char buffer[32];

    for (int i = 0; i < len; i++ ) {
        snprintf(buffer, sizeof(buffer), "%d", int(fps[i].min_fps * 1000));
        str.append(buffer);
        if (i < len-1)
            str.append(",");
    }
    return str;
}

String8 QCameraParameters::createFpsString(const cam_fps_range_t* fps, int len)
{
    String8 str;
    char buffer[32];

    if (len > 0) {
        snprintf(buffer, sizeof(buffer), "(%d,%d)",
                 int(fps[0].min_fps * 1000),
                 int(fps[0].max_fps * 1000));
        str.append(buffer);
    }
    for (int i = 1; i < len; i++) {
        snprintf(buffer, sizeof(buffer), ",(%d,%d)",
                 int(fps[i].min_fps * 1000),
                 int(fps[i].max_fps * 1000));
        str.append(buffer);
    }
    return str;
}

int QCameraParameters::lookupAttr(const QCameraMap arr[], int len, const char *name)
{
    if (name) {
        for (int i = 0; i < len; i++) {
            if (!strcmp(arr[i].desc, name))
                return arr[i].val;
        }
    }
    return NAME_NOT_FOUND;
}

status_t QCameraParameters::setAutoExposure(const QCameraParameters& params)
{
    ALOGV("%s",__func__);
    const char *str = params.get(KEY_QC_AUTO_EXPOSURE);
    if (str != NULL) {
        int32_t value = lookupAttr(AUTO_EXPOSURE_MAP,
                                   sizeof(AUTO_EXPOSURE_MAP)/sizeof(AUTO_EXPOSURE_MAP[0]),
                                   str);
        if (value != NAME_NOT_FOUND) {
            set(KEY_QC_AUTO_EXPOSURE, str);
            return NO_ERROR;
        }
    }
    ALOGE("Invalid auto exposure value: %s", (str == NULL) ? "NULL" : str);
    return BAD_VALUE;
}

#define DATA_PTR(MEM_OBJ,INDEX) MEM_OBJ->getPtr( INDEX )

status_t QCameraParameters::setPreviewSize(const QCameraParameters& params)
{
    int width, height;
    params.getPreviewSize(&width, &height);
    ALOGV("Requested preview size %d x %d", width, height);

    // Validate the preview size
    for (size_t i = 0; i < m_pCapability->preview_sizes_tbl_cnt; ++i) {
        if (width ==  m_pCapability->preview_sizes_tbl[i].width
           && height ==  m_pCapability->preview_sizes_tbl[i].height) {
            CameraParameters::setPreviewSize(width, height);
            ALOGE("setPreviewSize:  width: %d   heigh: %d", width, height);
            return NO_ERROR;
        }
    }
    ALOGE("Invalid preview size requested: %dx%d", width, height);
    return BAD_VALUE;
}

status_t QCameraParameters::setPictureSize(const QCameraParameters& params)
{
    int width, height;
    params.getPictureSize(&width, &height);
    ALOGV("Requested picture size %d x %d", width, height);

    // Validate the picture size
    for (size_t i = 0; i < m_pCapability->picture_sizes_tbl_cnt; ++i) {
        if (width ==  m_pCapability->picture_sizes_tbl[i].width
           && height ==  m_pCapability->picture_sizes_tbl[i].height) {
            CameraParameters::setPictureSize(width, height);
            ALOGE("setPictureSize:  width: %d   heigh: %d", width, height);
            return NO_ERROR;
        }
    }
    ALOGE("Invalid picture size requested: %dx%d", width, height);
    return BAD_VALUE;
}


status_t QCameraParameters::setVideoSize(const QCameraParameters& params)
{
    const char *str= NULL;
    int width, height;
    ALOGV("%s: E", __func__);
    str = params.get(KEY_VIDEO_SIZE);
    if(!str) {
        //If application didn't set this parameter string, use the values from
        //getPreviewSize() as video dimensions.
        params.getPreviewSize(&width, &height);
        ALOGE("No Record Size requested, use the preview dimensions");
    } else {
        params.getVideoSize(&width,&height);
    }

    // Validate the video size
    for (size_t i = 0; i < m_pCapability->video_sizes_tbl_cnt; ++i) {
        if (width ==  m_pCapability->video_sizes_tbl[i].width
                && height ==  m_pCapability->video_sizes_tbl[i].height) {
            CameraParameters::setVideoSize(width,height);
            ALOGE("%s:  width: %d   heigh: %d",__func__, width, height);
            return NO_ERROR;
        }
    }

    ALOGE("Invalid video size requested: %dx%d", width, height);
    ALOGV("%s: X", __func__);
    return BAD_VALUE;
}

status_t QCameraParameters::setPreviewFormat(const QCameraParameters& params)
{
    const char *str = params.getPreviewFormat();
    int32_t previewFormat =
        lookupAttr(PREVIEW_FORMATS_MAP,
                   sizeof(PREVIEW_FORMATS_MAP) / sizeof(QCameraMap),
                   str);
    if (previewFormat != NAME_NOT_FOUND) {
        mPreviewFormat = (cam_format_t)previewFormat;
        CameraParameters::setPreviewFormat(str);
        return NO_ERROR;
    }
    ALOGE("Invalid preview format value: %s", (str == NULL) ? "NULL" : str);
    return BAD_VALUE;
}

status_t QCameraParameters::setPictureFormat(const QCameraParameters& params)
{
    status_t rc = NO_ERROR;
    const char *pictureFormat = params.getPictureFormat();
    CameraParameters::setPictureFormat(pictureFormat);
    return rc;
}

status_t QCameraParameters::setPreviewFrameRate(const QCameraParameters& params)
{
    status_t rc = NO_ERROR;
    int fps = params.getPreviewFrameRate();
    CameraParameters::setPreviewFrameRate(fps);
    return rc;
}

status_t QCameraParameters::updateParameters(QCameraParameters& params,
                                                        bool &needRestart)
{
    status_t final_rc = NO_ERROR;
    status_t rc;

    parm_buffer_t *p_table = (parm_buffer_t*)DATA_PTR(m_pParamHeap,0);
    if(initBatchUpdateTable(p_table) < 0 ) {
        ALOGE("%s:Failed to initialize group update table",__func__);
        rc = BAD_TYPE;
        goto SET_PARAM_DONE;
    }

    if ((rc = setAutoExposure(params)))                 final_rc = rc;
    if ((rc = setPreviewSize(params)))                  final_rc = rc;
    if ((rc = setVideoSize(params)))                    final_rc = rc;
    if ((rc = setPictureSize(params)))                  final_rc = rc;
    if ((rc = setPreviewFormat(params)))                final_rc = rc;
    if ((rc = setPictureFormat(params)))                final_rc = rc;
    if ((rc = setPreviewFrameRate(params)))             final_rc = rc;


    //TODO
    needRestart = false;
SET_PARAM_DONE:
    return final_rc;
}

status_t QCameraParameters::commitParameters()
{
    return commitSetBatch();
}

status_t QCameraParameters::initDefaultParameters()
{
    status_t rc = NO_ERROR;
    int32_t value;

    parm_buffer_t *p_table = (parm_buffer_t*) DATA_PTR(m_pParamHeap,0);
    if(initBatchUpdateTable(p_table) < 0 ) {
        ALOGE("%s:Failed to initialize group update table", __func__);
        return BAD_TYPE;
    }

    /*************************Initialize Values******************************/
    // Set read only parameters from camera capability
    set(KEY_SMOOTH_ZOOM_SUPPORTED,
        m_pCapability->smooth_zoom_supported? VALUE_TRUE : VALUE_FALSE);
    set(KEY_ZOOM_SUPPORTED,
        m_pCapability->zoom_supported? VALUE_TRUE : VALUE_FALSE);
    set(KEY_VIDEO_SNAPSHOT_SUPPORTED,
        m_pCapability->video_snapshot_supported? VALUE_TRUE : VALUE_FALSE);
    set(KEY_VIDEO_STABILIZATION_SUPPORTED,
        m_pCapability->video_stablization_supported? VALUE_TRUE : VALUE_FALSE);
    set(KEY_AUTO_EXPOSURE_LOCK_SUPPORTED,
        m_pCapability->auto_exposure_lock_supported? VALUE_TRUE : VALUE_FALSE);
    set(KEY_AUTO_WHITEBALANCE_LOCK_SUPPORTED,
        m_pCapability->auto_wb_lock_supported? VALUE_TRUE : VALUE_FALSE);
    set(KEY_QC_SUPPORTED_CAMERA_FEATURES,
        m_pCapability->qcom_supported_feature_mask);
    set(KEY_MAX_NUM_DETECTED_FACES_HW, 0);
    set(KEY_MAX_NUM_DETECTED_FACES_SW, m_pCapability->max_num_roi);
    set(KEY_QC_MAX_NUM_REQUESTED_FACES, m_pCapability->max_num_roi);
    // Set focal length, horizontal view angle, and vertical view angle
    setFloat(KEY_FOCAL_LENGTH, m_pCapability->focal_length);
    setFloat(KEY_HORIZONTAL_VIEW_ANGLE, m_pCapability->hor_view_angle);
    setFloat(KEY_VERTICAL_VIEW_ANGLE, m_pCapability->ver_view_angle);

    // Set supported preview sizes
    if (m_pCapability->preview_sizes_tbl_cnt > 0) {
        String8 previewSizeValues = createSizesString(
                m_pCapability->preview_sizes_tbl, m_pCapability->preview_sizes_tbl_cnt);
        set(KEY_SUPPORTED_PREVIEW_SIZES, previewSizeValues.string());
        // Set default preview size
        CameraParameters::setPreviewSize(m_pCapability->preview_sizes_tbl[0].width,
                                         m_pCapability->preview_sizes_tbl[0].height);
    } else {
        ALOGE("%s: supported preview sizes cnt is 0!!!", __func__);
    }

    // Set supported video sizes
    if (m_pCapability->video_sizes_tbl_cnt > 0) {
        String8 videoSizeValues = createSizesString(
                m_pCapability->video_sizes_tbl, m_pCapability->video_sizes_tbl_cnt);
        set(KEY_SUPPORTED_VIDEO_SIZES, videoSizeValues.string());
        // Set default video size
        CameraParameters::setVideoSize(m_pCapability->video_sizes_tbl[0].width,
                                       m_pCapability->video_sizes_tbl[0].height);

        //Set preferred Preview size for video
        String8 vSize = createSizesString(&m_pCapability->video_sizes_tbl[0], 1);
        set(KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO, vSize.string());
    } else {
        ALOGE("%s: supported video sizes cnt is 0!!!", __func__);
    }

    // Set supported picture sizes
    if (m_pCapability->picture_sizes_tbl_cnt) {
        String8 pictureSizeValues = createSizesString(
                m_pCapability->picture_sizes_tbl, m_pCapability->picture_sizes_tbl_cnt);
        set(KEY_SUPPORTED_PICTURE_SIZES, pictureSizeValues.string());
        // Set default picture size
        CameraParameters::setPictureSize(m_pCapability->picture_sizes_tbl[0].width,
                       m_pCapability->picture_sizes_tbl[0].height);
    } else {
        ALOGE("%s: supported picture sizes cnt is 0!!!", __func__);
    }

    // Set supported thumbnail sizes
    String8 thumbnailSizeValues = createSizesString(
            THUMBNAIL_SIZES_MAP,
            sizeof(THUMBNAIL_SIZES_MAP)/sizeof(cam_dimension_t));
    set(KEY_SUPPORTED_JPEG_THUMBNAIL_SIZES, thumbnailSizeValues.string());
    // Set default thumnail size
    set(KEY_JPEG_THUMBNAIL_WIDTH, THUMBNAIL_SIZES_MAP[0].width);
    set(KEY_JPEG_THUMBNAIL_HEIGHT, THUMBNAIL_SIZES_MAP[0].height);

    // Set supported preview formats
    String8 previewFormatValues = createValuesString(
            (int *)m_pCapability->supported_preview_fmts,
            m_pCapability->supported_preview_fmt_cnt,
            PREVIEW_FORMATS_MAP,
            sizeof(PREVIEW_FORMATS_MAP)/sizeof(QCameraMap));
    set(KEY_SUPPORTED_PREVIEW_FORMATS, previewFormatValues.string());
    // Set default preview format
    CameraParameters::setPreviewFormat(PIXEL_FORMAT_YUV420SP);

    // Set default Video Format
    set(KEY_VIDEO_FRAME_FORMAT, PIXEL_FORMAT_YUV420SP);

    // Set supported picture formats
    set(KEY_SUPPORTED_PICTURE_FORMATS, PIXEL_FORMAT_JPEG);
    // Set default picture Format
    CameraParameters::setPictureFormat(PIXEL_FORMAT_JPEG);

    // TODO
    // Set supported raw formats
    // Set default raw format

    //set default jpeg quality and thumbnail quality
    set(KEY_JPEG_QUALITY, 85);
    set(KEY_JPEG_THUMBNAIL_QUALITY, 85);

    // Set FPS ranges
    if (m_pCapability->fps_ranges_tbl_cnt > 0) {
        String8 fpsRangeValues = createFpsString(m_pCapability->fps_ranges_tbl,
                                                 m_pCapability->fps_ranges_tbl_cnt);
        set(KEY_SUPPORTED_PREVIEW_FPS_RANGE, fpsRangeValues.string());
        setPreviewFpsRange(int(m_pCapability->fps_ranges_tbl[0].min_fps * 1000),
                           int(m_pCapability->fps_ranges_tbl[0].max_fps * 1000));
        AddSetParmEntryToBatch(p_table,
                               CAM_INTF_PARM_FPS_RANGE,
                               sizeof(cam_fps_range_t),
                               &m_pCapability->fps_ranges_tbl[0]);
    } else {
        ALOGE("%s: supported fps ranges cnt is 0!!!", __func__);
    }

    // Set supported focus modes
    if (m_pCapability->supported_focus_modes_cnt > 0) {
        String8 focusModeValues = createValuesString(
                (int *)m_pCapability->supported_focus_modes,
                m_pCapability->supported_focus_modes_cnt,
                FOCUS_MODES_MAP,
                sizeof(FOCUS_MODES_MAP)/sizeof(QCameraMap));
        set(KEY_SUPPORTED_FOCUS_MODES, focusModeValues);

        // Set default focus mode and update corresponding parameter buf
        set(KEY_FOCUS_MODE, FOCUS_MODE_AUTO);
        mFocusMode = m_pCapability->supported_focus_modes[0];
        value = mFocusMode;
        AddSetParmEntryToBatch(p_table,
                               CAM_INTF_PARM_FOCUS_MODE,
                               sizeof(value),
                               &value);
    } else {
        ALOGE("%s: supported focus modes cnt is 0!!!", __func__);
    }

    // Set focus areas
    set(KEY_MAX_NUM_FOCUS_AREAS, m_pCapability->max_num_focus_areas);
    set(KEY_FOCUS_AREAS, DEFAULT_CAMERA_AREA);

    // Set metering areas
    set(KEY_MAX_NUM_METERING_AREAS, m_pCapability->max_num_metering_areas);
    set(KEY_METERING_AREAS, DEFAULT_CAMERA_AREA);

    // Set Saturation
    set(KEY_QC_MIN_SATURATION, m_pCapability->saturation_ctrl.min_value);
    set(KEY_QC_MAX_SATURATION, m_pCapability->saturation_ctrl.max_value);
    set(KEY_QC_SATURATION_STEP, m_pCapability->saturation_ctrl.step);
    set(KEY_QC_SATURATION, m_pCapability->saturation_ctrl.def_value);
    value = m_pCapability->saturation_ctrl.def_value;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_SATURATION,
                           sizeof(value),
                           &value);

    // Set Sharpness
    set(KEY_QC_MIN_SHARPNESS, m_pCapability->sharpness_ctrl.min_value);
    set(KEY_QC_MAX_SHARPNESS, m_pCapability->sharpness_ctrl.max_value);
    set(KEY_QC_SHARPNESS_STEP, m_pCapability->sharpness_ctrl.step);
    set(KEY_QC_SHARPNESS, m_pCapability->sharpness_ctrl.def_value);
    value = m_pCapability->sharpness_ctrl.def_value;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_SHARPNESS,
                           sizeof(value),
                           &value);

    // Set Contrast
    set(KEY_QC_MIN_CONTRAST, m_pCapability->contrast_ctrl.min_value);
    set(KEY_QC_MAX_CONTRAST, m_pCapability->contrast_ctrl.max_value);
    set(KEY_QC_CONTRAST_STEP, m_pCapability->contrast_ctrl.step);
    set(KEY_QC_CONTRAST, m_pCapability->contrast_ctrl.def_value);
    value = m_pCapability->contrast_ctrl.def_value;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_CONTRAST,
                           sizeof(value),
                           &value);

    // Set SCE factor
    set(KEY_QC_MIN_SCE_FACTOR, m_pCapability->sce_ctrl.min_value); // -10
    set(KEY_QC_MAX_SCE_FACTOR, m_pCapability->sce_ctrl.max_value); // 10
    set(KEY_QC_SCE_FACTOR_STEP, m_pCapability->sce_ctrl.step);     // 10
    set(KEY_QC_SCE_FACTOR, m_pCapability->sce_ctrl.def_value);     // 0
    value = m_pCapability->sce_ctrl.def_value;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_SCE_FACTOR,
                           sizeof(value),
                           &value);

    // Set Brightness
    set(KEY_QC_MIN_BRIGHTNESS, m_pCapability->brightness_ctrl.min_value); // 0
    set(KEY_QC_MAX_BRIGHTNESS, m_pCapability->brightness_ctrl.max_value); // 6
    set(KEY_QC_BRIGHTNESS_STEP, m_pCapability->brightness_ctrl.step);     // 1
    set(KEY_QC_BRIGHTNESS, m_pCapability->brightness_ctrl.def_value);     // 3
    value = m_pCapability->brightness_ctrl.def_value;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_BRIGHTNESS,
                           sizeof(value),
                           &value);

    // Set Auto exposure
    String8 autoExposureValues = createValuesString(
            (int *)m_pCapability->supported_aec_modes,
            m_pCapability->supported_aec_modes_cnt,
            AUTO_EXPOSURE_MAP,
            sizeof(AUTO_EXPOSURE_MAP) / sizeof(QCameraMap));
    set(KEY_QC_SUPPORTED_AUTO_EXPOSURE, autoExposureValues.string());
    set(KEY_QC_AUTO_EXPOSURE, AUTO_EXPOSURE_FRAME_AVG);
    value = CAM_AEC_MODE_FRAME_AVERAGE;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_AEC_ALGO_TYPE,
                           sizeof(value),
                           &value);

    // Set Exposure Compensation
    set(KEY_MAX_EXPOSURE_COMPENSATION, m_pCapability->exposure_compensation_max); // 12
    set(KEY_MIN_EXPOSURE_COMPENSATION, m_pCapability->exposure_compensation_min); // -12
    set(KEY_EXPOSURE_COMPENSATION, m_pCapability->exposure_compensation_default); // 0
    setFloat(KEY_EXPOSURE_COMPENSATION_STEP, m_pCapability->exposure_compensation_step); // 1/6
    value = m_pCapability->exposure_compensation_default;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_EXPOSURE_COMPENSATION,
                           sizeof(value),
                           &value);

    // Set Antibanding
    String8 antibandingValues = createValuesString(
            (int *)m_pCapability->supported_antibandings,
            m_pCapability->supported_antibandings_cnt,
            ANTIBANDING_MODES_MAP,
            sizeof(ANTIBANDING_MODES_MAP) / sizeof(QCameraMap));
    set(KEY_SUPPORTED_ANTIBANDING, antibandingValues);
    set(KEY_ANTIBANDING, ANTIBANDING_OFF);
    value = CAM_ANTIBANDING_MODE_OFF;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_ANTIBANDING,
                           sizeof(value),
                           &value);

    // Set Effect
    String8 effectValues = createValuesString(
            (int *)m_pCapability->supported_effects,
            m_pCapability->supported_effects_cnt,
            EFFECT_MODES_MAP,
            sizeof(EFFECT_MODES_MAP) / sizeof(QCameraMap));
    set(KEY_SUPPORTED_EFFECTS, effectValues);
    set(KEY_EFFECT, EFFECT_NONE);
    value = CAM_EFFECT_MODE_OFF;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_EFFECT,
                           sizeof(value),
                           &value);

    // Set WhiteBalance
    String8 whitebalanceValues = createValuesString(
            (int *)m_pCapability->supported_white_balances,
            m_pCapability->supported_white_balances_cnt,
            WHITE_BALANCE_MODES_MAP,
            sizeof(WHITE_BALANCE_MODES_MAP) / sizeof(QCameraMap));
    set(KEY_SUPPORTED_WHITE_BALANCE, whitebalanceValues);
    set(KEY_WHITE_BALANCE, WHITE_BALANCE_AUTO);
    value = CAM_WB_MODE_AUTO;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_WHITE_BALANCE,
                           sizeof(value),
                           &value);

    // Set Flash mode
    String8 flashValues = createValuesString(
            (int *)m_pCapability->supported_flash_modes,
            m_pCapability->supported_flash_modes_cnt,
            FLASH_MODES_MAP,
            sizeof(FLASH_MODES_MAP) / sizeof(QCameraMap));
    set(KEY_SUPPORTED_FLASH_MODES, flashValues);
    set(KEY_FLASH_MODE, FLASH_MODE_OFF);
    value = CAM_FLASH_MODE_OFF;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_LED_MODE,
                           sizeof(value),
                           &value);

    // Set Scene Mode
    String8 sceneModeValues = createValuesString(
            (int *)m_pCapability->supported_scene_modes,
            m_pCapability->supported_scene_modes_cnt,
            SCENE_MODES_MAP,
            sizeof(SCENE_MODES_MAP) / sizeof(QCameraMap));
    set(KEY_SUPPORTED_SCENE_MODES, sceneModeValues);
    set(KEY_SCENE_MODE, SCENE_MODE_AUTO);
    value = CAM_SCENE_MODE_OFF; // mode off corresponding to API auto mode
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_BESTSHOT_MODE,
                           sizeof(value),
                           &value);

    // Set ISO Mode
    String8 isoValues = createValuesString(
            (int *)m_pCapability->supported_iso_modes,
            m_pCapability->supported_iso_modes_cnt,
            ISO_MODES_MAP,
            sizeof(ISO_MODES_MAP) / sizeof(QCameraMap));
    set(KEY_QC_SUPPORTED_ISO_MODES, isoValues);
    set(KEY_QC_ISO_MODE, ISO_AUTO);
    value = CAM_ISO_MODE_AUTO;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_ISO,
                           sizeof(value),
                           &value);

    // Set HFR
    String8 hfrValues = createHfrValuesString(
            m_pCapability->hfr_tbl,
            m_pCapability->hfr_tbl_cnt,
            HFR_MODES_MAP,
            sizeof(HFR_MODES_MAP) / sizeof(QCameraMap));
    set(KEY_QC_SUPPORTED_VIDEO_HIGH_FRAME_RATE_MODES, hfrValues.string());
    String8 hfrSizeValues = createHfrSizesString(
            m_pCapability->hfr_tbl,
            m_pCapability->hfr_tbl_cnt);
    set(KEY_QC_SUPPORTED_HFR_SIZES, hfrSizeValues.string());
    set(KEY_QC_VIDEO_HIGH_FRAME_RATE, VIDEO_HFR_OFF);
    value = CAM_HFR_MODE_OFF;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_HFR,
                           sizeof(value),
                           &value);

    // Set Focus algorithms
    String8 focusAlgoValues = createValuesString(
            (int *)m_pCapability->supported_focus_algos,
            m_pCapability->supported_focus_algos_cnt,
            FOCUS_ALGO_MAP,
            sizeof(FOCUS_ALGO_MAP) / sizeof(QCameraMap));
    set(KEY_QC_SUPPORTED_FOCUS_ALGOS, focusAlgoValues);
    set(KEY_QC_FOCUS_ALGO, FOCUS_ALGO_AUTO);
    value = CAM_FOCUS_ALGO_AUTO;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_FOCUS_ALGO_TYPE,
                           sizeof(value),
                           &value);

    // Set Zoom Ratios
    String8 zoomRatioValues = createZoomRatioValuesString(
            m_pCapability->zoom_ratio_tbl,
            m_pCapability->zoom_ratio_tbl_cnt);
    set(KEY_ZOOM_RATIOS, zoomRatioValues);
    set(KEY_MAX_ZOOM, m_pCapability->zoom_ratio_tbl_cnt - 1);
    set(KEY_ZOOM, 0);
    value = m_pCapability->zoom_ratio_tbl[0];
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_ZOOM,
                           sizeof(value),
                           &value);

    // Set ACE Bracketing/HDR
    cam_exp_bracketing_t aec;
    memset(&aec, 0, sizeof(aec));
    aec.mode = CAM_HDR_BRACKETING_OFF;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_HDR,
                           sizeof(aec),
                           &aec);

    // Set Denoise
    String8 denoiseValues = createValuesStringFromMap(
       DENOISE_ON_OFF_MODES_MAP, sizeof(DENOISE_ON_OFF_MODES_MAP) / sizeof(QCameraMap));
    set(KEY_QC_SUPPORTED_DENOISE, denoiseValues.string());
    set(KEY_QC_DENOISE, DENOISE_OFF);
    cam_denoise_param_t denoise;
    memset(&denoise, 0, sizeof(denoise));
    denoise.denoise_enable = 0;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_WAVELET_DENOISE,
                           sizeof(denoise),
                           &denoise);

    // Set feature enable/disable
    String8 enableDisableValues = createValuesStringFromMap(
        ENABLE_DISABLE_MODES_MAP, sizeof(ENABLE_DISABLE_MODES_MAP) / sizeof(QCameraMap));

    // Set Lens Shading
    set(KEY_QC_SUPPORTED_LENSSHADE_MODES, enableDisableValues);
    set(KEY_QC_LENSSHADE, VALUE_ENABLE);
    value = 1;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_ROLLOFF,
                           sizeof(value),
                           &value);

    // Set MCE
    set(KEY_QC_SUPPORTED_MEM_COLOR_ENHANCE_MODES, enableDisableValues);
    set(KEY_QC_MEMORY_COLOR_ENHANCEMENT, VALUE_ENABLE);
    value = 1;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_MCE,
                           sizeof(value),
                           &value);

    // Set Histogram
    set(KEY_QC_SUPPORTED_HISTOGRAM_MODES, enableDisableValues);
    set(KEY_QC_HISTOGRAM, VALUE_DISABLE);
    value = 0;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_HISTOGRAM,
                           sizeof(value),
                           &value);
    //Set Red Eye Reduction
    set(KEY_QC_SUPPORTED_REDEYE_REDUCTION, enableDisableValues);
    set(KEY_QC_REDEYE_REDUCTION, VALUE_DISABLE);
    value = 0;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_REDEYE_REDUCTION,
                           sizeof(value),
                           &value);

    // Set feature on/off
    String8 onOffValues = createValuesStringFromMap(
        ON_OFF_MODES_MAP, sizeof(ON_OFF_MODES_MAP) / sizeof(QCameraMap));

    //Set Scene Detection
    set(KEY_QC_SUPPORTED_SCENE_DETECT, onOffValues);
    set(KEY_QC_SCENE_DETECT, VALUE_OFF);
    value = 0;
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_ASD_ENABLE,
                           sizeof(value),
                           &value);

    //Set Face Detection
    set(KEY_QC_SUPPORTED_FACE_DETECTION, onOffValues);
    set(KEY_QC_FACE_DETECTION, VALUE_OFF);
    cam_fd_set_parm_t faceDetection;
    memset(&faceDetection, 0, sizeof(faceDetection));
    AddSetParmEntryToBatch(p_table,
                           CAM_INTF_PARM_FD,
                           sizeof(faceDetection),
                           &faceDetection);

    //Set ZSL
    set(KEY_QC_SUPPORTED_ZSL_MODES, onOffValues);
    set(KEY_QC_ZSL, VALUE_OFF);
    mZslMode = false;

    //Set Touch AF/AEC
    set(KEY_QC_SUPPORTED_TOUCH_AF_AEC, onOffValues);
    set(KEY_QC_TOUCH_AF_AEC, VALUE_OFF);

    // Set default Auto Exposure lock value
    set(KEY_AUTO_EXPOSURE_LOCK, VALUE_FALSE);

    //Set default AWB_LOCK lock value
    set(KEY_AUTO_WHITEBALANCE_LOCK, VALUE_FALSE);

    // Set default Camera mode
    set(KEY_QC_CAMERA_MODE, 0);

    commitSetBatch();

    return rc;
}

status_t QCameraParameters::init(cam_capability_t *capabilities, mm_camera_vtbl_t *mmOps)
{
    status_t rc = NO_ERROR;

    m_pCapability = capabilities;
    m_pCamOpsTbl = mmOps;

    //Allocate Set Param Buffer
    m_pParamHeap = new QCameraHeapMemory();
    rc = m_pParamHeap->allocate(1, sizeof(parm_buffer_t));
    if(rc != OK) {
        rc = NO_MEMORY;
        ALOGE("Failed to allocate SETPARM Heap memory");
        goto TRANS_INIT_ERROR1;
    }

    //Map memory for parameters buffer
    rc = m_pCamOpsTbl->ops->map_buf(m_pCamOpsTbl->camera_handle,
                             CAM_MAPPING_BUF_TYPE_PARM_BUF,
                             m_pParamHeap->getFd(0),
                             sizeof(parm_buffer_t));
    if(rc < 0) {
        ALOGE("%s:failed to map SETPARM buffer",__func__);
        rc = FAILED_TRANSACTION;
        goto TRANS_INIT_ERROR2;
    }

    initDefaultParameters();

    goto TRANS_INIT_DONE;

TRANS_INIT_ERROR2:
    m_pParamHeap->deallocate();

TRANS_INIT_ERROR1:
    delete m_pParamHeap;
    m_pParamHeap = NULL;

TRANS_INIT_DONE:
    return rc;
}

void QCameraParameters::deinit()
{
    //clear all entries in the map
    String8 emptyStr;
    QCameraParameters::unflatten(emptyStr);

    if (NULL != m_pCamOpsTbl) {
        m_pCamOpsTbl->ops->unmap_buf(
                             m_pCamOpsTbl->camera_handle,
                             CAM_MAPPING_BUF_TYPE_PARM_BUF);
        m_pCamOpsTbl = NULL;
    }
    m_pCapability = NULL;
    if (NULL != m_pParamHeap) {
        m_pParamHeap->deallocate();
        delete m_pParamHeap;
        m_pParamHeap = NULL;
    }
}

// Parse string like "(1, 2, 3, 4, ..., N)"
// num is pointer to an allocated array of size N
static int parseNDimVector(const char *str, int *num, int N, char delim = ',')
{
    char *start, *end;
    if(num == NULL) {
        ALOGE("Invalid output array (num == NULL)");
        return -1;
    }
    //check if string starts and ends with parantheses
    if(str[0] != '(' || str[strlen(str)-1] != ')') {
        ALOGE("Invalid format of string %s, valid format is (n1, n2, n3, n4 ...)", str);
        return -1;
    }
    start = (char*) str;
    start ++;
    for(int i=0; i<N; i++) {
        *(num+i) = (int) strtol(start, &end, 10);
        if(*end != delim && i < N-1) {
            ALOGE("Cannot find delimeter '%c' in string \"%s\". end = %c", delim, str, *end);
            return -1;
        }
        start = end+1;
    }
    return 0;
}

// Parse string like "640x480" or "10000,20000"
static int parse_pair(const char *str, int *first, int *second, char delim,
                      char **endptr = NULL)
{
    // Find the first integer.
    char *end;
    int w = (int)strtol(str, &end, 10);
    // If a delimeter does not immediately follow, give up.
    if (*end != delim) {
        ALOGE("Cannot find delimeter (%c) in str=%s", delim, str);
        return -1;
    }

    // Find the second integer, immediately after the delimeter.
    int h = (int)strtol(end+1, &end, 10);

    *first = w;
    *second = h;

    if (endptr) {
        *endptr = end;
    }

    return 0;
}

static void parseSizesList(const char *sizesStr, Vector<Size> &sizes)
{
    if (sizesStr == 0) {
        return;
    }

    char *sizeStartPtr = (char *)sizesStr;

    while (true) {
        int width, height;
        int success = parse_pair(sizeStartPtr, &width, &height, 'x',
                                 &sizeStartPtr);
        if (success == -1 || (*sizeStartPtr != ',' && *sizeStartPtr != '\0')) {
            ALOGE("Picture sizes string \"%s\" contains invalid character.", sizesStr);
            return;
        }
        sizes.push(Size(width, height));

        if (*sizeStartPtr == '\0') {
            return;
        }
        sizeStartPtr++;
    }
}


void QCameraParameters::getSupportedHfrSizes(Vector<Size> &sizes) const
{
    const char *hfrSizesStr = get(KEY_QC_SUPPORTED_HFR_SIZES);
    parseSizesList(hfrSizesStr, sizes);
}

void QCameraParameters::setPreviewFpsRange(int minFPS, int maxFPS)
{
    char str[32];
    snprintf(str, sizeof(str), "%d,%d", minFPS, maxFPS);
    set(KEY_PREVIEW_FPS_RANGE, str);
}

void QCameraParameters::setPreviewFrameRateMode(const char *mode)
{
    set(KEY_QC_PREVIEW_FRAME_RATE_MODE, mode);
}

const char *QCameraParameters::getPreviewFrameRateMode() const
{
    return get(KEY_QC_PREVIEW_FRAME_RATE_MODE);
}

void QCameraParameters::setTouchIndexAec(int x, int y)
{
    char str[32];
    snprintf(str, sizeof(str), "%dx%d", x, y);
    set(KEY_QC_TOUCH_INDEX_AEC, str);
}

void QCameraParameters::getTouchIndexAec(int *x, int *y) const
{
    *x = -1;
    *y = -1;

    // Get the current string, if it doesn't exist, leave the -1x-1
    const char *p = get(KEY_QC_TOUCH_INDEX_AEC);
    if (p == 0)
        return;

    int tempX, tempY;
    if (parse_pair(p, &tempX, &tempY, 'x') == 0) {
        *x = tempX;
        *y = tempY;
    }
}

void QCameraParameters::setTouchIndexAf(int x, int y)
{
    char str[32];
    snprintf(str, sizeof(str), "%dx%d", x, y);
    set(KEY_QC_TOUCH_INDEX_AF, str);
}

void QCameraParameters::getTouchIndexAf(int *x, int *y) const
{
    *x = -1;
    *y = -1;

    // Get the current string, if it doesn't exist, leave the -1x-1
    const char *p = get(KEY_QC_TOUCH_INDEX_AF);
    if (p == 0)
        return;

    int tempX, tempY;
    if (parse_pair(p, &tempX, &tempY, 'x') == 0) {
        *x = tempX;
        *y = tempY;
	}
}

void QCameraParameters::getMeteringAreaCenter(int *x, int *y) const
{
    //Default invalid values
    *x = -2000;
    *y = -2000;

    const char *p = get(KEY_METERING_AREAS);
    if(p != NULL) {
        int arr[5] = {-2000, -2000, -2000, -2000, 0};
        parseNDimVector(p, arr, 5); //p = "(x1, y1, x2, y2, weight)"
        *x = (arr[0] + arr[2])/2; //center_x = (x1+x2)/2
        *y = (arr[1] + arr[3])/2; //center_y = (y1+y2)/2
    }
}

status_t QCameraParameters::getStreamFormat(cam_stream_type_t streamType,
                                            cam_format_t &format)
{
    status_t ret = NO_ERROR;

    format = CAM_FORMAT_MAX;
    switch (streamType) {
    case CAM_STREAM_TYPE_PREVIEW:
    case CAM_STREAM_TYPE_POSTVIEW:
        format = mPreviewFormat;
        break;
    case CAM_STREAM_TYPE_SNAPSHOT:
    case CAM_STREAM_TYPE_OFFLINE_PROC:
        format = CAM_FORMAT_YUV_420_NV21;
        break;
    case CAM_STREAM_TYPE_VIDEO:
        format = CAM_FORMAT_YUV_420_NV12;
        break;
    case CAM_STREAM_TYPE_RAW:
        //TODO: Get proper raw format
        format = CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG;
        break;
    case CAM_STREAM_TYPE_METADATA:
    case CAM_STREAM_TYPE_DEFAULT:
    default:
        break;
    }

    return ret;
}

status_t QCameraParameters::getStreamDimension(cam_stream_type_t streamType,
                                               cam_dimension_t &dim)
{
    status_t ret = NO_ERROR;

    memset(&dim, 0, sizeof(cam_dimension_t));

    switch (streamType) {
    case CAM_STREAM_TYPE_PREVIEW:
        getPreviewSize(&dim.width, &dim.height);
        break;
    case CAM_STREAM_TYPE_POSTVIEW:
        getPreviewSize(&dim.width, &dim.height);
        break;
    case CAM_STREAM_TYPE_SNAPSHOT:
        getPictureSize(&dim.width, &dim.height);
        break;
    case CAM_STREAM_TYPE_VIDEO:
        getVideoSize(&dim.width, &dim.height);
        break;
    case CAM_STREAM_TYPE_RAW:
        dim = m_pCapability->raw_dim;
        break;
    case CAM_STREAM_TYPE_METADATA:
        dim.width = sizeof(cam_metadata_info_t);
        dim.height = 1;
        break;
    case CAM_STREAM_TYPE_OFFLINE_PROC:
        getPictureSize(&dim.width, &dim.height);
        break;
    case CAM_STREAM_TYPE_DEFAULT:
    default:
        ALOGE("%s: no dimension for unsupported stream type %d",
              __func__, streamType);
        ret = BAD_VALUE;
        break;
    }

    return ret;
}

int QCameraParameters::getPreviewHalPixelFormat() const
{
    int32_t halPixelFormat;

    switch (mPreviewFormat) {
    case CAM_FORMAT_YUV_420_NV12:
        halPixelFormat = HAL_PIXEL_FORMAT_YCbCr_420_SP;
        break;
    case CAM_FORMAT_YUV_420_NV21:
        halPixelFormat = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        break;
    case CAM_FORMAT_YUV_420_NV21_ADRENO:
        halPixelFormat = HAL_PIXEL_FORMAT_YCrCb_420_SP_ADRENO;
        break;
    case CAM_FORMAT_YUV_420_YV12:
        halPixelFormat = HAL_PIXEL_FORMAT_YV12;
        break;
    case CAM_FORMAT_YUV_422_NV16:
    case CAM_FORMAT_YUV_422_NV61:
    default:
        halPixelFormat = HAL_PIXEL_FORMAT_YCrCb_420_SP;
        break;
    }
    return halPixelFormat;
}

void QCameraParameters::getThumbnailSize(int *width, int *height) const
{
    *width = getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
    *height = getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);
}

int QCameraParameters::getZSLBurstInterval()
{
    // TODO
    int val = 1;
    return val;
}

int QCameraParameters::getZSLQueueDepth()
{
    //TODO
    int val = 2;
    return val;
}

int QCameraParameters::getZSLBackLookCount()
{
    // TODO
    int look_back = 2;
    return look_back;
}

int QCameraParameters::setRecordingHintValue(bool /*value*/)
{
    // TODO
    return 0;
}

uint8_t QCameraParameters::getNumOfSnapshots()
{
    // TODO
    return 1;
}

bool QCameraParameters::isNoDisplayMode()
{
    // TODO
    return false;
}

bool QCameraParameters::isWNREnabled()
{
    // TODO
    return false;
}

bool QCameraParameters::isSmoothZoomRunning()
{
    // TODO
    return false;
}

int QCameraParameters::getJpegQuality()
{
    return getInt(QCameraParameters::KEY_JPEG_QUALITY);
}

int QCameraParameters::getJpegRotation() {
    return getInt(QCameraParameters::KEY_ROTATION);
}

int QCameraParameters::parseGPSCoordinate(const char *coord_str, rat_t* coord)
{
    if(coord == NULL) {
        ALOGE("%s: error, invalid argument coord == NULL", __func__);
        return BAD_VALUE;
    }
    float degF = atof(coord_str);
    if (degF < 0) {
        degF = -degF;
    }
    float minF = (degF - (int) degF) * 60;
    float secF = (minF - (int) minF) * 60;

    getRational(&coord[0], (int)degF, 1);
    getRational(&coord[1], (int)minF, 1);
    getRational(&coord[2], (int)(secF * 10000), 10000);
    return NO_ERROR;
}

int32_t QCameraParameters::getExifDateTime(char *dateTime, uint32_t &count)
{
    //get time and date from system
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime (&rawtime);
    //Write datetime according to EXIF Spec
    //"YYYY:MM:DD HH:MM:SS" (20 chars including \0)
    snprintf(dateTime, 20, "%04d:%02d:%02d %02d:%02d:%02d",
             timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
             timeinfo->tm_mday, timeinfo->tm_hour,
             timeinfo->tm_min, timeinfo->tm_sec);
    count = 20;

    return NO_ERROR;
}

int32_t QCameraParameters::getRational(rat_t *rat, int num, int denom)
{
    if (NULL == rat) {
        ALOGE("%s: NULL rat input", __func__);
        return BAD_VALUE;
    }
    rat->num = num;
    rat->denom = denom;
    return NO_ERROR;
}

int32_t QCameraParameters::getExifFocalLength(rat_t *focalLength)
{
    int focalLengthValue =
        (int)(getFloat(QCameraParameters::KEY_FOCAL_LENGTH) * FOCAL_LENGTH_DECIMAL_PRECISION);
    return getRational(focalLength, focalLengthValue, FOCAL_LENGTH_DECIMAL_PRECISION);
}

uint16_t QCameraParameters::getExifIsoSpeed()
{
    // TODO
    //const char *iso_str = get(QCameraParameters::KEY_QC_ISO_MODE);
    //int iso_index = attr_lookup(iso, sizeof(iso) / sizeof(str_map), iso_str);
    //int iso_value = iso_speed_values[iso_index];
    return 0;
}

int32_t QCameraParameters::getExifGpsProcessingMethod(char *gpsProcessingMethod,
                                                      uint32_t &count)
{
    const char *str = get(QCameraParameters::KEY_GPS_PROCESSING_METHOD);
    if(str != NULL) {
        memcpy(gpsProcessingMethod, ExifAsciiPrefix, EXIF_ASCII_PREFIX_SIZE);
        count = EXIF_ASCII_PREFIX_SIZE;
        strncpy(gpsProcessingMethod + EXIF_ASCII_PREFIX_SIZE, str, strlen(str));
        count += strlen(str);
        gpsProcessingMethod[count++] = '\0'; // increase 1 for the last NULL char
        return NO_ERROR;
    } else {
        return BAD_VALUE;
    }
}

int32_t QCameraParameters::getExifLatitude(rat_t *latitude,
                                           char *latRef)
{
    const char *str = get(QCameraParameters::KEY_GPS_LATITUDE);
    if(str != NULL) {
        parseGPSCoordinate(str, latitude);

        //set Latitude Ref
        float latitudeValue = getFloat(QCameraParameters::KEY_GPS_LATITUDE);
        if(latitudeValue < 0.0f) {
            latRef[0] = 'S';
        } else {
            latRef[0] = 'N';
        }
        latRef[1] = '\0';
        return NO_ERROR;
    }else{
        return BAD_VALUE;
    }
}

int32_t QCameraParameters::getExifLongitude(rat_t *longitude,
                                            char *lonRef)
{
    const char *str = get(QCameraParameters::KEY_GPS_LONGITUDE);
    if(str != NULL) {
        parseGPSCoordinate(str, longitude);

        //set Longitude Ref
        float longitudeValue = getFloat(QCameraParameters::KEY_GPS_LONGITUDE);
        if(longitudeValue < 0.0f) {
            lonRef[0] = 'W';
        } else {
            lonRef[0] = 'E';
        }
        lonRef[1] = '\0';
        return NO_ERROR;
    }else{
        return BAD_VALUE;
    }
}

int32_t QCameraParameters::getExifAltitude(rat_t *altitude,
                                           char *altRef)
{
    const char *str = get(QCameraParameters::KEY_GPS_ALTITUDE);
    if(str != NULL) {
        double value = atof(str);
        *altRef = 0;
        if(value < 0){
            *altRef = 1;
            value = -value;
        }
        return getRational(altitude, value*1000, 1000);
    }else{
        return BAD_VALUE;
    }
}

int32_t QCameraParameters::getExifGpsDateTimeStamp(char *gpsDateStamp,
                                                   uint32_t bufLen,
                                                   rat_t *gpsTimeStamp)
{
    const char *str = get(QCameraParameters::KEY_GPS_TIMESTAMP);
    if(str != NULL) {
        time_t unixTime = (time_t)atol(str);
        struct tm *UTCTimestamp = gmtime(&unixTime);

        strftime(gpsDateStamp, bufLen, "%Y:%m:%d", UTCTimestamp);

        getRational(&gpsTimeStamp[0], UTCTimestamp->tm_hour, 1);
        getRational(&gpsTimeStamp[1], UTCTimestamp->tm_min, 1);
        getRational(&gpsTimeStamp[2], UTCTimestamp->tm_sec, 1);

        return NO_ERROR;
    } else {
        return BAD_VALUE;
    }
}

int32_t QCameraParameters::updateFocusDistances(cam_focus_distances_info_t *focusDistances)
{
    String8 str;
    char buffer[32] = {0};
    //set all distances to infinity if focus mode is infinity
    if(mFocusMode == CAM_FOCUS_MODE_INFINITY) {
        str.append("Infinity,Infinity,Infinity");
    } else {
        snprintf(buffer, sizeof(buffer), "%f", focusDistances->focus_distance[0]);
        str.append(buffer);
        snprintf(buffer, sizeof(buffer), ",%f", focusDistances->focus_distance[1]);
        str.append(buffer);
        snprintf(buffer, sizeof(buffer), ",%f", focusDistances->focus_distance[2]);
        str.append(buffer);
    }
    ALOGD("%s: setting KEY_FOCUS_DISTANCES as %s", __FUNCTION__, str.string());
    set(QCameraParameters::KEY_FOCUS_DISTANCES, str.string());
    return NO_ERROR;
}

int32_t QCameraParameters::setHistogram(bool enabled)
{
    if(m_bHistogramEnabled == enabled) {
        ALOGD("%s: histogram flag not changed, no ops here", __func__);
        return NO_ERROR;
    }

    // set parm for histogram
    parm_buffer_t *p_table = (parm_buffer_t*)DATA_PTR(m_pParamHeap,0);
    if(initBatchUpdateTable(p_table) < 0 ) {
        ALOGE("%s:Failed to initialize group update table", __func__);
        return BAD_TYPE;
    }

    int8_t value = enabled;
    int32_t rc = NO_ERROR;
    rc = AddSetParmEntryToBatch(p_table,
                                CAM_INTF_PARM_HISTOGRAM,
                                sizeof(value),
                                &value);
    if (rc != NO_ERROR) {
        ALOGE("%s:Failed to update table", __func__);
        return rc;
    }

    rc = commitSetBatch();
    if (rc != NO_ERROR) {
        ALOGE("%s:Failed to set histogram", __func__);
        return rc;
    }

    m_bHistogramEnabled = enabled;

    ALOGD(" Histogram -> %s", m_bHistogramEnabled ? "Enabled" : "Disabled");

    return rc;
}

int32_t QCameraParameters::setFaceDetection(bool enabled)
{
    if(m_bFaceDetectionEnabled == enabled) {
        ALOGD("%s: face detection flag not changed, no ops here", __func__);
        return NO_ERROR;
    }

    // set parm for face detection
    int requested_faces = getInt(QCameraParameters::KEY_QC_MAX_NUM_REQUESTED_FACES);
    cam_fd_set_parm_t fd_set_parm;
    memset(&fd_set_parm, 0, sizeof(cam_fd_set_parm_t));
    fd_set_parm.fd_mode = enabled;
    fd_set_parm.num_fd = requested_faces;

    parm_buffer_t *p_table = (parm_buffer_t*)DATA_PTR(m_pParamHeap,0);
    if(initBatchUpdateTable(p_table) < 0 ) {
        ALOGE("%s:Failed to initialize group update table", __func__);
        return BAD_TYPE;
    }
    int32_t rc = NO_ERROR;

    rc = AddSetParmEntryToBatch(p_table,
                                CAM_INTF_PARM_FD,
                                sizeof(fd_set_parm),
                                &fd_set_parm);
    if (rc != NO_ERROR) {
        ALOGE("%s:Failed to update table", __func__);
        return rc;
    }

    rc = commitSetBatch();
    if (rc != NO_ERROR) {
        ALOGE("%s:Failed to set face detection parm", __func__);
        return rc;
    }

    m_bFaceDetectionEnabled = enabled;
    ALOGD(" FaceDetection -> %s", m_bFaceDetectionEnabled ? "Enabled" : "Disabled");

    return rc;
}

cam_focus_mode_type QCameraParameters::getFocusMode()
{
    // TODO
    return CAM_FOCUS_MODE_AUTO;
}

int32_t QCameraParameters::initBatchUpdateTable(parm_buffer_t *p_table)
{
    memset(p_table, 0, sizeof(parm_buffer_t));
    p_table->first_flagged_entry = CAM_INTF_PARM_MAX;
    return NO_ERROR;
}

int32_t QCameraParameters::AddSetParmEntryToBatch(parm_buffer_t *p_table,
                                                  cam_intf_parm_type_t paramType,
                                                  uint32_t paramLength,
                                                  void *paramValue)
{
    int position = paramType;
    int current, next;

    /*************************************************************************
    *                 Code to take care of linking next flags                *
    *************************************************************************/
    current = GET_FIRST_PARAM_ID(p_table);
    if (position == current){
        //DO NOTHING
    } else if (position < current){
        SET_NEXT_PARAM_ID(position, p_table, current);
        SET_FIRST_PARAM_ID(p_table, position);
    } else {
        /* Search for the position in the linked list where we need to slot in*/
        while (position > GET_NEXT_PARAM_ID(current, p_table))
            current = GET_NEXT_PARAM_ID(current, p_table);

        /*If node already exists no need to alter linking*/
        if (position != GET_NEXT_PARAM_ID(current, p_table)) {
            next = GET_NEXT_PARAM_ID(current, p_table);
            SET_NEXT_PARAM_ID(current, p_table, position);
            SET_NEXT_PARAM_ID(position, p_table, next);
        }
    }

    /*************************************************************************
    *                   Copy contents into entry                             *
    *************************************************************************/

    if (paramLength > sizeof(parm_type_t)) {
        ALOGE("%s:Size of input larger than max entry size",__func__);
        return BAD_VALUE;
    }
    memcpy(POINTER_OF(paramType,p_table), paramValue, paramLength);
    return NO_ERROR;
}

int32_t QCameraParameters::AddGetParmEntryToBatch(parm_buffer_t *p_table,
                                                  cam_intf_parm_type_t paramType)
{
    int position = paramType;
    int current, next;

    /*************************************************************************
    *                 Code to take care of linking next flags                *
    *************************************************************************/
    current = GET_FIRST_PARAM_ID(p_table);
    if (position == current){
        //DO NOTHING
    } else if (position < current){
        SET_NEXT_PARAM_ID(position, p_table, current);
        SET_FIRST_PARAM_ID(p_table, position);
    } else {
        /* Search for the position in the linked list where we need to slot in*/
        while (position > GET_NEXT_PARAM_ID(current, p_table))
            current = GET_NEXT_PARAM_ID(current, p_table);

        /*If node already exists no need to alter linking*/
        if (position != GET_NEXT_PARAM_ID(current, p_table)) {
            next=GET_NEXT_PARAM_ID(current, p_table);
            SET_NEXT_PARAM_ID(current, p_table, position);
            SET_NEXT_PARAM_ID(position, p_table, next);
        }
    }

    return NO_ERROR;
}

int32_t QCameraParameters::commitSetBatch()
{
    parm_buffer_t *p_table = (parm_buffer_t*) DATA_PTR(m_pParamHeap, 0);
    return m_pCamOpsTbl->ops->set_parms(m_pCamOpsTbl->camera_handle, p_table);
}

int32_t QCameraParameters::commitGetBatch()
{
    parm_buffer_t *p_table = (parm_buffer_t*) DATA_PTR(m_pParamHeap, 0);
    return m_pCamOpsTbl->ops->get_parms(m_pCamOpsTbl->camera_handle, p_table);
}

}; // namespace android

