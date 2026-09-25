// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "MRUtilityKitRoom.h"
#include "MRUtilityKitAnchor.h"
#include "MRUtilityKitSeatsComponent.h"
#include "MRUtilityKitSubsystem.h"
#include "MRUtilityKitBPLibrary.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/EngineVersionComparison.h"
#include "MRUtilityKitSharedHelper.h"
#include "OculusXRHMDRuntimeSettings.h"
#include "OculusXRAnchorBPFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "MRUtilityKitRoom"

namespace
{
	double GetSeamlessFactor(double Perimeter, double StepSize)
	{
		double RoundedPerimeter = FMath::RoundHalfFromZero(Perimeter / StepSize);
		if (RoundedPerimeter <= 0.0)
		{
			RoundedPerimeter = 1.0;
		}
		return Perimeter / RoundedPerimeter;
	}

	FBox2D GetBoundsFromBoxForSide(const EMRUKBoxSide Side, const FBox& Box)
	{
		switch (Side)
		{
			case EMRUKBoxSide::XPos:
			case EMRUKBoxSide::XNeg:
			{
				return FBox2D(FVector2D(Box.Min.Y, Box.Min.Z), FVector2D(Box.Max.Y, Box.Max.Z));
			}
			case EMRUKBoxSide::YPos:
			case EMRUKBoxSide::YNeg:
			{
				return FBox2D(FVector2D(Box.Min.X, Box.Min.Z), FVector2D(Box.Max.X, Box.Max.Z));
			}
			case EMRUKBoxSide::ZPos:
			case EMRUKBoxSide::ZNeg:
			{
				return FBox2D(FVector2D(Box.Min.X, Box.Min.Y), FVector2D(Box.Max.X, Box.Max.Y));
			}
		}
		return {};
	}

	FVector GetNormalBoxSide(const EMRUKBoxSide Side)
	{
		switch (Side)
		{
			case EMRUKBoxSide::XPos:
			{
				return FVector(1, 0, 0);
			}
			case EMRUKBoxSide::XNeg:
			{
				return FVector(-1, 0, 0);
			}
			case EMRUKBoxSide::YPos:
			{
				return FVector(0, 1, 0);
			}
			case EMRUKBoxSide::YNeg:
			{
				return FVector(0, -1, 0);
			}
			case EMRUKBoxSide::ZPos:
			{
				return FVector(0, 0, 1);
			}
			case EMRUKBoxSide::ZNeg:
			{
				return FVector(0, 0, -1);
			}
		}
		return {};
	}

	FVector GetWorldPos(const FVector2D Pos2D, const AMRUKAnchor* ParentAnchor, const EMRUKBoxSide Side)
	{
		FVector LocalPos = FVector::Zero();
		switch (Side)
		{
			case EMRUKBoxSide::XPos:
			{
				LocalPos = FVector(ParentAnchor->VolumeBounds.Max.X, Pos2D.X, Pos2D.Y);
				break;
			}
			case EMRUKBoxSide::XNeg:
			{
				LocalPos = FVector(ParentAnchor->VolumeBounds.Min.X, Pos2D.X, Pos2D.Y);
				break;
			}
			case EMRUKBoxSide::YPos:
			{
				LocalPos = FVector(Pos2D.X, ParentAnchor->VolumeBounds.Max.Y, Pos2D.Y);
				break;
			}
			case EMRUKBoxSide::YNeg:
			{
				LocalPos = FVector(Pos2D.X, ParentAnchor->VolumeBounds.Min.Y, Pos2D.Y);
				break;
			}
			case EMRUKBoxSide::ZPos:
			{
				LocalPos = FVector(Pos2D.X, Pos2D.Y, ParentAnchor->VolumeBounds.Max.Z);
				break;
			}
			case EMRUKBoxSide::ZNeg:
			{
				LocalPos = FVector(Pos2D.X, Pos2D.Y, ParentAnchor->VolumeBounds.Min.Z);
				break;
			}
		}
		return ParentAnchor->ActorToWorld().TransformPosition(LocalPos);
	}

	bool IsActorOrientationHorizontal(const AActor* Actor)
	{
		if (!Actor)
		{
			return false;
		}
		if (Actor->GetActorUpVector().Z >= UE_INV_SQRT_2) // walls, door or similar
		{
			return false;
		}
		if (FMath::Abs(Actor->GetActorUpVector().X) >= UE_INV_SQRT_2)
		{
			return true;
		}
		return false;
	}

	double CalculatePolygonArea(const TArray<FVector2D>& Points)
	{
		double Area = 0.0;

		// Calculate Area using the Shoelace formula
		for (int32 j = 0; j < Points.Num(); ++j)
		{
			const FVector2D& P1 = Points[j];
			const FVector2D& P2 = Points[(j + 1) % Points.Num()];
			Area += (P1.X * P2.Y - P2.X * P1.Y);
		}

		// Take the absolute value and divide by 2
		return FMath::Abs(Area) * 0.5;
	}
} // namespace

AMRUKRoom::AMRUKRoom(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create a scene component as root so we can attach spawned actors to it
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
}

void AMRUKRoom::EndPlay(EEndPlayReason::Type Reason)
{
	for (const auto& Anchor : AllAnchors)
	{
		OnAnchorRemoved.Broadcast(Anchor);
		Anchor->Destroy();
	}

	GetGameInstance()->GetSubsystem<UMRUKSubsystem>()->UnregisterRoom(this);
	Super::EndPlay(Reason);
}

AMRUKAnchor* AMRUKRoom::SpawnAnchor()
{
	FActorSpawnParameters SpawnParameters{};
	SpawnParameters.Owner = this;
	AMRUKAnchor* Anchor = GetWorld()->SpawnActor<AMRUKAnchor>(SpawnParameters);
	Anchor->Room = this;
	GetRootComponent()->SetMobility(EComponentMobility::Movable);
	Anchor->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	return Anchor;
}

void AMRUKRoom::AddAnchorToRoom(AMRUKAnchor* Anchor)
{
	const FString Semantics = FString::Join(Anchor->SemanticClassifications, TEXT("-"));
#if WITH_EDITOR
	if (Anchor->SemanticClassifications.Num() > 0)
	{
		Anchor->SetActorLabel(Semantics);
	}
#endif

	UE_LOG(LogMRUK, Log, TEXT("Add '%s' anchor '%s' to room '%s'"), *Semantics, *Anchor->AnchorUUID.ToString(), *AnchorUUID.ToString());

	if (Anchor->HasLabel(FMRUKLabels::Floor))
	{
		FloorAnchor_DEPRECATED = Anchor;
		FloorAnchors.Push(Anchor);
		RoomLayout.FloorUuid = Anchor->AnchorUUID;
	}
	if (Anchor->HasLabel(FMRUKLabels::Ceiling))
	{
		CeilingAnchor_DEPRECATED = Anchor;
		CeilingAnchors.Push(Anchor);
		RoomLayout.CeilingUuid = Anchor->AnchorUUID;
	}
	if (Anchor->HasLabel(FMRUKLabels::WallFace) || Anchor->HasLabel(FMRUKLabels::InvisibleWallFace) || Anchor->HasLabel(FMRUKLabels::InnerWallFace) || Anchor->HasLabel(FMRUKLabels::OtherRoomFace))
	{
		WallAnchors.Push(Anchor);
		RoomLayout.WallsUuid.Push(Anchor->AnchorUUID);
	}
	if (Anchor->HasLabel(FMRUKLabels::GlobalMesh))
	{
		GlobalMeshAnchor = Anchor;
	}
	if (Anchor->HasLabel(FMRUKLabels::Couch))
	{
		SeatAnchors.Push(Anchor);
	}

	AllAnchors.Push(Anchor);
}

