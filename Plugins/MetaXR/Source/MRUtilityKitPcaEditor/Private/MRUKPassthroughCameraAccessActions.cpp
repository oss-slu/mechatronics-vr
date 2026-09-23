// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "MRUKPassthroughCameraAccessActions.h"
#include "MRUKPassthroughCameraAccess.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

UClass* FMRUKPassthroughCameraAccessActions::GetSupportedClass() const
{
	return UMRUKPassthroughCameraAccess::StaticClass();
}

FText FMRUKPassthroughCameraAccessActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_PassthroughCameraAccess", "PassthroughCameraAcccess");
}

FColor FMRUKPassthroughCameraAccessActions::GetTypeColor() const
{
	return FColor::Cyan;
}

const TArray<FText>& FMRUKPassthroughCameraAccessActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus{
		LOCTEXT("AssetMRUtilityKitPassthroughCameraAccessSubMenu", "MRUtilityKit")
	};

	return SubMenus;
}

uint32 FMRUKPassthroughCameraAccessActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

UClass* FMRUKPassthroughCameraAccessTextureActions::GetSupportedClass() const
{
	return UMRUKPassthroughCameraAccessTexture::StaticClass();
}

FText FMRUKPassthroughCameraAccessTextureActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_PassthroughCameraAccess", "PassthroughCameraAcccessTexture");
}

FColor FMRUKPassthroughCameraAccessTextureActions::GetTypeColor() const
{
	return FColor::Cyan;
}

const TArray<FText>& FMRUKPassthroughCameraAccessTextureActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus{
		LOCTEXT("AssetMRUtilityKitPassthroughCameraAccessSubMenu", "MRUtilityKit")
	};

	return SubMenus;
}

uint32 FMRUKPassthroughCameraAccessTextureActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

#undef LOCTEXT_NAMESPACE
