// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LessonStep.h"
#include "LessonTypes.h"
#include "InteractionStep.generated.h"

class AActor;
class UPrimitiveComponent;
class UGrabComponent;

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class MECHATRONICSVR_API UInteractionStep : public ULessonStep
{
	GENERATED_BODY()

public:
	UInteractionStep();

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Interaction Step|Setup")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Step|Criteria")
	EInteractionType InteractionType = EInteractionType::Grab;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Step|Criteria", meta=(ClampMin="1"))
	int32 RequiredCount = 1;

	UFUNCTION(BlueprintPure, Category="Interaction Step|Status")
	int32 GetCurrentCount() const { return CurrentCount; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Step|Proximity", meta=(ClampMin="1.0"))
	float FocusRadius = 75.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Step|Proximity", meta=(ClampMin="0.1"))
	float MinFocusTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Step|Proximity")
	bool bContinuous = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Step|Proximity", meta=(ClampMin="0.0"))
	float MaxDistance = 0.0f;

	UFUNCTION(BlueprintCallable, Category="Interaction Step|Setup")
	void SetTargetActor(AActor* InTarget);

	UFUNCTION(BlueprintCallable, Category="Interaction Step|Manual")
	void ReportInteraction();

	virtual bool CheckCompletion_Implementation() const;

protected:
	virtual void OnStarted() override;
	virtual void OnStopped() override;
	virtual void TickStep(float DeltaTime) override;

private:
	void BindForInteraction();
	void UnbindForInteraction();

	UFUNCTION()
	void HandleGrabbed();

	UFUNCTION()
	void HandleDropped();

	void BumpCountAndMaybeComplete();
	bool ComputeProximityFocused(float DeltaTime);

	UPROPERTY(Transient)
	int32 CurrentCount = 0;

	UPROPERTY(Transient)
	float AccumulatedFocus = 0.f;
	UPROPERTY(Transient)
	bool  bFocusedThisFrame = false;
	UPROPERTY(Transient)
	bool  bFocusedLastFrame = false;

	UPROPERTY(Transient)
	UGrabComponent* CachedGrabComp = nullptr;
};
