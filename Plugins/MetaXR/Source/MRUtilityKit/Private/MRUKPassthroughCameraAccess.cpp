// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "MRUKPassthroughCameraAccess.h"

#include "ExternalTexture.h"
#include "Engine/GameInstance.h"
#include "MRUtilityKitSharedHelper.h"
#include "MRUtilityKitTelemetry.h"
#include "OculusXRTelemetryMarker.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#if PLATFORM_ANDROID
#include "IVulkanDynamicRHI.h"
#include "AndroidPermissionFunctionLibrary.h"
#endif

namespace
{
#if PLATFORM_ANDROID
	void* GetVulkanInstanceProcAddr(uint64 Instance, const char* Name)
	{
#if !UE_VERSION_OLDER_THAN(5, 6, 0)
		if (Instance == 0)
		{
			return GetIVulkanDynamicRHI()->RHIGetVkInstanceGlobalProcAddr(Name);
		}
		else
#endif
		{
			return GetIVulkanDynamicRHI()->RHIGetVkInstanceProcAddr(Name);
		}
	}
#endif

	uint32 SizeForPixelFormat(EPixelFormat PixelFormat)
	{
		switch (PixelFormat)
		{
			case PF_R8G8B8A8:
			case PF_R8G8B8A8_SNORM:
				return sizeof(uint8) * 4;

			default:
				checkNoEntry();
		}
		checkNoEntry();
		return 0;
	}

	FDateTime DateTimeFromUnixMicroseconds(int64 TimestampMicroseconds)
	{
		const int64 Seconds = TimestampMicroseconds / 1'000'000;
		const int64 Microseconds = TimestampMicroseconds % 1'000'000;
		const FDateTime DateTime = FDateTime::FromUnixTimestamp(Seconds);
		const FTimespan ExtraTime = FTimespan::FromMicroseconds(Microseconds);
		return DateTime + ExtraTime;
	}
} // namespace

void UMRUKPassthroughCameraAccessSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bInitialized = true;

	// Bind to the OnWorldPreActorTick delegate
	PreActorTickHandle = FWorldDelegates::OnWorldPreActorTick.AddUObject(this, &UMRUKPassthroughCameraAccessSubsystem::OnWorldPreActorTick);
}

void UMRUKPassthroughCameraAccessSubsystem::Deinitialize()
{
	bInitialized = false;

	// Remove the delegate binding
	if (PreActorTickHandle.IsValid())
	{
		FWorldDelegates::OnWorldPreActorTick.Remove(PreActorTickHandle);
		PreActorTickHandle.Reset();
	}

	if (bCameraInitialized)
	{
		MRUKShared::GetInstance()->CameraDeinitialize();
		bCameraInitialized = false;
	}
	Super::Deinitialize();
}

TArray<FVector2D> UMRUKPassthroughCameraAccessSubsystem::GetSupportedResolutions(EMRUKCameraEye CameraEye)
{
	const int32 EyeIndex = static_cast<int32>(CameraEye);
	int32 Length = 0;
	MRUKShared::Vector2i* Buffer = MRUKShared::GetInstance()->CameraGetSupportedResolutions(EyeIndex, &Length);

	TArray<FVector2D> Resolutions;
	Resolutions.SetNum(Length);
	for (int32 i = 0; i < Length; ++i)
	{
		Resolutions[i].X = Buffer[i].x;
		Resolutions[i].Y = Buffer[i].y;
	}

	MRUKShared::GetInstance()->CameraFreeSupportedResolutions(Buffer);

	return Resolutions;
}

