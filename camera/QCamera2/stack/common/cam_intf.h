/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of Code Aurora Forum, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __QCAMERA_INTF_H__
#define __QCAMERA_INTF_H__

#include <stdint.h>
#include <pthread.h>
#include <inttypes.h>
#include <media/msmb_camera.h>

#define PAD_TO_WORD(a)               (((a)+3)&~3)
#define PAD_TO_2K(a)                 (((a)+2047)&~2047)
#define PAD_TO_4K(a)                 (((a)+4095)&~4095)
#define PAD_TO_8K(a)                 (((a)+8191)&~8191)

#define CEILING32(X) (((X) + 0x0001F) & 0xFFFFFFE0)
#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)
#define CEILING4(X)  (((X) + 0x0003) & 0xFFFC)
#define CEILING2(X)  (((X) + 0x0001) & 0xFFFE)

#define MAX_ZOOMS_CNT 64
#define MAX_SIZES_CNT 12
#define MAX_EXP_BRACKETING_LENGTH 32
#define MAX_ROI 5

typedef enum {
    CAM_POSITION_BACK,
    CAM_POSITION_FRONT
} cam_position_t;

typedef enum {
    CAM_FORMAT_YUV_420_NV12,
    CAM_FORMAT_YUV_420_NV21,
    CAM_FORMAT_YUV_420_NV21_ADRENO,
    CAM_FORMAT_BAYER_SBGGR10,
    CAM_FORMAT_RDI,
    CAM_FORMAT_YUV_420_YV12,
    CAM_FORMAT_YUV_422_NV16,
    CAM_FORMAT_YUV_422_NV61,
    CAM_FORMAT_YUV_422_YUYV,
    CAM_FORMAT_SAEC,
    CAM_FORMAT_SAWB,
    CAM_FORMAT_SAFC,
    CAM_FORMAT_SHST,
    CAM_FORMAT_MAX
} cam_format_t;

typedef enum {
    CAM_STREAM_TYPE_DEFAULT,   /* default stream type */
    CAM_STREAM_TYPE_PREVIEW,   /* preview */
    CAM_STREAM_TYPE_POSTVIEW,  /* postview */
    CAM_STREAM_TYPE_SNAPSHOT,  /* snapshot */
    CAM_STREAM_TYPE_VIDEO,     /* video */
    CAM_STREAM_TYPE_MAX,
} cam_stream_type_t;

typedef enum {
    CAM_PAD_NONE,
    CAM_PAD_TO_WORD,   /*2 bytes*/
    CAM_PAD_TO_LONG_WORD, /*4 bytes*/
    CAM_PAD_TO_8, /*8 bytes*/
    CAM_PAD_TO_16, /*16 bytes*/
    CAM_PAD_TO_1K, /*1k bytes*/
    CAM_PAD_TO_2K, /*2k bytes*/
    CAM_PAD_TO_4K,
    CAM_PAD_TO_8K
} cam_pad_format_t;

typedef enum {
    /* followings are per camera */
    CAM_MAPPING_BUF_TYPE_HIST_BUF,    /* histogram, need here? or put in meta data stream? */
    CAM_MAPPING_BUF_TYPE_CAPABILITY,  /* mapping camera capability buffer */
    CAM_MAPPING_BUF_TYPE_SETPARM_BUF, /* mapping set_parameters buffer */
    CAM_MAPPING_BUF_TYPE_GETPARM_BUF, /* mapping get_parameters buffer */

    /* followings are per stream */
    CAM_MAPPING_BUF_TYPE_STREAM_BUF,  /* mapping stream buffers */
    CAM_MAPPING_BUF_TYPE_STREAM_INFO, /* mapping stream information buffer */
    CAM_MAPPING_BUF_TYPE_MAX
} cam_mapping_buf_type;

typedef struct {
    cam_mapping_buf_type type;
    uint32_t stream_id;   /* stream id: valid if STREAM_BUF */
    uint32_t frame_idx;   /* frame index: valid if STREAM_BUF or HIST_BUF */
    unsigned long cookie; /* could be job_id(uint32_t) to identify mapping job */
    int fd;               /* origin fd */
    uint32_t size;        /* size of the buffer */
} cam_buf_map_type;

typedef struct {
    cam_mapping_buf_type type;
    uint32_t stream_id;   /* stream id: valid if STREAM_BUF */
    uint32_t frame_idx;   /* frame index: valid if STREAM_BUF or HIST_BUF */
    unsigned long cookie; /* could be job_id(uint32_t) to identify unmapping job */
} cam_buf_unmap_type;

