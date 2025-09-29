// Fill out your copyright notice in the Description page of Project Settings.


#include "AssembleStep.h"
#include "AssemblyActor.h"
#include "PartActor.h"
#include "DSP/AudioDebuggingUtilities.h"
#include "Engine/Engine.h"

UAssembleStep::UAssembleStep()
{
	bWantsTickWhileActive = false;
	bAlsoPollEachTick = false;

	bRequireAllTargetsPresent = true;
	bRequireTargetConnected = true;
	bRequireFullyAssembled = false;
	MinTargetsSatisfied = 0;
	StepType = ELessonStepType::Assemble;
	
}

void UAssembleStep::SetAssemblyActor(AAssemblyActor* InAssembly)
{
	AssemblyActor = InAssembly;
}

bool UAssembleStep::CheckCompletion_Implementation() const
{
	if (!IsValid(AssemblyActor))
		return false;
	if (bRequireFullyAssembled)
	{
		return AssemblyActor->IsFullyAssembled();
	}
	TMap<TSubclassOf<APartActor>, TArray<APartActor*>> Found;
	if (!AreTargetsPresent(Found))
		return false;

	if (!AreTargetsConnected(Found))
		return false;

	return true;
}

void UAssembleStep::OnStarted()
{

	if (!IsValid(AssemblyActor))
	{
		UE_LOG(LogTemp,Error,TEXT("AssemblyActor is not set"))
		return;
	}
	
	BindAssemblyEvents();
	
	if (CheckCompletion())
	{
		return;
	}
	NotifyUpdated();
}

void UAssembleStep::OnStopped()
{
	UnbindAssemblyEvents();
}

void UAssembleStep::OnReset()
{
	UnbindAssemblyEvents();
}

void UAssembleStep::BindAssemblyEvents()
{
	AssemblyActor->OnAssemblyStateChanged.AddDynamic(this, &UAssembleStep::HandleAssemblyStateChanged);

	AssemblyActor->OnPartsConnected.AddDynamic(this, &UAssembleStep::HandlePartsConnected);
}

void UAssembleStep::UnbindAssemblyEvents()
{
	AssemblyActor->OnAssemblyStateChanged.RemoveDynamic(this, &UAssembleStep::HandleAssemblyStateChanged);

	AssemblyActor->OnPartsConnected.RemoveDynamic(this, &UAssembleStep::HandlePartsConnected);
}

void UAssembleStep::EvaluateAndMaybeComplete()
{
	if (bAlsoPollEachTick)
	{
		NotifyUpdated();
	}
	if (CheckCompletion())
	{
		CompleteStep();
	}
}

bool UAssembleStep::AreTargetsPresent(TMap<TSubclassOf<APartActor>, TArray<APartActor*>>& OutFound) const
{
	OutFound.Reset();

	if (!IsValid(AssemblyActor)) return false;

	const TArray<TObjectPtr<APartActor>>& Parts = AssemblyActor->Parts;

	if (TargetPartClasses.Num() == 0) return true;

	for (TSubclassOf<APartActor> TargetClass : TargetPartClasses)
	{
		if (!*TargetClass) continue;

		TArray<APartActor*>& Bucket = OutFound.FindOrAdd(TargetClass);
		for (APartActor* P : Parts)
		{
			if (IsValid(P) && P->IsA(TargetClass))
			{
				Bucket.Add(P);
			}
		}
	}

	int32 SatisfiedClasses = 0;
	for (const auto &Kvp : OutFound)
	{
		if (Kvp.Value.Num() > 0)
		{
			++SatisfiedClasses;
		}
	}

	if (MinTargetsSatisfied > 0 && SatisfiedClasses > MinTargetsSatisfied)
	{
		return false;
	}

	if (bRequireAllTargetsPresent)
	{
		for (TSubclassOf<APartActor> TargetClass : TargetPartClasses)
		{
			if (!*TargetClass) continue;
			const TArray<APartActor*>* BucketPtr = OutFound.Find(TargetClass);
			if (!BucketPtr || BucketPtr->Num() == 0)
			{
				return false;
			}
		}
		return true;
	}
	else
	{
		return SatisfiedClasses > 0 || (TargetPartClasses.Num() == 0);
	}
}

bool UAssembleStep::AreTargetsConnected(const TMap<TSubclassOf<APartActor>, TArray<APartActor*>>& Found) const
{
	if (!IsValid(AssemblyActor)) return false;

	if (!bRequireTargetConnected) return true;

	if (TargetPartClasses.Num() == 0) return true;

	for (const auto& Kvp : Found)
	{
		const TArray<APartActor*>& Instances = Kvp.Value;
		if (Instances.Num() == 0) continue;
		for (APartActor* P : Instances)
		{
			bool bHasAnyConnection = false;
			for (const auto& Conn : AssemblyActor->Connections)
			{
				if (!Conn.bIsConnected) continue;
				if (Conn.PartA == P || Conn.PartB == P)
				{
					bHasAnyConnection = true;
					break;
				}
			}
			if (!bHasAnyConnection) return false;
		}
	}
	return true;
}

void UAssembleStep::HandleAssemblyStateChanged(EAssemblyState /*NewState*/)
{
	EvaluateAndMaybeComplete();
}

void UAssembleStep::HandlePartsConnected(APartActor* PartA, APartActor* PartB)
{
	// Handle base connection (one part is null, meaning base connection)
	if (!PartA || !PartB)
	{
		// This is a base connection - one of the parts is the base
		APartActor* ConnectedPart = PartA ? PartA : PartB;
        
		if (ConnectedPart)
		{
			UE_LOG(LogTemp, Log, TEXT("AssembleStep: Part %s connected to base"), *ConnectedPart->GetName());
			EvaluateAndMaybeComplete();
		}
		return;
	}

	// Handle normal part-to-part connection
	UE_LOG(LogTemp, Log, TEXT("AssembleStep: Parts %s and %s connected"), *PartA->GetName(), *PartB->GetName());
	EvaluateAndMaybeComplete();
}
// Helper function to check if a single part is a target
bool UAssembleStep::IsTargetPart(APartActor* Part) const
{
	if (!Part) return false;
    
	for (TSubclassOf<APartActor> TargetClass : TargetPartClasses)
	{
		if (Part->IsA(TargetClass))
		{
			return true;
		}
	}
	return false;
}



