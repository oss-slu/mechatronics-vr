// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "OculusXRTelemetryShared.h"
#include <Templates/Function.h>
#include <Templates/Tuple.h>
#include <Misc/Optional.h>

namespace OculusXRTelemetry
{
	bool OCULUSXRTELEMETRY_API IsTelemetryEnabled();
	bool OCULUSXRTELEMETRY_API IsActive();
	void OCULUSXRTELEMETRY_API IfActiveThen(TUniqueFunction<void()> Function);
	TTuple<FString, FString> OCULUSXRTELEMETRY_API GetProjectIdAndName();

	void OCULUSXRTELEMETRY_API SendEvent(const char* EventName, const char* Param, const char* Source = TelemetrySource);
	void OCULUSXRTELEMETRY_API SendEvent(const char* EventName, bool bParam, const char* Source = TelemetrySource);
	void OCULUSXRTELEMETRY_API SendEvent(const char* EventName, float Param, const char* Source = TelemetrySource);
	void OCULUSXRTELEMETRY_API AddMetadata(const char* Name, const char* Param);

	void OCULUSXRTELEMETRY_API SetDeveloperTelemetryConsent(bool DoesConsent);
	void OCULUSXRTELEMETRY_API SaveUnifiedConsent(bool DoesConsent);
	void OCULUSXRTELEMETRY_API SaveUnifiedConsentWithOlderVersion(bool DoesConsent, int ConsentVersion);
	TOptional<bool> OCULUSXRTELEMETRY_API GetUnifiedConsent();
	bool OCULUSXRTELEMETRY_API IsConsentGiven();
	void OCULUSXRTELEMETRY_API PropagateTelemetryConsent();

	FString OCULUSXRTELEMETRY_API GetConsentTitle();
	FString OCULUSXRTELEMETRY_API GetConsentMarkdownText();
	FString OCULUSXRTELEMETRY_API GetConsentNotificationMarkdownText(const char* consentChangeLocationMarkdown);
	FString OCULUSXRTELEMETRY_API GetConsentSettingsChangeText();
	bool OCULUSXRTELEMETRY_API IsConsentSettingsChangeEnabled();

	bool OCULUSXRTELEMETRY_API ShouldShowTelemetryConsentWindow();
	bool OCULUSXRTELEMETRY_API ShouldShowTelemetryNotification();
	void OCULUSXRTELEMETRY_API SetNotificationShown();

} // namespace OculusXRTelemetry
