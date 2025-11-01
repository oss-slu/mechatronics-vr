// @lint-ignore-every LICENSELINT
// Copyright Epic Games, Inc. All Rights Reserved.

#include "OculusXRPlatformToolSettings.h"

UOculusXRPlatformToolSettings::UOculusXRPlatformToolSettings()
	: OculusTargetPlatform(EOculusXRPlatformTarget::Rift)
{
	uint8 NumPlatforms = (uint8)EOculusXRPlatformTarget::Length;
	OculusApplicationID.Init("", NumPlatforms);
	OculusApplicationToken.Init("", NumPlatforms);
	OculusReleaseChannel.Init("Alpha", NumPlatforms);
	OculusReleaseNote.Init("", NumPlatforms);
	OculusLaunchFilePath.Init("", NumPlatforms);
	OculusSymbolDirPath.Init("", NumPlatforms);
	OculusLanguagePacksPath.Init("", NumPlatforms);
	OculusExpansionFilesPath.Init("", NumPlatforms);
	OculusAssetConfigs.Init(FOculusXRAssetConfigArray(), NumPlatforms);
	UploadDebugSymbols = true;

	for (int i = 0; i < NumPlatforms; i++)
	{
		OculusAssetConfigs[i].ConfigArray = TArray<FOculusXRAssetConfig>();
	}
}

void UOculusXRPlatformToolSettings::PostInitProperties()
{
	Super::PostInitProperties();

	// Unreal will not populate an empty array beyond one from an ini file
	// the platformtool widget system assumes in many places all of the entries
	// when loaded is size NumPlatforms as created in the constructor.

	const int NumPlatforms = static_cast<int>(EOculusXRPlatformTarget::Length);
	OculusApplicationID.SetNum(NumPlatforms);
	OculusApplicationToken.SetNum(NumPlatforms);
	OculusReleaseChannel.SetNum(NumPlatforms);
	OculusReleaseNote.SetNum(NumPlatforms);
	OculusLaunchFilePath.SetNum(NumPlatforms);
	OculusSymbolDirPath.SetNum(NumPlatforms);
	OculusLanguagePacksPath.SetNum(NumPlatforms);
	OculusExpansionFilesPath.SetNum(NumPlatforms);
	OculusAssetConfigs.SetNum(NumPlatforms);
	OculusAssetConfigs.SetNum(NumPlatforms);
}
