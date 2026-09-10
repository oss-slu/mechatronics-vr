// Fill out your copyright notice in the Description page of Project Settings.


#include "LessonUIManagerComponent.h"
#include "PartActor.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/Engine.h"
#include "LessonUIActor.h"

// Sets default values for this component's properties
ULessonUIManagerComponent::ULessonUIManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// Default settings
	InstructionVisibility = ELessonUIVisibility::OnlyDuringSteps;
	ProgressVisibility = ELessonUIVisibility::Always;
	DefaultHighlightType = EHighlightType::Outline;

	HighlightColor = FLinearColor::Green;
	TargetPartColor = FLinearColor::Yellow;
	HighlightIntensity = 1.5f;
	PulseSpeed = 2.0f;

	FadeInDuration = 0.5f;
	FadeOutDuration = 0.3f;
	bAnimateInstructions = true;
	bAnimateProgress = true;

	AudioVolume = 0.7f;

	CurrentProgress = 0.0f;
	CurrentStepNumber = 0;
	TotalSteps = 0;
	bUIVisible = true;

	// ...
}


void ULessonUIManagerComponent::DebugShowWidgetLocation()
{
	if (!InstructionWidgetComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("DebugShowWidgetLocation: No widget component"));
		return;
	}
    
	FVector WidgetLocation = InstructionWidgetComponent->GetComponentLocation();
    
	// Draw debug sphere at widget location
	DrawDebugSphere(
		GetWorld(),
		WidgetLocation,
		50.0f,
		12,
		FColor::Green,
		false,
		5.0f,
		0,
		5.0f
	);
    
	// Draw line from player to widget
	if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
	{
		DrawDebugLine(
			GetWorld(),
			PlayerPawn->GetActorLocation(),
			WidgetLocation,
			FColor::Yellow,
			false,
			5.0f,
			0,
			3.0f
		);
	}
    
	UE_LOG(LogTemp, Warning, TEXT("DebugShowWidgetLocation: Widget at %s"), *WidgetLocation.ToString());
}

// Called when the game starts
void ULessonUIManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	UE_LOG(LogTemp, Log, TEXT("LessonUIManagerComponent::BeginPlay - Initializing UI"));
	RefreshWidgetReferences();

	// UE_LOG(LogTemp, Log, TEXT("LessonUIManagerComponent::BeginPlay - UI Initialized"));
	//
	// if (InstructionWidget && InstructionWidgetComponent)
	// {
	// 	GetWorld()->GetTimerManager().SetTimer(DebugTestTimer, [this]()
	// 	{
	// 		UE_LOG(LogTemp, Error, TEXT("=== FORCING WIDGET VISIBLE FOR TEST ==="));
 //        
	// 		InstructionWidgetComponent->SetVisibility(true);
	// 		InstructionWidgetComponent->SetRelativeLocation(FVector(200, 0, 0));  // Right in front
 //        
	// 		if (UTextBlock* TextBlock = Cast<UTextBlock>(InstructionWidget->GetWidgetFromName("InstructionText")))
	// 		{
	// 			TextBlock->SetText(FText::FromString("TEST - CAN YOU SEE ME?"));
	// 			UE_LOG(LogTemp, Error, TEXT("Set test text"));
	// 		}
	// 		else
	// 		{
	// 			UE_LOG(LogTemp, Error, TEXT("Could not find TextBlock named 'InstructionText'"));
	// 		}
 //        
	// 		DebugShowWidgetLocation();
 //        
	// 	}, 2.0f, false);
	// }
	//
}

void ULessonUIManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearHighlights();

	//clean up timers
	// Clean up timers
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	//cclean up widgets
	DestroyUIWidgets();

	Super::EndPlay(EndPlayReason);
}





