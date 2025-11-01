// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "Generated/MRUtilityKitShared.h"
#include "MRUtilityKit.h"
#include "OculusXRAnchorTypes.h"
#include "OculusXRHMDRuntimeSettings.h"
#include "OculusXRAnchorTypes.h"

MRUKShared::MrukUuid ToMrukShared(const FOculusXRUUID& Uuid);

FOculusXRUUID ToUnreal(const MRUKShared::MrukUuid& Uuid);

FBox3d ToUnreal(const UWorld* World, const MRUKShared::MrukVolume& Volume);

FBox2d ToUnreal(const UWorld* World, const MRUKShared::MrukPlane& Plane);

TArray<FVector2D> ToUnreal(const UWorld* World, const FVector2f* const Boundary, uint32_t BoundaryCount);

FString ToUnreal(MRUKShared::MrukLabel Label);

uint32_t ToMrukSharedSurfaceTypes(int32 ComponentTypes);

MRUKShared::MrukLabelFilter ToMrukShared(const FMRUKLabelFilter LabelFilter);

FVector UnitVectorToUnreal(const FVector3f& UnitVector);

FVector3f UnitVectorToMrukShared(const FVector& UnitVector);

FVector PositionToUnreal(const FVector3f& Position, const float WorldToMeters);

FVector3f PositionToMrukShared(const FVector& Position, const float WorldToMeters);

FQuat ToUnreal(const MRUKShared::MrukQuatf& Q);

MRUKShared::MrukQuatf ToMrukShared(const FQuat& Q);

FTransform ToUnreal(const MRUKShared::MrukPosef& Pose, const float WorldToMeters);

MRUKShared::MrukPosef ToMrukShared(const FTransform& Transform, const float WorldToMeters);
