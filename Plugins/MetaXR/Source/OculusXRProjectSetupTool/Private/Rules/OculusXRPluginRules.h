// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once
#include "OculusXRSetupRule.h"

// Collection of rules related to plugins. Can be extended as needed
namespace OculusXRPluginRules
{
	class FUseRecommendedXRAPIRule final : public ISetupRule
	{
	public:
		FUseRecommendedXRAPIRule()
			: ISetupRule("Plugin_UseRecommendedXRAPI",
				  NSLOCTEXT("OculusXRPluginRules", "UseRecommendedXRAPI_DisplayName", "Use Recommended XR API"),
				  NSLOCTEXT("OculusXRPluginRules", "UseRecommendedXRAPI_Description", "It is currently recommended to use OVRPlugin + OpenXR for the XR API"),
				  ESetupRuleCategory::Plugins,
				  ESetupRuleSeverity::Warning) {}

		virtual bool IsApplied() const override;

	protected:
		virtual void ApplyImpl(bool& OutShouldRestartEditor) override;
	};

	class FDisableOculusVRRule final : public ISetupRule
	{
	public:
		FDisableOculusVRRule()
			: ISetupRule("Plugin_DisableOculusVR",
				  NSLOCTEXT("OculusXRPluginRules", "DisableOculusVR_DisplayName", "Disable OculusVR Plugin"),
				  NSLOCTEXT("OculusXRPluginRules", "DisableOculusVR_Description", "The OculusVR plugin is deprecated and should be disabled"),
				  ESetupRuleCategory::Plugins,
				  ESetupRuleSeverity::Warning) {}

		virtual bool IsApplied() const override;

	protected:
		virtual void ApplyImpl(bool& OutShouldRestartEditor) override;

	private:
		FString PluginName = "OculusVR";
		bool bApplied = false;
	};

	class FDisableSteamVRRule final : public ISetupRule
	{
	public:
		FDisableSteamVRRule()
			: ISetupRule("Plugin_DisableSteamVR",
				  NSLOCTEXT("OculusXRPluginRules", "DisableSteamVR_DisplayName", "Disable SteamVR Plugin"),
				  NSLOCTEXT("OculusXRPluginRules", "DisableSteamVR_Description", "The SteamVR plugin is deprecated and should be disabled"),
				  ESetupRuleCategory::Plugins,
				  ESetupRuleSeverity::Warning) {}

		virtual bool IsApplied() const override;

	protected:
		virtual void ApplyImpl(bool& OutShouldRestartEditor) override;

	private:
		FString PluginName = "SteamVR";
		bool bApplied = false;
	};

	class FUseHorizonOSSDKRule final : public ISetupRule
	{
	public:
		FUseHorizonOSSDKRule()
			: ISetupRule("Plugin_UseHorizonOSSDK",
				  NSLOCTEXT("OculusXRPluginRules", "UseHorizonOSSDK_DisplayName", "Update Android Manifest to support Horizon OS SDK"),
				  NSLOCTEXT("OculusXRPluginRules", "UseHorizonOSSDK_Description", "The Android manifest needs to be updated to support usage of the Horizon OS SDK."),
				  ESetupRuleCategory::Plugins,
				  ESetupRuleSeverity::Critical) {}

		virtual bool IsApplied() const override;

	protected:
		virtual void ApplyImpl(bool& OutShouldRestartEditor) override;
	};

	// All defined plugin rules. Add new rules to this table for them to be auto-registered
	inline TArray<SetupRulePtr> PluginRules_Table{
		MakeShared<FUseRecommendedXRAPIRule>(),
		MakeShared<FDisableOculusVRRule>(),
		MakeShared<FDisableSteamVRRule>(),
		MakeShared<FUseHorizonOSSDKRule>()
	};
} // namespace OculusXRPluginRules
