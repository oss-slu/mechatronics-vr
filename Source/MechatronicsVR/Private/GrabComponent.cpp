// Fill out your copyright notice in the Description page of Project Settings.


#include "GrabComponent.h"
#include "MotionControllerComponent.h"
#include "PartActor.h"

// Sets default values for this component's properties
UGrabComponent::UGrabComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryGrabRelativeRotation = FRotator::ZeroRotator;
	
	// ...
}


bool UGrabComponent::TryGrab(UMotionControllerComponent* MotionController, bool bIsSecondaryGrab)
{
	
	if (!MotionController || bIsHeld)
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	// Store grab rotation for secondary grab
	
	// Get the parent component (what the macro attaches)
	USceneComponent* ParentToAttach = GetAttachParent();
	if (!ParentToAttach)
	{
		ParentToAttach = Owner->GetRootComponent();
	}

	// Store grab rotation for secondary grab
	if (bIsSecondaryGrab && PrimaryGrabComponent)
	{
		PrimaryGrabRelativeRotation = ParentToAttach->GetRelativeRotation();
	}


	// Store references before attachment
	MotionControllerRef = MotionController;
	bIsHeld = true;

	// Disable physics
	if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
	{
		RootPrimitive->SetSimulatePhysics(false);
		RootPrimitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	// Attach parent to motion controller (matching the macro)

	if (const bool bAttachmentSuccessful = ParentToAttach->AttachToComponent(
		MotionController,
		FAttachmentTransformRules(
			EAttachmentRule::KeepWorld,    // Location Rule
			EAttachmentRule::KeepWorld,    // Rotation Rule  
			EAttachmentRule::KeepWorld,    // Scale Rule
			true                            // Weld Simulated Bodies
		)
	); !bAttachmentSuccessful)
	{
		//attachment failed, reset state
		bIsHeld=false;
		MotionControllerRef=nullptr;

		UE_LOG(LogTemp, Error, TEXT("GrabComponent: Failed to attach to motion controller"));
		return false;
	}

	// Play haptic feedback
	if (OnGrabHapticEffect)
	{
		// Get the player controller from the motion controller's owner
		if (const APawn* Pawn = Cast<APawn>(MotionController->GetOwner()))
		{
			if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
			{
				//determine which hand
				EControllerHand Hand = EControllerHand::Right;
				if (MotionController->MotionSource == FName("Left"))
				{
					Hand = EControllerHand::Right;
				}
				PC->PlayHapticEffect(OnGrabHapticEffect, Hand);
			}
		}
	}
	// Fire events
	OnGrabbed.Broadcast();
    
    // Notify PartActor
    if (APartActor* Part = Cast<APartActor>(Owner))
    {
        Part->OnPartGrabbed();
    }

    return true;
}

bool UGrabComponent::TryRelease()
{
	if (!bIsHeld) return false;

	AActor *Owner = GetOwner();
	if (Owner)
	{
		// Detach from motion controller
		Owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		if (bSimulateOnDrop)
		{
			if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
			{
				// Re-enable physics
				RootPrimitive->SetSimulatePhysics(true);
			}
		}
	}

	// Clear references
	bIsHeld = false;
	MotionControllerRef = nullptr;
	PrimaryGrabComponent = nullptr;

	//fire the dropped event
	// Notify the PartActor if it is one
	if (Owner)
	{
		if (APartActor* Part = Cast<APartActor>(Owner))
		{
			Part->OnPartReleased();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("GrabComponent: Released"));

	return true;
}

FName UGrabComponent::GetHeldByHand() const
{
	if (MotionControllerRef)
	{
		return MotionControllerRef->MotionSource;
	}
	return NAME_None;
}

// Called when the game starts
void UGrabComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

