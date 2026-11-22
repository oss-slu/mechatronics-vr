// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LessonStep.h"
#include "MetahumanBase.h"
#include "MetahumanLessonStep.generated.h"

class AActor;

/**
 * 
 */
UCLASS()
class MECHATRONICSVR_API UMetahumanLessonStep : public ULessonStep
{
	GENERATED_BODY()
public:
	UMetahumanLessonStep();
	
	/** Sound to play when brought to professor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metahuman Lesson Step")
	USoundBase* VoiceLine = nullptr;

	/** Facial animation corresponding to sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metahuman Lesson Step")
	UAnimSequence* VoiceAnimation = nullptr;

	/** Trigger Metahuman voice line with attached assets*/
	UFUNCTION(BlueprintCallable, Category = "Metahuman Lesson Step")
	void PlayVoiceLine();

	/**
	 * Sets the actor that marks where this step's lesson takes place (the lesson's assembly).
	 * PlayVoiceLine uses it to pick the instructor that belongs to this lesson when the world
	 * contains several (e.g. one per level instance). Set by ULessonManagerComponent.
	 * @param InAnchor Actor in the lesson's area; may be null (falls back to the first instructor found).
	 */
	void SetAnchorActor(AActor* InAnchor) { AnchorActor = InAnchor; }

protected:
	/** Lesson location marker used to choose which instructor speaks. */
	TWeakObjectPtr<AActor> AnchorActor;

	virtual void OnStarted() override;
	virtual void OnStopped() override;
};
