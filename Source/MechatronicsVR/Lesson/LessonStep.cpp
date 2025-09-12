// Fill out your copyright notice in the Description page of Project Settings.


#include "LessonStep.h"


ULessonStep::ULessonStep()
{
	bStepCompleted = false;
	bIsActive = false;
}



void ULessonStep::StartStep()
{
	if (bIsActive)
	{
		return;
	}

	bIsActive = true;
	bStepCompleted = false;

	OnStarted();
	OnStepStarted.Broadcast(this);

	if (CheckCompletion())
	{
		CompleteStep();
	}
}

void ULessonStep::StopStep()
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
	if (CheckCompletion())
	{
		CompleteStep();
	}
}

bool ULessonStep::CheckCompletion_Implementation()
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

void ULessonStep::CompleteStep()
{
	if (bStepCompleted)
	{
		return;
	}
	bStepCompleted = true;

	OnStepCompleted.Broadcast(this);
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








