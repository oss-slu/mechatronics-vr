// Fill out your copyright notice in the Description page of Project Settings.


#include "MechatronicsGameInstance.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"


UMechatronicsGameInstance::UMechatronicsGameInstance()
{
	CurrentLessonIndex = 0;
	bAllowLessonSkipping = true;
	bAutoProgressLessons = false;
	bPersistProgress = true;
	bInitialized = false;
}

void UMechatronicsGameInstance::Init()
{
	Super::Init();

	UE_LOG(LogTemp, Log, TEXT("MechatronicsGameInstance: Initialized"));
	
	// Note: Lesson data is managed per-level by GameMode, not by GameInstance
	//  GameInstance only tracks cross-level progress if needed
	
	bInitialized = true;
}

// === LESSON LOADING ===

bool UMechatronicsGameInstance::LoadPreviousLesson()
{
	if (!HasPreviousLesson())
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadPreviousLesson: No previous lesson available"));
		return false;
	}
    
	return StartLessonByIndex(CurrentLessonIndex - 1);
}
bool UMechatronicsGameInstance::LoadNextLesson()
{
	if (!HasPreviousLesson())
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadPreviousLesson: No previous lesson available"));
		return false;
	}
	return StartLessonByIndex(CurrentLessonIndex - 1);
}

auto UMechatronicsGameInstance::StartLesson(const FName LessonName) -> bool
{
	ULessonDataAsset* FoundLesson = FindLessonByName(LessonName);
	if (!FoundLesson)
	{
		UE_LOG(LogTemp, Error, TEXT("StartLesson: Lesson '%s' not found."), *LessonName.ToString());
		return false;
	}

	// find the index of this lesson
	int32 LessonIndex = AvailableLessons.IndexOfByKey(FoundLesson);
	if (LessonIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Error, TEXT("StartLesson: Lesson '%s' not in available lessons."), *LessonName.ToString());
		return false;
	}

	return StartLessonByIndex(LessonIndex);
}

auto UMechatronicsGameInstance::StartLessonByIndex(const int32 LessonIndex) -> bool
{
	if (!IsValidLessonIndex(LessonIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("StartLessonByIndex: Invalid lesson index %d."), LessonIndex);
		return false;
	}
	return LoadLessonInternal(LessonIndex);
}

bool UMechatronicsGameInstance::LoadLessonInternal(int32 LessonIndex)
{
	if (!IsValidLessonIndex(LessonIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("LoadLessonInternal: Invalid lesson index %d."), LessonIndex);
		return false;
	}

	ULessonDataAsset* NewLessonData = AvailableLessons[LessonIndex];
	if (!NewLessonData)
	{
		UE_LOG(LogTemp, Error, TEXT("LoadLessonInternal: Lesson data at index %d is null."), LessonIndex);
		return false;
	}

	//Validate lesson data
	if (!NewLessonData->IsLessonValid())
	{
		TArray<FString> ValidationErrors = NewLessonData->GetValidationErrors();
		UE_LOG(LogTemp, Error, TEXT("LoadLessonInternal: Lesson '%s' is invalid. Errors:"), *NewLessonData->GetName());
		for (const FString& Error : ValidationErrors)
		{
			UE_LOG(LogTemp, Error, TEXT(" - %s"), *Error);
		}
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("LoadLessonInternal: Loading lesson '%s' (Index: %d)"), 
		  *NewLessonData->LessonTitle, LessonIndex);

	// store previous data for comparison
	ULessonDataAsset* PreviousLessonData = ActiveLessonData;
	int32 PreviousLessonIndex = CurrentLessonIndex;

	//update current lesson data
	CurrentLessonIndex = LessonIndex;
	ActiveLessonData = NewLessonData;

	// change to associated level if needed
	FString TargetLevelName = NewLessonData->AssociatedLevelName.ToString();
	if (!TargetLevelName.IsEmpty() && TargetLevelName != CurrentLevelName)
	{
		if (!ChangeToLevel(TargetLevelName))
		{
			// Failed to change level, revert lesson data
			ActiveLessonData = PreviousLessonData;
			CurrentLessonIndex = PreviousLessonIndex;
			return false;
		}
	}

	// Save Progress if enabled
	if (bPersistProgress)
	{
		SaveLessonProgress();
	}

	// broadcast events
	BroadcastLessonChanged();
	BroadcastProgressChanged();

	
	UE_LOG(LogTemp, Log, TEXT("LoadLessonInternal: Successfully loaded lesson '%s'"), 
		   *NewLessonData->LessonTitle);
    
	return true;
	
}

