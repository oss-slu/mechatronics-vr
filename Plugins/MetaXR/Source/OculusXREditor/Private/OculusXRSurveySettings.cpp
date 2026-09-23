// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "OculusXRSurveySettings.h"

#define LOCTEXT_NAMESPACE "OculusXRSurveySettings"

UOculusXRSurveySettings::UOculusXRSurveySettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UOculusXRSurveySettings::ShouldShowSurvey() const
{
	// Don't show if already shown
	if (bSurveyShown)
	{
		return false;
	}

	// Need at least 4 launches
	if (LaunchCount < 4)
	{
		return false;
	}

	// Check if at least 28 days have passed since first launch
	if (FirstLaunchDate.IsEmpty())
	{
		return false;
	}

	FDateTime FirstLaunchDateTime;
	if (FDateTime::Parse(FirstLaunchDate, FirstLaunchDateTime))
	{
		FDateTime Now = FDateTime::Now();
		FTimespan TimeSinceFirstLaunch = Now - FirstLaunchDateTime;
		return TimeSinceFirstLaunch.GetTotalDays() >= 28;
	}

	return false;
}

void UOculusXRSurveySettings::MarkSurveyShown()
{
	bSurveyShown = true;
	SaveConfig();
}

void UOculusXRSurveySettings::RecordLaunch()
{
	// If this is the first launch, record the date
	if (FirstLaunchDate.IsEmpty())
	{
		FDateTime Now = FDateTime::Now();
		FirstLaunchDate = Now.ToString();
	}

	// Increment launch count
	LaunchCount++;

	// Save to config file
	SaveConfig();
}

#undef LOCTEXT_NAMESPACE
