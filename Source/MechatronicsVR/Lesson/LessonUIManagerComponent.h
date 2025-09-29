#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundBase.h"
#include "LessonUIManagerComponent.generated.h"

class APartActor;
class ULessonStep;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInstructionShown, const FText&, InstructionText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProgressUpdated, float, ProgressPercentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartsHighlighted, const TArray<APartActor*>&, HighlightedParts);

UENUM(BlueprintType)
enum class ELessonUIVisibility : uint8
{
    Always          UMETA(DisplayName = "Always Visible"),
    OnlyDuringSteps UMETA(DisplayName = "Only During Steps"),
    OnDemand        UMETA(DisplayName = "On Demand Only")
};

UENUM(BlueprintType)
enum class EHighlightType : uint8
{
    Outline         UMETA(DisplayName = "Outline Highlight"),
    Glow           UMETA(DisplayName = "Glow Effect"),
    Material       UMETA(DisplayName = "Material Override"),
    Pulse          UMETA(DisplayName = "Pulse Animation")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ULessonUIManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULessonUIManagerComponent();

    // === 3D WIDGET COMPONENTS ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI Components")
    TObjectPtr<UWidgetComponent> InstructionWidgetComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI Components")
    TObjectPtr<UWidgetComponent> ProgressWidgetComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI Components")
    TObjectPtr<UWidgetComponent> LessonHUDComponent;

    // === WIDGET CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Widgets")
    TSubclassOf<UUserWidget> InstructionWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Widgets")
    TSubclassOf<UUserWidget> ProgressWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Widgets")
    TSubclassOf<UUserWidget> LessonHUDClass;