typedef enum {
    CAM_MAPPING_TYPE_FD_MAPPING,
    CAM_MAPPING_TYPE_FD_UNMAPPING,
    CAM_MAPPING_TYPE_MAX
} cam_mapping_type;

typedef struct {
    cam_mapping_type msg_type;
    union {
        cam_buf_map_type buf_map;
        cam_buf_unmap_type buf_unmap;
    } payload;
} cam_sock_packet_t;

typedef enum {
    CAM_MODE_2D = (1<<0),
    CAM_MODE_3D = (1<<1)
} cam_mode_t;

typedef struct {
    uint32_t len;
    uint32_t y_offset;
    uint32_t cbcr_offset;
} cam_sp_len_offset_t;

typedef struct{
    uint32_t len;
    uint32_t offset;
} cam_mp_len_offset_t;

typedef struct {
    int num_planes;
    union {
        cam_sp_len_offset_t sp;
        cam_mp_len_offset_t mp[8];
    };
    uint32_t frame_len;
} cam_frame_len_offset_t;

typedef struct {
    int32_t width;
    int32_t height;
} cam_dimension_t;

typedef struct {
    cam_dimension_t dim;
    uint8_t frame_skip;
} cam_hfr_info_t;

typedef struct {
    float min_fps;
    float max_fps;
} cam_fps_range_t;

typedef enum {
    CAM_HFR_MODE_OFF,
    CAM_HFR_MODE_60FPS,
    CAM_HFR_MODE_90FPS,
    CAM_HFR_MODE_120FPS,
    CAM_HFR_MODE_150FPS,
} cam_hfr_mode_t;

typedef enum {
    CAM_WB_MODE_AUTO,
    CAM_WB_MODE_CUSTOM,
    CAM_WB_MODE_INCANDESCENT,
    CAM_WB_MODE_FLUORESCENT,
    CAM_WB_MODE_DAYLIGHT,
    CAM_WB_MODE_CLOUDY_DAYLIGHT,
    CAM_WB_MODE_TWILIGHT,
    CAM_WB_MODE_SHADE,
    CAM_WB_MODE_MAX
} cam_wb_mode_type;

typedef enum {
    CAM_ANTIBANDING_MODE_OFF,
    CAM_ANTIBANDING_MODE_60HZ,
    CAM_ANTIBANDING_MODE_50HZ,
    CAM_ANTIBANDING_MODE_AUTO,
    CAM_ANTIBANDING_MODE_AUTO_50HZ,
    CAM_ANTIBANDING_MODE_AUTO_60HZ,
    CAM_ANTIBANDING_MODE_MAX,
} cam_antibanding_mode_type;

/* Enum Type for different ISO Mode supported */
typedef enum {
    CAM_ISO_MODE_AUTO,
    CAM_ISO_MODE_DEBLUR,
    CAM_ISO_MODE_100,
    CAM_ISO_MODE_200,
    CAM_ISO_MODE_400,
    CAM_ISO_MODE_800,
    CAM_ISO_MODE_1600,
    CAM_ISO_MODE_MAX
} cam_iso_mode_type;

typedef enum {
    CAM_AEC_MODE_FRAME_AVERAGE,
    CAM_AEC_MODE_CENTER_WEIGHTED,
    CAM_AEC_MODE_SPOT_METERING,
    CAM_AEC_MODE_SMART_METERING,
    CAM_AEC_MODE_USER_METERING,
    CAM_AEC_MODE_SPOT_METERING_ADV,
    CAM_AEC_MODE_CENTER_WEIGHTED_ADV,
    CAM_AEC_MODE_MAX
} cam_auto_exposure_mode_type;

/* Auto focus mode */
typedef enum {
    CAM_FOCUS_MODE_NORMAL,
    CAM_FOCUS_MODE_MACRO,
    CAM_FOCUA_MODE_AUTO,
    CAM_FOCUS_MODE_CAF,
    CAM_FOCUS_MODE_INFINITY,
    CAM_FOCUS_MODE_MAX
} cam_focus_mode_type;

