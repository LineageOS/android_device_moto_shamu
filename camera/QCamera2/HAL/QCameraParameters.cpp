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
#include <string.h>
#include <stdlib.h>
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
const char QCameraParameters::KEY_QC_SELECTABLE_ZONE_AF[] = "selectable-zone-af";
const char QCameraParameters::KEY_QC_SUPPORTED_SELECTABLE_ZONE_AF[] = "selectable-zone-af-values";
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
const char QCameraParameters::KEY_QC_POWER_MODE_SUPPORTED[] = "power-mode-supported";
const char QCameraParameters::KEY_QC_ZSL[] = "zsl";
const char QCameraParameters::KEY_QC_SUPPORTED_ZSL_MODES[] = "zsl-values";
const char QCameraParameters::KEY_QC_CAMERA_MODE[] = "camera-mode";
const char QCameraParameters::KEY_QC_AE_BRACKET_HDR[] = "ae-bracket-hdr";
const char QCameraParameters::KEY_QC_POWER_MODE[] = "power-mode";
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

// Values for auto scene detection settings.
const char QCameraParameters::SCENE_DETECT_OFF[] = "off";
const char QCameraParameters::SCENE_DETECT_ON[] = "on";

// Formats for setPreviewFormat and setPictureFormat.
const char QCameraParameters::PIXEL_FORMAT_YUV420SP_ADRENO[] = "yuv420sp-adreno";
const char QCameraParameters::PIXEL_FORMAT_RAW[] = "raw";
const char QCameraParameters::PIXEL_FORMAT_YV12[] = "yuv420p";
const char QCameraParameters::PIXEL_FORMAT_NV12[] = "nv12";

// Values for focus mode settings.
const char QCameraParameters::FOCUS_MODE_NORMAL[] = "normal";
const char QCameraParameters::KEY_QC_SKIN_TONE_ENHANCEMENT[] = "skinToneEnhancement";
const char QCameraParameters::KEY_QC_SUPPORTED_SKIN_TONE_ENHANCEMENT_MODES[] = "skinToneEnhancement-values";

// Values for ISO Settings
const char QCameraParameters::ISO_AUTO[] = "auto";
const char QCameraParameters::ISO_HJR[] = "ISO_HJR";
const char QCameraParameters::ISO_100[] = "ISO100";
const char QCameraParameters::ISO_200[] = "ISO200";
const char QCameraParameters::ISO_400[] = "ISO400";
const char QCameraParameters::ISO_800[] = "ISO800";
const char QCameraParameters::ISO_1600[] = "ISO1600";

 //Values for Lens Shading
const char QCameraParameters::LENSSHADE_ENABLE[] = "enable";
const char QCameraParameters::LENSSHADE_DISABLE[] = "disable";

// Values for auto exposure settings.
const char QCameraParameters::AUTO_EXPOSURE_FRAME_AVG[] = "frame-average";
const char QCameraParameters::AUTO_EXPOSURE_CENTER_WEIGHTED[] = "center-weighted";
const char QCameraParameters::AUTO_EXPOSURE_SPOT_METERING[] = "spot-metering";

const char QCameraParameters::KEY_QC_GPS_LATITUDE_REF[] = "gps-latitude-ref";
const char QCameraParameters::KEY_QC_GPS_LONGITUDE_REF[] = "gps-longitude-ref";
const char QCameraParameters::KEY_QC_GPS_ALTITUDE_REF[] = "gps-altitude-ref";
const char QCameraParameters::KEY_QC_GPS_STATUS[] = "gps-status";
const char QCameraParameters::KEY_QC_EXIF_DATETIME[] = "exif-datetime";

const char QCameraParameters::KEY_QC_HISTOGRAM[] = "histogram";
const char QCameraParameters::KEY_QC_SUPPORTED_HISTOGRAM_MODES[] = "histogram-values";

//Values for Histogram Shading
const char QCameraParameters::HISTOGRAM_ENABLE[] = "enable";
const char QCameraParameters::HISTOGRAM_DISABLE[] = "disable";

//Values for Skin Tone Enhancement Modes
const char QCameraParameters::SKIN_TONE_ENHANCEMENT_ENABLE[] = "enable";
const char QCameraParameters::SKIN_TONE_ENHANCEMENT_DISABLE[] = "disable";

