// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "OculusXRTelemetryPrivacySettings.h"

#include "OculusXRTelemetryModule.h"
#include "OculusXRTelemetry.h"
#include "Misc/EngineVersionComparison.h"

#include <regex>

#define LOCTEXT_NAMESPACE "OculusXRTelemetryPrivacySettings"

constexpr int CONSENT_NOTIFICATION_MAX_LENGTH = 1024;
TMap<FString, FString> GetLinks(const std::string& markdown, std::string& textWithoutLink)
{
	TMap<FString, FString> links;
	const std::regex linkRegex(R"(\[(.*?)\]\((.*?)\))");

	std::smatch matches;
	std::string::const_iterator searchStart(markdown.cbegin());
	while (std::regex_search(searchStart, markdown.cend(), matches, linkRegex))
	{
		links.Add(matches[1].str().c_str(), matches[2].str().c_str());
		searchStart = matches.suffix().first;
	}

	const std::string linkReplacement = "";
	textWithoutLink = std::regex_replace(markdown, linkRegex, linkReplacement);
	return links;
}

UOculusXRTelemetryPrivacySettings::UOculusXRTelemetryPrivacySettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!OculusXRTelemetry::IsActive())
	{
		return;
	}

	auto optEnabled = OculusXRTelemetry::GetUnifiedConsent();
	bIsEnabled = optEnabled.Get(false);

	std::string SettingsText(TCHAR_TO_ANSI(*OculusXRTelemetry::GetConsentSettingsChangeText()));

	std::string settingsDesc = "";
	Links = GetLinks(SettingsText, settingsDesc);
	Description = FText::FromString(settingsDesc.c_str());
}

void UOculusXRTelemetryPrivacySettings::GetToggleCategoryAndPropertyNames(FName& OutCategory, FName& OutProperty) const
{
	OutCategory = FName("Options");
	OutProperty = FName("bIsEnabled");
}

FText UOculusXRTelemetryPrivacySettings::GetFalseStateLabel() const
{
	return LOCTEXT("FalseStateLabel", "Only share essential data");
}

FText UOculusXRTelemetryPrivacySettings::GetFalseStateTooltip() const
{
	return LOCTEXT("FalseStateTooltip", "Only share essential data");
}

FText UOculusXRTelemetryPrivacySettings::GetFalseStateDescription() const
{
	return Description;
}

FText UOculusXRTelemetryPrivacySettings::GetTrueStateLabel() const
{
	return LOCTEXT("TrueStateLabel", "Share additional data");
}

FText UOculusXRTelemetryPrivacySettings::GetTrueStateTooltip() const
{
	return LOCTEXT("TrueStateTooltip", "Share additional data");
}

FText UOculusXRTelemetryPrivacySettings::GetTrueStateDescription() const
{
	return Description;
}

FString UOculusXRTelemetryPrivacySettings::GetAdditionalInfoUrl() const
{
	if (Links.Num() > 0)
	{
#if UE_VERSION_OLDER_THAN(5, 7, 0)
		return Links.begin().Value();
#else
		return Links.begin()->Value;
#endif
	}
	return FString();
}

FText UOculusXRTelemetryPrivacySettings::GetAdditionalInfoUrlLabel() const
{
	if (Links.Num() > 0)
	{
#if UE_VERSION_OLDER_THAN(5, 7, 0)
		return FText::FromString(Links.begin().Key());
#else
		return FText::FromString(Links.begin()->Key);
#endif
	}
	return FText();
}

#if WITH_EDITOR
void UOculusXRTelemetryPrivacySettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UOculusXRTelemetryPrivacySettings, bIsEnabled))
	{
		using namespace OculusXRTelemetry;
		if (OculusXRTelemetry::IsActive())
		{
			OculusXRTelemetry::SaveUnifiedConsent(bIsEnabled);
			PropagateTelemetryConsent();
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE
