// Fill out your copyright notice in the Description page of Project Settings.


#include "LessonDataAsset.h"

#include "AssembleStep.h"
#include "FocusStep.h"
#include "InteractionStep.h"
#include "LessonStep.h"

class UInteractionStep;
class UFocusStep;

ULessonDataAsset::ULessonDataAsset()
{
	LessonTitle = FText::FromString("New Lesson");
	LessonDescription = FText::FromString("Describe the lesson here.");
	AssociatedLevelName = NAME_None;

	ExpectedPartCount = 0;
	ExpectedConnectionCount = 0;
	bAllowFreePlayAfterCompletion = true;
	LessonTimeoutMinutes = 0.0f; // no timeout by default
}

bool ULessonDataAsset::IsLessonValid() const
{
	// Basic validation
	if (LessonTitle.IsEmpty())
		return false;
        
	if (StepDefinitions.Num() == 0)
		return false;
        
	if (AssociatedLevelName.IsNone())
		return false;
        
	// Validate each step instance
	for (const FLessonStepData& Step : StepDefinitions)
	{
		if (!Step.StepInstance)
		{
			return false;
		}
        
		// Check if step instance has instruction text
		if (Step.StepInstance->InstructionText.IsEmpty())
		{
			return false;
		}
	}
    
	return true;
}
bool ULessonDataAsset::HasValidLevel() const
{
	return !AssociatedLevelName.IsNone();
}

TArray<FString> ULessonDataAsset::GetValidationErrors() const
{
	TArray<FString> Errors;
    
	if (LessonTitle.IsEmpty())
		Errors.Add("Lesson title is empty");
        
	if (StepDefinitions.Num() == 0)
		Errors.Add("No lesson steps defined");
        
	if (AssociatedLevelName.IsNone())
		Errors.Add("No associated level specified");
        
	// Check each step instance
	for (int32 i = 0; i < StepDefinitions.Num(); i++)
	{
		const FLessonStepData& Step = StepDefinitions[i];
        
		if (!Step.StepInstance)
		{
			Errors.Add(FString::Printf(TEXT("Step %d: No step instance assigned"), i + 1));
			continue;
		}
        
		if (Step.StepInstance->InstructionText.IsEmpty())
		{
			Errors.Add(FString::Printf(TEXT("Step %d: Step instance has no instruction text"), i + 1));
		}
        
		// Type-specific validation
		if (UAssembleStep* AssembleStep = Cast<UAssembleStep>(Step.StepInstance))
		{
			if (AssembleStep->TargetPartClasses.Num() == 0)
			{
				Errors.Add(FString::Printf(TEXT("Step %d: Assembly step has no target parts"), i + 1));
			}
		}
		else if (const UFocusStep* FocusStep = Cast<UFocusStep>(Step.StepInstance))
		{
			if (!FocusStep->TargetActor)
			{
				Errors.Add(FString::Printf(TEXT("Step %d: Focus step has no target actor"), i + 1));
			}
		}
		else if (UInteractionStep* InteractionStep = Cast<UInteractionStep>(Step.StepInstance))
		{
			if (!InteractionStep->TargetActor)
			{
				Errors.Add(FString::Printf(TEXT("Step %d: Interaction step has no target actor"), i + 1));
			}
		}
	}
    
	return Errors;
}
bool ULessonDataAsset::IsValidStepIndex(int32 StepIndex) const
{
	return StepIndex >= 0 && StepIndex < StepDefinitions.Num();
}

FLessonStepData ULessonDataAsset::GetStepData(const int32 StepIndex) const
{
	if (IsValidStepIndex(StepIndex))
	{
		return StepDefinitions[StepIndex];
	}
    
	// Return default step data if invalid index
	return FLessonStepData();
}


