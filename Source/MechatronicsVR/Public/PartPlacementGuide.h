// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SnapPointComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PartPlacementGuide.generated.h"

/**
 * 
 */
UCLASS()
class MECHATRONICSVR_API UPartPlacementGuide : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "PartPlacementGuides")
	static void DrawArrow(const APlayerController* PlayerController, const USnapPointComponent* SnapPointSource, const USnapPointComponent* SnapPointDestination);
};
