// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "MRUtilityKitOpenXrExtensionPlugin.h"

#include "IOpenXRHMDModule.h"
#include "OpenXRCore.h"

void FMRUKOpenXrExtensionPlugin::RegisterAsOpenXRExtension()
{
#if defined(WITH_OCULUS_BRANCH)
	// Feature not enabled on Marketplace build. Currently only for the meta fork
	RegisterOpenXRExtensionModularFeature();
#endif
}

bool FMRUKOpenXrExtensionPlugin::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add("XR_FB_spatial_entity");
	OutExtensions.Add("XR_FB_spatial_entity_query");
	OutExtensions.Add("XR_FB_spatial_entity_storage");
	OutExtensions.Add("XR_FB_scene");
	OutExtensions.Add("XR_FB_spatial_entity_container");
	OutExtensions.Add("XR_FB_scene_capture");
	OutExtensions.Add("XR_META_spatial_entity_discovery");
	return true;
}

bool FMRUKOpenXrExtensionPlugin::GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	return true;
}

void FMRUKOpenXrExtensionPlugin::OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader)
{
	if (OpenXrEventHandler)
	{
		OpenXrEventHandler((void*)InHeader, Context);
	}
}
