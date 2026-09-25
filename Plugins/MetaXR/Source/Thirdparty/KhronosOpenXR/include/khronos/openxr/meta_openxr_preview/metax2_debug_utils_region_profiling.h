#ifndef METAX2_DEBUG_UTILS_REGION_PROFILING_H_
#define METAX2_DEBUG_UTILS_REGION_PROFILING_H_ 1

/**********************
This file is @generated from the OpenXR XML API registry.
Language    :   C99
Copyright   :   (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
***********************/

#include <openxr/openxr.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef XR_METAX2_debug_utils_region_profiling

// XR_METAX2_debug_utils_region_profiling is a preprocessor guard. Do not pass it to API calls.
#define XR_METAX2_debug_utils_region_profiling 1
#define XR_METAX2_debug_utils_region_profiling_SPEC_VERSION 1
#define XR_METAX2_DEBUG_UTILS_REGION_PROFILING_EXTENSION_NAME "XR_METAX2_debug_utils_region_profiling"
static const XrStructureType XR_TYPE_DEBUG_UTILS_REGION_PROFILING_BEGIN_INFO_METAX2 = (XrStructureType) 1000557001;
static const XrStructureType XR_TYPE_DEBUG_UTILS_REGION_PROFILING_INFO_METAX2 = (XrStructureType) 1000557002;

typedef enum XrRegionWorkloadTypeMETAX2 {
    // Region does not describe a specific workload type
    XR_REGION_WORKLOAD_TYPE_UNSPECIFIED_METAX2 = 0,
    // Region contains initialization of XR application functionality such as asset loading
    XR_REGION_WORKLOAD_TYPE_SCENE_LOADING_METAX2 = 1,
    // Region contains execution of meaningful XR application functionality such as scene rendering
    XR_REGION_WORKLOAD_TYPE_SCENE_EXECUTING_METAX2 = 2,
    XR_REGION_WORKLOAD_TYPE_METAX2_MAX_ENUM = 0x7FFFFFFF
} XrRegionWorkloadTypeMETAX2;
// XrDebugUtilsRegionProfilingInfoMETAX2 extends XrDebugUtilsLabelEXT
typedef struct XrDebugUtilsRegionProfilingInfoMETAX2 {
    XrStructureType               type;
    const void* XR_MAY_ALIAS      next;
    XrRegionWorkloadTypeMETAX2    workloadType;
} XrDebugUtilsRegionProfilingInfoMETAX2;

#endif /* XR_METAX2_debug_utils_region_profiling */

#ifdef __cplusplus
}
#endif

#endif