AMRUKAnchor* AMRUKRoom::FindAnchorByUuid(const FOculusXRUUID& Uuid)
{
	for (AMRUKAnchor* Anchor : AllAnchors)
	{
		if (Anchor->AnchorUUID == Uuid)
		{
			return Anchor;
		}
	}
	return nullptr;
}

void AMRUKRoom::RemoveAnchor(AMRUKAnchor* Anchor)
{
	AllAnchors.Remove(Anchor);
	WallAnchors.Remove(Anchor);
	CeilingAnchors.Remove(Anchor);
	FloorAnchors.Remove(Anchor);
	if (CeilingAnchor_DEPRECATED == Anchor)
	{
		CeilingAnchor_DEPRECATED = nullptr;
	}
	if (FloorAnchor_DEPRECATED == Anchor)
	{
		FloorAnchor_DEPRECATED = nullptr;
	}
}

void AMRUKRoom::InitializeRoom()
{
	ComputeRoomBounds();
	ComputeAnchorHierarchy();
	ComputeSeats();
	ComputeRoomEdges();
	KeyWallAnchor = nullptr;
}

void AMRUKRoom::ComputeRoomBounds()
{
	RoomBounds.Init();

	auto AddBoundaryPointsToBounds = [this](const TArray<AMRUKAnchor*>& InAnchors) {
		for (AMRUKAnchor* Anchor : InAnchors)
		{
			if (IsValid(Anchor))
			{
				const FTransform Transform = Anchor->GetTransform();
				for (const FVector2D& Vertex : Anchor->PlaneBoundary2D)
				{
					const FVector Pos = Transform.TransformPosition(FVector(0.0f, Vertex.X, Vertex.Y));
					RoomBounds += Pos;
				}
			}
		}
	};

	AddBoundaryPointsToBounds(FloorAnchors);
	AddBoundaryPointsToBounds(CeilingAnchors);
}

void AMRUKRoom::ComputeAnchorHierarchy()
{
	// Reset anchor hierarchy
	for (auto& Anchor : AllAnchors)
	{
		Anchor->ParentAnchor = nullptr;
		Anchor->ChildAnchors.Empty();
	}

	constexpr float OffsetTolerance = 4.0f; // 4 cm offset allowed
	// Find things where are attached to walls such as doors, windows frames or wall art
	for (const auto& WallAnchor : WallAnchors)
	{
		if (!WallAnchor)
		{
			continue;
		}
		const FTransform& WallTransform = WallAnchor->GetTransform();
		const FVector WallNormal = WallTransform.GetUnitAxis(EAxis::X);
		for (const auto& ChildAnchor : AllAnchors)
		{
			// Don't parent walls to themselves
			if (ChildAnchor == WallAnchor)
			{
				continue;
			}
			const FTransform& ChildTransform = ChildAnchor->GetTransform();
			const FVector ChildNormal = ChildTransform.GetUnitAxis(EAxis::X);
			// Check that the two transforms face the same direction
			if (!FVector::Coincident(WallNormal, ChildNormal))
			{
				continue;
			}
			// Check that the position is close to the surface (they are a little bit offset
			// to prevent Z fighting so allow for that).
			FVector LocalPos = WallTransform.InverseTransformPosition(ChildTransform.GetLocation());
			if (FMath::Abs(LocalPos.X) > OffsetTolerance)
			{
				continue;
			}
			// Check that the anchor is within the wall boundary
			if (!WallAnchor->IsPositionInBoundary(FVector2D(LocalPos.Y, LocalPos.Z)))
			{
				continue;
			}
			// We have a match
			ensureMsgf(!ChildAnchor->ParentAnchor, TEXT("This anchor already has a parent"));
			ChildAnchor->ParentAnchor = WallAnchor;
			WallAnchor->ChildAnchors.Push(ChildAnchor);
		}
	}

	// Find volumes on the floor
	for (AMRUKAnchor* Floor : FloorAnchors)
	{
		const FTransform& FloorTransform = Floor->GetTransform();
		double FloorHeight = FloorTransform.GetLocation().Z;
		for (const auto& ChildAnchor : AllAnchors)
		{
			// Don't parent the floor to itself
			if (ChildAnchor == Floor)
			{
				continue;
			}
			const FTransform& ChildTransform = ChildAnchor->GetTransform();
			const FVector ChildXAxis = ChildTransform.GetUnitAxis(EAxis::X);
			const FBox& ChildVolumeBounds = ChildAnchor->VolumeBounds;
			// Only interested in scene volumes, the assumption is that all scene volumes have X axis pointing downwards
			if (!ChildVolumeBounds.IsValid || !FVector::Coincident(ChildXAxis, FVector::DownVector))
			{
				continue;
			}
			double ChildBottom = ChildTransform.GetLocation().Z - ChildVolumeBounds.Max.X;
			// Check that the volume is on the floor
			if (FMath::Abs(FloorHeight - ChildBottom) > OffsetTolerance)
			{
				continue;
			}
			FVector LocalPos = FloorTransform.InverseTransformPosition(ChildTransform.GetLocation());
			// Check that child anchor is within the bounds of the floor
			if (!Floor->IsPositionInBoundary(FVector2D(LocalPos.Y, LocalPos.Z)))
			{
				continue;
			}
			// We have a match
			ensureMsgf(!ChildAnchor->ParentAnchor, TEXT("This anchor already has a parent"));
			ChildAnchor->ParentAnchor = Floor;
			Floor->ChildAnchors.Push(ChildAnchor);
		}
	}

	// Find relationship between scene volumes
	for (const auto& ParentAnchor : AllAnchors)
	{
		if (!ParentAnchor)
		{
			continue;
		}
		const FTransform& ParentTransform = ParentAnchor->GetTransform();
		const FVector ParentXAxis = ParentTransform.GetUnitAxis(EAxis::X);
		const FBox& ParentVolumeBounds = ParentAnchor->VolumeBounds;
		// Only interested in scene volumes, the assumption is that all scene volumes have X axis pointing downwards
		if (!ParentVolumeBounds.IsValid || !FVector::Coincident(ParentXAxis, FVector::DownVector))
		{
			continue;
		}
		double ParentTop = ParentTransform.GetLocation().Z - ParentVolumeBounds.Min.X;
		for (const auto& ChildAnchor : AllAnchors)
		{
			// Don't parent anchors to themselves
			if (ChildAnchor == ParentAnchor)
			{
				continue;
			}
			const FTransform& ChildTransform = ChildAnchor->GetTransform();
			const FVector ChildXAxis = ChildTransform.GetUnitAxis(EAxis::X);
			const FBox& ChildVolumeBounds = ChildAnchor->VolumeBounds;
			// Only interested in scene volumes, the assumption is that all scene volumes have X axis pointing downwards
			if (!ChildVolumeBounds.IsValid || !FVector::Coincident(ChildXAxis, FVector::DownVector))
			{
				continue;
			}
			double ChildBottom = ChildTransform.GetLocation().Z - ChildVolumeBounds.Max.X;
			// Check that the two volumes are stack on top of each other
			if (FMath::Abs(ParentTop - ChildBottom) > OffsetTolerance)
			{
				continue;
			}
			// Check that at least one of the corners of the child volume is inside the bounds of the parent's volume
			// when projected onto the horizontal plane. This is to match the Scene Capture tool which requires the
			// user to defined stacked volumes by starting with one corner of the volume which must be on the parent's
			// volume.
			bool bAnyCornerInside = false;
			for (int i = 0; i < 4; ++i)
			{
				// Get a different corner on each iteration of the loop (height is not important here)
				FVector ChildLocalPos(0.0f, i < 2 ? ChildVolumeBounds.Min.Y : ChildVolumeBounds.Max.Y, i % 2 == 0 ? ChildVolumeBounds.Min.Z : ChildVolumeBounds.Max.Z);
				FVector LocalPos = ParentTransform.InverseTransformPosition(ChildTransform.TransformPosition(ChildLocalPos));
				// Check that child anchor is within the bounds of the parent on the horizontal plane
				if (LocalPos.Y >= ParentVolumeBounds.Min.Y && LocalPos.Y <= ParentVolumeBounds.Max.Y && LocalPos.Z >= ParentVolumeBounds.Min.Z && LocalPos.Z <= ParentVolumeBounds.Max.Z)
				{
					bAnyCornerInside = true;
					break;
				}
			}
			if (!bAnyCornerInside)
			{
				continue;
			}
			// We have a match
			ensureMsgf(!ChildAnchor->ParentAnchor, TEXT("This anchor already has a parent"));
			ChildAnchor->ParentAnchor = ParentAnchor;
			ParentAnchor->ChildAnchors.Push(ChildAnchor);
		}
	}
}

