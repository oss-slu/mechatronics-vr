// Fill out your copyright notice in the Description page of Project Settings.


#include "LessonManagerComponent.h"
#include "AssembleStep.h"
#include "FocusStep.h"
#include "InteractionStep.h"
#include "MechatronicsGameInstance.h"
#include "AssemblyActor.h"
#include "EngineUtils.h"
#include "PartActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

class UIManager;
class StepValidator;

// Sets default values for this component's properties
ULessonManagerComponent::ULessonManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // check every 100ms

	CurrentStepIndex = 0;
	CurrentStep = nullptr;
	bIsLessonActive = false;
	bIsLessonCompleted = false;
	bAutoAdvanceSteps = true;
	bAllowStepSkipping = false;
	StepTransitionDelay = 1.0f;

	bInitialized = false;
	bWaitingForStepTransition = false;

	// ...
}




// Called when the game starts
void ULessonManagerComponent::BeginPlay()
{
	Super::BeginPlay();

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

	UE_LOG(LogTemp, Log, TEXT("LessonManagerComponent::BeginPlay - Initializing"));

	FindExternalReferences();

	ConnectToExistingSystems();

	// try to get lesson data from game instance
	if (UMechatronicsGameInstance* GameInstance = Cast<UMechatronicsGameInstance>(GetWorld()->GetGameInstance()))
	{
		if (ULessonDataAsset* ActiveLesson = GameInstance->GetActiveLessonData())
		{
			InitializeLessonFromDataAsset(ActiveLesson);
		}
	}
	bInitialized = true;
	// ...
	
	
	
}


// Called every frame
void ULessonManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsLessonActive || bIsLessonCompleted || bWaitingForStepTransition)
		return;

	//check current step completion
	if (CurrentStep && CheckStepCompletion())
	{
		if (bAutoAdvanceSteps)
		{
			CompleteCurrentStep();
		}
	}
	
	// ...
}
// === LESSON INITIALIZATION ===

bool ULessonManagerComponent::InitializeLessonFromDataAsset(ULessonDataAsset* LessonData)
{
	if (!LessonData)
	{
		UE_LOG(LogTemp, Error, TEXT("LessonManagerComponent::InitializeLessonFromDataAsset - Invalid Lesson Data Asset"));
		return false;
	}
	if (!LessonData->IsLessonValid())
	{UE_LOG(LogTemp, Error, TEXT("LessonManagerComponent::InitializeLessonFromDataAsset - Lesson data validation failed"));
	   return false;
	}

	UE_LOG(LogTemp, Log, TEXT("LessonManagerComponent::InitializeLessonFromDataAsset - Initializing lesson: %s"), 
		 *LessonData->LessonTitle.ToString());
    
	// Store lesson data
	CurrentLessonData = LessonData;

	LessonSteps.Empty();
	CurrentStep = nullptr;
	CurrentStepIndex = 0;
	bIsLessonCompleted = false;

	//create steps from lesson data
	for (const FLessonStepData& StepData : LessonData->StepDefinitions)
	{
		if (StepData.StepInstance)
		{
			LessonSteps.Add(StepData.StepInstance);
            
			// Bind to step completion event
			StepData.StepInstance->OnStepCompleted.AddDynamic(this, &ULessonManagerComponent::HandleStepCompleted);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("InitializeLessonFromDataAsset - Null step instance found"));
		}
	}

	for (ULessonStep* Step : LessonSteps)
	{
		if (Step)
		{
			// Give steps world context
			Step->SetWorldContext(this);
        
			// Bind to step completion event
			Step->OnStepCompleted.AddDynamic(this, &ULessonManagerComponent::HandleStepCompleted);
		}
	}
    
	UE_LOG(LogTemp, Log, TEXT("InitializeLessonFromDataAsset - Using %d steps"), LessonSteps.Num());
    
	return LessonSteps.Num() > 0;
}


