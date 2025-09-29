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
	if (bIsActive)
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

	if (CheckCompletion())
	{
		CompleteStep();
	}
}

void ULessonStep::CompleteStep()
{
	if (!bIsActive)
	{
		return;
	}

	OnStopped();
	bIsActive = false;
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