bool UMRUKPassthroughCameraAccessSubsystem::Play(int& Width, int& Height, int MaxFramerate, EMRUKCameraEye CameraEye)
{
	const int32 CameraEyeIndex = static_cast<int32>(CameraEye);

	UE_LOG(LogMRUK, Log, TEXT("Play camera eye: %d"), CameraEyeIndex);

	if (CameraEyeIndex < 0 || CameraEyeIndex >= Textures.Num())
	{
		UE_LOG(LogMRUK, Error, TEXT("Failed to play. Invalid camera eye: %d"), CameraEyeIndex);
		return false;
	}

#if PLATFORM_ANDROID
	static const FString HEADSET_CAMERA_PERMISSION("horizonos.permission.HEADSET_CAMERA");
	if (!UAndroidPermissionFunctionLibrary::CheckPermission(HEADSET_CAMERA_PERMISSION))
	{
		UE_LOG(LogMRUK, Error, TEXT("For Passthrough Camera Access to work %s permission needs to be granted."), *HEADSET_CAMERA_PERMISSION);
		return false;
	}
#endif // PLATFORM_ANDROID

#if PLATFORM_ANDROID
	IVulkanDynamicRHI* VulkanRHI = GetIVulkanDynamicRHI();
	if (!VulkanRHI)
	{
		ensureMsgf(VulkanRHI, TEXT("Passthrough Camera Access is only supported with the Vulkan RHI"));
		return false;
	}
#endif

	MRUKShared::CameraIntrinsics CameraIntrinsicsData{};
	if (!MRUKShared::GetInstance()->CameraPlay(static_cast<int>(CameraEye), &Width, &Height, &CameraIntrinsicsData, MaxFramerate))
	{
		if (Width == -1 || Height == -1)
		{
			UE_LOG(LogMRUK, Error, TEXT("Requested resolution %dx%d is too small."), Width, Height);
		}
		else
		{
			UE_LOG(LogMRUK, Error, TEXT("Failed to play camera eye index %d."), CameraEye);
		}
		return false;
	}

#if PLATFORM_ANDROID
	if (!bCameraInitialized)
	{
		bCameraInitialized = true;
		VkInstance Instance = VulkanRHI->RHIGetVkInstance();
		VkPhysicalDevice PhysicalDevice = VulkanRHI->RHIGetVkPhysicalDevice();
		VkDevice Device = VulkanRHI->RHIGetVkDevice();
		VkQueue Queue = VulkanRHI->RHIGetGraphicsVkQueue();
		uint32 QueueFamilyIndex = VulkanRHI->RHIGetGraphicsQueueFamilyIndex();
		void* GetInstanceProcAddrFunc = reinterpret_cast<void*>(GetVulkanInstanceProcAddr);

		ENQUEUE_RENDER_COMMAND(InitializePcaCamera)
		([this, Instance, PhysicalDevice, Device, Queue, QueueFamilyIndex, GetInstanceProcAddrFunc, CameraEye](FRHICommandListImmediate& RHICmdList) {
			MRUKShared::GetInstance()->CameraInitializeVulkan(reinterpret_cast<uint64_t>(Instance),
				reinterpret_cast<uint64_t>(PhysicalDevice), reinterpret_cast<uint64_t>(Device),
				reinterpret_cast<uint64_t>(Queue), QueueFamilyIndex, GetInstanceProcAddrFunc);
		});
	}
	ENQUEUE_RENDER_COMMAND(InitializePcaCamera)
	([this, CameraEyeIndex](FRHICommandListImmediate& RHICmdList) {
		UTexture* Texture = Textures[CameraEyeIndex];
		void* NativeTextureHandle = Texture->GetResource()->GetTexture2DRHI()->GetNativeResource();
		MRUKShared::GetInstance()->CameraSetNativeTexture(CameraEyeIndex, &NativeTextureHandle);
	});
#endif

	UE_LOG(LogMRUK, Log, TEXT("Playing camera eye index %d with resolution %dx%d."), CameraEye, Width, Height);

	const double WorldToMeters = GetWorld() ? GetWorld()->GetWorldSettings()->WorldToMeters : 100.0;

	CameraIntrinsics[CameraEyeIndex].FocalLength = FVector2D(CameraIntrinsicsData.focalLength);
	CameraIntrinsics[CameraEyeIndex].PrincipalPoint = FVector2D(CameraIntrinsicsData.principalPoint);
	CameraIntrinsics[CameraEyeIndex].SensorResolution = FIntVector2(CameraIntrinsicsData.sensorResolution.x, CameraIntrinsicsData.sensorResolution.y);

	const FVector LensPosition = FVector(-CameraIntrinsicsData.lensTranslation.Z, CameraIntrinsicsData.lensTranslation.X, CameraIntrinsicsData.lensTranslation.Y) * WorldToMeters;

	// TODO: Simplify. This converts currently from Camera2API -> Unity -> Unreal. Convert directly from Camera2API to Unreal.
	const FQuat Orientation = FQuat(-CameraIntrinsicsData.lensRotation.x, -CameraIntrinsicsData.lensRotation.y, CameraIntrinsicsData.lensRotation.z, CameraIntrinsicsData.lensRotation.w).Inverse() * FQuat::MakeFromEuler(FVector(-180.0f, 0.0f, 0.0f));
	const FQuat LensOrientation = FQuat(Orientation.Z, Orientation.X, Orientation.Y, Orientation.W);

	CameraIntrinsics[CameraEyeIndex].LensOffsetPosition = LensPosition;
	CameraIntrinsics[CameraEyeIndex].LensOffsetOrientation = LensOrientation;

	return true;
}

