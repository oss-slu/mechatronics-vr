// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "MRUtilityKitBPLibrary.h"

#include "MRUtilityKit.h"
#include "Generated/MRUtilityKitShared.h"
#include "MRUtilityKitAnchor.h"
#include "MRUtilityKitSubsystem.h"
#include "ProceduralMeshComponent.h"
#include "VectorUtil.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "TextureResource.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "OculusXRRoomLayoutManagerComponent.h"

namespace
{
	TArray<FVector> RecalculateNormals(const TArray<FVector>& Vertices, const TArray<uint32>& Triangles)
	{
		TArray<FVector> Normals;

		// Initialize the normals array with zero vectors
		Normals.Init(FVector::ZeroVector, Vertices.Num());

		// Iterate through each triangle
		for (int32 TriangleIndex = 0; TriangleIndex < Triangles.Num(); TriangleIndex += 3)
		{
			// Get the vertices of the triangle
			const FVector VertexA = Vertices[Triangles[TriangleIndex]];
			const FVector VertexB = Vertices[Triangles[TriangleIndex + 1]];
			const FVector VertexC = Vertices[Triangles[TriangleIndex + 2]];

			// Calculate the triangle's normal
			const FVector TriangleNormal = FVector::CrossProduct(VertexC - VertexA, VertexB - VertexA).GetSafeNormal();

			// Add the triangle's normal to each of its vertices' normals
			Normals[Triangles[TriangleIndex]] += TriangleNormal;
			Normals[Triangles[TriangleIndex + 1]] += TriangleNormal;
			Normals[Triangles[TriangleIndex + 2]] += TriangleNormal;
		}

		// Normalize the vertex normals
		for (FVector& Normal : Normals)
		{
			if (!Normal.IsNearlyZero())
			{
				Normal.Normalize();
			}
			else
			{
				Normal = FVector::UpVector;
			}
		}

		return Normals;
	}

	TArray<FProcMeshTangent> RecalculateTangents(const TArray<FVector>& Normals)
	{
		TArray<FProcMeshTangent> Tangents;

		// Initialize the tangents array with zero tangents
		Tangents.Init(FProcMeshTangent(0.f, 0.f, 0.f), Normals.Num());

		// Iterate through each normal
		for (int32 VertexIndex = 0; VertexIndex < Normals.Num(); VertexIndex++)
		{
			const FVector& Normal = Normals[VertexIndex];

			// Calculate a tangent based on the normal
			FVector TangentX = FVector(1.0f, 0.0f, 0.0f);

			// Gram-Schmidt orthogonalization
			TangentX -= Normal * FVector::DotProduct(TangentX, Normal);
			if (!TangentX.IsNearlyZero())
			{
				TangentX.Normalize();
			}
			else
			{
				TangentX = FVector::UpVector;
			}

			// Store the tangent in the array
			Tangents[VertexIndex] = FProcMeshTangent(TangentX, false);
		}

		return Tangents;
	}

	void SetScaleRecursivelyAdjustingForRotationInternal(USceneComponent* SceneComponent, const FVector& UnRotatedScale, const FQuat& AccumulatedRotation, const FVector& ParentReciprocalScale)
	{
		if (SceneComponent)
		{
			const FQuat RelativeRotation = SceneComponent->GetRelativeRotationCache().RotatorToQuat(SceneComponent->GetRelativeRotation());
			const FQuat Rotation = AccumulatedRotation * RelativeRotation;
			const FVector RotatedXAxis = Rotation.GetAxisX();
			const FVector RotatedYAxis = Rotation.GetAxisY();
			const FVector RotatedZAxis = Rotation.GetAxisZ();
			FVector RotatedScale;
			if (FMath::Abs(RotatedXAxis.X) >= UE_INV_SQRT_2)
			{
				RotatedScale.X = UnRotatedScale.X;
			}
			else if (FMath::Abs(RotatedXAxis.Y) >= UE_INV_SQRT_2)
			{
				RotatedScale.X = UnRotatedScale.Y;
			}
			else
			{
				RotatedScale.X = UnRotatedScale.Z;
			}

			if (FMath::Abs(RotatedYAxis.X) >= UE_INV_SQRT_2)
			{
				RotatedScale.Y = UnRotatedScale.X;
			}
			else if (FMath::Abs(RotatedYAxis.Y) >= UE_INV_SQRT_2)
			{
				RotatedScale.Y = UnRotatedScale.Y;
			}
			else
			{
				RotatedScale.Y = UnRotatedScale.Z;
			}

			if (FMath::Abs(RotatedZAxis.X) >= UE_INV_SQRT_2)
			{
				RotatedScale.Z = UnRotatedScale.X;
			}
			else if (FMath::Abs(RotatedZAxis.Y) >= UE_INV_SQRT_2)
			{
				RotatedScale.Z = UnRotatedScale.Y;
			}
			else
			{
				RotatedScale.Z = UnRotatedScale.Z;
			}

			const FVector OldScale = SceneComponent->GetRelativeScale3D();
			const FVector NewScale = ParentReciprocalScale * RotatedScale * OldScale;
			SceneComponent->SetRelativeScale3D(NewScale);
			const FVector NewParentReciprocalScale = ParentReciprocalScale * (OldScale / NewScale);
			for (USceneComponent* Child : SceneComponent->GetAttachChildren())
			{
				if (Child)
				{
					SetScaleRecursivelyAdjustingForRotationInternal(Child, UnRotatedScale, Rotation, NewParentReciprocalScale);
				}
			}
		}
	}
} // namespace

