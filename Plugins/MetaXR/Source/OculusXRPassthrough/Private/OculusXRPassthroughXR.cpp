// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "OculusXRPassthroughXR.h"

#include "CommonRenderResources.h"
#include "Engine/GameEngine.h"
#include "Engine/RendererSettings.h"
#include "IOpenXRHMDModule.h"
#include "IOpenXRHMD.h"
#include "IXRTrackingSystem.h"
#include "Materials/Material.h"
#include "OculusXRHMDRuntimeSettings.h"
#include "OculusXRHMD_CustomPresent.h"
#include "OculusXRPassthroughXRFunctions.h"
#include "OculusXRPassthroughModule.h"
#include "OculusXRPassthroughEventHandling.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "Math/DoubleFloat.h"
#include "RHIStaticStates.h"
#if !UE_VERSION_OLDER_THAN(5, 5, 0)
#include "RHIResourceUtils.h"
#endif

#define LOCTEXT_NAMESPACE "OculusXRPassthrough"

namespace XRPassthrough
{
	FPassthroughXRSceneViewExtension::FPassthroughXRSceneViewExtension(const FAutoRegister& AutoRegister)
		: FHMDSceneViewExtension(AutoRegister)
		, InvAlphaTexture(nullptr)
	{
	}

	FPassthroughXRSceneViewExtension::~FPassthroughXRSceneViewExtension()
	{
		ENQUEUE_RENDER_COMMAND(FPassthroughXRSceneViewExtension_dtor)([Tex = MoveTemp(InvAlphaTexture)](FRHICommandListImmediate& RHICmdList) mutable {
			Tex.SafeRelease();
		});
	}

	void FPassthroughXRSceneViewExtension::PostRenderViewFamily_RenderThread(FRDGBuilder& GraphBuilder, FSceneViewFamily& InViewFamily)
	{
		TSharedPtr<FPassthroughXR> Instance = FPassthroughXR::GetInstance().Pin();
		check(Instance);

		FRHITexture* TargetTexture = InViewFamily.RenderTarget->GetRenderTargetTexture();
		if (!TargetTexture)
		{
			return;
		}

		if (!Instance->GetSettings().bExtInvertedAlphaAvailable && InvAlphaTexture == nullptr)
		{
#if UE_VERSION_OLDER_THAN(5, 5, 0)
			const auto CVarPropagateAlpha = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.PostProcessing.PropagateAlpha"));
			const bool bPropagateAlpha = EAlphaChannelMode::FromInt(CVarPropagateAlpha->GetValueOnRenderThread()) == EAlphaChannelMode::AllowThroughTonemapper;
#else
			const auto CVarPropagateAlpha = IConsoleManager::Get().FindConsoleVariable(TEXT("r.PostProcessing.PropagateAlpha"));
			const bool bPropagateAlpha = CVarPropagateAlpha->GetBool();
#endif
			if (bPropagateAlpha)
			{
				const FRHITextureDesc TextureDesc = TargetTexture->GetDesc();
				const uint32 SizeX = TextureDesc.GetSize().X;
				const uint32 SizeY = TextureDesc.GetSize().Y;
				const EPixelFormat ColorFormat = TextureDesc.Format;
				const uint32 NumMips = TextureDesc.NumMips;
				const uint32 NumSamples = TextureDesc.NumSamples;

				const FClearValueBinding ColorTextureBinding = FClearValueBinding::Black;

				const ETextureCreateFlags InvTextureCreateFlags = TexCreate_ShaderResource | TexCreate_RenderTargetable;
				FRHITextureCreateDesc InvTextureDesc{};
				if (TargetTexture->GetTexture2DArray() != nullptr)
				{
					InvTextureDesc = FRHITextureCreateDesc::Create2DArray(TEXT("InvAlphaTexture"))
										 .SetArraySize(2)
										 .SetExtent(SizeX, SizeY)
										 .SetFormat(ColorFormat)
										 .SetNumMips(NumMips)
										 .SetNumSamples(NumSamples)
										 .SetFlags(InvTextureCreateFlags | TexCreate_TargetArraySlicesIndependently)
										 .SetClearValue(ColorTextureBinding);
				}
				else
				{
					InvTextureDesc = FRHITextureCreateDesc::Create2D(TEXT("InvAlphaTexture"))
										 .SetExtent(SizeX, SizeY)
										 .SetFormat(ColorFormat)
										 .SetNumMips(NumMips)
										 .SetNumSamples(NumSamples)
										 .SetFlags(InvTextureCreateFlags)
										 .SetClearValue(ColorTextureBinding);
				}
				InvAlphaTexture = RHICreateTexture(InvTextureDesc);
			}
		}

		if (InvAlphaTexture)
		{
			const FRHITextureDesc TextureDesc = TargetTexture->GetDesc();
			FIntRect TextureRect = FIntRect(0, 0, TextureDesc.GetSize().X, TextureDesc.GetSize().Y);
			FRDGTextureRef TargetRDG = RegisterExternalTexture(GraphBuilder, TargetTexture, TEXT("OculusXRPassthrough_ColorSwapChainTexture"));
			FRDGTextureRef InvAlphaRDG = RegisterExternalTexture(GraphBuilder, InvAlphaTexture, TEXT("OculusXRPassthrough_InvAlphaTexture"));
			OculusXRHMD::FCustomPresent::AddInvertTextureAlphaPass(GraphBuilder, TargetRDG, InvAlphaRDG, TextureRect, InViewFamily.GetFeatureLevel(), InViewFamily.GetShaderPlatform());
		}
	}

	void FPassthroughXRSceneViewExtension::PostRenderBasePassMobile_RenderThread(FRHICommandList& RHICmdList, FSceneView& InView)
	{
		TSharedPtr<FPassthroughXR> Passthrough = FPassthroughXR::GetInstance().Pin();
		if (Passthrough.IsValid())
		{
			Passthrough->RenderPokeAHoleMeshes(RHICmdList, InView);
		}
	}

	TWeakPtr<FPassthroughXR> FPassthroughXR::GetInstance()
	{
		return FOculusXRPassthroughModule::Get().GetPassthroughExtensionPlugin();
	}

	FPassthroughXR::FPassthroughXR()
		: SceneViewExtension(nullptr)
		, bPassthroughInitialized(false)
		, PassthroughInstance{ XR_NULL_HANDLE }
		, OpenXRHMD(nullptr)
	{
	}

	void FPassthroughXR::RegisterAsOpenXRExtension()
	{
		// Feature not enabled on Marketplace build. Currently only for the meta fork
		RegisterOpenXRExtensionModularFeature();
	}

	bool FPassthroughXR::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
	{
		OutExtensions.Add(XR_FB_PASSTHROUGH_EXTENSION_NAME);
		OutExtensions.Add(XR_META_PASSTHROUGH_LAYER_RESUMED_EVENT_EXTENSION_NAME);
		return true;
	}

