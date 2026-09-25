// @lint-ignore-every LICENSELINT
// Copyright Epic Games, Inc. All Rights Reserved.

#include "OculusXRSimulator.h"
#include "Misc/EngineVersionComparison.h"

#include <sstream>

#include "JsonObjectConverter.h"
#if UE_VERSION_OLDER_THAN(5, 6, 0)
#include "MaterialHLSLGenerator.h"
#endif

#include "Algo/MaxElement.h"

#if PLATFORM_WINDOWS
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "libzip/zip.h"

#include "HAL/FileManager.h"
#include "OculusXRHMDRuntimeSettings.h"
#include "OculusXRTelemetryEvents.h"
#include "Misc/MessageDialog.h"
#include "OpenXR/OculusXROpenXRUtilities.h"
#include "Internationalization/Regex.h"

#include "Windows/WindowsPlatformMisc.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "Windows/WindowsPlatformProcess.h"
#include "Windows/WindowsHWrapper.h"
#include <filesystem>
#include <algorithm>

#if WITH_EDITOR
#include "UnrealEdMisc.h"
#endif // WITH_EDITOR

static const TCHAR* OpenXrRuntimeEnvKey = TEXT("XR_RUNTIME_JSON");
static const TCHAR* PreviousOpenXrRuntimeEnvKey = TEXT("XR_RUNTIME_JSON_PREV");
static const TCHAR* XRSimProtocolKey = TEXT("SOFTWARE\\Classes\\xrsim\\shell\\open\\command");

namespace
{
	FString ExtractExecutablePath(const FString& command)
	{
		if (command.IsEmpty())
		{
			return "";
		}

		FString executablePath;
		if (command[0] == '"')
		{
			// Quoted path: "C:\Path\To\MetaXRSimulator.exe" %1
			executablePath = command.Replace(TEXT("\""), TEXT(""));
		}
		else
		{
			// Unquoted path: C:\Program Files\MetaXRSimulator\v1\MetaXRSimulator.exe %1
			executablePath = command;
		}

		size_t paramPos = executablePath.Find("%1");
		if (paramPos != INDEX_NONE)
		{
			executablePath = executablePath.Mid(0, paramPos);
		}

		// Trim whitespace
		return executablePath.TrimEnd();
	}

	FString GetSimulatorInstallDirectory()
	{
		// Check CurrentUser first since that's where it's typically installed
		FString Command = "";
		if (!FWindowsPlatformMisc::QueryRegKey(HKEY_CURRENT_USER, XRSimProtocolKey, TEXT(""), Command) && Command.IsEmpty())
		{
			if (!FWindowsPlatformMisc::QueryRegKey(HKEY_LOCAL_MACHINE, XRSimProtocolKey, TEXT(""), Command) && Command.IsEmpty())
			{
				return "";
			}
		}

		if (Command.IsEmpty())
		{
			return "";
		}

		auto executablePath = ExtractExecutablePath(Command);

		if (!executablePath.IsEmpty() && FPaths::FileExists(executablePath))
		{
			return FPaths::GetPath(executablePath);
		}

		return "";
	}

} // namespace

FMetaXRSimulator::FMetaXRSimulator()
{
}

bool FMetaXRSimulator::IsSimulatorActivated()
{
	FString MetaXRSimPath = GetSimulatorJsonPath();
	FString CurRuntimePath = FWindowsPlatformMisc::GetEnvironmentVariable(OpenXrRuntimeEnvKey);
	return !MetaXRSimPath.IsEmpty() && MetaXRSimPath == CurRuntimePath;
}

