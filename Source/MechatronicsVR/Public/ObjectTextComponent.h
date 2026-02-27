// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ObjectTextComponent.generated.h"

class UWidgetComponent;
class UUserWidget;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MECHATRONICSVR_API UObjectTextComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UObjectTextComponent();

protected:
    virtual void OnRegister() override;
    virtual void OnUnregister() override;
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** The text to display (instance-editable) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Object Text")
    FText LabelText = FText::FromString(TEXT("Label"));

    /** Whether the label is visible (instance-editable) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Object Text")
    bool bLabelVisible = true;

    /** Offset above the owning actor (in cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Object Text", meta=(ClampMin="0.0"))
    float HeightOffset = 100.0f;

    /** Widget Blueprint class to display (e.g., WBP_ObjectLabel) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Object Text")
    TSubclassOf<UUserWidget> LabelWidgetClass;

    /** Draw size of the widget in pixels */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Object Text")
    FVector2D DrawSize = FVector2D(1136.f, 640.f);

    /** Pivot for the widget (0..1). (0.5,0) = centered and anchored at top */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Object Text")
    FVector2D Pivot = FVector2D(0.5f, 0.0f);

    /** If true, the label rotates to face the player camera (VR-friendly) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Object Text")
    bool bFacePlayer = true;

    /** If true, don’t tilt up/down (keeps it upright) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Object Text")
    bool bLockPitch = true;

    /** Uniform world scale for the widget component */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Object Text", meta=(ClampMin="0.01"))
    float WorldScale = 1.0f;

    /** Set the displayed label text (updates the widget) */
    UFUNCTION(BlueprintCallable, Category="Object Text")
    void SetLabelText(const FText& NewText);

    /** Set visibility of the label */
    UFUNCTION(BlueprintCallable)
    void SetLabelVisible(bool bInVisible);

    UFUNCTION(BlueprintCallable, Category="Object Text")
    void ShowLabel();

    UFUNCTION(BlueprintCallable, Category="Object Text")
    void HideLabel();

private:
    /** The actual widget component rendered in world space */
    UPROPERTY(Transient)
    UWidgetComponent* WidgetComponent = nullptr;

private:
    void ApplyAllSettings();
    void UpdateWidgetText();
    void UpdateWidgetVisibility();
    void UpdateWidgetTransform();
};