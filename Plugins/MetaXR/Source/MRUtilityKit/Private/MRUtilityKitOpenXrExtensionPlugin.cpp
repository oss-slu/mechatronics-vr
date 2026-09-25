// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "MRUtilityKitOpenXrExtensionPlugin.h"

#include "IOpenXRHMDModule.h"
#include "MRUtilityKitSubsystem.h"
#include "OpenXRCore.h"

void FMRUKOpenXrExtensionPlugin::RegisterAsOpenXRExtension()
{
	RegisterOpenXRExtensionModularFeature();
}

bool FMRUKOpenXrExtensionPlugin::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	return true;
}

bool FMRUKOpenXrExtensionPlugin::GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add("XR_FB_spatial_entity");
	OutExtensions.Add("XR_FB_spatial_entity_query");
	OutExtensions.Add("XR_FB_spatial_entity_storage");
	OutExtensions.Add("XR_FB_scene");
	OutExtensions.Add("XR_FB_spatial_entity_container");
	OutExtensions.Add("XR_FB_scene_capture");
	OutExtensions.Add("XR_META_spatial_entity_discovery");
	OutExtensions.Add("XR_META_spatial_entity_room_mesh");
	OutExtensions.Add("XR_META_environment_raycast");
	OutExtensions.Add("XR_EXT_future");				   // Required by XR_META_environment_raycast
	OutExtensions.Add("XR_KHR_convert_timespec_time"); // Required by XR_META_environment_raycast
	OutExtensions.Add("XR_META_dynamic_object_tracker");
	OutExtensions.Add("XR_META_dynamic_object_keyboard");
	OutExtensions.Add("XR_EXT_spatial_marker_tracking");
	OutExtensions.Add("XR_EXT_spatial_entity");
	OutExtensions.Add("XR_METAX1_passthrough_camera_data");
	return true;
}

void FMRUKOpenXrExtensionPlugin::OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader)
{
	if (OpenXrEventHandler)
	{
		OpenXrEventHandler(const_cast<XrEventDataBaseHeader*>(InHeader), Context);
	}
}

void FMRUKOpenXrExtensionPlugin::PostCreateSession(XrSession InSession)
{
	if (MrukSubsystem)
	{
		MrukSubsystem->InitializeOpenXR();
	}
}