void UMRUKPassthroughCameraAccessSubsystem::Stop(EMRUKCameraEye CameraEye)
{
	const int32 CameraEyeIndex = static_cast<int32>(CameraEye);

	UE_LOG(LogMRUK, Log, TEXT("Stop camera eye: %d"), CameraEyeIndex);

	if (CameraEyeIndex < 0 || CameraEyeIndex >= Textures.Num())
	{
		UE_LOG(LogMRUK, Error, TEXT("Failed to stop. Invalid camera eye: %d"), CameraEyeIndex);
		return;
	}

	MRUKShared::GetInstance()->CameraStop(CameraEyeIndex);
	// Enqueue a update native texture command to clear the native texture
	ENQUEUE_RENDER_COMMAND(InitializePcaCamera)
	([this, CameraEyeIndex](FRHICommandListImmediate& RHICmdList) {
		MRUKShared::GetInstance()->CameraUpdateNativeTexture(CameraEyeIndex);
	});
}

void UMRUKPassthroughCameraAccessSubsystem::GetCameraPose(EMRUKCameraEye Eye, FVector& OutPosition, FQuat& OutOrientation)
{
	const int32 EyeIndex = static_cast<int32>(Eye);
	if (EyeIndex >= TimestampNsMonotonic.Num())
	{
		UE_LOG(LogMRUK, Error, TEXT("Invalid Eye passed to GetCameraPose"));
		OutPosition = FVector::ZeroVector;
		OutOrientation = FQuat::Identity;
		return;
	}

	const int64 TimestampNs = TimestampNsMonotonic[static_cast<int32>(Eye)];

	FVector3f CameraPosition{};
	MRUKShared::Quatf CameraRotation{};
	MRUKShared::GetInstance()->GetHeadsetPoseAtTime(TimestampNs, &CameraPosition, &CameraRotation);

	const double WorldToMeters = GetWorld() ? GetWorld()->GetWorldSettings()->WorldToMeters : 100.0;
	const FVector Position = PositionToUnreal(CameraPosition, WorldToMeters);
	const FQuat Orientation = ToUnreal(CameraRotation);

	OutPosition = Position + Orientation * CameraIntrinsics[EyeIndex].LensOffsetPosition;
	OutOrientation = Orientation * CameraIntrinsics[EyeIndex].LensOffsetOrientation;
}

FMRUKCameraIntrinsics UMRUKPassthroughCameraAccessSubsystem::GetCameraIntrinsics(EMRUKCameraEye Eye) const
{
	const int32 EyeIndex = static_cast<int32>(Eye);
	if (EyeIndex >= TimestampNsMonotonic.Num())
	{
		UE_LOG(LogMRUK, Error, TEXT("Invalid Eye passed to GetCameraPose"));
		return {};
	}
	return CameraIntrinsics[EyeIndex];
}

void UMRUKPassthroughCameraAccessSubsystem::CalcSensorCropRegion(EMRUKCameraEye Eye, double& OutX, double& OutY, double& OutWidth, double& OutHeight)
{
	const int32 EyeIndex = static_cast<int32>(Eye);
	if (EyeIndex >= Textures.Num() || !Textures[EyeIndex] || !Textures[EyeIndex]->PassthroughCameraAccess)
	{
		OutX = 0.0f;
		OutY = 0.0f;
		OutWidth = 0.0f;
		OutHeight = 0.0f;
		return;
	}

	const FVector2D SensorResolution(CameraIntrinsics[EyeIndex].SensorResolution.X, CameraIntrinsics[EyeIndex].SensorResolution.Y);
	const FVector2D CurrentResolution(Textures[EyeIndex]->PassthroughCameraAccess->ResolutionWidth, Textures[EyeIndex]->PassthroughCameraAccess->ResolutionHeight);
	FVector2D ScaleFactor = CurrentResolution / SensorResolution;
	ScaleFactor /= FMath::Max(ScaleFactor.X, ScaleFactor.Y);

	const double X = SensorResolution.X * (1.0f - ScaleFactor.X) * 0.5;
	const double Y = SensorResolution.Y * (1.0f - ScaleFactor.Y) * 0.5;
	const double Width = SensorResolution.X * ScaleFactor.X;
	const double Height = SensorResolution.Y * ScaleFactor.Y;

	OutX = X;
	OutY = Y;
	OutWidth = Width;
	OutHeight = Height;
}