typedef enum {
    CAM_SCENE_MODE_OFF,
    CAM_SCENE_MODE_AUTO,
    CAM_SCENE_MODE_LANDSCAPE,
    CAM_SCENE_MODE_SNOW,
    CAM_SCENE_MODE_BEACH,
    CAM_SCENE_MODE_SUNSET,
    CAM_SCENE_MODE_NIGHT,
    CAM_SCENE_MODE_PORTRAIT,
    CAM_SCENE_MODE_BACKLIGHT,
    CAM_SCENE_MODE_SPORTS,
    CAM_SCENE_MODE_ANTISHAKE,
    CAM_SCENE_MODE_FLOWERS,
    CAM_SCENE_MODE_CANDLELIGHT,
    CAM_SCENE_MODE_FIREWORKS,
    CAM_SCENE_MODE_PARTY,
    CAM_SCENE_MODE_NIGHT_PORTRAIT,
    CAM_SCENE_MODE_THEATRE,
    CAM_SCENE_MODE_ACTION,
    CAM_SCENE_MODE_AR,
    CAM_SCENE_MODE_MAX
} cam_scene_mode_type;

typedef enum {
    CAM_EFFECT_MODE_OFF,
    CAM_EFFECT_MODE_MONO,
    CAM_EFFECT_MODE_NEGATIVE,
    CAM_EFFECT_MODE_SOLARIZE,
    CAM_EFFECT_MODE_SEPIA,
    CAM_EFFECT_MODE_POSTERIZE,
    CAM_EFFECT_MODE_WHITEBOARD,
    CAM_EFFECT_MODE_BLACKBOARD,
    CAM_EFFECT_MODE_AQUA,
    CAM_EFFECT_MODE_EMBOSS,
    CAM_EFFECT_MODE_SKETCH,
    CAM_EFFECT_MODE_NEON,
    CAM_EFFECT_MODE_MAX
} cam_effect_mode_type;

typedef enum {
    CAM_FLASH_MODE_OFF,
    CAM_FLASH_MODE_AUTO,
    CAM_FLASH_MODE_ON,
    CAM_FLASH_MODE_TORCH,
    CAM_FLASH_MODE_MAX
} cam_flash_mode_t;

typedef struct{
    int modes_supported;                                    /* mask of modes supported: 2D, 3D */
    cam_position_t position;                                /* sensor position: front, back */
    uint32_t sensor_mount_angle;                            /* sensor mount angle */

    float focal_length;                                     /* focal length */
    float hor_view_angle;                                   /* horizontal view angle */
    float ver_view_angle;                                   /* vertical view angle */

    uint8_t zoom_ratio_tbl_cnt;                             /* table size for zoom ratios */
    int zoom_ratio_tbl[MAX_ZOOMS_CNT];                      /* zoom ratios table */

    uint8_t preview_sizes_tbl_cnt;                          /* preview sizes table size */
    cam_dimension_t preview_sizes_tbl[MAX_SIZES_CNT];       /* preiew sizes table */

    uint8_t video_sizes_tbl_cnt;                            /* video sizes table size */
    cam_dimension_t video_sizes_tbl[MAX_SIZES_CNT];         /* video sizes table */

    uint8_t picture_sizes_tbl_cnt;                          /* picture sizes table size */
    cam_dimension_t picture_sizes_tbl[MAX_SIZES_CNT];       /* picture sizes table */

    uint8_t thunmbnail_sizes_tbl_cnt;                       /* thumbnail sizes table size */
    cam_dimension_t thunmbnail_sizes_tbl[MAX_SIZES_CNT];    /* thumbnail sizes table */

    uint8_t fps_ranges_tbl_cnt;                             /* fps ranges table size */
    cam_fps_range_t fps_ranges_tbl[MAX_SIZES_CNT];          /* fps ranges table */

    uint8_t hfr_tbl_cnt;                                    /* table size for HFR */
    cam_hfr_info_t hfr_tbl[MAX_SIZES_CNT];                  /* HFR table */
    cam_hfr_mode_t max_hfr_mode;                            /* max supported hfr mode */

    /* supported preview formats */
    uint8_t supported_preview_fmt_cnt;
    cam_format_t supported_preview_fmts[CAM_FORMAT_MAX];

    /* supported picture formats */
    uint8_t supported_picture_fmt_cnt;
    cam_format_t supported_picture_fmts[CAM_FORMAT_MAX];

    /* supported effect modes */
    uint8_t supported_effects_cnt;
    cam_effect_mode_type supported_effects[CAM_EFFECT_MODE_MAX];

    /* supported white balance modes */
    uint8_t supported_white_balances_cnt;
    cam_wb_mode_type supported_white_balances[CAM_WB_MODE_MAX];

    /* supported antibanding modes */
    uint8_t supported_antibandings_cnt;
    cam_antibanding_mode_type supported_antibandings[CAM_ANTIBANDING_MODE_MAX];

    /* supported scene modes */
    uint8_t supported_scene_modes_cnt;
    cam_scene_mode_type supported_scene_modes[CAM_SCENE_MODE_MAX];

    /* supported flash modes */
    uint8_t supported_flash_modes_cnt;
    cam_flash_mode_t supported_flash_modes[CAM_FLASH_MODE_MAX];

    /* supported focus modes */
    uint8_t supported_focus_modes_cnt;
    cam_focus_mode_type supported_focus_modes[CAM_FOCUS_MODE_MAX];

    /* supported iso modes */
    uint8_t supported_iso_modes_cnt;
    cam_iso_mode_type supported_iso_modes[CAM_ISO_MODE_MAX];

    /* supported auto exposure modes */
    uint8_t supported_aec_modes_cnt;
    cam_auto_exposure_mode_type supported_aec_modes[CAM_AEC_MODE_MAX];

    int exposure_compensation_min;       /* min value of exposure compensation */
    int exposure_compensation_max;       /* max value of exposure compensation */
    int exposure_compensation_step;      /* exposure compensation step value */

    uint8_t auto_wb_lock_supported;       /* flag if auto white balance lock is supported */
    uint8_t zoom_supported;               /* flag if zoom is supported */
    uint8_t smooth_zoom_supported;        /* flag if smooth zoom is supported */
    uint8_t auto_exposure_lock_supported; /* flag if auto exposure lock is supported */
    uint8_t video_snapshot_supported;     /* flag if video snapshot is supported */
    uint8_t video_stablization_supported; /* flag id video stablization is supported */

    uint8_t max_num_roi;                  /* max number of roi canbe detected */
    uint8_t max_num_focus_areas;          /* max num of focus areas */
    uint8_t max_num_metering_areas;       /* max num opf metering areas */
    uint8_t max_zoom_for_snap;            /* max zoom value for snapshot */
} cam_capapbility_t;

