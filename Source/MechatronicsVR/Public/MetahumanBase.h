// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MetahumanBase.generated.h"

UCLASS()
class MECHATRONICSVR_API AMetahumanBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMetahumanBase();

	/** Exposed BP Function */
	UFUNCTION(BlueprintImplementableEvent, Category = "Metahuman Events")
	void TriggerVoiceLine(USoundBase* VoiceLine, UAnimSequence* VoiceAnimation);

};