void UMRUKPassthroughCameraAccessSubsystem::ViewportPointToWorldSpaceRay(EMRUKCameraEye Eye, FVector2D ViewportPoint, FVector& OutPosition, FVector& OutDirection)
{
	const int32 EyeIndex = static_cast<int32>(Eye);
	if (EyeIndex >= TimestampNsMonotonic.Num())
	{
		UE_LOG(LogMRUK, Error, TEXT("Invalid Eye passed to ViewportPointToWorldSpaceRay"));
	}

	FVector CameraPosition;
	FQuat CameraOrientation;
	GetCameraPose(Eye, CameraPosition, CameraOrientation);

	double SensorCropX, SensorCropY, SensorCropWidth, SensorCropHeight;
	CalcSensorCropRegion(Eye, SensorCropX, SensorCropY, SensorCropWidth, SensorCropHeight);

	OutPosition = CameraPosition;

	const double FocalLengthX = CameraIntrinsics[EyeIndex].FocalLength.X;
	const double FocalLengthY = CameraIntrinsics[EyeIndex].FocalLength.Y;

	if (FocalLengthX == 0.0 || FocalLengthY == 0.0)
	{
		OutDirection = FVector::ZeroVector;
		return;
	}

	const FVector DirectionInCamera(
		1.0,
		(SensorCropX + SensorCropWidth * ViewportPoint.X - CameraIntrinsics[EyeIndex].PrincipalPoint.X) / FocalLengthX,
		(SensorCropY + SensorCropHeight * ViewportPoint.Y - CameraIntrinsics[EyeIndex].PrincipalPoint.Y) / FocalLengthY);

	FVector RayDirection = DirectionInCamera;

	RayDirection = CameraOrientation.RotateVector(RayDirection.GetSafeNormal());

	OutDirection = RayDirection.GetSafeNormal();
}

FVector2D UMRUKPassthroughCameraAccessSubsystem::WorldToViewportPoint(EMRUKCameraEye Eye, FVector WorldPosition)
{
	const int32 EyeIndex = static_cast<int32>(Eye);
	if (EyeIndex >= TimestampNsMonotonic.Num())
	{
		UE_LOG(LogMRUK, Error, TEXT("Invalid Eye passed to WorldToViewportPoint"));
	}

	FVector CameraPosition;
	FQuat CameraOrientation;
	GetCameraPose(Eye, CameraPosition, CameraOrientation);

	FTransform CameraTransform;
	CameraTransform.SetLocation(CameraPosition);
	CameraTransform.SetRotation(CameraOrientation);

	const FVector PositionInCameraSpace = CameraTransform.Inverse().TransformPosition(WorldPosition);

	const FVector2D FocalLength = CameraIntrinsics[EyeIndex].FocalLength;
	const FVector2D PrincipalPoint = CameraIntrinsics[EyeIndex].PrincipalPoint;

	const FVector2D SensorPoint(
		(PositionInCameraSpace.Y / PositionInCameraSpace.X) * FocalLength.X + PrincipalPoint.X,
		(PositionInCameraSpace.Z / PositionInCameraSpace.X) * FocalLength.Y + PrincipalPoint.Y);

	double SensorCropX, SensorCropY, SensorCropWidth, SensorCropHeight;
	CalcSensorCropRegion(Eye, SensorCropX, SensorCropY, SensorCropWidth, SensorCropHeight);

	return FVector2D{
		(SensorPoint.X - SensorCropX) / SensorCropWidth,
		(SensorPoint.Y - SensorCropY) / SensorCropHeight,
	};
}

FDateTime UMRUKPassthroughCameraAccessSubsystem::GetTimestamp(EMRUKCameraEye Eye) const
{
	const int32 EyeIndex = static_cast<int32>(Eye);
	if (EyeIndex >= Timestamp.Num())
	{
		UE_LOG(LogMRUK, Error, TEXT("Invalid Eye passed to GetTimestamp"));
		return 0;
	}
	return Timestamp[EyeIndex];
}

