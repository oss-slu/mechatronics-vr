// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "TextureResource.h"
#include "UObject/Object.h"
#include "Containers/StaticArray.h"
#include "Subsystems/EngineSubsystem.h"
#include "Engine/Texture.h"
#include "UnrealClient.h"
#include "Engine/World.h"

#include "MRUKPassthroughCameraAccess.generated.h"

UENUM(BlueprintType)
enum class EMRUKCameraEye : uint8
{
	Left = 0,
	Right,
	Count,
};

UENUM(BlueprintType)
enum class EMRUKCameraPlayState : uint8
{
	Stopped = 0,
	Playing,
	PlaySuspended,
};

USTRUCT(BlueprintType)
struct FMRUKCameraIntrinsics
{
	GENERATED_BODY()

	/// The focal length of the camera.
	FVector2D FocalLength;
	/// The principal point of the camera.
	FVector2D PrincipalPoint;
	/// The sensor resolution of the camera.
	FIntVector2 SensorResolution;
	/// The translation of the camera sensor relative to the headset.
	FVector LensOffsetPosition;
	/// The orientation of the camera sensor relative to the headset.
	FQuat LensOffsetOrientation;
};

UCLASS(BlueprintType, ClassGroup = MRUtilityKit, meta = (DisplayName = "MR Utility Kit Passthrough Camera Access Subsystem"))
class MRUTILITYKIT_API UMRUKPassthroughCameraAccessSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	/**
	 * Get a list of all supported resolutions.
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	TArray<FVector2D> GetSupportedResolutions(EMRUKCameraEye CameraEye);

	/**
	 * Start the passthrough camera for the given eye.
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	bool Play(int& Width, int& Height, int MaxFramerate, EMRUKCameraEye CameraEye);

	/**
	 * Stop the passthrough camera for the given eye.
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	void Stop(EMRUKCameraEye CameraEye);

	/**
	 * Get the pose of the camera.
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	void GetCameraPose(EMRUKCameraEye Eye, FVector& OutPosition, FQuat& OutOrientation);

	/**
	 * Get the camera intrinsics.
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	FMRUKCameraIntrinsics GetCameraIntrinsics(EMRUKCameraEye Eye) const;

	/**
	 * Constructs a ray in world space that points to the given point in passthrough camera space.
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	void ViewportPointToWorldSpaceRay(EMRUKCameraEye Eye, FVector2D ViewportPoint, FVector& OutPosition, FVector& OutDirection);

	/**
	 * Converts the world position into the passthrough camera space.
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	FVector2D WorldToViewportPoint(EMRUKCameraEye Eye, FVector WorldPosition);

	/**
	 * Timestamp associated with the latest camera image.
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	FDateTime GetTimestamp(EMRUKCameraEye Eye) const;

	void AddTexture(class UMRUKPassthroughCameraAccessTexture* Texture);

	void RemoveTexture(class UMRUKPassthroughCameraAccessTexture* Texture);

private:
	TStaticArray<TObjectPtr<UMRUKPassthroughCameraAccessTexture>, static_cast<int>(EMRUKCameraEye::Count)> Textures = {};
	TStaticArray<int64_t, static_cast<int>(EMRUKCameraEye::Count)> TimestampNsMonotonic = {};
	TStaticArray<FDateTime, static_cast<int>(EMRUKCameraEye::Count)> Timestamp = {};
	TStaticArray<FMRUKCameraIntrinsics, static_cast<int>(EMRUKCameraEye::Count)> CameraIntrinsics = {};

	bool bInitialized = false;
	bool bCameraInitialized = false;

	/** Handle for the OnWorldPreActorTick delegate */
	FDelegateHandle PreActorTickHandle;

	void CalcSensorCropRegion(EMRUKCameraEye Eye, double& OutX, double& OutY, double& OutWidth, double& OutHeight);

	/** Called before actors tick to update textures */
	void OnWorldPreActorTick(UWorld* World, ELevelTick TickType, float DeltaTime);
};

