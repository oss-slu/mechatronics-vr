// Fill out your copyright notice in the Description page of Project Settings.


#include "MetahumanLessonStep.h"
#include "MetahumanBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Level.h"
#include "GameFramework/Actor.h"

UMetahumanLessonStep::UMetahumanLessonStep()
{
	
}

void UMetahumanLessonStep::OnStarted()
{
	PlayVoiceLine();
}

void UMetahumanLessonStep::OnStopped()
{
	Super::OnStopped();
}

void UMetahumanLessonStep::PlayVoiceLine()
{
	// Find metahuman professor asset in level
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMetahumanBase::StaticClass(), FoundActors);
	AMetahumanBase* Chosen = nullptr;

	// With several instructors in the world (one per lesson level instance), only the one that
	// belongs to this lesson should speak: same level as the anchor, else the closest to it.
	if (const AActor* Anchor = AnchorActor.Get())
	{
		float BestDistSq = TNumericLimits<float>::Max();
		for (AActor* Actor : FoundActors)
		{
			AMetahumanBase* Candidate = Cast<AMetahumanBase>(Actor);
			if (!Candidate)
			{
				continue;
			}
			if (Candidate->GetLevel() == Anchor->GetLevel())
			{
				Chosen = Candidate;
				break;
			}
			const float DistSq = FVector::DistSquared(Candidate->GetActorLocation(), Anchor->GetActorLocation());
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				Chosen = Candidate;
			}
		}
	}
	else if (FoundActors.Num() > 0)
	{
		Chosen = Cast<AMetahumanBase>(FoundActors[0]);
	}

	if (Chosen)
	{
		UE_LOG(LogTemp, Log, TEXT("MetahumanLessonStep: voice line goes to %s (level %s)"),
			*Chosen->GetName(), Chosen->GetLevel() ? *Chosen->GetLevel()->GetOuter()->GetName() : TEXT("?"));
		// Call professor's voice line function with attached audio and animation asset
		Chosen->TriggerVoiceLine(VoiceLine, VoiceAnimation);
	}
}
