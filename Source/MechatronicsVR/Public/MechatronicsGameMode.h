// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "../Lesson/LessonManagerComponent.h"
#include "../Lesson/LessonDataAsset.h"
#include "../Lesson/LessonTypes.h"
#include "MechatronicsGameMode.generated.h"

/**
 * 
 */
// Forward declarations
class ULessonManagerComponent;
class ULessonUIManagerComponent;
class ULessonDataAsset;
class AAssemblyActor;

UCLASS()
class MECHATRONICSVR_API AMechatronicsGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AMechatronicsGameMode();

	// === VR CONFIGURATION ===
    
	/** VR Pawn class to use (can be set in editor) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Configuration")
	TSubclassOf<APawn> VRPawnClass;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:


	
	// === LESSON SYSTEM COMPONENTS ===
	/** Manages lesson progression and step validation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lesson System")
	TObjectPtr<ULessonManagerComponent> LessonManager;

	//** Manages lesson UI elements */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lesson System")
	TObjectPtr<ULessonUIManagerComponent> UIManager;

	// === LESSON CONFIGURATION ===
	/** Lesson data to load automatically when level starts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lesson System")
	TObjectPtr<ULessonDataAsset> AutoStartLessonData;

	/** Assembly actor that this lesson will focus on */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lesson System")
	TSubclassOf<AAssemblyActor> AssemblyActorClass;

	/** Whether to start lesson automatically on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lesson Configuration")
	bool bAutoStartLesson = true;

	// === LEVEL-SPECIFIC LESSON CONFIGURATION ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Lesson")
	TObjectPtr<ULessonDataAsset> LevelLessonData;



	// === PUBLIC API ===
    
	/** Start a specific lesson */
	UFUNCTION(BlueprintCallable, Category = "Lesson System")
	bool StartLesson(ULessonDataAsset* LessonData);


	/** Stop current lesson */
	UFUNCTION(BlueprintCallable, Category = "Lesson System")
	void StopLesson();

	/** Check if a lesson is currently active */
	UFUNCTION(BlueprintPure, Category = "Lesson System")
	bool IsLessonActive() const;

	/** Get current lesson progress (0-100) */
	UFUNCTION(BlueprintPure, Category = "Lesson System")
	float GetLessonProgress() const;

	// === ACCESSORS ===
    
	UFUNCTION(BlueprintPure, Category = "Lesson System")
	ULessonManagerComponent* GetLessonManager() const { return LessonManager; }

	UFUNCTION(BlueprintPure, Category = "Lesson System")
	ULessonUIManagerComponent* GetUIManager() const { return UIManager; }

protected:
	// === INTERNAL FUNCTIONS ===
    
	/** Initialize lesson system components */
	void InitializeLessonSystem();

	/**
	 * Bound to LessonManager->OnLessonCompleted. If the finished lesson has a NextLesson,
	 * starts it after the lesson's NextLessonDelaySeconds. Ignored when the lesson was
	 * ended by StopLesson() or level shutdown rather than finishing its last step.
	 * @param LessonData The lesson that just ended (may be null).
	 */
	UFUNCTION()
	void HandleLessonCompleted(ULessonDataAsset* LessonData);



private:
	UPROPERTY()
	TObjectPtr<AAssemblyActor> AssemblyActor = nullptr;

	/** Track if lesson system has been initialized */
	bool bLessonSystemInitialized = false;
	FTimerHandle LessonStartTimerHandle;

	/** Timer that starts the follow-up lesson after one completes. */
	FTimerHandle NextLessonTimerHandle;

	/** True while StopLesson()/EndPlay is ending a lesson, so that end doesn't chain to NextLesson. */
	bool bSuppressLessonChaining = false;
};
