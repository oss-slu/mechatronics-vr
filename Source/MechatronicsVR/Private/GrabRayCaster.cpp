// Fill out your copyright notice in the Description page of Project Settings.


#include "GrabRayCaster.h"
#include "Engine/World.h"
#include "MotionControllerComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UGrabRayCaster::UGrabRayCaster()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	// ...
}

void UGrabRayCaster::DeselectObject()
{
	// if referencing an object previous tick
	if (ActorRef != nullptr)
	{
		// set overlay material to none
		UStaticMeshComponent* MeshComponent = ActorRef->FindComponentByClass<UStaticMeshComponent>();
		if (MeshComponent)
		{
			MeshComponent->SetOverlayMaterial(nullptr);
		}
		// dereference object
		ActorRef = nullptr;
	}
}


void UGrabRayCaster::CheckReachForGrabComponent()
{
	FVector ComponentLocation = MotionControllerRef->GetComponentLocation();
	
	FVector LocalForward = FVector::ForwardVector; // (1, 0, 0)

	// Apply rotation offset relative to controller's rotation
	FRotator HandOffsetRotation(-65.f, IsLeft ? 60.f : -60.f, 0.f); // Manual values
	FVector AdjustedDirection = MotionControllerRef->GetComponentRotation().RotateVector(HandOffsetRotation.RotateVector(LocalForward));
	
	FVector CastEndPoint = ComponentLocation + (AdjustedDirection * CastRange);

	
	FHitResult HitResult;
	FCollisionQueryParams TraceParams(FName(TEXT("GrabTrace")), true, GetOwner());
	TraceParams.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->LineTraceSingleByObjectType(
		HitResult,
		ComponentLocation,
		CastEndPoint,
		ECC_PhysicsBody,
		TraceParams
	);

	DrawDebugLine(GetWorld(), ComponentLocation, CastEndPoint, bHit ? FColor::Green : FColor::Red, false, 0.f, 0, 1.f);

	if (bHit)
	{
		if (AActor* GrabActor = HitResult.GetActor())
		{
			UE_LOG(LogTemp, Log, TEXT("Hit GrabComponent Actor: %s"), *GrabActor->GetName());

			// action for if you high different objects on consecutive ticks
			if (ActorRef != GrabActor)
			{
				DeselectObject();
			}

			// save reference to object
			ActorRef = GrabActor;

			UStaticMeshComponent* MeshComponent = GrabActor->FindComponentByClass<UStaticMeshComponent>();

			if (MeshComponent && OverlayMaterial)
			{
				MeshComponent->SetOverlayMaterial(OverlayMaterial);
			}

			// grab logic goes here TBD 
		}
	}
	else
	{
		DeselectObject();
	}
}



// Called when the game starts
void UGrabRayCaster::BeginPlay()
{
	Super::BeginPlay();

	// Get reference to parent motion controller
	MotionControllerRef = Cast<UMotionControllerComponent>(GetAttachParent());	// Attached to motion controller in blueprint for VRPawn
	if (MotionControllerRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found parent Motion Controller: %s"), *MotionControllerRef->GetName());
		if (MotionControllerRef->GetName() == TEXT("MotionControllerLeftGrip"))	// need a better way for this
		{
			IsLeft = true;
		}
	}
	// ...
}


// Called every frame
void UGrabRayCaster::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CheckReachForGrabComponent();
	
	// ...
}