void AMRUKRoom::ComputeSeats()
{
	for (const auto& SeatAnchor : SeatAnchors)
	{
		if (SeatAnchor)
		{
			UMRUKSeatsComponent* SeatsComponent = SeatAnchor->FindComponentByClass<UMRUKSeatsComponent>();
			if (!SeatsComponent)
			{
				SeatsComponent = NewObject<UMRUKSeatsComponent>(SeatAnchor, TEXT("Seats"));
				SeatsComponent->RegisterComponent();
			}
			SeatsComponent->CalculateSeatPoses();
		}
	}
}

void AMRUKRoom::ComputeRoomEdges()
{
	if (FloorAnchors.IsEmpty())
	{
		UE_LOG(LogMRUK, Warning, TEXT("Floor anchor not set, can not compute room edges"));
		return;
	}

	if (FloorAnchors.Num() > 1)
	{
		// We do not support computing room edges with multiple floors.
		return;
	}

	RoomEdges_DEPRECATED.Empty();
	for (AMRUKAnchor* Anchor : FloorAnchors)
	{
		const TArray<FVector2D>& FloorBoundary = Anchor->PlaneBoundary2D;
		const FTransform& FloorTransform = Anchor->GetActorTransform();

		for (int i = 0; i < FloorBoundary.Num(); ++i)
		{
			const FVector2D& BoundaryPoint = FloorBoundary[i];
			FVector Edge = FVector(0.0, BoundaryPoint.X, BoundaryPoint.Y);
			Edge = FloorTransform.TransformPosition(Edge);
			Edge.Z = 0.0;
			RoomEdges_DEPRECATED.Push(Edge);
		}
	}
}

bool AMRUKRoom::IsPositionInRoom(const FVector& Position, bool TestVerticalBounds)
{
	const float WorldToMeters = GetWorld()->GetWorldSettings()->WorldToMeters;
	return MRUKShared::GetInstance()->IsPositionInRoom(ToMrukShared(AnchorUUID), PositionToMrukShared(Position, WorldToMeters), TestVerticalBounds);
}

bool AMRUKRoom::GenerateRandomPositionInRoom(FVector& OutPosition, float MinDistanceToSurface, bool AvoidVolumes)
{
	return GenerateRandomPositionInRoomFromStream(OutPosition, FRandomStream(NAME_None), MinDistanceToSurface, AvoidVolumes);
}

bool AMRUKRoom::GenerateRandomPositionInRoomFromStream(FVector& OutPosition, const FRandomStream& RandomStream, float MinDistanceToSurface, bool AvoidVolumes)
{
	if (FloorAnchors.IsEmpty())
	{
		return false;
	}
	if (MinDistanceToSurface > RoomBounds.GetExtent().GetMin())
	{
		// We can exit early here as we know it's not possible to generate a position in the room that satisfies
		// the MinDistanceToSurface requirement
		return false;
	}
	FVector Position;
	constexpr int MaxIterations = 2000;
	// Bail after MaxIteration tries to avoid infinite loop in case MinDistanceToSurface is too large
	// and we can't find a position which does not intersect with the walls and volumes
	for (int i = 0; i < MaxIterations; ++i)
	{
		Position.X = RandomStream.FRandRange(RoomBounds.Min.X + MinDistanceToSurface, RoomBounds.Max.X - MinDistanceToSurface);
		Position.Y = RandomStream.FRandRange(RoomBounds.Min.Y + MinDistanceToSurface, RoomBounds.Max.Y - MinDistanceToSurface);
		Position.Z = RandomStream.FRandRange(RoomBounds.Min.Z + MinDistanceToSurface, RoomBounds.Max.Z - MinDistanceToSurface);
		if (!IsPositionInRoom(Position))
		{
			// Reject points that are outside the room
			continue;
		}
		FVector SurfacePos;
		double SurfaceDistance;
		FMRUKLabelFilter Filter;
		Filter.IncludedLabels = { FMRUKLabels::WallFace };
		if (MinDistanceToSurface > 0.0f && TryGetClosestSurfacePosition(Position, SurfacePos, SurfaceDistance, Filter, MinDistanceToSurface))
		{
			// Reject points that are too close to the walls
			continue;
		}

		if (AvoidVolumes && IsPositionInSceneVolume(Position, true, MinDistanceToSurface))
		{
			// Reject points inside volumes if avoid volumes has been enabled
			continue;
		}
		OutPosition = Position;
		return true;
	}
	return false;
}

