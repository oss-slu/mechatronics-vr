// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "OculusXRTelemetryModule.h"
#include "OculusXRTelemetryShared.h"
#include <Misc/App.h>
#include <UnrealEngine.h>
#include <IXRTrackingSystem.h>

#define LOCTEXT_NAMESPACE "OculusXRTelemetry"

DEFINE_LOG_CATEGORY(LogOculusXRTelemetry);

//-------------------------------------------------------------------------------------------------
// FOculusXRTelemetryModule
//-------------------------------------------------------------------------------------------------

OculusXR::EngineTelemetryPlugin FOculusXRTelemetryModule::TelemetryPlugin{};

FOculusXRTelemetryModule::FOculusXRTelemetryModule()
{
}

void FOculusXRTelemetryModule::StartupModule()
{
#if OCULUS_HMD_SUPPORTED_PLATFORMS
	// Init module if app can render
	if (FApp::CanEverRender())
	{
		UE_LOG(LogOculusXRTelemetry, Log, TEXT("Starting Meta XR Telemetry Module."));

		if (!TelemetryPlugin.InitializePlugin())
		{
			UE_LOG(LogHMD, Log, TEXT("Failed to initialize Engine Telemetry plugin"));
		}
	}
#endif // OCULUS_HMD_SUPPORTED_PLATFORMS
}

void FOculusXRTelemetryModule::ShutdownModule()
{
#if OCULUS_HMD_SUPPORTED_PLATFORMS
	if (TelemetryPlugin.IsInitialized())
	{
		OculusXRTelemetry::FTelemetryBackend::OnEditorShutdown();
		TelemetryPlugin.TeardownPlugin();
	}
#endif // OCULUS_HMD_SUPPORTED_PLATFORMS
}

#if OCULUS_HMD_SUPPORTED_PLATFORMS
OculusXR::EngineTelemetryPlugin& FOculusXRTelemetryModule::GetTelemetryPlugin()
{
	return TelemetryPlugin;
}
#endif // OCULUS_HMD_SUPPORTED_PLATFORMS

IMPLEMENT_MODULE(FOculusXRTelemetryModule, OculusXRTelemetry)

#undef LOCTEXT_NAMESPACE
