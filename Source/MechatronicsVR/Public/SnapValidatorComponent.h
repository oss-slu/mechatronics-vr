// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SnapPointComponent.h"
#include "Components/ActorComponent.h"
#include "SnapValidatorComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MECHATRONICSVR_API USnapValidatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USnapValidatorComponent();

	//Called when a snap attempt is made
	UFUNCTION(BlueprintCallable, Category = "Snap Validation")
	virtual bool isSnapValid(USnapPointComponent* SnapPoint, const USnapPointComponent * TargetSnapPoint) const;

	//called when a snap is completed (dunno if this will do anything but it is here
	UFUNCTION(BlueprintCallable, Category = "Snap Validation")
	virtual void OnSnapCompleted(USnapPointComponent* SnapPoint, USnapPointComponent* TargetSnapPoint);

	UFUNCTION(BlueprintCallable, Category = "Snap Validation")
	virtual bool CanBeDisassembled(USnapPointComponent* SnapPoint) const;

	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// UPROPERTY(BlueprintReadOnly, Category = "Lesson")
	// TObjectPtr<ULessonStep> CurrentStep;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
