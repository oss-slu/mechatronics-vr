// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "MotionControllerComponent.h"
#include "Materials/MaterialInterface.h"
#include "GrabRayCaster.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MECHATRONICSVR_API UGrabRayCaster : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrabRayCaster();

	/** Range of telekinesis reach  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GrabRayCaster")
	float CastRange = 500.0f;

	/** Sweep sphere radius  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GrabRayCaster")
	float CapsuleRadius = 25.0f;

	/** Time it takes to suck object to hand  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GrabRayCaster")
	float SuckTime = 0.2f;	// needs to match delay node in VRPawn blueprint

	/** If this is ray caster for left hand*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GrabRayCaster")
	bool IsLeft = false;
	
	/** Reference to the motion controller the cast is attached to  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GrabCast", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMotionControllerComponent> MotionControllerRef = nullptr;

	/** Reference to the object being interacted with  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GrabCast", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> ActorRef = nullptr;

	/** Reference to overlay material  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GrabCast", meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* OverlayMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Script/Engine.Material'/Game/M_GrabbableOutline.M_GrabbableOutline'"));

	
	/** Ray cast from each hand and check for grabables */	// now uses sphere for more tolerance
	UFUNCTION(BlueprintCallable, Category = "GrabCast")
	UGrabComponent* CheckReachForGrabComponent();

	/** Actions for deselecting object */
	UFUNCTION(BlueprintCallable, Category = "GrabCast")
	void DeselectObject();

	/** Force selected object to motion controller location */
	UFUNCTION(BlueprintCallable, Category = "GrabCast")
	void SuckObjectToSource();		// has known bug (see .cpp)

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