UMRUKLoadFromDevice* UMRUKLoadFromDevice::LoadSceneFromDeviceAsync(const UObject* WorldContext
	,
	EMRUKSceneModel SceneModel)
{
	// We must have a valid contextual world for this action, so we don't even make it
	// unless we can resolve the UWorld from WorldContext.
	if (!ensureAlwaysMsgf(IsValid(WorldContext), TEXT("World Context was not valid.")))
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
	if (!ensureAlwaysMsgf(IsValid(World), TEXT("World was not valid.")))
	{
		return nullptr;
	}

	// Create a new UMyDelayAsyncAction, and store function arguments in it.
	UMRUKLoadFromDevice* NewAction = NewObject<UMRUKLoadFromDevice>();
	NewAction->World = World;
	NewAction->SceneModel = SceneModel;
	NewAction->RegisterWithGameInstance(World->GetGameInstance());
	return NewAction;
}

void UMRUKLoadFromDevice::Activate()
{
	UMRUKSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UMRUKSubsystem>();
	Subsystem->OnSceneLoaded.AddDynamic(this, &UMRUKLoadFromDevice::OnSceneLoaded);

	{
		Subsystem->LoadSceneFromDevice(SceneModel);
	}
}

void UMRUKLoadFromDevice::OnSceneLoaded(bool Succeeded)
{
	if (!World.IsValid())
	{
		UE_LOG(LogMRUK, Warning, TEXT("World is not valid anymore. Can not fire on scene loaded events"));
		return;
	}
	UMRUKSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UMRUKSubsystem>();
	Subsystem->OnSceneLoaded.RemoveDynamic(this, &UMRUKLoadFromDevice::OnSceneLoaded);
	if (Succeeded)
	{
		Success.Broadcast();
	}
	else
	{
		Failure.Broadcast();
	}
	SetReadyToDestroy();
}

UMRUKConfigureTrackables* UMRUKConfigureTrackables::ConfigureTrackablesAsync(const UObject* WorldContext, const FMRUKTrackerConfiguration& Configuration)
{
	if (!ensureAlwaysMsgf(IsValid(WorldContext), TEXT("World Context was not valid.")))
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
	if (!ensureAlwaysMsgf(IsValid(World), TEXT("World was not valid.")))
	{
		return nullptr;
	}

	UMRUKConfigureTrackables* NewAction = NewObject<UMRUKConfigureTrackables>();
	NewAction->World = World;
	NewAction->TrackerConfiguration = Configuration;
	NewAction->RegisterWithGameInstance(World->GetGameInstance());
	return NewAction;
}

void UMRUKConfigureTrackables::OnTrackablesConfigured(bool Succeeded)
{
	if (!World.IsValid())
	{
		UE_LOG(LogMRUK, Warning, TEXT("World is not valid anymore. Can not fire on trackables configured event"));
		return;
	}
	UMRUKSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UMRUKSubsystem>();
	Subsystem->OnTrackablesConfigured.RemoveDynamic(this, &UMRUKConfigureTrackables::OnTrackablesConfigured);
	if (Succeeded)
	{
		Success.Broadcast();
	}
	else
	{
		Failure.Broadcast();
	}
	SetReadyToDestroy();
}

