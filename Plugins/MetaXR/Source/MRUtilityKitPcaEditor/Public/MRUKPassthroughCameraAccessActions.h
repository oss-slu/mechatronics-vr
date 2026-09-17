// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

class MRUTILITYKITPCAEDITOR_API FMRUKPassthroughCameraAccessActions : public FAssetTypeActions_Base
{
public:
	virtual UClass* GetSupportedClass() const override;

	virtual FText GetName() const override;

	virtual FColor GetTypeColor() const override;

	virtual const TArray<FText>& GetSubMenus() const override;

	virtual uint32 GetCategories() override;
};

class MRUTILITYKITPCAEDITOR_API FMRUKPassthroughCameraAccessTextureActions : public FAssetTypeActions_Base
{
public:
	virtual UClass* GetSupportedClass() const override;

	virtual FText GetName() const override;

	virtual FColor GetTypeColor() const override;

	virtual const TArray<FText>& GetSubMenus() const override;

	virtual uint32 GetCategories() override;
};
