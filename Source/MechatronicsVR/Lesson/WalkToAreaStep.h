// Copyright notice placeholder, matching the other lesson steps.

#pragma once

#include "CoreMinimal.h"
#include "LessonStep.h"
#include "WalkToAreaStep.generated.h"

/**
 * Lesson step that completes when the player walks into a marked area of the level.
 *
 * Used to hand the player from one lesson to the next: after the Electromagnetic Rotation lesson, Payton tells the
 * player to walk into the DC motor room, and this step holds the lesson open until the player is actually there.
 * When it completes it is the last step, so the lesson ends, ULessonManagerComponent::OnLessonCompleted fires and
 * AMechatronicsGameMode starts ULessonDataAsset::NextLesson (the DC motor lesson).
 *
 * The area is a level actor found by ACTOR TAG (a lesson data asset can't hard-reference level actors). Place a
 * Trigger Box (or any actor / volume) over the doorway or room and add TargetActorTag to its Tags array.
 * "Inside" is tested against the player's HEAD (camera) position, not the pawn root, because in room-scale VR the
 * pawn root stays put while the player physically walks around.
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class MECHATRONICSVR_API UWalkToAreaStep : public ULessonStep
{
	GENERATED_BODY()

public:
	UWalkToAreaStep();

	/** Tag on the level actor (e.g. a Trigger Box) that marks the area the player must enter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk To Area Step|Setup")
	FName TargetActorTag = TEXT("DCMotorRoom");

	/** True once the player has been seen inside the area. */
	virtual bool CheckCompletion_Implementation() const override;

protected:
	virtual void OnStarted() override;
	virtual void OnStopped() override;

	/** Replaces the base TickStep, which stops its own timer on the first call. Tests the player's position and completes the step. */
	virtual void TickStep(float DeltaTime) override;

private:
	/** @return the actor carrying TargetActorTag, or nullptr if none exists in any loaded level. */
	AActor* FindTargetActor() const;

	/**
	 * @param OutLocation Receives the player's head position (camera), or the pawn location if there is no camera.
	 * @return false if there is no local player yet.
	 */
	bool GetPlayerLocation(FVector& OutLocation) const;

	/** @return true if Point is inside Area (volume shape when it is an AVolume, else its component bounds). */
	static bool IsPointInArea(const AActor* Area, const FVector& Point);

	/** Set by TickStep when the player is inside the area. */
	UPROPERTY(Transient)
	bool bPlayerArrived = false;

	/** Avoids logging the "no actor with tag" warning every tick. */
	bool bWarnedMissingTarget = false;
};