typedef struct  {
    int32_t left;
    int32_t top;
    int32_t width;
    int32_t height;
} cam_rect_t;

typedef enum {
    CAM_META_DATA_TYPE_DEF, /* no meta data header used */
    CAM_META_DATA_TYPE_MAX
} cam_meta_header_t;

typedef enum {
    CAM_STREAMING_MODE_CONTINUOUS, /* continous streaming */
    CAM_STREAMING_MODE_BURST,      /* burst streaming */
    CAM_STREAMING_MODE_MAX
} cam_streaming_mode_t;

/* stream info */
typedef struct {
    /* stream type*/
    cam_stream_type_t stream_type;

    /* sensor used */
    uint32_t sensor_idx;

    /* image format */
    cam_format_t src_fmt;     /* source image format */
    cam_format_t dest_fmt;    /* dest image format */

    /* image dimension */
    cam_dimension_t src_dim;  /* source image dimension */
    cam_dimension_t dest_dim; /* dest image dimension */

    /* image rotation */
    uint8_t rotation;

    /* streaming type */
    cam_streaming_mode_t streaming_mode;
    /* num of frames needs to be generated.
     * only valid when streaming_mode = CAM_STREAMING_MODE_BURST */
    uint8_t num_of_burst;

    cam_meta_header_t meta_header;
    cam_pad_format_t padding_format;

    /* the following fields are retrived from mctl */
    cam_frame_len_offset_t offset; /* frame offset */
    cam_rect_t crop;               /* crop information */
} cam_stream_info_t;

/* bundle information may put inside setparm buf?? */
#define MAX_STRAEM_NUM_IN_BUNDLE 4
typedef struct {
    uint8_t num;
    uint32_t stream_ids[MAX_STRAEM_NUM_IN_BUNDLE]; /* stream ids */
} cam_stream_bundle_t;

/* event from server */
typedef enum {
    CAM_EVENT_TYPE_MAP_UNMAP_DONE,
    CAM_EVENT_TYPE_AUTO_FOCUS_DONE,
    CAM_EVENT_TYPE_ZOOM_DONE,
    CAM_EVENT_TYPE_POST_PROC_DONE,
    CAM_EVENT_TYPE_MAX
} cam_event_type_t;