bool UMechatronicsGameInstance::ChangeToLevel(const FString& LevelName)
{
	if (LevelName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ChangeToLevel: Level name is empty."));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("ChangeToLevel: Changing to level '%s'"), *LevelName);

	//store current level name
	CurrentLevelName = LevelName;

	// use Unreal's level loading system
	UGameplayStatics::OpenLevel(this, FName(*LevelName));

	return true;
}

// LESSON COMPLETION

void UMechatronicsGameInstance::CompleteCurrentLesson()
{
	if (!ActiveLessonData)
	{
		UE_LOG(LogTemp, Warning, TEXT("CompleteCurrentLesson: No active lesson to complete."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("CompleteCurrentLesson: Completing lesson '%s'"), 
		   *ActiveLessonData->LessonTitle);

	//Broadcast completion event
	OnLessonCompleted.Broadcast(ActiveLessonData);

	// auto-progress if enabled
	if (bAutoProgressLessons && HasNextLesson())
	{
		UE_LOG(LogTemp, Log, TEXT("CompleteCurrentLesson: Auto-progressing to next lesson."));
		LoadNextLesson();
	}
}

void UMechatronicsGameInstance::ResetLessonProgress()
{
	UE_LOG(LogTemp, Log, TEXT("ResetLessonProgress: Resetting to first lesson"));
    
	CurrentLessonIndex = 0;
    
	if (AvailableLessons.Num() > 0)
	{
		LoadLessonInternal(0);
	}
	else
	{
		ActiveLessonData = nullptr;
		CurrentLevelName = "";
		BroadcastLessonChanged();
		BroadcastProgressChanged();
	}
}

// QUERY FUNCTIONS
bool UMechatronicsGameInstance::HasNextLesson() const
{
	return (CurrentLessonIndex + 1) < AvailableLessons.Num();
}

bool UMechatronicsGameInstance::HasPreviousLesson() const
{
	return CurrentLessonIndex > 0;
}

TArray<ULessonDataAsset*> UMechatronicsGameInstance::GetAllLessons() const
{
	TArray<ULessonDataAsset*> Lessons;
	for (const TObjectPtr<ULessonDataAsset>& LessonPtr: AvailableLessons)
	{
		if (LessonPtr)
		{
			Lessons.Add(LessonPtr);
		}
	}
	return Lessons;
}

float UMechatronicsGameInstance::GetLessonProgressPercentage() const
{
	if (AvailableLessons.Num() == 0) return 0.0f;
	return (static_cast<float>(CurrentLessonIndex) / static_cast<float>(AvailableLessons.Num())) * 100.0f;
}


// === INTERNAL UTILITIES ===
void UMechatronicsGameInstance::BroadcastLessonChanged()
{
	UE_LOG(LogTemp, Log, TEXT("BroadcastLessonChanged: Broadcasting lesson change event."));
	OnLessonChanged.Broadcast(ActiveLessonData);
}

void UMechatronicsGameInstance::BroadcastProgressChanged()
{
	OnLessonProgressChanged.Broadcast(CurrentLessonIndex, AvailableLessons.Num());
}

bool UMechatronicsGameInstance::IsValidLessonIndex(int32 Index) const
{
	return Index >= 0 && Index < AvailableLessons.Num();
}

ULessonDataAsset* UMechatronicsGameInstance::FindLessonByName(const FName LessonName) const
{
	for (const TObjectPtr<ULessonDataAsset>& LessonPtr: AvailableLessons)
	{
		if (LessonPtr && LessonPtr->GetFName() == LessonName)
		{
			return LessonPtr;
		}
	}
	return nullptr;
}

void UMechatronicsGameInstance::ValidateAvailableLessons()
{
	// Simplified - validation happens at GameMode level per-level
	// This is only for tracking available lessons, not loading them
	UE_LOG(LogTemp, Log, TEXT("ValidateAvailableLessons: %d lessons in registry"), AvailableLessons.Num());
}


// === SAVE/LOAD (Basic Implementation) ===

void UMechatronicsGameInstance::SaveLessonProgress()
{
	// TODO: Implement save system using UGameplayStatics::SaveGameToSlot()
	// For now, just log the action
	UE_LOG(LogTemp, Log, TEXT("SaveLessonProgress: Saving lesson index %d"), CurrentLessonIndex);
}

void UMechatronicsGameInstance::LoadLessonProgress()
{
	// TODO: Implement load system using UGameplayStatics::LoadGameFromSlot()
	// For now, just log the action
	UE_LOG(LogTemp, Log, TEXT("LoadLessonProgress: Loading lesson progress"));
}



