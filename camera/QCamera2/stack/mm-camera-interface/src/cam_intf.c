/* Copyright (c) 2014, The Linux Foundation. All rights reserved.
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
 *     * Neither the name of The Linux Foundation nor the names of its
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

#include "cam_intf.h"

void *POINTER_OF_PARAM(cam_intf_parm_type_t PARAM_ID,
                 void *table_ptr)
{
    parm_buffer_new_t *TABLE_PTR = (parm_buffer_new_t *)table_ptr;
    int32_t j = 0, i = TABLE_PTR->num_entry;
    parm_entry_type_new_t *curr_param =
                (parm_entry_type_new_t *)&TABLE_PTR->entry[0];

    for (j = 0; j < i; j++) {
      if (PARAM_ID == curr_param->entry_type) {
        return (void *)&curr_param->data[0];
      }
      curr_param = GET_NEXT_PARAM(curr_param, parm_entry_type_new_t);
    }
    curr_param = (parm_entry_type_new_t *)&TABLE_PTR->entry[0];
    return NULL;
}

void *get_pointer_of(cam_intf_parm_type_t meta_id,
        const metadata_buffer_t* metadata)
{
    switch(meta_id) {
        case CAM_INTF_META_HISTOGRAM:
            return POINTER_OF_META(CAM_INTF_META_HISTOGRAM, metadata);
        case CAM_INTF_META_FACE_DETECTION:
            return POINTER_OF_META(CAM_INTF_META_FACE_DETECTION, metadata);
        case CAM_INTF_META_AUTOFOCUS_DATA:
            return POINTER_OF_META(CAM_INTF_META_AUTOFOCUS_DATA, metadata);
        case CAM_INTF_META_CROP_DATA:
            return POINTER_OF_META(CAM_INTF_META_CROP_DATA, metadata);
        case CAM_INTF_META_PREP_SNAPSHOT_DONE:
            return POINTER_OF_META(CAM_INTF_META_PREP_SNAPSHOT_DONE, metadata);
        case CAM_INTF_META_GOOD_FRAME_IDX_RANGE:
            return POINTER_OF_META(CAM_INTF_META_GOOD_FRAME_IDX_RANGE, metadata);
        case CAM_INTF_META_ASD_HDR_SCENE_DATA:
            return POINTER_OF_META(CAM_INTF_META_ASD_HDR_SCENE_DATA, metadata);
        case CAM_INTF_META_ASD_SCENE_TYPE:
            return POINTER_OF_META(CAM_INTF_META_ASD_SCENE_TYPE, metadata);
        case CAM_INTF_META_CHROMATIX_LITE_ISP:
            return POINTER_OF_META(CAM_INTF_META_CHROMATIX_LITE_ISP, metadata);
        case CAM_INTF_META_CHROMATIX_LITE_PP:
            return POINTER_OF_META(CAM_INTF_META_CHROMATIX_LITE_PP, metadata);
        case CAM_INTF_META_CHROMATIX_LITE_AE:
            return POINTER_OF_META(CAM_INTF_META_CHROMATIX_LITE_AE, metadata);
        case CAM_INTF_META_CHROMATIX_LITE_AWB:
            return POINTER_OF_META(CAM_INTF_META_CHROMATIX_LITE_AWB, metadata);
        case CAM_INTF_META_CHROMATIX_LITE_AF:
            return POINTER_OF_META(CAM_INTF_META_CHROMATIX_LITE_AF, metadata);
        case CAM_INTF_META_VALID:
            return POINTER_OF_META(CAM_INTF_META_VALID, metadata);
        case CAM_INTF_META_FRAME_DROPPED:
            return POINTER_OF_META(CAM_INTF_META_FRAME_DROPPED, metadata);
        case CAM_INTF_META_PENDING_REQUESTS:
            return POINTER_OF_META(CAM_INTF_META_PENDING_REQUESTS, metadata);
        case CAM_INTF_META_FRAME_NUMBER:
            return POINTER_OF_META(CAM_INTF_META_FRAME_NUMBER, metadata);
        case CAM_INTF_META_COLOR_CORRECT_MODE:
            return POINTER_OF_META(CAM_INTF_META_COLOR_CORRECT_MODE, metadata);
        case CAM_INTF_META_COLOR_CORRECT_TRANSFORM:
            return POINTER_OF_META(CAM_INTF_META_COLOR_CORRECT_TRANSFORM, metadata);
        case CAM_INTF_META_COLOR_CORRECT_GAINS:
            return POINTER_OF_META(CAM_INTF_META_COLOR_CORRECT_GAINS, metadata);
        case CAM_INTF_META_PRED_COLOR_CORRECT_TRANSFORM:
            return POINTER_OF_META(CAM_INTF_META_PRED_COLOR_CORRECT_TRANSFORM, metadata);
        case CAM_INTF_META_PRED_COLOR_CORRECT_GAINS:
            return POINTER_OF_META(CAM_INTF_META_PRED_COLOR_CORRECT_GAINS, metadata);
        case CAM_INTF_META_AEC_PRECAPTURE_ID:
            return POINTER_OF_META(CAM_INTF_META_AEC_PRECAPTURE_ID, metadata);
        case CAM_INTF_META_AEC_ROI:
            return POINTER_OF_META(CAM_INTF_META_AEC_ROI, metadata);
        case CAM_INTF_META_AEC_STATE:
            return POINTER_OF_META(CAM_INTF_META_AEC_STATE, metadata);
        case CAM_INTF_PARM_FOCUS_MODE:
            return POINTER_OF_META(CAM_INTF_PARM_FOCUS_MODE, metadata);
        case CAM_INTF_META_AF_ROI:
            return POINTER_OF_META(CAM_INTF_META_AF_ROI, metadata);
        case CAM_INTF_META_AF_STATE:
            return POINTER_OF_META(CAM_INTF_META_AF_STATE, metadata);
        case CAM_INTF_META_AF_TRIGGER_ID:
            return POINTER_OF_META(CAM_INTF_META_AF_TRIGGER_ID, metadata);
        case CAM_INTF_PARM_WHITE_BALANCE:
            return POINTER_OF_META(CAM_INTF_PARM_WHITE_BALANCE, metadata);
        case CAM_INTF_META_AWB_REGIONS:
            return POINTER_OF_META(CAM_INTF_META_AWB_REGIONS, metadata);
        case CAM_INTF_META_AWB_STATE:
            return POINTER_OF_META(CAM_INTF_META_AWB_STATE, metadata);
        case CAM_INTF_META_BLACK_LEVEL_LOCK:
            return POINTER_OF_META(CAM_INTF_META_BLACK_LEVEL_LOCK, metadata);
        case CAM_INTF_META_MODE:
            return POINTER_OF_META(CAM_INTF_META_MODE, metadata);
        case CAM_INTF_META_EDGE_MODE:
            return POINTER_OF_META(CAM_INTF_META_EDGE_MODE, metadata);
        case CAM_INTF_META_FLASH_POWER:
            return POINTER_OF_META(CAM_INTF_META_FLASH_POWER, metadata);
        case CAM_INTF_META_FLASH_FIRING_TIME:
            return POINTER_OF_META(CAM_INTF_META_FLASH_FIRING_TIME, metadata);
        case CAM_INTF_META_FLASH_MODE:
            return POINTER_OF_META(CAM_INTF_META_FLASH_MODE, metadata);
        case CAM_INTF_META_FLASH_STATE:
            return POINTER_OF_META(CAM_INTF_META_FLASH_STATE, metadata);
        case CAM_INTF_META_HOTPIXEL_MODE:
            return POINTER_OF_META(CAM_INTF_META_HOTPIXEL_MODE, metadata);
        case CAM_INTF_META_LENS_APERTURE:
            return POINTER_OF_META(CAM_INTF_META_LENS_APERTURE, metadata);
        case CAM_INTF_META_LENS_FILTERDENSITY:
            return POINTER_OF_META(CAM_INTF_META_LENS_FILTERDENSITY, metadata);
        case CAM_INTF_META_LENS_FOCAL_LENGTH:
            return POINTER_OF_META(CAM_INTF_META_LENS_FOCAL_LENGTH, metadata);
        case CAM_INTF_META_LENS_FOCUS_DISTANCE:
            return POINTER_OF_META(CAM_INTF_META_LENS_FOCUS_DISTANCE, metadata);
        case CAM_INTF_META_LENS_FOCUS_RANGE:
            return POINTER_OF_META(CAM_INTF_META_LENS_FOCUS_RANGE, metadata);
        case CAM_INTF_META_LENS_STATE:
            return POINTER_OF_META(CAM_INTF_META_LENS_STATE, metadata);
        case CAM_INTF_META_LENS_OPT_STAB_MODE:
            return POINTER_OF_META(CAM_INTF_META_LENS_OPT_STAB_MODE, metadata);
        case CAM_INTF_META_NOISE_REDUCTION_MODE:
            return POINTER_OF_META(CAM_INTF_META_NOISE_REDUCTION_MODE, metadata);
        case CAM_INTF_META_SCALER_CROP_REGION:
            return POINTER_OF_META(CAM_INTF_META_SCALER_CROP_REGION, metadata);
        case CAM_INTF_META_SCENE_FLICKER:
            return POINTER_OF_META(CAM_INTF_META_SCENE_FLICKER, metadata);
        case CAM_INTF_META_SENSOR_EXPOSURE_TIME:
            return POINTER_OF_META(CAM_INTF_META_SENSOR_EXPOSURE_TIME, metadata);
        case CAM_INTF_META_SENSOR_FRAME_DURATION:
            return POINTER_OF_META(CAM_INTF_META_SENSOR_FRAME_DURATION, metadata);
        case CAM_INTF_META_SENSOR_SENSITIVITY:
            return POINTER_OF_META(CAM_INTF_META_SENSOR_SENSITIVITY, metadata);
        case CAM_INTF_META_SENSOR_TIMESTAMP:
            return POINTER_OF_META(CAM_INTF_META_SENSOR_TIMESTAMP, metadata);
        case CAM_INTF_META_SHADING_MODE:
            return POINTER_OF_META(CAM_INTF_META_SHADING_MODE, metadata);
        case CAM_INTF_META_STATS_FACEDETECT_MODE:
            return POINTER_OF_META(CAM_INTF_META_STATS_FACEDETECT_MODE, metadata);
        case CAM_INTF_META_STATS_HISTOGRAM_MODE:
            return POINTER_OF_META(CAM_INTF_META_STATS_HISTOGRAM_MODE, metadata);
        case CAM_INTF_META_STATS_SHARPNESS_MAP_MODE:
            return POINTER_OF_META(CAM_INTF_META_STATS_SHARPNESS_MAP_MODE, metadata);
        case CAM_INTF_META_STATS_SHARPNESS_MAP:
            return POINTER_OF_META(CAM_INTF_META_STATS_SHARPNESS_MAP, metadata);
        case CAM_INTF_META_LENS_SHADING_MAP:
            return POINTER_OF_META(CAM_INTF_META_LENS_SHADING_MAP, metadata);
        case CAM_INTF_META_AEC_INFO:
            return POINTER_OF_META(CAM_INTF_META_AEC_INFO, metadata);
        case CAM_INTF_META_SENSOR_INFO:
            return POINTER_OF_META(CAM_INTF_META_SENSOR_INFO, metadata);
        case CAM_INTF_META_ASD_SCENE_CAPTURE_TYPE:
            return POINTER_OF_META(CAM_INTF_META_ASD_SCENE_CAPTURE_TYPE, metadata);
        case CAM_INTF_META_PRIVATE_DATA:
            return POINTER_OF_META(CAM_INTF_META_PRIVATE_DATA, metadata);
        default:
            return NULL;
    }
}

