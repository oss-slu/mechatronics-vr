// Fill out your copyright notice in the Description page of Project Settings.


#include "AssembleStep.h"
#include "AssemblyActor.h"
#include "PartActor.h"
#include "MechatronicsVR/Public/SnapPointComponent.h"
#include "MechatronicsVR/Public/MechatronicsGameMode.h"
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

static void ClearGuideHighlights(UAssembleStep* Step)
{
	if (!Step || !Step->GetWorld())
	{
		return;
	}

	if (AMechatronicsGameMode* GameMode = Step->GetWorld()->GetAuthGameMode<AMechatronicsGameMode>())
	{
		if (ULessonUIManagerComponent* UIManager = GameMode->GetUIManager())
		{
			UIManager->ClearHighlights();
		}
	}
}

static void UpdateGuideHighlight(UAssembleStep* Step)
{
	if (!Step || !IsValid(Step->AssemblyActor))
	{
		ClearGuideHighlights(Step);
		return;
	}

	USnapPointComponent* TargetSnap = nullptr;
	if (Step->TargetPartClasses.Num() > 0)
	{
		for (TSubclassOf<APartActor> TargetClass : Step->TargetPartClasses)
		{
			if (!TargetClass)
			{
				continue;
			}
			for (APartActor* Part : Step->AssemblyActor->Parts)
			{
				if (IsValid(Part) && Part->IsA(TargetClass))
				{
					const TArray<USnapPointComponent*> SnapPoints = Part->GetSnapPoints();
					for (USnapPointComponent* SnapPoint : SnapPoints)
					{
						if (SnapPoint && !SnapPoint->bIsAssembled)
						{
							TargetSnap = SnapPoint;
							break;
						}
					}
					if (TargetSnap)
					{
						break;
					}
				}
			}
			if (TargetSnap)
			{
				break;
			}
		}
	}

	if (!TargetSnap && Step->AssemblyActor->GetBaseSnapPoints().Num() > 0)
	{
		TargetSnap = Step->AssemblyActor->GetBaseSnapPoints()[0];
	}

	if (!TargetSnap)
	{
		ClearGuideHighlights(Step);
		return;
	}

	if (APartActor* TargetPart = Cast<APartActor>(TargetSnap->GetOwner()))
	{
		if (AMechatronicsGameMode* GameMode = Step->GetWorld()->GetAuthGameMode<AMechatronicsGameMode>())
		{
			if (ULessonUIManagerComponent* UIManager = GameMode->GetUIManager())
			{
				UIManager->HighlightSinglePartWithType(TargetPart, EHighlightType::Outline, FLinearColor::Green);
			}
		}
	}
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
	UpdateGuideHighlight(this);
	
	if (CheckCompletion())
	{
		return;
	}
	NotifyUpdated();
}

void UAssembleStep::OnStopped()
{
	UnbindAssemblyEvents();
	ClearGuideHighlights(this);
}

void UAssembleStep::OnReset()
{
	UnbindAssemblyEvents();
	ClearGuideHighlights(this);
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

void UAssembleStep::EvaluateConnectionStatus()


{

	UE_LOG(LogTemp, Error, TEXT("=== EvaluateConnectionStatus ==="));
	UE_LOG(LogTemp, Error, TEXT("  - bIsActive: %s"), bIsActive ? TEXT("TRUE") : TEXT("FALSE"));  // ← Add this
	UE_LOG(LogTemp, Error, TEXT("  - bStepCompleted: %s"), bStepCompleted ? TEXT("TRUE") : TEXT("FALSE"));  // ← Add this
    
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
		if (!TargetClass) continue;

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
			if (!IsValid(P)) continue;
			bool bHasAnyConnection = false;
			for (const auto& Conn : AssemblyActor->Connections)
			{
				if (!Conn.bIsConnected) continue;
				const bool bPartAValid = !Conn.PartA || IsValid(Conn.PartA);
				const bool bPartBValid = !Conn.PartB || IsValid(Conn.PartB);
				if (!bPartAValid && !bPartBValid) continue;
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
	EvaluateConnectionStatus();
}

void UAssembleStep::HandlePartsConnected(APartActor* PartA, APartActor* PartB)
{
	UE_LOG(LogTemp, Error, TEXT("=== HandlePartsConnected Called ==="));
	UE_LOG(LogTemp, Error, TEXT("  - PartA: %s"), PartA ? *PartA->GetName() : TEXT("NULL (BASE)"));
	UE_LOG(LogTemp, Error, TEXT("  - PartB: %s"), PartB ? *PartB->GetName() : TEXT("NULL"));

	// Handle base connection (one part is null, meaning base connection)
	if (!PartA || !PartB)
	{
		APartActor* ConnectedPart = PartA ? PartA : PartB;
		if (!IsValid(ConnectedPart))
		{
			UE_LOG(LogTemp, Error, TEXT("  - ERROR: Both parts are NULL or invalid!"));
			return;
		}

		UE_LOG(LogTemp, Error, TEXT("  - Detected BASE CONNECTION for %s"), *ConnectedPart->GetName());
		UE_LOG(LogTemp, Error, TEXT("  - Calling EvaluateConnectionStatus()"));
		EvaluateConnectionStatus();
		UE_LOG(LogTemp, Error, TEXT("  - Returned from EvaluateConnectionStatus()"));
		return;
	}

	// Ignore stale/invalid part-to-part events before updating the guide or evaluating completion.
	if (!IsValid(PartA) || !IsValid(PartB))
	{
		UE_LOG(LogTemp, Error, TEXT("  - Ignoring invalid part-to-part connection"));
		return;
	}

	// Detected PART-TO-PART CONNECTION
	UE_LOG(LogTemp, Error, TEXT("  - Detected PART-TO-PART CONNECTION"));
	UE_LOG(LogTemp, Log, TEXT("AssembleStep: Parts %s and %s connected"), *PartA->GetName(), *PartB->GetName());
	UpdateGuideHighlight(this);
	EvaluateConnectionStatus();
}
// Helper function to check if a single part is a target
bool UAssembleStep::IsTargetPart(APartActor* Part) const
{
	if (!Part) return false;

	for (TSubclassOf<APartActor> TargetClass : TargetPartClasses)
	{
		UClass* ClassPtr = TargetClass.Get();
		// Skip if the class reference is null
		if (!ClassPtr) continue;

		if (Part->IsA(ClassPtr))
		{
			return true;
		}
	}
	return false;
}



