// Fill out your copyright notice in the Description page of Project Settings.


#include "LessonStep.h"


ULessonStep::ULessonStep()
{
	bStepCompleted = false;
	bIsActive = false;
}

void ULessonStep::SetWorldContext(UObject* InWorldContextObject)
{
	WorldContextObject = InWorldContextObject;
}

UWorld* ULessonStep::GetWorld() const
{
	if (WorldContextObject.IsValid())
	{
		return WorldContextObject->GetWorld();
	}
	 
	// Fallback to router's world
	if (const UObject* Outer = GetOuter())
	{
		return Outer->GetWorld();
	}
    
	return nullptr;
}


void ULessonStep::StartStep()
{
	if (!bIsActive)
	{
		return;
	}

	bIsActive = true;
	bStepCompleted = false;

	UE_LOG(LogTemp, Log, TEXT("ULessonStep::StartStep - Starting step: %s"), 
		   *InstructionText);
    
	StartTicking();  // Add this
	
	OnStarted();
	OnStepStarted.Broadcast(this);
}

void ULessonStep::CompleteStep()
{
	UE_LOG(LogTemp, Error, TEXT("=== LessonStep::CompleteStep ==="));
	UE_LOG(LogTemp, Error, TEXT("  - this pointer: %p"), this);
	UE_LOG(LogTemp, Error, TEXT("  - Class: %s"), *GetClass()->GetName());
    
	if (!bIsActive)
	{
		UE_LOG(LogTemp, Error, TEXT("  - NOT ACTIVE, returning"));
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("  - Calling OnStopped()"));
	OnStopped();
    
	bIsActive = false;
	bStepCompleted = true;
    
	UE_LOG(LogTemp, Error, TEXT("  - Delegate info:"));
	UE_LOG(LogTemp, Error, TEXT("    - IsBound: %s"), OnStepCompleted.IsBound() ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Error, TEXT("    - Delegate address: %p"), &OnStepCompleted);
    
	// Check if we can get invocation list (it's internal, but we can try)
	UE_LOG(LogTemp, Error, TEXT("  - About to call Broadcast()"));
    
	// Try calling IsBoundToObject to see if it's bound to LessonManager
	UE_LOG(LogTemp, Error, TEXT("  - Checking bindings..."));
    
	OnStepCompleted.Broadcast(this);
    
	UE_LOG(LogTemp, Error, TEXT("  - Broadcast() returned"));
	UE_LOG(LogTemp, Error, TEXT("  - If you don't see HandleStepCompleted, the binding is broken"));
}

void ULessonStep::ResetStep()
{
	bIsActive = false;
	bStepCompleted = false;

	OnReset();
	OnStepReset.Broadcast(this);
	NotifyUpdated();
	
}

void ULessonStep::TickStep(float DeltaTime)
{
	if (!bIsActive || !bWantsTickWhileActive || bStepCompleted)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ULessonStep::EndStep - Ending step: %s"), 
		   *InstructionText);
    
	StopTicking();  // Add this
    
	
	if (CheckCompletion())
	{
		CompleteStep();
	}
}

void ULessonStep::StartTicking()
{
	if (bWantsTickWhileActive && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			TickTimer,
			this,
			&ULessonStep::HandleTick,
			TickInterval,
			true  // Loop
		);
	}
}

void ULessonStep::StopTicking()
{
	if (GetWorld() && TickTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(TickTimer);
	}
}

void ULessonStep::HandleTick()
{
	if (bWantsTickWhileActive)
	{
		TickStep(TickInterval);
	}
}


bool ULessonStep::CheckCompletion_Implementation() const
{
	return false; // should be defined in subclasses
}

void ULessonStep::setPreviousStep(ULessonStep* InPreviousStep)
{
	PreviousStep = InPreviousStep;
}

void ULessonStep::setNextStep(ULessonStep* InNextStep)
{
	NextStep = InNextStep;
}




void ULessonStep::NotifyUpdated()
{
	OnStepUpdated.Broadcast(this);
}

void ULessonStep::OnStarted()
{
	//to be defined in subclass
}

void ULessonStep::OnStopped()
{
	//to be defined in subclass
}

void ULessonStep::OnReset()
{
	// to be defined in subclass
}








