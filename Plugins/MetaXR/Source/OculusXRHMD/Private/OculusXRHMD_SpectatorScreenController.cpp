// @lint-ignore-every LICENSELINT
// Copyright Epic Games, Inc. All Rights Reserved.

#include "OculusXRHMD_SpectatorScreenController.h"

#if OCULUS_HMD_SUPPORTED_PLATFORMS
#include "OculusXRHMD.h"
#include "TextureResource.h"
#include "Engine/TextureRenderTarget2D.h"

#if !UE_VERSION_OLDER_THAN(5, 6, 0)
#include "XRCopyTexture.h"
#endif

namespace OculusXRHMD
{

	//-------------------------------------------------------------------------------------------------
	// FSpectatorScreenController
	//-------------------------------------------------------------------------------------------------

	FSpectatorScreenController::FSpectatorScreenController(FOculusXRHMD* InOculusXRHMD)
		: FDefaultSpectatorScreenController(InOculusXRHMD)
		, OculusXRHMD(InOculusXRHMD)
		, SpectatorMode_GameThread(EMRSpectatorScreenMode::Default)
		, ForegroundRenderTexture_GameThread(nullptr)
		, BackgroundRenderTexture_GameThread(nullptr)
		, SpectatorMode_RenderThread(EMRSpectatorScreenMode::Default)
		, ForegroundRenderTexture_RenderThread(nullptr)
		, BackgroundRenderTexture_RenderThread(nullptr)
	{
	}

#if UE_VERSION_OLDER_THAN(5, 6, 0)
	void FSpectatorScreenController::BeginRenderViewFamily()
#else
	void FSpectatorScreenController::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
#endif
	{
		UTextureRenderTarget2D* ForegroundTexture = ForegroundRenderTexture_GameThread.Get();
		UTextureRenderTarget2D* BackgroundTexture = BackgroundRenderTexture_GameThread.Get();

		EMRSpectatorScreenMode SpectatorMode = SpectatorMode_GameThread;
		FTextureRenderTargetResource* ForegroundResource = ForegroundTexture ? ForegroundTexture->GetRenderTargetResource() : nullptr;
		FTextureRenderTargetResource* BackgroundResource = BackgroundTexture ? BackgroundTexture->GetRenderTargetResource() : nullptr;
		ENQUEUE_RENDER_COMMAND(FSpectatorScreenController_LatchRenderValues)
		(
			[this, SpectatorMode, ForegroundResource, BackgroundResource](FRHICommandList&) {
				SpectatorMode_RenderThread = SpectatorMode;
				ForegroundRenderTexture_RenderThread = ForegroundResource;
				BackgroundRenderTexture_RenderThread = BackgroundResource;
			});

#if UE_VERSION_OLDER_THAN(5, 6, 0)
		FDefaultSpectatorScreenController::BeginRenderViewFamily();
#else
		FDefaultSpectatorScreenController::BeginRenderViewFamily(InViewFamily);
#endif
	}

#if UE_VERSION_OLDER_THAN(5, 6, 0)
	void FSpectatorScreenController::RenderSpectatorScreen_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture* BackBuffer, FTextureRHIRef RenderTexture, FVector2D WindowSize)
	{
		CheckInRenderThread(RHICmdList);
		if (OculusXRHMD->GetCustomPresent_Internal())
		{
			if (SpectatorMode_RenderThread == EMRSpectatorScreenMode::ExternalComposition)
			{
				if (ForegroundRenderTexture_RenderThread && BackgroundRenderTexture_RenderThread)
				{
					RenderSpectatorModeExternalComposition(
						RHICmdList,
						FTextureRHIRef(BackBuffer),
						ForegroundRenderTexture_RenderThread->GetRenderTargetTexture(),
						BackgroundRenderTexture_RenderThread->GetRenderTargetTexture());
					return;
				}
			}
			else if (SpectatorMode_RenderThread == EMRSpectatorScreenMode::DirectComposition)
			{
				if (BackgroundRenderTexture_RenderThread)
				{
					RenderSpectatorModeDirectComposition(
						RHICmdList,
						FTextureRHIRef(BackBuffer),
						BackgroundRenderTexture_RenderThread->GetRenderTargetTexture());
					return;
				}
			}
			FDefaultSpectatorScreenController::RenderSpectatorScreen_RenderThread(RHICmdList, BackBuffer, RenderTexture, WindowSize);
		}
	}
	void FSpectatorScreenController::RenderSpectatorModeUndistorted(FRHICommandListImmediate& RHICmdList, FTextureRHIRef TargetTexture, FTextureRHIRef EyeTexture, FTextureRHIRef OtherTexture, FVector2D WindowSize)
	{
		CheckInRenderThread(RHICmdList);
		FSettings* Settings = OculusXRHMD->GetSettings_RenderThread();
		FIntRect DestRect(0, 0, TargetTexture->GetSizeX() / 2, TargetTexture->GetSizeY());
		for (int i = 0; i < 2; ++i)
		{
			OculusXRHMD->CopyTexture_RenderThread(RHICmdList, EyeTexture, Settings->EyeRenderViewport[i], TargetTexture, DestRect, false, true);
			DestRect.Min.X += TargetTexture->GetSizeX() / 2;
			DestRect.Max.X += TargetTexture->GetSizeX() / 2;
		}
	}

	void FSpectatorScreenController::RenderSpectatorModeDistorted(FRHICommandListImmediate& RHICmdList, FTextureRHIRef TargetTexture, FTextureRHIRef EyeTexture, FTextureRHIRef OtherTexture, FVector2D WindowSize)
	{
		CheckInRenderThread(RHICmdList);
		FCustomPresent* CustomPresent = OculusXRHMD->GetCustomPresent_Internal();
		FTextureRHIRef MirrorTexture = CustomPresent->GetMirrorTexture();
		if (MirrorTexture)
		{
			FIntRect SrcRect(0, 0, MirrorTexture->GetSizeX(), MirrorTexture->GetSizeY());
			FIntRect DestRect(0, 0, TargetTexture->GetSizeX(), TargetTexture->GetSizeY());
			OculusXRHMD->CopyTexture_RenderThread(RHICmdList, MirrorTexture, SrcRect, TargetTexture, DestRect, false, true);
		}
	}

	void FSpectatorScreenController::RenderSpectatorModeSingleEye(FRHICommandListImmediate& RHICmdList, FTextureRHIRef TargetTexture, FTextureRHIRef EyeTexture, FTextureRHIRef OtherTexture, FVector2D WindowSize)
	{
		CheckInRenderThread(RHICmdList);
		FSettings* Settings = OculusXRHMD->GetSettings_RenderThread();
		const FIntRect SrcRect = Settings->EyeRenderViewport[0];
		const FIntRect DstRect(0, 0, TargetTexture->GetSizeX(), TargetTexture->GetSizeY());

		OculusXRHMD->CopyTexture_RenderThread(RHICmdList, EyeTexture, SrcRect, TargetTexture, DstRect, false, true);
	}

	void FSpectatorScreenController::RenderSpectatorModeDirectComposition(FRHICommandListImmediate& RHICmdList, FTextureRHIRef TargetTexture, const FTextureRHIRef SrcTexture) const
	{
		CheckInRenderThread(RHICmdList);
		const FIntRect SrcRect(0, 0, SrcTexture->GetSizeX(), SrcTexture->GetSizeY());
		const FIntRect DstRect(0, 0, TargetTexture->GetSizeX(), TargetTexture->GetSizeY());

		OculusXRHMD->CopyTexture_RenderThread(RHICmdList, SrcTexture, SrcRect, TargetTexture, DstRect, false, true);
	}

	void FSpectatorScreenController::RenderSpectatorModeExternalComposition(FRHICommandListImmediate& RHICmdList, FTextureRHIRef TargetTexture, const FTextureRHIRef FrontTexture, const FTextureRHIRef BackTexture) const
	{
		CheckInRenderThread(RHICmdList);
		const FIntRect FrontSrcRect(0, 0, FrontTexture->GetSizeX(), FrontTexture->GetSizeY());
		const FIntRect FrontDstRect(0, 0, TargetTexture->GetSizeX() / 2, TargetTexture->GetSizeY());
		const FIntRect BackSrcRect(0, 0, BackTexture->GetSizeX(), BackTexture->GetSizeY());
		const FIntRect BackDstRect(TargetTexture->GetSizeX() / 2, 0, TargetTexture->GetSizeX(), TargetTexture->GetSizeY());

		OculusXRHMD->CopyTexture_RenderThread(RHICmdList, FrontTexture, FrontSrcRect, TargetTexture, FrontDstRect, false, true);
		OculusXRHMD->CopyTexture_RenderThread(RHICmdList, BackTexture, BackSrcRect, TargetTexture, BackDstRect, false, true);
	}
