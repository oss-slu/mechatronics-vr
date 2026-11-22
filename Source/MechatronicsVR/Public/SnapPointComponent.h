// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"

#include "SnapPointComponent.generated.h"

class APartActor;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MECHATRONICSVR_API USnapPointComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USnapPointComponent();
	bool CanAcceptSnapID(FName OtherSnapID) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snap Point")
	FName SnapID;

	/** Currently showing preview for this part */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snap Preview")
	TObjectPtr<APartActor> PreviewingPart;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snap Point")
	TArray<TSubclassOf<APartActor>> CompatibleParts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap Point")
	TArray<FName> CompatibleSnapIDs; // ✅ Specific and flexible

	UPROPERTY(BlueprintReadWrite, Category = "Assembly")
	bool bIsAssembled = false;

	/**
	 * The snap point this one is currently mated to (null while unassembled).
	 *
	 * Set on both ends by AAssemblyActor::ConnectParts and cleared by AAssemblyActor::DetachPart.
	 * AAssemblyActor::Connections is wiped every tick (constraints are never created), so this is
	 * the reliable way to ask "which terminal is this contact plugged into?". For example the
	 * motor reads ContactSnapPoint->ConnectedSnapPoint->SnapID to tell the battery's positive
	 * terminal from its negative one and pick the spin direction.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assembly")
	TObjectPtr<USnapPointComponent> ConnectedSnapPoint = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap Point")
	FString Metadata;

	
	/** Is this snap point part of the current assembly step? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assembly Sequence")
	bool bIsActiveInCurrentStep = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snap Detection")
	TObjectPtr<USphereComponent> SnapDetectionSphere;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snap Detection")
    TArray<TObjectPtr<USnapPointComponent>> NearbySnapPoints;

	/** Radius for snap detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap Detection")
	float SnapDetectionRadius = 5.0f; // 5cm - much more reasonable

	



	
	UFUNCTION(BlueprintCallable, Category = "Snap Point")
	bool CanAcceptPart(const APartActor* Part) const;
	
	UFUNCTION(BlueprintCallable, Category = "Snap Point")
	bool CanAcceptPoint(const USnapPointComponent* OtherSnapPoint) const;

	UFUNCTION(BlueprintCallable, Category = "Snap Point")
	void OnSnapDetectionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
													 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(BlueprintCallable, Category = "Snap Point")
	void OnSnapDetectionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
												   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Get the closest compatible snap point within range */
	UFUNCTION(BlueprintCallable, Category = "Snap Detection")
	USnapPointComponent* GetClosestCompatibleSnapPoint() const;
	void CleanupNearbySnapPoints();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