bool AMRUKRoom::GenerateRandomPositionOnSurface(EMRUKSpawnLocation SpawnLocation, float MinDistanceToEdge,
	FMRUKLabelFilter LabelFilter, FVector& OutPosition, FVector& OutNormal)
{
	TArray<Surface> Surfaces;
	float TotalUsableSurfaceArea = 0.0f;
	const float MinWidth = 2.0f * MinDistanceToEdge;

	OutPosition = FVector::ZeroVector;
	OutNormal = FVector::ForwardVector;
	for (auto& Anchor : AllAnchors)
	{
		if (!LabelFilter.PassesFilter(Anchor->SemanticClassifications))
		{
			continue;
		}
		if (Anchor->PlaneBounds.bIsValid)
		{
			bool bSkipPlane = false;

			const bool bIsHorizontal = IsActorOrientationHorizontal(Anchor);

			// We skip the plane if it's vertical and if we are not spawning for vertical surfaces
			if (SpawnLocation == EMRUKSpawnLocation::VerticalSurfaces)
			{
				bSkipPlane = bIsHorizontal;
			}
			// We skip the plane if it's not horizontal and if it's the ceiling
			else if (SpawnLocation == EMRUKSpawnLocation::OnTopOfSurface)
			{
				bSkipPlane = !bIsHorizontal;
				if (Anchor->SemanticClassifications.Contains(FMRUKLabels::Ceiling))
				{
					bSkipPlane = true;
				}
			}
			else if (SpawnLocation == EMRUKSpawnLocation::AnySurface)
			{
				bSkipPlane = false;
			}
			else if (SpawnLocation == EMRUKSpawnLocation::HangingDown)
			{
				bSkipPlane = !Anchor->SemanticClassifications.Contains(FMRUKLabels::Ceiling);
			}

			if (!bSkipPlane)
			{
				const FVector2D Size = Anchor->PlaneBounds.GetSize();
				if (Size.X > MinWidth && Size.Y > MinWidth)
				{
					const float UsableArea = (Size.X - MinWidth) * (Size.Y - MinWidth);
					TotalUsableSurfaceArea += UsableArea;
					Surfaces.Add({ Anchor, UsableArea, true, Anchor->PlaneBounds, EMRUKBoxSide{} });
				}
			}
		}

		if (Anchor->VolumeBounds.IsValid)
		{
			for (int FaceIndex = 0; FaceIndex < 6; ++FaceIndex)
			{
				const EMRUKBoxSide BoxSide = static_cast<EMRUKBoxSide>(FaceIndex);

				// Only top when spawning on top of surfaces. The negative X face corresponds to the top surface.
				if (SpawnLocation == EMRUKSpawnLocation::OnTopOfSurface && BoxSide != EMRUKBoxSide::XNeg)
				{
					continue;
				}

				// Switch top and bottom faces. The vertical surfaces are the Y and Z faces.
				if (SpawnLocation == EMRUKSpawnLocation::VerticalSurfaces && FaceIndex < 2)
				{
					continue;
				}

				// Only bottom when spawning on hanging down. The positive X face corresponds to the top surface.
				if (SpawnLocation == EMRUKSpawnLocation::HangingDown && BoxSide != EMRUKBoxSide::XPos)
				{
					continue;
				}

				FBox2D Bound = GetBoundsFromBoxForSide(BoxSide, Anchor->VolumeBounds);

				if (const FVector2D Size = Bound.GetSize(); Size.X > MinWidth && Size.Y > MinWidth)
				{
					const float UsableArea = (Size.X - MinWidth) * (Size.Y - MinWidth);
					TotalUsableSurfaceArea += UsableArea;
					Surfaces.Add({ Anchor, UsableArea, false, Bound, BoxSide });
				}
			}
		}
	}
	if (Surfaces.Num() == 0)
	{
		return false;
	}

	constexpr int MaxIterations = 1000;
	for (int i = 0; i < MaxIterations; ++i)
	{
		// Pick a random surface weighted by surface area (surfaces with a larger
		// area have more chance of being chosen)
		float Rand = FMath::RandRange(0.f, TotalUsableSurfaceArea);
		int Index = 0;
		for (; Index < Surfaces.Num() - 1; ++Index)
		{
			Rand -= Surfaces[Index].UsableArea;
			if (Rand <= 0.0f)
			{
				break;
			}
		}
		Surface& SelectedSurface = Surfaces[Index];
		AMRUKAnchor* Anchor = SelectedSurface.Anchor;
		const bool bIsPlane = SelectedSurface.bIsPlane;
		const FBox2D& Bounds = SelectedSurface.Bounds;
		const EMRUKBoxSide BoxSide = SelectedSurface.Side;

		FVector2D Pos = FVector2D(
			FMath::RandRange(Bounds.Min.X + MinDistanceToEdge, Bounds.Max.X - MinDistanceToEdge),
			FMath::RandRange(Bounds.Min.Y + MinDistanceToEdge, Bounds.Max.Y - MinDistanceToEdge));

		if (bIsPlane && !Anchor->IsPositionInBoundary(Pos))
		{
			continue;
		}

		if (bIsPlane)
		{
			const FVector Pos3DPlane = Anchor->ActorToWorld().TransformPosition(FVector(0.f, Pos.X, Pos.Y));
			OutPosition = Pos3DPlane;
			OutNormal = Anchor->ActorToWorld().TransformVector(FVector::BackwardVector);
			return true;
		}

		OutPosition = GetWorldPos(Pos, Anchor, BoxSide);
		OutNormal = Anchor->ActorToWorld().TransformVector(GetNormalBoxSide(BoxSide));
		return true;
	}
	return false;
}

AMRUKAnchor* AMRUKRoom::Raycast(const FVector& Origin, const FVector& Direction, float MaxDist, const FMRUKLabelFilter& LabelFilter, FMRUKHit& OutHit)
{
	const float WorldToMeters = GetWorld()->GetWorldSettings()->WorldToMeters;

	MRUKShared::Hit Hit{};

	if (MRUKShared::GetInstance()->RaycastRoom(ToMrukShared(AnchorUUID), PositionToMrukShared(Origin, WorldToMeters), UnitVectorToMrukShared(Direction), MaxDist / WorldToMeters, ToMrukShared(LabelFilter), &Hit))
	{
		AMRUKAnchor* Anchor = FindAnchorByUuid(ToUnreal(Hit.sceneAnchorUuid));
		if (Anchor)
		{
			OutHit.HitDistance = Hit.hitDistance * WorldToMeters;
			OutHit.HitNormal = -UnitVectorToUnreal(Hit.hitNormal);
			OutHit.HitPosition = PositionToUnreal(Hit.hitPosition, WorldToMeters);
			return Anchor;
		}
	}

	return nullptr;
}

bool AMRUKRoom::RaycastAll(const FVector& Origin, const FVector& Direction, float MaxDist, const FMRUKLabelFilter& LabelFilter, TArray<FMRUKHit>& OutHits, TArray<AMRUKAnchor*>& OutAnchors)
{
	const float WorldToMeters = GetWorld()->GetWorldSettings()->WorldToMeters;

	static constexpr uint32 MaxHitCount = 128;
	MRUKShared::Hit Hits[MaxHitCount];
	uint32 HitCount = MaxHitCount;

	if (MRUKShared::GetInstance()->RaycastRoomAll(ToMrukShared(AnchorUUID), PositionToMrukShared(Origin, WorldToMeters), UnitVectorToMrukShared(Direction), MaxDist / WorldToMeters, ToMrukShared(LabelFilter), Hits, &HitCount))
	{
		OutHits.Reserve(HitCount);
		OutAnchors.Reserve(HitCount);
		for (uint32 i = 0; i < HitCount; ++i)
		{
			const MRUKShared::Hit& Hit = Hits[i];
			AMRUKAnchor* Anchor = FindAnchorByUuid(ToUnreal(Hit.sceneAnchorUuid));
			if (Anchor)
			{
				OutAnchors.Push(Anchor);
				FMRUKHit OutHit;
				OutHit.HitDistance = Hit.hitDistance * WorldToMeters;
				OutHit.HitNormal = -UnitVectorToUnreal(Hit.hitNormal);
				OutHit.HitPosition = PositionToUnreal(Hit.hitPosition, WorldToMeters);
				OutHits.Push(OutHit);
			}
		}
		return true;
	}

	return false;
}

void AMRUKRoom::ClearRoom()
{
	RoomLayout = {};
	AnchorUUID = {};
	SpaceHandle = {};
	RoomBounds.Init();
	for (auto& Anchor : AllAnchors)
	{
		if (Anchor)
		{
			Anchor->Destroy();
		}
	}
	AllAnchors.Empty();
	WallAnchors.Empty();
	SeatAnchors.Empty();
	FloorAnchor_DEPRECATED = nullptr;
	FloorAnchors.Empty();
	CeilingAnchor_DEPRECATED = nullptr;
	CeilingAnchors.Empty();
	KeyWallAnchor = nullptr;
}