#else
	void FSpectatorScreenController::RenderSpectatorScreen_RenderThread(FRDGBuilder& GraphBuilder, FRDGTextureRef BackBuffer, FRDGTextureRef SrcTexture, FRDGTextureRef LayersTexture, FVector2f WindowSize)
	{
		if (OculusXRHMD->GetCustomPresent_Internal())
		{
			if (SpectatorMode_RenderThread == EMRSpectatorScreenMode::ExternalComposition)
			{
				if (ForegroundRenderTexture_RenderThread && BackgroundRenderTexture_RenderThread)
				{
					FRDGTextureRef FrontTexture = RegisterExternalTexture(GraphBuilder, ForegroundRenderTexture_RenderThread->GetRenderTargetTexture(), TEXT("OculusXRHMD_ForegroundRenderTexture"));
					const FIntRect FrontSrcRect(0, 0, FrontTexture->Desc.GetSize().X, FrontTexture->Desc.GetSize().Y);
					const FIntRect FrontDstRect(0, 0, BackBuffer->Desc.GetSize().X / 2, BackBuffer->Desc.GetSize().Y);

					FRDGTextureRef BackTexture = RegisterExternalTexture(GraphBuilder, BackgroundRenderTexture_RenderThread->GetRenderTargetTexture(), TEXT("OculusXRHMD_BackgroundRenderTexture"));
					const FIntRect BackSrcRect(0, 0, BackTexture->Desc.GetSize().X, BackTexture->Desc.GetSize().Y);
					const FIntRect BackDstRect(BackBuffer->Desc.GetSize().X / 2, 0, BackBuffer->Desc.GetSize().X, BackBuffer->Desc.GetSize().Y);

					FXRCopyTextureOptions Options(FeatureLevel_RenderThread, ShaderPlatform_RenderThread);
					Options.SetDisplayMappingOptions(OculusXRHMD);

					AddXRCopyTexturePass(GraphBuilder, RDG_EVENT_NAME("OculusXR_Spectator_ExternalComposition_Front"), FrontTexture, FrontSrcRect, BackBuffer, FrontDstRect, Options);
					AddXRCopyTexturePass(GraphBuilder, RDG_EVENT_NAME("OculusXR_Spectator_ExternalComposition_Back"), BackTexture, BackSrcRect, BackBuffer, BackDstRect, Options);
					return;
				}
			}
			else if (SpectatorMode_RenderThread == EMRSpectatorScreenMode::DirectComposition)
			{
				if (BackgroundRenderTexture_RenderThread)
				{
					FRDGTextureRef BackTexture = RegisterExternalTexture(GraphBuilder, BackgroundRenderTexture_RenderThread->GetRenderTargetTexture(), TEXT("OculusXRHMD_BackgroundRenderTexture"));

					const FIntRect SrcRect(0, 0, BackTexture->Desc.GetSize().X, BackTexture->Desc.GetSize().Y);
					const FIntRect DstRect(0, 0, BackBuffer->Desc.GetSize().X, BackBuffer->Desc.GetSize().Y);

					FXRCopyTextureOptions Options(FeatureLevel_RenderThread, ShaderPlatform_RenderThread);
					Options.SetDisplayMappingOptions(OculusXRHMD);

					AddXRCopyTexturePass(GraphBuilder, RDG_EVENT_NAME("OculusXR_Spectator_DirectComposition"), BackTexture, SrcRect, BackBuffer, DstRect, Options);
					return;
				}
			}
			FDefaultSpectatorScreenController::RenderSpectatorScreen_RenderThread(GraphBuilder, BackBuffer, SrcTexture, LayersTexture, WindowSize);
		}
	}

	void FSpectatorScreenController::AddSpectatorModePass(ESpectatorScreenMode SpectatorMode, FRDGBuilder& GraphBuilder, FRDGTextureRef TargetTexture,
		FRDGTextureRef EyeTexture, FRDGTextureRef OtherTexture, FVector2f WindowSize)
	{
		FXRCopyTextureOptions Options(FeatureLevel_RenderThread, ShaderPlatform_RenderThread);
		Options.SetDisplayMappingOptions(OculusXRHMD);

		switch (SpectatorMode)
		{
			case ESpectatorScreenMode::Undistorted:
			{
				FSettings* Settings = OculusXRHMD->GetSettings_RenderThread();
				FIntRect DestRect(0, 0, TargetTexture->Desc.GetSize().X / 2, TargetTexture->Desc.GetSize().Y);
				for (int i = 0; i < 2; ++i)
				{
					AddXRCopyTexturePass(GraphBuilder, RDG_EVENT_NAME("OculusXR_Spectator_Undistorted"), EyeTexture, Settings->EyeRenderViewport[i], TargetTexture, DestRect, Options);
					DestRect.Min.X += TargetTexture->Desc.GetSize().X / 2;
					DestRect.Max.X += TargetTexture->Desc.GetSize().X / 2;
				}
			}
			break;
			case ESpectatorScreenMode::Distorted:
			{
				FCustomPresent* CustomPresent = OculusXRHMD->GetCustomPresent_Internal();
				FTextureRHIRef MirrorTexture = CustomPresent->GetMirrorTexture();
				if (MirrorTexture)
				{
					FRDGTextureRef RDGMirrorTexture = RegisterExternalTexture(GraphBuilder, MirrorTexture, TEXT("OculusXRHMD_MirrorTexture"));
					FIntRect SrcRect(0, 0, MirrorTexture->GetSizeX(), MirrorTexture->GetSizeY());
					FIntRect DestRect(0, 0, TargetTexture->Desc.GetSize().X, TargetTexture->Desc.GetSize().Y);
					AddXRCopyTexturePass(GraphBuilder, RDG_EVENT_NAME("OculusXR_Spectator_Distorted"), RDGMirrorTexture, SrcRect, TargetTexture, DestRect, Options);
				}
			}
			break;
			case ESpectatorScreenMode::SingleEye:
			{
				FSettings* Settings = OculusXRHMD->GetSettings_RenderThread();
				const FIntRect SrcRect = Settings->EyeRenderViewport[0];
				const FIntRect DstRect(0, 0, TargetTexture->Desc.GetSize().X, TargetTexture->Desc.GetSize().Y);

				AddXRCopyTexturePass(GraphBuilder, RDG_EVENT_NAME("OculusXR_Spectator_SingleEye"), EyeTexture, SrcRect, TargetTexture, DstRect, Options);
			}
			break;
			default:
				FDefaultSpectatorScreenController::AddSpectatorModePass(SpectatorMode, GraphBuilder, TargetTexture, EyeTexture, OtherTexture, WindowSize);
				break;
		}
	}

#endif

} // namespace OculusXRHMD

#endif // OCULUS_HMD_SUPPORTED_PLATFORMS