void UMRUKConfigureTrackables::Activate()
{
	UMRUKSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UMRUKSubsystem>();
	Subsystem->OnTrackablesConfigured.AddDynamic(this, &UMRUKConfigureTrackables::OnTrackablesConfigured);
	Subsystem->ConfigureTrackers(TrackerConfiguration);
}

UMRUKLoadFromJson* UMRUKLoadFromJson::LoadSceneFromJsonAsync(const UObject* WorldContext, const FString& JsonString, EMRUKSceneModel SceneModel)
{
	// We must have a valid contextual world for this action, so we don't even make it
	// unless we can resolve the UWorld from WorldContext.

	if (!ensureAlwaysMsgf(IsValid(WorldContext), TEXT("World Context was not valid.")))
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
	if (!ensureAlwaysMsgf(IsValid(World), TEXT("World was not valid.")))
	{
		return nullptr;
	}

	// Create a new UMyDelayAsyncAction, and store function arguments in it.
	UMRUKLoadFromJson* NewAction = NewObject<UMRUKLoadFromJson>();
	NewAction->World = World;
	NewAction->Json = JsonString;
	NewAction->SceneModel = SceneModel;
	NewAction->RegisterWithGameInstance(World->GetGameInstance());
	return NewAction;
}

void UMRUKLoadFromJson::Activate()
{
	UMRUKSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UMRUKSubsystem>();
	Subsystem->OnSceneLoaded.AddDynamic(this, &UMRUKLoadFromJson::OnSceneLoaded);
	Subsystem->LoadSceneFromJsonString(Json, SceneModel);
}

void UMRUKLoadFromJson::OnSceneLoaded(bool Succeeded)
{
	if (!World.IsValid())
	{
		UE_LOG(LogMRUK, Warning, TEXT("World is not valid anymore. Can not fire on scene loaded events"));
		return;
	}
	UMRUKSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UMRUKSubsystem>();
	Subsystem->OnSceneLoaded.RemoveDynamic(this, &UMRUKLoadFromJson::OnSceneLoaded);
	if (Succeeded)
	{
		Success.Broadcast();
	}
	else
	{
		Failure.Broadcast();
	}
	SetReadyToDestroy();
}

bool UMRUKBPLibrary::LoadGlobalMeshFromDevice(FOculusXRUInt64 SpaceHandle, UProceduralMeshComponent* OutProceduralMesh, bool LoadCollision, const UObject* WorldContext)
{
	ensure(OutProceduralMesh);

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
	if (!ensureAlwaysMsgf(IsValid(WorldContext), TEXT("World Context was not valid.")))
	{
		return false;
	}

	UOculusXRRoomLayoutManagerComponent* RoomLayoutManager = World->GetGameInstance()->GetSubsystem<UMRUKSubsystem>()->GetRoomLayoutManager();
	const bool bLoadResult = RoomLayoutManager->LoadTriangleMesh(SpaceHandle.Value, OutProceduralMesh, LoadCollision);
	if (!bLoadResult)
	{
		UE_LOG(LogMRUK, Warning, TEXT("Could not load triangle mesh from layout manager"));
		return false;
	}

	return true;
}

void UMRUKBPLibrary::RecalculateProceduralMeshAndTangents(UProceduralMeshComponent* Mesh)
{
	if (!IsValid(Mesh))
	{
		return;
	}

	for (int32 SectionIndex = 0; SectionIndex < Mesh->GetNumSections(); ++SectionIndex)
	{
		FProcMeshSection* Section = Mesh->GetProcMeshSection(SectionIndex);

		// Get vertices of the section
		TArray<FVector> Vertices;
		for (const FProcMeshVertex& Vertex : Section->ProcVertexBuffer)
		{
			Vertices.Add(Vertex.Position);
		}

		// Calculate normals and tangents
		TArray<FVector> Normals = RecalculateNormals(Vertices, Section->ProcIndexBuffer);
		TArray<FProcMeshTangent> Tangents = RecalculateTangents(Normals);
		TArray<FVector2D> EmptyUV;
		TArray<FColor> EmptyVertexColors;

		// Update mesh section
		Mesh->UpdateMeshSection(SectionIndex, Vertices, Normals, EmptyUV, EmptyVertexColors, Tangents);
	}
}

bool UMRUKBPLibrary::IsUnrealEngineMetaFork()
{
#if defined(WITH_OCULUS_BRANCH)
	return true;
#else
	return false;
#endif
}

