// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

// Including the khronos openxr header here to override the one included from IOpenXREntensionPlugin, making sure we use the latest one.
#include "khronos/openxr/openxr.h"
#include "khronos/openxr/meta_openxr_preview/meta_passthrough_layer_resumed_event.h"
#include "IOpenXRExtensionPlugin.h"
#include "IOpenXRHMD.h"
#include "IStereoLayers.h"
#include "OculusXRPassthroughLayerShapes.h"
#include "OculusXRPassthroughMesh.h"
#include "SceneViewExtension.h"

#define LOCTEXT_NAMESPACE "OculusXRPassthrough"

class UProceduralMeshComponent;
class FOpenXRSwapchain;
class FOpenXRHMD;
class UMaterial;

namespace XRPassthrough
{
	struct FSettings
	{
		bool bPassthroughEnabled = false;

		bool bExtInvertedAlphaAvailable = false;
		bool bExtPassthroughAvailable = false;
		bool bExtTriangleMeshAvailable = false;
		bool bExtColorLutAvailable = false;
		bool bExtLayerResumedEventAvailable = false;
	};

	class FPassthroughXRSceneViewExtension : public FHMDSceneViewExtension
	{
	public:
		FPassthroughXRSceneViewExtension(const FAutoRegister& AutoRegister);
		virtual ~FPassthroughXRSceneViewExtension() override;

		// ISceneViewExtension
		virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {}
		virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {}
		virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {}
		virtual void PostRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily) override;
		virtual void PostRenderBasePassMobile_RenderThread(FRHICommandList& RHICmdList, FSceneView& InView) override;