const char QCameraParameters::KEY_QC_SHARPNESS[] = "sharpness";
const char QCameraParameters::KEY_QC_MAX_SHARPNESS[] = "max-sharpness";
const char QCameraParameters::KEY_QC_CONTRAST[] = "contrast";
const char QCameraParameters::KEY_QC_MAX_CONTRAST[] = "max-contrast";
const char QCameraParameters::KEY_QC_SATURATION[] = "saturation";
const char QCameraParameters::KEY_QC_MAX_SATURATION[] = "max-saturation";

const char QCameraParameters::KEY_QC_SINGLE_ISP_OUTPUT_ENABLED[] = "single-isp-output-enabled";
const char QCameraParameters::KEY_QC_SUPPORTED_CAMERA_FEATURES[] = "qc-camera-features";
const char QCameraParameters::KEY_QC_MAX_NUM_REQUESTED_FACES[] = "qc-max-num-requested-faces";

//Values for DENOISE
const char QCameraParameters::DENOISE_OFF[] = "denoise-off";
const char QCameraParameters::DENOISE_ON[] = "denoise-on";

// Values for selectable zone af Settings
const char QCameraParameters::SELECTABLE_ZONE_AF_AUTO[] = "auto";
const char QCameraParameters::SELECTABLE_ZONE_AF_SPOT_METERING[] = "spot-metering";
const char QCameraParameters::SELECTABLE_ZONE_AF_CENTER_WEIGHTED[] = "center-weighted";
const char QCameraParameters::SELECTABLE_ZONE_AF_FRAME_AVERAGE[] = "frame-average";

// Values for Face Detection settings.
const char QCameraParameters::FACE_DETECTION_OFF[] = "off";
const char QCameraParameters::FACE_DETECTION_ON[] = "on";

// Values for MCE settings.
const char QCameraParameters::MCE_ENABLE[] = "enable";
const char QCameraParameters::MCE_DISABLE[] = "disable";

// Values for HFR settings.
const char QCameraParameters::VIDEO_HFR_OFF[] = "off";
const char QCameraParameters::VIDEO_HFR_2X[] = "60";
const char QCameraParameters::VIDEO_HFR_3X[] = "90";
const char QCameraParameters::VIDEO_HFR_4X[] = "120";

// Values for Redeye Reduction settings.
const char QCameraParameters::REDEYE_REDUCTION_ENABLE[] = "enable";
const char QCameraParameters::REDEYE_REDUCTION_DISABLE[] = "disable";

// Values for HDR settings.
const char QCameraParameters::HDR_ENABLE[] = "enable";
const char QCameraParameters::HDR_DISABLE[] = "disable";

// Values for ZSL settings.
const char QCameraParameters::ZSL_OFF[] = "off";
const char QCameraParameters::ZSL_ON[] = "on";

// Values for HDR Bracketing settings.
const char QCameraParameters::AE_BRACKET_HDR_OFF[] = "Off";
const char QCameraParameters::AE_BRACKET_HDR[] = "HDR";
const char QCameraParameters::AE_BRACKET[] = "AE-Bracket";

const char QCameraParameters::LOW_POWER[] = "Low_Power";
const char QCameraParameters::NORMAL_POWER[] = "Normal_Power";

static const char* portrait = "portrait";
static const char* landscape = "landscape";

QCameraParameters::QCameraParameters()
    : CameraParameters(),
      m_pCapability(NULL),
      m_pCamOpsTbl(NULL),
      m_pParamHeap(NULL),
      mRecordingHint(false),
      m_bHistogramEnabled(false),
      m_bFaceDetectionEnabled(false),
      m_bDebugFps(false),
      m_nDumpFrameEnabled(0),
      mFocusMode(CAM_FOCUS_MODE_MAX)
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
      mFocusMode(CAM_FOCUS_MODE_MAX)
{
}