FVector2D UMRUKBPLibrary::ComputeCentroid(const TArray<FVector2D>& PolygonPoints)
{
	FVector2D Centroid = FVector2D::ZeroVector;

	if (PolygonPoints.Num() == 0)
	{
		return Centroid;
	}
	else if (PolygonPoints.Num() == 1)
	{
		return PolygonPoints[0];
	}

	double SignedArea = 0.0;
	for (int32 VertexIndex = 0; VertexIndex < PolygonPoints.Num(); ++VertexIndex)
	{
		const double X0 = PolygonPoints[VertexIndex].X;
		const double Y0 = PolygonPoints[VertexIndex].Y;
		const double X1 = PolygonPoints[(VertexIndex + 1) % PolygonPoints.Num()].X;
		const double Y1 = PolygonPoints[(VertexIndex + 1) % PolygonPoints.Num()].Y;

		const double A = X0 * Y1 - X1 * Y0;
		SignedArea += A;

		Centroid.X += (X0 + X1) * A;
		Centroid.Y += (Y0 + Y1) * A;
	}

	return Centroid / (6.0 * (SignedArea * 0.5));
}

void UMRUKBPLibrary::SetScaleRecursivelyAdjustingForRotation(USceneComponent* SceneComponent, const FVector& UnRotatedScale)
{
	SetScaleRecursivelyAdjustingForRotationInternal(SceneComponent, UnRotatedScale, FQuat::Identity, FVector::OneVector);
}

FVector UMRUKBPLibrary::ComputeDirectionAwayFromClosestWall(const AMRUKAnchor* Anchor, int& OutCardinalAxisIndex, const TArray<int> ExcludedAxes)
{
	double ClosestWallDistance = DBL_MAX;
	FVector AwayFromWall{};

	for (int32 CardinalAxisIndex = 0; CardinalAxisIndex < 4; ++CardinalAxisIndex)
	{
		if (ExcludedAxes.Contains(CardinalAxisIndex))
		{
			continue;
		}
		// Shoot a ray along the cardinal directions
		// The "Up" (i.e. Z axis) for anchors typically points away from the facing direction, but it depends
		// entirely on how the user defined the volume in scene capture.
		const FVector CardinalAxis = (FQuat::MakeFromEuler({ 0.0, 0.0, 90.0 * CardinalAxisIndex }).RotateVector(Anchor->GetActorUpVector()));

		for (const auto& WallAnchor : Anchor->Room->WallAnchors)
		{
			if (!WallAnchor)
			{
				continue;
			}
			FMRUKHit Hit{};
			if (!WallAnchor->Raycast(Anchor->GetActorLocation(), CardinalAxis, 0.0, Hit))
			{
				continue;
			}
			const double DistToWall = FVector::Distance(Hit.HitPosition, Anchor->GetActorLocation());
			if (DistToWall < ClosestWallDistance)
			{
				ClosestWallDistance = DistToWall;
				AwayFromWall = -CardinalAxis;
				OutCardinalAxisIndex = CardinalAxisIndex;
			}
		}
	}

	return AwayFromWall;
}

UTexture2D* UMRUKBPLibrary::ConstructTexture2D(UTextureRenderTarget2D* RenderTarget2D, UObject* Outer, const FString& TexName)
{
	const int32 SizeX = RenderTarget2D->SizeX;
	const int32 SizeY = RenderTarget2D->SizeY;
	UTexture2D* Tex = UTexture2D::CreateTransient(SizeX, SizeY, RenderTarget2D->GetFormat());
	Tex->AddToRoot();
	Tex->Filter = TF_Bilinear;
	Tex->CompressionSettings = TC_Default;
	Tex->SRGB = 0;
	Tex->UpdateResource();

	FTextureRenderTargetResource* RenderTargetResource = RenderTarget2D->GameThread_GetRenderTargetResource();
	FReadSurfaceDataFlags ReadSurfaceDataFlags;
	ReadSurfaceDataFlags.SetLinearToGamma(false);

	TArray<FColor> OutBMP;
	RenderTargetResource->ReadPixels(OutBMP, ReadSurfaceDataFlags);

	FTexture2DMipMap& Mip = Tex->GetPlatformData()->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(Data, OutBMP.GetData(), SizeX * SizeY * 4);
	Mip.BulkData.Unlock();

	Tex->UpdateResource();
	return Tex;
}

