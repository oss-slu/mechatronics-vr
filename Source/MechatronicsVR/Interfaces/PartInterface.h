// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PartInterface.generated.h"

class USnapPointComponent;
class AAssemblyActor;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPartInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MECHATRONICSVR_API IPartInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Part Interface")
	FName GetPartID() const;
	virtual FName GetPartID_Implementation() const = 0;

	 UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Part Interface")
    FText GetPartDisplayName() const;
    virtual FText GetPartDisplayName_Implementation() const = 0;

    // === SNAP SYSTEM INTEGRATION ===
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Part Interface")
    TArray<USnapPointComponent*> GetSnapPoints() const;
    virtual TArray<USnapPointComponent*> GetSnapPoints_Implementation() const = 0;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Part Interface")
    bool CanSnapToPart(const TScriptInterface<IPartInterface>& OtherPart) const;
    virtual bool CanSnapToPart_Implementation(const TScriptInterface<IPartInterface>& OtherPart) const { return true; }

    // === ASSEMBLY STATE ===
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Part Interface")
    bool IsAssembled() const;
    virtual bool IsAssembled_Implementation() const = 0;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Part Interface")
    AAssemblyActor* GetAssemblyActor() const;
    virtual AAssemblyActor* GetAssemblyActor_Implementation() const { return nullptr; }

    // === GRAB SYSTEM INTEGRATION ===
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Part Interface")
    bool IsGrabbable() const;
    virtual bool IsGrabbable_Implementation() const { return true; }

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Part Interface")
    bool IsCurrentlyGrabbed() const;
    virtual bool IsCurrentlyGrabbed_Implementation() const = 0;

    // === LESSON SYSTEM INTEGRATION ===
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Part Interface")
    bool IsAllowedInCurrentLessonStep() const;
    virtual bool IsAllowedInCurrentLessonStep_Implementation() const { return true; }

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Part Interface")
    void SetLessonHighlight(bool bHighlighted);
    virtual void SetLessonHighlight_Implementation(bool bHighlighted) {}
};
