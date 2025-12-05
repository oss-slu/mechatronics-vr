// Fill out your copyright notice in the Description page of Project Settings.


#include "VRSmoothMovementComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UVRSmoothMovementComponent::UVRSmoothMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UVRSmoothMovementComponent::BeginPlay()
{
    Super::BeginPlay();

    // Get camera component from owner
    Camera = GetOwner()->FindComponentByClass<UCameraComponent>();

    // Try to get the motion controller (left or right)
    TArray<UMotionControllerComponent*> Controllers;
    GetOwner()->GetComponents<UMotionControllerComponent>(Controllers);
    if (Controllers.Num() > 0)
    {
        // Optionally choose left or right here. Defaulting to first found.
        MotionController = Controllers[0];
    }

    PreviousPosition = GetOwner()->GetActorLocation();
}

void UVRSmoothMovementComponent::MoveWithThumbstickInput(float AxisX, float AxisY)
{
    if (!bSmoothMovementEnabled || !Camera) return;

    if (FMath::IsNearlyZero(AxisX) && FMath::IsNearlyZero(AxisY)) return;

    UE_LOG(LogTemp, Warning, TEXT("Thumbstick Vector: X=%f, Y=%f"), AxisX, AxisY);

    // Get camera forward/right vectors (ignoring pitch)
    FVector Forward = Camera->GetForwardVector();
    Forward.Z = 0.f;
    Forward.Normalize();

    FVector Right = Camera->GetRightVector();
    Right.Z = 0.f;
    Right.Normalize();
    
    PendingInput = (Forward * AxisY) + (Right * AxisX);
    if (!PendingInput.IsNearlyZero())
    {
        PendingInput.Normalize();
    }
}

void UVRSmoothMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bSmoothMovementEnabled && !PendingInput.IsNearlyZero())
    {
        FVector DesiredMovement = PendingInput * MoveSpeed * DeltaTime;
        FVector CurrentLocation = GetOwner()->GetActorLocation();
        FVector CurrentMovement = CurrentLocation - PreviousPosition;

        // Smooth interpolation
        FVector SmoothedMovement = FMath::VInterpTo(CurrentMovement, DesiredMovement, DeltaTime, 10.f);

        // Move the actor
        GetOwner()->AddActorWorldOffset(SmoothedMovement, true);

        PreviousPosition = GetOwner()->GetActorLocation();
    }

    // Reset input for next frame
    PendingInput = FVector::ZeroVector;
}
