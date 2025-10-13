// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/** Movement speed value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float MovementSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	bool bSmoothMovementEnabled = false;
	
	/** Actions for deselecting object */
	//UFUNCTION(BlueprintCallable, Category = "Movement")
	//void MoveForward(float Value);

	/** Actions for deselecting object */
	//UFUNCTION(BlueprintCallable, Category = "GrabCast")
	//void MoveRight(float Value);
	
private:
	//FVector PendingInput;
	//class UCameraComponent* Camera;
		
};