typedef enum {
    CAM_HDR_BRACKETING_OFF,
    CAM_HDR_MODE,
    CAM_EXP_BRACKETING_MODE
} cam_hdr_mode;

typedef struct {
    cam_hdr_mode mode;
    uint32_t hdr_enable;
    uint32_t total_frames;
    uint32_t total_hal_frames;
    char values[MAX_EXP_BRACKETING_LENGTH];  /* user defined values */
} cam_exp_bracketing_t;

typedef enum {
    CAM_AEC_ROI_OFF,
    CAM_AEC_ROI_ON
} cam_aec_roi_ctrl_t;

typedef enum {
    CAM_AEC_ROI_BY_INDEX,
    CAM_AEC_ROI_BY_COORDINATE,
} cam_aec_roi_type_t;

typedef struct {
    uint32_t x;
    uint32_t y;
} cam_coordinate_type_t;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t dx;
    uint16_t dy;
} cam_roi_t;

typedef struct {
    cam_aec_roi_ctrl_t aec_roi_enable;
    cam_aec_roi_type_t aec_roi_type;
    union {
        cam_coordinate_type_t coordinate;
        uint32_t aec_roi_idx;
    } cam_aec_roi_position;
} cam_set_aec_roi_t;

typedef struct {
    uint32_t frm_id;
    uint8_t num_roi;
    cam_roi_t roi[MAX_ROI];
    uint8_t is_multiwindow;
} cam_roi_info_t;

typedef struct {
    int denoise_enable;
    int process_plates;
} cam_denoise_param_t;

typedef struct {
    int fd_mode;
    int num_fd;
} cam_fd_set_parm_t;

/*************************************/
/****The following part is for jpeg***/
/*************************************/
/* Exif Tag Data Type */
typedef enum
{
    EXIF_BYTE      = 1,
    EXIF_ASCII     = 2,
    EXIF_SHORT     = 3,
    EXIF_LONG      = 4,
    EXIF_RATIONAL  = 5,
    EXIF_UNDEFINED = 7,
    EXIF_SLONG     = 9,
    EXIF_SRATIONAL = 10
} exif_tag_type_t;

/* Exif Rational Data Type */
typedef struct
{
    uint32_t  num;    // Numerator
    uint32_t  denom;  // Denominator
} rat_t;

/* Exif Signed Rational Data Type */
typedef struct
{
    int32_t  num;    // Numerator
    int32_t  denom;  // Denominator
} srat_t;

typedef struct
{
  exif_tag_type_t type;
  uint8_t copy;
  uint32_t count;
  union
  {
    char      *_ascii;
    uint8_t   *_bytes;
    uint8_t    _byte;
    uint16_t  *_shorts;
    uint16_t   _short;
    uint32_t  *_longs;
    uint32_t   _long;
    rat_t     *_rats;
    rat_t      _rat;
    uint8_t   *_undefined;
    int32_t   *_slongs;
    int32_t    _slong;
    srat_t    *_srats;
    srat_t     _srat;
  } data;
} exif_tag_entry_t;

typedef struct {
    uint32_t      tag_id;
    exif_tag_entry_t  tag_entry;
} exif_tags_info_t;

/* Exif Tag ID */
typedef uint32_t exif_tag_id_t;
/* Exif Info (opaque definition) */
struct exif_info_t;
typedef struct exif_info_t * exif_info_obj_t;

/*****************************************************************************
 *                 Code for Domain Socket Based Parameters                   *
 ****************************************************************************/

#define POINTER_OF(PARAM_ID,TABLE_PTR)    \
        (&(TABLE_PTR->entry[PARAM_ID].data))

#define GET_FIRST_PARAM_ID(TABLE_PTR)     \
        (TABLE_PTR->first_flagged_entry)

#define SET_FIRST_PARAM_ID(TABLE_PTR,PARAM_ID)     \
        TABLE_PTR->first_flagged_entry=PARAM_ID

#define GET_NEXT_PARAM_ID(CURRENT_PARAM_ID,TABLE_PTR)    \
        (TABLE_PTR->entry[CURRENT_PARAM_ID].next_flagged_entry)

#define SET_NEXT_PARAM_ID(CURRENT_PARAM_ID,TABLE_PTR,NEXT_PARAM_ID)    \
        TABLE_PTR->entry[CURRENT_PARAM_ID].next_flagged_entry=NEXT_PARAM_ID;

