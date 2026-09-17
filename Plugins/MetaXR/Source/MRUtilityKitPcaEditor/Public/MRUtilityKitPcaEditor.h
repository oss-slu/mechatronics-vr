// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "Modules/ModuleManager.h"
#include "MRUKPassthroughCameraAccessActions.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMRUKEditorPca, Log, All);

class FMRUKPcaEditor : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

private:
	TSharedPtr<FMRUKPassthroughCameraAccessActions> PcaActions;
};