ULessonStep* ULessonManagerComponent::CreateStepFromData(const FLessonStepData& StepData)
{
	if (!StepData.StepInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateStepFromData - No step instance specified"));
		return nullptr;
	}

	//just return the pre-configured instance directly
	UE_LOG(LogTemp, Log, TEXT("CreateStepFromData - Using step: %s"), 
		 *StepData.StepInstance->GetClass()->GetName());

	return StepData.StepInstance;
}

// === LESSON CONTROL ===

bool ULessonManagerComponent::StartLesson()
{
	if (!CurrentLessonData || LessonSteps.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("LessonManagerComponent::StartLesson - No lesson data or steps available"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("LessonManagerComponent::StartLesson - Starting lesson: %s"), 
		 *CurrentLessonData->LessonTitle.ToString());

	bIsLessonActive = true;
	bIsLessonCompleted = false;
	CurrentStepIndex = 0;
	bWaitingForStepTransition = false;

	// Activate first step
	if (ActivateStep(0))
	{
		OnLessonStarted.Broadcast(CurrentLessonData);
		UpdateUI();
		return true;
	}

	return false;
}

void ULessonManagerComponent::EndLesson()
{
	UE_LOG(LogTemp, Log, TEXT("LessonManagerComponent::EndLesson - Ending lesson"));

	DeactivateCurrentStep();
	bIsLessonActive = false;

	if (!bIsLessonCompleted)
	{
		bIsLessonCompleted = true;
		OnLessonCompleted.Broadcast(CurrentLessonData);
	}
}

bool ULessonManagerComponent::AdvanceStep()
{
	if (!bIsLessonActive || CurrentStepIndex >= LessonSteps.Num() - 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("LessonManagerComponent::AdvanceStep - Cannot advance step"));
		return false;
	}

	return GoToStep(CurrentStepIndex + 1);
}

bool ULessonManagerComponent::GoToStep(int32 StepIndex)
{
	if (StepIndex < 0 || StepIndex >= LessonSteps.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("LessonManagerComponent::GoToStep - Invalid step index: %d"), StepIndex);
		return false;
	}
	if (!bAllowStepSkipping && StepIndex != CurrentStepIndex + 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("LessonManagerComponent::GoToStep - Step skipping not allowed"));
		return false;
	}

	
	UE_LOG(LogTemp, Log, TEXT("LessonManagerComponent::GoToStep - Going to step %d"), StepIndex);

	ULessonStep* OldStep = CurrentStep;

	DeactivateCurrentStep();

	//activate new step

	// Activate new step
	if (ActivateStep(StepIndex))
	{
		OnStepChanged.Broadcast(OldStep, CurrentStep);
		UpdateUI();
		return true;
	}
	return false;
}