void UMRUKPassthroughCameraAccessSubsystem::AddTexture(UMRUKPassthroughCameraAccessTexture* Texture)
{
	const int32 EyeIndex = static_cast<int32>(Texture->PassthroughCameraAccess->CameraEye);
	if (EyeIndex >= Textures.Num() || Textures[EyeIndex] == Texture)
	{
		return;
	}

	if (Textures[EyeIndex])
	{
		UE_LOG(LogMRUK, Error, TEXT("Only one PCA texture per eye is allowed!"));
		return;
	}

	Textures[EyeIndex] = Texture;
}

void UMRUKPassthroughCameraAccessSubsystem::RemoveTexture(UMRUKPassthroughCameraAccessTexture* Texture)
{
	for (int32 i = 0; i < Textures.Num(); ++i)
	{
		if (Textures[i] == Texture)
		{
			Textures[i] = nullptr;
		}
	}
}

void UMRUKPassthroughCameraAccessSubsystem::OnWorldPreActorTick(UWorld* World, ELevelTick TickType, float DeltaTime)
{
	// Only process textures if we are initialized. The MRUK shared library gets initialized after the this subsystem because this subsystem is a engine
	// subsystem and the MRUK shared library gets initialized in game instance subsystem.
	if (!bInitialized || !MRUKShared::GetInstance())
	{
		return;
	}

	for (UMRUKPassthroughCameraAccessTexture* Texture : Textures)
	{
		if (!Texture)
		{
			continue;
		}
		int64_t TimestampMicrosecondsRealtime = 0;
		const int32 EyeIndex = static_cast<int32>(Texture->PassthroughCameraAccess->CameraEye);
		if (EyeIndex >= TimestampNsMonotonic.Num())
		{
			UE_LOG(LogMRUK, Error, TEXT("Invalid CameraEye on PassthroughCameraAccess of texture"));
			continue;
		}
		bool bSuccess = false;
		FPassthroughCameraAccessTextureResource* Resource = static_cast<FPassthroughCameraAccessTextureResource*>(Texture->GetResource());
#if !PLATFORM_ANDROID
		if (const uint8* Buffer = MRUKShared::GetInstance()->CameraAcquireLatestCpuImage(EyeIndex, &TimestampMicrosecondsRealtime, &TimestampNsMonotonic[EyeIndex]))
		{
			bSuccess = true;
			ENQUEUE_RENDER_COMMAND(PcaTextureResourceRender)
			([Texture, Buffer, Resource](FRHICommandListImmediate& RHICmdList) {
				check(Resource);
				Resource->UpdateFromCpuBuffer(RHICmdList, Buffer, Texture->CalcTextureMemorySizeEnum(TMC_ResidentMips));
				MRUKShared::GetInstance()->CameraReleaseLatestCpuImage(static_cast<int>(Texture->PassthroughCameraAccess->CameraEye));
			});
		}
#else
		if (MRUKShared::GetInstance()->CameraGetLatestImage(static_cast<int>(Texture->PassthroughCameraAccess->CameraEye), &TimestampMicrosecondsRealtime, &TimestampNsMonotonic[EyeIndex]))
		{
			bSuccess = true;
			ENQUEUE_RENDER_COMMAND(UpdatePcaTexture)
			([Texture, Resource](FRHICommandListImmediate& RHICmdList) {
				Resource->UpdateFromGpuBuffer(RHICmdList);
				MRUKShared::GetInstance()->CameraUpdateNativeTexture(static_cast<int>(Texture->PassthroughCameraAccess->CameraEye));
			});
		}
#endif
		if (bSuccess)
		{
			Timestamp[EyeIndex] = DateTimeFromUnixMicroseconds(static_cast<int64>(TimestampMicrosecondsRealtime));
		}
	}
}

UMRUKPassthroughCameraAccess::UMRUKPassthroughCameraAccess()
{
	FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject(this, &UMRUKPassthroughCameraAccess::OnSuspend);
	FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddUObject(this, &UMRUKPassthroughCameraAccess::OnResume);
}

bool UMRUKPassthroughCameraAccess::Play()
{
	UMRUKPassthroughCameraAccessSubsystem* const PcaSubsystem = GEngine->GetEngineSubsystem<UMRUKPassthroughCameraAccessSubsystem>();
	const bool bSuccess = PcaSubsystem->Play(ResolutionWidth, ResolutionHeight, MaxFramerate, CameraEye);
	if (bSuccess)
	{
		PlayState = EMRUKCameraPlayState::Playing;
	}
	return bSuccess;
}

