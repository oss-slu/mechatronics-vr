// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "OculusXROSVersions.generated.h"

// Struct that wraps the version number, this allows for better customization of the appearance in the editor.
USTRUCT()
struct OCULUSXRHMD_API FOculusXROSVersion
{
	GENERATED_BODY()
public:
	FOculusXROSVersion();

	int32 GetVersion() const;

	UPROPERTY(EditAnywhere, Category = "OculusXR")
	int32 Version;

	UPROPERTY(EditAnywhere, Category = "OculusXR")
	bool bLatest;
};

// Helper class for getting version information
struct OCULUSXRHMD_API FOculusXROSVersionHelpers
{
public:
	static const int32 MinimumOSVersion = 60;

	static FString GetVersionStringFromPlugin();
	static int32 GetOSVersion();
};