	bool FPassthroughXR::GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions)
	{
		OutExtensions.Add(XR_EXT_COMPOSITION_LAYER_INVERTED_ALPHA_EXTENSION_NAME);
		OutExtensions.Add(XR_FB_TRIANGLE_MESH_EXTENSION_NAME); // TODO this doesn't seem optional?
		OutExtensions.Add(XR_META_PASSTHROUGH_COLOR_LUT_EXTENSION_NAME);
		return true;
	}

	const void* FPassthroughXR::OnCreateInstance(class IOpenXRHMDModule* InModule, const void* InNext)
	{
		if (InModule != nullptr)
		{
			Settings.bExtInvertedAlphaAvailable = InModule->IsExtensionEnabled(XR_EXT_COMPOSITION_LAYER_INVERTED_ALPHA_EXTENSION_NAME);
			Settings.bExtPassthroughAvailable = InModule->IsExtensionEnabled(XR_FB_PASSTHROUGH_EXTENSION_NAME);
			Settings.bExtTriangleMeshAvailable = InModule->IsExtensionEnabled(XR_FB_TRIANGLE_MESH_EXTENSION_NAME);
			Settings.bExtColorLutAvailable = InModule->IsExtensionEnabled(XR_META_PASSTHROUGH_COLOR_LUT_EXTENSION_NAME);
			Settings.bExtLayerResumedEventAvailable = InModule->IsExtensionEnabled(XR_META_PASSTHROUGH_LAYER_RESUMED_EVENT_EXTENSION_NAME);
		}
		return InNext;
	}

	const void* FPassthroughXR::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext)
	{
		InitOpenXRFunctions(InInstance);

		OpenXRHMD = GEngine->XRSystem->GetIOpenXRHMD();

		const UOculusXRHMDRuntimeSettings* HMDSettings = GetDefault<UOculusXRHMDRuntimeSettings>();
		Settings.bPassthroughEnabled = HMDSettings->bInsightPassthroughEnabled;

		return InNext;
	}

	void FPassthroughXR::PostCreateSession(XrSession InSession)
	{
		if (Settings.bPassthroughEnabled)
		{
			InitializePassthrough(InSession);
		}

		SceneViewExtension = FSceneViewExtensions::NewExtension<FPassthroughXRSceneViewExtension>();
	}

	void FPassthroughXR::OnDestroySession(XrSession InSession)
	{
		// Release resources
		ShutdownPassthrough(InSession);

		FlushRenderingCommands();

		SceneViewExtension.Reset();
		OpenXRHMD = nullptr;
	}

	class FPokeAHoleVertexDeclaration : public FRenderResource
	{
	public:
		FVertexDeclarationRHIRef VertexDeclarationRHI;

		virtual void InitRHI(FRHICommandListBase& RHICmdList)
		{
			FVertexDeclarationElementList Elements;
			uint16 Stride = sizeof(FVector3f);
			Elements.Add(FVertexElement(0, STRUCT_OFFSET(FFilterVertex, Position), VET_Float3, 0, Stride));
			VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
		}

		virtual void ReleaseRHI()
		{
			VertexDeclarationRHI.SafeRelease();
		}
	};

	TGlobalResource<FPokeAHoleVertexDeclaration> GPokeAHoleVertexDeclaration;

	class FPokeAHoleVS : public FGlobalShader
	{
		DECLARE_GLOBAL_SHADER(FPokeAHoleVS)
		SHADER_USE_PARAMETER_STRUCT(FPokeAHoleVS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_ARRAY(FMatrix44f, Transform, [2])
		SHADER_PARAMETER_ARRAY(FVector4f, TransformPositionHigh, [2])
		END_SHADER_PARAMETER_STRUCT()

		static FParameters MakeParameters(const FTransform& ObjectTransform, const FSceneView& View)
		{
			FMatrix ObjectToWorld = ObjectTransform.ToMatrixWithScale();
			auto MakeDFMatrix = [&ObjectToWorld](const FSceneView& View) {
				FMatrix ObjectToClip = ObjectToWorld * View.ViewMatrices.GetViewProjectionMatrix();
				FVector3f Origin = (FVector3f)ObjectToClip.GetOrigin();
				return FDFInverseMatrix::MakeFromRelativeWorldMatrix(Origin, ObjectToClip);
				// return FDFInverseMatrix(FMatrix44f(ObjectToClip), FVector3f::Zero());
			};

			FDFInverseMatrix RelativeObjectToView = MakeDFMatrix(View);
			const FSceneView* InstancedView = View.GetInstancedSceneView();
			FDFInverseMatrix RelativeObjectToView2 = InstancedView ? MakeDFMatrix(*InstancedView) : RelativeObjectToView;

			FParameters Result;
			Result.Transform[0] = RelativeObjectToView.M;
			Result.TransformPositionHigh[0] = RelativeObjectToView.PreTranslation;
			Result.Transform[1] = RelativeObjectToView2.M;
			Result.TransformPositionHigh[1] = RelativeObjectToView2.PreTranslation;
			return Result;
		}
	};

	class FPokeAHolePS : public FGlobalShader
	{
		DECLARE_GLOBAL_SHADER(FPokeAHolePS)
		SHADER_USE_PARAMETER_STRUCT(FPokeAHolePS, FGlobalShader);
		using FParameters = FEmptyShaderParameters;
	};

	IMPLEMENT_GLOBAL_SHADER(FPokeAHoleVS, "/Plugin/OculusXR/Private/PokeAHole.usf", "MainVS", SF_Vertex)
	IMPLEMENT_GLOBAL_SHADER(FPokeAHolePS, "/Plugin/OculusXR/Private/PokeAHole.usf", "MainPS", SF_Pixel)

	void FPassthroughXR::RenderPokeAHoleMeshes(FRHICommandList& RHICmdList, const FSceneView& InView)
	{
		if (PokeAHolesToRender_RenderThread.Num() == 0)
		{
			return;
		}

		// Sort so that draws with the same vertex buffer are together
		PokeAHolesToRender_RenderThread.Sort([](const FRenderPokeAHole& A, const FRenderPokeAHole& B) -> bool {
			return reinterpret_cast<uintptr_t>(A.Resources->VertexBuffer.GetReference()) < reinterpret_cast<uintptr_t>(B.Resources->VertexBuffer.GetReference());
		});

		FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(InView.GetShaderPlatform());
		TShaderMapRef<FPokeAHoleVS> VertexShader(GlobalShaderMap);
		TShaderMapRef<FPokeAHolePS> PixelShader(GlobalShaderMap);

		FGraphicsPipelineStateInitializer PSO;
		RHICmdList.ApplyCachedRenderTargets(PSO);
		PSO.DepthStencilState = TStaticDepthStencilState<>::GetRHI();
		PSO.BlendState = TStaticBlendState<>::GetRHI();
		PSO.RasterizerState = TStaticRasterizerState<>::GetRHI();
		PSO.PrimitiveType = PT_TriangleList;
		PSO.BoundShaderState.VertexDeclarationRHI = GPokeAHoleVertexDeclaration.VertexDeclarationRHI;
		PSO.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
		PSO.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
		SetGraphicsPipelineState(RHICmdList, PSO, 0);

		const uint32 InstanceCount = InView.bIsInstancedStereoEnabled ? 2 : 1;

		FRHIBuffer* LastBuffer = nullptr;

		for (const FRenderPokeAHole& Render : PokeAHolesToRender_RenderThread)
		{
			if (Render.Resources->NumPrimitives > 0)
			{
				check(Render.Resources->IndexBuffer.IsValid());
				check(Render.Resources->VertexBuffer.IsValid());

				FPokeAHoleVS::FParameters Params = FPokeAHoleVS::MakeParameters(Render.Transform, InView);
				SetShaderParameters(RHICmdList, VertexShader, VertexShader.GetVertexShader(), Params);

				if (Render.Resources->VertexBuffer != LastBuffer)
				{
					RHICmdList.SetStreamSource(0, Render.Resources->VertexBuffer, 0);
					LastBuffer = Render.Resources->VertexBuffer;
				}
				RHICmdList.DrawIndexedPrimitive(Render.Resources->IndexBuffer, 0, 0, Render.Resources->NumVertices, 0, Render.Resources->NumPrimitives, InstanceCount);
			}
		}
	}

	bool FPassthroughXR::IsPassthroughEnabled() const
	{
		return Settings.bPassthroughEnabled;
	}

	// This function is for processing an array with a split. The first half of the array, 0..NextIndex,
	// are items which have been marked as in-use for the current frame. The items after NextIndex were
	// in use last frame but are not yet in use this frame. This function searches for an item in the unused
	// half for which the specified field matches the search target. If no item is found, a new item is
	// added. The matching or added item is then swapped with the item at NextIndex, and NextIndex is incremented.
	// A reference to this item, after the swap, is returned.
	template <typename T, typename S>
	static T& FindOrAddItemAndSwap(uint32 StartIndex, uint32& NumUsed, TArray<T>& Array, const S& SearchTarget, S T::* Field)
	{
		for (int32 Index = StartIndex; Index < Array.Num(); ++Index)
		{
			if (Array[Index].*Field == SearchTarget)
			{
				if (Index >= (int32)NumUsed)
				{
					Array.Swap(Index, NumUsed);
					Index = NumUsed;
					++NumUsed;
				}
				return Array[Index];
			}
		}

		uint32 NewIndex = Array.AddDefaulted();
		Array.Swap(NewIndex, NumUsed);
		T& Item = Array[NumUsed];
		++NumUsed;
		Item.*Field = SearchTarget;
		return Item;
	}

	template <typename T, typename S>
	static T& FindOrAddItemAndSwap(uint32& NextIndex, TArray<T>& Array, const S& SearchTarget, S T::* Field)
	{
		return FindOrAddItemAndSwap(NextIndex, NextIndex, Array, SearchTarget, Field);
	}

	static FMatrix TransformToPassthroughSpace(const FTransform& Transform, float WorldToMetersScale, const FTransform& TrackingToWorld)
	{
		const FVector WorldToMetersScaleInv = FVector(WorldToMetersScale).Reciprocal();
		FTransform TransformWorld = Transform * TrackingToWorld.Inverse();
		TransformWorld.MultiplyScale3D(WorldToMetersScaleInv);
		TransformWorld.ScaleTranslation(WorldToMetersScaleInv);
		const FMatrix TransformWorldScaled = TransformWorld.ToMatrixWithScale();

		const FMatrix SwapAxisMatrix(
			FPlane(0.0f, 0.0f, -1.0f, 0.0f),
			FPlane(1.0f, 0.0f, 0.0f, 0.0f),
			FPlane(0.0f, 1.0f, 0.0f, 0.0f),
			FPlane(0.0f, 0.0f, 0.0f, 1.0f));

		return TransformWorldScaled * SwapAxisMatrix;
	}

	// Code taken from OVRPlugin (InsightMrManager.cpp)
	static bool DecomposeTransformMatrix(const FMatrix& Transform, XrPosef& OutPose, XrVector3f& OutScale)
	{
		FTransform outTransform = FTransform(Transform);
		FVector3f scale = FVector3f(outTransform.GetScale3D());
		FQuat4f rotation = FQuat4f(outTransform.GetRotation());
		FVector3f position = FVector3f(outTransform.GetLocation());

		if (scale.X == 0 || scale.Y == 0 || scale.Z == 0)
		{
			return false;
		}

		OutScale = XrVector3f{ scale.X, scale.Y, scale.Z };
		OutPose = XrPosef{ XrQuaternionf{ rotation.X, rotation.Y, rotation.Z, rotation.W }, XrVector3f{ position.X, position.Y, position.Z } };

		return true;
	}

#if UE_VERSION_OLDER_THAN(5, 6, 0)
	void FPassthroughXR::OnSetupLayers_RenderThread(XrSession InSession, const TArray<uint32>& VisibleLayers)
	{
#else
	void FPassthroughXR::OnBeginRendering_GameThread(XrSession InSession, FSceneViewFamily& InViewFamily, TArrayView<const uint32> VisibleLayers)
	{
#endif
		{
			// Start or stop passthrough if needed
			const bool bPassthroughEnabled = Settings.bPassthroughEnabled;

			if (bPassthroughEnabled && !bPassthroughInitialized)
			{
				InitializePassthrough(InSession);
			}

			if (!bPassthroughEnabled && bPassthroughInitialized)
			{
				ShutdownPassthrough(InSession);
			}

			if (!bPassthroughEnabled || PassthroughInstance == XR_NULL_HANDLE || !OpenXRHMD)
			{
				return;
			}

			// At this point, the layers are fixed and won't change until rendering is done.
			// Because we don't have any dirty bits, we essentially need to diff all of the
			// layer descs against the saved state from last frame, and generate a minimal
			// set of incremental updates. This is horrifying, but without significant API
			// changes it's the best we can do.

			// Many actions this frame will need to happen on the rendering or RHI threads.
			// We will make a list of actions to perform in this function, and forward that
			// list to the other threads via this struct.
			FFrameDeferredActions Frame;

			// We're also going to collect a list of layers to draw, with some extra information
			// for determining the draw order.
			struct FRenderedLayer
			{
				enum
				{
					Pass_Underlay = -2,
					Pass_PokeAHole = -1,
					Pass_Main = 0,
					Pass_Overlay = 1,
				};

				XrPassthroughLayerFB LayerHandle = XR_NULL_HANDLE;
				int32 Pass = Pass_Main;
				int32 Priority = 0;
				uint32 LayerId = 0;

				bool operator<(const FRenderedLayer& Other) const
				{
					// First order layers by pass
					if (Pass != Other.Pass)
					{
						return Pass < Other.Pass;
					}

					// Draw layers by ascending priority
					if (Priority != Other.Priority)
					{
						return Priority < Other.Priority;
					}

					// Draw layers by ascending id
					return LayerId < Other.LayerId;
				}
			};
			TArray<FRenderedLayer> LayersToSubmit;

			// Because we're diffing state, we need to delete any state which was used last
			// frame but is not needed this frame. These Used counters will track what state
			// has or has not been used. See FindOrAddItemAndSwap for the exact mechanism.
			uint32 UsedMeshes = 0;
			uint32 UsedLayers = 0;

			if (GEngine->StereoRenderingDevice.IsValid() && GEngine->StereoRenderingDevice->GetStereoLayers())
			{
				IStereoLayers* StereoLayers = GEngine->StereoRenderingDevice->GetStereoLayers();
				for (uint32 LayerId : VisibleLayers)
				{
#if UE_VERSION_OLDER_THAN(5, 6, 0)
					IStereoLayers::FLayerDesc LayerDesc;
					if (!StereoLayers->GetLayerDesc(LayerId, LayerDesc))
					{
						continue;
					}
#else
					const IStereoLayers::FLayerDesc* FoundDesc_ = StereoLayers->FindLayerDesc(LayerId);
					check(FoundDesc_);
					const IStereoLayers::FLayerDesc& LayerDesc = *FoundDesc_;
#endif

					const bool bIsReconstructed = LayerDesc.HasShape<FReconstructedLayer>();
					const bool bIsUserDefined = LayerDesc.HasShape<FUserDefinedLayer>();
					if (!bIsReconstructed && !bIsUserDefined)
						continue; // Not a passthrough layer

					// Find the record of this layer's resources from last frame, or create a new record
					FPassthroughLayer& Layer = FindOrAddItemAndSwap(UsedLayers, Layers, LayerDesc.Id, &FPassthroughLayer::LayerId);

					// Even though the layer ID matches, the layer desc may have been completely changed. We need to validate that all
					// assumptions from the last frame still hold, and remake state if that doesn't hold.
					XrPassthroughLayerPurposeFB LayerPurpose = bIsReconstructed ? XR_PASSTHROUGH_LAYER_PURPOSE_RECONSTRUCTION_FB : XR_PASSTHROUGH_LAYER_PURPOSE_PROJECTED_FB;
					if (Layer.LayerHandle == XR_NULL_HANDLE || Layer.Purpose != LayerPurpose)
					{
						CreateLayer(InSession, Frame, Layer, LayerPurpose);
					}
					if (Layer.LayerHandle == XR_NULL_HANDLE)
						continue;

					EOculusXRPassthroughLayerOrder PassthroughLayerOrder;
					if (bIsReconstructed)
					{
						const FReconstructedLayer& ReconstructedLayerProps = LayerDesc.GetShape<FReconstructedLayer>();
						Frame.StyleUpdates.Add({ ReconstructedLayerProps.EdgeStyleParameters, Layer.LayerHandle });
						check(Layer.Meshes.Num() == 0);

						PassthroughLayerOrder = ReconstructedLayerProps.PassthroughLayerOrder;
					}
					else
					{
						check(bIsUserDefined);

						const FUserDefinedLayer& UserDefinedLayerProps = LayerDesc.GetShape<FUserDefinedLayer>();
						Frame.StyleUpdates.Add({ UserDefinedLayerProps.EdgeStyleParameters, Layer.LayerHandle });

						PassthroughLayerOrder = UserDefinedLayerProps.PassthroughLayerOrder;

						uint32 UsedLayerMeshes = 0;
						for (const FUserDefinedGeometryDesc& GeometryDesc : UserDefinedLayerProps.UserGeometryList)
						{
							OculusXRHMD::FOculusPassthroughMeshRef GeomPassthroughMesh = GeometryDesc.PassthroughMesh;
							if (!GeomPassthroughMesh)
								continue;

							// Find the corresponding mesh from last frame
							FLayerUserDefinedMesh& LayerMesh = FindOrAddItemAndSwap(UsedLayerMeshes, Layer.Meshes, GeomPassthroughMesh, &FLayerUserDefinedMesh::SourceMesh);

							// Find the corresponding triangle mesh. Note that even if we don't use this mesh,
							// this call is necessary to mark the triangle mesh as still in-use.
							FUserDefinedMesh& Mesh = FindOrAddItemAndSwap(0, UsedMeshes, Meshes, GeomPassthroughMesh, &FUserDefinedMesh::SourceMesh);

							UpdateLayerMesh(InSession, Frame, LayerDesc, Layer, GeometryDesc, LayerMesh, Mesh);
						}

						// Delete any layer meshes that no longer exist
						for (int MeshIndex = UsedLayerMeshes; MeshIndex < Layer.Meshes.Num(); ++MeshIndex)
						{
							DestroyLayerMesh(Frame, Layer.Meshes[MeshIndex]);
						}
						Layer.Meshes.SetNum(UsedLayerMeshes);
					}

					int32 Pass = FRenderedLayer::Pass_Main;
					if (PassthroughLayerOrder == PassthroughLayerOrder_Underlay)
					{
						Pass = FRenderedLayer::Pass_Underlay;
					}
					else if (bIsUserDefined && (LayerDesc.Flags & IStereoLayers::LAYER_FLAG_SUPPORT_DEPTH) != 0)
					{
						Pass = FRenderedLayer::Pass_PokeAHole;
					}
					else if (PassthroughLayerOrder == PassthroughLayerOrder_Overlay)
					{
						Pass = FRenderedLayer::Pass_Overlay;
					}

					LayersToSubmit.Add({ Layer.LayerHandle, Pass, LayerDesc.Priority, LayerDesc.Id });
				}
			}

			// Delete any unused layers
			for (int Index = UsedLayers; Index < Layers.Num(); ++Index)
			{
				DestroyLayer(Frame, Layers[Index]);
			}
			Layers.SetNum(UsedLayers);

			// Delete any unused meshes
			for (int Index = UsedMeshes; Index < Meshes.Num(); ++Index)
			{
				DestroyTriangleMesh(Frame, Meshes[Index]);
				DestroyPokeAHoleResources(Frame, Meshes[Index]);
			}
			Meshes.SetNum(UsedMeshes);

			XrCompositionLayerPassthroughFB CompositionLayer;
			FMemory::Memset(&CompositionLayer, 0, sizeof(CompositionLayer));
			CompositionLayer.type = XR_TYPE_COMPOSITION_LAYER_PASSTHROUGH_FB;
			CompositionLayer.layerHandle = XR_NULL_HANDLE;
			CompositionLayer.flags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
			CompositionLayer.space = XR_NULL_HANDLE;

			LayersToSubmit.Sort();
			for (const FRenderedLayer& Layer : LayersToSubmit)
			{
				if (Layer.Pass < FRenderedLayer::Pass_Main)
				{
					CompositionLayer.layerHandle = Layer.LayerHandle;
					Frame.Underlays.Add(CompositionLayer);
				}
				else if (Layer.Pass > FRenderedLayer::Pass_Main)
				{
					CompositionLayer.layerHandle = Layer.LayerHandle;
					Frame.Overlays.Add(CompositionLayer);
				}
			}
			XrSpace Space = OpenXRHMD->GetTrackingSpace();

#if UE_VERSION_OLDER_THAN(5, 6, 0)
			FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();
#else
			ENQUEUE_RENDER_COMMAND(OculusXR_Passthrough_DeferredActions)
			([this, InSession, Frame = MoveTemp(Frame), Space](FRHICommandListImmediate& RHICmdList) mutable
#endif
			{
				for (FPokeAHoleResources* Poke : Frame.PokeAHolesToDestroy)
				{
					delete Poke;
				}

				for (const FCreatePokeAHole& Create : Frame.PokeAHolesToCreate)
				{
					check(Create.Resources);
					check(Create.MeshData.IsValid());
					Create.Resources->NumPrimitives = Create.MeshData->GetTriangles().Num() / 3;
					Create.Resources->NumVertices = Create.MeshData->GetVertices().Num();
#if UE_VERSION_OLDER_THAN(5, 5, 0)
					{
						TResourceArray<int32> Indices;
						Indices = TResourceArray<int32>::Super(Create.MeshData->GetTriangles());
						FRHIResourceCreateInfo IndexCreateInfo = FRHIResourceCreateInfo(TEXT("OculusXR_PokeAHole_IndexBuffer"), &Indices);
						Create.Resources->IndexBuffer = RHICmdList.CreateIndexBuffer(sizeof(int32), Indices.GetResourceDataSize(), BUF_Static, IndexCreateInfo);
					}
					{
						TResourceArray<FVector3f> Vertices;
						Vertices = TResourceArray<FVector3f>::Super(Create.MeshData->GetVertices());
						FRHIResourceCreateInfo VertexCreateInfo = FRHIResourceCreateInfo(TEXT("OculusXR_PokeAHole_IndexBuffer"), &Vertices);
						Create.Resources->VertexBuffer = RHICmdList.CreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, VertexCreateInfo);
					}
#else
					Create.Resources->IndexBuffer = UE::RHIResourceUtils::CreateIndexBufferFromArray<int32>(RHICmdList, TEXT("OculusXR_PokeAHole_IndexBuffer"),
						EBufferUsageFlags::Static, Create.MeshData->GetTriangles());
					Create.Resources->VertexBuffer = UE::RHIResourceUtils::CreateVertexBufferFromArray<FVector3f>(RHICmdList, TEXT("OculusXR_PokeAHole_VertexBuffer"),
						EBufferUsageFlags::Static, Create.MeshData->GetVertices());
#endif
				}

				PokeAHolesToRender_RenderThread = MoveTemp(Frame.PokeAHolesToRender);

				RHICmdList.EnqueueLambda([this, InSession, Space, Frame = MoveTemp(Frame)](FRHICommandListImmediate&) mutable {
					UnderlayLayers_RHIThread = MoveTemp(Frame.Underlays);
					OverlayLayers_RHIThread = MoveTemp(Frame.Overlays);
					TransformsToUpdate_RHIThread = MoveTemp(Frame.TransformsToUpdate);

					for (const FEdgeStyleUpdate& StyleUpdate : Frame.StyleUpdates)
					{
						UpdatePassthroughStyle_RHIThread(StyleUpdate.Layer, StyleUpdate.Style);
					}

					for (FRHIGeometryInstance* GeomInst : Frame.GeometryInstancesToDestroy)
					{
						if (GeomInst->InstanceHandle != XR_NULL_HANDLE)
						{
							if (XR_FAILED(xrDestroyGeometryInstanceFB(GeomInst->InstanceHandle)))
							{
								UE_LOG(LogOculusXRPassthrough, Error, TEXT("Failed removing passthrough surface from scene."));
							}
						}
						delete GeomInst;
					}

					for (XrPassthroughLayerFB LayerHandle : Frame.LayersToDestroy)
					{
						xrDestroyPassthroughLayerFB(LayerHandle);
					}

					for (XrTriangleMeshFB MeshHandle : Frame.MeshesToDestroy)
					{
						if (XR_FAILED(xrDestroyTriangleMeshFB.GetValue()(MeshHandle)))
						{
							UE_LOG(LogOculusXRPassthrough, Error, TEXT("Failed destroying passthrough surface mesh."));
						}
					}

					for (const FCreateGeometryInstance& Create : Frame.GeometryInstancesToCreate)
					{
						check(Create.GeometryInstance->InstanceHandle == XR_NULL_HANDLE);

						if (Create.MeshHandle != XR_NULL_HANDLE)
						{
							XrGeometryInstanceCreateInfoFB createInfo = { XR_TYPE_GEOMETRY_INSTANCE_CREATE_INFO_FB };
							// The pose and scale will be updated later after LateUpdate
							createInfo.pose.orientation = { 0, 0, 0, 1 };
							createInfo.pose.position = { 0, 0, 0 };
							createInfo.scale = { 1, 1, 1 };
							createInfo.layer = Create.LayerHandle;
							createInfo.mesh = Create.MeshHandle;
							createInfo.baseSpace = Space;

							XrGeometryInstanceFB InstanceHandle;
							if (XR_FAILED(xrCreateGeometryInstanceFB(InSession, &createInfo, &InstanceHandle)))
							{
								UE_LOG(LogOculusXRPassthrough, Error, TEXT("Failed adding passthrough mesh surface to scene."));
								continue;
							}

							Create.GeometryInstance->InstanceHandle = InstanceHandle;
						}
					}
				});
			}
#if !UE_VERSION_OLDER_THAN(5, 6, 0)
			);
#endif
		}
#if !UE_VERSION_OLDER_THAN(5, 6, 0)
	}

	void FPassthroughXR::OnBeginRendering_RenderThread(XrSession InSession, FRDGBuilder& GraphBuilder)
	{
#endif
		{
			check(IsInRenderingThread());

			TSharedPtr<IXRTrackingSystem> TrackingSystem = GEngine->XRSystem;
			if (!TrackingSystem.IsValid())
			{
				return;
			}

#if UE_VERSION_OLDER_THAN(5, 6, 0)
			FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();
#else
			AddPass(GraphBuilder, RDG_EVENT_NAME("OculusXRPassthrough_LateUpdateMeshTransforms"), [this, TrackingSystem = MoveTemp(TrackingSystem)](FRHICommandListImmediate& RHICmdList) mutable
#endif
			{
				// Grab the transforms and display time after LateUpdate
				FTransform TrackingToWorld = TrackingSystem->GetTrackingToWorldTransform();
				float WorldToMetersScale = TrackingSystem->GetWorldToMetersScale();
				XrSpace Space = OpenXRHMD->GetTrackingSpace();
				XrTime DisplayTime = OpenXRHMD->GetDisplayTime();
				RHICmdList.EnqueueLambda([this, TrackingToWorld, WorldToMetersScale, Space, DisplayTime](FRHICommandListImmediate&) {
					for (const FLateUpdateGeometryTransform& Update : TransformsToUpdate_RHIThread)
					{
						const FMatrix Transform = TransformToPassthroughSpace(Update.RelativeTransform, WorldToMetersScale, TrackingToWorld);

						XrGeometryInstanceTransformFB UpdateInfo = { XR_TYPE_GEOMETRY_INSTANCE_TRANSFORM_FB };
						bool result = DecomposeTransformMatrix(Transform, UpdateInfo.pose, UpdateInfo.scale);
						if (!result)
						{
							UE_LOG(LogOculusXRPassthrough, Error, TEXT("Failed decomposing the transform matrix."));
							return;
						}

						UpdateInfo.baseSpace = Space;
						UpdateInfo.time = DisplayTime;

						check(Update.GeometryInstance && Update.GeometryInstance->InstanceHandle != XR_NULL_HANDLE);
						if (XR_FAILED(xrGeometryInstanceSetTransformFB(Update.GeometryInstance->InstanceHandle, &UpdateInfo)))
						{
							UE_LOG(LogOculusXRPassthrough, Error, TEXT("Failed updating passthrough mesh surface transform."));
						}
					}
					TransformsToUpdate_RHIThread.Reset();
				});
			}
#if !UE_VERSION_OLDER_THAN(5, 6, 0)
			);
#endif
		}
	}

#if !UE_VERSION_OLDER_THAN(5, 6, 0)
	void FPassthroughXR::UpdateCompositionLayers_RHIThread(XrSession InSession, TArray<XrCompositionLayerBaseHeaderType*>& Headers)
#else
	void FPassthroughXR::UpdateCompositionLayers(XrSession InSession, TArray<XrCompositionLayerBaseHeaderType*>& Headers)
#endif
	{
		check(IsInRenderingThread() || IsInRHIThread());

		// Headers array already contains (at least) one layer which is the eye's layer.
		// Underlay/SupportDetph layers need to be inserted before that layer, ordered by priority.
		Headers.Reserve(Headers.Num() + UnderlayLayers_RHIThread.Num() + OverlayLayers_RHIThread.Num());
		Headers.InsertUninitialized(0, UnderlayLayers_RHIThread.Num());
		for (int UnderlayIndex = 0; UnderlayIndex < UnderlayLayers_RHIThread.Num(); ++UnderlayIndex)
		{
			Headers[UnderlayIndex] = reinterpret_cast<XrCompositionLayerBaseHeaderType*>(&UnderlayLayers_RHIThread[UnderlayIndex]);
		}
		int const OverlayBase = Headers.Num();
		Headers.AddUninitialized(OverlayLayers_RHIThread.Num());
		for (int OverlayIndex = 0; OverlayIndex < OverlayLayers_RHIThread.Num(); ++OverlayIndex)
		{
			Headers[OverlayBase + OverlayIndex] = reinterpret_cast<XrCompositionLayerBaseHeaderType*>(&OverlayLayers_RHIThread[OverlayIndex]);
		}
	}

	XrTriangleMeshFB FPassthroughXR::CreateTriangleMesh(XrSession Session, OculusXRHMD::FOculusPassthroughMesh& SourceMesh)
	{
		const TArray<int32>& Triangles = SourceMesh.GetTriangles();
		const TArray<FVector3f> Vertices = SourceMesh.GetVertices();

		XrTriangleMeshCreateInfoFB TriangleMeshInfo = { XR_TYPE_TRIANGLE_MESH_CREATE_INFO_FB };
		TriangleMeshInfo.flags = 0; // not mutable
		TriangleMeshInfo.triangleCount = Triangles.Num() / 3;
		static_assert(sizeof(uint32_t) == sizeof(Triangles[0]));
		TriangleMeshInfo.indexBuffer = reinterpret_cast<const uint32_t*>(Triangles.GetData());
		TriangleMeshInfo.vertexCount = Vertices.Num();
		static_assert(sizeof(XrVector3f) == sizeof(Vertices[0]));
		TriangleMeshInfo.vertexBuffer = reinterpret_cast<const XrVector3f*>(Vertices.GetData());
		TriangleMeshInfo.windingOrder = XR_WINDING_ORDER_UNKNOWN_FB;

		XrTriangleMeshFB MeshHandle = XR_NULL_HANDLE;
		if (XR_FAILED(xrCreateTriangleMeshFB.GetValue()(Session, &TriangleMeshInfo, &MeshHandle)))
		{
			UE_LOG(LogOculusXRPassthrough, Error, TEXT("Failed creating passthrough mesh surface."));
			return XR_NULL_HANDLE;
		}
		return MeshHandle;
	}

	void FPassthroughXR::DestroyTriangleMesh(FFrameDeferredActions& Frame, FUserDefinedMesh& Mesh)
	{
		Mesh.SourceMesh = nullptr;

		if (Mesh.Handle != XR_NULL_HANDLE)
		{
			Frame.MeshesToDestroy.Add(Mesh.Handle);
			Mesh.Handle = XR_NULL_HANDLE;
		}
	}

	void FPassthroughXR::CreateLayer(XrSession InSession, FFrameDeferredActions& Frame, FPassthroughLayer& Layer, XrPassthroughLayerPurposeFB LayerPurpose)
	{
		DestroyLayer(Frame, Layer);

		XrPassthroughLayerCreateInfoFB PassthroughLayerCreateInfo = { XR_TYPE_PASSTHROUGH_LAYER_CREATE_INFO_FB };
		PassthroughLayerCreateInfo.passthrough = PassthroughInstance;
		PassthroughLayerCreateInfo.purpose = LayerPurpose;

		// Because layers are only displayed when requested, we can create them asynchronously on the game thread.
		// They need to be cleaned up on the RHI thread.
		XrPassthroughLayerFB LayerHandle = XR_NULL_HANDLE;
		XrResult CreateLayerResult = xrCreatePassthroughLayerFB(InSession, &PassthroughLayerCreateInfo, &LayerHandle);
		if (!XR_SUCCEEDED(CreateLayerResult))
		{
			UE_LOG(LogOculusXRPassthrough, Warning, TEXT("Failed to create passthrough layer, error : %i"), CreateLayerResult);
			return;
		}

		// TODO is this safe to do here? Does this need to be deferred to the RHI thread?
		XrResult ResumeLayerResult = xrPassthroughLayerResumeFB(LayerHandle);
		if (!XR_SUCCEEDED(ResumeLayerResult))
		{
			UE_LOG(LogOculusXRPassthrough, Warning, TEXT("Failed to resume passthrough layer, error : %i"), ResumeLayerResult);
			xrDestroyPassthroughLayerFB(LayerHandle);
			return;
		}

		Layer.LayerHandle = LayerHandle;
		Layer.Purpose = LayerPurpose;
	}

	void FPassthroughXR::DestroyLayer(FFrameDeferredActions& Frame, FPassthroughLayer& Layer)
	{
		for (FLayerUserDefinedMesh& Mesh : Layer.Meshes)
		{
			DestroyLayerMesh(Frame, Mesh);
		}
		Layer.Meshes.Reset();

		if (Layer.LayerHandle != XR_NULL_HANDLE)
		{
			Frame.LayersToDestroy.Add(Layer.LayerHandle);
			Layer.LayerHandle = XR_NULL_HANDLE;
		}
	}

	void FPassthroughXR::UpdateLayerMesh(XrSession InSession, FFrameDeferredActions& Frame, const IStereoLayers::FLayerDesc& LayerDesc,
		const FPassthroughLayer& Layer, const FUserDefinedGeometryDesc& GeometryDesc, FLayerUserDefinedMesh& LayerMesh, FUserDefinedMesh& Mesh)
	{
		// If this is a new mesh, schedule deferred creation
		if (LayerMesh.RHIInstanceHandle == nullptr)
		{
			if (Mesh.Handle == XR_NULL_HANDLE)
			{
				Mesh.Handle = CreateTriangleMesh(InSession, *Mesh.SourceMesh.GetReference());
			}

			LayerMesh.RHIInstanceHandle = new FRHIGeometryInstance;
			Frame.GeometryInstancesToCreate.Add({ LayerMesh.RHIInstanceHandle, Layer.LayerHandle, Mesh.Handle });
		}

		// Schedule the transform to be updated after LateUpdate
		Frame.TransformsToUpdate.Add({ LayerMesh.RHIInstanceHandle, GeometryDesc.Transform });

		// Update the poke-a-hole actor.
		const bool bPassthroughSupportsDepth = (LayerDesc.Flags & IStereoLayers::LAYER_FLAG_SUPPORT_DEPTH) != 0;
		if (bPassthroughSupportsDepth)
		{
			if (!Mesh.PokeAHoleResources)
			{
				CreatePokeAHoleResources(Frame, Mesh);
			}

			if (Mesh.PokeAHoleResources)
			{
				Frame.PokeAHolesToRender.Add({ Mesh.PokeAHoleResources, GeometryDesc.Transform });
			}
		}
	}

	void FPassthroughXR::DestroyLayerMesh(FFrameDeferredActions& Frame, FLayerUserDefinedMesh& Mesh)
	{
		Mesh.SourceMesh = nullptr;

		if (Mesh.RHIInstanceHandle != nullptr)
		{
			Frame.GeometryInstancesToDestroy.Add(Mesh.RHIInstanceHandle);
			Mesh.RHIInstanceHandle = nullptr;
		}
	}

	void FPassthroughXR::CreatePokeAHoleResources(FFrameDeferredActions& Frame, FUserDefinedMesh& Mesh)
	{
		DestroyPokeAHoleResources(Frame, Mesh);

		check(Mesh.SourceMesh);
		Mesh.PokeAHoleResources = new FPokeAHoleResources;
		Frame.PokeAHolesToCreate.Add({ Mesh.PokeAHoleResources, Mesh.SourceMesh });
	}

	void FPassthroughXR::DestroyPokeAHoleResources(FFrameDeferredActions& Frame, FUserDefinedMesh& Mesh)
	{
		if (Mesh.PokeAHoleResources)
		{
			Frame.PokeAHolesToDestroy.Add(Mesh.PokeAHoleResources);
			Mesh.PokeAHoleResources = nullptr;
		}
	} // namespace XRPassthrough

	void FPassthroughXR::UpdatePassthroughStyle_RHIThread(XrPassthroughLayerFB XrPassthroughLayer, const FEdgeStyleParameters& EdgeStyleParameters)
	{
		XrPassthroughStyleFB Style = { XR_TYPE_PASSTHROUGH_STYLE_FB };

		Style.textureOpacityFactor = EdgeStyleParameters.TextureOpacityFactor;

		Style.edgeColor = { 0, 0, 0, 0 };
		if (EdgeStyleParameters.bEnableEdgeColor)
		{
			Style.edgeColor = {
				EdgeStyleParameters.EdgeColor.R,
				EdgeStyleParameters.EdgeColor.G,
				EdgeStyleParameters.EdgeColor.B,
				EdgeStyleParameters.EdgeColor.A
			};
		}

		/// Color map
		union AllColorMapDescriptors
		{
			XrPassthroughColorMapMonoToRgbaFB rgba;
			XrPassthroughColorMapMonoToMonoFB mono;
			XrPassthroughBrightnessContrastSaturationFB bcs;
			XrPassthroughColorMapLutMETA lut;
			XrPassthroughColorMapInterpolatedLutMETA interpLut;
		};
		AllColorMapDescriptors colorMap;
		if (Settings.bExtColorLutAvailable && EdgeStyleParameters.bEnableColorMap)
		{
			void* colorMapDataDestination = nullptr;
			unsigned int expectedColorMapDataSize = 0;
			switch (EdgeStyleParameters.ColorMapType)
			{
				case ColorMapType_None:
					break;
				case ColorMapType_GrayscaleToColor:
					colorMap.rgba = { XR_TYPE_PASSTHROUGH_COLOR_MAP_MONO_TO_RGBA_FB };
					expectedColorMapDataSize = sizeof(colorMap.rgba.textureColorMap);
					colorMapDataDestination = colorMap.rgba.textureColorMap;
					Style.next = &colorMap.rgba;
					break;
				case ColorMapType_Grayscale:
					colorMap.mono = { XR_TYPE_PASSTHROUGH_COLOR_MAP_MONO_TO_MONO_FB };
					expectedColorMapDataSize = sizeof(colorMap.mono.textureColorMap);
					colorMapDataDestination = colorMap.mono.textureColorMap;
					Style.next = &colorMap.mono;
					break;
				case ColorMapType_ColorAdjustment:
					colorMap.bcs = { XR_TYPE_PASSTHROUGH_BRIGHTNESS_CONTRAST_SATURATION_FB };
					expectedColorMapDataSize = 3 * sizeof(float);
					colorMapDataDestination = &colorMap.bcs.brightness;
					Style.next = &colorMap.bcs;
					break;
				case ColorMapType_ColorLut:
					colorMap.lut = { XR_TYPE_PASSTHROUGH_COLOR_MAP_LUT_META };
					colorMap.lut.colorLut = reinterpret_cast<const XrPassthroughColorLutMETA&>(EdgeStyleParameters.ColorLutDesc.ColorLuts[0]);
					colorMap.lut.weight = EdgeStyleParameters.ColorLutDesc.Weight;
					Style.next = &colorMap.lut;
					break;
				case ColorMapType_ColorLut_Interpolated:
					colorMap.interpLut = { XR_TYPE_PASSTHROUGH_COLOR_MAP_INTERPOLATED_LUT_META };
					colorMap.interpLut.sourceColorLut = reinterpret_cast<const XrPassthroughColorLutMETA&>(EdgeStyleParameters.ColorLutDesc.ColorLuts[0]);
					colorMap.interpLut.targetColorLut = reinterpret_cast<const XrPassthroughColorLutMETA&>(EdgeStyleParameters.ColorLutDesc.ColorLuts[1]);
					colorMap.interpLut.weight = EdgeStyleParameters.ColorLutDesc.Weight;
					Style.next = &colorMap.lut;
					break;
				default:
					UE_LOG(LogOculusXRPassthrough, Error, TEXT("Passthrough style has unexpected color map type: %i"), EdgeStyleParameters.ColorMapType);
					return;
			}

			// Validate color map data size and copy it over
			if (colorMapDataDestination != nullptr)
			{
				if (EdgeStyleParameters.ColorMapData.Num() != expectedColorMapDataSize)
				{
					UE_LOG(LogOculusXRPassthrough, Error,
						TEXT("Passthrough color map size for type %i is expected to be %i instead of %i"),
						EdgeStyleParameters.ColorMapType,
						expectedColorMapDataSize,
						EdgeStyleParameters.ColorMapData.Num());
					return;
				}

				uint8* ColorMapData = (uint8*)EdgeStyleParameters.ColorMapData.GetData();
				memcpy(colorMapDataDestination, ColorMapData, expectedColorMapDataSize);
			}
		}

		XrResult Result = xrPassthroughLayerSetStyleFB(XrPassthroughLayer, &Style);
		if (!XR_SUCCEEDED(Result))
		{
			UE_LOG(LogOculusXRPassthrough, Error, TEXT("Failed setting passthrough style, error : %i"), Result);
			return;
		}
	}

	void FPassthroughXR::OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader)
	{
		switch (InHeader->type)
		{
			case XR_TYPE_EVENT_DATA_PASSTHROUGH_LAYER_RESUMED_META:
				if (Settings.bExtLayerResumedEventAvailable)
				{
					const XrEventDataPassthroughLayerResumedMETA* LayerResumedEvent =
						reinterpret_cast<const XrEventDataPassthroughLayerResumedMETA*>(InHeader);

					for (auto& Layer : Layers)
					{
						if (Layer.LayerHandle == LayerResumedEvent->layer)
						{
							UE_LOG(LogOculusXRPassthrough, Log, TEXT("FOculusXRPassthroughEventHandling - Passthrough Layer #%d resumed"), Layer.LayerId);

							// Send event
							OculusXRPassthrough::FOculusXRPassthroughEventDelegates::OculusPassthroughLayerResumed.Broadcast(Layer.LayerId);
							break;
						}
					}
				}
				break;
		}
	}