bool AMRUKRoom::DoesRoomHave(const TArray<FString>& Labels)
{
	if (Labels.IsEmpty())
	{
		return true;
	}

	TArray<FString> RemainingLabels = Labels;

	for (const auto& Anchor : AllAnchors)
	{
		for (const auto& AnchorLabel : Anchor->SemanticClassifications)
		{
			const auto AnchorLabelIndex = RemainingLabels.Find(AnchorLabel);
			if (AnchorLabelIndex != INDEX_NONE)
			{
				RemainingLabels.RemoveAt(AnchorLabelIndex);
				if (RemainingLabels.IsEmpty())
				{
					return true;
				}
			}
		}
	}

	return false;
}

AMRUKAnchor* AMRUKRoom::TryGetClosestSurfacePosition(const FVector& WorldPosition, FVector& OutSurfacePosition, double& OutSurfaceDistance, const FMRUKLabelFilter& LabelFilter, double MaxDistance)
{
	if (MaxDistance <= 0.0)
	{
		MaxDistance = DBL_MAX;
	}
	OutSurfacePosition = FVector::Zero();
	AMRUKAnchor* ClosestAnchor = nullptr;

	for (const auto& Anchor : AllAnchors)
	{
		if (!Anchor || !Anchor->PassesLabelFilter(LabelFilter))
		{
			continue;
		}

		FVector SurfacePos{};
		const double Distance = Anchor->GetClosestSurfacePosition(WorldPosition, SurfacePos);
		if (Distance < MaxDistance)
		{
			MaxDistance = Distance;
			OutSurfacePosition = SurfacePos;
			ClosestAnchor = Anchor;
		}
	}

	OutSurfaceDistance = MaxDistance;
	return ClosestAnchor;
}

AMRUKAnchor* AMRUKRoom::IsPositionInSceneVolume(const FVector& WorldPosition, bool TestVerticalBounds, double Tolerance)
{
	for (const auto& Anchor : AllAnchors)
	{
		if (!Anchor)
		{
			continue;
		}

		if (Anchor->IsPositionInVolumeBounds(WorldPosition, TestVerticalBounds, Tolerance))
		{
			return Anchor;
		}
	}

	return nullptr;
}

AMRUKAnchor* AMRUKRoom::TryGetClosestSeatPose(const FVector& RayOrigin, const FVector& RayDirection, FTransform& OutSeatTransform)
{
	FTransform ClosestPose{};
	AMRUKAnchor* ClosestAnchor = nullptr;
	double ClosestDot = DBL_MIN;

	for (const auto& SeatAnchor : SeatAnchors)
	{
		if (!SeatAnchor)
		{
			continue;
		}
		const UMRUKSeatsComponent* SeatsComponent = SeatAnchor->FindComponentByClass<UMRUKSeatsComponent>();
		if (!SeatsComponent)
		{
			continue;
		}

		for (const auto& SeatPose : SeatsComponent->SeatPoses)
		{
			const FVector VecToSeat = (SeatPose.GetLocation() - RayOrigin).GetSafeNormal();
			const double ThisDot = RayDirection.Dot(VecToSeat);
			if (ThisDot <= ClosestDot)
			{
				continue;
			}
			ClosestDot = ThisDot;
			ClosestPose = SeatPose;
			ClosestAnchor = SeatAnchor;
		}
	}

	OutSeatTransform = ClosestPose;
	return ClosestAnchor;
}

TArray<AMRUKAnchor*> AMRUKRoom::GetAnchorsByLabel(const FString& Label) const
{
	TArray<TObjectPtr<AMRUKAnchor>> Anchors;

	for (const auto& Anchor : AllAnchors)
	{
		if (Anchor && Anchor->HasLabel(Label))
		{
			Anchors.Push(Anchor);
		}
	}
	return Anchors;
}

AMRUKAnchor* AMRUKRoom::GetFirstAnchorByLabel(const FString& Label) const
{
	const TArray<AMRUKAnchor*> Anchors = GetAnchorsByLabel(Label);
	if (Anchors.IsEmpty())
	{
		return nullptr;
	}
	return Anchors[0];
}

AMRUKAnchor* AMRUKRoom::GetBestPoseFromRaycast(const FVector& RayOrigin, const FVector& RayDirection, double MaxDist, const FMRUKLabelFilter& LabelFilter, FTransform& OutPose, EMRUKPositioningMethod PositioningMethod)
{
	FTransform BestPose{};

	FMRUKHit Hit{};
	AMRUKAnchor* HitAnchor = Raycast(RayOrigin, RayDirection, MaxDist, LabelFilter, Hit);
	if (!HitAnchor)
	{
		return nullptr;
	}

	FVector PosePosition = Hit.HitPosition;
	FVector PoseUp = FVector::UpVector;
	// By default, use the surface normal for pose forward
	// Caution: Make sure all the cases of this being "up" are caught below
	FVector PoseForward = Hit.HitNormal;

	constexpr double ParallelTolerance = 0.999;

	if (!HitAnchor->VolumeBounds.IsValid && Hit.HitNormal.Dot(PoseUp) >= ParallelTolerance)
	{
		// HitNormal and PoseUp are parallel. E.g. Walls and floors.
		PoseForward = FVector{ RayOrigin.X - Hit.HitPosition.X, RayOrigin.Y - Hit.HitPosition.Y, 0.0 }.GetSafeNormal();
	}
	else if (HitAnchor->VolumeBounds.IsValid)
	{
		// This is a volume object, and the ray has hit the top surface
		if (Hit.HitNormal.Dot(FVector::UpVector) >= ParallelTolerance)
		{
			const FTransform& Transform = HitAnchor->GetActorTransform();

			switch (PositioningMethod)
			{
				case EMRUKPositioningMethod::Center:
				{
					const FVector HitLocalPos = Transform.InverseTransformPosition(Hit.HitPosition);

					double ShortestDistance = DBL_MAX;
					FVector Forward = FVector::ZeroVector;

					double Dist = FMath::Abs(HitLocalPos.Y - HitAnchor->VolumeBounds.Min.Y);
					if (Dist < ShortestDistance)
					{
						ShortestDistance = Dist;
						Forward = -HitAnchor->GetActorRightVector();
					}
					Dist = FMath::Abs(HitLocalPos.Y - HitAnchor->VolumeBounds.Max.Y);
					if (Dist < ShortestDistance)
					{
						ShortestDistance = Dist;
						Forward = HitAnchor->GetActorRightVector();
					}
					Dist = FMath::Abs(HitLocalPos.Z - HitAnchor->VolumeBounds.Min.Z);
					if (Dist < ShortestDistance)
					{
						ShortestDistance = Dist;
						Forward = -HitAnchor->GetActorUpVector();
					}
					Dist = FMath::Abs(HitLocalPos.Z - HitAnchor->VolumeBounds.Max.Z);
					if (Dist < ShortestDistance)
					{
						ShortestDistance = Dist;
						Forward = HitAnchor->GetActorUpVector();
					}

					PoseForward = Forward;
					PosePosition = Transform.TransformPosition(FVector::ZeroVector);
					break;
				}
				case EMRUKPositioningMethod::Edge:
				{
					const FVector HitLocalPos = Transform.InverseTransformPosition(Hit.HitPosition);

					double ShortestDistance = DBL_MAX;
					FVector PoseLocal = FVector::ZeroVector;

					double Dist = FMath::Abs(HitLocalPos.Y - HitAnchor->VolumeBounds.Min.Y);
					if (Dist < ShortestDistance)
					{
						ShortestDistance = Dist;
						PoseForward = -HitAnchor->GetActorRightVector();
						PoseLocal = { 0.0, HitAnchor->VolumeBounds.Min.Y, HitLocalPos.Z };
					}
					Dist = FMath::Abs(HitLocalPos.Y - HitAnchor->VolumeBounds.Max.Y);
					if (Dist < ShortestDistance)
					{
						ShortestDistance = Dist;
						PoseForward = HitAnchor->GetActorRightVector();
						PoseLocal = { 0.0, HitAnchor->VolumeBounds.Max.Y, HitLocalPos.Z };
					}
					Dist = FMath::Abs(HitLocalPos.Z - HitAnchor->VolumeBounds.Min.Z);
					if (Dist < ShortestDistance)
					{
						ShortestDistance = Dist;
						PoseForward = -HitAnchor->GetActorUpVector();
						PoseLocal = { 0.0, HitLocalPos.Y, HitAnchor->VolumeBounds.Min.Z };
					}
					Dist = FMath::Abs(HitLocalPos.Z - HitAnchor->VolumeBounds.Max.Z);
					if (Dist < ShortestDistance)
					{
						ShortestDistance = Dist;
						PoseForward = HitAnchor->GetActorUpVector();
						PoseLocal = { 0.0, HitLocalPos.Y, HitAnchor->VolumeBounds.Max.Z };
					}
					PosePosition = Transform.TransformPosition(PoseLocal);
					break;
				}
				default:
				{
					const FVector HitLocalPos = Transform.InverseTransformPosition(Hit.HitPosition);
					PosePosition = Transform.TransformPosition({ 0.0, HitLocalPos.Y, HitLocalPos.Z });
					PoseForward = FVector{ RayOrigin.X - Hit.HitPosition.X, RayOrigin.Y - Hit.HitPosition.Y, 0.0 }.GetSafeNormal();
					break;
				}
			}
		}
	}

	BestPose.SetLocation(PosePosition);
	BestPose.SetRotation(UKismetMathLibrary::MakeRotFromXZ(PoseForward, PoseUp).Quaternion());
	OutPose = BestPose;

	return HitAnchor;
}

