// Copyright notice placeholder, matching the other lesson steps.

#include "WalkToAreaStep.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Volume.h"
#include "Camera/PlayerCameraManager.h"

UWalkToAreaStep::UWalkToAreaStep()
{
	// The timer-driven TickStep below is what watches the player; the manager also polls CheckCompletion().
	bWantsTickWhileActive = true;
}

AActor* UWalkToAreaStep::FindTargetActor() const
{
	UWorld* World = GetWorld();
	if (!World || TargetActorTag.IsNone())
	{
		return nullptr;
	}

	// TActorIterator walks every loaded level, including Level Instances such as the DC room inside Demo_LVL.
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Candidate = *It;
		if (IsValid(Candidate) && Candidate->ActorHasTag(TargetActorTag))
		{
			return Candidate;
		}
	}
	return nullptr;
}

bool UWalkToAreaStep::GetPlayerLocation(FVector& OutLocation) const
{
	UWorld* World = GetWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return false;
	}

	// Prefer the head (camera): in VR the pawn root doesn't follow physical room-scale walking.
	if (PC->PlayerCameraManager)
	{
		OutLocation = PC->PlayerCameraManager->GetCameraLocation();
		return true;
	}
	if (const APawn* Pawn = PC->GetPawn())
	{
		OutLocation = Pawn->GetActorLocation();
		return true;
	}
	return false;
}

bool UWalkToAreaStep::IsPointInArea(const AActor* Area, const FVector& Point)
{
	if (const AVolume* Volume = Cast<AVolume>(Area))
	{
		// Exact test against the brush shape (Trigger Box, Trigger Volume, ...).
		return Volume->EncompassesPoint(Point);
	}

	// Any other actor: axis-aligned box around its colliding components.
	return Area->GetComponentsBoundingBox(/*bNonColliding=*/ true).IsInside(Point);
}

void UWalkToAreaStep::OnStarted()
{
	bPlayerArrived = false;
	bWarnedMissingTarget = false;

	UE_LOG(LogTemp, Log, TEXT("UWalkToAreaStep::OnStarted - waiting for player to enter area tagged '%s'"),
		*TargetActorTag.ToString());
}

void UWalkToAreaStep::OnStopped()
{
	StopTicking();
}

void UWalkToAreaStep::TickStep(float /*DeltaTime*/)
{
	if (!bIsActive || bStepCompleted)
	{
		return;
	}

	const AActor* Area = FindTargetActor();
	if (!Area)
	{
		if (!bWarnedMissingTarget)
		{
			bWarnedMissingTarget = true;
			UE_LOG(LogTemp, Warning, TEXT("UWalkToAreaStep: no actor tagged '%s' in the level, so this step can't complete. "
				"Add the tag to a Trigger Box over the destination room."), *TargetActorTag.ToString());
		}
		return;
	}

	FVector PlayerLocation;
	if (!GetPlayerLocation(PlayerLocation))
	{
		return;
	}

	if (IsPointInArea(Area, PlayerLocation))
	{
		UE_LOG(LogTemp, Log, TEXT("UWalkToAreaStep: player entered '%s', completing step"), *Area->GetName());
		bPlayerArrived = true;
		CompleteStep();
	}
}

bool UWalkToAreaStep::CheckCompletion_Implementation() const
{
	return bPlayerArrived;
}
