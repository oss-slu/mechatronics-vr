// MagneticFieldVisualizerComponent.cpp
#include "MagneticFieldVisualizerComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

UMagneticFieldVisualizerComponent::UMagneticFieldVisualizerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UMagneticFieldVisualizerComponent::BeginPlay()
{
    Super::BeginPlay();
    EnsureNiagaraComponent();

    if (bEnabled)
    {
        RebuildVisualization();
    }
    else
    {
        ClearVisualization();
    }
}

#if WITH_EDITOR
void UMagneticFieldVisualizerComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // In-editor updates when you tweak values
    if (!IsRunningGame())
    {
        EnsureNiagaraComponent();
        if (bEnabled) RebuildVisualization();
        else ClearVisualization();
    }
}
#endif

void UMagneticFieldVisualizerComponent::EnsureNiagaraComponent()
{
    if (NiagaraComp) return;

    // Create and attach
    NiagaraComp = NewObject<UNiagaraComponent>(GetOwner(), UNiagaraComponent::StaticClass(), TEXT("MagFieldNiagaraComp"));
    NiagaraComp->SetupAttachment(this);
    NiagaraComp->RegisterComponent();

    NiagaraComp->SetAutoActivate(false);
    NiagaraComp->SetVisibility(false, true);

    if (NiagaraSystemAsset)
    {
        NiagaraComp->SetAsset(NiagaraSystemAsset);
    }
}

void UMagneticFieldVisualizerComponent::SetEnabled(bool bInEnabled)
{
    bEnabled = bInEnabled;
    if (bEnabled) RebuildVisualization();
    else ClearVisualization();
}

void UMagneticFieldVisualizerComponent::ClearVisualization()
{
    EnsureNiagaraComponent();
    if (!NiagaraComp) return;

    NiagaraComp->DeactivateImmediate();
    NiagaraComp->SetVisibility(false, true);

    // clear arrays
    TArray<FVector> EmptyVec;
    TArray<float> EmptyFloat;
    UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComp, TEXT("User.Positions"), EmptyVec);
    UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComp, TEXT("User.Directions"), EmptyVec);
    UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(NiagaraComp, TEXT("User.Strengths"), EmptyFloat);
}

void UMagneticFieldVisualizerComponent::RebuildVisualization()
{
    EnsureNiagaraComponent();
    if (!NiagaraComp || !NiagaraSystemAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("No Connected Niagara Component"));
        return;
    }
    
    NiagaraComp->SetAsset(NiagaraSystemAsset);

    TArray<FVector> Positions, Directions;
    TArray<float> Strengths;
    GenerateSamplesAndField(Positions, Directions, Strengths);

    // Push arrays to Niagara user params
    UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComp, TEXT("User.Positions"), Positions);
    UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComp, TEXT("User.Directions"), Directions);
    UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(NiagaraComp, TEXT("User.Strengths"), Strengths);

    // Pass scalar params
    NiagaraComp->SetVariableFloat(TEXT("User.DisplayMaxMagnitude"), DisplayMaxMagnitude);

    NiagaraComp->SetVisibility(true, true);
    NiagaraComp->Activate(true);
}

void UMagneticFieldVisualizerComponent::GenerateSamplesAndField(TArray<FVector>& OutPositions, TArray<FVector>& OutDirections, TArray<float>& OutStrengths) const
{
    OutPositions.Reset();
    OutDirections.Reset();
    OutStrengths.Reset();

    // Component-aligned box sampling in local space
    const FVector Ext = BoundsExtent;
    const float S = FMath::Max(1.0f, Step);

    const int32 Nx = FMath::FloorToInt((2.0f * Ext.X) / S) + 1;
    const int32 Ny = FMath::FloorToInt((2.0f * Ext.Y) / S) + 1;
    const int32 Nz = FMath::FloorToInt((2.0f * Ext.Z) / S) + 1;

    OutPositions.Reserve(Nx * Ny * Nz);
    OutDirections.Reserve(Nx * Ny * Nz);
    OutStrengths.Reserve(Nx * Ny * Nz);

    const FTransform T = GetComponentTransform();
    const FVector Origin = T.GetLocation();

    for (int32 ix = 0; ix < Nx; ++ix)
    {
        const float lx = -Ext.X + ix * S;
        for (int32 iy = 0; iy < Ny; ++iy)
        {
            const float ly = -Ext.Y + iy * S;
            for (int32 iz = 0; iz < Nz; ++iz)
            {
                const float lz = -Ext.Z + iz * S;

                const FVector LocalPoint(lx, ly, lz);
                const FVector WorldPoint = T.TransformPosition(LocalPoint);

                const float Dist = FVector::Distance(WorldPoint, Origin);
                if (Dist > MaxRange) continue;

                FVector Dir;
                float Mag;
                EvaluateFieldAtPoint(WorldPoint, Dir, Mag);

                // Normalize magnitude to 0 - 1 for visualization
                const float NormMag = FMath::Clamp(Mag / DisplayMaxMagnitude, 0.0f, 1.0f);

                OutPositions.Add(WorldPoint);
                OutDirections.Add(Dir);
                OutStrengths.Add(NormMag);
            }
        }
    }
}

void UMagneticFieldVisualizerComponent::EvaluateFieldAtPoint(const FVector& WorldPoint, FVector& OutDir, float& OutMag) const
{
    const FTransform T = GetComponentTransform();
    const FVector Origin = T.GetLocation();

    // Choose dipole direction in world space
    FVector M;
    switch (DipoleAxis)
    {
        case EMagFieldAxis::X: M = T.GetUnitAxis(EAxis::X); break;
        case EMagFieldAxis::Y: M = T.GetUnitAxis(EAxis::Y); break;
        case EMagFieldAxis::Z: M = T.GetUnitAxis(EAxis::Z); break;
        default: M = T.GetUnitAxis(EAxis::X); break;
    }

    FVector R = WorldPoint - Origin;
    float d = R.Size();
    d = FMath::Max(d, MinDistanceClamp);

    const FVector Rhat = R / d;

    // Dipole-like field (directional shape), scaled for visuals        (formula from GPT)
    // B ~ (3(m·r) r - m) / d^3
    const float mdotr = FVector::DotProduct(M, Rhat);
    FVector B = (3.0f * mdotr) * Rhat - M;

    // Apply falloff and overall strength
    const float Falloff = 1.0f / (d * d * d);
    B *= (Strength * Falloff);

    OutMag = B.Size();

    if (OutMag > KINDA_SMALL_NUMBER)
    {
        OutDir = B / OutMag;
    }
    else
    {
        OutDir = FVector::ForwardVector;
        OutMag = 0.0f;
    }
}