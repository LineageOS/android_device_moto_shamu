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

#define LOG_NDEBUG 0
#define LOG_NIDEBUG 0
#define LOG_TAG "qomx_image_core"
#include <utils/Log.h>

#include "qomx_core.h"

#define BUFF_SIZE 255

omx_core *gomx_core_components;

//Map the library name with the component name
component_libname_mapping libNameMapping[] =
{
  { "OMX.qcom.image.jpeg.encoder", "libqomx_jpegenc.so"},
};

static int getIndexFromHandle(OMX_IN OMX_HANDLETYPE *ahComp, int *acompIndex,
  int *ainstanceIndex);

/*==============================================================================
* Function : OMX_Init
* Parameters: None
* Description: This is the first call that is made to the OMX Core
* and initializes the OMX IL core
==============================================================================*/
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_Init()
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (gomx_core_components) {
    if (gomx_core_components->is_initialized) {
      return lrc;
    }
  }
  gomx_core_components = malloc(sizeof(omx_core));
  if (gomx_core_components) {
    gomx_core_components->is_initialized = TRUE;
    pthread_mutex_init(&gomx_core_components->core_lock, NULL);
  } else {
    lrc = OMX_ErrorInsufficientResources;
  }
  return lrc;
}
/*==============================================================================
* Function : OMX_Deinit
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Deinit all the OMX components
==============================================================================*/
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_Deinit()
{
  if (gomx_core_components) {
    gomx_core_components->is_initialized = FALSE;
    pthread_mutex_destroy(&gomx_core_components->core_lock);
    free(gomx_core_components);
    gomx_core_components = NULL;
  }
  ALOGE("%s %d: Complete", __func__, __LINE__);
  return OMX_ErrorNone;
}

/*==============================================================================
* Function : getComponentFromList
* Parameters: componentName
* Return Value : component_index
* Description: If the componnt is already present in the list, return the
* component index. If not return the next index to create the component.
==============================================================================*/
static int getComponentFromList(char *acomponentName){
  int lindex = -1, i =0;

  if (acomponentName) {
    for (i=0;i < OMX_COMP_MAX_NUM; i++) {
      if (gomx_core_components->component[i]){
        if (!strcmp(gomx_core_components->component[i]->name,acomponentName)) {
          lindex = i;
          return lindex;
        }
        i++;
      }
      else {
        return i;
      }
    }
  }
  return lindex;
}

