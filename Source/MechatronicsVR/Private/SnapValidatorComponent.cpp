// Fill out your copyright notice in the Description page of Project Settings.


#include "MechatronicsVR/Public/SnapValidatorComponent.h"

// Sets default values for this component's properties
USnapValidatorComponent::USnapValidatorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

bool USnapValidatorComponent::isSnapValid(USnapPointComponent* SnapPoint,
	const USnapPointComponent* TargetSnapPoint) const
{
	if (!SnapPoint || !TargetSnapPoint) return false;
	return SnapPoint->CanAcceptPoint(TargetSnapPoint);
}
void USnapValidatorComponent::OnSnapCompleted(USnapPointComponent* SnapPoint, USnapPointComponent* TargetSnapPoint)
{
}

bool USnapValidatorComponent::CanBeDisassembled(USnapPointComponent* SnapPoint) const
{
	//default
	if (!SnapPoint) return false;
	return true;
}


// Called when the game starts
void USnapValidatorComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USnapValidatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

