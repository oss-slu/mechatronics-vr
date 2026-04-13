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

void UMetahumanLessonStep::PlayVoiceLine()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMetahumanBase::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		Cast<AMetahumanBase>(FoundActors[0])->TriggerVoiceLine(VoiceLine, VoiceAnimation);
	}
}
