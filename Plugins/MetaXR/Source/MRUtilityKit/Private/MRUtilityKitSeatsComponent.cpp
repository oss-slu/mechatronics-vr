// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "MRUtilityKitSeatsComponent.h"
#include "MRUtilityKitAnchor.h"
#include "MRUtilityKitRoom.h"
#include "Kismet/KismetMathLibrary.h"

void UMRUKSeatsComponent::CalculateSeatPoses(double SeatWidth)
{
	const AMRUKAnchor* Anchor = Cast<AMRUKAnchor>(GetOwner());
	if (!Anchor)
	{
		return;
	}

	SeatPoses.Empty();

	const FVector2D SurfaceDimensions = Anchor->PlaneBounds.GetExtent();
	const double SurfaceRatio = SurfaceDimensions.X / SurfaceDimensions.Y;
	const FVector SeatForward = Anchor->GetFacingDirection();
	const FVector SeatUp = FVector::UpVector;
	const FQuat SeatRotation = UKismetMathLibrary::MakeRotFromXZ(SeatForward, SeatUp).Quaternion();

	if (SurfaceRatio < 2.0 && SurfaceRatio > 0.5)
	{
		// If the surface dimensions are mostly square (likely a chair), just have one centered seat.
		FTransform SeatPose{};
		SeatPose.SetLocation(Anchor->GetActorLocation());
		SeatPose.SetRotation(SeatRotation);
		SeatPoses.Add(SeatPose);
	}
	else
	{
		const bool bXLong = SurfaceDimensions.X > SurfaceDimensions.Y;
		const double LongestDimension = bXLong ? SurfaceDimensions.X : SurfaceDimensions.Y;
		const int32 NumSeats = FMath::Floor(LongestDimension / SeatWidth);

		const double SeatPadding = (LongestDimension - (NumSeats * SeatWidth)) / NumSeats;
		const double FirstSeatOffset = (-LongestDimension + SeatPadding + SeatWidth) * 0.5;

		for (int32 i = 0; i < NumSeats; ++i)
		{
			const FVector SeatRight = bXLong ? Anchor->GetActorRightVector() : Anchor->GetActorUpVector();

			const double Offset = FirstSeatOffset + (SeatWidth + SeatPadding) * i;
			const FVector SeatPosition = Anchor->GetActorLocation() + SeatRight * Offset;

			FTransform SeatPose{};
			SeatPose.SetLocation(SeatPosition);
			SeatPose.SetRotation(SeatRotation);
			SeatPoses.Add(SeatPose);
		}
	}
}
