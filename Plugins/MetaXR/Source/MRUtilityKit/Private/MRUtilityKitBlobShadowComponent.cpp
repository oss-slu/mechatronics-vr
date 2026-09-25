// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "MRUtilityKitBlobShadowComponent.h"
#include "MRUtilityKitTelemetry.h"
#include "MRUtilityKit.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"

UMRUKBlobShadowComponent::UMRUKBlobShadowComponent()
{
	const ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneAsset(TEXT("/Engine/BasicShapes/Plane"));
	if (PlaneAsset.Succeeded())
	{
		SetStaticMesh(PlaneAsset.Object);
	}
	else
	{
		UE_LOG(LogMRUK, Log, TEXT("Blob shadow couldn't find plane mesh in /Engine/BasicShapes/Plane"));
	}

	const ConstructorHelpers::FObjectFinder<UMaterialInstance> BlobShadowMaterialAsset(TEXT("/OculusXR/Materials/MI_BlobShadow"));
	if (BlobShadowMaterialAsset.Succeeded())
	{
		SetMaterial(0, BlobShadowMaterialAsset.Object);
	}
	else
	{
		UE_LOG(LogMRUK, Log, TEXT("Blob shadow couldn't find blob shadow material in /OculusXR/Materials/MI_BlobShadow"));
	}

	// Prevent sorting issue with transparent ground
	SetTranslucentSortPriority(1);

	// We don't want any collision
	SetCollisionProfileName("NoCollision");

	// Need tick to be enabled
	SetComponentTickEnabled(true);
	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = true;
}

void UMRUKBlobShadowComponent::BeginPlay()
{
	Super::BeginPlay();

	OculusXRTelemetry::TScopedMarker<MRUKTelemetry::FLoadBlobShadowMarker> Event(static_cast<int>(GetTypeHash(this)));

	// Create dynamic material (for roundness and gradient settings)
	DynMaterial = CreateAndSetMaterialInstanceDynamic(0);

	// Since we're updating the component size and position every frame it's better to not be influenced by parent
	SetUsingAbsoluteLocation(true);
	SetUsingAbsoluteRotation(true);
	SetUsingAbsoluteScale(true);

	// Compute size and position once
	UpdatePlaneSizeAndPosition();
}

void UMRUKBlobShadowComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update component size and position every frame
	UpdatePlaneSizeAndPosition();
}

void UMRUKBlobShadowComponent::UpdatePlaneSizeAndPosition()
{
	FVector Origin;
	FVector2D Extent;
	double Yaw;
	ComputeOwner2DBounds(Origin, Extent, Yaw);

	// Additional extent
	Extent += FVector2D::UnitVector * ExtraExtent;
	// Plane mesh is 100x100, multiplying by 0.02f to match the correct size when scaling
	SetWorldScale3D(FVector(Extent * 0.02f, 1.f));
	SetWorldRotation(FRotator(0.f, Yaw, 0.f));

	// Sphere trace to the ground
	FHitResult OutHit;
	TArray<AActor*> ActorsToIgnore;
	if (AActor* Owner = GetOwner())
	{
		ActorsToIgnore.Add(Owner);
	}
	const bool bHasHit = UKismetSystemLibrary::SphereTraceSingle(
		this,
		Origin,
		Origin + FVector::DownVector * MaxVerticalDistance,
		Extent.Length() * 0.5f,
		TraceTypeQuery1,
		true,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		OutHit,
		true);
	float Opacity = 0.f;
	if (bHasHit)
	{
		// Make plane visible
		SetHiddenInGame(false);
		// Impact + some offset to avoid Z-fighting
		SetWorldLocation(OutHit.ImpactPoint + FVector::UpVector * 0.02f);
		// Set opacity based on distance to ground
		Opacity = FMath::GetMappedRangeValueClamped(
			FVector2D(MaxVerticalDistance - FadeDistance, MaxVerticalDistance),
			FVector2D(1.f, 0.f),
			OutHit.Distance);
	}
	else
	{
		// Hide plane
		SetHiddenInGame(true);
	}

	// Update material's parameters
	if (DynMaterial)
	{
		DynMaterial->SetScalarParameterValue("CornerWorldSize", FMath::Min(Extent.X, Extent.Y) * Roundness);
		DynMaterial->SetScalarParameterValue("Gradient", Gradient);
		DynMaterial->SetScalarParameterValue("GradientPower", GradientPower);
		DynMaterial->SetScalarParameterValue("Opacity", Opacity);
	}
	// In case DynMaterial doesn't exist (e.g. in editor), update values directly on the mesh
	else
	{
		SetScalarParameterValueOnMaterials("CornerWorldSize", FMath::Min(Extent.X, Extent.Y) * Roundness);
		SetScalarParameterValueOnMaterials("Gradient", Gradient);
		SetScalarParameterValueOnMaterials("GradientPower", GradientPower);
		SetScalarParameterValueOnMaterials("Opacity", Opacity);
	}
}

void UMRUKBlobShadowComponent::ComputeOwner2DBounds(FVector& OutOrigin, FVector2D& OutExtent, double& OutYaw) const
{
	const AActor* Actor = GetOwner();
	if (!Actor)
	{
		OutOrigin = FVector::ZeroVector;
		OutExtent = FVector2D::ZeroVector;
		OutYaw = 0.0;
		return;
	}

	// Calculate local space BoundingBox from all components, but keep yaw to have a correct 2D bounding box at the end
	FBox Box(ForceInit);
	const FRotator YawOnly = FRotator(0.f, Actor->GetActorRotation().Yaw, 0.f);
	const FTransform ActorToWorld = FTransform(YawOnly.Quaternion());
	const FTransform WorldToActor = ActorToWorld.Inverse();

	Actor->ForEachComponent<UPrimitiveComponent>(true, [&](const UPrimitiveComponent* InPrimComp) {
		// Ignore editor & blob shadow components
		if (InPrimComp->IsRegistered() && !InPrimComp->IsEditorOnly() && !InPrimComp->bUseAttachParentBound && !InPrimComp->IsA<UMRUKBlobShadowComponent>())
		{
			const FTransform ComponentToActor = InPrimComp->GetComponentTransform() * WorldToActor;
			Box += InPrimComp->CalcBounds(ComponentToActor).GetBox();
		}
	});

	const FTransform Transform = Actor->GetTransform();
	// Project 3D extent to 2D
	const FVector ProjectedExtent = FVector::VectorPlaneProject(Box.GetExtent(), FVector::UpVector);

	OutOrigin = ActorToWorld.TransformPosition(Box.GetCenter());
	OutExtent = FVector2D(ProjectedExtent);
	OutYaw = Transform.GetRotation().Rotator().Yaw;
}
