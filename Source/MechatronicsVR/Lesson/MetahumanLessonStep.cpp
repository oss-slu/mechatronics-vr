// Fill out your copyright notice in the Description page of Project Settings.


#include "MetahumanLessonStep.h"
#include "MetahumanBase.h"
#include "Kismet/GameplayStatics.h"

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
	if (FoundActors.Num() > 0)
	{
		// Call professor's voice line function with attached audio and animation asset
		Cast<AMetahumanBase>(FoundActors[0])->TriggerVoiceLine(VoiceLine, VoiceAnimation);
	}
}