#if UE_VERSION_OLDER_THAN(5, 6, 0)
	const void* FPassthroughXR::OnEndProjectionLayer(XrSession InSession, int32 InLayerIndex, const void* InNext, XrCompositionLayerFlags& OutFlags)
#else
	const void* FPassthroughXR::OnEndProjectionLayer_RHIThread(XrSession InSession, int32 InLayerIndex, const void* InNext, XrCompositionLayerFlags& OutFlags)
#endif
	{
		check(IsInRenderingThread() || IsInRHIThread());

		if (UnderlayLayers_RHIThread.Num() != 0)
		{
			OutFlags |= XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_UNPREMULTIPLIED_ALPHA_BIT;

			if (Settings.bExtInvertedAlphaAvailable)
			{
				OutFlags |= XR_COMPOSITION_LAYER_INVERTED_ALPHA_BIT_EXT;
			}
		}

		return InNext;
	}

	void FPassthroughXR::InitializePassthrough(XrSession InSession)
	{
		if (bPassthroughInitialized)
			return;

		bPassthroughInitialized = true;

		check(IsInGameThread());

		const XrPassthroughCreateInfoFB PassthroughCreateInfo = { XR_TYPE_PASSTHROUGH_CREATE_INFO_FB };

		XrResult CreatePassthroughResult = xrCreatePassthroughFB(InSession, &PassthroughCreateInfo, &PassthroughInstance);
		if (!XR_SUCCEEDED(CreatePassthroughResult))
		{
			UE_LOG(LogOculusXRPassthrough, Error, TEXT("xrCreatePassthroughFB failed, error : %i"), CreatePassthroughResult);
			return;
		}

		XrResult PassthroughStartResult = xrPassthroughStartFB(PassthroughInstance);
		if (!XR_SUCCEEDED(PassthroughStartResult))
		{
			UE_LOG(LogOculusXRPassthrough, Error, TEXT("xrPassthroughStartFB failed, error : %i"), PassthroughStartResult);
			return;
		}
	}

	void FPassthroughXR::ShutdownPassthrough(XrSession InSession)
	{
		if (!bPassthroughInitialized)
			return;

		bPassthroughInitialized = false;

		check(IsInGameThread());

		TArray<FPassthroughLayer> TmpLayers = MoveTemp(Layers);
		TArray<FUserDefinedMesh> TmpMeshes = MoveTemp(Meshes);
		XrPassthroughFB PassthroughHandle = PassthroughInstance;

		Layers.Reset();
		Meshes.Reset();
		PassthroughInstance = XR_NULL_HANDLE;

		for (FPassthroughLayer& Layer : TmpLayers)
		{
			for (FLayerUserDefinedMesh& LayerMesh : Layer.Meshes)
			{
				LayerMesh.SourceMesh = nullptr;
			}
		}
		for (FUserDefinedMesh& Mesh : TmpMeshes)
		{
			Mesh.SourceMesh = nullptr;
		}

		ENQUEUE_RENDER_COMMAND(OculusXRPassthrough_Shutdown)
		([this, ShutdownLayers = MoveTemp(TmpLayers), ShutdownMeshes = MoveTemp(TmpMeshes), PassthroughHandle](FRHICommandListImmediate& RHICmdList) mutable {
			PokeAHolesToRender_RenderThread.Reset();

			RHICmdList.EnqueueLambda([this, InnerLayers = MoveTemp(ShutdownLayers), InnerMeshes = MoveTemp(ShutdownMeshes), PassthroughHandle](FRHICommandListImmediate& RHICmdList) {
				UnderlayLayers_RHIThread.Reset();
				OverlayLayers_RHIThread.Reset();
				TransformsToUpdate_RHIThread.Reset();

				for (const FPassthroughLayer& Layer : InnerLayers)
				{
					for (const FLayerUserDefinedMesh& LayerMesh : Layer.Meshes)
					{
						if (LayerMesh.RHIInstanceHandle)
						{
							if (LayerMesh.RHIInstanceHandle->InstanceHandle != XR_NULL_HANDLE)
							{
								xrDestroyGeometryInstanceFB(LayerMesh.RHIInstanceHandle->InstanceHandle);
							}
							delete LayerMesh.RHIInstanceHandle;
						}
					}

					xrDestroyPassthroughLayerFB(Layer.LayerHandle);
				}

				for (const FUserDefinedMesh& Mesh : InnerMeshes)
				{
					xrDestroyTriangleMeshFB.GetValue()(Mesh.Handle);
					if (Mesh.PokeAHoleResources)
					{
						delete Mesh.PokeAHoleResources;
					}
				}

				if (PassthroughHandle != XR_NULL_HANDLE)
				{
					XrResult Result = xrDestroyPassthroughFB(PassthroughHandle);
					if (!XR_SUCCEEDED(Result))
					{
						UE_LOG(LogOculusXRPassthrough, Error, TEXT("xrDestroyPassthroughFB failed, error : %i"), Result);
					}
				}
			});
		});
	}
} // namespace XRPassthrough

#undef LOCTEXT_NAMESPACE
