#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "LessonUIActor.generated.h"

class APartActor;
class UTextBlock;

UCLASS()
class MECHATRONICSVR_API ALessonUIActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ALessonUIActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// === WIDGET COMPONENT ===
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> WidgetComponent;

	// === CONFIGURATION ===
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration")
	FVector2D WidgetDrawSize = FVector2D(800, 400);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration")
	bool bAlwaysFacePlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration")
	float RotationSpeed = 10.0f;

	// === PUBLIC API ===
	
	UFUNCTION(BlueprintCallable, Category = "Lesson UI")
	void ShowInstruction(const FString& InstructionString);

	UFUNCTION(BlueprintCallable, Category = "Lesson UI")
	void HideInstruction();

	UFUNCTION(BlueprintCallable, Category = "Lesson UI")
	void SetInstructionText(const FString& InstructionString);

	UFUNCTION(BlueprintCallable, Category = "Lesson UI")
	void PositionNearActor(AActor* TargetActor, const FVector& Offset = FVector(150, 0, 100));

	UFUNCTION(BlueprintCallable, Category = "Lesson UI")
	void SetVisible(bool bVisible);

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> CachedWidget;

	void UpdateFacingPlayer(float DeltaTime);
	FVector GetPlayerLocation() const;
};