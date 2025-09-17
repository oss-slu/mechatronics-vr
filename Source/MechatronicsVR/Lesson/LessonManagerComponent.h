// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LessonTypes.h"
#include "LessonDataAsset.h"
#include "LessonStep.h"
#include "LessonManagerComponent.generated.h"

// Forward declarations
class ALessonUIManager;
class ALessonValidator;
class UAssembleStep;
class UFocusStep;
class UInteractionStep;
class AAssemblyActor;
class APartActor;
class UMechatronicsGameInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLessonStarted, ULessonDataAsset*, LessonData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLessonCompleted, ULessonDataAsset*, LessonData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStepChanged, ULessonStep*, OldStep, ULessonStep*, NewStep);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStepCompleted, ULessonStep*, CompletedStep, int32, StepIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLessonProgress, int32, CurrentStep, int32, TotalSteps);


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
 //    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lesson Manager|references")
	// TObjectPtr<ALessonUIManager> UIManager;
 //
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lesson Manager|References")
	// TObjectPtr<ALessonValidator> StepValidator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lesson Manager|References")
	TObjectPtr<AAssemblyActor> AssemblyActor;


	// === SETTINGS ===
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lesson Manager|Settings")
	bool bAutoAdvanceSteps = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lesson Manager|Settings")
	bool bAllowStepSkipping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lesson Manager|Settings")
	float StepTransitionDelay = 1.0f;

	

	// === EVENTS ===
    
	UPROPERTY(BlueprintAssignable, Category = "Lesson Events")
	FOnLessonStarted OnLessonStarted;

	UPROPERTY(BlueprintAssignable, Category = "Lesson Events")
	FOnLessonCompleted OnLessonCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Lesson Events")
	FOnStepChanged OnStepChanged;

	UPROPERTY(BlueprintAssignable, Category = "Lesson Events")
	FOnStepCompleted OnStepCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Lesson Events")
	FOnLessonProgress OnLessonProgress;

	// === PUBLIC API ===
	UFUNCTION(BlueprintCallable, Category = "Lesson Manager")
	bool InitializeLessonFromDataAsset(ULessonDataAsset* LessonData);

	UFUNCTION(BlueprintCallable, Category = "Lesson Manager")
	bool StartLesson();

	UFUNCTION(BlueprintCallable, Category = "Lesson Manager")
	bool AdvanceStep();

	UFUNCTION(BlueprintCallable, Category = "Lesson Manager")
	bool GoToStep(int32 StepIndex);

	UFUNCTION(BlueprintCallable, Category = "Lesson Manager")
	bool CompleteCurrentStep();

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


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// === STEP MANAGEMENT ===
        
    ULessonStep* CreateStepFromData(const FLessonStepData& StepData);
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
    
	void UpdateValidator();

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
};

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
