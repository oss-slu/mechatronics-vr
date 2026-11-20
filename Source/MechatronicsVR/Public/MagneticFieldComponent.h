#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "MagneticFieldComponent.generated.h"

UCLASS(ClassGroup = (Mechatronics), meta = (BlueprintSpawnableComponent, DisplayName = "Magnetic Field"))
class MECHATRONICSVR_API UMagneticFieldComponent : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()

public:
	UMagneticFieldComponent(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magnetic Field")
	bool bFieldEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magnetic Field", meta = (ClampMin = "0.0", UIMax = "5.0"))
	float FieldStrength = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magnetic Field", meta = (ClampMin = "0.5", Units = "cm"))
	float PoleSeparation = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magnetic Field", meta = (ClampMin = "1.0", Units = "cm"))
	float MaxFieldRadius = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magnetic Field|Display", meta = (ClampMin = "1", ClampMax = "6"))
	int32 FieldLinePlanes = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magnetic Field|Display", meta = (ClampMin = "2", ClampMax = "24"))
	int32 LinesPerPlane = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magnetic Field|Display", meta = (ClampMin = "0.2", Units = "cm"))
	float ArrowSpacing = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magnetic Field|Display", meta = (ClampMin = "0.1", Units = "cm"))
	float ArrowSize = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magnetic Field|Display", meta = (ClampMin = "0.001"))
	float MinVisibleStrength = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magnetic Field|Display", meta = (ClampMin = "0.001"))
	float FullColorStrength = 2.0f;

	UFUNCTION(BlueprintCallable, Category = "Magnetic Field")
	void SetFieldEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Magnetic Field")
	void SetFieldStrength(float NewStrength);

	UFUNCTION(BlueprintCallable, Category = "Magnetic Field")
	void RebuildField();

	UFUNCTION(BlueprintCallable, Category = "Magnetic Field", meta = (WorldContext = "WorldContextObject"))
	static void SetAllMagneticFieldsEnabled(const UObject* WorldContextObject, bool bEnabled);

	virtual void OnRegister() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	FVector FieldAt(const FVector& LocalPoint) const;
};
