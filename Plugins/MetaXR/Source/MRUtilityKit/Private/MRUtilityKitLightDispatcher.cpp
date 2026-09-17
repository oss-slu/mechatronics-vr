// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "MRUtilityKitLightDispatcher.h"
#include "MRUtilityKit.h"
#include "MRUtilityKitTelemetry.h"
#include "Components/PointLightComponent.h"
#include "Engine/PointLight.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "UObject/ConstructorHelpers.h"

AMRUKLightDispatcher::AMRUKLightDispatcher()
{
	PrimaryActorTick.bCanEverTick = true;

	const ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> MpcAsset(TEXT("/OculusXR/Materials/MPC_Highlights"));
	if (MpcAsset.Succeeded())
	{
		Collection = MpcAsset.Object;
	}
	else
	{
		UE_LOG(LogMRUK, Log, TEXT("Light dispatcher couldn't find material parameter collection in /OculusXR/Materials/MPC_Highlights"));
	}
}

void AMRUKLightDispatcher::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	FillParameterCollection();
}

void AMRUKLightDispatcher::FillParameterCollection()
{
	if (!Collection || PointLightComponents.IsEmpty())
	{
		return;
	}

	if (!GetWorld())
	{
		return;
	}

	UMaterialParameterCollectionInstance* Instance = GetWorld()->GetParameterCollectionInstance(Collection);
	if (!Instance)
	{
		return;
	}

	for (int32 LightIndex = 0; LightIndex < PointLightComponents.Num(); LightIndex++)
	{
		const UPointLightComponent* Light = PointLightComponents[LightIndex];
		if (!IsValid(Light))
		{
			continue;
		}

		const int32 ParameterStep = LightIndex * 3;

		// It's not possible to expand the amount of parameters in collection at runtime,
		// in case we exceed the count of existing parameters break the loop
		if (Collection->VectorParameters.Num() < ParameterStep + 3)
		{
			break;
		}

		// Prepare parameters
		FCollectionVectorParameter PositionParam;
		FCollectionVectorParameter DataParam;
		FCollectionVectorParameter ColorParam;

		PositionParam.ParameterName = FName(FString::Printf(TEXT("PointLightPosition%d"), LightIndex));
		DataParam.ParameterName = FName(FString::Printf(TEXT("PointLightData%d"), LightIndex));
		ColorParam.ParameterName = FName(FString::Printf(TEXT("PointLightColor%d"), LightIndex));

		PositionParam.DefaultValue = FLinearColor(Light->GetComponentLocation());
		DataParam.DefaultValue = FLinearColor(
			1.0f / Light->AttenuationRadius,
			Light->ComputeLightBrightness(),
			Light->LightFalloffExponent,
			Light->bUseInverseSquaredFalloff);
		ColorParam.DefaultValue = Light->GetLightColor();

		// Fill collection's vector parameters
		Collection->VectorParameters[ParameterStep] = PositionParam;
		Collection->VectorParameters[ParameterStep + 1] = DataParam;
		Collection->VectorParameters[ParameterStep + 2] = ColorParam;
	}

	// Send count of lights
	Collection->ScalarParameters[0].DefaultValue = PointLightComponents.Num();
	UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), Collection, TEXT("TotalLights"), PointLightComponents.Num());

	// Update instance
	Instance->UpdateRenderState(false);
}

void AMRUKLightDispatcher::AddAdditionalPointLightActor(AActor* Actor)
{
	AdditionalActorsToLookForPointLightComponents.AddUnique(Actor);
	AddPointLightsFromActor(Actor);
}

void AMRUKLightDispatcher::ForceUpdateCollection()
{
	FillPointLights();
	FillParameterCollection();
	PointLightComponents.Empty();
}

void AMRUKLightDispatcher::BeginPlay()
{
	Super::BeginPlay();
	OculusXRTelemetry::TScopedMarker<MRUKTelemetry::FLoadLightDispatcherMarker> Event(static_cast<int>(GetTypeHash(this)));
	FillPointLights();
}

void AMRUKLightDispatcher::FillPointLights()
{
	// Make sure we don't have duplicates in the array
	PointLightComponents.Empty();

	if (ShouldFetchPointLightsAtBeginPlay)
	{
		// Fetch all point light actors from the level

		TArray<AActor*> PointLightActors;
		UGameplayStatics::GetAllActorsOfClass(this, APointLight::StaticClass(), PointLightActors);

		for (AActor* Actor : PointLightActors)
		{
			const APointLight* PointLightActor = Cast<APointLight>(Actor);
			if (!PointLightActor)
			{
				continue;
			}

			PointLightComponents.Add(PointLightActor->PointLightComponent);
		}
	}
	else
	{
		// Only use the point lights that have been specified in ManualPointLights

		for (AActor* Actor : ManualPointLights)
		{
			if (!IsValid(Actor))
			{
				continue;
			}

			const APointLight* PointLightActor = Cast<APointLight>(Actor);
			if (!PointLightActor)
			{
				continue;
			}

			PointLightComponents.Add(PointLightActor->PointLightComponent);
		}
	}

	// Check the additional added actors for point lights and add them in case they have
	// PointLightComponents attached
	for (AActor* Actor : AdditionalActorsToLookForPointLightComponents)
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		AddPointLightsFromActor(Actor);
	}
}

void AMRUKLightDispatcher::AddPointLightsFromActor(const AActor* Actor)
{
	TArray<UPointLightComponent*> LightComponents;
	Actor->GetComponents(LightComponents, false);
	PointLightComponents.Append(LightComponents);
}
