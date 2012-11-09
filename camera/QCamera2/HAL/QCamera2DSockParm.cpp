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
 */

#define LOG_NIDEBUG 0
#define LOG_TAG "QCamera2DSockParm"

#include <stdlib.h>
#include <utils/Log.h>
#include <utils/Errors.h>
#include <hardware/camera.h>

#include "QCamera2DSockParm.h"

namespace android {

int initGroupUpdateTable(transactionType tableType,void *p_table)
{
    set_parm_buffer_t *p_setTable=NULL;
    get_parm_buffer_t *p_getTable=NULL;

    switch(tableType){
        case MM_CAMERA_SET_PARAMETERS:
            p_setTable=(set_parm_buffer_t *)p_table;
            memset(p_setTable,0,sizeof(set_parm_buffer_t));
            p_setTable->first_flagged_entry=CAM_INTF_PARM_MAX;
            break;
        case MM_CAMERA_GET_PARAMETERS:
            p_getTable=(get_parm_buffer_t *)p_table;
            memset(p_getTable,0,sizeof(get_parm_buffer_t));
            p_getTable->first_flagged_entry=CAM_INTF_PARM_MAX;
            break;
        default:
            ALOGE("%s:Invalid Transation type",__func__);
            return -1;
            break;
    }
    return 0;
}

int setParmEntry(set_parm_buffer_t *p_table,
        cam_intf_parm_type_t paramType,
        uint32_t paramLength,
        void *paramValue)
{
    int position=paramType;
    int current,next;

    /*************************************************************************
    *                 Code to take care of linking next flags                *
    *************************************************************************/
    current=GET_FIRST_PARAM_ID(p_table);
    if(position==current){
        //DO NOTHING
    } else if(position<current){
        SET_NEXT_PARAM_ID(position,p_table,current);
        SET_FIRST_PARAM_ID(p_table,position);
    } else {
        /* Search for the position in the linked list where we need to slot in*/
        while(position>GET_NEXT_PARAM_ID(current,p_table))
            current=GET_NEXT_PARAM_ID(current,p_table);

        /*If node already exists no need to alter linking*/
        if(position!=GET_NEXT_PARAM_ID(current,p_table)) {
            next=GET_NEXT_PARAM_ID(current,p_table);
            SET_NEXT_PARAM_ID(current,p_table,position);
            SET_NEXT_PARAM_ID(position,p_table,next);
        }
    }

    /*************************************************************************
    *                   Copy contents into entry                             *
    *************************************************************************/

    if(paramLength>sizeof(set_parm_type_t)) {
        ALOGE("%s:Size of input larger than max entry size",__func__);
        return -1;
    }
    memcpy(POINTER_OF(paramType,p_table),paramValue,paramLength);
    return 0;
}

int setParmFlush(set_parm_buffer_t * /*p_table*/)
{
    //Send SET_PARM_V4L2_IOCTL(GroupUpdate);
    return 0;
}

int getParm(get_parm_buffer_t *p_table,
        cam_intf_parm_type_t paramType)
{
    int position=paramType;
    int current,next;

    /*************************************************************************
    *                 Code to take care of linking next flags                *
    *************************************************************************/
    current=GET_FIRST_PARAM_ID(p_table);
    if(position==current){
        //DO NOTHING
    } else if(position<current){
        SET_NEXT_PARAM_ID(position,p_table,current);
        SET_FIRST_PARAM_ID(p_table,position);
    } else {
        /* Search for the position in the linked list where we need to slot in*/
        while(position>GET_NEXT_PARAM_ID(current,p_table))
            current=GET_NEXT_PARAM_ID(current,p_table);

        /*If node already exists no need to alter linking*/
        if(position!=GET_NEXT_PARAM_ID(current,p_table)) {
            next=GET_NEXT_PARAM_ID(current,p_table);
            SET_NEXT_PARAM_ID(current,p_table,position);
            SET_NEXT_PARAM_ID(position,p_table,next);
        }
    }

    //Send GET_PARM_V4L2_IOCTL(paramType);
    return 0;
}

}; /*namespace android*/
