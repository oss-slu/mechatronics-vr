// Copyright notice placeholder, matching the other lesson steps.

#include "TerminalSwapStep.h"
#include "PartActor.h"
#include "SnapPointComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"

UTerminalSwapStep::UTerminalSwapStep()
{
	// The timer-driven TickStep below is what watches the part; the manager also polls CheckCompletion() each frame.
	bWantsTickWhileActive = true;
}

APartActor* UTerminalSwapStep::FindWatchedPart() const
{
	UWorld* World = GetWorld();
	if (!World || !*WatchedPartClass)
	{
		return nullptr;
	}

	APartActor* SubclassMatch = nullptr;
	for (TActorIterator<APartActor> It(World); It; ++It)
	{
		APartActor* Part = *It;
		if (!IsValid(Part) || !Part->IsA(WatchedPartClass))
		{
			continue;
		}
		if (Part->GetClass() == WatchedPartClass)
		{
			return Part;
		}
		if (!SubclassMatch)
		{
			SubclassMatch = Part;
		}
	}
	return SubclassMatch;
}

FName UTerminalSwapStep::GetCurrentTerminalID() const
{
	const APartActor* Part = FindWatchedPart();
	if (!Part)
	{
		return NAME_None;
	}

	for (const USnapPointComponent* Snap : Part->GetSnapPoints())
	{
		if (Snap && Snap->bIsAssembled && Snap->ConnectedSnapPoint)
		{
			return Snap->ConnectedSnapPoint->SnapID;
		}
	}
	return NAME_None;
}

void UTerminalSwapStep::OnStarted()
{
	bSwapped = false;
	BaselineTerminalID = GetCurrentTerminalID();

	UE_LOG(LogTemp, Log, TEXT("UTerminalSwapStep::OnStarted - baseline terminal: %s"), *BaselineTerminalID.ToString());
}

void UTerminalSwapStep::OnStopped()
{
	StopTicking();
}

void UTerminalSwapStep::TickStep(float /*DeltaTime*/)
{
	if (!bIsActive || bStepCompleted)
	{
		return;
	}

	const FName Current = GetCurrentTerminalID();
	if (Current.IsNone())
	{
		// Detached (mid-swap) or never connected: keep waiting.
		return;
	}

	if (BaselineTerminalID.IsNone())
	{
		// The wire wasn't on a terminal when the step began; the first terminal it lands on is the one to leave.
		BaselineTerminalID = Current;
		return;
	}

	if (Current != BaselineTerminalID)
	{
		UE_LOG(LogTemp, Log, TEXT("UTerminalSwapStep: %s -> %s, swap detected"),
			*BaselineTerminalID.ToString(), *Current.ToString());
		bSwapped = true;
		CompleteStep();
	}
}

bool UTerminalSwapStep::CheckCompletion_Implementation() const
{
	return bSwapped;
}