AMRUKAnchor* AMRUKRoom::GetKeyWall(double Tolerance)
{
	if (KeyWallAnchor)
	{
		return KeyWallAnchor;
	}

	TArray<TObjectPtr<AMRUKAnchor>> SortedWalls = WallAnchors;
	SortedWalls.Sort([](const AMRUKAnchor& A, const AMRUKAnchor& B) { return A.PlaneBounds.GetExtent().X < B.PlaneBounds.GetExtent().X; });
	// Find the first one with no other walls behind it.
	// SortedWalls is sorted from shortest side to longest
	for (int i = SortedWalls.Num() - 1; i >= 0; --i)
	{
		AMRUKAnchor* WallAnchor = SortedWalls[i];

		bool bNoPointsBehind = true;

		// Loop through the other corners, making sure none is behind the wall in question
		for (const auto& RoomEdge : RoomEdges_DEPRECATED)
		{
			FVector VecToCorner = RoomEdge - WallAnchor->GetActorLocation();
			// Due to anchor precision, we use a tolerance value.
			// For example, an adjacent wall edge may be just behind the wall, leading to a false result
			VecToCorner -= WallAnchor->GetActorForwardVector() * Tolerance;

			bNoPointsBehind &= (-WallAnchor->GetActorForwardVector()).Dot(VecToCorner) >= 0.0;
			if (!bNoPointsBehind)
			{
				break;
			}
		}

		if (bNoPointsBehind)
		{
			KeyWallAnchor = WallAnchor;
			return WallAnchor;
		}
	}

	return nullptr;
}

AMRUKAnchor* AMRUKRoom::GetLargestSurface(const FString& Label)
{
	MRUKShared::LabelFilter LabelFilter{};
	LabelFilter.surfaceType = static_cast<uint32_t>(MRUKShared::SurfaceType::Plane) | static_cast<uint32_t>(MRUKShared::SurfaceType::Volume);
	LabelFilter.includedLabels = static_cast<uint32_t>(LabelToMrukShared(Label));
	LabelFilter.includedLabelsSet = true;
	MRUKShared::Uuid FoundUuid{};
	if (MRUKShared::GetInstance()->FindLargestSurface(ToMrukShared(AnchorUUID), LabelFilter, &FoundUuid))
	{
		return FindAnchorByUuid(ToUnreal(FoundUuid));
	}
	return nullptr;
}

void AMRUKRoom::AttachProceduralMeshToWalls(const TArray<FString>& CutHoleLabels, UMaterialInterface* ProceduralMaterial)
{
	AttachProceduralMeshToWalls({}, CutHoleLabels, ProceduralMaterial);
}

void AMRUKRoom::ComputeWallMeshUVAdjustments(const TArray<FMRUKTexCoordModes>& WallTextureCoordinateModes, TArray<FMRUKAnchorWithPlaneUVs>& OutAnchorsWithPlaneUVs)
{
	TArray<TObjectPtr<AMRUKAnchor>> ConnectedWalls = ComputeConnectedWalls();
	double Perimeter = 0.0;
	for (const auto& WallAnchor : ConnectedWalls)
	{
		Perimeter += WallAnchor->PlaneBounds.GetSize().X;
	}
	const float WorldToMeters = GetWorldSettings()->WorldToMeters;
	const double WallHeight = RoomBounds.GetSize().Z;
	const double SeamlessWorldToMeters = GetSeamlessFactor(Perimeter, WorldToMeters);
	double UOffset = 0.0;
	const TArray<FMRUKTexCoordModes>& TexCoordModes = WallTextureCoordinateModes.IsEmpty() ? TArray<FMRUKTexCoordModes>{ FMRUKTexCoordModes{} } : WallTextureCoordinateModes;
	for (AMRUKAnchor* WallAnchor : ConnectedWalls)
	{
		const double WallWidth = WallAnchor->PlaneBounds.GetSize().X;
		TArray<FMRUKPlaneUV> PlaneUVAdjustments;
		for (const auto TexCoordMode : TexCoordModes)
		{
			float DenominatorX;
			float DenominatorY;
			// Determine the scaling in the V direction first, if this is set to maintain aspect
			// ratio we need to come back to it after U scaling has been determined.
			switch (TexCoordMode.V)
			{
				// Default to stretch in case maintain aspect ratio is set for both axes
				default:
				case EMRUKCoordModeV::Stretch:
				{
					DenominatorY = WallHeight;
					break;
				}
				case EMRUKCoordModeV::Metric:
				{
					DenominatorY = WorldToMeters;
					break;
				}
			}
			switch (TexCoordMode.U)
			{
				default:
				case EMRUKCoordModeU::Stretch:
				{
					DenominatorX = Perimeter;
					break;
				}
				case EMRUKCoordModeU::Metric:
				{
					DenominatorX = WorldToMeters;
					break;
				}
				case EMRUKCoordModeU::MetricSeamless:
				{
					DenominatorX = SeamlessWorldToMeters;
					break;
				}
				case EMRUKCoordModeU::MaintainAspectRatio:
				{
					DenominatorX = DenominatorY;
					break;
				}
				case EMRUKCoordModeU::MaintainAspectRatioSeamless:
				{
					DenominatorX = GetSeamlessFactor(Perimeter, DenominatorY);
					break;
				}
			}
			// Do another pass on V in case it has maintain aspect ratio set
			if (TexCoordMode.V == EMRUKCoordModeV::MaintainAspectRatio)
			{
				DenominatorY = DenominatorX;
			}

			const FVector2D Offset(UOffset / DenominatorX, 0);
			const FVector2D Scale(WallWidth / DenominatorX, WallHeight / DenominatorY);
			PlaneUVAdjustments.Push({ Offset, Scale });
		}
		if (!WallAnchor->HasLabel(FMRUKLabels::InvisibleWallFace))
		{
			OutAnchorsWithPlaneUVs.Push({ WallAnchor, PlaneUVAdjustments });
		}
		UOffset += WallWidth;
	}
}

