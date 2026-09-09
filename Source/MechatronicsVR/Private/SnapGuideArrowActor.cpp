// Fill out your copyright notice in the Description page of Project Settings.

#include "SnapGuideArrowActor.h"
#include "SnapPointComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

ASnapGuideArrowActor::ASnapGuideArrowActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->SetupAttachment(RootComponent);
	ArrowMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	ArrowMesh->SetGenerateOverlapEvents(false);
	ArrowMesh->CastShadow = false;

	// Attempt to load the default starter content arrow mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ArrowMeshAsset(TEXT("/Game/StarterContent/Blueprints/Assets/SM_Arrows.SM_Arrows"));
	if (ArrowMeshAsset.Succeeded())
	{
		ArrowMesh->SetStaticMesh(ArrowMeshAsset.Object);
		// Scale appropriately for VR visualization
		ArrowMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
	}

	// Initially hidden until PointToSnapPoint is called
	SetActorHiddenInGame(true);
}

void ASnapGuideArrowActor::BeginPlay()
{
	Super::BeginPlay();
}

void ASnapGuideArrowActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TargetSnapPoint && !IsHidden())
	{
		UpdateArrowTransform(DeltaTime);
	}
}

void ASnapGuideArrowActor::PointBetween(AActor* InSourceActor, USnapPointComponent* InTargetSnapPoint, float InSourceOffset)
{
	if (!InSourceActor || !InTargetSnapPoint)
	{
		ClearTargetSnapPoint();
		return;
	}

	SourceActor = InSourceActor;
	TargetSnapPoint = InTargetSnapPoint;
	SourceOffset = InSourceOffset;
	AnimationTimer = 0.0f;

	ShowGuide();
	UpdateArrowTransform(0.0f);
}

void ASnapGuideArrowActor::PointToSnapPoint(USnapPointComponent* InTargetSnapPoint, float InHoverDistance)
{
	if (!InTargetSnapPoint)
	{
		ClearTargetSnapPoint();
		return;
	}

	SourceActor = nullptr;
	TargetSnapPoint = InTargetSnapPoint;
	HoverDistance = InHoverDistance;
	AnimationTimer = 0.0f;

	ShowGuide();
	UpdateArrowTransform(0.0f);
}

void ASnapGuideArrowActor::ClearTargetSnapPoint()
{
	SourceActor = nullptr;
	TargetSnapPoint = nullptr;
	HideGuide();
}

void ASnapGuideArrowActor::ShowGuide()
{
	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);
}

void ASnapGuideArrowActor::HideGuide()
{
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
}

void ASnapGuideArrowActor::UpdateArrowTransform(float DeltaTime)
{
	if (!TargetSnapPoint)
	{
		return;
	}

	AnimationTimer += DeltaTime;

	// Mode 1: Source-to-target tracking (starts at the object to be placed and points to destination)
	if (SourceActor.IsValid())
	{
		const FVector SourceLocation = SourceActor->GetActorLocation();
		const FVector TargetLocation = TargetSnapPoint->GetComponentLocation();
		const FVector DeltaVec = TargetLocation - SourceLocation;
		const float TotalDistance = DeltaVec.Size();

		if (TotalDistance > KINDA_SMALL_NUMBER)
		{
			const FVector ForwardDir = DeltaVec.GetSafeNormal();

			// Apply subtle sine bobbing along the path vector
			float OffsetDist = SourceOffset;
			if (bEnableBobbing)
			{
				OffsetDist += FMath::Sin(AnimationTimer * BobbingSpeed * 2.0f * PI) * BobbingAmplitude;
			}

			// Clamp offset so arrow doesn't overshoot target if very close
			OffsetDist = FMath::Clamp(OffsetDist, 0.0f, TotalDistance * 0.8f);

			const FVector ArrowLocation = SourceLocation + (ForwardDir * OffsetDist);
			FRotator ArrowRotation = ForwardDir.Rotation();

			if (bEnableSpin)
			{
				ArrowRotation.Roll = FMath::Fmod(AnimationTimer * SpinRate, 360.0f);
			}

			SetActorLocationAndRotation(ArrowLocation, ArrowRotation);
			return;
		}
	}

	// Mode 2: Fallback to socket hover indicator (points into socket from front approach)
	const FVector SnapLocation = TargetSnapPoint->GetComponentLocation();
	const FVector OutwardVector = TargetSnapPoint->GetForwardVector();

	// Hover distance along the outward vector plus sine-wave bobbing
	float CurrentDistance = HoverDistance;
	if (bEnableBobbing)
	{
		CurrentDistance += FMath::Sin(AnimationTimer * BobbingSpeed * 2.0f * PI) * BobbingAmplitude;
	}

	// Position arrow hovering in front of the snap socket
	const FVector NewLocation = SnapLocation + (OutwardVector * CurrentDistance);

	// Rotate arrow to point into the socket (opposite of OutwardVector)
	const FVector InwardDirection = -OutwardVector;
	FRotator NewRotation = InwardDirection.Rotation();

	if (bEnableSpin)
	{
		NewRotation.Roll = FMath::Fmod(AnimationTimer * SpinRate, 360.0f);
	}

	SetActorLocationAndRotation(NewLocation, NewRotation);
}
