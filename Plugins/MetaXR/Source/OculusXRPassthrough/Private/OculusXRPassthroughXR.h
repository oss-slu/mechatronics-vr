// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

// Including the khronos openxr header here to override the one included from IOpenXREntensionPlugin, making sure we use the latest one.
#include "khronos/openxr/openxr.h"
#include "khronos/openxr/meta_openxr_preview/meta_passthrough_layer_resumed_event.h"
#include "IOpenXRExtensionPlugin.h"
#include "IStereoLayers.h"
#include "OculusXRPassthroughLayer.h"
#include "OculusXRPassthroughXR_DeletionQueue.h"
#include "SceneViewExtension.h"

#define LOCTEXT_NAMESPACE "OculusXRPassthrough"

class IOpenXRHMD;
class UMaterial;

namespace XRPassthrough
{
	struct FSettings
	{
		UMaterial* PokeAHoleMaterial;

		bool bPassthroughEnabled;

		bool bExtInvertedAlphaAvailable;
		bool bExtPassthroughAvailable;
		bool bExtTriangleMeshAvailable;
		bool bExtColorLutAvailable;
		bool bExtLayerResumedEventAvailable;
	};
	typedef TSharedPtr<FSettings, ESPMode::ThreadSafe> FSettingsPtr;

	class FPassthroughXRSceneViewExtension : public FHMDSceneViewExtension
	{
	public:
		FPassthroughXRSceneViewExtension(const FAutoRegister& AutoRegister);
		~FPassthroughXRSceneViewExtension();

		// ISceneViewExtension
		void SetupViewFamily(FSceneViewFamily& InViewFamily) override;
		void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {};
		void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {};
		void PostRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily) override;

	private:
		void InvertTextureAlpha_RenderThread(FRHICommandList& RHICmdList, FRHITexture* Texture, FRHITexture* TempTexture, const FIntRect& ViewportRect);

		FTextureRHIRef InvAlphaTexture;
		IRendererModule* RendererModule;

		bool bShouldInvertAlpha_RenderThread;
	};

	class FPassthroughXR : public IOpenXRExtensionPlugin
	{
	public:
		// IOculusXROpenXRHMDPlugin
		virtual void* OnWaitFrame(XrSession InSession, void* InNext) override;
		virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
		virtual bool GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
		virtual const void* OnCreateInstance(class IOpenXRHMDModule* InModule, const void* InNext) override;
		virtual const void* OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext) override;
		virtual void PostCreateSession(XrSession InSession) override;
		virtual void OnDestroySession(XrSession InSession) override;
#if UE_VERSION_OLDER_THAN(5, 6, 0)
		virtual const void* OnEndProjectionLayer(XrSession InSession, int32 InLayerIndex, const void* InNext, XrCompositionLayerFlags& OutFlags) override;
#else
		virtual const void* OnEndProjectionLayer_RHIThread(XrSession InSession, int32 InLayerIndex, const void* InNext, XrCompositionLayerFlags& OutFlags) override;
#endif

#if !UE_VERSION_OLDER_THAN(5, 6, 0)
		virtual void OnCreateLayer(uint32 LayerId) override;
		virtual void OnDestroyLayer(uint32 LayerId) override;
		virtual void OnSetLayerDesc(uint32 LayerId) override;
#endif
#if UE_VERSION_OLDER_THAN(5, 6, 0)
		virtual void UpdateCompositionLayers(XrSession InSession, TArray<XrCompositionLayerBaseHeaderType*>& Headers) override;
#else
		virtual void UpdateCompositionLayers_RHIThread(XrSession InSession, TArray<XrCompositionLayerBaseHeader*>& Headers) override;
#endif
		virtual void OnWorldTickEnd(UWorld* InWorld, ELevelTick InTickType, float InDeltaSeconds);

#if UE_VERSION_OLDER_THAN(5, 6, 0)
		virtual void OnBeginRendering_GameThread(XrSession InSession) override;
#else
		virtual void OnBeginRendering_GameThread(XrSession InSession, FSceneViewFamily& InViewFamily, TArrayView<const uint32> VisibleLayers) override;
#endif
		virtual void OnPostRender_RenderThread(FRDGBuilder& GraphBuilder);

		virtual void OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader) override;

	public:
		static TWeakPtr<FPassthroughXR> GetInstance();
		static bool IsPassthoughLayerDesc(const IStereoLayers::FLayerDesc& LayerDesc);
		FPassthroughXR();
		virtual ~FPassthroughXR();
		void RegisterAsOpenXRExtension();

		XrPassthroughFB GetPassthroughInstance() const
		{
			return PassthroughInstance;
		}

		FSettingsPtr GetSettings() const
		{
			return Settings;
		}

		OCULUSXRPASSTHROUGH_API bool IsPassthroughEnabled(void) const;

	private:
		FPassthroughLayerPtr CreateStereoLayerFromDesc(const IStereoLayers::FLayerDesc& LayerDesc) const;
		void ShutdownPassthrough(XrSession InSession);
		void InitializePassthrough(XrSession InSession);
		void Update_GameThread(XrSession InSession);

		TSharedPtr<FPassthroughXRSceneViewExtension> SceneViewExtension;

		TMap<uint32, FPassthroughLayerPtr> LayerMap;
		TArray<FPassthroughLayerPtr> Layers_RenderThread;
		TArray<FPassthroughLayerPtr> Layers_RHIThread;

		XRPassthrough::FDeferredDeletionQueue DeferredDeletion;

		bool bPassthroughInitialized;
		XrPassthroughFB PassthroughInstance;

		FSettingsPtr Settings;

		IOpenXRHMD* OpenXRHMD;

		float WorldToMetersScale;
		float WorldToMetersScale_RenderThread;
		FTransform TrackingToWorld;
		FTransform TrackingToWorld_RenderThread;

		FDelegateHandle WorldTickEndDelegateHandle;
	};

} // namespace XRPassthrough

#undef LOCTEXT_NAMESPACE
