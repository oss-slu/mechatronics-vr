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
	

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Assemble Step|Targets")
	TArray<TSubclassOf<APartActor>> TargetPartClasses;

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
	bool IsTargetPart(APartActor* Part) const;

protected:
	virtual void OnStarted() override;
	virtual void OnStopped() override;
	virtual void OnReset() override;
	

private:

	
	void BindAssemblyEvents();
	void UnbindAssemblyEvents();

	void EvaluateConnectionStatus();

	bool AreTargetsPresent(TMap<TSubclassOf<APartActor>, TArray<APartActor*>> &OutFound) const;

	bool AreTargetsConnected(const TMap<TSubclassOf<APartActor>, TArray<APartActor*>> &Found) const;

	UFUNCTION()
	void HandleAssemblyStateChanged(EAssemblyState NewState);

	
	
};