#define INCLUDE(PARAM_ID,DATATYPE,COUNT)  \
        DATATYPE member_variable_##PARAM_ID[ COUNT ]

typedef enum{
    CAM_INTF_PARM_FOCUS_DISTANCES,
    CAM_INTF_PARM_QUERY_FALSH4SNAP,
    CAM_INTF_PARM_3D_FRAME_FORMAT,
    CAM_INTF_PARM_FPS_RANGE,
    CAM_INTF_PARM_OP_MODE,
    CAM_INTF_PARM_DIMENSION,
    CAM_INTF_PARM_SNAPSHOT_BURST_NUM,
    CAM_INTF_PARM_EXPOSURE,
    CAM_INTF_PARM_SHARPNESS,
    CAM_INTF_PARM_CONTRAST,
    CAM_INTF_PARM_SATURATION,
    CAM_INTF_PARM_BRIGHTNESS,
    CAM_INTF_PARM_WHITE_BALANCE,
    CAM_INTF_PARM_ISO,
    CAM_INTF_PARM_ZOOM,
    CAM_INTF_PARM_LUMA_ADAPTATION,
    CAM_INTF_PARM_ANTIBANDING,
    CAM_INTF_PARM_CONTINUOUS_AF,
    CAM_INTF_PARM_HJR,
    CAM_INTF_PARM_EFFECT,
    CAM_INTF_PARM_FPS,
    CAM_INTF_PARM_FPS_MODE,
    CAM_INTF_PARM_EXPOSURE_COMPENSATION,
    CAM_INTF_PARM_LED_MODE,
    CAM_INTF_PARM_ROLLOFF,
    CAM_INTF_PARM_MODE,
    CAM_INTF_PARM_FOCUS_RECT,
    CAM_INTF_PARM_AEC_ROI,
    CAM_INTF_PARM_AF_ROI,
    CAM_INTF_PARM_FOCUS_MODE,
    CAM_INTF_PARM_CAF_ENABLE,
    CAM_INTF_PARM_BESTSHOT_MODE,
    CAM_INTF_PARM_VIDEO_DIS,
    CAM_INTF_PARM_VIDEO_ROT,
    CAM_INTF_PARM_SCE_FACTOR,
    CAM_INTF_PARM_FD,
    CAM_INTF_PARM_AEC_LOCK,
    CAM_INTF_PARM_AWB_LOCK,
    CAM_INTF_PARM_MCE,
    CAM_INTF_PARM_HORIZONTAL_VIEW_ANGLE,
    CAM_INTF_PARM_VERTICAL_VIEW_ANGLE,
    CAM_INTF_PARM_RESET_LENS_TO_INFINITY,
    CAM_INTF_PARM_HFR,
    CAM_INTF_PARM_REDEYE_REDUCTION,
    CAM_INTF_PARM_WAVELET_DENOISE,
    CAM_INTF_PARM_3D_DISPLAY_DISTANCE,
    CAM_INTF_PARM_3D_VIEW_ANGLE,
    CAM_INTF_PARM_ZOOM_RATIO,
    CAM_INTF_PARM_HISTOGRAM,
    CAM_INTF_PARM_ASD_ENABLE,
    CAM_INTF_PARM_RECORDING_HINT,
    CAM_INTF_PARM_PREVIEW_FORMAT,
    CAM_INTF_PARM_DIS_ENABLE,
    CAM_INTF_PARM_FULL_LIVESHOT,
    CAM_INTF_PARM_LOW_POWER_MODE,
    CAM_INTF_PARM_HDR,
    CAM_INTF_PARM_MAX
} cam_intf_parm_type_t;


typedef union {
/*************************************************************************************
 *          ID from (cam_intf_parm_type_t)         DATATYPE                   COUNT
 *************************************************************************************/
    INCLUDE(CAM_INTF_PARM_QUERY_FALSH4SNAP,        int,                       1);
    INCLUDE(CAM_INTF_PARM_HDR,                     cam_exp_bracketing_t,      1);
} get_parm_type_t;