	private:
		FTextureRHIRef InvAlphaTexture;
	};

	class FPassthroughXR : public IOpenXRExtensionPlugin
	{
#if defined(WITH_OCULUS_BRANCH) || !UE_VERSION_OLDER_THAN(5, 6, 0)
		using XrCompositionLayerBaseHeaderType = XrCompositionLayerBaseHeader;
#else
		// epic branch has member as const
		using XrCompositionLayerBaseHeaderType = const XrCompositionLayerBaseHeader;
#endif
	public:
		// IOculusXROpenXRHMDPlugin
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

#if UE_VERSION_OLDER_THAN(5, 6, 0)
		virtual void UpdateCompositionLayers(XrSession InSession, TArray<XrCompositionLayerBaseHeaderType*>& Headers) override;
#else
		virtual void UpdateCompositionLayers_RHIThread(XrSession InSession, TArray<XrCompositionLayerBaseHeader*>& Headers) override;
#endif

#if UE_VERSION_OLDER_THAN(5, 6, 0)
		virtual void OnSetupLayers_RenderThread(XrSession InSession, const TArray<uint32>& LayerIds) override;
#else
		virtual void OnBeginRendering_GameThread(XrSession InSession, FSceneViewFamily& InViewFamily, TArrayView<const uint32> VisibleLayers) override;
		virtual void OnBeginRendering_RenderThread(XrSession InSession, FRDGBuilder& GraphBuilder) override;
#endif

		virtual void OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader) override;

		static TWeakPtr<FPassthroughXR> GetInstance();

		FPassthroughXR();
		void RegisterAsOpenXRExtension();

		XrPassthroughFB GetPassthroughInstance() const
		{
			return PassthroughInstance;
		}

		const FSettings& GetSettings() const
		{
			return Settings;
		}

		void RenderPokeAHoleMeshes(FRHICommandList& RHICmdList, const FSceneView& InView);

		OCULUSXRPASSTHROUGH_API bool IsPassthroughEnabled() const;

	private:
		struct FRHIGeometryInstance
		{
			XrGeometryInstanceFB InstanceHandle = XR_NULL_HANDLE;
		};

		struct FLayerUserDefinedMesh
		{
			OculusXRHMD::FOculusPassthroughMeshRef SourceMesh;
			FRHIGeometryInstance* RHIInstanceHandle = nullptr;
		};

		struct FPassthroughLayer
		{
			uint32 LayerId;
			XrPassthroughLayerFB LayerHandle = XR_NULL_HANDLE;
			XrPassthroughLayerPurposeFB Purpose;
			TArray<FLayerUserDefinedMesh> Meshes;
		};

		struct FCreateGeometryInstance
		{
			FRHIGeometryInstance* GeometryInstance;
			XrPassthroughLayerFB LayerHandle;
			XrTriangleMeshFB MeshHandle;
		};

		struct FLateUpdateGeometryTransform
		{
			FRHIGeometryInstance* GeometryInstance;
			FTransform RelativeTransform;
		};

		struct FEdgeStyleUpdate
		{
			// TODO: We currently need to copy the entire color LUT from FEdgeStyleParameters to ensure that it lives onto the RHI thread.
			// Converting that data to a proper FRHIResource with a UObject wrapper would be a good fix, but until then this is the safe way.
			FEdgeStyleParameters Style;
			XrPassthroughLayerFB Layer;
		};

		struct FPokeAHoleResources
		{
			FBufferRHIRef VertexBuffer;
			FBufferRHIRef IndexBuffer;
			uint32 NumPrimitives;
			uint32 NumVertices;
		};

		struct FCreatePokeAHole
		{
			FPokeAHoleResources* Resources;
			OculusXRHMD::FOculusPassthroughMeshRef MeshData;
		};

		struct FRenderPokeAHole
		{
			FPokeAHoleResources* Resources;
			FTransform Transform;
		};

		struct FFrameDeferredActions
		{
			TArray<XrCompositionLayerPassthroughFB> Overlays;
			TArray<XrCompositionLayerPassthroughFB> Underlays;
			TArray<FEdgeStyleUpdate> StyleUpdates;
			TArray<FLateUpdateGeometryTransform> TransformsToUpdate;
			TArray<FCreateGeometryInstance> GeometryInstancesToCreate;
			TArray<FRHIGeometryInstance*> GeometryInstancesToDestroy;
			TArray<FCreatePokeAHole> PokeAHolesToCreate;
			TArray<FRenderPokeAHole> PokeAHolesToRender;
			TArray<FPokeAHoleResources*> PokeAHolesToDestroy;
			TArray<XrPassthroughLayerFB> LayersToDestroy;
			TArray<XrTriangleMeshFB> MeshesToDestroy;
		};

		struct FUserDefinedMesh
		{
			OculusXRHMD::FOculusPassthroughMeshRef SourceMesh;
			XrTriangleMeshFB Handle = XR_NULL_HANDLE;
			FPokeAHoleResources* PokeAHoleResources = nullptr;
		};

		void InitializePassthrough(XrSession InSession);
		void ShutdownPassthrough(XrSession InSession);

		void CreatePokeAHoleResources(FFrameDeferredActions& Frame, FUserDefinedMesh& Mesh);
		void DestroyPokeAHoleResources(FFrameDeferredActions& Frame, FUserDefinedMesh& Mesh);

		void UpdateLayerMesh(XrSession InSession, FFrameDeferredActions& Frame,
			const IStereoLayers::FLayerDesc& LayerDesc, const FPassthroughLayer& Layer,
			const FUserDefinedGeometryDesc& GeometryDesc,
			FLayerUserDefinedMesh& LayerMesh,
			FUserDefinedMesh& Mesh);
		void DestroyLayerMesh(FFrameDeferredActions& Frame, FLayerUserDefinedMesh& Mesh);

		void CreateLayer(XrSession InSession, FFrameDeferredActions& Frame, FPassthroughLayer& Layer, XrPassthroughLayerPurposeFB LayerPurpose);
		void DestroyLayer(FFrameDeferredActions& Frame, FPassthroughLayer& Layer);

		XrTriangleMeshFB CreateTriangleMesh(XrSession Session, OculusXRHMD::FOculusPassthroughMesh& SourceMesh);
		void DestroyTriangleMesh(FFrameDeferredActions& Frame, FUserDefinedMesh& Mesh);

		void UpdatePassthroughStyle_RHIThread(XrPassthroughLayerFB XrPassthroughLayer,
			const FEdgeStyleParameters& EdgeStyleParameters);

		TSharedPtr<FPassthroughXRSceneViewExtension> SceneViewExtension;

		TArray<FPassthroughLayer> Layers;
		TArray<FUserDefinedMesh> Meshes;

		bool bPassthroughInitialized;
		XrPassthroughFB PassthroughInstance;

		FSettings Settings;

		IOpenXRHMD* OpenXRHMD;

		TArray<FRenderPokeAHole> PokeAHolesToRender_RenderThread;

		TArray<XrCompositionLayerPassthroughFB> UnderlayLayers_RHIThread;
		TArray<XrCompositionLayerPassthroughFB> OverlayLayers_RHIThread;
		TArray<FLateUpdateGeometryTransform> TransformsToUpdate_RHIThread;
	};

} // namespace XRPassthrough

#undef LOCTEXT_NAMESPACE
