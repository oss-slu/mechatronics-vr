// Fill out your copyright notice in the Description page of Project Settings.


#include "MetahumanLessonStep.h"
#include "MetahumanBase.h"

UMetahumanLessonStep::UMetahumanLessonStep()
{

}

void UMetahumanLessonStep::PlayVoiceLine()
{
	if (MetahumanRef)
	{
		MetahumanRef->TriggerVoiceLine(VoiceLine, VoiceAnimation);
	}
}
