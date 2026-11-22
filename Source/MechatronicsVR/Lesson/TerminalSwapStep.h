// Copyright notice placeholder, matching the other lesson steps.

#pragma once

#include "CoreMinimal.h"
#include "LessonStep.h"
#include "TerminalSwapStep.generated.h"

class APartActor;

/**
 * Lesson step that completes once a watched part has been moved to a DIFFERENT terminal than the one it was on
 * when it was first seen connected during this step.
 *
 * Used by the Electromagnetic Rotation lesson: Payton asks the player to swap the two battery leads, and this step
 * holds the lesson (so he doesn't move on to "next, we will build a DC motor") until the swap actually happens.
 *
 * "Terminal" is read from USnapPointComponent::ConnectedSnapPoint->SnapID, NOT from AAssemblyActor::Connections,
 * because the assembly wipes its connection list one tick after every snap (see CleanupInvalidConstraints).
 *
 * Lifecycle:
 *  - OnStarted: remembers the terminal the part is on now (NAME_None if it isn't connected yet).
 *  - TickStep (timer, TickInterval): if the baseline is still NAME_None, adopts the first terminal the part connects
 *    to. After that, completes when the part is connected to a terminal whose SnapID differs from the baseline.
 *    A part that is detached (no terminal) is simply waiting; putting it back on the same terminal does not complete.
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class MECHATRONICSVR_API UTerminalSwapStep : public ULessonStep
{
	GENERATED_BODY()

public:
	UTerminalSwapStep();

	/**
	 * Class of the part whose terminal is watched (for the Electromagnetic lesson: BP_ContactPart, i.e. Contact1).
	 * Exact class matches are preferred over subclasses, so sibling wire classes don't get mixed up.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terminal Swap Step|Setup")
	TSubclassOf<APartActor> WatchedPartClass;

	/** True once the watched part has been seen on a terminal different from its baseline. */
	virtual bool CheckCompletion_Implementation() const override;

protected:
	virtual void OnStarted() override;
	virtual void OnStopped() override;

	/** Replaces the base TickStep, which stops its own timer on the first call. Samples the terminal and completes the step. */
	virtual void TickStep(float DeltaTime) override;

private:
	/** @return the part being watched, or nullptr if none exists in the world. */
	APartActor* FindWatchedPart() const;

	/**
	 * @return SnapID of the snap point the watched part is currently plugged into (e.g. BatteryPositiveTerminal),
	 * or NAME_None when it is not connected to anything.
	 */
	FName GetCurrentTerminalID() const;

	/** Terminal seen when the part was first connected during this step (NAME_None until then). */
	UPROPERTY(Transient)
	FName BaselineTerminalID = NAME_None;

	/** Set by TickStep when a different terminal is observed. */
	UPROPERTY(Transient)
	bool bSwapped = false;
};
