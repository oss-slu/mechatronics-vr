// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SnapGuideArrowActor.generated.h"

class USnapPointComponent;
class UStaticMeshComponent;
class USceneComponent;

/**
 * ASnapGuideArrowActor
 * 
 * 3D world-space guide indicator actor designed for VR assembly tasks.
 * Positioned hovering outside a target snap/attachment point and points inward
 * along the socket's approach axis, with optional gentle bobbing and pulsing animation.
 */
UCLASS()
class MECHATRONICSVR_API ASnapGuideArrowActor : public AActor
{
	GENERATED_BODY()
	
public:	
	/** Default constructor */
	ASnapGuideArrowActor();

	/**
	 * Configures and aligns the arrow to start at the source part (the object to be placed)
	 * and point directly to the destination snap point on the assembly.
	 *
	 * @param InSourceActor The part actor that needs to be picked up and placed.
	 * @param InTargetSnapPoint The destination socket component it should be placed onto.
	 * @param InSourceOffset Offset from the source actor's origin towards the target.
	 * @note Dynamically tracks movements of both source and target in Tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Snap Guide")
	void PointBetween(AActor* InSourceActor, USnapPointComponent* InTargetSnapPoint, float InSourceOffset = 10.0f);

	/**
	 * Configures and aligns the arrow to point directly into the specified destination snap point.
	 *
	 * @param InTargetSnapPoint The snap point component that the user needs to attach a part onto.
	 * @param InHoverDistance Distance (in cm) to offset the arrow back along the approach axis so it does not occlude the socket.
	 */
	UFUNCTION(BlueprintCallable, Category = "Snap Guide")
	void PointToSnapPoint(USnapPointComponent* InTargetSnapPoint, float InHoverDistance = 15.0f);

	/**
	 * Clears the active target and hides the guide arrow.
	 */
	UFUNCTION(BlueprintCallable, Category = "Snap Guide")
	void ClearTargetSnapPoint();

	/**
	 * Shows the guide indicator.
	 */
	UFUNCTION(BlueprintCallable, Category = "Snap Guide")
	void ShowGuide();

	/**
	 * Hides the guide indicator.
	 */
	UFUNCTION(BlueprintCallable, Category = "Snap Guide")
	void HideGuide();

	/**
	 * Checks if the arrow is currently active and pointing to a valid target snap point.
	 *
	 * @return True if a target snap point is currently assigned.
	 */
	UFUNCTION(BlueprintPure, Category = "Snap Guide")
	bool HasActiveTarget() const { return TargetSnapPoint != nullptr; }

	/**
	 * Retrieves the currently targeted snap point component.
	 *
	 * @return Pointer to the target snap point component.
	 */
	UFUNCTION(BlueprintPure, Category = "Snap Guide")
	USnapPointComponent* GetTargetSnapPoint() const { return TargetSnapPoint; }

	/**
	 * Retrieves the source actor that needs to be placed.
	 *
	 * @return Pointer to the source actor.
	 */
	UFUNCTION(BlueprintPure, Category = "Snap Guide")
	AActor* GetSourceActor() const { return SourceActor.Get(); }

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Static mesh representing the visual 3D arrow */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ArrowMesh;

	/** Distance (in cm) the arrow hovers away from the snap point along its normal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap Guide|Appearance")
	float HoverDistance = 15.0f;

	/** Enable or disable floating / bobbing animation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap Guide|Animation")
	bool bEnableBobbing = true;

	/** Amplitude of the bobbing motion in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap Guide|Animation")
	float BobbingAmplitude = 3.0f;

	/** Speed frequency of the bobbing motion (cycles per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap Guide|Animation")
	float BobbingSpeed = 2.5f;

	/** Enable or disable rotating spin around the approach axis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap Guide|Animation")
	bool bEnableSpin = false;

	/** Rotation rate in degrees per second if spin is enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap Guide|Animation")
	float SpinRate = 45.0f;

private:
	/** Active source part/actor that needs to be placed */
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> SourceActor = nullptr;

	/** Offset distance from source actor origin along direction to target */
	float SourceOffset = 10.0f;

	/** Active snap point we are pointing towards */
	UPROPERTY(Transient)
	TObjectPtr<USnapPointComponent> TargetSnapPoint = nullptr;

	/** Accumulated time for animation */
	float AnimationTimer = 0.0f;

	/** Updates the world transform taking into account source part and target socket movement */
	void UpdateArrowTransform(float DeltaTime);
};
