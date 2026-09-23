// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "MRUtilityKitSceneDataProvider.h"
#include "MRUtilityKit.h"

void AMRUKSceneDataProvider::GetRoom(FString& RoomJSON, FString& RoomName)
{
	if (!bUseRandomRoom)
	{
		if (!SpecificRoomName.IsEmpty())
		{
			for (const TPair<FString, UDataTable*>& Room : Rooms)
			{
				UDataTable* const RoomDT = Room.Value;
				const FJSONData* JSONData = RoomDT->FindRow<FJSONData>(FName(SpecificRoomName), "", false);
				if (JSONData)
				{
					RoomJSON = JSONData->JSON;
					RoomName = SpecificRoomName;
					return;
				}
			}
			UE_LOG(LogMRUK, Warning, TEXT("Specific room name not found, using random room."));
		}
		else
		{
			UE_LOG(LogMRUK, Warning, TEXT("Specific room name not defined, using random room."));
		}
	}

	if (bUseRandomRoomFromClass)
	{
		if (!SpecificRoomClass.IsEmpty())
		{
			UDataTable* const* RoomDTPtr = Rooms.Find(SpecificRoomClass);
			if (RoomDTPtr)
			{
				UDataTable* const RoomDT = *RoomDTPtr;
				if (RoomDT)
				{
					TArray<FJSONData*> AllRows;
					RoomDT->GetAllRows("", AllRows);
					TArray<FName> RowNames = RoomDT->GetRowNames();
					const int32 NumRows = AllRows.Num() - 1;
					const int32 RowIndex = FMath::RandRange(0, NumRows);

					RoomJSON = AllRows[RowIndex]->JSON;
					RoomName = RowNames[RowIndex].ToString();
					return;
				}
			}

			UE_LOG(LogMRUK, Warning, TEXT("Specific room class not found, using random room."));
		}
		else
		{
			UE_LOG(LogMRUK, Warning, TEXT("Specific room class not defined, using random room."));
		}
	}

	const int32 NumRooms = Rooms.Num() - 1;
	const int32 RoomIndex = FMath::RandRange(0, NumRooms);

	TArray<UDataTable*> ChildArray;
	Rooms.GenerateValueArray(ChildArray);

	UDataTable* const Room = ChildArray[RoomIndex];

	const int32 NumRows = Room->GetRowMap().Num() - 1;
	const int32 RowIndex = FMath::RandRange(0, NumRows);

	TArray<FJSONData*> RandomRoomRows;
	TArray<FName> RandomRoomRowNames = Room->GetRowNames();
	Room->GetAllRows("", RandomRoomRows);

	RoomJSON = RandomRoomRows[RowIndex]->JSON;
	RoomName = RandomRoomRowNames[RowIndex].ToString();
}

void AMRUKSceneDataProvider::BeginPlay()
{
	Super::BeginPlay();
}

void AMRUKSceneDataProvider::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
