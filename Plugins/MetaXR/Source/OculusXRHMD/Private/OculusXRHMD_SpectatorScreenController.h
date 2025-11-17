// @lint-ignore-every LICENSELINT
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "OculusXRHMDPrivate.h"

#if OCULUS_HMD_SUPPORTED_PLATFORMS
#include "DefaultSpectatorScreenController.h"
#include "Engine/TextureRenderTarget2D.h"

namespace OculusXRHMD
{

	// Oculus specific spectator screen modes that override the regular VR spectator screens
	enum class EMRSpectatorScreenMode : uint8
	{
		Default,
		ExternalComposition,
		DirectComposition
	};

	//-------------------------------------------------------------------------------------------------
	// FSpectatorScreenController
	//-------------------------------------------------------------------------------------------------

	class FSpectatorScreenController : public FDefaultSpectatorScreenController
	{
	public:
		FSpectatorScreenController(class FOculusXRHMD* InOculusXRHMD);

		void SetMRSpectatorScreenMode(EMRSpectatorScreenMode Mode) { SpectatorMode_GameThread = Mode; }
		void SetMRForeground(UTextureRenderTarget2D* Texture) { ForegroundRenderTexture_GameThread = Texture; }
		void SetMRBackground(UTextureRenderTarget2D* Texture) { BackgroundRenderTexture_GameThread = Texture; }

#if UE_VERSION_OLDER_THAN(5, 6, 0)
		virtual void RenderSpectatorScreen_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture* BackBuffer, FTextureRHIRef RenderTarget, FVector2D WindowSize) override;
		virtual void RenderSpectatorModeUndistorted(FRHICommandListImmediate& RHICmdList, FTextureRHIRef TargetTexture, FTextureRHIRef EyeTexture, FTextureRHIRef OtherTexture, FVector2D WindowSize) override;
		virtual void RenderSpectatorModeDistorted(FRHICommandListImmediate& RHICmdList, FTextureRHIRef TargetTexture, FTextureRHIRef EyeTexture, FTextureRHIRef OtherTexture, FVector2D WindowSize) override;
		virtual void RenderSpectatorModeSingleEye(FRHICommandListImmediate& RHICmdList, FTextureRHIRef TargetTexture, FTextureRHIRef EyeTexture, FTextureRHIRef OtherTexture, FVector2D WindowSize) override;
#else
		virtual void RenderSpectatorScreen_RenderThread(class FRDGBuilder& GraphBuilder, FRDGTextureRef BackBuffer, FRDGTextureRef SrcTexture, FRDGTextureRef LayersTexture, FVector2f WindowSize) override;
		virtual void AddSpectatorModePass(ESpectatorScreenMode SpectatorMode, FRDGBuilder& GraphBuilder, FRDGTextureRef TargetTexture, FRDGTextureRef EyeTexture, FRDGTextureRef OtherTexture, FVector2f WindowSize) override;
#endif

#if UE_VERSION_OLDER_THAN(5, 6, 0)
		virtual void BeginRenderViewFamily() override;
#else
		virtual void BeginRenderViewFamily(FSceneViewFamily& ViewFamily) override;
#endif

	private:
		FOculusXRHMD* OculusXRHMD;
		EMRSpectatorScreenMode SpectatorMode_GameThread;
		TWeakObjectPtr<UTextureRenderTarget2D> ForegroundRenderTexture_GameThread;
		TWeakObjectPtr<UTextureRenderTarget2D> BackgroundRenderTexture_GameThread;

		EMRSpectatorScreenMode SpectatorMode_RenderThread;
		FTextureRenderTargetResource* ForegroundRenderTexture_RenderThread;
		FTextureRenderTargetResource* BackgroundRenderTexture_RenderThread;

		void RenderSpectatorModeDirectComposition(FRHICommandListImmediate& RHICmdList, FTextureRHIRef TargetTexture, const FTextureRHIRef SrcTexture) const;
		void RenderSpectatorModeExternalComposition(FRHICommandListImmediate& RHICmdList, FTextureRHIRef TargetTexture, const FTextureRHIRef FrontTexture, const FTextureRHIRef BackTexture) const;
	};

} // namespace OculusXRHMD

#endif // OCULUS_HMD_SUPPORTED_PLATFORMS
