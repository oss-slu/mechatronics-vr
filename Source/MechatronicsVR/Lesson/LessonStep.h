// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LessonStep.generated.h"

/**
 * 
 */
UCLASS()
class MECHATRONICSVR_API ULessonStep : public UObject
{
	GENERATED_BODY()
	
public:

	// Broadcasts
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLessonStepStarted, ULessonStep*, Step);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLessonStepUpdated, ULessonStep*, Step);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLessonStepCompleted, ULessonStep*, Step);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLessonStepReset, ULessonStep*, Step);
	
	ULessonStep();
	
	// Base properties
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lesson|Step")
	FText InstructionText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lesson|Step")
	TObjectPtr<ULessonStep> PreviousStep = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lesson|Step")
	TObjectPtr<ULessonStep> NextStep = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lesson|Step")
	bool bStepCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lesson|Step")
	bool bWantsTickWhileActive = false;

	// Base Functions
	UFUNCTION(BlueprintCallable, Category = "Lesson|Step")
	virtual void StartStep();

	UFUNCTION(BlueprintCallable, Category = "Lesson|Step")
	virtual void StopStep();

	UFUNCTION(BlueprintCallable, Category = "Lesson|Step")
	virtual void ResetStep();
	
	UFUNCTION(BlueprintCallable, Category = "Lesson|Step")
	virtual void TickStep(float DeltaTime);

	UFUNCTION(BlueprintNativeEvent, Category="Lesson|Step")
	bool CheckCompletion() const;
	virtual bool CheckCompletion_Implementation();

	UFUNCTION(BlueprintPure, Category="Lesson|Step")
	bool IsActive() const {return bIsActive;}

	UFUNCTION(BlueprintCallable, Category="Lesson|Step")
	void setPreviousStep(ULessonStep * InPreviousStep);

	UFUNCTION(BlueprintCallable, Category="Lesson|Step")
	void setNextStep(ULessonStep * InNextStep);

	// Event Dispatchers
	UPROPERTY(BlueprintAssignable, Category="Lesson|Events")
	FOnLessonStepStarted OnStepStarted;

	UPROPERTY(BlueprintAssignable, Category="Lesson|Events")
	FOnLessonStepUpdated OnStepUpdated;

	UPROPERTY(BlueprintAssignable, Category="Lesson|Events")
	FOnLessonStepCompleted OnStepCompleted;

	UPROPERTY(BlueprintAssignable, Category="Lesson|Events")
	FOnLessonStepReset OnStepReset;

protected:
	UFUNCTION(BlueprintCallable, Category = "Lesson|Step")
	virtual void CompleteStep();

	UFUNCTION(BlueprintCallable, Category = "Lesson|Step")
	void NotifyUpdated();

	virtual void OnStarted();

	virtual void OnStopped();
	
	virtual void OnReset();

	UPROPERTY(VisibleAnywhere, Category="Lesson|Internal")
	bool bIsActive = false;
};
