// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LessonTypes.h"
#include "LessonDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Category = "Lesson System")
class MECHATRONICSVR_API ULessonDataAsset : public UDataAsset
{
	GENERATED_BODY()


public:

	// LESSON METADATA
	ULessonDataAsset();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lesson Info")
	FText LessonTitle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lesson Info", meta = (MultiLine = true))
	FText LessonDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lesson Info")
	FName AssociatedLevelName;

	// === LESSON CONTENT ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lesson Content")
	TArray<FLessonStepData> StepDefinitions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lesson Content")
	TArray<TSubclassOf<APartActor>> RequiredParts;

	// === VALIDATION DATA ===

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
	int32 ExpectedPartCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation") 
	int32 ExpectedConnectionCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bAllowFreePlayAfterCompletion = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	float LessonTimeoutMinutes = 0.0f; // 0 = no timeout
public:
	// validation functions
	UFUNCTION(BlueprintCallable, Category = "Lesson Validation" )
	bool IsLessonValid() const;

	UFUNCTION(BlueprintPure, Category = "Lesson")
	int32 GetStepCount() const { return StepDefinitions.Num(); }

	UFUNCTION(BlueprintPure, Category = "Lesson")
	bool HasValidLevel() const;

	UFUNCTION(BlueprintCallable, Category = "Lesson")
	TArray<FString> GetValidationErrors() const;

	// === STEP ACCESS ===
    
	UFUNCTION(BlueprintPure, Category = "Lesson")
	bool IsValidStepIndex(int32 StepIndex) const;

	UFUNCTION(BlueprintPure, Category = "Lesson")
	FLessonStepData GetStepData(int32 StepIndex) const;

};
