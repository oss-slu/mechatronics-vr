// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "MRUtilityKitPositionGenerator.h"
#include "MRUtilityKitSubsystem.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "CollisionShape.h"

bool AMRUtilityKitPositionGenerator::CanSpawnBox(const UWorld* World, const FBox& Box, const FVector& SpawnPosition, const FQuat& SpawnRotation, const FCollisionQueryParams& QueryParams, const ECollisionChannel CollisionChannel)
{
	TArray<FOverlapResult> OutOverlaps;
	const bool bHasOverlap = World->OverlapMultiByChannel(OutOverlaps, SpawnPosition, SpawnRotation, CollisionChannel, FCollisionShape::MakeBox(Box.GetExtent()), QueryParams);
	return !bHasOverlap;
}

void AMRUtilityKitPositionGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (RunOnStart)
	{
		UMRUKSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UMRUKSubsystem>();
		if (Subsystem)
		{
			if (Subsystem->SceneLoadStatus == EMRUKInitStatus::Complete)
			{
				SceneLoaded(true);
			}
			Subsystem->OnSceneLoaded.AddUniqueDynamic(this, &AMRUtilityKitPositionGenerator::SceneLoaded);
		}
	}
}

bool AMRUtilityKitPositionGenerator::GenerateRandomPositionsOnSurface(TArray<FTransform>& OutTransforms)
{
	UMRUKSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UMRUKSubsystem>();
	bool bSuccess = true;
	switch (RandomSpawnSettings.RoomFilter)
	{
		case EMRUKRoomFilter::None:
			break;
		case EMRUKRoomFilter::CurrentRoomOnly:
		{
			AMRUKRoom* Room = Subsystem->GetCurrentRoom();
			if (!Room)
			{
				UE_LOG(LogMRUK, Warning, TEXT("No current room available. Cannot generate random positions."));
				bSuccess = false;
			}
			else
			{
				bSuccess = GenerateRandomPositionsOnSurfaceInRoom(Room, OutTransforms);
			}
			break;
		}
		case EMRUKRoomFilter::AllRooms:
		{
			for (AMRUKRoom* Room : Subsystem->Rooms)
			{
				if (!Room)
				{
					continue;
				}
				if (!GenerateRandomPositionsOnSurfaceInRoom(Room, OutTransforms))
				{
					bSuccess = false;
				}
			}
			break;
		}
		default:
			break;
	}
	return bSuccess;
}

