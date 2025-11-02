// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionStep.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "GrabComponent.h"

UInteractionStep::UInteractionStep()
{
	InteractionType = EInteractionType::Grab;
	RequiredCount = 1;
	bWantsTickWhileActive = false;
	FocusRadius = 75.f;
	MinFocusTime = 1.0f;
	bContinuous = true;
	MaxDistance = 0.0f;
	StepType = ELessonStepType::Interaction;
}

void UInteractionStep::SetTargetActor(AActor* InTarget)
{
	TargetActor = InTarget;
}

void UInteractionStep::ReportInteraction()
{
	if (!IsActive() || bStepCompleted) return;

	if (InteractionType == EInteractionType::Manual)
	{
		BumpCountAndMaybeComplete();
	}
}

bool UInteractionStep::CheckCompletion_Implementation() const
{
	return CurrentCount >= RequiredCount;
}

void UInteractionStep::OnStarted()
{
	CurrentCount = 0;
	AccumulatedFocus = 0.0f;
	bFocusedThisFrame = false;
	bFocusedLastFrame = false;

	if (!IsValid(TargetActor))
	{
		UE_LOG(LogTemp, Error, TEXT("UInteractionStep::OnStarted; Target Actor is not set"));
	}
	bWantsTickWhileActive = (InteractionType == EInteractionType::Proximity);

	BindForInteraction();

	if (CheckCompletion())
	{
		CompleteStep();
		return;
	}
	NotifyUpdated();
}

void UInteractionStep::OnStopped()
{
	UnbindForInteraction();
}

void UInteractionStep::TickStep(float DeltaTime)
{
	if (!IsActive() || bStepCompleted) return;

	if (bWantsTickWhileActive)
	{
		if (ComputeProximityFocused(DeltaTime))
		{
			BumpCountAndMaybeComplete();
		}
		else if (bContinuous)
		{
			AccumulatedFocus = 0.0f;
		}
	}
}

void UInteractionStep::BindForInteraction()
{
	UnbindForInteraction();

	if (!IsValid(TargetActor)) return;

	if (InteractionType == EInteractionType::Grab || InteractionType == EInteractionType::Drop)
	{
		CachedGrabComp = TargetActor->FindComponentByClass<UGrabComponent>();

		if (!CachedGrabComp)
		{
			UE_LOG(LogTemp, Warning, TEXT("UInteractionStep: Target '%s' has no UGrabComponent."), *TargetActor->GetName());
			return;
		}
		if (InteractionType == EInteractionType::Grab)
		{
			CachedGrabComp->OnGrabbed.AddDynamic(this, &UInteractionStep::HandleGrabbed);
		}
		else // Drop
		{
			CachedGrabComp->OnDropped.AddDynamic(this, &UInteractionStep::HandleDropped);
		}
		//There are no delegates needed for proximity and manual
	}
}

void UInteractionStep::UnbindForInteraction()
{
	if (CachedGrabComp)
	{
		CachedGrabComp->OnGrabbed.RemoveDynamic(this, &UInteractionStep::HandleGrabbed);
		CachedGrabComp->OnDropped.RemoveDynamic(this, &UInteractionStep::HandleDropped);
		CachedGrabComp = nullptr;
	}
}

void UInteractionStep::HandleGrabbed()
{
	BumpCountAndMaybeComplete();
}

void UInteractionStep::HandleDropped()
{
	BumpCountAndMaybeComplete();
}

void UInteractionStep::BumpCountAndMaybeComplete()
{
	++CurrentCount;
	NotifyUpdated();
	if (CheckCompletion())
	{
		CompleteStep();
	}
}

bool UInteractionStep::ComputeProximityFocused(float DeltaTime)
{
	if (!IsValid(TargetActor)) return false;

	FVector CamLoc;
	FRotator CamRot;
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(),0))
	{
		PC->GetPlayerViewPoint(CamLoc, CamRot);
	}
	else
	{
		return false;
	}
	FVector Origin, Extent;
	TargetActor->GetActorBounds(true, Origin, Extent);

	const float Dist = FVector::Distance(CamLoc, Origin);

	if (MaxDistance > 0.0f && Dist > MaxDistance)
	{
		bFocusedThisFrame = false;
		return false;
	}
	const bool bInRadius = Dist <= FocusRadius;
	bFocusedThisFrame = bInRadius;

	if (bInRadius)
	{
		AccumulatedFocus += DeltaTime;
	}
	else if (bContinuous)
	{
		AccumulatedFocus = 0.0f;
	}

	if (bFocusedThisFrame != bFocusedLastFrame)
	{
		NotifyUpdated();
	}
	bFocusedLastFrame = bFocusedThisFrame;

	return AccumulatedFocus >= MinFocusTime && bInRadius;
}
