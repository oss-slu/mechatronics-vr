// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "OculusXRSurveySettings.generated.h"

UCLASS(MinimalAPI, hidecategories = Object, config = EditorSettings)
class UOculusXRSurveySettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	// First launch date stored as string
	UPROPERTY(config)
	FString FirstLaunchDate;

	// Number of times the editor has been launched
	UPROPERTY(config)
	int32 LaunchCount = 0;

	// Whether the survey has been shown to the user
	UPROPERTY(config)
	bool bSurveyShown = false;

public:
	// Check if enough time and launches have passed to show the survey
	bool ShouldShowSurvey() const;

	// Mark that the survey has been shown
	void MarkSurveyShown();

	// Record a new editor launch
	void RecordLaunch();
};
