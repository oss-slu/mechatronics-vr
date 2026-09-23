// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "MRUKPassthroughCameraAccessFactory.h"
#include "MRUKPassthroughCameraAccess.h"
#include "AssetTypeCategories.h"

UMRUKPassthroughCameraAccessFactory::UMRUKPassthroughCameraAccessFactory()
{
	SupportedClass = UMRUKPassthroughCameraAccess::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UMRUKPassthroughCameraAccessFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UMRUKPassthroughCameraAccess>(InParent, Name, Flags);
}

uint32 UMRUKPassthroughCameraAccessFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Misc;
}

UMRUKPassthroughCameraAccessTextureFactory::UMRUKPassthroughCameraAccessTextureFactory()
{
	SupportedClass = UMRUKPassthroughCameraAccessTexture::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UMRUKPassthroughCameraAccessTextureFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UMRUKPassthroughCameraAccessTexture>(InParent, Name, Flags);
}

uint32 UMRUKPassthroughCameraAccessTextureFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Misc;
}