UProceduralMeshComponent* AMRUKRoom::GetOrCreateGlobalMeshProceduralMeshComponent(bool& bOutExistedAlready) const
{
	// Try to find the global mesh procedural mesh component if it already exists
	TArray<UProceduralMeshComponent*> ProcMeshComponents;
	GetComponents<UProceduralMeshComponent>(ProcMeshComponents);
	for (const auto& ProcMeshComponent : ProcMeshComponents)
	{
		if (ProcMeshComponent->ComponentHasTag("GlobalMesh"))
		{
			bOutExistedAlready = true;
			return ProcMeshComponent;
		}
	}

	// Create the procedural mesh component if it doesn't exist already
	UProceduralMeshComponent* ProceduralMesh = NewObject<UProceduralMeshComponent>(GlobalMeshAnchor, TEXT("GlobalMesh"));
	ProceduralMesh->ComponentTags.Add("GlobalMesh");
	ProceduralMesh->RegisterComponent();
	GlobalMeshAnchor->AddInstanceComponent(ProceduralMesh);
	bOutExistedAlready = false;
	return ProceduralMesh;
}

void AMRUKRoom::SetupGlobalMeshProceduralMeshComponent(UProceduralMeshComponent& ProcMeshComponent, bool bExistedAlready, UMaterialInterface* Material) const
{
	ProcMeshComponent.SetMaterial(0, Material);
	if (!bExistedAlready)
	{
		ProcMeshComponent.SetCollisionProfileName(TEXT("BlockAll"));
		GlobalMeshAnchor->AddOwnedComponent(GlobalMeshAnchor->GetRootComponent());
		ProcMeshComponent.AttachToComponent(GlobalMeshAnchor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		ProcMeshComponent.SetRelativeScale3D(FVector(GetWorld()->GetWorldSettings()->WorldToMeters));
	}
}

bool AMRUKRoom::LoadGlobalMeshFromDevice(UMaterialInterface* Material)
{
	return GenerateProceduralSceneMesh(Material);
}

bool AMRUKRoom::LoadGlobalMeshFromJsonString(const FString& JsonString, UMaterialInterface* Material)
{
	return GenerateProceduralSceneMesh(Material);
}

bool AMRUKRoom::GenerateProceduralSceneMesh(UMaterialInterface* Material)
{
	if (!GlobalMeshAnchor)
	{
		UE_LOG(LogMRUK, Warning, TEXT("This room doesn't have a scene mesh anchor"));
		return false;
	}

	bool bProcMeshExisted = false;
	UProceduralMeshComponent* ProcMesh = GetOrCreateGlobalMeshProceduralMeshComponent(bProcMeshExisted);

	static constexpr bool bCreateCollision = true;
	TArray<FVector> EmptyNormals;
	TArray<FVector2D> EmptyUV;
	TArray<FColor> EmptyVertexColors;
	TArray<FProcMeshTangent> EmptyTangents;
	ProcMesh->CreateMeshSection(0, GlobalMeshAnchor->SceneMeshPositions, GlobalMeshAnchor->SceneMeshIndices, EmptyNormals, EmptyUV, EmptyVertexColors, EmptyTangents, bCreateCollision);

	SetupGlobalMeshProceduralMeshComponent(*ProcMesh, bProcMeshExisted, Material);

	return true;
}

FVector AMRUKRoom::ComputeCentroid(double Z)
{
	if (FloorAnchors.Num() == 0 || CeilingAnchors.Num() == 0)
	{
		return FVector::ZeroVector;
	}

	Z = FMath::Clamp(Z, 0.0, 1.0);

	double TotalArea = 0.0;
	FVector2D FloorCentroid = FVector2D::ZeroVector;
	for (const AMRUKAnchor* Anchor : FloorAnchors)
	{
		const FVector2D CentroidLS = UMRUKBPLibrary::ComputeCentroid(Anchor->PlaneBoundary2D);
		const FVector CentroidWS = Anchor->GetActorTransform().TransformPosition(FVector(0.0, CentroidLS.X, CentroidLS.Y));
		const FVector PlaneNormal = Anchor->GetActorTransform().TransformVector(FVector::UpVector);
		const float Area = CalculatePolygonArea(Anchor->PlaneBoundary2D);
		// Multiply the area by the dot product of up vector (0,0,1) and the plane normal
		FloorCentroid += FVector2D(CentroidWS.X, CentroidWS.Y) * Area * PlaneNormal.Z;
		TotalArea += Area * PlaneNormal.Z;
	}
	if (TotalArea > 0.0f)
	{
		FloorCentroid /= TotalArea;
	}
	return FVector(FloorCentroid.X, FloorCentroid.Y, FMath::Lerp(RoomBounds.Min.Z, RoomBounds.Max.Z, Z));
}

void AMRUKRoom::AttachProceduralMeshToWalls(const TArray<FMRUKTexCoordModes>& WallTextureCoordinateModes, const TArray<FString>& CutHoleLabels, UMaterialInterface* ProceduralMaterial)
{
	TArray<FMRUKAnchorWithPlaneUVs> AnchorsWithPlaneUVs;
	ComputeWallMeshUVAdjustments(WallTextureCoordinateModes, AnchorsWithPlaneUVs);
	for (const auto& AnchorWithPlaneUVs : AnchorsWithPlaneUVs)
	{
		AnchorWithPlaneUVs.Anchor->AttachProceduralMesh(AnchorWithPlaneUVs.PlaneUVs, CutHoleLabels, true, ProceduralMaterial);
	}
}

TArray<TObjectPtr<AMRUKAnchor>> AMRUKRoom::ComputeConnectedWalls() const
{
	if (WallAnchors.IsEmpty())
	{
		return {};
	}

	TArray<TObjectPtr<AMRUKAnchor>> ConnectedWalls;
	TArray<TObjectPtr<AMRUKAnchor>> RemainingWalls = WallAnchors;

	for (int i = RemainingWalls.Num() - 1; i >= 0; --i)
	{
		if (!RemainingWalls[i])
		{
			RemainingWalls.RemoveAt(i);
		}
	}

	ConnectedWalls.Reserve(WallAnchors.Num());
	ConnectedWalls.Push(RemainingWalls.Last());
	RemainingWalls.Pop();

	while (!RemainingWalls.IsEmpty())
	{
		AMRUKAnchor* PrevWall = ConnectedWalls.Last();
		FVector LocalMaxEdge(0, PrevWall->PlaneBounds.Max.X, 0);
		FVector MaxEdge = PrevWall->GetTransform().TransformPosition(LocalMaxEdge);
		int ClosestIndex = 0;
		float ClosestDist = UE_MAX_FLT;
		for (int i = 0; i < RemainingWalls.Num(); i++)
		{
			AMRUKAnchor* WallAnchor = RemainingWalls[i];
			FVector LocalMinEdge(0, WallAnchor->PlaneBounds.Min.X, 0);
			FVector MinEdge = WallAnchor->GetTransform().TransformPosition(LocalMinEdge);
			const double Dist = FVector::Dist2D(MaxEdge, MinEdge);
			if (Dist < ClosestDist)
			{
				ClosestDist = Dist;
				ClosestIndex = i;
			}
		}
		ConnectedWalls.Push(RemainingWalls[ClosestIndex]);
		RemainingWalls.RemoveAt(ClosestIndex);
	}

	return ConnectedWalls;
}

TArray<AActor*> AMRUKRoom::SpawnInterior(const TMap<FString, FMRUKSpawnGroup>& SpawnGroups, const TArray<FString>& CutHoleLabels, UMaterialInterface* ProceduralMaterial, bool ShouldFallbackToProcedural)
{
	return SpawnInteriorFromStream(SpawnGroups, FRandomStream(NAME_None), CutHoleLabels, ProceduralMaterial, ShouldFallbackToProcedural);
}

TArray<AActor*> AMRUKRoom::SpawnInteriorFromStream(const TMap<FString, FMRUKSpawnGroup>& SpawnGroups, const FRandomStream& RandomStream, const TArray<FString>& CutHoleLabels, UMaterialInterface* ProceduralMaterial, bool GlobalShouldFallbackToProcedural)
{
	TArray<AActor*> InteriorActors;

	auto ShouldFallbackToProcedural = [GlobalShouldFallbackToProcedural](const FMRUKSpawnGroup* SpawnGroup) -> bool {
		check(SpawnGroup);
		switch (SpawnGroup->FallbackToProcedural)
		{
			case EMRUKFallbackToProceduralOverwrite::Default:
			{
				return GlobalShouldFallbackToProcedural;
			}
			case EMRUKFallbackToProceduralOverwrite::Fallback:
			{
				return true;
			}
			case EMRUKFallbackToProceduralOverwrite::NoFallback:
			{
				return false;
			}
		}
		return false;
	};

	const float WorldToMeters = GetWorldSettings()->WorldToMeters;
	const FMRUKSpawnGroup* WallFace = SpawnGroups.Find(FMRUKLabels::WallFace);
	if (!WallFace || (WallFace->Actors.IsEmpty() && ShouldFallbackToProcedural(WallFace)))
	{
		// If no wall mesh is given we want to spawn the walls procedural to make seamless UVs
		AttachProceduralMeshToWalls(CutHoleLabels, ProceduralMaterial);
	}
	const FMRUKSpawnGroup* Floor = SpawnGroups.Find(FMRUKLabels::Floor);
	if (FloorAnchors.Num() > 0 && (!Floor || (Floor->Actors.IsEmpty() && ShouldFallbackToProcedural(Floor))))
	{
		// Use metric scaling to match walls
		for (AMRUKAnchor* Anchor : FloorAnchors)
		{
			const FVector2D Scale = Anchor->PlaneBounds.GetSize() / WorldToMeters;
			Anchor->AttachProceduralMesh({ { FVector2D::ZeroVector, Scale } }, CutHoleLabels, true, ProceduralMaterial);
		}
	}
	const FMRUKSpawnGroup* Ceiling = SpawnGroups.Find(FMRUKLabels::Ceiling);
	if (CeilingAnchors.Num() > 0 && (!Ceiling || (Ceiling->Actors.IsEmpty() && ShouldFallbackToProcedural(Ceiling))))
	{
		// Use metric scaling to match walls
		for (AMRUKAnchor* Anchor : CeilingAnchors)
		{
			const FVector2D Scale = Anchor->PlaneBounds.GetSize() / WorldToMeters;
			Anchor->AttachProceduralMesh({ { FVector2D::ZeroVector, Scale } }, CutHoleLabels, true, ProceduralMaterial);
		}
	}
	UMRUKSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UMRUKSubsystem>();

	for (const auto& Anchor : AllAnchors)
	{
		if (!Anchor)
		{
			continue;
		}
		if (Anchor->SemanticClassifications.IsEmpty())
		{
			Anchor->AttachProceduralMesh();
			continue;
		}

		bool bSpawnProceduralMesh = true;
		for (const auto& SemanticClassification : Anchor->SemanticClassifications)
		{
			if (SemanticClassification == FMRUKLabels::WallFace && Anchor->SemanticClassifications.Contains(FMRUKLabels::InvisibleWallFace))
			{
				// Treat anchors with WALL_FACE and INVISIBLE_WALL_FACE as anchors that only have INVISIBLE_WALL_FACE
				continue;
			}

			const FMRUKSpawnGroup* SpawnGroup = SpawnGroups.Find(SemanticClassification);

			if (!SpawnGroup)
			{
				continue;
			}
			if (SpawnGroup->Actors.IsEmpty())
			{
				if (!ShouldFallbackToProcedural(SpawnGroup))
				{
					bSpawnProceduralMesh = false;
				}
				continue;
			}

			bSpawnProceduralMesh = false;

			int Index = 0;
			if (SpawnGroup->Actors.Num() > 1)
			{
				if (SpawnGroup->SelectionMode == EMRUKSpawnerSelectionMode::Random)
				{
					Index = RandomStream.RandRange(0, SpawnGroup->Actors.Num() - 1);
				}
				else if (SpawnGroup->SelectionMode == EMRUKSpawnerSelectionMode::ClosestSize)
				{
					if (Anchor->VolumeBounds.IsValid)
					{
						const double AnchorSize = FMath::Pow(Anchor->VolumeBounds.GetVolume(), 1.0 / 3.0);
						double ClosestSizeDifference = UE_BIG_NUMBER;
						for (int i = 0; i < SpawnGroup->Actors.Num(); ++i)
						{
							const FMRUKSpawnActor& SpawnActor = SpawnGroup->Actors[i];
							FBox Bounds = Subsystem->GetActorClassBounds(SpawnActor.Actor);
							if (Bounds.IsValid)
							{
								const double SpawnActorSize = FMath::Pow(Bounds.GetVolume(), 1.0 / 3.0);
								const double SizeDifference = FMath::Abs(AnchorSize - SpawnActorSize);
								if (SizeDifference < ClosestSizeDifference)
								{
									ClosestSizeDifference = SizeDifference;
									Index = i;
								}
							}
						}
					}
				}
			}

			const FMRUKSpawnActor& SpawnActor = SpawnGroup->Actors[Index];
			if (SpawnActor.Actor)
			{
				AActor* InteriorActor = Anchor->SpawnInterior(SpawnActor.Actor, SpawnActor.MatchAspectRatio, SpawnActor.CalculateFacingDirection, SpawnActor.ScalingMode);
				InteriorActors.Push(InteriorActor);
			}
			else
			{
				UE_LOG(LogMRUK, Error, TEXT("Actor is nullptr for label %s."), *SemanticClassification);
			}
			break;
		}

		if (bSpawnProceduralMesh)
		{
			Anchor->AttachProceduralMesh(CutHoleLabels, true, ProceduralMaterial);
		}
	}

	return InteriorActors;
}

bool AMRUKRoom::IsWallAnchor(AMRUKAnchor* Anchor) const
{
	return WallAnchors.Contains(Anchor);
}

#undef LOCTEXT_NAMESPACE