typedef union {
/**************************************************************************************
 *          ID from (cam_intf_parm_type_t)          DATATYPE                     COUNT
 **************************************************************************************/
    INCLUDE(CAM_INTF_PARM_SNAPSHOT_BURST_NUM,       uint32_t,                    1);
    INCLUDE(CAM_INTF_PARM_EXPOSURE,                 int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_SHARPNESS,                int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_CONTRAST,                 int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_SATURATION,               int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_BRIGHTNESS,               int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_WHITE_BALANCE,            int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_ISO,                      int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_ZOOM,                     int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_LUMA_ADAPTATION,          int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_ANTIBANDING,              int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_CONTINUOUS_AF,            int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_HJR,                      int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_EFFECT,                   int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_FPS,                      uint32_t,                    1);
    INCLUDE(CAM_INTF_PARM_FPS_MODE,                 int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_EXPOSURE_COMPENSATION,    int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_LED_MODE,                 int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_ROLLOFF,                  int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_MODE,                     cam_mode_t,                  1);
    INCLUDE(CAM_INTF_PARM_FOCUS_RECT,               int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_AEC_ROI,                  cam_set_aec_roi_t,           1);
    INCLUDE(CAM_INTF_PARM_AF_ROI,                   cam_roi_info_t,              1);
    INCLUDE(CAM_INTF_PARM_FOCUS_MODE,               int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_CAF_ENABLE,               uint32_t,                    1);
    INCLUDE(CAM_INTF_PARM_BESTSHOT_MODE,            int32_t,                     1);
//    INCLUDE(CAM_INTF_PARM_VIDEO_DIS,                video_dis_param_ctrl_t,      1);
//    INCLUDE(CAM_INTF_PARM_VIDEO_ROT,                video_rotation_param_ctrl_t, 1);
    INCLUDE(CAM_INTF_PARM_SCE_FACTOR,               int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_FD,                       cam_fd_set_parm_t,           1);
    INCLUDE(CAM_INTF_PARM_AEC_LOCK,                 int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_AWB_LOCK,                 int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_MCE,                      int32_t,                     1);
//    INCLUDE(CAM_INTF_PARM_HORIZONTAL_VIEW_ANGLE,    focus_distances_info_t,      1);
//    INCLUDE(CAM_INTF_PARM_VERTICAL_VIEW_ANGLE,      focus_distances_info_t,      1);
    INCLUDE(CAM_INTF_PARM_RESET_LENS_TO_INFINITY,   uint8_t,                     1);
    INCLUDE(CAM_INTF_PARM_HFR,                      int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_REDEYE_REDUCTION,         int32_t,                     1);
    INCLUDE(CAM_INTF_PARM_WAVELET_DENOISE,          cam_denoise_param_t,         1);
    INCLUDE(CAM_INTF_PARM_3D_DISPLAY_DISTANCE,      float,                       1);
    INCLUDE(CAM_INTF_PARM_3D_VIEW_ANGLE,            uint32_t,                    1);
    INCLUDE(CAM_INTF_PARM_ZOOM_RATIO,               uint32_t,                    1);
    INCLUDE(CAM_INTF_PARM_HISTOGRAM,                int8_t,                      1);
    INCLUDE(CAM_INTF_PARM_ASD_ENABLE,               uint32_t,                    1);
    INCLUDE(CAM_INTF_PARM_RECORDING_HINT,           uint32_t,                    1);
    INCLUDE(CAM_INTF_PARM_PREVIEW_FORMAT,           uint32_t,                    1);
    INCLUDE(CAM_INTF_PARM_DIS_ENABLE,               uint32_t,                    1);
    INCLUDE(CAM_INTF_PARM_FULL_LIVESHOT,            uint32_t,                    1);
    INCLUDE(CAM_INTF_PARM_LOW_POWER_MODE,           uint32_t,                    1);
    INCLUDE(CAM_INTF_PARM_HDR,                      cam_exp_bracketing_t,        1);
} set_parm_type_t;

/****************************DO NOT MODIFY BELOW THIS LINE!!!!*********************/



typedef struct {
    get_parm_type_t data;
    uint8_t next_flagged_entry;
} get_parm_entry_type_t;

typedef struct {
    uint8_t first_flagged_entry;
    get_parm_entry_type_t entry[CAM_INTF_PARM_MAX];
} get_parm_buffer_t;

typedef struct {
    set_parm_type_t data;
    uint8_t next_flagged_entry;
} set_parm_entry_type_t;

typedef struct {
    uint8_t first_flagged_entry;
    set_parm_entry_type_t entry[CAM_INTF_PARM_MAX];
} set_parm_buffer_t;

typedef enum {
    MM_CAMERA_SET_PARAMETERS,
    MM_CAMERA_GET_PARAMETERS
} transactionType;



#endif /* __QCAMERA_INTF_H__ */