void ULessonUIManagerComponent::DestroyUIWidgets()
{
	if (InstructionWidgetComponent)
	{
		InstructionWidgetComponent->DestroyComponent();
		InstructionWidgetComponent = nullptr;
		InstructionWidget = nullptr;
	}
	
	if (ProgressWidgetComponent)
	{
		ProgressWidgetComponent->DestroyComponent();
		ProgressWidgetComponent = nullptr;
		ProgressWidget = nullptr;
	}
	
	if (LessonHUDComponent)
	{
		LessonHUDComponent->DestroyComponent();
		LessonHUDComponent = nullptr;
		LessonHUD = nullptr;
	}
	
	UE_LOG(LogTemp, Log, TEXT("DestroyUIWidgets: Destroyed all UI widgets"));
}
// === 3D POSITIONING FUNCTIONS ===
void ULessonUIManagerComponent::PositionInstructionWidget(const FVector& WorldLocation, const FRotator& WorldRotation) const
{
	if (InstructionWidgetComponent)
	{
		InstructionWidgetComponent->SetWorldLocation(WorldLocation);
		InstructionWidgetComponent->SetWorldRotation(WorldRotation);

		UE_LOG(LogTemp, Log, TEXT("PositionInstructionWidget: Set to location %s"), *WorldLocation.ToString());
	}
}

void ULessonUIManagerComponent::PositionWidgetNearActor(AActor* TargetActor, const FVector& Offset)
{
	if (!TargetActor || !InstructionWidgetComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("PositionWidgetNearActor: Invalid target actor"));
		return;
	}

	const FVector ActorLocation = TargetActor->GetActorLocation();
	const FVector WidgetPosition = ActorLocation + Offset;

	PositionInstructionWidget(WidgetPosition);

	//optionally face the player
	if (bInstructionLooksAtPlayer)
	{
		const FVector PlayerLocation = GetPlayerLocation();
		const FVector LookDirection = (PlayerLocation - WidgetPosition).GetSafeNormal();
		const FRotator LookRotation = FRotationMatrix::MakeFromX(LookDirection).Rotator();
		InstructionWidgetComponent->SetWorldRotation(LookRotation);
	}
	UE_LOG(LogTemp, Log, TEXT("PositionWidgetNearActor: Positioned near %s"), *TargetActor->GetName());
}

void ULessonUIManagerComponent::PositionWidgetNearPart(APartActor* TargetPart, const FVector& Offset)
{
	PositionWidgetNearActor(TargetPart, Offset);
}

void ULessonUIManagerComponent::SetWidgetFollowMode(bool bShouldFollow, AActor* ActorToFollow)
{
	bInstructionFollowsPlayer = bShouldFollow;
	FollowTarget = ActorToFollow;

	//enable/disable ticking based on follow mode
	PrimaryComponentTick.bCanEverTick = bShouldFollow || bInstructionLooksAtPlayer || bProgressFollowsPlayer || bProgressLooksAtPlayer;
	bTickEnabled = PrimaryComponentTick.bCanEverTick;

	UE_LOG(LogTemp, Log, TEXT("SetWidgetFollowMode: Follow = %s, Target = %s"), 
		   bShouldFollow ? TEXT("true") : TEXT("false"), 
		   ActorToFollow ? *ActorToFollow->GetName() : TEXT("Player"));
}

void ULessonUIManagerComponent::ResetWidgetPositions()
{
	if (InstructionWidgetComponent)
	{
		InstructionWidgetComponent->SetRelativeLocation(InstructionWidgetOffset);
		InstructionWidgetComponent->SetRelativeRotation(FRotator::ZeroRotator);
	}
	if (ProgressWidgetComponent)
	{
		ProgressWidgetComponent->SetRelativeLocation(ProgressWidgetOffset);
		ProgressWidgetComponent->SetRelativeRotation(FRotator::ZeroRotator);
	}
	if (LessonHUDComponent)
	{
		LessonHUDComponent->SetRelativeLocation(LessonHUDOffset);
		LessonHUDComponent->SetRelativeRotation(FRotator::ZeroRotator);
	}

	UE_LOG(LogTemp, Log, TEXT("ResetWidgetPositions: Reset to default offsets"));
}

