// Fill out your copyright notice in the Description page of Project Settings.


#include "MechatronicsGameMode.h"
#include "../Lesson/LessonManagerComponent.h"
#include "../Lesson/LessonUIManagerComponent.h"
#include "../Lesson/LessonDataAsset.h"
#include "AssemblyActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

#include "EngineUtils.h"

AMechatronicsGameMode::AMechatronicsGameMode()
{
	PrimaryActorTick.bCanEverTick = false;

	//create lesson system components
	LessonManager = CreateDefaultSubobject<ULessonManagerComponent>(TEXT("LessonManager"));
	UIManager = CreateDefaultSubobject<ULessonUIManagerComponent>(TEXT("UIManager"));
}

void AMechatronicsGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Initialize lesson system
	InitializeLessonSystem();

	// Auto-start lesson if configured
	if (bAutoStartLesson && AutoStartLessonData)
	{
		UE_LOG(LogTemp, Log, TEXT("MechatronicsGameMode: Auto-starting lesson"));
        
		// Small delay to ensure everything is fully loaded
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, [this]()
		{
			StartLesson(AutoStartLessonData);
		}, 0.5f, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MechatronicsGameMode: No auto-start lesson configured"));
	}

	if (AssemblyActorClass)
	{
		for (TActorIterator<AAssemblyActor> It(GetWorld()); It; ++It)
		{
			AAssemblyActor* PotentialAssembly = *It;
			if  (PotentialAssembly && 
				PotentialAssembly->IsA(AssemblyActorClass))
			{
				AssemblyActor = PotentialAssembly;
				UE_LOG(LogTemp, Log, TEXT("%s: Found AssemblyActor: %s"), 
					*GetName(), *AssemblyActor->GetName());
				break;
			}
		}
		if (!AssemblyActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: Could not find instance of class %s"), 
				*GetName(), *AssemblyActorClass->GetName());
		}
	}
}

void AMechatronicsGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsLessonActive())
	{
		StopLesson();
	}
	Super::EndPlay(EndPlayReason);
}

void AMechatronicsGameMode::InitializeLessonSystem()
{
	if(bLessonSystemInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("MechatronicsGameMode: Lesson system already initialized"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MechatronicsGameMode: Initializing lesson system"));

	// Validate components exist
	if (!LessonManager)
	{
		UE_LOG(LogTemp, Error, TEXT("MechatronicsGameMode: LessonManager is null"));
		return;
	}

	if (!UIManager)
	{
		UE_LOG(LogTemp, Error, TEXT("MechatronicsGameMode: UIManager is null"));
		return;
	}
	bLessonSystemInitialized = true;
	UE_LOG(LogTemp, Log, TEXT("MechatronicsGameMode: Lesson system initialized successfully"));
	
	
}

bool AMechatronicsGameMode::StartLesson(ULessonDataAsset* LessonData)
{
	if (!LessonData)
	{
		UE_LOG(LogTemp, Error, TEXT("MechatronicsGameMode::StartLesson - Invalid Lesson Data Asset"));
		return false;
	}

	if (!bLessonSystemInitialized)
	{
		UE_LOG(LogTemp, Error, TEXT("MechatronicsGameMode::StartLesson - Lesson system not initialized"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("MechatronicsGameMode::StartLesson - Starting lesson: %s"), *LessonData->LessonTitle.ToString());

	// Initialize lesson in LessonManager - it will find the assembly itself

	if (!LessonManager->InitializeLessonFromDataAsset(LessonData))
	{
		UE_LOG(LogTemp, Error, TEXT("MechatronicsGameMode::StartLesson - Failed to initialize lesson"));
		return false;
	}

	LessonManager->StartLesson();

	UE_LOG(LogTemp, Log, TEXT("MechatronicsGameMode::StartLesson - Lesson started successfully"));
	return true;
}

void AMechatronicsGameMode::StopLesson()
{
	if (!LessonManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("MechatronicsGameMode::StopLesson - No lesson manager"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MechatronicsGameMode::StopLesson - Stopping current lesson"));
	LessonManager->EndLesson();
}

bool AMechatronicsGameMode::IsLessonActive() const
{
	return LessonManager && LessonManager->IsLessonActive();
}

float AMechatronicsGameMode::GetLessonProgress() const
{
	if (!LessonManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("MechatronicsGameMode::GetLessonProgress - No lesson manager"));
		return 0.0f;
	}

	return LessonManager->GetLessonProgress();
}

