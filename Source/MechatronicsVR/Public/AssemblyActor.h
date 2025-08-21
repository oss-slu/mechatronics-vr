// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "AssemblyActor.generated.h"

class UAssemblyComponent;
class USnapPointComponent;
class APartActor;

UENUM(BlueprintType)
enum class EAssemblyState : uint8
{
	Empty			   UMETA(DisplayName = "Empty"),
	PartiallyAssembled UMETA(DisplayName = "Partially Assembled"),
	FullyAssembled     UMETA(DisplayName = "Fully Assembled"),
	
};
USTRUCT(BlueprintType)
struct FPartConnection
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part Connection")
	TObjectPtr<APartActor> PartA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part Connection")
	TObjectPtr<APartActor> PartB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part Connection")
	TObjectPtr<USnapPointComponent> SnapPointA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part Connection")
	TObjectPtr<USnapPointComponent> SnapPointB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part Connection")
	TObjectPtr<UPhysicsConstraintComponent> Constraint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part Connection")
	bool bIsConnected;

	/** Is this connection to a base snap point? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part Connection")
	bool bIsBaseConnection;

	FPartConnection()
	{
		PartA = nullptr;
		PartB = nullptr;
		SnapPointA = nullptr;
		SnapPointB = nullptr;
		Constraint = nullptr;
		bIsConnected = false;
		bIsBaseConnection = false;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssemblyStateChanged, EAssemblyState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartsConnected, APartActor*, PartA, APartActor*, PartB);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartDisconnected, APartActor*, PartA, APartActor*, PartB);



UCLASS()
class MECHATRONICSVR_API AAssemblyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAssemblyActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Snap points that belong directly to the assembly (for base connections) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assembly")
	TArray<USnapPointComponent*> BaseSnapPoints; 
    
	/** Root component for snap points */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assembly")
	TObjectPtr<USceneComponent> SnapPointRoot;
    
	/** Is this assembly ready to accept parts? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assembly")
	bool bIsAssemblyActive = true;
    
	/** Optional: Visual indicator for base snap points (for debugging) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assembly")
	bool bShowBaseSnapPoints = true;
    
	/** Get all base snap points on this assembly */
	UFUNCTION(BlueprintCallable, Category = "Assembly")
	TArray<USnapPointComponent*> GetBaseSnapPoints() const { return BaseSnapPoints; }
    

    
	/** Check if a part is the first part (attached to base) */
	UFUNCTION(BlueprintCallable, Category = "Assembly")
	bool IsPartAttachedToBase(APartActor* Part) const;

	/* ================== PART MANAGEMENT ================== */

	/** All parts that belong to this assembly */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assembly")
	TArray<TObjectPtr<APartActor>> Parts;

	/** Current connections between parts */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assembly")
	TArray<FPartConnection> Connections;

	/** Current assembly state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assembly")
	EAssemblyState AssemblyState;

	/** Expected number of parts for full assembly */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assembly")
	int32 ExpectedPartCount = 8;

	/** Expected number of parts for full assembly */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assembly")
	int32 ExpectedConnectionCount = 7;

	// ================== ASSEMBLY FUNCTIONS ==================
	/** Add a part to the assembly */
	UFUNCTION(BlueprintCallable, Category = "Assembly")
	void AddPart(APartActor* NewPart);

	/** Remove a part from the assembly */
	UFUNCTION(BlueprintCallable, Category = "Assembly")
	void RemovePart(APartActor* Part);

	/** Attempt to connect two parts via their snap points */
	UFUNCTION(BlueprintCallable, Category = "Assembly")
	bool ConnectParts(APartActor* PartA, APartActor* PartB,
		USnapPointComponent* SnapPointA, USnapPointComponent* SnapPointB);

	/** Disconnect two parts */
	UFUNCTION(BlueprintCallable, Category = "Assembly")
	bool DisconnectParts(APartActor* PartA, APartActor* PartB);

	/** Check if assembly is complete */
	UFUNCTION(BlueprintCallable, Category = "Assembly")
	bool IsFullyAssembled() const;

	/** Get assembly completion percentage */
	UFUNCTION(BlueprintCallable, Category = "Assembly")
	float GetAssemblyProgress() const;

	/** Update assembly state based on current connections */
	UFUNCTION(BlueprintCallable, Category = "Assembly")
	void UpdateAssemblyState();

	/** Find connection between two parts (returns copy) */
	UFUNCTION(BlueprintCallable, Category = "Assembly")
	FPartConnection FindConnection(APartActor* PartA, APartActor* PartB);

	/** Check if connection exists between two parts */
	UFUNCTION(BlueprintCallable, Category = "Assembly")
	bool HasConnection(APartActor* PartA, APartActor* PartB) const;

	/** Check if two parts are connected */
	UFUNCTION(BlueprintCallable, Category = "Assembly")
	bool ArePartsConnected(APartActor* PartA, APartActor* PartB) const;

	/** Get all parts connected to a specific part */
	UFUNCTION(BlueprintCallable, Category = "Assembly")
	TArray<APartActor*> GetConnectedParts(APartActor* Part) const;
	

	// ================== EVENTS ==================
	UPROPERTY(BlueprintAssignable, Category = "Assembly Events")
	FOnAssemblyStateChanged OnAssemblyStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Assembly Events")
	FOnPartsConnected OnPartsConnected;

	UPROPERTY(BlueprintAssignable, Category = "Assembly Events")
	FOnPartDisconnected OnPartDisconnected;

protected:
	// ================== INTERNAL FUNCTIONS ==================
	UPhysicsConstraintComponent* CreateConstraintBetweenParts(APartActor* PartA, USnapPointComponent* SnapPointA,
															  APartActor* PadtB, USnapPointComponent* SnapPointB);

	/** Determine constraint type between two parts */
	void ConfigureConstraintType(UPhysicsConstraintComponent* Constraint, 
		USnapPointComponent* SnapPointA, USnapPointComponent* SnapPointB);

	UPhysicsConstraintComponent* CreateConstraintToBase(
	APartActor* Part, 
	USnapPointComponent* PartSnapPoint,
	USnapPointComponent* BaseSnapPoint);

	/** CLean up broken constraints */
	void CleanupInvalidConstraints();
	
private:
	/** Internal constraint storage for cleanup */
	UPROPERTY()
	TArray<TObjectPtr<UPhysicsConstraintComponent>> PhysicalConstraints;
	

};
