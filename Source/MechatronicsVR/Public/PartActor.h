// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssemblyActor.h"
#include "PartActor.generated.h"


class UAssemblyComponent;
class USnapValidatorComponent;
class USnapPointComponent;
class AAssemblyActor; 
UCLASS()
class MECHATRONICSVR_API APartActor : public AActor
{
	GENERATED_BODY()
	
public:
	
	APartActor();
	USnapPointComponent* GetBestSnapPointFor(USnapPointComponent* TargetSnapPoint) const;

	/** Visual mesh of the part */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Part")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** Currently previewing snap with this target snap point */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snap Preview")
	TObjectPtr<USnapPointComponent> CurrentTargetSnapPoint = nullptr;

	/** Check if we should show preview based on grab state and nearby snap points */
	UFUNCTION(BlueprintCallable, Category = "Snap Preview")
	void UpdatePreviewState();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	TSoftObjectPtr<APartActor> PartAssembledOnto = nullptr;

	// /** Find the best snap point to preview with */
	UFUNCTION(BlueprintCallable, Category = "Snap Preview") 
	USnapPointComponent* FindBestPreviewTarget() const;

	// Add this property in the public section with your other components
	/** Component that handles grab interactions */
	/** Optional: specific snap point IDs this part prefers to connect to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assembly Configuration")
	TArray<FName> PreferredTargetSnapIDs;

	/** Attempt to snap to the current preview target */
	UFUNCTION(BlueprintCallable, Category = "Snap")
	bool TrySnapToPreview();


	
	/** Preview mesh that shows where this part will snap */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snap Preview")
	TObjectPtr<UStaticMeshComponent> PreviewMesh;

	/** Material for preview (ghost-like appearance) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap Preview")
	TObjectPtr<UMaterialInterface> PreviewMaterial;

	/** Maximum distance for actual snapping when released (in Unreal units, cm) */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Snap")
	float SnapDistance = 50.0f;

	/** Corresponding Assembly Actor (if any) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part")
	TSoftObjectPtr<AAssemblyActor> AssemblyActor = nullptr;

	/** Preview opacity (0.0 to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap Preview")
	float PreviewOpacity = 0.3f;

	/** Preview color tint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap Preview")
	FLinearColor PreviewColor = FLinearColor::Green;

	/** Currently showing preview? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snap Preview")
	bool bShowingPreview = false;

	UFUNCTION(BlueprintCallable, Category = "Grab State")
	bool IsAttachedToMotionController() const;

	// Preview functions
	/** Show preview of where this part will snap */
	UFUNCTION(BlueprintCallable, Category = "Snap Preview")
	void ShowSnapPreview();
	void ShowSnapPreviewInternal(USnapPointComponent* SourceSnapPoint, USnapPointComponent* TargetSnapPoint);
	void OnPartGrabbed();
	void OnPartReleased();

	/** Hide the snap preview */
	UFUNCTION(BlueprintCallable, Category = "Snap Preview")
	void HideSnapPreview();

	// /** Calculate where this part should be positioned when snapped */
	UFUNCTION(BlueprintCallable, Category = "Snap Preview")
	FTransform CalculateSnapTransform(USnapPointComponent* SourceSnapPoint, USnapPointComponent* TargetSnapPoint) const;


	/** Snap system for this part */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Part")
	TObjectPtr<UAssemblyComponent> Assembly;

	/** Validator for lesson logic */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Part")
	TObjectPtr<USnapValidatorComponent> SnapValidator;

	/** Returns all snap points on this part */
	UFUNCTION(BlueprintCallable, Category = "Part")
	const TArray<USnapPointComponent*> GetSnapPoints() const;




	// Current nearby parts for snapping
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snap Detection")
	TArray<TObjectPtr<APartActor>> NearbyParts;

	
	

protected:


    
	// /** Clear snap highlighting for a specific part */
	// UFUNCTION(BlueprintCallable, Category = "Snap Detection")
	// void ClearSnapHighlight(APartActor* OtherPart);
  

	/** Maximum distance for snap detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap Detection")
	float MaxSnapDistance = 50.0f;
    
	/** Current snap candidate (if any) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snap Detection")
	TObjectPtr<APartActor> SnapCandidate;
    
	/** My snap point that would be used for snapping */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snap Detection")
	TObjectPtr<USnapPointComponent> MySnapPoint;
    
	/** The other part's snap point that would be used for snapping */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snap Detection")
	TObjectPtr<USnapPointComponent> CandidateSnapPoint;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
