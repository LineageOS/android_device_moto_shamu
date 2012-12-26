/*
**
** Copyright 2008, The Android Open Source Project
** Copyright (c) 2012, The Linux Foundation. All rights reserved.
** Not a Contribution. Apache license notifications and license are
** retained for attribution purposes only.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
#ifndef ANDROID_HARDWARE_QCAMERA_PARAMETERS_H
#define ANDROID_HARDWARE_QCAMERA_PARAMETERS_H

#include <camera/CameraParameters.h>
#include <cutils/properties.h>
#include <hardware/camera.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <utils/Errors.h>
#include "cam_intf.h"
#include "QCameraMem.h"

extern "C" {
#include <mm_jpeg_interface.h>
}

namespace android {

struct FPSRange{
    int minFPS;
    int maxFPS;
    FPSRange(){
        minFPS = 0;
        maxFPS = 0;
    };
    FPSRange(int min,int max){
        minFPS = min;
        maxFPS = max;
    };
};

//EXIF globals
static const char ExifAsciiPrefix[] = { 0x41, 0x53, 0x43, 0x49, 0x49, 0x0, 0x0, 0x0 };          // "ASCII\0\0\0"
static const char ExifUndefinedPrefix[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };   // "\0\0\0\0\0\0\0\0"

#define GPS_PROCESSING_METHOD_SIZE       101
#define EXIF_ASCII_PREFIX_SIZE           8   //(sizeof(ExifAsciiPrefix))
#define FOCAL_LENGTH_DECIMAL_PRECISION   100

class QCameraParameters: public CameraParameters
{
public:
    QCameraParameters();
    QCameraParameters(const String8 &params);
    ~QCameraParameters();

    // Supported PREVIEW/RECORDING SIZES IN HIGH FRAME RATE recording, sizes in pixels.
    // Example value: "800x480,432x320". Read only.
    static const char KEY_QC_SUPPORTED_HFR_SIZES[];
    // The mode of preview frame rate.
    // Example value: "frame-rate-auto, frame-rate-fixed".
    static const char KEY_QC_PREVIEW_FRAME_RATE_MODE[];
    static const char KEY_QC_SUPPORTED_PREVIEW_FRAME_RATE_MODES[];
    static const char KEY_QC_PREVIEW_FRAME_RATE_AUTO_MODE[];
    static const char KEY_QC_PREVIEW_FRAME_RATE_FIXED_MODE[];

    static const char KEY_QC_SKIN_TONE_ENHANCEMENT[] ;
    static const char KEY_QC_SUPPORTED_SKIN_TONE_ENHANCEMENT_MODES[] ;

    //Touch Af/AEC settings.
    static const char KEY_QC_TOUCH_AF_AEC[];
    static const char KEY_QC_SUPPORTED_TOUCH_AF_AEC[];
    //Touch Index for AEC.
    static const char KEY_QC_TOUCH_INDEX_AEC[];
    //Touch Index for AF.
    static const char KEY_QC_TOUCH_INDEX_AF[];
    // Current auto scene detection mode.
    // Example value: "off" or "on" constants. Read/write.
    static const char KEY_QC_SCENE_DETECT[];
    // Supported auto scene detection settings.
    // Example value: "off,on". Read only.
    static const char KEY_QC_SUPPORTED_SCENE_DETECT[];

    static const char KEY_QC_ISO_MODE[];
    static const char KEY_QC_SUPPORTED_ISO_MODES[];
    static const char KEY_QC_LENSSHADE[] ;
    static const char KEY_QC_SUPPORTED_LENSSHADE_MODES[] ;

    static const char KEY_QC_AUTO_EXPOSURE[];
    static const char KEY_QC_SUPPORTED_AUTO_EXPOSURE[];

    static const char KEY_QC_GPS_LATITUDE_REF[];
    static const char KEY_QC_GPS_LONGITUDE_REF[];
    static const char KEY_QC_GPS_ALTITUDE_REF[];
    static const char KEY_QC_GPS_STATUS[];
    static const char KEY_QC_EXIF_DATETIME[];
    static const char KEY_QC_MEMORY_COLOR_ENHANCEMENT[];
    static const char KEY_QC_SUPPORTED_MEM_COLOR_ENHANCE_MODES[];

    static const char KEY_QC_ZSL[];
    static const char KEY_QC_SUPPORTED_ZSL_MODES[];

    static const char KEY_QC_CAMERA_MODE[];

    static const char KEY_QC_VIDEO_HIGH_FRAME_RATE[];
    static const char KEY_QC_SUPPORTED_VIDEO_HIGH_FRAME_RATE_MODES[];
    static const char KEY_QC_HIGH_DYNAMIC_RANGE_IMAGING[];
    static const char KEY_QC_SUPPORTED_HDR_IMAGING_MODES[];
    static const char KEY_QC_AE_BRACKET_HDR[];

    // DENOISE
    static const char KEY_QC_DENOISE[];
    static const char KEY_QC_SUPPORTED_DENOISE[];

    //Selectable zone AF.
    static const char KEY_QC_FOCUS_ALGO[];
    static const char KEY_QC_SUPPORTED_FOCUS_ALGOS[];

    //Face Detection
    static const char KEY_QC_FACE_DETECTION[];
    static const char KEY_QC_SUPPORTED_FACE_DETECTION[];

    // supported camera features to be queried by Snapdragon SDK
    //Read only
    static const char KEY_QC_SUPPORTED_CAMERA_FEATURES[];

    //Indicates number of faces requested by the application.
    //This value will be rejected if the requested faces
    //greater than supported by hardware.
    //Write only.
    static const char KEY_QC_MAX_NUM_REQUESTED_FACES[];

    //Redeye Reduction
    static const char KEY_QC_REDEYE_REDUCTION[];
    static const char KEY_QC_SUPPORTED_REDEYE_REDUCTION[];
    static const char EFFECT_EMBOSS[];
    static const char EFFECT_SKETCH[];
    static const char EFFECT_NEON[];

    // Values for Touch AF/AEC
    static const char TOUCH_AF_AEC_OFF[];
    static const char TOUCH_AF_AEC_ON[];

    // Values for Scene mode
    static const char SCENE_MODE_ASD[];
    static const char SCENE_MODE_BACKLIGHT[];
    static const char SCENE_MODE_FLOWERS[];
    static const char SCENE_MODE_AR[];
    static const char PIXEL_FORMAT_YUV420SP_ADRENO[]; // ADRENO
    static const char PIXEL_FORMAT_RAW[];
    static const char PIXEL_FORMAT_YV12[]; // NV12
    static const char PIXEL_FORMAT_NV12[]; //NV12

    // ISO values
    static const char ISO_AUTO[];
    static const char ISO_HJR[];
    static const char ISO_100[];
    static const char ISO_200[];
    static const char ISO_400[];
    static const char ISO_800[];
    static const char ISO_1600[];

    // Values for auto exposure settings.
    static const char AUTO_EXPOSURE_FRAME_AVG[];
    static const char AUTO_EXPOSURE_CENTER_WEIGHTED[];
    static const char AUTO_EXPOSURE_SPOT_METERING[];
    static const char AUTO_EXPOSURE_SMART_METERING[];
    static const char AUTO_EXPOSURE_USER_METERING[];
    static const char AUTO_EXPOSURE_SPOT_METERING_ADV[];
    static const char AUTO_EXPOSURE_CENTER_WEIGHTED_ADV[];

    static const char KEY_QC_SHARPNESS[];
    static const char KEY_QC_MIN_SHARPNESS[];
    static const char KEY_QC_MAX_SHARPNESS[];
    static const char KEY_QC_SHARPNESS_STEP[];
    static const char KEY_QC_CONTRAST[];
    static const char KEY_QC_MIN_CONTRAST[];
    static const char KEY_QC_MAX_CONTRAST[];
    static const char KEY_QC_CONTRAST_STEP[];
    static const char KEY_QC_SATURATION[];
    static const char KEY_QC_MIN_SATURATION[];
    static const char KEY_QC_MAX_SATURATION[];
    static const char KEY_QC_SATURATION_STEP[];
    static const char KEY_QC_BRIGHTNESS[];
    static const char KEY_QC_MIN_BRIGHTNESS[];
    static const char KEY_QC_MAX_BRIGHTNESS[];
    static const char KEY_QC_BRIGHTNESS_STEP[];
    static const char KEY_QC_SCE_FACTOR[];
    static const char KEY_QC_MIN_SCE_FACTOR[];
    static const char KEY_QC_MAX_SCE_FACTOR[];
    static const char KEY_QC_SCE_FACTOR_STEP[];

    static const char KEY_QC_HISTOGRAM[] ;
    static const char KEY_QC_SUPPORTED_HISTOGRAM_MODES[] ;

    // Values for SKIN TONE ENHANCEMENT
    static const char SKIN_TONE_ENHANCEMENT_ENABLE[] ;
    static const char SKIN_TONE_ENHANCEMENT_DISABLE[] ;

    // Values for Denoise
    static const char DENOISE_OFF[] ;
    static const char DENOISE_ON[] ;

    // Values for auto exposure settings.
    static const char FOCUS_ALGO_AUTO[];
    static const char FOCUS_ALGO_SPOT_METERING[];
    static const char FOCUS_ALGO_CENTER_WEIGHTED[];
    static const char FOCUS_ALGO_FRAME_AVERAGE[];

    // Values for HDR Bracketing settings.
    static const char AE_BRACKET_HDR_OFF[];
    static const char AE_BRACKET_HDR[];
    static const char AE_BRACKET[];

    // Values for HFR settings.
    static const char VIDEO_HFR_OFF[];
    static const char VIDEO_HFR_2X[];
    static const char VIDEO_HFR_3X[];
    static const char VIDEO_HFR_4X[];
    static const char VIDEO_HFR_5X[];

    // Values for feature on/off settings.
    static const char VALUE_OFF[];
    static const char VALUE_ON[];

    // Values for feature enable/disable settings.
    static const char VALUE_ENABLE[];
    static const char VALUE_DISABLE[];

    // Values for feature true/false settings.
    static const char VALUE_FALSE[];
    static const char VALUE_TRUE[];

    enum {
        CAMERA_ORIENTATION_UNKNOWN = 0,
        CAMERA_ORIENTATION_PORTRAIT = 1,
        CAMERA_ORIENTATION_LANDSCAPE = 2,
    };
    typedef struct {
        const char *const desc;
        int val;
    } QCameraMap;

    int getOrientation() const;
    void setOrientation(int orientation);
    void getSupportedHfrSizes(Vector<Size> &sizes) const;
    void setPreviewFpsRange(int minFPS,int maxFPS);
    void setPreviewFrameRateMode(const char *mode);
    const char *getPreviewFrameRateMode() const;
    void setTouchIndexAec(int x, int y);
    void getTouchIndexAec(int *x, int *y) const;
    void setTouchIndexAf(int x, int y);
    void getTouchIndexAf(int *x, int *y) const;
    void getMeteringAreaCenter(int * x, int *y) const;

    status_t init(cam_capability_t *, mm_camera_vtbl_t *);
    void deinit();
    status_t assign(QCameraParameters& params);
    status_t initDefaultParameters();
    status_t updateParameters(QCameraParameters&, bool &needRestart);
    status_t commitParameters();
    status_t setAutoExposure(const QCameraParameters& );
    status_t setPreviewSize(const QCameraParameters& );
    status_t setVideoSize(const QCameraParameters& );
    status_t setPictureSize(const QCameraParameters& );
    status_t setPreviewFormat(const QCameraParameters& );
    status_t setPictureFormat(const QCameraParameters& );
    status_t setPreviewFrameRate(const QCameraParameters& );

    int getPreviewHalPixelFormat() const;
    status_t getStreamFormat(cam_stream_type_t streamType,
                             cam_format_t &format);
    status_t getStreamDimension(cam_stream_type_t streamType,
                                cam_dimension_t &dim);
    void getThumbnailSize(int *width, int *height) const;

    int getZSLBurstInterval();
    int getZSLQueueDepth();
    int getZSLBackLookCount();
    bool isZSLMode() {return mZslMode;};
    bool isNoDisplayMode();
    bool isWNREnabled();
    bool isSmoothZoomRunning();
    uint8_t getNumOfSnapshots();
    bool getRecordingHintValue() {return mRecordingHint;}; // return local copy of video hint
    int setRecordingHintValue(bool value); // set local copy of video hint and send to server
                                           // no change in parameters value
    int getJpegQuality();
    int getJpegRotation();

    int32_t getExifDateTime(char *dateTime, uint32_t &count);
    int32_t getExifFocalLength(rat_t *focalLenght);
    uint16_t getExifIsoSpeed();
    int32_t getExifGpsProcessingMethod(char *gpsProcessingMethod, uint32_t &count);
    int32_t getExifLatitude(rat_t *latitude, char *latRef);
    int32_t getExifLongitude(rat_t *longitude, char *lonRef);
    int32_t getExifAltitude(rat_t *altitude, char *altRef);
    int32_t getExifGpsDateTimeStamp(char *gpsDateStamp, uint32_t bufLen, rat_t *gpsTimeStamp);
    int32_t updateFocusDistances(cam_focus_distances_info_t *focusDistances);

    bool isFpsDebugEnabled() {return m_bDebugFps;};
    bool isHistogramEnabled() {return m_bHistogramEnabled;};
    bool isFaceDetectionEnabled() {return m_bFaceDetectionEnabled;};
    int32_t setHistogram(bool enabled);
    int32_t setFaceDetection(bool enabled);
    int getEnabledFileDumpMask() {return m_nDumpFrameEnabled;};

    cam_focus_mode_type getFocusMode();

private:
    int parseGPSCoordinate(const char *coord_str, rat_t* coord);
    int32_t getRational(rat_t *rat, int num, int denom);
    String8 createSizesString(const cam_dimension_t *sizes, int len);
    String8 createValuesString(const int *values, int len,
                               const QCameraMap* map, int map_len);
    String8 createValuesStringFromMap(const QCameraMap* map,
                                      int map_len);
    String8 createHfrValuesString(const cam_hfr_info_t *values, int len,
                                  const QCameraMap* map, int map_len);
    String8 createHfrSizesString(const cam_hfr_info_t *values, int len);
    String8 createFpsString(const cam_fps_range_t* fps, int len);
    String8 createFrameratesString(const cam_fps_range_t *fps, int len);
    String8 createZoomRatioValuesString(int *zoomRatios, int length);
    int lookupAttr(const QCameraMap arr[], int len, const char *name);

    // ops for batch set/get params with server
    int initBatchUpdateTable(parm_buffer_t *p_table);
    int AddSetParmEntryToBatch(parm_buffer_t *p_table,
                               cam_intf_parm_type_t paramType,
                               uint32_t paramLength,
                               void *paramValue);
    int commitSetBatch();
    int AddGetParmEntryToBatch(parm_buffer_t *p_table,
                               cam_intf_parm_type_t paramType);
    int commitGetBatch();

    // Map from strings to values
    static const cam_dimension_t THUMBNAIL_SIZES_MAP[];
    static const QCameraMap AUTO_EXPOSURE_MAP[];
    static const QCameraMap PREVIEW_FORMATS_MAP[];
    static const QCameraMap FOCUS_MODES_MAP[];
    static const QCameraMap EFFECT_MODES_MAP[];
    static const QCameraMap SCENE_MODES_MAP[];
    static const QCameraMap FLASH_MODES_MAP[];
    static const QCameraMap FOCUS_ALGO_MAP[];
    static const QCameraMap WHITE_BALANCE_MODES_MAP[];
    static const QCameraMap ANTIBANDING_MODES_MAP[];
    static const QCameraMap ISO_MODES_MAP[];
    static const QCameraMap HFR_MODES_MAP[];
    static const QCameraMap HDR_BRACKETING_MODES_MAP[];
    static const QCameraMap ON_OFF_MODES_MAP[];
    static const QCameraMap ENABLE_DISABLE_MODES_MAP[];
    static const QCameraMap DENOISE_ON_OFF_MODES_MAP[];
    static const QCameraMap TRUE_FALSE_MODES_MAP[];

    cam_capability_t *m_pCapability;
    mm_camera_vtbl_t *m_pCamOpsTbl;
    QCameraHeapMemory *m_pParamHeap;
    bool mZslMode;                  // if ZSL is enabled
    bool mRecordingHint;            // local copy of recording hint
    bool m_bHistogramEnabled;       // if histogram is enabled
    bool m_bFaceDetectionEnabled;   // if face detection is enabled
    bool m_bDebugFps;               // if FPS need to be logged
    int  m_nDumpFrameEnabled;       // mask for type of dumping enabled
    cam_focus_mode_type mFocusMode;

    cam_format_t mPreviewFormat;
    int32_t mFps;
};

}; // namespace android

#endif
