// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "OculusXRScene.h"
#include "OculusXRSceneModule.h"
#include "OculusXRHMDPrivate.h"
#include "OculusXRHMD.h"
#include "OculusXRPluginWrapper.h"
#include "OculusXRSceneFunctionsOVR.h"
#include "OculusXRSceneFunctionsOpenXR.h"

#define LOCTEXT_NAMESPACE "OculusXRScene"

namespace OculusXRScene
{
	EOculusXRAnchorResult::Type FOculusXRScene::GetScenePlane(uint64 AnchorHandle, FVector& OutPos, FVector& OutSize)
	{
		if (const auto& Impl = GetOculusXRSceneFunctionsImpl())
		{
			return Impl->GetScenePlane(AnchorHandle, OutPos, OutSize);
		}
		return EOculusXRAnchorResult::Failure;
	}

	EOculusXRAnchorResult::Type FOculusXRScene::GetSceneVolume(uint64 AnchorHandle, FVector& OutPos, FVector& OutSize)
	{
		if (const auto& Impl = GetOculusXRSceneFunctionsImpl())
		{
			return Impl->GetSceneVolume(AnchorHandle, OutPos, OutSize);
		}
		return EOculusXRAnchorResult::Failure;
	}

	EOculusXRAnchorResult::Type FOculusXRScene::GetSemanticClassification(uint64 AnchorHandle, TArray<FString>& OutSemanticClassifications)
	{
		if (const auto& Impl = GetOculusXRSceneFunctionsImpl())
		{
			return Impl->GetSemanticClassification(AnchorHandle, OutSemanticClassifications);
		}
		return EOculusXRAnchorResult::Failure;
	}

	EOculusXRAnchorResult::Type FOculusXRScene::GetBoundary2D(uint64 AnchorHandle, TArray<FVector2f>& OutVertices)
	{
		if (const auto& Impl = GetOculusXRSceneFunctionsImpl())
		{
			return Impl->GetBoundary2D(AnchorHandle, OutVertices);
		}
		return EOculusXRAnchorResult::Failure;
	}

	EOculusXRAnchorResult::Type FOculusXRScene::RequestSceneCapture(uint64& OutRequestID)
	{
		if (const auto& Impl = GetOculusXRSceneFunctionsImpl())
		{
			return Impl->RequestSceneCapture(OutRequestID);
		}
		return EOculusXRAnchorResult::Failure;
	}

	EOculusXRAnchorResult::Type FOculusXRScene::GetRoomLayout(uint64 AnchorHandle, const uint32 MaxWallsCapacity, FOculusXRUUID& OutCeilingUuid, FOculusXRUUID& OutFloorUuid, TArray<FOculusXRUUID>& OutWallsUuid)
	{
		if (const auto& Impl = GetOculusXRSceneFunctionsImpl())
		{
			return Impl->GetRoomLayout(AnchorHandle, MaxWallsCapacity, OutCeilingUuid, OutFloorUuid, OutWallsUuid);
		}
		return EOculusXRAnchorResult::Failure;
	}

	EOculusXRAnchorResult::Type FOculusXRScene::GetTriangleMesh(uint64 AnchorHandle, TArray<FVector>& Vertices, TArray<int32>& Triangles)
	{
		if (const auto& Impl = GetOculusXRSceneFunctionsImpl())
		{
			return Impl->GetTriangleMesh(AnchorHandle, Vertices, Triangles);
		}
		return EOculusXRAnchorResult::Failure;
	}

	// Requests to change the current boundary visibility
	EOculusXRAnchorResult::Type FOculusXRScene::RequestBoundaryVisibility(EOculusXRBoundaryVisibility NewVisibilityRequest)
	{
		if (const auto& Impl = GetOculusXRSceneFunctionsImpl())
		{
			return Impl->RequestBoundaryVisibility(NewVisibilityRequest);
		}
		return EOculusXRAnchorResult::Failure;
	}

	EOculusXRAnchorResult::Type FOculusXRScene::GetBoundaryVisibility(EOculusXRBoundaryVisibility& OutVisibility)
	{
		if (const auto& Impl = GetOculusXRSceneFunctionsImpl())
		{
			return Impl->GetBoundaryVisibility(OutVisibility);
		}
		return EOculusXRAnchorResult::Failure;
	}


	TSharedPtr<IOculusXRSceneFunctions> FOculusXRScene::SceneFunctionsImpl = nullptr;

	TSharedPtr<IOculusXRSceneFunctions> FOculusXRScene::GetOculusXRSceneFunctionsImpl()
	{
		if (SceneFunctionsImpl == nullptr)
		{
			const FName SystemName(TEXT("OpenXR"));
			const bool IsOpenXR = GEngine->XRSystem && GEngine->XRSystem.IsValid() && (GEngine->XRSystem->GetSystemName() == SystemName);
			if (OculusXRHMD::FOculusXRHMD::GetOculusXRHMD() != nullptr)
			{
				SceneFunctionsImpl = MakeShared<FOculusXRSceneFunctionsOVR>();
			}
			else if (IsOpenXR)
			{
				SceneFunctionsImpl = MakeShared<FOculusXRSceneFunctionsOpenXR>();
			}
		}

		return SceneFunctionsImpl;
	}
} // namespace OculusXRScene

#undef LOCTEXT_NAMESPACE
