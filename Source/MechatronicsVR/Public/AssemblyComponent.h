// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "AssemblyComponent.generated.h"

class USnapPointComponent;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MECHATRONICSVR_API UAssemblyComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAssemblyComponent();
	
	/** All snap points belonging to this part (gathered at BeginPlay) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Assembly")
	TArray<TObjectPtr<USnapPointComponent>> SnapPoints;

	/** Refresh SnapPoints list by scanning children */
	UFUNCTION(BlueprintCallable, Category = "Assembly")
	void RegisterSnapPoints();

	/** Return all snap points on this part */
	UFUNCTION(BlueprintCallable, Category = "Assembly")
	TArray<USnapPointComponent*> GetSnapPoints() const;

	//snap point debug
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Assembly|Debug")
	bool bShowSnapPointDebug = false;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
