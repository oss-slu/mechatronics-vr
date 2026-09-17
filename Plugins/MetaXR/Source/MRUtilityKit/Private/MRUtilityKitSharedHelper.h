// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "Generated/MRUtilityKitShared.h"
#include "MRUtilityKit.h"
#include "OculusXRAnchorTypes.h"
#include "OculusXRHMDRuntimeSettings.h"

MRUKShared::Uuid ToMrukShared(const FOculusXRUUID& Uuid);

MRUKShared::SceneModel ToMrukShared(EMRUKSceneModel SceneModel);

EMRUKSceneModel ToUnreal(MRUKShared::SceneModel SceneModel);

FOculusXRUUID ToUnreal(const MRUKShared::Uuid& Uuid);

FBox3d ToUnreal(const UWorld* World, const MRUKShared::Volume& Volume);

FBox2d ToUnreal(const UWorld* World, const MRUKShared::Plane& Plane);

TArray<FVector2D> ToUnreal(const UWorld* World, const FVector2f* const Boundary, uint32_t BoundaryCount);

FString ToUnreal(MRUKShared::Label Label);

MRUKShared::Label LabelToMrukShared(const FString& Label);

uint32_t ToMrukSharedSurfaceTypes(int32 ComponentTypes);

MRUKShared::LabelFilter ToMrukShared(const FMRUKLabelFilter LabelFilter);

FVector UnitVectorToUnreal(const FVector3f& UnitVector);

FVector3f UnitVectorToMrukShared(const FVector& UnitVector);

FVector PositionToUnreal(const FVector3f& Position, const float WorldToMeters);

FVector3f PositionToMrukShared(const FVector& Position, const float WorldToMeters);

FQuat ToUnreal(const MRUKShared::Quatf& Q);

MRUKShared::Quatf ToMrukShared(const FQuat& Q);

FTransform ToUnreal(const MRUKShared::Posef& Pose, const float WorldToMeters);

MRUKShared::Posef ToMrukShared(const FTransform& Transform, const float WorldToMeters);