void ULessonManagerComponent::CompleteCurrentStep()
{
	if (!CurrentStep)
	{
		UE_LOG(LogTemp, Warning, TEXT("LessonManagerComponent::CompleteCurrentStep - No current step to complete"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("LessonManagerComponent::CompleteCurrentStep - Completing step %d"), CurrentStepIndex);
	CurrentStep->CompleteStep();
	OnStepCompleted.Broadcast(CurrentStep, CurrentStepIndex);

	//check if this was the last step
	if (CurrentStepIndex >= LessonSteps.Num() - 1)
	{
		UE_LOG(LogTemp, Log, TEXT("LessonManagerComponent::CompleteCurrentStep - Lesson completed"));
		bIsLessonCompleted = true;
		EndLesson();

		//notify game instance
		if (UMechatronicsGameInstance* GameInstance = Cast<UMechatronicsGameInstance>(GetWorld()->GetGameInstance()))
		{
			GameInstance->CompleteCurrentLesson();
		}
	}
	else if (bAutoAdvanceSteps)
	{
		//delay before advancing to the next step
		bWaitingForStepTransition = true;
		GetWorld()->GetTimerManager().SetTimer(
			StepTransitionTimer,
			this,
			&ULessonManagerComponent::HandleStepTransitionDelay,
			StepTransitionDelay,
			false  // No loop
		);
	}

	UpdateUI();
}


void ULessonManagerComponent::ResetLesson()
{
	UE_LOG(LogTemp, Log, TEXT("LessonManagerComponent::ResetLesson - Resetting lesson"));
    
	DeactivateCurrentStep();
    
	// Reset all steps
	for (ULessonStep* Step : LessonSteps)
	{
		if (Step)
		{
			Step->bStepCompleted = false;  // Reset the property directly
			Step->ResetStep();
		}
	}
    
	CurrentStepIndex = 0;
	bIsLessonCompleted = false;
	bWaitingForStepTransition = false;
    
	// Clear any pending timers
	if (GetWorld() && StepTransitionTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(StepTransitionTimer);
	}
    
	if (bIsLessonActive)
	{
		ActivateStep(0);
		UpdateUI();
	}
}

// === STEP MANAGEMENT ===

bool ULessonManagerComponent::ActivateStep(int32 StepIndex)
{
	if (StepIndex < 0 || StepIndex >= LessonSteps.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("LessonManagerComponent::ActivateStep - Invalid step index: %d"), StepIndex);
		return false;
	}
	CurrentStepIndex = StepIndex;
	CurrentStep = LessonSteps[StepIndex];

	if (!CurrentStep)
	{
		UE_LOG(LogTemp, Error, TEXT("LessonManagerComponent::ActivateStep - Step at index %d is null"), StepIndex);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("LessonManagerComponent::ActivateStep - Activating step %d: %s"), 
		   StepIndex, *CurrentStep->InstructionText.ToString());
    
	// Update step references with current world objects
	UpdateStepReferences();

	// start the step
	CurrentStep->StartStep();

	//Update validator and UI
	// UpdateValidator();
	UpdateUI();

	return true;
}

// TODO: Make sure to properly stop and clean up the current step, check this function to make sure it works with the steps (JULIAN AND ANTHONY), RN THIS WILL COMPILE BUT IDK IF IT WORKS

void ULessonManagerComponent::DeactivateCurrentStep()
{
	if (CurrentStep)
	{
		UE_LOG(LogTemp, Log, TEXT("LessonManagerComponent::DeactivateCurrentStep - Deactivating step %d"), CurrentStepIndex);
		CurrentStep->StopTicking();
		CurrentStep = nullptr;
	}
}

void ULessonManagerComponent::UpdateStepReferences()
{
	if (!CurrentStep)
		return;

	// Update references for specific step types
	if (UAssembleStep* AssembleStep = Cast<UAssembleStep>(CurrentStep))
	{
		if (AssemblyActor)
		{
			AssembleStep->SetAssemblyActor(AssemblyActor);
		}
	}

	// TODO: Update other step types as needed
}


// === EVENT HANDLERS ===

void ULessonManagerComponent::HandleStepCompleted(ULessonStep* CompletedStep)
{
	UE_LOG(LogTemp, Log, TEXT("LessonManagerComponent::HandleStepCompleted - Step completed: %s"), 
		   CompletedStep ? *CompletedStep->InstructionText.ToString() : TEXT("Unknown"));

	
	if (CompletedStep == CurrentStep)
	{
		CompleteCurrentStep();
	}
}

void ULessonManagerComponent::HandleAssemblyPartsConnected(APartActor* PartA, APartActor* PartB)
{
	// Forward to current step if it's an assembly step
	if (UAssembleStep* AssembleStep = Cast<UAssembleStep>(CurrentStep))
	{
		AssembleStep->HandlePartsConnected(PartA, PartB);
	}
}


void ULessonManagerComponent::HandleStepTransitionDelay()
{
	UE_LOG(LogTemp, Log, TEXT("LessonManagerComponent::HandleStepTransitionDelay - Advancing after delay"));
	bWaitingForStepTransition = false;
	AdvanceStep();

	
}

// === QUERY FUNCTIONS ===

float ULessonManagerComponent::GetLessonProgress() const
{
	if (LessonSteps.Num() == 0)
		return 0.0f;
        
	int32 CompletedSteps = 0;
	for (const ULessonStep* Step : LessonSteps)
	{
		if (Step && Step->bStepCompleted)  // Use the property directly
		{
			CompletedSteps++;
		}
	}
    
	return (static_cast<float>(CompletedSteps) / static_cast<float>(LessonSteps.Num())) * 100.0f;
}
bool ULessonManagerComponent::HasNextStep() const
{
	return CurrentStepIndex < LessonSteps.Num() - 1;
}

bool ULessonManagerComponent::HasPreviousStep() const
{
	return CurrentStepIndex > 0;
}

bool ULessonManagerComponent::CheckStepCompletion()
{
	if (!CurrentStep)
		return false;

	return CurrentStep->CheckCompletion();
}

void ULessonManagerComponent::ValidateCurrentStep()
{
	if (CurrentStep)
	{
		CurrentStep->CheckCompletion();
	}
}

void ULessonManagerComponent::SetUIManager(ULessonUIManagerComponent* InUIManager)
{
	UIManager = InUIManager;
    
	if (UIManager)
	{
		UE_LOG(LogTemp, Log, TEXT("LessonManagerComponent: UI Manager reference set"));
        
		// Optionally bind to UI Manager events here
		// UIManager->OnSomeEvent.AddDynamic(this, &ULessonManagerComponent::HandleSomeEvent);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LessonManagerComponent: UI Manager reference cleared"));
	}
}

// === EXTERNAL REFERENCES ===

void ULessonManagerComponent::FindExternalReferences()
{
	UE_LOG(LogTemp, Log, TEXT("LessonManagerComponent::FindExternalReferences - Finding world references"));
    
	// Find AssemblyActor in the level if not already set
	if (!AssemblyActor)
	{
		AssemblyActor = Cast<AAssemblyActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AAssemblyActor::StaticClass()));
		if (AssemblyActor)
		{
			UE_LOG(LogTemp, Log, TEXT("LessonManagerComponent::FindExternalReferences - Found AssemblyActor: %s"), 
				   *AssemblyActor->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("FindExternalReferences - No AssemblyActor found in level"));
		}
	}
	// TODO: Find UIManager and StepValidator when implemented
	
}

