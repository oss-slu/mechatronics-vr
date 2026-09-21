// @lint-ignore-every LICENSELINT
// Copyright Epic Games, Inc. All Rights Reserved.

#include "OculusXRTelemetryWrapper.h"
#include "OculusXRTelemetryModule.h"
#include <Misc/App.h>
#include <Misc/Paths.h>
#include <Interfaces/IPluginManager.h>

#if PLATFORM_ANDROID
#include <dlfcn.h>
#define MIN_SDK_VERSION 29
#endif

namespace OculusXR
{
	struct TelemetryEntryPoint
	{
		const char* EntryPointName;
		void** EntryPointPtr;
	};

	EngineTelemetryPlugin& EngineTelemetryPlugin::GetPlugin()
	{
		return FOculusXRTelemetryModule::Get().GetTelemetryPlugin();
	}

	bool EngineTelemetryPlugin::IsPluginEnabled()
	{
		return GetPlugin().IsInitialized();
	}

	EngineTelemetryPlugin::EngineTelemetryPlugin()
		: Initialized(false)
		, EngineTelemetryLibrary(nullptr)
	{
	}

	EngineTelemetryPlugin::~EngineTelemetryPlugin()
	{
		TeardownPlugin();
	}

	bool EngineTelemetryPlugin::InitializePlugin()
	{
		if (IsInitialized())
		{
			UE_LOG(LogOculusXRTelemetry, Warning, TEXT("Telemetry plugin already initialized"));
			return true;
		}

		EngineTelemetryLibrary = LoadSharedLibrary();
		if (EngineTelemetryLibrary == nullptr)
		{
			UE_LOG(LogOculusXRTelemetry, Warning, TEXT("Failed to load EngineTelemetry shared library"));
			return false;
		}

		Initialized = LoadTelemetryFunctions();
		if (IsInitialized())
		{
			UE_LOG(LogOculusXRTelemetry, Log, TEXT("EngineTelemetry initialized successfully"));
			Init();
		}
		else
		{
			TeardownPlugin();
			return false;
		}

		return true;
	}

	void EngineTelemetryPlugin::TeardownPlugin()
	{
		if (IsInitialized())
		{
			ShutDown();
		}
		Initialized = false;

		if (EngineTelemetryLibrary)
		{
			FPlatformProcess::FreeDllHandle(EngineTelemetryLibrary);
			EngineTelemetryLibrary = nullptr;
		}

		FMemory::Memset(this, 0, sizeof(EngineTelemetryPlugin));

		UE_LOG(LogOculusXRTelemetry, Log, TEXT("Engine Telemetry plugin destroyed successfully"));
	}

	void* EngineTelemetryPlugin::LoadSharedLibrary() const
	{
		void* LibraryPtr = nullptr;

#if PLATFORM_ANDROID
		const bool VersionValid = FAndroidMisc::GetAndroidBuildVersion() >= MIN_SDK_VERSION;
#else
		const bool VersionValid = true;
#endif

		if (VersionValid)
		{
#if PLATFORM_WINDOWS
			FString BinariesPath = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("OculusXR"))->GetBaseDir(), TEXT("/Source/ThirdParty/EngineTelemetry/Lib/Win64"));
			FPlatformProcess::PushDllDirectory(*BinariesPath);
			LibraryPtr = FPlatformProcess::GetDllHandle(*(BinariesPath / "EngineTelemetry.dll"));
			FPlatformProcess::PopDllDirectory(*BinariesPath);
#elif PLATFORM_ANDROID
			LibraryPtr = FPlatformProcess::GetDllHandle(TEXT("libEngineTelemetry.so"));
#endif // PLATFORM_ANDROID
		}

		return LibraryPtr;
	}

	void* EngineTelemetryPlugin::LoadEntryPoint(const char* EntryPointName) const
	{
		if (EngineTelemetryLibrary == nullptr)
		{
			return nullptr;
		}

#if PLATFORM_WINDOWS
		void* ptr = GetProcAddress((HMODULE)EngineTelemetryLibrary, EntryPointName);
		if (ptr == nullptr)
		{
			UE_LOG(LogOculusXRTelemetry, Error, TEXT("Unable to load entry point: %s"), ANSI_TO_TCHAR(EntryPointName));
		}
		return ptr;
#elif PLATFORM_ANDROID
		void* ptr = dlsym(EngineTelemetryLibrary, EntryPointName);
		if (ptr == nullptr)
		{
			UE_LOG(LogOculusXRTelemetry, Error, TEXT("Unable to load entry point: %s, error %s"), ANSI_TO_TCHAR(EntryPointName), ANSI_TO_TCHAR(dlerror()));
		}
		return ptr;
#else
		UE_LOG(LogOculusXRTelemetry, Error, TEXT("LoadEntryPoint: Unsupported platform"));
		return nullptr;
#endif
	}

	bool EngineTelemetryPlugin::LoadTelemetryFunctions()
	{
#define ENGINE_TELEMETRY_BIND_ENTRY_POINT(Func) \
	{                                           \
		"engineTelemetry_" #Func, (void**)&Func \
	}

		TelemetryEntryPoint entryPointArray[] = {
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(Init),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(ShutDown),

			ENGINE_TELEMETRY_BIND_ENTRY_POINT(SendEvent),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(SendUnifiedEvent),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(AddCustomMetadata),

			ENGINE_TELEMETRY_BIND_ENTRY_POINT(SetDeveloperTelemetryConsent),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(SaveUnifiedConsent),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(SaveUnifiedConsentWithOlderVersion),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(GetUnifiedConsent),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(GetConsentTitle),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(GetConsentMarkdownText),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(GetConsentNotificationMarkdownText),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(ShouldShowTelemetryConsentWindow),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(ShouldShowTelemetryNotification),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(SetNotificationShown),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(GetConsentSettingsChangeText),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(IsConsentSettingsChangeEnabled),

			ENGINE_TELEMETRY_BIND_ENTRY_POINT(QplMarkerStart),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(QplMarkerEnd),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(QplMarkerPoint),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(QplMarkerPointCached),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(QplMarkerAnnotation),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(QplCreateMarkerHandle),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(QplDestroyMarkerHandle),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(OnEditorShutdown),
			ENGINE_TELEMETRY_BIND_ENTRY_POINT(QplSetConsent)
		};
#undef ENGINE_TELEMETRY_BIND_ENTRY_POINT

		UE_LOG(LogOculusXRTelemetry, Log, TEXT("EngineTelemetry -- Binding Functions"));

		bool result = true;
		for (int i = 0; i < UE_ARRAY_COUNT(entryPointArray); ++i)
		{
			*(entryPointArray[i].EntryPointPtr) = LoadEntryPoint(entryPointArray[i].EntryPointName);

			if (*entryPointArray[i].EntryPointPtr == nullptr)
			{
				UE_LOG(LogOculusXRTelemetry, Error, TEXT(" %s [FAILURE]"), ANSI_TO_TCHAR(entryPointArray[i].EntryPointName));
				result = false;
			}
			else
			{
				UE_LOG(LogOculusXRTelemetry, Log, TEXT(" %s [SUCCESS]"), ANSI_TO_TCHAR(entryPointArray[i].EntryPointName));
			}
		}

		return result;
	}
} // namespace OculusXR