FLinearColor UMRUKBPLibrary::GetMatrixColumn(const FMatrix& Matrix, int32 Index)
{
	ensure(0 <= Index && Index < 4);
	FLinearColor V;
	V.R = Matrix.M[0][Index];
	V.G = Matrix.M[1][Index];
	V.B = Matrix.M[2][Index];
	V.A = Matrix.M[3][Index];
	return V;
}

void UMRUKBPLibrary::CreateMeshSegmentation(const TArray<FVector>& MeshPositions, const TArray<uint32>& MeshIndices,
	float PointsPerUnitX, float PointsPerUnitY, float PointsPerUnitZ, const FVector& ReservedMin, const FVector& ReservedMax,
	TArray<FMRUKMeshSegment>& OutSegments, FMRUKMeshSegment& OutReservedSegment, float WorldToMeters)
{
	if (!MRUKShared::GetInstance())
	{
		UE_LOG(LogMRUK, Error, TEXT("MRUK shared library is not available. To use this functionality make sure the library is included"));
		return;
	}

	TArray<FVector3f> MeshPositionsF;
	MeshPositionsF.Reserve(MeshPositions.Num());
	for (const FVector& V : MeshPositions)
	{
		MeshPositionsF.Add(FVector3f(V));
	}

	MRUKShared::Mesh3f* MeshSegmentsF = nullptr;
	uint32 MeshSegmentsCount = 0;

	MRUKShared::Mesh3f ReservedMeshSegmentF{};

	const FVector3f ReservedMinF(ReservedMin);
	const FVector3f ReservedMaxF(ReservedMax);

	MRUKShared::GetInstance()->ComputeMeshSegmentation(MeshPositionsF.GetData(), MeshPositionsF.Num(), MeshIndices.GetData(),
		MeshIndices.Num(), PointsPerUnitX / WorldToMeters, PointsPerUnitY / WorldToMeters, PointsPerUnitZ / WorldToMeters, ReservedMinF, ReservedMaxF, &MeshSegmentsF,
		&MeshSegmentsCount, &ReservedMeshSegmentF);

	OutSegments.Reserve(MeshSegmentsCount);
	for (uint32 MeshSegmentIndex = 0; MeshSegmentIndex < MeshSegmentsCount; ++MeshSegmentIndex)
	{
		const MRUKShared::Mesh3f& SegmentF = MeshSegmentsF[MeshSegmentIndex];
		if (SegmentF.numIndices == 0)
		{
			continue;
		}

		FMRUKMeshSegment MeshSegment{};
		MeshSegment.Indices.Reserve(SegmentF.numIndices);
		MeshSegment.Positions.Reserve(SegmentF.numVertices);
		for (uint32 TriangleIndex = 0; TriangleIndex < SegmentF.numIndices; ++TriangleIndex)
		{
			MeshSegment.Indices.Add(SegmentF.indices[TriangleIndex]);
		}
		for (uint32 VertexIndex = 0; VertexIndex < SegmentF.numVertices; ++VertexIndex)
		{
			const FVector3f& V = SegmentF.vertices[VertexIndex];
			MeshSegment.Positions.Add({ V.X, V.Y, V.Z });
		}

		OutSegments.Emplace(MoveTemp(MeshSegment));
	}

	if (ReservedMeshSegmentF.numIndices && ReservedMeshSegmentF.numVertices)
	{
		OutReservedSegment.Indices.Reserve(ReservedMeshSegmentF.numIndices);
		OutReservedSegment.Positions.Reserve(ReservedMeshSegmentF.numVertices);
		for (uint32 TriangleIndex = 0; TriangleIndex < ReservedMeshSegmentF.numIndices; ++TriangleIndex)
		{
			OutReservedSegment.Indices.Add(ReservedMeshSegmentF.indices[TriangleIndex]);
		}
		for (uint32 VertexIndex = 0; VertexIndex < ReservedMeshSegmentF.numVertices; ++VertexIndex)
		{
			const FVector3f& V = ReservedMeshSegmentF.vertices[VertexIndex];
			OutReservedSegment.Positions.Add({ V.X, V.Y, V.Z });
		}
	}

	MRUKShared::GetInstance()->FreeMeshSegmentation(MeshSegmentsF, MeshSegmentsCount, &ReservedMeshSegmentF);
}
