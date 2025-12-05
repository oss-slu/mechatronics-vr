// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "VRSmoothMovementComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MECHATRONICSVR_API UVRSmoothMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVRSmoothMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	/** Called every frame */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Call this from input bindings with the thumbstick axis values */
	UFUNCTION(BlueprintCallable, Category = "VR Movement")
	void MoveWithThumbstickInput(float AxisX, float AxisY);

	/** Enable or disable smooth movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	bool bSmoothMovementEnabled = true;

	/** Movement speed in units/second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Movement")
	float MoveSpeed = 300.0f;
	
private:
	UCameraComponent* Camera = nullptr;
	UMotionControllerComponent* MotionController = nullptr;
	FVector PendingInput = FVector::ZeroVector;
	FVector PreviousPosition = FVector::ZeroVector;
};
