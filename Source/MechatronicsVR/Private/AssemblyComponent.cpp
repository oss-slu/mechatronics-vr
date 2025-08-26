// Fill out your copyright notice in the Description page of Project Settings.

#include "AssemblyComponent.h"
#include "SnapPointComponent.h"


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
	// Draw snap point debug visualization if enabled
#if WITH_EDITOR
    if (bShowSnapPointDebug)
    {
        for (const USnapPointComponent* SnapPoint : SnapPoints)
        {
            if (!SnapPoint)
                continue;
            
            FVector Location = SnapPoint->GetComponentLocation();
            FColor DrawColor = SnapPoint->bIsAssembled ? FColor::Red : FColor::Green;
            
            // Draw sphere at snap point location
            DrawDebugSphere(
                GetWorld(),
                Location,
                5.0f, // Radius
                12,   // Segments
                DrawColor,
                false, // Persistent lines
                -1.0f  // Lifetime (-1 = single frame)
            );
            
            // Draw arrow showing snap direction
            FVector ForwardVector = SnapPoint->GetForwardVector();
            DrawDebugDirectionalArrow(
                GetWorld(),
                Location,
                Location + (ForwardVector * 15.0f),
                5.0f,
                DrawColor,
                false,
                -1.0f,
                0,
                2.0f
            );
            
            // Draw snap point ID as text
            DrawDebugString(
                GetWorld(),
                Location + FVector(0, 0, 10.0f),
                SnapPoint->SnapID.ToString(),
                nullptr,
                DrawColor,
                0.0f,
                true
            );
            
            // Optional: Draw detection radius
            if (SnapPoint->SnapDetectionSphere)
            {
	            const float DetectionRadius = SnapPoint->SnapDetectionSphere->GetScaledSphereRadius();
                DrawDebugSphere(
                    GetWorld(),
                    Location,
                    DetectionRadius,
                    16,
                    FColor::Cyan,
                    false,
                    -1.0f,
                    0,
                    1.0f // Thinner line
                );
            }
        }
    }
#endif
	
}