bool AMRUtilityKitPositionGenerator::GenerateRandomPositionsOnSurfaceInRoom(AMRUKRoom* Room, TArray<FTransform>& OutTransforms)
{
	if (!Room)
	{
		UE_LOG(LogMRUK, Warning, TEXT("Room is null. Cannot generate random positions."));
		return false;
	}

	bool bInitializedAnchor = IsValid(RandomSpawnSettings.ActorInstance);
	if (bInitializedAnchor && RandomSpawnSettings.ActorClass)
	{
		UE_LOG(LogMRUK, Error, TEXT("Cannot use an initialized Actor AND a defined ActorClass together. Use one of the options"));
		return false;
	}
	if (!bInitializedAnchor && !RandomSpawnSettings.ActorClass)
	{
		UE_LOG(LogMRUK, Error, TEXT("Please define ActorClass."));
		return false;
	}

	UMRUKSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UMRUKSubsystem>();
	const FBox Bounds = bInitializedAnchor ? RandomSpawnSettings.ActorInstance->CalculateComponentsBoundingBoxInLocalSpace() : Subsystem->GetActorClassBounds(RandomSpawnSettings.ActorClass);

	float MinRadius = 0.0f;
	float CenterOffset = Bounds.GetCenter().Z;
	float BaseOffset = -Bounds.Min.Z;

	FBox AdjustedBounds;

	if (Bounds.IsValid)
	{
		constexpr float ClearanceDistance = 0.01f;
		CenterOffset = Bounds.GetCenter().Z;

		MinRadius = FMath::Min(FMath::Min(-Bounds.Min.X, -Bounds.Min.Y), FMath::Min(Bounds.Max.X, Bounds.Max.Y));
		if (MinRadius < 0.0f)
		{
			MinRadius = 0.0f;
		}

		FVector Min = Bounds.Min;
		FVector Max = Bounds.Max;
		Min.Z += ClearanceDistance;

		if (Max.Z < Min.Z)
		{
			Max.Z = Min.Z;
		}

		AdjustedBounds = FBox(Min, Max);
		if (RandomSpawnSettings.OverrideBounds > 0)
		{
			FVector Center = FVector(0.0f, 0.0f, ClearanceDistance);
			FVector Extents = FVector((RandomSpawnSettings.OverrideBounds), (RandomSpawnSettings.OverrideBounds), ClearanceDistance);
			AdjustedBounds = FBox(Center - Extents, Center + Extents);
		}
	}

	int32 FoundPositions = 0;

	for (int32 i = 0; i < RandomSpawnSettings.SpawnAmount; ++i)
	{
		for (int32 j = 0; j < RandomSpawnSettings.MaxIterations; ++j)
		{
			FVector SpawnPosition = FVector::ZeroVector;
			FVector SpawnNormal = FVector::ZeroVector;
			bool bFoundSpawnPos = false;
			if (RandomSpawnSettings.SpawnLocations == EMRUKSpawnLocation::Floating)
			{
				FVector OutPos;
				const bool bFoundRandomPos = Room->GenerateRandomPositionInRoom(OutPos, MinRadius, true);
				if (!bFoundRandomPos)
				{
					break;
				}
				SpawnPosition = OutPos;
				bFoundSpawnPos = true;
			}
			else
			{
				FVector Normal;
				FVector Pos;
				if (Room->GenerateRandomPositionOnSurface(RandomSpawnSettings.SpawnLocations, MinRadius, RandomSpawnSettings.Labels, Pos, Normal))
				{
					SpawnPosition = Pos + Normal * BaseOffset;
					SpawnNormal = Normal;
					const FVector Center = SpawnPosition + Normal * CenterOffset;
					const bool bInRoom = Room->IsPositionInRoom(Center);
					if (!bInRoom)
					{
						continue;
					}
					if (Room->IsPositionInSceneVolume(Center))
					{
						continue;
					}
					FMRUKHit Hit;
					if (Room->Raycast(SpawnPosition, Normal, RandomSpawnSettings.SurfaceClearanceDistance, RandomSpawnSettings.Labels, Hit))
					{
						continue;
					}
					bFoundSpawnPos = true;
				}
			}
			FQuat SpawnRotation = FQuat::Identity;
			if (!SpawnNormal.IsNearlyZero())
			{
				SpawnNormal.Normalize();
				SpawnRotation = FQuat::FindBetweenNormals(FVector::UpVector, SpawnNormal);
			}

			if (RandomSpawnSettings.CheckOverlaps && Bounds.IsValid && bFoundSpawnPos)
			{
				const FBox WorldBounds(AdjustedBounds.Min + SpawnPosition - AdjustedBounds.GetCenter(), AdjustedBounds.Max + SpawnPosition - AdjustedBounds.GetCenter());

				const FVector AdjustedSpawnPos = SpawnPosition + SpawnRotation * AdjustedBounds.GetCenter();

				// Check against world
				const FCollisionQueryParams& QueryParams = FCollisionQueryParams::DefaultQueryParam;
				if (!CanSpawnBox(GetTickableGameObjectWorld(), WorldBounds, AdjustedSpawnPos, SpawnRotation, QueryParams, RandomSpawnSettings.CollisionChannel))
				{
					continue;
				}
			}
			if (bInitializedAnchor && bFoundSpawnPos)
			{
				RandomSpawnSettings.ActorInstance->SetActorLocationAndRotation(SpawnPosition, SpawnRotation);

				// Ignore SpawnAmount once we have a successful move of existing object in the scene
				return true;
			}

			if (bFoundSpawnPos)
			{
				OutTransforms.Add(FTransform(SpawnRotation, SpawnPosition, FVector::OneVector));
				FoundPositions++;
				break;
			}
		}
	}
	return FoundPositions == RandomSpawnSettings.SpawnAmount;
}

void AMRUtilityKitPositionGenerator::SceneLoaded(bool Success)
{
	if (Success)
	{
		TArray<FTransform> OutTransforms;
		const bool bSuccess = GenerateRandomPositionsOnSurface(OutTransforms);

		if (!bSuccess)
		{
			UE_LOG(LogMRUK, Warning, TEXT("Generate Random Positions on Surface not successful"));
			return;
		}

		if (RandomSpawnSettings.ActorClass)
		{
			for (const FTransform& Transform : OutTransforms)
			{
				FActorSpawnParameters Params;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				GetWorld()->SpawnActor(RandomSpawnSettings.ActorClass, &Transform, Params);
			}
		}
	}
}