/*==============================================================================
* Function : getLibName
* Parameters: componentName, libName
* Return Value : library name associated with the component name
* Description: Get the library name associated with the component name from the
* component name - library name mapping.
*============================================================================*/
static uint8_t getLibName(char *acomponentName, char *alibName)
{
  uint8_t cnt;

  for (cnt=0; cnt < (sizeof(libNameMapping)/sizeof(libNameMapping[0])); cnt++) {
    if (!strcmp(libNameMapping[cnt].componentName, acomponentName)) {
      strlcpy(alibName, libNameMapping[cnt].libName, BUFF_SIZE);
      break;
    }
  }
  return 1;
}
/*==============================================================================
* Function : getLibName
* Parameters: corecomponent
* Return Value : The next instance index if available
* Description: Get the next available index for to store the new instance of the
* component being created.
*============================================================================*/
static int getInstanceIndex(omx_core_component *acomponent)
{
  int linstance_index = -1;
  int i = 0;

  for (i=0; i<OMX_COMP_MAX_INSTANCES; i++) {
    if (!acomponent->handle[i]) {
      linstance_index = i;
    }
  }
  return linstance_index;
}
/*==============================================================================
* Function : loadComponent
* Parameters: corecomponent
* Return Value : The next instance index if available
* Description: Get the next available index for to store the new instance of the
* component being created.
*============================================================================*/
static void * loadComponent(char *alibName)
{
  void *lhandle = NULL;

  lhandle = dlopen(alibName, RTLD_NOW);
  return lhandle;
}
/*==============================================================================
* Function : OMX_GetHandle
* Parameters: handle, componentName, appData, callbacks
* Return Value : OMX_ERRORTYPE
* Description: Construct and load the requested omx library
==============================================================================*/
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_GetHandle(
  OMX_OUT OMX_HANDLETYPE* handle,
  OMX_IN OMX_STRING componentName,
  OMX_IN OMX_PTR appData,
  OMX_IN OMX_CALLBACKTYPE* callBacks)
{
  int lcomp_index, linstance_index = 0;
  omx_core_component *core_comp = NULL;
  char libName[BUFF_SIZE] = {0};
  void *lhandle, *lfnaddress, *lobj_ptr, *lhandle_ptr;
  OMX_COMPONENTTYPE *lcomp;

  getLibName(componentName, libName);
  if (libName == NULL) {
     return OMX_ErrorInvalidComponent;
  }

  pthread_mutex_lock(&gomx_core_components->core_lock);
  if (handle) {
    *handle = NULL;
    //Get the component index from the list
    lcomp_index = getComponentFromList(componentName);
    if (lcomp_index >= 0) {
      //If component already present get the instance index
      if (gomx_core_components->component[lcomp_index]) {
        linstance_index =
          getInstanceIndex(gomx_core_components->component[lcomp_index]);
      }
        if (linstance_index >= 0) {
          //Open the library dynamically if its not already open
          if (!gomx_core_components->component[lcomp_index]) {
             core_comp = malloc(sizeof(omx_core_component));
             core_comp->name = componentName;
             core_comp->lib_handle = loadComponent(libName);
             if (core_comp->lib_handle) {
              core_comp->open = TRUE;
              //Init the component and get component functions
              core_comp->comp_func_ptr = dlsym(core_comp->lib_handle,
                "create_component_fns");
              //Get Instance of the component
              core_comp->obj_ptr = dlsym(core_comp->lib_handle, "getInstance");
              } else{
                ALOGE("%s : Error: Cannot Create Component",__func__);
                pthread_mutex_unlock(&gomx_core_components->core_lock);
                return OMX_ErrorNotImplemented;
              }
            }
            if (core_comp->obj_ptr) {
              //Call the function from the address to create the obj
              lobj_ptr = (*core_comp->obj_ptr)();
              ALOGE("%s: get intsance pts is %p", __func__, lobj_ptr);
              //Get function mapping for the component
              if (core_comp->comp_func_ptr) {
                //Call the function from the address to get the func ptrs
                lcomp = (*core_comp->comp_func_ptr)(lobj_ptr);
                *handle = core_comp->handle[linstance_index] =
                   (OMX_HANDLETYPE)lcomp;
                gomx_core_components->component[lcomp_index] = core_comp;

                ALOGD("%s %d: handle = %p Instanceindex = %d,"
                  "coreComp = %p lcomp_index %d g_ptr %p", __func__,__LINE__,
                  core_comp->handle[linstance_index], linstance_index,
                  gomx_core_components->component[lcomp_index],
                  lcomp_index, gomx_core_components);

                lcomp->SetCallbacks(lcomp, callBacks, appData);
              } else {
                ALOGE("%s:L#%d: Failed to find symbol dlerror=%s\n",
                  __func__, __LINE__, dlerror());
                free(core_comp);
                pthread_mutex_unlock(&gomx_core_components->core_lock);
                return OMX_ErrorInvalidComponent;
              }
            } else {
               ALOGE("%s:L#%d: Failed to find symbol dlerror=%s\n",
                __func__, __LINE__, dlerror());
                free(core_comp);
                pthread_mutex_unlock(&gomx_core_components->core_lock);
                return OMX_ErrorInvalidComponent;
            }
         } else {
             ALOGE("%s : Error: Exceeded max number of components",
               __func__);
             pthread_mutex_unlock(&gomx_core_components->core_lock);
             return OMX_ErrorNotImplemented;
          }
        } else {
           ALOGE("%s : Error: Component Not found",__func__);
           pthread_mutex_unlock(&gomx_core_components->core_lock);
           return OMX_ErrorNotImplemented;
        }
  } else {
    ALOGE("\n OMX_GetHandle: NULL handle \n");
    pthread_mutex_unlock(&gomx_core_components->core_lock);
    return OMX_ErrorBadParameter;;
  }
  pthread_mutex_unlock(&gomx_core_components->core_lock);
  ALOGD("%s:%d] Success", __func__, __LINE__);
  return OMX_ErrorNone;
}

