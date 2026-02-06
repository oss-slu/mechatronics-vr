// MagneticFieldVisualizerComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "MagneticFieldVisualizerComponent.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

UENUM(BlueprintType)
enum class EMagFieldAxis : uint8
{
    X UMETA(DisplayName="X"),
    Y UMETA(DisplayName="Y"),
    Z UMETA(DisplayName="Z")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MECHATRONICSVR_API UMagneticFieldVisualizerComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UMagneticFieldVisualizerComponent();

    UFUNCTION(BlueprintCallable, Category="MagField")
    void SetEnabled(bool bInEnabled);

    UFUNCTION(BlueprintCallable, Category="MagField")
    void RebuildVisualization();

    UFUNCTION(BlueprintCallable, Category="MagField")
    void ClearVisualization();

protected:
    virtual void BeginPlay() override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    void EnsureNiagaraComponent();
    void GenerateSamplesAndField(TArray<FVector>& OutPositions, TArray<FVector>& OutDirections, TArray<float>& OutStrengths) const;

    // Dipole field evaluation (arbitrary units)
    void EvaluateFieldAtPoint(const FVector& WorldPoint, FVector& OutDir, float& OutMag) const;

public:
    // --- User-facing parameters ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MagField|System")
    bool bEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MagField|System")
    TObjectPtr<UNiagaraSystem> NiagaraSystemAsset = nullptr;

    // Which local axis is the dipole direction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MagField|Field")
    EMagFieldAxis DipoleAxis = EMagFieldAxis::X;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MagField|Field", meta=(ClampMin="0.0"))
    float Strength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MagField|Field", meta=(ClampMin="0.01"))
    float MinDistanceClamp = 10.0f; // cm, prevents blow-ups

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MagField|Field", meta=(ClampMin="1.0"))
    float MaxRange = 200.0f; // cm

    // Visualization volume around the component
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MagField|Sampling")
    FVector BoundsExtent = FVector(100.0f, 100.0f, 100.0f); // cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MagField|Sampling", meta=(ClampMin="1.0"))
    float Step = 25.0f; // cm grid spacing

    // For mapping magnitude to 0 - 1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MagField|Visualization", meta=(ClampMin="0.0001"))
    float DisplayMaxMagnitude = 1.0f;

private:
    UPROPERTY(Transient)
    TObjectPtr<UNiagaraComponent> NiagaraComp = nullptr;
};