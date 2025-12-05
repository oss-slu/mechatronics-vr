#include "LessonUIActor.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

ALessonUIActor::ALessonUIActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Create widget component (just like VR Template menu)
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(RootComponent);
	WidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	WidgetComponent->SetDrawSize(WidgetDrawSize);
	WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WidgetComponent->SetVisibility(true);
}

void ALessonUIActor::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("LessonUIActor::BeginPlay - %s"), *GetName());
	
	// Get the widget
	CachedWidget = WidgetComponent->GetUserWidgetObject();
	
	if (CachedWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("  - Widget loaded: %s"), *CachedWidget->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("  - No widget loaded! Check Widget Class in BP"));
	}
	
	// Start hidden
	SetVisible(false);
}

void ALessonUIActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Make widget face player
	if (bAlwaysFacePlayer && WidgetComponent->IsVisible())
	{
		UpdateFacingPlayer(DeltaTime);
	}
}

void ALessonUIActor::ShowInstruction(const FString& InstructionString)
{
	UE_LOG(LogTemp, Warning, TEXT("LessonUIActor::ShowInstruction - '%s'"), *InstructionString);
    
	SetInstructionText(InstructionString);
	SetVisible(true);

	
}

void ALessonUIActor::HideInstruction()
{
	SetVisible(false);
}

void ALessonUIActor::SetInstructionText(const FString& InstructionString)
{
	if (!CachedWidget)
	{
		CachedWidget = WidgetComponent->GetUserWidgetObject();
	}
    
	if (CachedWidget)
	{
		// Find the TextBlock named "InstructionText"
		if (UTextBlock* TextBlock = Cast<UTextBlock>(CachedWidget->GetWidgetFromName("InstructionText")))
		{
			// Convert FString to FText for the TextBlock
			TextBlock->SetText(FText::FromString(InstructionString));
			UE_LOG(LogTemp, Warning, TEXT("  - Set text successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("  - Could not find TextBlock named 'InstructionText'"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("  - CachedWidget is NULL"));
	}
}

void ALessonUIActor::PositionNearActor(AActor* TargetActor, const FVector& Offset)
{
	if (!TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("PositionNearActor: Invalid target actor"));
		return;
	}
	
	FVector NewLocation = TargetActor->GetActorLocation() + Offset;
	SetActorLocation(NewLocation);
	
	UE_LOG(LogTemp, Warning, TEXT("LessonUIActor: Positioned at %s near %s"), 
		*NewLocation.ToString(), *TargetActor->GetName());
}

void ALessonUIActor::SetVisible(bool bVisible)
{
	WidgetComponent->SetVisibility(bVisible);
	WidgetComponent->SetHiddenInGame(!bVisible);
	
	UE_LOG(LogTemp, Warning, TEXT("LessonUIActor::SetVisible - %s"), bVisible ? TEXT("TRUE") : TEXT("FALSE"));
}

void ALessonUIActor::UpdateFacingPlayer(float DeltaTime)
{
	FVector PlayerLocation = GetPlayerLocation();
	FVector ToPlayer = PlayerLocation - GetActorLocation();
	ToPlayer.Z = 0; // Keep upright
	
	if (!ToPlayer.IsNearlyZero())
	{
		FRotator TargetRotation = ToPlayer.Rotation();
		FRotator CurrentRotation = GetActorRotation();
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationSpeed);
		SetActorRotation(NewRotation);
	}
}

FVector ALessonUIActor::GetPlayerLocation() const
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APlayerCameraManager* CameraManager = PC->PlayerCameraManager)
		{
			return CameraManager->GetCameraLocation();
		}
		
		if (APawn* PlayerPawn = PC->GetPawn())
		{
			return PlayerPawn->GetActorLocation();
		}
	}
	
	return FVector::ZeroVector;
}