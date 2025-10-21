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
#include "MechatronicsGameMode.h"
#include "PartActor.h"

// Sets default values for this component's properties
UGrabRayCaster::UGrabRayCaster()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	// ...
}


// Also, if the pulled objects collides with a physics object, the pull can be off (could interp location, but I would rather it be more physical)
void UGrabRayCaster::SuckObjectToSource() const
{
	UStaticMeshComponent* MeshComponent = ActorRef->FindComponentByClass<UStaticMeshComponent>();
	if (UPrimitiveComponent* PrimitiveComp = MeshComponent)
	{
		if (const FBodyInstance* BodyInst = PrimitiveComp->GetBodyInstance())
		{
			const FVector CenterOfMassLocation = BodyInst->GetCOMPosition();
			const FVector TargetLocation = MotionControllerRef->GetComponentLocation();
            
			// Get current velocity
			const FVector CurrentVelocity = PrimitiveComp->GetPhysicsLinearVelocity();
            
			// Calculate required velocity to reach target in SuckTime
			const FVector RequiredVelocity = (TargetLocation - CenterOfMassLocation) / SuckTime;
            
			// Calculate the change in velocity needed
			const FVector DeltaVelocity = RequiredVelocity - CurrentVelocity;
            
			// Correct impulse calculation: J = m * Δv
			const float Mass = PrimitiveComp->GetMass();
			const FVector Impulse = Mass * DeltaVelocity;
            
			// Disable gravity
			PrimitiveComp->SetEnableGravity(false);
            
			// Apply the impulse
			if (MeshComponent)
            {
                if (MeshComponent->IsSimulatingPhysics())
                {
                    MeshComponent->AddImpulse(Impulse);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("SuckObjectToSource: Enabling physics and applying impulse"));
                    MeshComponent->SetSimulatePhysics(true);
                    MeshComponent->WakeRigidBody();
                    MeshComponent->AddImpulse(Impulse);
                }
            }
            
			// Re-enable gravity after SuckTime
			FTimerHandle TimerHandle;
			ActorRef->GetWorldTimerManager().SetTimer(TimerHandle, [PrimitiveComp]()
			{
				if (IsValid(PrimitiveComp))
				{
					PrimitiveComp->SetEnableGravity(true);
				}
			}, SuckTime, false);  // Use SuckTime instead of 0.2f
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

					if (APartActor* PartActor = Cast<APartActor>(GrabActor))
					{
						if (UWorld* World = GetWorld())
						{
							if (AGameModeBase* GameMode = World->GetAuthGameMode())
							{
								if (AMechatronicsGameMode* MechGameMode = Cast<AMechatronicsGameMode>(GameMode))
								{
									if (ULessonUIManagerComponent* UIManager = MechGameMode->GetUIManager())
									{
										UIManager->PauseHighlightForPart(PartActor);
									}
								}
							}
							
						}
					}
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

