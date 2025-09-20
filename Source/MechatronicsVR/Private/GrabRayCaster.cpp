// Fill out your copyright notice in the Description page of Project Settings.


#include "GrabRayCaster.h"
#include "Engine/World.h"
#include "GrabComponent.h"
#include "MotionControllerComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UGrabRayCaster::UGrabRayCaster()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	// ...
}

// KNOWN BUG: Sometimes will pull object too far (past the hand)
// Also, if the pulled objects collides with a physics object, the pull can be off (could interp location, but I would rather it be more physical)
void UGrabRayCaster::SuckObjectToSource()
{
	UStaticMeshComponent* MeshComponent = ActorRef->FindComponentByClass<UStaticMeshComponent>();
	if (UPrimitiveComponent* PrimitiveComp = MeshComponent)
	{
		if (FBodyInstance* BodyInst = PrimitiveComp->GetBodyInstance())
		{
			FVector CenterOfMassLocation = BodyInst->GetCOMPosition();
			
			FVector Velocity = (MotionControllerRef->GetComponentLocation() - CenterOfMassLocation) / SuckTime;

			float Mass = PrimitiveComp->GetMass();
			FVector Impulse = Velocity * Mass * 2;

			// disable gravity for duration of suck
			// I realize I could have used the word "pull" instead of "suck" but it's too late now
			PrimitiveComp->SetEnableGravity(false);
			FTimerHandle TimerHandle;
			ActorRef->GetWorldTimerManager().SetTimer(TimerHandle, [PrimitiveComp]()
			{
				PrimitiveComp->SetEnableGravity(true);
			}, 0.2f, false);
			
			if (MeshComponent && MeshComponent->IsSimulatingPhysics())
			{
				MeshComponent->AddImpulse(Impulse);
			}
		}
	}
}


void UGrabRayCaster::DeselectObject()
{
	// if referencing an object previous tick
	if (ActorRef != nullptr)
	{
		// set overlay material to none
		if (UStaticMeshComponent* MeshComponent = ActorRef->FindComponentByClass<UStaticMeshComponent>())
		{
			MeshComponent->SetOverlayMaterial(nullptr);
		}
		// dereference object
		ActorRef = nullptr;
	}
}

UGrabComponent* UGrabRayCaster::CheckReachForGrabComponent()
{
	FVector ComponentLocation = MotionControllerRef->GetComponentLocation();
	
	FVector LocalForward = FVector::ForwardVector; // (1, 0, 0)

	// Apply rotation offset relative to controller's rotation
	FRotator HandOffsetRotation(-58.f, IsLeft ? 50.f : -50.f, 0.f); // Manual values (close-ish, not perfect)
	FVector AdjustedDirection = MotionControllerRef->GetComponentRotation().RotateVector(HandOffsetRotation.RotateVector(LocalForward));
	
	FVector CastEndPoint = ComponentLocation + (AdjustedDirection * CastRange);

	// Create the capsule collision shape
	FCollisionShape SweepShape = FCollisionShape::MakeSphere(CapsuleRadius);
	
	FHitResult HitResult;
	
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	
	FCollisionQueryParams QueryParams;

	// ray cast sweep with sphere
	bool bHit = GetWorld()->SweepSingleByObjectType(
		HitResult,
		ComponentLocation,
		CastEndPoint,
		FQuat::Identity,
		ObjectQueryParams,
		SweepShape,
		QueryParams
	);

	/*
	// debug
	if (bHit)
	{
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, CapsuleRadius, 16,FColor::Green, false, 0.0f);
	}
	else
	{
		DrawDebugSphere(GetWorld(), CastEndPoint, CapsuleRadius,16,  FColor::Red, false, 0.0f);
	}
	*/
	
	if (bHit)
	{
		if (AActor* GrabActor = HitResult.GetActor())
		{
			if (UGrabComponent* GrabComponent = GrabActor->GetComponentByClass<UGrabComponent>())
			{
				UE_LOG(LogTemp, Log, TEXT("Hit GrabComponent Actor: %s"), *GrabActor->GetName());

				// action for if you hit different objects on consecutive ticks
				if (ActorRef != GrabActor)
				{
					DeselectObject();
				}

				// save reference to object
				ActorRef = GrabActor;

				UStaticMeshComponent* MeshComponent = GrabActor->FindComponentByClass<UStaticMeshComponent>();
				
				if (GrabComponent->bIsHeld)
				{
					MeshComponent->SetOverlayMaterial(nullptr);
				}
				else if (MeshComponent && OverlayMaterial)
				{
					MeshComponent->SetOverlayMaterial(OverlayMaterial);
				}
				return GrabComponent;
			}
		}
	}
	else
	{
		DeselectObject();
	}
	return nullptr;
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

