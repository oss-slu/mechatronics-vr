// Fill out your copyright notice in the Description page of Project Settings.


#include "PartPlacementGuide.h"
#include "GameFramework/PlayerController.h"


void UPartPlacementGuide::DrawArrow(const APlayerController* PlayerController, const USnapPointComponent* SnapPointSource, const USnapPointComponent* SnapPointDestination)
{
	FVector2D LineStartOnScreen;
	PlayerController->ProjectWorldLocationToScreen(SnapPointSource->GetComponentTransform().GetLocation(),LineStartOnScreen);
	FVector2D LineEndOnScreen;
	PlayerController->ProjectWorldLocationToScreen(SnapPointDestination->GetComponentTransform().GetLocation(),LineEndOnScreen);

	

}