QCameraParameters::~QCameraParameters()
{
    if (NULL != m_pParamHeap) {
        m_pParamHeap->deallocate();
        delete m_pParamHeap;
        m_pParamHeap = NULL;
    }
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


String8 QCameraParameters::create_sizes_str(const cam_dimension_t *sizes, int len)
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
    str = params.get(QCameraParameters::KEY_VIDEO_SIZE);
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

    /* if(isRestartRequired(params)) {
           SEND STOP EVENT
           MARK FLAG FOR RESTART
       }
    */

    if ((rc = setPreviewSize(params)))                  final_rc = rc;
    if ((rc = setVideoSize(params)))                    final_rc = rc;
    if ((rc = setPictureSize(params)))                  final_rc = rc;
    //if ((rc = setPreviewFormat(params)))                final_rc = rc;


   /* if(IS FLAG SET FOR RESTART)
            SEND START EVENT
   */

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
    QCameraParameters param;
    String8 vSize;
    parm_buffer_t *p_table = (parm_buffer_t*) DATA_PTR(m_pParamHeap,0);
    if(initBatchUpdateTable(p_table) < 0 ) {
        ALOGE("%s:Failed to initialize group update table",__func__);
        rc = BAD_TYPE;
        goto INIT_DEF_DONE;
    }

    /**************************Initialize Strings****************************/
    mPictureSizeValues = create_sizes_str(
            m_pCapability->picture_sizes_tbl, m_pCapability->picture_sizes_tbl_cnt);
    mPreviewSizeValues = create_sizes_str(
            m_pCapability->preview_sizes_tbl, m_pCapability->preview_sizes_tbl_cnt);
    mVideoSizeValues = create_sizes_str(
            m_pCapability->video_sizes_tbl,m_pCapability->video_sizes_tbl_cnt);
  /*  mPreviewFormatValues = create_values_str(
            m_pCapability->supported_preview_fmts,pCap->supported_preview_fmt_cnt);*/

    /*************************Initialize Values******************************/

    //set supported preview sizes
    CameraParameters::setPreviewSize(m_pCapability->preview_sizes_tbl[0].width,
            m_pCapability->preview_sizes_tbl[0].height);
    QCameraParameters::set(QCameraParameters::KEY_SUPPORTED_PREVIEW_SIZES,
            mPreviewSizeValues.string());

    //set supported video sizes
    QCameraParameters::set(QCameraParameters::KEY_SUPPORTED_VIDEO_SIZES,
            mVideoSizeValues.string());
    vSize = create_sizes_str(&m_pCapability->video_sizes_tbl[0], 1);
    QCameraParameters::set(QCameraParameters::KEY_VIDEO_SIZE, vSize.string());

    // set supported picture sizes
    CameraParameters::setPictureSize(m_pCapability->picture_sizes_tbl[0].width,
            m_pCapability->picture_sizes_tbl[0].height);
    QCameraParameters::set(QCameraParameters::KEY_SUPPORTED_PICTURE_SIZES,
            mPictureSizeValues.string());

    //Set Preview Format
    CameraParameters::setPreviewFormat(QCameraParameters::PIXEL_FORMAT_YUV420SP);
    QCameraParameters::set(QCameraParameters::KEY_SUPPORTED_PREVIEW_FORMATS,
            mPreviewFormatValues.string());

    commitSetBatch();

INIT_DEF_DONE:
    return rc;
}

