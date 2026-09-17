// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "Modules/ModuleManager.h"
#include "MRUKPassthroughCameraAccessActions.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMRUKEditor, Log, All);

class FMRUKEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};
