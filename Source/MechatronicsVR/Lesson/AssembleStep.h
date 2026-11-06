// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LessonStep.h"
#include "AssembleStep.generated.h"

class AAssemblyActor;
class APartActor;



/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class MECHATRONICSVR_API UAssembleStep : public ULessonStep
{
	GENERATED_BODY()
public:
	UAssembleStep();

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assemble Step|References")
	TSubclassOf<AAssemblyActor> AssemblyActorClass;
	

	/**
	 * Part classes this step targets (original method; existing lessons use it).
	 *
	 * Satisfied once ANY instance of each class is assembled. Ignored when TargetPartNames is set,
	 * so a step can migrate to named instances by filling TargetPartNames, then clearing this.
	 */
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Assemble Step|Targets")
	TArray<TSubclassOf<APartActor>> TargetPartClasses;

	/**
	 * Specific level parts this step targets, by object name (AActor::GetFName(), e.g. "Contact1").
	 *
	 * When non-empty this replaces TargetPartClasses everywhere (completion, snap previews,
	 * snap validation, instruction placement), and the step completes only when EVERY named part
	 * has been snapped into the assembly. Use it when a level has several parts of one class that
	 * all must be placed (one class entry would be satisfied by the first). Object names are
	 * stable in PIE and packaged builds; the Outliner label is editor-only, so rename the actor's
	 * object to match its label if you want them to read the same.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assemble Step|Targets")
	TArray<FName> TargetPartNames;

	/** True when this step targets specific named parts (TargetPartNames) rather than classes. */
	bool UsesNamedTargets() const { return TargetPartNames.Num() > 0; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assemble Step|Criteria")
	bool bRequireAllTargetsPresent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assemble Step|Criteria")
	bool bRequireTargetConnected = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assemble Step|Criteria")
	bool bRequireFullyAssembled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assemble Step|Criteria")
	int32 MinTargetsSatisfied = 0;

	UPROPERTY()
	TObjectPtr<AAssemblyActor> AssemblyActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assemble Step|Perf")
	bool bAlsoPollEachTick = false;

	UFUNCTION(BlueprintCallable, Category = "Assemble Step|Setup")
	void SetAssemblyActor(AAssemblyActor* InAssembly);

	virtual bool CheckCompletion_Implementation() const override;
	UFUNCTION()
	void HandlePartsConnected(APartActor* PartA, APartActor* PartB);

	/**
	 * Whether a part is one this step asks the learner to assemble.
	 *
	 * Matches by object name against TargetPartNames when that list is set, otherwise by
	 * class against TargetPartClasses. Shared by the LessonManager (instruction placement) and
	 * SnapValidatorComponent (which parts may snap during this step).
	 *
	 * @param Part Part to test; nullptr returns false.
	 * @return True if the part is a target of this step.
	 */
	bool IsTargetPart(const APartActor* Part) const;

protected:
	virtual void OnStarted() override;
	virtual void OnStopped() override;
	virtual void OnReset() override;

	// Track current target part instances for visual preview management
	UPROPERTY()
	TArray<TObjectPtr<APartActor>> CurrentTargetPartInstances;

	// Enable arrows and outlines for target parts in this step
	void UpdateTargetPartVisuals();

	// Disable arrows and outlines for target parts
	void ClearTargetPartVisuals();

	// Find all part actors that match the target part classes
	void FindTargetPartInstances(TArray<APartActor*>& OutParts) const;
	

private:

	
	void BindAssemblyEvents();
	void UnbindAssemblyEvents();

	void EvaluateConnectionStatus();

	bool AreTargetsPresent(TMap<TSubclassOf<APartActor>, TArray<APartActor*>> &OutFound) const;

	bool AreTargetsConnected(const TMap<TSubclassOf<APartActor>, TArray<APartActor*>> &Found) const;

	/**
	 * Checks that every part named in TargetPartNames exists and has been snapped into the assembly.
	 *
	 * A part counts as placed when it is in AssemblyActor->Parts (added by ConnectParts on a
	 * successful snap) and at least one of its snap points has bIsAssembled set. Snap state is used
	 * instead of Connections because AssemblyActor currently drops connections one tick after
	 * creating them.
	 *
	 * @return False if AssemblyActor is invalid, a named part is missing from the level, or any
	 *         named part is not yet placed; true otherwise.
	 */
	bool AreAllNamedTargetsPlaced() const;

	UFUNCTION()
	void HandleAssemblyStateChanged(EAssemblyState NewState);

	
	
};