/*==============================================================================
* Function : getIndexFromComponent
* Parameters: handle,
* Return Value : Component present - true or false, Instance Index, Component
* Index
* Description: Check if the handle is present in the list and get the component
* index and instance index for the component handle.
==============================================================================*/
static int getIndexFromHandle(OMX_IN OMX_HANDLETYPE *ahComp, int *acompIndex,
  int *ainstanceIndex)
{
  int lcompIndex = -1, lindex = -1;
  uint8_t lisPresent = FALSE;
  for (lcompIndex = 0; lcompIndex < OMX_COMP_MAX_NUM; lcompIndex++) {
    for (lindex = 0; lindex < OMX_COMP_MAX_INSTANCES; lindex++) {
      if (gomx_core_components->component[lcompIndex]) {
        if ((OMX_COMPONENTTYPE *)gomx_core_components->
          component[lcompIndex]->handle[lindex] ==
          (OMX_COMPONENTTYPE *)ahComp ) {
          lisPresent = TRUE;
          *acompIndex = lcompIndex;
          *ainstanceIndex = lindex;
        }
      }
    }
  }
  return lisPresent;
}

/*==============================================================================
* Function : isCompActive
* Parameters: aComp
* Return Value : int
* Description: Check if the component has any active instances
==============================================================================*/
static uint8_t isCompActive(omx_core_component *acomp)
{
  uint8_t lindex = 0;
  uint8_t lret = FALSE;
  for (lindex = 0; lindex < OMX_COMP_MAX_INSTANCES; lindex++) {
    if (acomp->handle[lindex] != NULL) {
      return TRUE;
    }
  }
  return lret;
}

/*==============================================================================
* Function : OMX_FreeHandle
* Parameters: hComp
* Return Value : OMX_ERRORTYPE
* Description: Deinit the omx component and remove it from the global list
==============================================================================*/
OMX_API OMX_ERRORTYPE OMX_APIENTRY OMX_FreeHandle(
  OMX_IN OMX_HANDLETYPE hComp)
{
  int lret = OMX_ErrorNone, lrc;
  int lcomponentIndex, lInstanceIndex;
  OMX_COMPONENTTYPE *lcomp;

  ALOGE("%s %d", __func__, __LINE__);
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }

  lcomp = (OMX_COMPONENTTYPE *)hComp;

  if (getIndexFromHandle(hComp, &lcomponentIndex, &lInstanceIndex)) {
    pthread_mutex_lock(&gomx_core_components->core_lock);
    //Deinit the component;
    lret = lcomp->ComponentDeInit(hComp);
    if (lret == OMX_ErrorNone) {
      //Remove the handle from the comp structure
      gomx_core_components->component[lcomponentIndex]->
        handle[lInstanceIndex] = NULL;
      if (!isCompActive(gomx_core_components->component[lcomponentIndex])) {
        lrc = dlclose(gomx_core_components->component[lcomponentIndex]->
          lib_handle);
        gomx_core_components->component[lcomponentIndex]->lib_handle = NULL;
        gomx_core_components->component[lcomponentIndex]->obj_ptr = NULL;
        gomx_core_components->component[lcomponentIndex]->comp_func_ptr = NULL;
        gomx_core_components->component[lcomponentIndex]->open = FALSE;
        free(gomx_core_components->component[lcomponentIndex]);
        gomx_core_components->component[lcomponentIndex] = NULL;
        if (lrc) {
          ALOGE("\n Failed to unload the cmponent");
        }
      } else {
        ALOGE("%s %d:Component is still Active", __func__, __LINE__);
      }
    } else {
      ALOGE("%s : Failed to release the cmponent", __func__);
    }
    pthread_mutex_unlock(&gomx_core_components->core_lock);
  } else {
    ALOGE("\n Component Instance not found \n");
    return OMX_ErrorBadParameter;
  }
  return lret;
}
