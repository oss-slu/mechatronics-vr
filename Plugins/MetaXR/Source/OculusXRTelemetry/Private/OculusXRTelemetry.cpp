// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "OculusXRTelemetry.h"
#include "OculusXRTelemetryModule.h"
#include "OculusXRTelemetryPrivacySettings.h"
#include <Async/Async.h>
#include <Containers/StringConv.h>
#include <GeneralProjectSettings.h>

namespace OculusXRTelemetry
{
	bool IsActive()
	{
#if OCULUS_HMD_SUPPORTED_PLATFORMS
		if constexpr (FTelemetryBackend::IsNullBackend())
		{
			return false;
		}

		// IsActive() can be called during shutdown, after the FOculusXRHMDModule has been unloaded.
		// This means we can't use the checked FOculusXRHMDModule::Get(), we instead need to use a fallible
		// GetModule(..) and check that the module exists.
		if (!FOculusXRTelemetryModule::IsAvailable())
		{
			return false;
		}

		// Module is loaded, check if the plugin initialized
		auto& telemetryModule = FOculusXRTelemetryModule::Get();
		if (telemetryModule.GetTelemetryPlugin().IsInitialized())
		{
			return true;
		}
#endif // OCULUS_HMD_SUPPORTED_PLATFORMS
		return false;
	}

	void IfActiveThen(TUniqueFunction<void()> Function)
	{
		AsyncTask(ENamedThreads::GameThread, [F = MoveTemp(Function)]() {
			if (IsActive())
			{
				F();
			}
		});
	}

	void PropagateTelemetryConsent()
	{
#ifdef WITH_EDITOR
		if (OculusXR::EngineTelemetryPlugin::IsPluginEnabled())
		{
			const bool bHasConsent = OculusXR::EngineTelemetryPlugin::GetPlugin().GetUnifiedConsent(UNREAL_TOOL_ID) == telemetryBool_True;
			OculusXR::EngineTelemetryPlugin::GetPlugin().QplSetConsent(bHasConsent);
			OculusXR::EngineTelemetryPlugin::GetPlugin().SetDeveloperTelemetryConsent(bHasConsent);
		}
#endif
	}

	TTuple<FString, FString> GetProjectIdAndName()
	{
		const UGeneralProjectSettings& ProjectSettings = *GetDefault<UGeneralProjectSettings>();
		return MakeTuple(ProjectSettings.ProjectID.ToString(), ProjectSettings.ProjectName);
	}

	bool IsConsentGiven()
	{
#ifdef WITH_EDITOR
		if (const UOculusXRTelemetryPrivacySettings* EditorPrivacySettings = GetDefault<UOculusXRTelemetryPrivacySettings>())
		{
			return EditorPrivacySettings->bIsEnabled;
		}
#endif
		return false;
	}

	void SendEvent(const char* EventName, float Param, const char* Source)
	{
		const FString StrVal = FString::Printf(TEXT("%f"), Param);
		SendEvent(EventName, TCHAR_TO_ANSI(*StrVal));
	}

	void SendEvent(const char* EventName, bool bParam, const char* Source)
	{
		SendEvent(EventName, bParam ? "true" : "false");
	}

	void SendEvent(const char* EventName, const char* Param, const char* Source)
	{
		if (IsActive())
		{
			OculusXR::EngineTelemetryPlugin::GetPlugin().SendEvent(EventName, Param, Source);
		}
	}

	void AddMetadata(const char* Name, const char* Param)
	{
		if (IsActive())
		{
			OculusXR::EngineTelemetryPlugin::GetPlugin().AddCustomMetadata(Name, Param);
		}
	}

	bool IsTelemetryEnabled()
	{
		return IsActive();
	}

	void SetDeveloperTelemetryConsent(bool DoesConsent)
	{
		if (IsActive())
		{
			OculusXR::EngineTelemetryPlugin::GetPlugin().SetDeveloperTelemetryConsent(DoesConsent ? telemetryBool_True : telemetryBool_False);
		}
	}

