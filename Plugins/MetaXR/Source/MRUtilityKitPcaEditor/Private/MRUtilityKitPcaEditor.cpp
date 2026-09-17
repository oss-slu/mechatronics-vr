// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "MRUtilityKitPcaEditor.h"

#include "AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "FMRUKPcaEditor"

DEFINE_LOG_CATEGORY(LogMRUKEditorPca);

void FMRUKPcaEditor::StartupModule()
{
	// Register asset types
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FMRUKPassthroughCameraAccessActions));
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FMRUKPassthroughCameraAccessTextureActions));
}

void FMRUKPcaEditor::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(PcaActions.ToSharedRef());
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMRUKPcaEditor, MRUtilityKitPcaEditor)
