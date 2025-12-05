// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "OculusXRPluginRules.h"
#include "CoreMinimal.h"
#include "OculusXRHMDRuntimeSettings.h"
#include "OculusXRPSTUtils.h"
#include "Editor/GameProjectGeneration/Public/GameProjectGenerationModule.h"
#include "Interfaces/IPluginManager.h"
#include "Interfaces/IProjectManager.h"
#include "Misc/MessageDialog.h"
#include "Misc/FileHelper.h"

namespace OculusXRPluginRules
{
	static const FString ManifestPath = "/Build/Android/ManifestRequirementsAdditions.txt";

	namespace
	{
		bool IsPluginEnabled(const FString& PluginName)
		{
			const auto Plugin = IPluginManager::Get().FindPlugin(PluginName);
			if (!Plugin)
			{
				return false;
			}

			return Plugin->IsEnabled();
		}

		bool DisablePlugin(const FString& PluginName)
		{
			FText FailMessage;
			bool bSuccess = IProjectManager::Get().SetPluginEnabled(
				PluginName, false, FailMessage);
			const bool bIsProjectDirty = IProjectManager::Get().IsCurrentProjectDirty();
			if (bSuccess && bIsProjectDirty)
			{
				FGameProjectGenerationModule::Get().TryMakeProjectFileWriteable(FPaths::GetProjectFilePath());
				bSuccess = IProjectManager::Get().SaveCurrentProjectToDisk(FailMessage);
			}
			if (!bSuccess)
			{
				FMessageDialog::Open(EAppMsgType::Ok, FailMessage);
			}

			return bSuccess && !bIsProjectDirty;
		}
	} // namespace
	bool FUseRecommendedXRAPIRule::IsApplied() const
	{
		const UOculusXRHMDRuntimeSettings* Settings = GetMutableDefault<UOculusXRHMDRuntimeSettings>();
		return Settings->XrApi == EOculusXRXrApi::OVRPluginOpenXR;
	}

	void FUseRecommendedXRAPIRule::ApplyImpl(bool& OutShouldRestartEditor)
	{
		OCULUSXR_UPDATE_SETTINGS(UOculusXRHMDRuntimeSettings, XrApi, EOculusXRXrApi::OVRPluginOpenXR);
		OutShouldRestartEditor = false;
	}

	bool FDisableOculusVRRule::IsApplied() const
	{
		return bApplied || !IsPluginEnabled(PluginName);
	}

	void FDisableOculusVRRule::ApplyImpl(bool& OutShouldRestartEditor)
	{
		OutShouldRestartEditor = DisablePlugin(PluginName);
		bApplied = OutShouldRestartEditor;
	}

	bool FDisableSteamVRRule::IsApplied() const
	{
		return bApplied || !IsPluginEnabled(PluginName);
	}

	void FDisableSteamVRRule::ApplyImpl(bool& OutShouldRestartEditor)
	{
		OutShouldRestartEditor = DisablePlugin(PluginName);
		bApplied = OutShouldRestartEditor;
	}

	bool FUseHorizonOSSDKRule::IsApplied() const
	{
		if (GIsAutomationTesting)
		{
			return true;
		}

		const UOculusXRHMDRuntimeSettings* Settings = GetDefault<UOculusXRHMDRuntimeSettings>();

		FString Path(FPaths::ProjectDir() + ManifestPath);
		if (!FPaths::FileExists(Path))
		{
			return false;
		}

		TArray<FString> Lines;
		if (!FFileHelper::LoadFileToStringArray(Lines, *Path))
		{
			return false;
		}

		for (const FString& Line : Lines)
		{
			// Default is the current OS version
			int32 MinVer = FOculusXROSVersionHelpers::GetOSVersion();
			int32 TargetVer = MinVer;

			// Override
			if (Settings->bHorizonOSVersionOverride)
			{
				MinVer = Settings->MinOSVersion.GetVersion();
				TargetVer = Settings->TargetOSVersion.GetVersion();
			}

			FString ExpectedStr = FString::Printf(TEXT("<horizonos:uses-horizonos-sdk xmlns:horizonos=\"http://schemas.horizonos/sdk\" horizonos:minSdkVersion=\"%d\" horizonos:targetSdkVersion=\"%d\" />"), MinVer, TargetVer);

			if (Line.Contains(ExpectedStr))
			{
				return true;
			}
		}

		return false;
	}

	void FUseHorizonOSSDKRule::ApplyImpl(bool& OutShouldRestartEditor)
	{
		const UOculusXRHMDRuntimeSettings* Settings = GetDefault<UOculusXRHMDRuntimeSettings>();

		TArray<FString> Lines;

		FString Path(FPaths::ProjectDir() + ManifestPath);
		if (FPaths::FileExists(Path))
		{
			FFileHelper::LoadFileToStringArray(Lines, *Path);
		}

		// Remove existing lines that match the uses-horizonos-sdk tag
		int32 FoundIndex = INDEX_NONE;
		do
		{
			FoundIndex = Lines.IndexOfByPredicate([](const FString& element) {
				return element.Contains("horizonos:uses-horizonos-sdk");
			});
			if (FoundIndex != INDEX_NONE)
			{
				Lines.RemoveAt(FoundIndex);
			}
		}
		while (FoundIndex != INDEX_NONE);

		int32 MinVer = FOculusXROSVersionHelpers::GetOSVersion();
		int32 TargetVer = MinVer;

		if (Settings->bHorizonOSVersionOverride)
		{
			MinVer = Settings->MinOSVersion.GetVersion();
			TargetVer = Settings->TargetOSVersion.GetVersion();
		}

		FString XmlStr = FString::Printf(TEXT("<horizonos:uses-horizonos-sdk xmlns:horizonos=\"http://schemas.horizonos/sdk\" horizonos:minSdkVersion=\"%d\" horizonos:targetSdkVersion=\"%d\" />"), MinVer, TargetVer);

		Lines.Add(XmlStr);

		FFileHelper::SaveStringArrayToFile(Lines, *Path);
	}
} // namespace OculusXRPluginRules

#undef LOCTEXT_NAMESPACE
