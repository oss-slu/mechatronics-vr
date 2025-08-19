// Fill out your copyright notice in the Description page of Project Settings.

#include "SnapPointComponent.h"
#include "MechatronicsVR/Public/AssemblyComponent.h"

// Sets default values for this component's properties
UAssemblyComponent::UAssemblyComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UAssemblyComponent::RegisterSnapPoints()
{
	SnapPoints.Empty();
	TArray<USceneComponent*> Children;
	GetChildrenComponents(true, Children);
	for (USceneComponent* Child : Children)
	{
		if (USnapPointComponent* SnapPoint = Cast<USnapPointComponent>(Child))
		{
			SnapPoints.Add(SnapPoint);
		}
	}
}

 TArray<USnapPointComponent*> UAssemblyComponent::GetSnapPoints() const
{ 
	TArray<USnapPointComponent*> Result;
	for (TObjectPtr<USnapPointComponent> SnapPoint : SnapPoints)
	{
		if (SnapPoint)
		{
			Result.Add(SnapPoint.Get());
		}
	}
	return Result;
}


// Called when the game starts
void UAssemblyComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	RegisterSnapPoints();
	
}


// Called every frame
void UAssemblyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

