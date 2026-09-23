// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "OculusXROSVersionPropertyCustomization.h"
#include "DetailWidgetRow.h"
#include "PropertyRestriction.h"
#include "DetailLayoutBuilder.h"
#include "Styling/DefaultStyleCache.h"
#include "PropertyCustomizationHelpers.h"

static const FString LatestStr = "Latest";

TSharedRef<IPropertyTypeCustomization> FOculusXROSVersionCustomization::MakeInstance()
{
	return MakeShared<FOculusXROSVersionCustomization>();
}

FOculusXROSVersionCustomization::FOculusXROSVersionCustomization()
{
	int32 MinVer = FOculusXROSVersionHelpers::MinimumOSVersion;
	int32 TargetVer = FOculusXROSVersionHelpers::GetOSVersion();

	for (int32 i = MinVer; i <= TargetVer; ++i)
	{
		VersionList.Add(MakeShared<FString>(FString::FromInt(i)));
	}

	VersionList.Add(MakeShared<FString>(LatestStr));
}

void FOculusXROSVersionCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	HeaderRow
		.NameContent()
			[PropertyHandle->CreatePropertyNameWidget()]
		.ValueContent()
			[PropertyCustomizationHelpers::MakePropertyComboBox(nullptr,
				FOnGetPropertyComboBoxStrings::CreateRaw(this, &FOculusXROSVersionCustomization::GetComboBoxStrings),
				FOnGetPropertyComboBoxValue::CreateRaw(this, &FOculusXROSVersionCustomization::GetComboBoxValue, PropertyHandle),
				FOnPropertyComboBoxValueSelected::CreateRaw(this, &FOculusXROSVersionCustomization::OnComboBoxSelectionMade, PropertyHandle))];
}

void FOculusXROSVersionCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

FOculusXROSVersion FOculusXROSVersionCustomization::GetOSVersionFromProperty(TSharedRef<IPropertyHandle> PropertyHandle) const
{
	TSharedPtr<IPropertyHandle> VersionProp = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOculusXROSVersion, Version));
	TSharedPtr<IPropertyHandle> LatestProp = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOculusXROSVersion, bLatest));
	if (!VersionProp || !LatestProp)
	{
		return FOculusXROSVersion();
	}

	bool bIsLatest = false;
	LatestProp->GetValue(bIsLatest);

	int32 value = 0;
	VersionProp->GetValue(value);

	FOculusXROSVersion Result;
	Result.Version = value;
	Result.bLatest = bIsLatest;

	return Result;
}

FString FOculusXROSVersionCustomization::ConvertToVersionString(const FOculusXROSVersion& Version) const
{
	return Version.bLatest ? LatestStr : FString::FromInt(Version.Version);
}

void FOculusXROSVersionCustomization::GetComboBoxStrings(TArray<TSharedPtr<FString>>& Options, TArray<TSharedPtr<SToolTip>>& ToolTips, TArray<bool>& RestrictedItems)
{
	Options = VersionList;
}

FString FOculusXROSVersionCustomization::GetComboBoxValue(TSharedRef<IPropertyHandle> PropertyHandle)
{
	auto Version = GetOSVersionFromProperty(PropertyHandle);
	return ConvertToVersionString(Version);
}

void FOculusXROSVersionCustomization::OnComboBoxSelectionMade(const FString& Value, TSharedRef<IPropertyHandle> PropertyHandle)
{
	TSharedPtr<IPropertyHandle> VersionProp = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOculusXROSVersion, Version));
	TSharedPtr<IPropertyHandle> LatestProp = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOculusXROSVersion, bLatest));
	if (!VersionProp || !LatestProp)
	{
		return;
	}

	if (Value == LatestStr)
	{
		VersionProp->SetValue(0);
		LatestProp->SetValue(true);
	}
	else
	{
		VersionProp->SetValue(FCString::Atoi(*Value));
		LatestProp->SetValue(false);
	}
}
