// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "MRUtilityKitSharedHelper.h"
#include "GameFramework/WorldSettings.h"

#include <XRTrackingSystemBase.h>

MRUKShared::MrukUuid ToMrukShared(const FOculusXRUUID& Uuid)
{
	MRUKShared::MrukUuid MrukUuid;
	memcpy(MrukUuid.data, Uuid.UUIDBytes, sizeof(MrukUuid.data));
	return MrukUuid;
}

FOculusXRUUID ToUnreal(const MRUKShared::MrukUuid& Uuid)
{
	FOculusXRUUID ConvertedUuid;
	memcpy(ConvertedUuid.UUIDBytes, Uuid.data, sizeof(ConvertedUuid.UUIDBytes));
	return ConvertedUuid;
}

FBox3d ToUnreal(const UWorld* World, const MRUKShared::MrukVolume& Volume)
{
	FVector Min{ -Volume.min.Z, Volume.min.X, Volume.min.Y };
	Min.X -= Volume.max.Z - Volume.min.Z;
	const FVector Size{ Volume.max.Z - Volume.min.Z, Volume.max.X - Volume.min.X, Volume.max.Y - Volume.min.Y };
	const FVector Max = Min + Size;

	const float WorldToMeters = World ? World->GetWorldSettings()->WorldToMeters : 100.0f;
	const FBox3d Box(Min * WorldToMeters, Max * WorldToMeters);
	return Box;
}

FBox2d ToUnreal(const UWorld* World, const MRUKShared::MrukPlane& Plane)
{
	const float WorldToMeters = World ? World->GetWorldSettings()->WorldToMeters : 100.0f;
	FBox2d Box(FVector2D{ Plane.x, Plane.y } * WorldToMeters, FVector2D{ Plane.x + Plane.width, Plane.y + Plane.height } * WorldToMeters);
	return Box;
}

TArray<FVector2D> ToUnreal(const UWorld* World, const FVector2f* const Boundary, uint32_t BoundaryCount)
{
	const float WorldToMeters = World ? World->GetWorldSettings()->WorldToMeters : 100.0f;
	TArray<FVector2D> ConvertedBoundary;
	ConvertedBoundary.SetNum(BoundaryCount);
	for (uint32_t i = 0; i < BoundaryCount; ++i)
	{
		ConvertedBoundary[i] = FVector2D{ Boundary[i] } * WorldToMeters;
	}
	return ConvertedBoundary;
}

FString ToUnreal(MRUKShared::MrukLabel Label)
{
	switch (Label)
	{
		case MRUKShared::MRUK_LABEL_FLOOR:
			return "FLOOR";
		case MRUKShared::MRUK_LABEL_CEILING:
			return "CEILING";
		case MRUKShared::MRUK_LABEL_WALL_FACE:
			return "WALL_FACE";
		case MRUKShared::MRUK_LABEL_TABLE:
			return "TABLE";
		case MRUKShared::MRUK_LABEL_COUCH:
			return "COUCH";
		case MRUKShared::MRUK_LABEL_DOOR_FRAME:
			return "DOOR_FRAME";
		case MRUKShared::MRUK_LABEL_WINDOW_FRAME:
			return "WINDOW_FRAME";
		case MRUKShared::MRUK_LABEL_OTHER:
			return "OTHER";
		case MRUKShared::MRUK_LABEL_STORAGE:
			return "STORAGE";
		case MRUKShared::MRUK_LABEL_BED:
			return "BED";
		case MRUKShared::MRUK_LABEL_SCREEN:
			return "SCREEN";
		case MRUKShared::MRUK_LABEL_LAMP:
			return "LAMP";
		case MRUKShared::MRUK_LABEL_PLANT:
			return "PLANT";
		case MRUKShared::MRUK_LABEL_WALL_ART:
			return "WALL_ART";
		case MRUKShared::MRUK_LABEL_SCENE_MESH:
			return "GLOBAL_MESH";
		case MRUKShared::MRUK_LABEL_INVISIBLE_WALL_FACE:
			return "INVISIBLE_WALL_FACE";
		case MRUKShared::MRUK_LABEL_UNKNOWN:
			return "UNKNOWN";
		case MRUKShared::MRUK_LABEL_INNER_WALL_FACE:
			return "INNER_WALL_FACE";
		case MRUKShared::MRUK_LABEL_TABLETOP:
			return "TABLETOP";
		case MRUKShared::MRUK_LABEL_SITTING_AREA:
			return "SITTING_AREA";
		case MRUKShared::MRUK_LABEL_SLEEPING_AREA:
			return "SLEEPING_AREA";
		case MRUKShared::MRUK_LABEL_STORAGE_TOP:
			return "STORAGE_TOP";
		default:
			return "UNKNOWN";
	}
}

