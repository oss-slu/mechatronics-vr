// Fill out your copyright notice in the Description page of Project Settings.


#include "LessonDataAsset.h"

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
	// basic validaiton
	if (LessonTitle.IsEmpty()) return false;
	if (StepDefinitions.Num() == 0) return false;
	if (AssociatedLevelName.IsNone()) return false;

	//validate each step
	for (const FLessonStepData& Step : StepDefinitions)
	{
		if (Step.InstructionText.IsEmpty()) return false;
		if (Step.StepType == ELessonStepType::Assemble && Step.TargetParts.Num() == 0) return false;
		if (Step.StepType == ELessonStepType::Focus && !Step.FocusTarget) return false;
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
        
	// Check each step
	for (int32 i = 0; i < StepDefinitions.Num(); i++)
	{
		const FLessonStepData& Step = StepDefinitions[i];
        
		if (Step.InstructionText.IsEmpty())
			Errors.Add(FString::Printf(TEXT("Step %d: Instruction text is empty"), i + 1));
            
		if (Step.StepType == ELessonStepType::Assemble && Step.TargetParts.Num() == 0)
			Errors.Add(FString::Printf(TEXT("Step %d: Assembly step has no target parts"), i + 1));
            
		if (Step.StepType == ELessonStepType::Focus && !Step.FocusTarget)
			Errors.Add(FString::Printf(TEXT("Step %d: Focus step has no target actor"), i + 1));
	}
    
	return Errors;
}
bool ULessonDataAsset::IsValidStepIndex(int32 StepIndex) const
{
	return StepIndex >= 0 && StepIndex < StepDefinitions.Num();
}

FLessonStepData ULessonDataAsset::GetStepData(int32 StepIndex) const
{
	if (IsValidStepIndex(StepIndex))
	{
		return StepDefinitions[StepIndex];
	}
    
	// Return default step data if invalid index
	return FLessonStepData();
}


