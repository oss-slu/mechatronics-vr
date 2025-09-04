#pragma once

#include "CoreMinimal.h"
#include "LessonTypes.generated.h"

// Forward declarations
class APartActor;

UENUM(BlueprintType)
enum class ELessonStepType : uint8
{
	Assemble    UMETA(DisplayName = "Assembly Step"),
	Focus       UMETA(DisplayName = "Focus Step"), 
	Interaction UMETA(DisplayName = "Interaction Step")
};

USTRUCT(BlueprintType)
struct FLessonStepData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Step")
	ELessonStepType StepType = ELessonStepType::Assemble;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Step", meta = (MultiLine = true))
	FText InstructionText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Step")
	TArray<TSubclassOf<APartActor>> TargetParts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Step")
	TObjectPtr<AActor> FocusTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Step", meta = (ClampMin = "0.0"))
	float CompletionDelay = 0.0f;

	FLessonStepData()
	{
		StepType = ELessonStepType::Assemble;
		InstructionText = FText::FromString("Complete this step");
		CompletionDelay = 0.0f;
	}
};