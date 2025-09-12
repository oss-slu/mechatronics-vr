// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "../Lesson/LessonDataAsset.h"
#include "MechatronicsGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLessonChanged, ULessonDataAsset*, NewLesson);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLessonProgressChanged, int32, CurrentLessonIndex, int32, TotalLessons);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLessonCompleted, ULessonDataAsset*, CompletedLesson);

/**
 * 
 */
UCLASS()
class MECHATRONICSVR_API UMechatronicsGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UMechatronicsGameInstance();

	// === ACTIVE LESSON DATA ===
    
	UPROPERTY(BlueprintReadOnly, Category = "Lesson System")
	TObjectPtr<ULessonDataAsset> ActiveLessonData;

	UPROPERTY(BlueprintReadOnly, Category = "Lesson System")
	FString CurrentLevelName;

	UPROPERTY(BlueprintReadOnly, Category = "Lesson System")
	int32 CurrentLessonIndex = 0;

	// === LESSON LIBRARY ===
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lesson System", meta = (AllowedClasses = "/Script/Engine.LessonDataAsset"))
	TArray<TObjectPtr<ULessonDataAsset>> AvailableLessons;

	// === LESSON PROGRESSION SETTINGS ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lesson Settings")
	bool bAutoProgressLessons = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lesson Settings")
	bool bAllowLessonSkipping = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lesson Settings")
	bool bPersistProgress = true;

	// === EVENTS ===
	UPROPERTY(BlueprintAssignable, Category = "Lesson Events")
	FOnLessonChanged OnLessonChanged;

	UPROPERTY(BlueprintAssignable, Category = "Lesson Events") 
	FOnLessonProgressChanged OnLessonProgressChanged;

	UPROPERTY(BlueprintAssignable, Category = "Lesson Events")
	FOnLessonCompleted OnLessonCompleted;

	// === PUBLIC API ===
	UFUNCTION(BlueprintCallable, Category = "Lesson System")
	bool LoadNextLesson();

	UFUNCTION(BlueprintCallable, Category = "Lesson System")
	bool LoadPreviousLesson();

	UFUNCTION(BlueprintCallable, Category = "Lesson System")
	bool StartLesson(FName LessonName);

	
	UFUNCTION(BlueprintCallable, Category = "Lesson System")
	bool StartLessonByIndex(int32 LessonIndex);

	UFUNCTION(BlueprintCallable, Category = "Lesson System")
	void CompleteCurrentLesson();

	UFUNCTION(BlueprintCallable, Category = "Lesson System")
	void ResetLessonProgress();

	// === GETTERS ===
	 
	UFUNCTION(BlueprintPure, Category = "Lesson System")
	ULessonDataAsset* GetActiveLessonData() const { return ActiveLessonData; }

	UFUNCTION(BlueprintPure, Category = "Lesson System")
	int32 GetCurrentLessonIndex() const { return CurrentLessonIndex; }

	UFUNCTION(BlueprintPure, Category = "Lesson System")
	int32 GetTotalLessonCount() const { return AvailableLessons.Num(); }
	UFUNCTION(BlueprintPure, Category = "Lesson System")
	bool HasNextLesson() const;

	UFUNCTION(BlueprintPure, Category = "Lesson System")
	bool HasPreviousLesson() const;

	UFUNCTION(BlueprintPure, Category = "Lesson System")
	TArray<ULessonDataAsset*> GetAllLessons() const;

	UFUNCTION(BlueprintPure, Category = "Lesson System")
	float GetLessonProgressPercentage() const;

protected:
	// === INITIALIZATION ===
    
	virtual void Init() override;

	// === INTERNAL LESSON MANAGEMENT ===
    
	bool LoadLessonInternal(int32 LessonIndex);
	bool ChangeToLevel(const FString& LevelName);
	void BroadcastLessonChanged();
	void BroadcastProgressChanged();

	// === VALIDATION ===
    
	bool IsValidLessonIndex(int32 Index) const;
	ULessonDataAsset* FindLessonByName(FName LessonName) const;
	void ValidateAvailableLessons();

	// === SAVE/LOAD ===
    
	void SaveLessonProgress();
	void LoadLessonProgress();
    
private:
	// === PRIVATE DATA ===
    
	bool bInitialized = false;
};