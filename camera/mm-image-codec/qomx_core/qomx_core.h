/*Copyright (c) 2012, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
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
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

#ifndef QOMX_CORE_H
#define QOMX_CORE_H

#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <pthread.h>
#include <dlfcn.h>
#include <stdlib.h>
#include "OMX_Component.h"

#define TRUE 1
#define FALSE 0
#define OMX_COMP_MAX_INSTANCES 3
#define OMX_CORE_MAX_ROLES 1
#define OMX_COMP_MAX_NUM 1
#define OMX_SPEC_VERSION 0x00000101

typedef void * (*get_Instance)(void);
typedef void * (*create_component_functions)(OMX_PTR aobj);


/** _omx_core_component: OMX Component structure
*    @handle: array of number of instances of the component
*    @roles: array of roles played by the component
*    @name: Name of the component
*    @open: Is the component active
*    @lib_handle: Library handle after dlopen
*    @obj_ptr: Function ptr to get the instance of the component
*    @comp_func_ptr: Function ptr to map the functions in the
*     OMX handle to its respective function implementation in
*     the component
**/
typedef struct _omx_core_component
{
  OMX_HANDLETYPE *handle[OMX_COMP_MAX_INSTANCES];  //Instance handle
  char *roles[OMX_CORE_MAX_ROLES];  //Roles played by the component
  char *name;  //Component Name
  uint8_t open;  //Is component active
  void *lib_handle;
  get_Instance obj_ptr;
  create_component_functions comp_func_ptr;
}omx_core_component;


/** _omx_core: Global structure that contains all the active
*   components
*    @component: array of active components
*    @is_initialized: Flag to check if the OMX core has been
*    initialized
*    @core_lock: Lock to syncronize the omx core operations
**/
typedef struct _omx_core
{
  omx_core_component *component[OMX_COMP_MAX_NUM];  //Array of pointers to components
  uint8_t is_initialized;  //Is the component initiaziled
  pthread_mutex_t core_lock;
}omx_core;


/** _component_libname_mapping: Structure containing the mapping
*    between the library name and the corresponding .so name
*    @componenName: name of the component
     @libName: Name of the .so library
**/
typedef struct _component_libname_mapping{
  char * componentName;
  char * libName;
}component_libname_mapping;

#endif
