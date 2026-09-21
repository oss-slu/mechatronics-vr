// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "Generated/MRUtilityKitShared.h"
#include "MRUtilityKit.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"

MRUKShared* MRUKShared::Instance = nullptr;

MRUKShared::MRUKShared(void* Handle)
	: MRUKSharedHandle(Handle)
{
	LoadNativeFunctions();
}

MRUKShared::~MRUKShared()
{
	UnloadNativeFunctions();

	FPlatformProcess::FreeDllHandle(MRUKSharedHandle);
	MRUKSharedHandle = nullptr;
}

void MRUKShared::LoadMRUKSharedLibrary()
{
	if (Instance)
	{
		return;
	}

	// Load the MR Utility Kit Shared library
	UE_LOG(LogMRUK, Log, TEXT("Loading MR Utility Kit Shared library"));
#if PLATFORM_WINDOWS
	const FString BinariesPath = FPaths::Combine(
		IPluginManager::Get().FindPlugin(TEXT("OculusXR"))->GetBaseDir(),
		TEXT("/Source/Thirdparty/MRUtilityKitShared/Lib/Win64"));
	FPlatformProcess::PushDllDirectory(*BinariesPath);
	void* Handle = FPlatformProcess::GetDllHandle(TEXT("mrutilitykitshared.dll"));
	FPlatformProcess::PopDllDirectory(*BinariesPath);
#elif PLATFORM_MAC
#if PLATFORM_CPU_ARM_FAMILY
	const FString BinariesPath = FPaths::Combine(
		IPluginManager::Get().FindPlugin(TEXT("OculusXR"))->GetBaseDir(),
		TEXT("/Source/Thirdparty/MRUtilityKitShared/Lib/MacArm"));
#else
	const FString BinariesPath = FPaths::Combine(
		IPluginManager::Get().FindPlugin(TEXT("OculusXR"))->GetBaseDir(),
		TEXT("/Source/Thirdparty/MRUtilityKitShared/Lib/Mac"));
#endif
	FPlatformProcess::PushDllDirectory(*BinariesPath);
	void* Handle = FPlatformProcess::GetDllHandle(TEXT("libmrutilitykitshared.dylib"));
	FPlatformProcess::PopDllDirectory(*BinariesPath);
#elif PLATFORM_ANDROID
	void* Handle = FPlatformProcess::GetDllHandle(TEXT("libmrutilitykitshared.so"));
#else
	void* Handle = nullptr;
#endif

	if (!Handle)
	{
		UE_LOG(LogMRUK, Error, TEXT("Failed to load MR Utility Kit Shared library"));
		return;
	}

	Instance = new MRUKShared(Handle);
}

void MRUKShared::FreeMRUKSharedLibrary()
{
	if (!Instance)
	{
		return;
	}
	delete Instance;
	Instance = nullptr;
}

void* MRUKShared::LoadFunction(const TCHAR* ProcName)
{
	void* Function = FPlatformProcess::GetDllExport(MRUKSharedHandle, ProcName);
	if (!Function)
	{
		UE_LOG(LogMRUK, Error, TEXT("Failed to load native function: %s"), ProcName);
	}
	return Function;
}