void UMRUKPassthroughCameraAccess::Stop()
{
	UMRUKPassthroughCameraAccessSubsystem* const PcaSubsystem = GEngine->GetEngineSubsystem<UMRUKPassthroughCameraAccessSubsystem>();
	PcaSubsystem->Stop(CameraEye);
	PlayState = EMRUKCameraPlayState::Stopped;
}

void UMRUKPassthroughCameraAccess::GetCameraPose(FVector& OutPosition, FQuat& OutOrientation)
{
	if (!IsCameraPlaying())
	{
		OutPosition = FVector::ZeroVector;
		OutOrientation = FQuat::Identity;
		return;
	}
	UMRUKPassthroughCameraAccessSubsystem* const PcaSubsystem = GEngine->GetEngineSubsystem<UMRUKPassthroughCameraAccessSubsystem>();
	PcaSubsystem->GetCameraPose(CameraEye, OutPosition, OutOrientation);
}

FMRUKCameraIntrinsics UMRUKPassthroughCameraAccess::GetCameraIntrinsics() const
{
	if (!IsCameraPlaying())
	{
		return {};
	}
	UMRUKPassthroughCameraAccessSubsystem* const PcaSubsystem = GEngine->GetEngineSubsystem<UMRUKPassthroughCameraAccessSubsystem>();
	return PcaSubsystem->GetCameraIntrinsics(CameraEye);
}

void UMRUKPassthroughCameraAccess::ViewportPointToWorldSpaceRay(FVector2D ViewportPoint, FVector& OutPosition, FVector& OutDirection)
{
	if (!IsCameraPlaying())
	{
		OutPosition = FVector::ZeroVector;
		OutDirection = FVector::ZeroVector;
		return;
	}
	UMRUKPassthroughCameraAccessSubsystem* const PcaSubsystem = GEngine->GetEngineSubsystem<UMRUKPassthroughCameraAccessSubsystem>();
	PcaSubsystem->ViewportPointToWorldSpaceRay(CameraEye, ViewportPoint, OutPosition, OutDirection);
}

FVector2D UMRUKPassthroughCameraAccess::WorldToViewportPoint(FVector WorldPosition)
{
	if (!IsCameraPlaying())
	{
		return FVector2D::ZeroVector;
	}
	UMRUKPassthroughCameraAccessSubsystem* const PcaSubsystem = GEngine->GetEngineSubsystem<UMRUKPassthroughCameraAccessSubsystem>();
	return PcaSubsystem->WorldToViewportPoint(CameraEye, WorldPosition);
}

FDateTime UMRUKPassthroughCameraAccess::GetTimestamp() const
{
	if (!IsCameraPlaying())
	{
		return {};
	}
	UMRUKPassthroughCameraAccessSubsystem* const PcaSubsystem = GEngine->GetEngineSubsystem<UMRUKPassthroughCameraAccessSubsystem>();
	return PcaSubsystem->GetTimestamp(CameraEye);
}

EMRUKCameraPlayState UMRUKPassthroughCameraAccess::GetCameraPlayState() const
{
	return PlayState;
}

bool UMRUKPassthroughCameraAccess::IsCameraPlaying() const
{
	return PlayState == EMRUKCameraPlayState::Playing || PlayState == EMRUKCameraPlayState::PlaySuspended;
}

void UMRUKPassthroughCameraAccess::OnSuspend()
{
	UE_LOG(LogMRUK, Log, TEXT("Passthrough Camera Access eye %d suspend"), CameraEye);
	if (PlayState == EMRUKCameraPlayState::Playing)
	{
		Stop();
		PlayState = EMRUKCameraPlayState::PlaySuspended;
	}
}

void UMRUKPassthroughCameraAccess::OnResume()
{
	UE_LOG(LogMRUK, Log, TEXT("Passthrough Camera Access eye %d resume"), CameraEye);
	if (PlayState == EMRUKCameraPlayState::PlaySuspended)
	{
		PlayState = EMRUKCameraPlayState::Stopped;
		Play();
	}
}

UMRUKPassthroughCameraAccessTexture::UMRUKPassthroughCameraAccessTexture(FVTableHelper& Helper)
	: UTexture(Helper)
{
#if PLATFORM_ANDROID
	SRGB = true;
#else
	SRGB = false;
#endif
}