UCLASS(BlueprintType)
class MRUTILITYKIT_API UMRUKPassthroughCameraAccess : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * From which camera eye the video should be played.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MR Utility Kit")
	EMRUKCameraEye CameraEye = EMRUKCameraEye::Left;

	/**
	 * Max resolution width of the video.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MR Utility Kit")
	int ResolutionWidth = 1280;

	/**
	 * Max resolution height of the video.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MR Utility Kit")
	int ResolutionHeight = 960;

	/**
	 * Maximum framerate of the video.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MR Utility Kit")
	int MaxFramerate = 60;

	UMRUKPassthroughCameraAccess();

	/**
	 * Start the passthrough camera.
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	bool Play();

	/**
	 * Stop the passthrough camera.
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	void Stop();

	/**
	 * Get the pose of the camera.
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	void GetCameraPose(FVector& OutPosition, FQuat& OutOrientation);

	/**
	 * Get the camera intrinsics.
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	FMRUKCameraIntrinsics GetCameraIntrinsics() const;

	/**
	 * Constructs a ray in world space that points to the given point in passthrough camera space.
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	void ViewportPointToWorldSpaceRay(FVector2D ViewportPoint, FVector& OutPosition, FVector& OutDirection);

	/**
	 * Converts the world position into the passthrough camera space.
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	FVector2D WorldToViewportPoint(FVector WorldPosition);

	/**
	 * Timestamp associated with the latest camera image.
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	FDateTime GetTimestamp() const;

	/**
	 * Get the current camera play state.
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	EMRUKCameraPlayState GetCameraPlayState() const;

	/**
	 * Check if the camera is currently playing.
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	bool IsCameraPlaying() const;

private:
	EMRUKCameraPlayState PlayState = EMRUKCameraPlayState::Stopped;

	UFUNCTION()
	void OnSuspend();

	UFUNCTION()
	void OnResume();
};

UCLASS(hidecategories = (Adjustments, Compositing, LevelOfDetail, ImportSettings, Object, Texture, Compression), MinimalAPI)
class UMRUKPassthroughCameraAccessTexture : public UTexture
{
	GENERATED_BODY()

public:
	UMRUKPassthroughCameraAccessTexture(FVTableHelper& Helper);

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "MR Utility Kit")
	TObjectPtr<UMRUKPassthroughCameraAccess> PassthroughCameraAccess = nullptr;

	/**
	 * Gets the current aspect ratio of the texture.
	 *
	 * @return Texture aspect ratio.
	 * @see GetHeight, GetWidth
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	MRUTILITYKIT_API float GetAspectRatio() const;

	/**
	 * Gets the current height of the texture.
	 *
	 * @return Texture height (in pixels).
	 * @see GetAspectRatio, GetWidth
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	MRUTILITYKIT_API int32 GetHeight() const;

	/**
	 * Gets the current width of the texture.
	 *
	 * @return Texture width (in pixels).
	 * @see GetAspectRatio, GetHeight
	 */
	UFUNCTION(BlueprintCallable, Category = "MR Utility Kit")
	MRUTILITYKIT_API int32 GetWidth() const;

	MRUTILITYKIT_API void RegisterTexture();

	MRUTILITYKIT_API void UnregisterTexture();

	MRUTILITYKIT_API virtual void BeginDestroy() override;

	MRUTILITYKIT_API virtual class FTextureResource* CreateResource() override;

	MRUTILITYKIT_API virtual EMaterialValueType GetMaterialType() const override;

	MRUTILITYKIT_API virtual float GetSurfaceWidth() const override;

	MRUTILITYKIT_API virtual float GetSurfaceHeight() const override;

	MRUTILITYKIT_API virtual float GetSurfaceDepth() const override;

	MRUTILITYKIT_API virtual uint32 GetSurfaceArraySize() const override;

	MRUTILITYKIT_API virtual FGuid GetExternalTextureGuid() const override;

	MRUTILITYKIT_API virtual ETextureClass GetTextureClass() const override;

	MRUTILITYKIT_API virtual uint32 CalcTextureMemorySizeEnum(ETextureMipCount Enum) const override;

	MRUTILITYKIT_API EPixelFormat GetPixelFormat() const;

private:
	const FGuid Guid = FGuid::NewGuid();
	const EPixelFormat PixelFormat = PF_R8G8B8A8;
};

class FPassthroughCameraAccessTextureResource : public FRenderTarget, public FTextureResource
{
public:
	MRUTILITYKIT_API FPassthroughCameraAccessTextureResource(UMRUKPassthroughCameraAccessTexture* InOwner, FGuid InTextureGuid);

	MRUTILITYKIT_API virtual ~FPassthroughCameraAccessTextureResource() = default;

	MRUTILITYKIT_API virtual FIntPoint GetSizeXY() const override;

	MRUTILITYKIT_API virtual FString GetFriendlyName() const override;

	MRUTILITYKIT_API virtual uint32 GetSizeX() const override;

	MRUTILITYKIT_API virtual uint32 GetSizeY() const override;

	MRUTILITYKIT_API virtual void InitRHI(FRHICommandListBase& RHICmdList) override;

	MRUTILITYKIT_API virtual void ReleaseRHI() override;

	MRUTILITYKIT_API void UpdateFromCpuBuffer(FRHICommandListImmediate& RHICmdList, const uint8* Buffer, int BufferSize);

	MRUTILITYKIT_API void UpdateFromGpuBuffer(FRHICommandListImmediate& RHICmdList);

private:
	/** The external texture GUID to use when initializing this resource. */
	FGuid TextureGuid;

	FIntPoint CurrentDimensions;
	EMRUKCameraEye CurrentEye;

	TObjectPtr<UMRUKPassthroughCameraAccessTexture> Owner = nullptr;

	void CreateTexture(FRHICommandListBase& RHICmdList);

	void UpdateTexture(FRHICommandListBase& RHICmdList);
};
