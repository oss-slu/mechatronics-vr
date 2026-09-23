// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "PropertyHandle.h"
#include "IPropertyTypeCustomization.h"
#include "OculusXROSVersions.h"

class OCULUSXREDITOR_API FOculusXROSVersionCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	FOculusXROSVersionCustomization();

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	FOculusXROSVersion GetOSVersionFromProperty(TSharedRef<IPropertyHandle> PropertyHandle) const;
	FString ConvertToVersionString(const FOculusXROSVersion& Version) const;

	void GetComboBoxStrings(TArray<TSharedPtr<FString>>& Options, TArray<TSharedPtr<SToolTip>>& ToolTips, TArray<bool>& RestrictedItems);
	FString GetComboBoxValue(TSharedRef<IPropertyHandle> PropertyHandle);
	void OnComboBoxSelectionMade(const FString& Value, TSharedRef<IPropertyHandle> PropertyHandle);

private:
	TArray<TSharedPtr<FString>> VersionList;
};
