// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LessonTypes.h"
#include "LessonDataAsset.h"
#include "LessonStep.h"
#include "LessonUIManagerComponent.h"
#include "LessonManagerComponent.generated.h"

// Forward declarations
class ULessonUIManagerComponent;
class ALessonValidator;
class UAssembleStep;
class UFocusStep;
class UInteractionStep;
class AAssemblyActor;
class APartActor;
class UMechatronicsGameInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManagerLessonStarted, ULessonDataAsset*, LessonData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManagerLessonCompleted, ULessonDataAsset*, LessonData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManagerStepChanged, ULessonStep*, OldStep, ULessonStep*, NewStep);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManagerStepCompleted, ULessonStep*, CompletedStep, int32, StepIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManagerLessonProgress, int32, CurrentStep, int32, TotalSteps);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MECHATRONICSVR_API ULessonManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULessonManagerComponent();

	// LESSON DATA
	UPROPERTY(BlueprintReadOnly, Category = "Lesson Manager")
	TObjectPtr<ULessonDataAsset> CurrentLessonData;

	UPROPERTY(BlueprintReadOnly, Category = "Lesson Manager")
	TArray<TObjectPtr<ULessonStep>> LessonSteps;

	UPROPERTY(BlueprintReadOnly, Category = "Lesson Manager")
	int32 CurrentStepIndex;

	UPROPERTY(BlueprintReadOnly, Category = "Lesson Manager")
	TObjectPtr<ULessonStep> CurrentStep;

	UPROPERTY(BlueprintReadOnly, Category = "Lesson Manager")
	bool bIsLessonActive;

	UPROPERTY(BlueprintReadOnly, Category = "Lesson Manager")
	bool bIsLessonCompleted;

	// === EXTERNAL REFERENCES ===
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Lesson Manager|References")
	TObjectPtr<ULessonUIManagerComponent> UIManager;
 //
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lesson Manager|References")
	// TObjectPtr<ALessonValidator> StepValidator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lesson Manager|References")
	TSubclassOf<AAssemblyActor> AssemblyActorClass;

	


	// === SETTINGS ===
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lesson Manager|Settings")
	bool bAutoAdvanceSteps = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lesson Manager|Settings")
	bool bAllowStepSkipping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lesson Manager|Settings")
	float StepTransitionDelay = 1.0f;

	

	// === EVENTS ===
    
	UPROPERTY(BlueprintAssignable, Category = "Lesson Events")
	FOnManagerLessonStarted OnLessonStarted;

	UPROPERTY(BlueprintAssignable, Category = "Lesson Events")
	FOnManagerLessonCompleted OnLessonCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Lesson Events")
	FOnManagerStepChanged OnStepChanged;

	UPROPERTY(BlueprintAssignable, Category = "Lesson Events")
	FOnManagerStepCompleted OnStepCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Lesson Events")
	FOnManagerLessonProgress OnLessonProgress;

	// === PUBLIC API ===
	UFUNCTION(BlueprintCallable, Category = "Lesson Manager")
	bool InitializeLessonFromDataAsset(ULessonDataAsset* LessonData);

	UFUNCTION(BlueprintCallable, Category = "Lesson Manager")
	bool StartLesson();
	void EndLesson();

	UFUNCTION(BlueprintCallable, Category = "Lesson Manager")
	bool AdvanceStep();

	UFUNCTION(BlueprintCallable, Category = "Lesson Manager")
	bool GoToStep(int32 StepIndex);

	UFUNCTION(BlueprintCallable, Category = "Lesson Manager")
	void CompleteCurrentStep();

	UFUNCTION(BlueprintCallable, Category = "Lesson Manager")
	void ResetLesson();

	// === GETTERS ===
    
	UFUNCTION(BlueprintPure, Category = "Lesson Manager")
	ULessonStep* GetCurrentStep() const { return CurrentStep; }

	UFUNCTION(BlueprintPure, Category = "Lesson Manager")
	int32 GetCurrentStepIndex() const { return CurrentStepIndex; }

	UFUNCTION(BlueprintPure, Category = "Lesson Manager")
	int32 GetTotalStepsCount() const { return LessonSteps.Num(); }

	UFUNCTION(BlueprintPure, Category = "Lesson Manager")
	float GetLessonProgress() const;

	UFUNCTION(BlueprintPure, Category = "Lesson Manager")
	bool IsLessonActive() const { return bIsLessonActive; }

	UFUNCTION(BlueprintPure, Category = "Lesson Manager")
	bool IsLessonCompleted() const { return bIsLessonCompleted; }

	UFUNCTION(BlueprintPure, Category = "Lesson Manager")
	bool HasNextStep() const;

	UFUNCTION(BlueprintPure, Category = "Lesson Manager")
	bool HasPreviousStep() const;
	// === VALIDATION ===
    
	UFUNCTION(BlueprintCallable, Category = "Lesson Manager")
	bool CheckStepCompletion();

	UFUNCTION(BlueprintCallable, Category = "Lesson Manager")
	void ValidateCurrentStep();

	/** Set the UI manager for this lesson manager */
	UFUNCTION(BlueprintCallable, Category = "Lesson System")
	void SetUIManager(ULessonUIManagerComponent* InUIManager);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	// === STEP MANAGEMENT ===

	static ULessonStep* CreateStepFromData(const FLessonStepData& StepData);
	
	void LinkStepsSequentially();
	bool ActivateStep(int32 StepIndex);
    void DeactivateCurrentStep();
    void UpdateStepReferences();

	// === EVENT HANDLERS ===
    
	UFUNCTION()
	void HandleStepCompleted(ULessonStep* CompletedStep);

	UFUNCTION()
	void HandleAssemblyPartsConnected(APartActor* PartA, APartActor* PartB);

	// === UI MANAGEMENT ===
    
	void UpdateUI();
	void ShowStepInstructions();
	void UpdateProgress();

	// === VALIDATION MANAGEMENT ===
    
	

	// === TIMER HANDLING ===
    
	void HandleStepTransitionDelay();
	FTimerHandle StepTransitionTimer;

	// === INITIALIZATION ===
    
	void FindExternalReferences();
	void ConnectToExistingSystems();

private:
	// === INTERNAL STATE ===
    
	bool bInitialized;
	bool bWaitingForStepTransition;

	UPROPERTY()
	TObjectPtr<AAssemblyActor> AssemblyActor = nullptr;




		
};
