#include "MagneticFieldComponent.h"

#include "Engine/CollisionProfile.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectIterator.h"

namespace
{
	constexpr int32 MaxFieldArrows = 4000;
}

UMagneticFieldComponent::UMagneticFieldComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetMobility(EComponentMobility::Movable);
	SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	SetGenerateOverlapEvents(false);
	SetCanEverAffectNavigation(false);
	SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ArrowMeshFinder(TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (ArrowMeshFinder.Succeeded())
	{
		SetStaticMesh(ArrowMeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ArrowMaterialFinder(TEXT("/Game/Materials/M_MagneticFieldArrow.M_MagneticFieldArrow"));
	if (ArrowMaterialFinder.Succeeded())
	{
		SetMaterial(0, ArrowMaterialFinder.Object);
	}
}

void UMagneticFieldComponent::OnRegister()
{
	Super::OnRegister();
	RebuildField();
}

#if WITH_EDITOR
void UMagneticFieldComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RebuildField();
}
#endif

void UMagneticFieldComponent::SetFieldEnabled(bool bEnabled)
{
	bFieldEnabled = bEnabled;
	RebuildField();
}

void UMagneticFieldComponent::SetFieldStrength(float NewStrength)
{
	FieldStrength = FMath::Max(0.0f, NewStrength);
	RebuildField();
}

void UMagneticFieldComponent::SetAllMagneticFieldsEnabled(const UObject* WorldContextObject, bool bEnabled)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World)
	{
		return;
	}

	for (TObjectIterator<UMagneticFieldComponent> It; It; ++It)
	{
		if (IsValid(*It) && It->GetWorld() == World)
		{
			It->SetFieldEnabled(bEnabled);
		}
	}
}

FVector UMagneticFieldComponent::FieldAt(const FVector& LocalPoint) const
{
	const double Half = PoleSeparation * 0.5;
	const FVector ToNorth = LocalPoint - FVector(Half, 0.0, 0.0);
	const FVector ToSouth = LocalPoint + FVector(Half, 0.0, 0.0);
	const double NorthDistSq = FMath::Max(ToNorth.SizeSquared(), UE_KINDA_SMALL_NUMBER);
	const double SouthDistSq = FMath::Max(ToSouth.SizeSquared(), UE_KINDA_SMALL_NUMBER);
	return FieldStrength * Half * Half *
		(ToNorth / (NorthDistSq * FMath::Sqrt(NorthDistSq)) - ToSouth / (SouthDistSq * FMath::Sqrt(SouthDistSq)));
}

void UMagneticFieldComponent::RebuildField()
{
	ClearInstances();
	SetNumCustomDataFloats(3);

	UStaticMesh* ArrowMesh = GetStaticMesh();
	if (!bFieldEnabled || FieldStrength <= 0.0f || !ArrowMesh)
	{
		MarkRenderStateDirty();
		return;
	}

	const FBox MeshBounds = ArrowMesh->GetBoundingBox();
	const FVector MeshSize = MeshBounds.GetSize().ComponentMax(FVector(UE_KINDA_SMALL_NUMBER));
	const double LogRange = FMath::Loge(FMath::Max(FullColorStrength, MinVisibleStrength) / MinVisibleStrength);

	const double Half = PoleSeparation * 0.5;
	const FVector North(Half, 0.0, 0.0);
	const FVector South = -North;
	const double PoleRadius = FMath::Max(Half * 0.15, 0.1);
	const double Step = FMath::Max(FMath::Min(ArrowSpacing * 0.2, PoleRadius * 0.5), 0.02);
	const int32 MaxSteps = FMath::Min(FMath::CeilToInt(MaxFieldRadius * 4.0 / Step), 20000);

	TArray<FTransform> Transforms;
	TArray<float> CustomData;

	auto AddArrow = [&](const FVector& Point, const FVector& Direction, double Magnitude)
	{
		const double Level = LogRange > UE_KINDA_SMALL_NUMBER
			? FMath::Clamp(FMath::Loge(Magnitude / MinVisibleStrength) / LogRange, 0.0, 1.0)
			: 1.0;
		const double Length = ArrowSize * (0.6 + 0.6 * Level);
		const FVector InstanceScale = FVector(Length * 0.45, Length * 0.45, Length) / MeshSize;
		const FQuat Rotation = FRotationMatrix::MakeFromZ(Direction).ToQuat();
		const FVector Location = Point - Rotation.RotateVector(MeshBounds.GetCenter() * InstanceScale);
		Transforms.Add(FTransform(Rotation, Location, InstanceScale));

		const FLinearColor Color = FLinearColor(240.0f * (1.0f - static_cast<float>(Level)), 1.0f, 1.0f).HSVToLinearRGB();
		CustomData.Add(Color.R);
		CustomData.Add(Color.G);
		CustomData.Add(Color.B);
	};

	auto Trace = [&](FVector Point, double Sign, const FVector& Target) -> bool
	{
		double Travelled = 0.0;
		double NextArrow = ArrowSpacing * 0.5;
		for (int32 StepIndex = 0; StepIndex < MaxSteps && Transforms.Num() < MaxFieldArrows; ++StepIndex)
		{
			if (FVector::Dist(Point, Target) < PoleRadius)
			{
				return true;
			}

			const FVector Field = FieldAt(Point);
			const double Magnitude = Field.Size();
			if (Magnitude < MinVisibleStrength || Point.Size() > MaxFieldRadius)
			{
				return false;
			}

			if (Travelled >= NextArrow)
			{
				AddArrow(Point, Field / Magnitude, Magnitude);
				NextArrow += ArrowSpacing;
			}

			const FVector MidPoint = Point + Field / Magnitude * (Sign * Step * 0.5);
			const FVector MidDirection = FieldAt(MidPoint).GetSafeNormal();
			if (MidDirection.IsNearlyZero())
			{
				return false;
			}

			Point += MidDirection * (Sign * Step);
			Travelled += Step;
		}
		return false;
	};

	const int32 Planes = FMath::Max(1, FieldLinePlanes);
	const int32 Lines = FMath::Max(2, LinesPerPlane);
	for (int32 Plane = 0; Plane < Planes; ++Plane)
	{
		const double Phi = UE_DOUBLE_PI * Plane / Planes;
		const FVector Radial(0.0, FMath::Cos(Phi), FMath::Sin(Phi));
		for (int32 Line = 0; Line < Lines; ++Line)
		{
			const double Theta = UE_DOUBLE_TWO_PI * (Line + 0.5) / Lines;
			const FVector Direction = FVector::ForwardVector * FMath::Cos(Theta) + Radial * FMath::Sin(Theta);
			if (!Trace(North + Direction * PoleRadius, 1.0, South))
			{
				const FVector Mirrored(-Direction.X, Direction.Y, Direction.Z);
				Trace(South + Mirrored * PoleRadius, -1.0, North);
			}
		}
	}

	if (Transforms.Num() > 0)
	{
		AddInstances(Transforms, false, false, false);
		SetCustomData(0, Transforms.Num() - 1, CustomData, false);
	}
	MarkRenderStateDirty();
}
