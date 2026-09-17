// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "GameFramework/Actor.h"
#include "OculusXRAnchorTypes.h"
#include "MRUtilityKit.h"

#include "MRUtilityKitTrackable.generated.h"

/**
 * Represents a trackable object in the Mixed Reality Utility Kit.
 * A trackable is a type of object that can be detected and tracked by the runtime,
 * such as keyboards and QR codes. Unlike scene anchors, trackables can move dynamically
 * and may become temporarily untracked if they cannot be seen by the device.
 *
 * Trackables are instantiated and managed by UMRUKSubsystem. When a new trackable is detected,
 * the subsystem creates an AMRUKTrackable actor and invokes the OnTrackableAdded event.
 */
UCLASS(ClassGroup = MRUtilityKit, meta = (DisplayName = "MR Utility Kit Trackable"))
class MRUTILITYKIT_API AMRUKTrackable : public AActor
{
	GENERATED_BODY()

public:
	/**
	 * The type of this trackable (e.g., Keyboard, QRCode)
	 */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "MR Utility Kit")
	EMRUKTrackableType TrackableType = EMRUKTrackableType::None;

	/**
	 * Whether this trackable is currently being tracked.
	 * A trackable may become temporarily untracked if it cannot be seen by the device.
	 */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "MR Utility Kit")
	bool IsTracked = false;

	/**
	 * If the trackable has a plane attached to it, this represents the bounds of that plane in
	 * local coordinate space.
	 */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "MR Utility Kit")
	FBox2D PlaneBounds{ ForceInit };

	/**
	 * If the trackable has a plane attached to it, this represents the boundary of it in
	 * local coordinate space.
	 */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "MR Utility Kit")
	TArray<FVector2D> PlaneBoundary2D;

	/**
	 * If the trackable has a volume attached to it, this represents the bounds of that volume in
	 * local coordinate space.
	 */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "MR Utility Kit")
	FBox VolumeBounds{ ForceInit };

	/**
	 * The type of payload the trackable carries.
	 */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "MR Utility Kit")
	EMRUKMarkerPayloadType PayloadType = EMRUKMarkerPayloadType::NonePayload;

	/**
	 * The marker's payload as a string.
	 * If this trackable is a marker (e.g., a QR Code) and its payload can be interpreted as a string,
	 * this property contains the decoded string. Otherwise, it is empty.
	 */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "MR Utility Kit")
	FString MarkerPayloadString;

	/**
	 * The marker's payload as raw bytes.
	 * If this trackable is a marker (e.g., a QR Code), this property contains the raw payload bytes.
	 * Otherwise, the array is empty.
	 */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "MR Utility Kit")
	TArray<uint8> MarkerPayloadBytes;

public:
	AMRUKTrackable(const FObjectInitializer& ObjectInitializer);

	void UpdateProperties(
		const FTransform& Transform,
		EMRUKTrackableType Type,
		bool bIsTracked,
		const FBox2d& Plane,
		TArray<FVector2D> Boundary2D,
		const FBox3d& Volume,
		EMRUKMarkerPayloadType MarkerPayloadType,
		FString PayloadString,
		TArray<uint8> PayloadBytes);
};
