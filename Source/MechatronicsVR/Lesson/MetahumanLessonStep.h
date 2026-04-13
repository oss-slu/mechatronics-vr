// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LessonStep.h"
#include "MetahumanBase.h"
#include "MetahumanLessonStep.generated.h"

/**
 * 
 */
UCLASS()
class MECHATRONICSVR_API UMetahumanLessonStep : public ULessonStep
{
	GENERATED_BODY()
public:
	UMetahumanLessonStep();

	/** Reference to professor object */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metahuman Lesson Step")
	AMetahumanBase* MetahumanRef = nullptr;
	
	/** Sound to play when brought to professor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metahuman Lesson Step")
	USoundBase* VoiceLine = nullptr;

	/** Facial animation corresponding to sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metahuman Lesson Step")
	UAnimSequence* VoiceAnimation = nullptr;

	/** Trigger Metahuman voice line with attached assets*/
	UFUNCTION(BlueprintCallable, Category = "Metahuman Lesson Step")
	void PlayVoiceLine();
};
