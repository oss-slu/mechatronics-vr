// Fill out your copyright notice in the Description page of Project Settings.


#include "MetahumanLessonStep.h"
#include "MetahumanBase.h"
#include "Kismet/GameplayStatics.h"

UMetahumanLessonStep::UMetahumanLessonStep()
{
	
}

void UMetahumanLessonStep::PlayVoiceLine()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMetahumanBase::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		MetahumanRef = Cast<AMetahumanBase>(FoundActors[0]);
		MetahumanRef->TriggerVoiceLine(VoiceLine, VoiceAnimation);
	}
	/**
	if (MetahumanRef)
	{
		MetahumanRef->TriggerVoiceLine(VoiceLine, VoiceAnimation);
	}
	else      // in case reference was not populated or is otherwise broken
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMetahumanBase::StaticClass(), FoundActors);
		if (FoundActors.Num() > 0)
		{
			MetahumanRef = Cast<AMetahumanBase>(FoundActors[0]);
			MetahumanRef->TriggerVoiceLine(VoiceLine, VoiceAnimation);
		}
	}
	*/
}
