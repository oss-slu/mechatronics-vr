// Fill out your copyright notice in the Description page of Project Settings.


#include "FocusStep.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/PrimitiveComponent.h"

UFocusStep::UFocusStep()
{
	bWantsTickWhileActive = true;
	FocusAngleDegrees = 8.0f;
	MaxFocusDistance = 0.0f;
	MinFocusTimeSeconds = 1.5f;
	bRequireLineOfSight = true;
	bRequireContinuousFocus = true;
}

void UFocusStep::SetTargetActor(AActor* InTarget)
{
	TargetActor = InTarget;
}

bool UFocusStep::CheckCompletion_Implementation() const
{
	if (!IsValid(TargetActor)) return false;

	if (!bIsFocusedThisFrame) return false;

	return FocusAccumulatedTime >= MinFocusTimeSeconds;
}

void UFocusStep::OnStarted()
{
	FocusAccumulatedTime = 0.0f;
	bIsFocusedThisFrame = false;
	bWasFocusedLastFrame = false;

	if (!IsValid(TargetActor))
	{
		UE_LOG(LogTemp, Error, TEXT("TargetActor is not set"));
	}
	NotifyUpdated();
}

void UFocusStep::OnStopped()
{
	Super::OnStopped();
}

void UFocusStep::TickStep(float DeltaTime)
{
	if (!IsActive() || bStepCompleted)
		return;

	bool bHadLOS = false;
	float AngleDeg = 0.0f;
	float Distance = 0.0f;

	bIsFocusedThisFrame = ComputeFocusThisFrame(bHadLOS, AngleDeg, Distance);

	if (bIsFocusedThisFrame)
	{
		FocusAccumulatedTime += DeltaTime;
	}
	else
	{
		if (bRequireContinuousFocus)
		{
			if (bWasFocusedLastFrame)
			{
				NotifyUpdated();
			}
			FocusAccumulatedTime = 0.0f;
		}
	}
	if (bIsFocusedThisFrame != bWasFocusedLastFrame)
	{
		NotifyUpdated();
	}
	bWasFocusedLastFrame = bIsFocusedThisFrame;
	if (CheckCompletion())
	{
		CompleteStep();
	}
}

bool UFocusStep::ComputeFocusThisFrame(bool& bOutHadLOS, float& OutAngleDeg, float& OutDistance) const
{
	bOutHadLOS = false;
	OutAngleDeg = 180.0f;
	OutDistance = TNumericLimits<float>::Max();

	if (!IsValid(TargetActor)) return false;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return false;

	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	FVector Origin, Extent;
	TargetActor->GetActorBounds(true,Origin,Extent);

	const FVector ToTarget = (Origin - CamLoc);
	const float Dist = ToTarget.Size();
	OutDistance = Dist;

	if (MaxFocusDistance < 0.0f && Dist > MaxFocusDistance) return false;
	
	const FVector Fwd = CamRot.Vector();
	const float Dot   = FVector::DotProduct(Fwd.GetSafeNormal(), ToTarget.GetSafeNormal());
	const float CosThresh = FMath::Cos(FMath::DegreesToRadians(FocusAngleDegrees));
	const bool bInAngle = (Dot >= CosThresh);
	OutAngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.f, 1.f)));

	if (!bInAngle) return false;

	if (bRequireLineOfSight)
	{
		FHitResult Hit;

		FCollisionQueryParams Params(SCENE_QUERY_STAT(FocusTrace), true);

		Params.bReturnPhysicalMaterial = false;

		if (APawn* Pawn = PC->GetPawn())
		{
			Params.AddIgnoredActor(Pawn);
		}

		const bool bHit = GetWorld()->LineTraceSingleByChannel(
			Hit, CamLoc, Origin, ECC_Visibility, Params);

		if (bHit && Hit.GetActor() && Hit.GetActor() != TargetActor)
		{
			//view is being blocked
			return false;
		}
		bOutHadLOS = true;
	}
	else
	{
		bOutHadLOS = true;
	}
	return true;
	
}