void UMRUKPassthroughCameraAccessTexture::RegisterTexture()
{
	if (GEngine)
	{
		if (UMRUKPassthroughCameraAccessSubsystem* const PcaSubsystem = GEngine->GetEngineSubsystem<UMRUKPassthroughCameraAccessSubsystem>())
		{
			PcaSubsystem->AddTexture(this);
		}
	}
}

void UMRUKPassthroughCameraAccessTexture::UnregisterTexture()
{
	if (GEngine)
	{
		if (UMRUKPassthroughCameraAccessSubsystem* const PcaSubsystem = GEngine->GetEngineSubsystem<UMRUKPassthroughCameraAccessSubsystem>())
		{
			PcaSubsystem->RemoveTexture(this);
		}
	}
}

float UMRUKPassthroughCameraAccessTexture::GetAspectRatio() const
{
	if (PassthroughCameraAccess->ResolutionHeight == 0)
	{
		return 0.0f;
	}

	return static_cast<float>(PassthroughCameraAccess->ResolutionWidth) / PassthroughCameraAccess->ResolutionHeight;
}

int32 UMRUKPassthroughCameraAccessTexture::GetWidth() const
{
	return PassthroughCameraAccess->ResolutionWidth;
}

int32 UMRUKPassthroughCameraAccessTexture::GetHeight() const
{
	return PassthroughCameraAccess->ResolutionHeight;
}

void UMRUKPassthroughCameraAccessTexture::BeginDestroy()
{
	UnregisterTexture();
	Super::BeginDestroy();
}

FTextureResource* UMRUKPassthroughCameraAccessTexture::CreateResource()
{
	return new FPassthroughCameraAccessTextureResource(this, Guid);
}

EMaterialValueType UMRUKPassthroughCameraAccessTexture::GetMaterialType() const
{
	return MCT_Texture2D;
}

float UMRUKPassthroughCameraAccessTexture::GetSurfaceWidth() const
{
	return static_cast<float>(PassthroughCameraAccess->ResolutionWidth);
}

float UMRUKPassthroughCameraAccessTexture::GetSurfaceHeight() const
{
	return static_cast<float>(PassthroughCameraAccess->ResolutionHeight);
}

float UMRUKPassthroughCameraAccessTexture::GetSurfaceDepth() const
{
	return 0.0f;
}

uint32 UMRUKPassthroughCameraAccessTexture::GetSurfaceArraySize() const
{
	return 0;
}

FGuid UMRUKPassthroughCameraAccessTexture::GetExternalTextureGuid() const
{
	return Guid;
}

ETextureClass UMRUKPassthroughCameraAccessTexture::GetTextureClass() const
{
	return ETextureClass::Other2DNoSource;
}

uint32 UMRUKPassthroughCameraAccessTexture::CalcTextureMemorySizeEnum(ETextureMipCount Enum) const
{
	return PassthroughCameraAccess->ResolutionWidth * PassthroughCameraAccess->ResolutionHeight * SizeForPixelFormat(PixelFormat);
}

EPixelFormat UMRUKPassthroughCameraAccessTexture::GetPixelFormat() const
{
	return PixelFormat;
}

FPassthroughCameraAccessTextureResource::FPassthroughCameraAccessTextureResource(UMRUKPassthroughCameraAccessTexture* InOwner, FGuid InTextureGuid)
	: TextureGuid(InTextureGuid)
	, Owner(InOwner)
{
}

FIntPoint FPassthroughCameraAccessTextureResource::GetSizeXY() const
{
	if (IsValid(Owner))
	{
		return FIntPoint(Owner->GetWidth(), Owner->GetHeight());
	}
	return FIntPoint(0, 0);
}

FString FPassthroughCameraAccessTextureResource::GetFriendlyName() const
{
	if (IsValid(Owner))
	{
		return Owner.GetPathName();
	}
	return TEXT("");
}

uint32 FPassthroughCameraAccessTextureResource::GetSizeX() const
{
	if (Owner)
	{
		return Owner->GetWidth();
	}
	return 0;
}

uint32 FPassthroughCameraAccessTextureResource::GetSizeY() const
{
	if (Owner)
	{
		return Owner->GetHeight();
	}
	return 0;
}

