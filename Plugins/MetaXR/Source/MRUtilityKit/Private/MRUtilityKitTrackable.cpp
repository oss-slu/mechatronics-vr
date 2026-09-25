// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "MRUtilityKitTrackable.h"
#include "MRUtilityKit.h"
#include "Engine/World.h"

AMRUKTrackable::AMRUKTrackable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
}

void AMRUKTrackable::UpdateProperties(
	const FTransform& Transform,
	EMRUKTrackableType Type,
	bool bIsTracked,
	const FBox2d& Plane,
	TArray<FVector2D> Boundary2D,
	const FBox3d& Volume,
	EMRUKMarkerPayloadType MarkerPayloadType,
	FString PayloadString,
	TArray<uint8> PayloadBytes)
{
	TrackableType = Type;
	IsTracked = bIsTracked;
	PlaneBounds = Plane;
	PlaneBoundary2D = MoveTemp(Boundary2D);
	VolumeBounds = Volume;
	PayloadType = MarkerPayloadType;
	MarkerPayloadString = MoveTemp(PayloadString);
	MarkerPayloadBytes = MoveTemp(PayloadBytes);

	SetActorTransform(Transform, false, nullptr, ETeleportType::ResetPhysics);
}
