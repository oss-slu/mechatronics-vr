// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GrabComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGrabbed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDropped);

UENUM(BlueprintType)
enum class EGrabType : uint8
{
	Free    UMETA(DisplayName = "Free"),
	Snap    UMETA(DisplayName = "Snap"),
	Custom  UMETA(DisplayName = "Custom")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MECHATRONICSVR_API UGrabComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrabComponent();

	/** Whether this component is currently held */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grab")
	bool bIsHeld = false;
	
	
    /** Reference to the motion controller holding this */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grab")
    class UMotionControllerComponent* MotionControllerRef = nullptr;

	/** Primary grab component if using two-handed grab */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grab")
	UGrabComponent* PrimaryGrabComponent = nullptr;

	/** Relative rotation when grabbed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grab")
	FRotator PrimaryGrabRelativeRotation;

	/** Should simulate physics when dropped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	bool bSimulateOnDrop = true;

	/** Haptic effect to play on grab */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	class UHapticFeedbackEffect_Base* OnGrabHapticEffect = nullptr;

	/** Event dispatchers */
	UPROPERTY(BlueprintAssignable, Category = "Grab Events")
	FOnGrabbed OnGrabbed;

	UPROPERTY(BlueprintAssignable, Category = "Grab Events")
	FOnDropped OnDropped;

	
	/** Try to grab this component */
	UFUNCTION(BlueprintCallable, Category = "Grab", meta = (ReturnDisplayName = " Is Held"))
	bool TryGrab(UMotionControllerComponent* MotionController, bool bIsSecondaryGrab = false);
	
	/** Release this component */
	UFUNCTION(BlueprintCallable, Category = "Grab", meta = (ReturnDisplayName = " Is Held"))
	bool TryRelease();

	/** Get which hand is holding this */
	UFUNCTION(BlueprintCallable, Category = "Grab")
	FName GetHeldByHand() const;

	/** Check if currently grabbed */
	UFUNCTION(BlueprintCallable, Category = "Grab")
	bool IsGrabbed() const { return bIsHeld; }


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