// === INSTRUCTION MANAGEMENT FUNCTIONS ===
void ULessonUIManagerComponent::ShowInstructionWithIcon(const FText& InstructionText, UTexture2D* Icon)
{
	if (!InstructionWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowInstructionWithIcon: Instruction widget not available"));
		return;
	}

	//update instruction text
	if (UTextBlock* TextBlock = Cast<UTextBlock>(InstructionWidget->GetWidgetFromName("InstructionText")))
	{
		TextBlock->SetText(InstructionText);
	}
	// update icon if provided
	if (Icon && bShowInstructionIcons)
	{
		if (UImage* IconImage = Cast<UImage>(InstructionWidget->GetWidgetFromName("InstructionIcon")))
		{
			IconImage->SetBrushFromTexture(Icon);
			IconImage->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		if (UImage* IconImage = Cast<UImage>(InstructionWidget->GetWidgetFromName("InstructionIcon")))
		{
			IconImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	//play sound if sound enabled and exists
	if (bPlayInstructionSound && InstructionSound)
	{
		PlayUISound(InstructionSound);
	}

	// Broadcast event
	OnInstructionShown.Broadcast(InstructionText);
    
	UE_LOG(LogTemp, Log, TEXT("ShowInstructionWithIcon: Displayed instruction with icon"));
}

void ULessonUIManagerComponent::ShowInstructionForPart(const FString& InstructionText, APartActor* TargetPart)
{
	if (!TargetPart)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowInstructionForPart: Invalid target part"));
		return;
	}
    
	UE_LOG(LogTemp, Warning, TEXT("ShowInstructionForPart: Part=%s"), *TargetPart->GetName());
    
	// This will spawn the actor if needed
	ShowInstruction(InstructionText);
    
	// Position near the part
	if (LessonUIActor)
	{
		LessonUIActor->PositionNearActor(TargetPart, FVector(150.0f, 0.0f, 100.0f));
		UE_LOG(LogTemp, Warning, TEXT("ShowInstructionForPart: Positioned near part"));
	}
    
	// Highlight the part
	HighlightSinglePart(TargetPart, TargetPartColor);
}
void ULessonUIManagerComponent::HideInstruction()
{
	if (InstructionWidget)
	{
		if (bAnimateInstructions)
		{
			AnimateInstructionOut();
		}
		else
		{
			InstructionWidget->SetVisibility(ESlateVisibility::Hidden);
			if (InstructionWidgetComponent)
			{
				InstructionWidgetComponent->SetVisibility(false);
			}
		}
	}
    
	CurrentInstructionText = FText::GetEmpty();
	UE_LOG(LogTemp, Log, TEXT("HideInstruction: Hidden instruction widget"));
}

void ULessonUIManagerComponent::UpdateInstructionWidget(const FString& InstructionText) const
{
	ShowInstruction(InstructionText);
}


// === PROGRESS MANAGEMENT FUNCTIONS ===

void ULessonUIManagerComponent::UpdateProgress(float ProgressPercentage)
{
	CurrentProgress = FMath::Clamp(ProgressPercentage, 0.0f, 100.0f);
    
	UpdateProgressWidget(CurrentProgress);

	//play sound on significant milestones
	if (bPlayInstructionSound && ProgressSound)
	{
		const int32 OldMilestone = (int32)((CurrentProgress - 1.0f) / 25.0f);
		if (const int32 NewMilestone = (int32)(ProgressPercentage / 25.0f); NewMilestone > OldMilestone)
		{
			PlayUISound(ProgressSound);
		}
	}
	
	// Broadcast event
	OnProgressUpdated.Broadcast(CurrentProgress);
    
	UE_LOG(LogTemp, Verbose, TEXT("UpdateProgress: %.1f%%"), CurrentProgress);
}

void ULessonUIManagerComponent::UpdateStepProgress(int32 CurrentStep, int32 TotalStepsCount)
{
	CurrentStepNumber = CurrentStep;
	TotalSteps = TotalStepsCount;

	//calculate percentage

	float ProgressPercentage = TotalSteps > 0 ? 
	  ((float)(CurrentStep + 1) / (float)TotalSteps) * 100.0f : 0.0f;
    
	UpdateProgress(ProgressPercentage);

	//update step text if available
	if (ProgressWidget)
	{
		if (UTextBlock* StepText = Cast<UTextBlock>(ProgressWidget->GetWidgetFromName("StepText")))
		{
			const FString StepString = FString::Printf(TEXT("Step %d of %d"), CurrentStep + 1, TotalSteps);
			StepText->SetText(FText::FromString(StepString));
		}
	}
    
	UE_LOG(LogTemp, Log, TEXT("UpdateStepProgress: Step %d of %d"), CurrentStep + 1, TotalSteps);
}

void ULessonUIManagerComponent::UpdateProgressWidget(float ProgressPercentage)
{
	if (!ProgressWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateProgressWidget: Progress widget not available"));
		return;
	}

	// update progress bar
	if (UProgressBar* ProgressBar = Cast<UProgressBar>(ProgressWidget->GetWidgetFromName("ProgressBar")))
	{
		ProgressBar->SetPercent(ProgressPercentage / 100.0f);
	}

	//update percentage text
	if (UTextBlock* PercentageText = Cast<UTextBlock>(ProgressWidget->GetWidgetFromName("PercentageText")))
	{
		const FString PercentString = FString::Printf(TEXT("%.0f%%"), ProgressPercentage);
		PercentageText->SetText(FText::FromString(PercentString));
	}

	if (bAnimateProgress && ProgressPercentage >= 100.0f)
	{
		ShowSuccessAnimation();
	}
}

// === HIGHLIGHTING FUNCTIONS ===
void ULessonUIManagerComponent::HighlightParts(const TArray<APartActor*>& PartsToHighlight)
{
	HighlightPartsWithType(PartsToHighlight, DefaultHighlightType);
}

void ULessonUIManagerComponent::HighlightPartsWithType(const TArray<APartActor*>& PartsToHighlight, EHighlightType HighlightType)
{
	//clear existing highlights
	ClearHighlights();

	//apply new highlights
	for (APartActor* Part : PartsToHighlight)
	{
		if (Part)
		{
			ApplyHighlightToActor(Part, HighlightColor, HighlightType);
			HighlightedActors.Add(Part);
		}
	}

	if (bPulseHighlights && HighlightedActors.Num() > 0)
	{
		PrimaryComponentTick.bCanEverTick = true;
		SetComponentTickEnabled(true);
	}

	//play sound if enabled
	if (HighlightSound && PartsToHighlight.Num() > 0)
	{
		PlayUISound(HighlightSound);
	}
}

void ULessonUIManagerComponent::HighlightSinglePart(APartActor* PartToHighlight, const FLinearColor& Color)
{
	if (!PartToHighlight)
	{
		UE_LOG(LogTemp, Warning, TEXT("HighlightSinglePart: Invalid part"));
		return;
	}

	ApplyHighlightToActor(PartToHighlight, Color, EHighlightType::Pulse);

	if (!HighlightedActors.Contains(PartToHighlight))
	{
		HighlightedActors.Add(PartToHighlight);
	}

	UE_LOG(LogTemp, Log, TEXT("HighlightSinglePart: Highlighted part %s"), *PartToHighlight->GetName());
}

void ULessonUIManagerComponent::ClearHighlights()
{
	for (AActor* Actor : HighlightedActors)
	{
		RemoveHighlightFromActor(Actor);
	}
	HighlightedActors.Empty();


	UE_LOG(LogTemp, Log, TEXT("ClearHighlights: Cleared all highlights"));
}

void ULessonUIManagerComponent::ClearHighlight(APartActor* Part)
{
	if (!Part)
		return;
    
	RemoveHighlightFromActor(Part);
	HighlightedActors.Remove(Part);
	HighlightMaterials.Remove(Part);
    
	UE_LOG(LogTemp, Log, TEXT("ClearHighlight: Cleared highlight for %s"), *Part->GetName());
}

void ULessonUIManagerComponent::ApplyHighlightToActor(AActor* Actor, const FLinearColor& Color, EHighlightType HighlightType)
{
	if (!Actor) return;

	APartActor* Part = Cast<APartActor>(Actor);
	if (!Part || !Part->Mesh) return;

	switch (HighlightType)
	{
	case EHighlightType::Outline:
		{
			// Use custom depth for outline effect
			Part->Mesh->SetRenderCustomDepth(true);
			Part->Mesh->CustomDepthStencilValue = 1;
			break;
		}

	case EHighlightType::Glow:
	case EHighlightType::Material:
	case EHighlightType::Pulse:
		{
			// Use OVERLAY MATERIAL like GrabRayCaster does
			// Do EXACTLY what GrabRayCaster does, but with a DYNAMIC instance for pulsing
			if (HighlightMaterial)
			{
				UE_LOG(LogTemp, Error, TEXT("ApplyHighlightToActor: Creating dynamic material from template"));
        
				// Create dynamic instance from the static template
				UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(HighlightMaterial, this);
        
				if (DynMaterial)
				{
					// Set initial parameters
					DynMaterial->SetVectorParameterValue("HighlightColor", Color);
					DynMaterial->SetScalarParameterValue("HighlightIntensity", HighlightIntensity);
            
					// Apply as overlay
					Part->Mesh->SetOverlayMaterial(DynMaterial);
            
					// Store the DYNAMIC instance for pulsing
					HighlightMaterials.Add(Actor, DynMaterial);
            
					UE_LOG(LogTemp, Error, TEXT("ApplyHighlightToActor: Applied dynamic overlay material"));
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("ApplyHighlightToActor: Failed to create dynamic material"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ApplyHighlightToActor: No HighlightMaterial set!"));
			}
			break;
		}
	}
}

void ULessonUIManagerComponent::UpdatePulsingHighlights(float DeltaTime)
{
	PulseTimer += DeltaTime;
	float PulseValue = (FMath::Sin(PulseTimer * PulseSpeed) + 1.0f) / 2.0f; // Normalize to 0-1
	
	for (auto& Elem : HighlightMaterials)
	{

		// Skip if this part is currently being grab-highlighted
		if (GrabHighlightedActors.Contains(Elem.Key))
		{
			continue;
		}
		// Update intensity based on pulse values
		if (Elem.Value)
		{
			const float CurrentIntensity = FMath::Lerp(PulseMinIntensity, PulseMaxIntensity, PulseValue);
			Elem.Value->SetScalarParameterValue("HighlightIntensity", CurrentIntensity);
		}
	}
}
void ULessonUIManagerComponent::RemoveHighlightFromActor(AActor* Actor)
{
	if (!Actor) return;

	APartActor* Part = Cast<APartActor>(Actor);
	if (!Part || !Part->Mesh) return;

	// Remove outline
	Part->Mesh->SetRenderCustomDepth(false);

	// Remove overlay material (like GrabRayCaster does)
	Part->Mesh->SetOverlayMaterial(nullptr);
	
	// Clean up from our tracking
	HighlightMaterials.Remove(Actor);
	
	UE_LOG(LogTemp, Log, TEXT("RemoveHighlightFromActor: Removed highlight from %s"), *Part->GetName());
}


void ULessonUIManagerComponent::CreateHighlightMaterial(const FLinearColor& Color)
{
	// This would create a custom highlight material if needed
	// Implementation depends on your material setup
	//TODO: im not sure if this is needed but oh well its here for now
}



void ULessonUIManagerComponent::PulseHighlights()
{
	//trigger a pulse animation on all highlighted actors
	for (auto& Pair: HighlightMaterials)
	{
		if (Pair.Value)
		{
			// Reset pulse timer to start pulse effect
			Pair.Value->SetScalarParameterValue("TriggerPulse", 1.0f);
		}
	}
}


// === ANIMATION FUNCTIONS ===
void ULessonUIManagerComponent::FadeInUI()
{
	SetUIVisibility(true);

	//implement fade in animation

	GetWorld()->GetTimerManager().SetTimer(
		FadeInTimer,
		[this]()
		{
			// Animation complete
			GetWorld()->GetTimerManager().ClearTimer(FadeInTimer);
		},
		FadeInDuration,
		false
	);
}

void ULessonUIManagerComponent::FadeOutUI()
{
	// implement fade out animation
	GetWorld()->GetTimerManager().SetTimer(
		FadeOutTimer,
		[this]()
		{
			// Animation complete
			SetUIVisibility(false);
			GetWorld()->GetTimerManager().ClearTimer(FadeOutTimer);
		},
		FadeOutDuration,
		false
	);
}

void ULessonUIManagerComponent::AnimateInstructionIn()
{
	if (!InstructionWidget)
		return;

	//set initial state
	InstructionWidget->SetRenderOpacity(0.0f);
	InstructionWidget->SetVisibility(ESlateVisibility::Visible);

	// Animate opacity
	// Note: For proper animation, you'd typically use UMG animations or a timeline
	InstructionWidget->SetRenderOpacity(1.0f);
}


void ULessonUIManagerComponent::AnimateInstructionOut()
{
	if (!InstructionWidget)
		return;
    
	InstructionWidget->SetRenderOpacity(0.0f);
	InstructionWidget->SetVisibility(ESlateVisibility::Hidden);
}

void ULessonUIManagerComponent::ShowSuccessAnimation()
{
	// Play success animation on progress widget
	if (ProgressWidget)
	{
		// Trigger success animation if it exists in the widget
		// This would typically be a UMG animation
		UE_LOG(LogTemp, Log, TEXT("ShowSuccessAnimation: Playing success animation"));
	}
    
	// Play success sound
	if (ProgressSound)
	{
		PlayUISound(ProgressSound);
	}
}

void ULessonUIManagerComponent::ShowErrorAnimation()
{
	// Play error animation on instruction widget
	if (InstructionWidget)
	{
		// Trigger error animation if it exists
		UE_LOG(LogTemp, Log, TEXT("ShowErrorAnimation: Playing error animation"));
	}
}


// === ENHANCED INSTRUCTION FUNCTIONS ===

void ULessonUIManagerComponent::ShowInstruction(const FString& InstructionText) const
{
    UE_LOG(LogTemp, Warning, TEXT("=== ShowInstruction Called ==="));
    UE_LOG(LogTemp, Warning, TEXT("  - Text: '%s'"), *InstructionText);
    
    // Spawn the actor if it doesn't exist
    if (!LessonUIActor)
    {
        if (!LessonUIActorClass)
        {
            UE_LOG(LogTemp, Error, TEXT("ShowInstruction: LessonUIActorClass not set!"));
            return;
        }
        
        if (!GetWorld())
        {
            UE_LOG(LogTemp, Error, TEXT("ShowInstruction: No world!"));
            return;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("ShowInstruction: Spawning LessonUIActor"));
        
        // Spawn in front of player
        FVector SpawnLocation = GetPlayerLocation() + (GetPlayerForwardVector() * 200.0f);
        SpawnLocation.Z += 50.0f; // Slightly above eye level
        
        FRotator SpawnRotation = FRotator::ZeroRotator;
        
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = GetOwner();
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        // Need to cast away const since we're modifying the actor pointer
        ULessonUIManagerComponent* MutableThis = const_cast<ULessonUIManagerComponent*>(this);
        MutableThis->LessonUIActor = GetWorld()->SpawnActor<ALessonUIActor>(
            LessonUIActorClass,
            SpawnLocation,
            SpawnRotation,
            SpawnParams
        );
        
        if (MutableThis->LessonUIActor)
        {
            UE_LOG(LogTemp, Warning, TEXT("ShowInstruction: Successfully spawned LessonUIActor at %s"), 
                *SpawnLocation.ToString());

        	LessonUIActor->SetActorScale3D(FVector(0.15f, 0.15f, 0.15f));
        	UE_LOG(LogTemp, Warning, TEXT("ShowInstruction: Scaled actor to 15%%"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("ShowInstruction: Failed to spawn LessonUIActor"));
            return;
        }
    }
    
    // Show the instruction
    if (LessonUIActor)
    {
        LessonUIActor->ShowInstruction(InstructionText);
        UE_LOG(LogTemp, Warning, TEXT("ShowInstruction: Displayed instruction on actor"));
    }
}
void ULessonUIManagerComponent::ShowInstructionAtLocation(const FString& InstructionText, const FVector& WorldLocation) const
{
	// Position the widget at the specified location
	PositionInstructionWidget(WorldLocation);

	// Show the instruction
	ShowInstruction(InstructionText);
}
	
// === UPDATED TICK COMPONENT ===
void ULessonUIManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
	// Update widget positions and rotations
	UpdateWidgetPositions(DeltaTime);
    
	// Update look-at behavior
	UpdateLookAtBehavior(DeltaTime);
    
	// Update pulsing highlights
	if (bPulseHighlights && HighlightedActors.Num() > 0)
	{
		UpdatePulsingHighlights(DeltaTime);
	}
}


// === POSITIONING HELPER FUNCTIONS ===
void ULessonUIManagerComponent::UpdateWidgetPositions(float DeltaTime) const
{
	FVector PlayerLocation = GetPlayerLocation();

	//update instruction widget following
	if (bInstructionLooksAtPlayer && InstructionWidgetComponent)
	{
		FVector TargetLocation = FollowTarget ? FollowTarget->GetActorLocation() : PlayerLocation;
		TargetLocation += InstructionWidgetOffset;

		const FVector CurrentLocation = InstructionWidgetComponent->GetComponentLocation();
		const FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, 5.0f);
		InstructionWidgetComponent->SetWorldLocation(NewLocation);
	}

	//Update progress widget following
	if (bProgressFollowsPlayer && ProgressWidgetComponent)
	{
		const FVector TargetLocation = PlayerLocation + ProgressWidgetOffset;
		const FVector CurrentLocation = ProgressWidgetComponent->GetComponentLocation();
		const FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, 5.0f);
		ProgressWidgetComponent->SetWorldLocation(NewLocation);
	}
}

void ULessonUIManagerComponent::UpdateLookAtBehavior(float DeltaTime)
{
	FVector PlayerLocation = GetPlayerLocation();

	//update instructtion widget look-at
	if (bInstructionLooksAtPlayer && InstructionWidgetComponent)
	{
		FVector WidgetLocation = InstructionWidgetComponent->GetComponentLocation();
		FVector LookDirection = (PlayerLocation - WidgetLocation).GetSafeNormal();
		FRotator TargetRotation = FRotationMatrix::MakeFromX(LookDirection).Rotator();
		FRotator CurrentRotation = InstructionWidgetComponent->GetComponentRotation();
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, LookAtSpeed);
        
		InstructionWidgetComponent->SetWorldRotation(NewRotation);
	}

	// Update progress widget look-at
	if (bProgressLooksAtPlayer && ProgressWidgetComponent)
	{
		FVector WidgetLocation = ProgressWidgetComponent->GetComponentLocation();
		FVector LookDirection = (PlayerLocation - WidgetLocation).GetSafeNormal();
		FRotator TargetRotation = FRotationMatrix::MakeFromX(LookDirection).Rotator();
		FRotator CurrentRotation = ProgressWidgetComponent->GetComponentRotation();
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, LookAtSpeed);
        
		ProgressWidgetComponent->SetWorldRotation(NewRotation);
	}
}
FVector ULessonUIManagerComponent::GetPlayerLocation() const
{
	if (GetWorld())
	{
		if (const APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
		{
			return PlayerPawn->GetActorLocation();
		}
	}
	return FVector::ZeroVector;
}


FVector ULessonUIManagerComponent::GetPlayerForwardVector() const
{
	if (GetWorld())
	{
		if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
		{
			return PlayerPawn->GetActorForwardVector();
		}
	}
	return FVector::ForwardVector;
}
// === VISIBILITY UPDATES ===

void ULessonUIManagerComponent::SetUIVisibility(bool bVisible)
{
	bUIVisible = bVisible;
    
	if (InstructionWidgetComponent)
		InstructionWidgetComponent->SetVisibility(bVisible);
	if (ProgressWidgetComponent)
		ProgressWidgetComponent->SetVisibility(bVisible);
	if (LessonHUDComponent)
		LessonHUDComponent->SetVisibility(bVisible);
    
	UE_LOG(LogTemp, Log, TEXT("SetUIVisibility: %s"), bVisible ? TEXT("Visible") : TEXT("Hidden"));
}

// === HELPER FUNCTIONS ===
void ULessonUIManagerComponent::RefreshWidgetReferences()
{
	if (InstructionWidgetComponent)
	{
		InstructionWidget = InstructionWidgetComponent->GetUserWidgetObject();
	}
    
	if (ProgressWidgetComponent)
	{
		ProgressWidget = ProgressWidgetComponent->GetUserWidgetObject();
	}
    
	if (LessonHUDComponent)
	{
		LessonHUD = LessonHUDComponent->GetUserWidgetObject();
	}
    
	UE_LOG(LogTemp, Log, TEXT("RefreshWidgetReferences: Updated widget references"));
}

void ULessonUIManagerComponent::PlayUISound(USoundBase* Sound)
{
	if (!Sound)
		return;
    
	UGameplayStatics::PlaySound2D(GetWorld(), Sound, AudioVolume);
}

UTextBlock* ULessonUIManagerComponent::FindTextBlockInWidget(UUserWidget* Widget, const FString& TextBlockName)
{
	if (!Widget)
		return nullptr;
    
	return Cast<UTextBlock>(Widget->GetWidgetFromName(*TextBlockName));
}

UProgressBar* ULessonUIManagerComponent::FindProgressBarInWidget(UUserWidget* Widget, const FString& ProgressBarName)
{
	if (!Widget)
		return nullptr;
    
	return Cast<UProgressBar>(Widget->GetWidgetFromName(*ProgressBarName));
}

UImage* ULessonUIManagerComponent::FindImageInWidget(UUserWidget* Widget, const FString& ImageName)
{
	if (!Widget)
		return nullptr;
    
	return Cast<UImage>(Widget->GetWidgetFromName(*ImageName));
}

void ULessonUIManagerComponent::PauseHighlightForPart(APartActor* Part)
{
	if (!Part) return;
    
	if (!GrabHighlightedActors.Contains(Part))
	{
		GrabHighlightedActors.Add(Part);
		UE_LOG(LogTemp, Log, TEXT("PauseHighlightForPart: Paused pulsing for %s"), *Part->GetName());
	}
}

void ULessonUIManagerComponent::ResumeHighlightForPart(APartActor* Part)
{
	if (!Part) return;
    
	GrabHighlightedActors.Remove(Part);
	UE_LOG(LogTemp, Log, TEXT("ResumeHighlightForPart: Resumed pulsing for %s"), *Part->GetName());
}
// === QUERY FUNCTIONS ===

bool ULessonUIManagerComponent::IsUIVisible() const
{
	return bUIVisible;
}

bool ULessonUIManagerComponent::IsPartHighlighted(APartActor* Part) const
{
	return HighlightedActors.Contains(Part);
}