	void SaveUnifiedConsent(bool DoesConsent)
	{
		if (IsActive())
		{
			OculusXR::EngineTelemetryPlugin::GetPlugin().SaveUnifiedConsent(UNREAL_TOOL_ID, DoesConsent ? telemetryBool_True : telemetryBool_False);
		}
	}

	void SaveUnifiedConsentWithOlderVersion(bool DoesConsent, int ConsentVersion)
	{
		if (IsActive())
		{
			OculusXR::EngineTelemetryPlugin::GetPlugin().SaveUnifiedConsentWithOlderVersion(UNREAL_TOOL_ID, DoesConsent ? telemetryBool_True : telemetryBool_False, ConsentVersion);
		}
	}

	TOptional<bool> GetUnifiedConsent()
	{
		TOptional<bool> result;
		if (IsActive())
		{
			auto telemetryResult = OculusXR::EngineTelemetryPlugin::GetPlugin().GetUnifiedConsent(UNREAL_TOOL_ID);
			switch (telemetryResult)
			{
				case telemetryOptionalBool_Unknown:
					break;
				case telemetryOptionalBool_True:
					result = true;
					break;
				case telemetryOptionalBool_False:
					result = false;
					break;
			}
		}

		return result;
	}

	FString GetConsentTitle()
	{
		if (IsActive())
		{
			char consentTitle[CONSENT_TITLE_MAX_LENGTH] = {};
			if (OculusXR::EngineTelemetryPlugin::GetPlugin().GetConsentTitle(consentTitle) == telemetryBool_True)
			{
				return FString(consentTitle);
			}
		}

		return FString();
	}

	FString GetConsentMarkdownText()
	{
		if (IsActive())
		{
			char consentMarkdown[CONSENT_TEXT_MAX_LENGTH] = {};
			if (OculusXR::EngineTelemetryPlugin::GetPlugin().GetConsentMarkdownText(consentMarkdown) == telemetryBool_True)
			{
				return FString(consentMarkdown);
			}
		}

		return FString();
	}

	FString GetConsentNotificationMarkdownText(const char* consentChangeLocationMarkdown)
	{
		if (IsActive())
		{
			char consentNotificationMarkdown[CONSENT_NOTIFICATION_MAX_LENGTH] = {};
			if (OculusXR::EngineTelemetryPlugin::GetPlugin().GetConsentNotificationMarkdownText(consentChangeLocationMarkdown, consentNotificationMarkdown) == telemetryBool_True)
			{
				return FString(consentNotificationMarkdown);
			}
		}

		return FString();
	}

	FString GetConsentSettingsChangeText()
	{
		if (IsActive())
		{
			char settingsChanged[CONSENT_TEXT_MAX_LENGTH] = {};
			if (OculusXR::EngineTelemetryPlugin::GetPlugin().GetConsentSettingsChangeText(settingsChanged) == telemetryBool_True)
			{
				return FString(settingsChanged);
			}
		}

		return FString();
	}

	bool IsConsentSettingsChangeEnabled()
	{
		if (IsActive())
		{
			return OculusXR::EngineTelemetryPlugin::GetPlugin().IsConsentSettingsChangeEnabled(UNREAL_TOOL_ID) == telemetryBool_True;
		}

		return false;
	}

	bool ShouldShowTelemetryConsentWindow()
	{
		if (IsActive())
		{
			return OculusXR::EngineTelemetryPlugin::GetPlugin().ShouldShowTelemetryConsentWindow(UNREAL_TOOL_ID) == telemetryBool_True;
		}

		return false;
	}

	bool ShouldShowTelemetryNotification()
	{
		if (IsActive())
		{
			return OculusXR::EngineTelemetryPlugin::GetPlugin().ShouldShowTelemetryNotification(UNREAL_TOOL_ID) == telemetryBool_True;
		}

		return false;
	}

	void SetNotificationShown()
	{
		if (IsActive())
		{
			OculusXR::EngineTelemetryPlugin::GetPlugin().SetNotificationShown(UNREAL_TOOL_ID);
		}
	}

} // namespace OculusXRTelemetry