void FPassthroughCameraAccessTextureResource::CreateTexture(FRHICommandListBase& RHICmdList)
{
	if (TextureRHI.IsValid())
	{
		TextureRHI.SafeRelease();
	}

	if (!IsValid(Owner))
	{
		return;
	}

	ETextureCreateFlags TextureCreateFlags = ETextureCreateFlags::ShaderResource | ETextureCreateFlags::UAV | ETextureCreateFlags::RenderTargetable;
#if PLATFORM_ANDROID
	TextureCreateFlags |= ETextureCreateFlags::SRGB;
#endif

	// Create texture
	static const FLazyName ClassName(TEXT("FPassthroughCameraAccessTextureResource"));
	const FRHITextureCreateDesc Desc =
		FRHITextureCreateDesc::Create2D(TEXT("PassthroughCameraAccessTextureResourceOutput"))
			.SetExtent({ Owner->GetWidth(), Owner->GetHeight() })
			.SetFormat(Owner->GetPixelFormat())
			.SetNumMips(1)
			.SetFlags(TextureCreateFlags)
			.SetInitialState(ERHIAccess::SRVMask)
			.SetClassName(ClassName)
			.SetOwnerName(GetOwnerName());
	TextureRHI = RHICmdList.CreateTexture(Desc);
	RHICmdList.BindDebugLabelName(TextureRHI, TEXT("PassthroughCameraAccessTextureResourceOutput"));
	RHICmdList.UpdateTextureReference(TextureReferenceRHI, TextureRHI);

	// Create sampler
	const FSamplerStateInitializerRHI SamplerStateInitializer(
		SF_Point,
		AM_Clamp,
		AM_Clamp,
		AM_Wrap,
		-1.0f);
	SamplerStateRHI = GetOrCreateSamplerState(SamplerStateInitializer);

	FExternalTextureRegistry::Get().RegisterExternalTexture(TextureGuid, TextureRHI, SamplerStateRHI);

	CurrentDimensions = FIntPoint(Owner->GetWidth(), Owner->GetHeight());
	CurrentEye = Owner->PassthroughCameraAccess->CameraEye;
}

void FPassthroughCameraAccessTextureResource::InitRHI(FRHICommandListBase& RHICmdList)
{
	OculusXRTelemetry::TScopedMarker<MRUKTelemetry::FLoadPassthroughCameraAccess> Event(static_cast<int>(GetTypeHash(this)));
	UpdateTexture(RHICmdList);
}

void FPassthroughCameraAccessTextureResource::ReleaseRHI()
{
	if (IsValid(Owner))
	{
		Owner->UnregisterTexture();
	}

	if (TextureRHI.IsValid())
	{
		TextureRHI.SafeRelease();
	}
}

void FPassthroughCameraAccessTextureResource::UpdateTexture(FRHICommandListBase& RHICmdList)
{
	if (!IsValid(Owner) || !IsValid(Owner->PassthroughCameraAccess))
	{
		return;
	}

	if (CurrentEye != Owner->PassthroughCameraAccess->CameraEye)
	{
		Owner->UnregisterTexture();
		CurrentEye = Owner->PassthroughCameraAccess->CameraEye;
		Owner->RegisterTexture();
	}
	if (!TextureRHI.IsValid() || CurrentDimensions.X != Owner->GetWidth() || CurrentDimensions.Y != Owner->GetHeight())
	{
		ReleaseRHI();
		CreateTexture(RHICmdList);
		Owner->RegisterTexture();
	}
}

void FPassthroughCameraAccessTextureResource::UpdateFromCpuBuffer(FRHICommandListImmediate& RHICmdList, const uint8* Buffer, int BufferSize)
{
	if (!IsValid(Owner))
	{
		return;
	}

	UpdateTexture(RHICmdList);

	check(BufferSize == Owner->CalcTextureMemorySizeEnum(TMC_ResidentMips));
	if (BufferSize != Owner->CalcTextureMemorySizeEnum(TMC_ResidentMips))
	{
		return;
	}

	const FUpdateTextureRegion2D Region(0, 0, 0, 0, CurrentDimensions.X, CurrentDimensions.Y);
	RHICmdList.UpdateTexture2D(TextureRHI, 0, Region, CurrentDimensions.X * SizeForPixelFormat(Owner->GetPixelFormat()), Buffer);
}

void FPassthroughCameraAccessTextureResource::UpdateFromGpuBuffer(FRHICommandListImmediate& RHICmdList)
{
	UpdateTexture(RHICmdList);
}