    // === 3D POSITIONING SETTINGS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Positioning")
    FVector InstructionWidgetOffset = FVector(150.0f, 0.0f, 100.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Positioning")
    FVector ProgressWidgetOffset = FVector(100.0f, -200.0f, 50.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Positioning")
    FVector LessonHUDOffset = FVector(200.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Positioning")
    FVector2D InstructionWidgetSize = FVector2D(400.0f, 200.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Positioning")
    FVector2D ProgressWidgetSize = FVector2D(300.0f, 100.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Positioning")
    FVector2D LessonHUDSize = FVector2D(500.0f, 300.0f);

    // === FOLLOW/LOOK-AT BEHAVIOR ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Behavior")
    bool bInstructionFollowsPlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Behavior")
    bool bInstructionLooksAtPlayer = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Behavior")
    bool bProgressFollowsPlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Behavior")
    bool bProgressLooksAtPlayer = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Behavior")
    float FollowSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Behavior")
    float LookAtSpeed = 10.0f;

    // === UI VISIBILITY SETTINGS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Visibility")
    ELessonUIVisibility InstructionVisibility = ELessonUIVisibility::OnlyDuringSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Visibility")
    ELessonUIVisibility ProgressVisibility = ELessonUIVisibility::Always;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Visibility")
    bool bShowInstructionIcons = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Visibility")
    bool bShowStepNumbers = true;

    // === HIGHLIGHT CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Highlighting")
    EHighlightType DefaultHighlightType = EHighlightType::Outline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Highlighting")
    TObjectPtr<UMaterialInterface> HighlightMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Highlighting")
    TObjectPtr<UMaterialInterface> TargetPartMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Highlighting")
    FLinearColor HighlightColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Highlighting")
    FLinearColor TargetPartColor = FLinearColor::Yellow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Highlighting")
    float HighlightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Highlighting")
    float PulseSpeed = 2.0f;

    // === ANIMATION SETTINGS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Animation")
    float FadeInDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Animation")
    float FadeOutDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Animation")
    bool bAnimateInstructions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Animation")
    bool bAnimateProgress = true;

    // === AUDIO SETTINGS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Audio")
    TObjectPtr<USoundBase> InstructionSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Audio")
    TObjectPtr<USoundBase> ProgressSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Audio")
    TObjectPtr<USoundBase> HighlightSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Audio")
    float AudioVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Configuration|Audio")
    bool bPlayInstructionSound = true;

    // === RUNTIME WIDGETS (Retrieved from Components) ===
    
    UPROPERTY(BlueprintReadOnly, Category = "UI Runtime")
    TObjectPtr<UUserWidget> InstructionWidget;

    UPROPERTY(BlueprintReadOnly, Category = "UI Runtime")
    TObjectPtr<UUserWidget> ProgressWidget;

    UPROPERTY(BlueprintReadOnly, Category = "UI Runtime")
    TObjectPtr<UUserWidget> LessonHUD;

    // === RUNTIME STATE ===
    
    UPROPERTY(BlueprintReadOnly, Category = "UI State")
    TArray<TObjectPtr<AActor>> HighlightedActors;

    UPROPERTY(BlueprintReadOnly, Category = "UI State")
    FText CurrentInstructionText;

    UPROPERTY(BlueprintReadOnly, Category = "UI State")
    float CurrentProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "UI State")
    int32 CurrentStepNumber = 0;

    UPROPERTY(BlueprintReadOnly, Category = "UI State")
    int32 TotalSteps = 0;

    // === EVENTS ===
    
    UPROPERTY(BlueprintAssignable, Category = "UI Events")
    FOnInstructionShown OnInstructionShown;

    UPROPERTY(BlueprintAssignable, Category = "UI Events")
    FOnProgressUpdated OnProgressUpdated;

    UPROPERTY(BlueprintAssignable, Category = "UI Events")
    FOnPartsHighlighted OnPartsHighlighted;

    // === PUBLIC API ===
    

    UFUNCTION(BlueprintCallable, Category = "Lesson UI")
    void ShowInstructionWithIcon(const FText& InstructionText, UTexture2D* Icon);

    UFUNCTION(BlueprintCallable, Category = "Lesson UI")
    void ShowInstructionAtLocation(const FString& InstructionText, const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Lesson UI")
    void ShowInstructionForPart(const FString& InstructionText, APartActor* TargetPart);

    UFUNCTION(BlueprintCallable, Category = "Lesson UI")
    void HideInstruction();

    UFUNCTION(BlueprintCallable, Category = "Lesson UI")
    void UpdateProgress(float ProgressPercentage);

    UFUNCTION(BlueprintCallable, Category = "Lesson UI")
    void UpdateStepProgress(int32 CurrentStep, int32 TotalStepsCount);

    UFUNCTION(BlueprintCallable, Category = "Lesson UI")
    void HighlightParts(const TArray<APartActor*>& PartsToHighlight);

    UFUNCTION(BlueprintCallable, Category = "Lesson UI")
    void HighlightPartsWithType(const TArray<APartActor*>& PartsToHighlight, EHighlightType HighlightType);

    UFUNCTION(BlueprintCallable, Category = "Lesson UI")
    void HighlightSinglePart(APartActor* PartToHighlight, const FLinearColor& Color = FLinearColor::Green);

    UFUNCTION(BlueprintCallable, Category = "Lesson UI")
    void ClearHighlights();

    UFUNCTION(BlueprintCallable, Category = "Lesson UI")
    void ClearHighlight(APartActor* Part);

    // === 3D POSITIONING API ===
    
    UFUNCTION(BlueprintCallable, Category = "Lesson UI|Positioning")
    void PositionInstructionWidget(const FVector& WorldLocation, const FRotator& WorldRotation = FRotator::ZeroRotator) const;

    UFUNCTION(BlueprintCallable, Category = "Lesson UI|Positioning")
    void PositionWidgetNearActor(AActor* TargetActor, const FVector& Offset = FVector(150.0f, 0.0f, 100.0f));

    UFUNCTION(BlueprintCallable, Category = "Lesson UI|Positioning")
    void PositionWidgetNearPart(APartActor* TargetPart, const FVector& Offset = FVector(150.0f, 0.0f, 100.0f));

    UFUNCTION(BlueprintCallable, Category = "Lesson UI|Positioning")
    void SetWidgetFollowMode(bool bShouldFollow, AActor* ActorToFollow = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Lesson UI|Positioning")
    void ResetWidgetPositions();
    void ShowInstruction(const FString& InstructionText) const;

    // === ADVANCED UI FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Lesson UI")
    void SetUIVisibility(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Lesson UI")
    void FadeInUI();

    UFUNCTION(BlueprintCallable, Category = "Lesson UI")
    void FadeOutUI();

    UFUNCTION(BlueprintCallable, Category = "Lesson UI")
    void PulseHighlights();

    UFUNCTION(BlueprintCallable, Category = "Lesson UI")
    void ShowSuccessAnimation();

    UFUNCTION(BlueprintCallable, Category = "Lesson UI")
    void ShowErrorAnimation();

    // === WIDGET COMPONENT ACCESS ===
    
    UFUNCTION(BlueprintPure, Category = "Lesson UI")
    UWidgetComponent* GetInstructionWidgetComponent() const { return InstructionWidgetComponent; }

    UFUNCTION(BlueprintPure, Category = "Lesson UI")
    UWidgetComponent* GetProgressWidgetComponent() const { return ProgressWidgetComponent; }

    UFUNCTION(BlueprintPure, Category = "Lesson UI")
    UWidgetComponent* GetLessonHUDComponent() const { return LessonHUDComponent; }

    // === WIDGET ACCESS ===
    
    UFUNCTION(BlueprintPure, Category = "Lesson UI")
    UUserWidget* GetInstructionWidget() const { return InstructionWidget; }

    UFUNCTION(BlueprintPure, Category = "Lesson UI")
    UUserWidget* GetProgressWidget() const { return ProgressWidget; }

    UFUNCTION(BlueprintPure, Category = "Lesson UI")
    UUserWidget* GetLessonHUD() const { return LessonHUD; }

    // === QUERY FUNCTIONS ===
    
    UFUNCTION(BlueprintPure, Category = "Lesson UI")
    bool IsUIVisible() const;

    UFUNCTION(BlueprintPure, Category = "Lesson UI")
    bool IsPartHighlighted(APartActor* Part) const;

    UFUNCTION(BlueprintPure, Category = "Lesson UI")
    int32 GetHighlightedPartsCount() const { return HighlightedActors.Num(); }

    void DebugShowWidgetLocation();

protected:
    // === UNREAL OVERRIDES ===
    
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === WIDGET MANAGEMENT ===
    
    void CreateUIWidgets();
    void DestroyUIWidgets();
    void RefreshWidgetReferences();

    // === INSTRUCTION MANAGEMENT ===
    
    void UpdateInstructionWidget(const FString& InstructionText) const;
    void UpdateProgressWidget(float ProgressPercentage);
    void AnimateInstructionIn();
    void AnimateInstructionOut();

    // === 3D POSITIONING ===
    
    void UpdateWidgetPositions(float DeltaTime) const;
    FVector GetPlayerLocation() const;
    FVector GetPlayerForwardVector() const;
    void UpdateLookAtBehavior(float DeltaTime);

    // === HIGHLIGHTING IMPLEMENTATION ===
    
    void ApplyHighlightToActor(AActor* Actor, const FLinearColor& Color, EHighlightType HighlightType);
    void RemoveHighlightFromActor(AActor* Actor);
    void CreateHighlightMaterial(const FLinearColor& Color);
    void UpdatePulsingHighlights(float DeltaTime);

    // === AUDIO ===
    
    void PlayUISound(USoundBase* Sound);

    // === HELPER FUNCTIONS ===
    
    UTextBlock* FindTextBlockInWidget(UUserWidget* Widget, const FString& TextBlockName);
    UProgressBar* FindProgressBarInWidget(UUserWidget* Widget, const FString& ProgressBarName);
    UImage* FindImageInWidget(UUserWidget* Widget, const FString& ImageName);

private:
    // === INTERNAL STATE ===
    
    UPROPERTY()
    TMap<TObjectPtr<AActor>, TObjectPtr<UMaterialInstanceDynamic>> HighlightMaterials;

    UPROPERTY()
    TArray<TObjectPtr<UMaterialInterface>> OriginalMaterials;

    UPROPERTY()
    TObjectPtr<AActor> FollowTarget;

    bool bUIVisible = true;
    float PulseTimer = 0.0f;
    bool bTickEnabled = false;

    // === ANIMATION TIMERS ===


    FTimerHandle DebugTestTimer;
    
    FTimerHandle FadeInTimer;
    FTimerHandle FadeOutTimer;
    FTimerHandle PulseTimer_Handle;
};