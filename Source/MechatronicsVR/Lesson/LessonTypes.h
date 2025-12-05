#pragma once

#include "CoreMinimal.h"
#include "LessonTypes.generated.h"

// Forward declarations
class APartActor;
class ULessonStep;

// Keep the enum for reference, but we won't use it in FLessonStepData anymore
UENUM(BlueprintType)
enum class ELessonStepType : uint8
{
	Assemble    UMETA(DisplayName = "Assembly Step"),
	Focus       UMETA(DisplayName = "Focus Step"), 
	Interaction UMETA(DisplayName = "Interaction Step")
};

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	Grab        UMETA(DisplayName="Grab"),
	Drop        UMETA(DisplayName="Drop"),
	Proximity   UMETA(DisplayName="Proximity"),
	Manual      UMETA(DisplayName="Manual")
};

USTRUCT(BlueprintType)
struct FLessonStepData
{
	GENERATED_BODY()

	// The step instance - configured directly in the data asset editor
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Step", Instanced)
	TObjectPtr<ULessonStep> StepInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Step")
	TSubclassOf<ULessonStep> StepClass;

	FLessonStepData()
	{
		StepInstance = nullptr;
	}
};