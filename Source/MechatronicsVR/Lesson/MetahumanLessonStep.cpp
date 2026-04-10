// Fill out your copyright notice in the Description page of Project Settings.


#include "MetahumanLessonStep.h"

UMetahumanLessonStep::UMetahumanLessonStep()
{
	// set default metahuman reference to professor
	static ConstructorHelpers::FClassFinder<AActor> MHClassAsset(TEXT("/Script/Engine.Blueprint'/Game/MetaHumans/Payton/BP_Payton.BP_Payton'"));

	if (MHClassAsset.Succeeded())
	{
		MetaHumanClass = MHClassAsset.Class;
	}
}

void UMetahumanLessonStep::PlayVoiceLine()
{
	if (MetaHumanClass)
	{
		// of course I cant call directly...
	}
}
