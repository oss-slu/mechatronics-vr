// @lint-ignore-every LICENSELINT
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if PLATFORM_WINDOWS

#include "Widgets/Notifications/SNotificationList.h"

DEFINE_LOG_CATEGORY_STATIC(LogMetaXRSim, Log, All);

/**  */
class FMetaXRSimulator
{
public:
	static FMetaXRSimulator& Get()
	{
		static FMetaXRSimulator instance;
		return instance;
	}
	FMetaXRSimulator(const FMetaXRSimulator&) = delete;
	FMetaXRSimulator& operator=(const FMetaXRSimulator&) = delete;

	bool IsSimulatorActivated();
	void ToggleOpenXRRuntime();
	bool IsSimulatorInstalled() const;

private:
	static void SpawnNotificationToInstallXRSim();
	FMetaXRSimulator();
	~FMetaXRSimulator() = default;
	FString GetSimulatorJsonPath() const;
};
#endif
