// @lint-ignore-every LICENSELINT
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include <memory.h>

#if PLATFORM_SUPPORTS_PRAGMA_PACK
#pragma pack(push, 8)
#endif

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#endif

#define ENGINE_TELEMETRY_API typedef
#include "EngineTelemetry.h"
#undef ENGINE_TELEMETRY_API

#if PLATFORM_WINDOWS
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#if PLATFORM_SUPPORTS_PRAGMA_PACK
#pragma pack(pop)
#endif

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#endif

#define ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(Func) engineTelemetry_##Func* Func

namespace OculusXR
{
	class OCULUSXRTELEMETRY_API EngineTelemetryPlugin
	{
	public:
		static EngineTelemetryPlugin& GetPlugin();
		static bool IsPluginEnabled();

		EngineTelemetryPlugin();
		~EngineTelemetryPlugin();

		bool InitializePlugin();
		void TeardownPlugin();

		void* LoadSharedLibrary() const;
		void* LoadEntryPoint(const char* EntryPointName) const;
		bool LoadTelemetryFunctions();

		bool IsInitialized() const { return Initialized; }

		// Function pointers
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(Init);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(ShutDown);

		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(SendEvent);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(SendUnifiedEvent);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(AddCustomMetadata);

		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(SetDeveloperTelemetryConsent);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(SaveUnifiedConsent);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(SaveUnifiedConsentWithOlderVersion);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(GetUnifiedConsent);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(GetConsentTitle);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(GetConsentMarkdownText);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(GetConsentNotificationMarkdownText);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(ShouldShowTelemetryConsentWindow);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(ShouldShowTelemetryNotification);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(SetNotificationShown);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(GetConsentSettingsChangeText);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(IsConsentSettingsChangeEnabled);

		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(QplMarkerStart);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(QplMarkerEnd);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(QplMarkerPoint);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(QplMarkerPointCached);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(QplMarkerAnnotation);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(QplCreateMarkerHandle);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(QplDestroyMarkerHandle);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(OnEditorShutdown);
		ENGINE_TELEMETRY_DECLARE_ENTRY_POINT(QplSetConsent);

	private:
		bool Initialized;
		void* EngineTelemetryLibrary;
	};

} // namespace OculusXR

#undef ENGINE_TELEMETRY_DECLARE_ENTRY_POINT
