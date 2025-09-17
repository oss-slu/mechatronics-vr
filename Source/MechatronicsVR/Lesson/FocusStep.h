// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LessonStep.h"
#include "FocusStep.generated.h"

class AActor;
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class MECHATRONICSVR_API UFocusStep : public ULessonStep
{
	GENERATED_BODY()

public:
	UFocusStep();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus Step|Setup")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus Step|Criteria", meta=(ClampMin="1.0", ClampMax="45.0"))
	float FocusAngleDegrees = 8.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus Step|Criteria", meta=(ClampMin="0.0"))
	float MaxFocusDistance = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus Step|Criteria", meta=(ClampMin="0.1"))
	float MinFocusTimeSeconds = 1.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus Step|Criteria")
	bool bRequireLineOfSight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus Step|Criteria")
	bool bRequireContinuousFocus = true;

	UFUNCTION(BlueprintCallable, Category="Focus Step|Setup")
	void SetTargetActor(AActor* InTarget);

	UFUNCTION(BlueprintPure, Category="Focus Step|Status")
	float GetAccumulatedFocusTime() const { return FocusAccumulatedTime; }

	UFUNCTION(BlueprintPure, Category="Focus Step|Status")
	bool IsCurrentlyFocused() const { return bIsFocusedThisFrame; }

	virtual bool CheckCompletion_Implementation() const;

protected:
	virtual void OnStarted() override;
	virtual void OnStopped() override;
	virtual void TickStep(float DeltaTime) override;

private:
	bool ComputeFocusThisFrame(bool& bOutHadLOS, float& OutAngleDeg, float& OutDistance) const;
	
	UPROPERTY(Transient)
	float FocusAccumulatedTime = 0.0f;

	UPROPERTY(Transient)
	bool bIsFocusedThisFrame = false;

	UPROPERTY(Transient)
	bool bWasFocusedLastFrame = false;
};