void FMetaXRSimulator::ToggleOpenXRRuntime()
{
	if (!IsSimulatorInstalled())
	{
		SpawnNotificationToInstallXRSim();
		return;
	}
	OculusXRTelemetry::TScopedMarker<OculusXRTelemetry::Events::FSimulator> Event;

	FString MetaXRSimPath = GetSimulatorJsonPath();

#if WITH_EDITOR
	if (OculusXR::IsOpenXRSystem())
	{
		FString ActivationText = IsSimulatorActivated() ? "deactivate" : "activate";
		FString Message = FString::Format(TEXT("A restart is required in order to {0} XR simulator. The restart must be performed from this dialog, opening and closing the editor manually will not work. Restart now?"), { ActivationText });
		if (FMessageDialog::Open(EAppMsgType::OkCancel, FText::FromString(Message)) == EAppReturnType::Cancel)
		{
			UE_LOG(LogMetaXRSim, Log, TEXT("Meta XR Simulator %s action canceled."), *ActivationText);
			const auto& NotEnd = Event.SetResult(OculusXRTelemetry::EAction::Fail).AddAnnotation("reason", "restart canceled");
			return;
		}
	}
#endif // WITH_EDITOR

	if (IsSimulatorActivated())
	{
		// Deactivate MetaXR Simulator
		FString PrevOpenXrRuntimeEnvKey = FWindowsPlatformMisc::GetEnvironmentVariable(PreviousOpenXrRuntimeEnvKey);

		FWindowsPlatformMisc::SetEnvironmentVar(PreviousOpenXrRuntimeEnvKey,
			TEXT(""));
		FWindowsPlatformMisc::SetEnvironmentVar(OpenXrRuntimeEnvKey, *PrevOpenXrRuntimeEnvKey);

		UE_LOG(LogMetaXRSim, Log, TEXT("Meta XR Simulator is deactivated. (%s : %s)"), OpenXrRuntimeEnvKey, *PrevOpenXrRuntimeEnvKey);
		const auto& NotEnd = Event.AddAnnotation("action", "deactivated");
	}
	else
	{
		// Activate MetaXR Simulator
		FString CurOpenXrRuntimeEnvKey = FWindowsPlatformMisc::GetEnvironmentVariable(OpenXrRuntimeEnvKey);

		FWindowsPlatformMisc::SetEnvironmentVar(PreviousOpenXrRuntimeEnvKey,
			*CurOpenXrRuntimeEnvKey);
		FWindowsPlatformMisc::SetEnvironmentVar(OpenXrRuntimeEnvKey, *MetaXRSimPath);

		UE_LOG(LogMetaXRSim, Log, TEXT("Meta XR Simulator is activated. (%s : %s)"), OpenXrRuntimeEnvKey, *MetaXRSimPath);
		const auto& NotEnd = Event.AddAnnotation("action", "activated");
	}

#if WITH_EDITOR
	if (OculusXR::IsOpenXRSystem())
	{
		FUnrealEdMisc::Get().RestartEditor(false);
	}
#endif // WITH_EDITOR
}

FString FMetaXRSimulator::GetSimulatorJsonPath() const
{
	return FPaths::Combine(GetSimulatorInstallDirectory(), TEXT("meta_openxr_simulator.json"));
}

bool FMetaXRSimulator::IsSimulatorInstalled() const
{
	auto InstallDir = GetSimulatorInstallDirectory();
	return !InstallDir.IsEmpty() && FPaths::DirectoryExists(InstallDir);
}

void FMetaXRSimulator::SpawnNotificationToInstallXRSim()
{
	if (!FModuleManager::Get().IsModuleLoaded("MainFrame"))
	{
		return;
	}

	if (!GetSimulatorInstallDirectory().IsEmpty())
	{
		// New XR Sim is already installed do not notify
		return;
	}

	FNotificationInfo UpdateSim(FText::FromString("Meta XR Simulator Update Available"));
	UpdateSim.bFireAndForget = false;
	UpdateSim.FadeInDuration = 0.5f;
	UpdateSim.FadeOutDuration = 0.5f;
	UpdateSim.ExpireDuration = 5.0f;
	UpdateSim.bUseThrobber = false;
	UpdateSim.bUseSuccessFailIcons = true;
	UpdateSim.SubText = FText::FromString("New version includes User Interface improvements and bug fixes.");
	TPromise<TSharedPtr<SNotificationItem>> BtnNotificationPromise;
	const auto ButtonClicked = [&, NotificationFuture = BtnNotificationPromise.GetFuture().Share()](bool bUseOld) {
		NotificationFuture.Get()->Fadeout();
		FPlatformProcess::LaunchURL(TEXT("https://developers.meta.com/horizon/downloads/package/meta-xr-simulator-windows"), nullptr, nullptr);
	};

	UpdateSim.ButtonDetails.Add(FNotificationButtonInfo(
		FText::FromString("Skip"),
		FText::FromString("Skip update and use old version of simulator"),
		FSimpleDelegate::CreateLambda(ButtonClicked, false),
		SNotificationItem::CS_Pending));

	UpdateSim.ButtonDetails.Add(FNotificationButtonInfo(
		FText::FromString("Download New Version"),
		FText::FromString("Download new version of Meta XR Simulator"),
		FSimpleDelegate::CreateLambda(ButtonClicked, true),
		SNotificationItem::CS_Pending));
	TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(UpdateSim);
	if (NotificationItem.IsValid())
	{
		NotificationItem->SetCompletionState(SNotificationItem::CS_Pending);
		BtnNotificationPromise.SetValue(NotificationItem);
	}
}

#endif // PLATFORM_WINDOWS