status_t QCameraParameters::init(cam_capability_t *capabilities, mm_camera_vtbl_t *mmOps)
{
    status_t rc = NO_ERROR;

    m_pCapability = capabilities;
    m_pCamOpsTbl = mmOps;

    /*Allocate Set Param Buffer*/
    m_pParamHeap = new QCameraHeapMemory();
    rc = m_pParamHeap->allocate(1, sizeof(parm_buffer_t));
    if(rc != OK) {
        rc = NO_MEMORY;
        ALOGE("Failed to allocate SETPARM Heap memory");
        goto TRANS_INIT_ERROR1;
    }

    /*Map memory for capability buffer*/
    rc = m_pCamOpsTbl->ops->map_buf(m_pCamOpsTbl->camera_handle,
                             CAM_MAPPING_BUF_TYPE_PARM_BUF,
                             m_pParamHeap->getFd(0),
                             sizeof(parm_buffer_t));
    if(rc < 0) {
        ALOGE("%s:failed to map SETPARM buffer",__func__);
        rc = FAILED_TRANSACTION;
        goto TRANS_INIT_ERROR2;
    }

    //    initDefaultParameters();

    goto TRANS_INIT_DONE;

TRANS_INIT_ERROR2:
    m_pParamHeap->deallocate();

TRANS_INIT_ERROR1:
    delete m_pParamHeap;
    m_pParamHeap = NULL;

TRANS_INIT_DONE:
    return rc;
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
    snprintf(str, sizeof(str), "%d,%d",minFPS,maxFPS);
    set(KEY_PREVIEW_FPS_RANGE,str);
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
    // TODO
    return HAL_PIXEL_FORMAT_YCrCb_420_SP;
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

bool QCameraParameters::isZSLMode()
{
    //TODO
    return false;
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

char *QCameraParameters::getExifDateTime()
{
    // TODO
    return NULL;
}

int32_t QCameraParameters::getExifFocalLength(rat_t */*focalLenght*/)
{
    // TODO
    return NO_ERROR;
}

uint16_t QCameraParameters::getExifIsoSpeed()
{
    // TODO
    return 0;
}

char * QCameraParameters::getExifGpsProcessingMethod()
{
    // TODO
    return NULL;
}

int32_t QCameraParameters::getExifLatitude(rat_t */*latitude*/,
                                           char */*latRef*/)
{
    // TODO
    return NO_ERROR;
}

int32_t QCameraParameters::getExifLongitude(rat_t */*longitude*/,
                                            char */*lonRef*/)
{
    // TODO
    return NO_ERROR;
}

int32_t QCameraParameters::getExifAltitude(rat_t */*altitude*/,
                                           char */*altRef*/)
{
    // TODO
    return NO_ERROR;
}

int32_t QCameraParameters::getExifGpsDateTimeStamp(char */*gpsDateStamp*/,
                                                   rat_t */*gpsTimeStamp*/)
{
    // TODO
    return NO_ERROR;
}

int32_t QCameraParameters::updateFocusDistances()
{
    parm_buffer_t *p_table = (parm_buffer_t*)DATA_PTR(m_pParamHeap,0);
    if(initBatchUpdateTable(p_table) < 0 ) {
        ALOGE("%s:Failed to initialize group update table",__func__);
        return BAD_TYPE;
    }

    int32_t rc = NO_ERROR;
    rc = AddGetParmEntryToBatch(p_table, CAM_INTF_PARM_FOCUS_DISTANCES);
    if (rc != NO_ERROR) {
        ALOGE("%s:Failed to update table",__func__);
        return rc;
    }

    rc = commitGetBatch();
    if (rc != NO_ERROR) {
        ALOGE("%s:Failed to GetParam",__func__);
        return rc;
    }

    cam_focus_distances_info_t *focusDistances =
        (cam_focus_distances_info_t *)POINTER_OF(CAM_INTF_PARM_FOCUS_DISTANCES, p_table);
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

    m_bHistogramEnabled = enabled;

    // TODO: set parm for histogram
    ALOGD(" Histogram -> %s", enabled ? "Enabled" : "Disabled");

    return NO_ERROR;
}

int32_t QCameraParameters::setFaceDetection(bool enabled)
{
    if(m_bFaceDetectionEnabled == enabled) {
        ALOGD("%s: face detection flag not changed, no ops here", __func__);
        return NO_ERROR;
    }

    m_bFaceDetectionEnabled = enabled;

    // TODO: set parm for face detection
    ALOGD(" FaceDetection -> %s", enabled ? "Enabled" : "Disabled");

    return NO_ERROR;
}

int32_t QCameraParameters::setAutoFocus(bool start)
{
    // TODO
    int32_t rc = NO_ERROR;
    if (start) {
        // start auto focus
    } else {
        // cancel auto focus
    }

    return rc;
}

int32_t QCameraParameters::prepareSnapshot()
{
    parm_buffer_t *p_table = (parm_buffer_t*)DATA_PTR(m_pParamHeap,0);
    if(initBatchUpdateTable(p_table) < 0 ) {
        ALOGE("%s:Failed to initialize group update table",__func__);
        return BAD_TYPE;
    }
    int8_t value = 1;
    int32_t rc = NO_ERROR;

    rc = AddSetParmEntryToBatch(p_table,
                                CAM_INTF_PARM_PREPARE_SNAPSHOT,
                                sizeof(value),
                                &value);
    if (rc != NO_ERROR) {
        ALOGE("%s:Failed to update table",__func__);
        return rc;
    }

    return commitSetBatch();
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