uint32_t ToMrukSharedSurfaceTypes(int32 ComponentTypes)
{
	uint32_t SurfaceTypes = 0;

	if ((ComponentTypes & (int32)EMRUKComponentType::Plane) != 0)
	{
		SurfaceTypes |= MRUKShared::MRUK_SURFACE_TYPE_PLANE;
	}
	if ((ComponentTypes & (int32)EMRUKComponentType::Volume) != 0)
	{
		SurfaceTypes |= MRUKShared::MRUK_SURFACE_TYPE_VOLUME;
	}
	if ((ComponentTypes & (int32)EMRUKComponentType::Mesh) != 0)
	{
		SurfaceTypes |= MRUKShared::MRUK_SURFACE_TYPE_MESH;
	}

	return SurfaceTypes;
}

MRUKShared::MrukLabelFilter ToMrukShared(const FMRUKLabelFilter LabelFilter)
{
	MRUKShared::MrukLabelFilter Filter{};

	for (int i = 0; i < LabelFilter.IncludedLabels.Num(); ++i)
	{
		Filter.includedLabelsSet = true;
		Filter.includedLabels |= MRUKShared::GetInstance()->StringToMrukLabel(TCHAR_TO_ANSI(*LabelFilter.IncludedLabels[i]));
	}

	for (int i = 0; i < LabelFilter.ExcludedLabels.Num(); ++i)
	{
		Filter.includedLabelsSet = true;
		Filter.includedLabels &= ~MRUKShared::GetInstance()->StringToMrukLabel(TCHAR_TO_ANSI(*LabelFilter.ExcludedLabels[i]));
	}

	Filter.surfaceType = ToMrukSharedSurfaceTypes(LabelFilter.ComponentTypes);

	return Filter;
}

FVector UnitVectorToUnreal(const FVector3f& UnitVector)
{
	return FVector(-UnitVector.Z, UnitVector.X, UnitVector.Y);
}

FVector3f UnitVectorToMrukShared(const FVector& UnitVector)
{
	return FVector3f(UnitVector.Y, UnitVector.Z, -UnitVector.X);
}

FVector PositionToUnreal(const FVector3f& Position, float WorldToMeters)
{
	return UnitVectorToUnreal(Position) * WorldToMeters;
}

FVector3f PositionToMrukShared(const FVector& Position, float WorldToMeters)
{
	return UnitVectorToMrukShared(Position) / WorldToMeters;
}

FQuat ToUnreal(const MRUKShared::MrukQuatf& Q)
{
	return FQuat(-Q.z, Q.x, Q.y, -Q.w);
}

MRUKShared::MrukQuatf ToMrukShared(const FQuat& Q)
{
	return MRUKShared::MrukQuatf{ static_cast<float>(Q.Y), static_cast<float>(Q.Z), -static_cast<float>(Q.X), -static_cast<float>(Q.W) };
}

FTransform ToUnreal(const MRUKShared::MrukPosef& Pose, float WorldToMeters)
{
	return FTransform(ToUnreal(Pose.rotation), PositionToUnreal(Pose.position, WorldToMeters));
}

MRUKShared::MrukPosef ToMrukShared(const FTransform& Transform, float WorldToMeters)
{
	return MRUKShared::MrukPosef{ PositionToMrukShared(Transform.GetTranslation(), WorldToMeters), ToMrukShared(Transform.GetRotation()) };
}