void ULessonManagerComponent::ConnectToExistingSystems()
{
	UE_LOG(LogTemp, Log, TEXT("LessonManagerComponent::ConnectToExistingSystems - Connecting to world systems"));
    
	// Connect to AssemblyActor events
	if (AssemblyActor)
	{
		// Just bind directly - Unreal handles duplicate bindings automatically
		AssemblyActor->OnPartsConnected.AddDynamic(this, &ULessonManagerComponent::HandleAssemblyPartsConnected);
		UE_LOG(LogTemp, Log, TEXT("ConnectToExistingSystems - Connected to AssemblyActor events"));
	}
    
	// TODO: Connect to other systems when available
}

// === UI MANAGEMENT ===
void ULessonManagerComponent::UpdateUI()
{
	//Update progress
	OnLessonProgress.Broadcast(CurrentStepIndex, LessonSteps.Num());

	//show current step instructions
	ShowStepInstructions();

	UE_LOG(LogTemp, Verbose, TEXT("UpdateUI - Progress: %d/%d, Lesson Progress: %.1f%%"), 
		 CurrentStepIndex + 1, LessonSteps.Num(), GetLessonProgress());
}

void ULessonManagerComponent::ShowStepInstructions()
{
	if (CurrentStep)
	{
		if (UIManager)
		{
			// UIManager->ShowInstruction(CurrentStep->InstructionText);
			// TODO: Implement when UIManager is available
			UE_LOG(LogTemp, Log, TEXT("ShowStepInstructions - Would show: %s"), 
				   *CurrentStep->InstructionText.ToString());
		}
		else
		{
			// For now, just log the instruction
			UE_LOG(LogTemp, Log, TEXT("LESSON INSTRUCTION: %s"), 
				   *CurrentStep->InstructionText.ToString());
		}
	}
}

void ULessonManagerComponent::UpdateProgress()
{
	if (UIManager)
	{
		const float ProgressPercentage = GetLessonProgress();
		UIManager->UpdateProgress(ProgressPercentage);
	}
}

