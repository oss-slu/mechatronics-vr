// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"

#include "MRUKPassthroughCameraAccessFactory.generated.h"

UCLASS(hidecategories = Object, MinimalAPI)
class UMRUKPassthroughCameraAccessFactory : public UFactory
{
	GENERATED_BODY()

public:
	UMRUKPassthroughCameraAccessFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	virtual uint32 GetMenuCategories() const override;
};

UCLASS(hidecategories = Object, MinimalAPI)
class UMRUKPassthroughCameraAccessTextureFactory : public UFactory
{
	GENERATED_BODY()

public:
	UMRUKPassthroughCameraAccessTextureFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	virtual uint32 GetMenuCategories() const override;
};
