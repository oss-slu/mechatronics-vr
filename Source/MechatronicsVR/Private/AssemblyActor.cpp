// Fill out your copyright notice in the Description page of Project Settings.

#include "AssemblyActor.h"
#include "PartActor.h"
#include "SnapPointComponent.h"


// Sets default values
AAssemblyActor::AAssemblyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize default state
	AssemblyState = EAssemblyState::Empty;

	// Create root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

}

// Called when the game starts or when spawned
void AAssemblyActor::BeginPlay()
{
	Super::BeginPlay();

	UpdateAssemblyState();
	
}

// Called every frame
void AAssemblyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CleanupInvalidConstraints();

}

void AAssemblyActor::AddPart(APartActor* NewPart)
{
	if (!NewPart )
	{
		UE_LOG(LogTemp, Warning, TEXT("AAssemblyActor::AddPart: Invalid part"));
		return;	
	}
	if (Parts.Contains(NewPart))
	{
		UE_LOG(LogTemp, Warning, TEXT("AAssemblyActor::AddPart: Part already in assembly"));
		return;
	}
	Parts.Add(NewPart);
	UpdateAssemblyState();

	UE_LOG(LogTemp, Log, TEXT("AAssemblyActor::AddPart: Added part %s. Total parts: %d"), 
		*NewPart->GetName(), Parts.Num());
}

void AAssemblyActor::RemovePart(APartActor* Part)
{
	if (!Part)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAssemblyActor::RemovePart: Part is null"));
		return;
	}

	if (!Parts.Contains(Part))
	{
		UE_LOG(LogTemp, Warning, TEXT("AAssemblyActor::RemovePart: Part %s not found in assembly"), *Part->GetName());
		return;
	}

	// Disconnect all connections involving this part
	for (const FPartConnection& Connection : Connections)
	{
		if (Connection.PartA == Part || Connection.PartB == Part)
		{
			DisconnectParts(Connection.PartA, Connection.PartB);
		}
	}

	Parts.Remove(Part);
	UpdateAssemblyState();

	UE_LOG(LogTemp, Log, TEXT("AAssemblyActor::RemovePart: Removed part %s. Total parts: %d"), 
		   *Part->GetName(), Parts.Num());
}

// ================== CONNECTION MANAGEMENT ==================

bool AAssemblyActor::ConnectParts(APartActor* PartA, APartActor* PartB, USnapPointComponent* SnapPointA, USnapPointComponent* SnapPointB)
{
	// Validate inputs
	if (!PartA || !PartB || !SnapPointA || !SnapPointB)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAssemblyActor::ConnectParts: Invalid input parameters"));
		return false;
	}
    
	// Check if parts are already connected
	if (ArePartsConnected(PartA, PartB))
	{
		UE_LOG(LogTemp, Warning, TEXT("AAssemblyActor::ConnectParts: Parts already connected"));
		return false;
	}
    
	// Validate snap point compatibility
	if (!SnapPointA->CanAcceptPoint(SnapPointB) || !SnapPointB->CanAcceptPoint(SnapPointA))
	{
		UE_LOG(LogTemp, Warning, TEXT("AAssemblyActor::ConnectParts: Snap points not compatible"));
		return false;
	}
    
	// Check if snap points are already assembled
	if (SnapPointA->bIsAssembled || SnapPointB->bIsAssembled)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAssemblyActor::ConnectParts: One or both snap points already assembled"));
		return false;
	}

	// Create physics constraint
	UPhysicsConstraintComponent* Constraint = CreateConstraintBetweenParts(PartA, SnapPointA, PartB, SnapPointB);
	if (!Constraint)
	{
		UE_LOG(LogTemp, Error, TEXT("AAssemblyActor::ConnectParts: Failed to create constraint"));
		return false;
	}

	//Create connection record
	FPartConnection NewConnection;
	NewConnection.PartA = PartA;
	NewConnection.PartB = PartB;
	NewConnection.Constraint = Constraint;

	Connections.Add(NewConnection);

	// Mark snap points as assembled
	SnapPointA->bIsAssembled = true;
	SnapPointB->bIsAssembled = true;

	// update assembly state
	UpdateAssemblyState();

	//fire events
	OnPartsConnected.Broadcast(PartA, PartB);

	UE_LOG(LogTemp, Log, TEXT("AAssemblyActor::ConnectParts: Successfully connected %s to %s"), 
		   *PartA->GetName(), *PartB->GetName());
    
	return true;
}

bool AAssemblyActor::DisconnectParts(APartActor* PartA, APartActor* PartB)
{
	if (!PartA || !PartB)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAssemblyActor::DisconnectParts: Invalid input parameters"));
		return false;
	}

	// Find the connection
	int32 ConnectionIndex = -1;
	for (int32 i = 0; i < Connections.Num(); ++i)
	{
		const FPartConnection& Connection = Connections[i];
		if ((Connection.PartA == PartA && Connection.PartB == PartB) ||
			(Connection.PartA == PartB && Connection.PartB == PartA))
		{
			ConnectionIndex = i;
			break;
		}
	}

	if (ConnectionIndex == -1)
	{
		UE_LOG(LogTemp, Warning, TEXT("AAssemblyActor::DisconnectParts: No connection found between %s and %s"), 
			   *PartA->GetName(), *PartB->GetName());
		return false;
	}
	FPartConnection &Connection = Connections[ConnectionIndex];

	// Destroy the physics constraint
	if (Connection.Constraint)
	{
		PhysicalConstraints.Remove(Connection.Constraint);
		Connection.Constraint->DestroyComponent();
	}
	// Find and unmark snap points
	TArray<USnapPointComponent*> SnapPointsA = PartA->GetSnapPoints();
	TArray<USnapPointComponent*> SnapPointsB = PartB->GetSnapPoints();

	for (USnapPointComponent* SnapA : SnapPointsA)
	{
		for (USnapPointComponent* SnapB : SnapPointsB)
		{
			if (SnapA->bIsAssembled && SnapB->bIsAssembled &&
				SnapA->CanAcceptPoint(SnapB) && SnapB->CanAcceptPoint(SnapA))
			{
				SnapA->bIsAssembled = false;
				SnapB->bIsAssembled = false;
				break;
			}
		}
	}

	// Remove the connection
	Connections.RemoveAt(ConnectionIndex);

	//Update assembly state
	UpdateAssemblyState();

	// Fire events
	OnPartDisconnected.Broadcast(PartA, PartB);

	UE_LOG(LogTemp, Log, TEXT("AAssemblyActor::DisconnectParts: Successfully disconnected %s from %s"), 
		   *PartA->GetName(), *PartB->GetName());
    
	return true;
	
}

// ================== ASSEMBLY STATE MANAGEMENT ==================

bool AAssemblyActor::IsFullyAssembled() const
{
	return (Parts.Num() >= ExpectedPartCount) && (Connections.Num() >= ExpectedConnectionCount) &&
		(AssemblyState == EAssemblyState::FullyAssembled);
}

float AAssemblyActor::GetAssemblyProgress() const
{
	if (ExpectedPartCount == 0 && ExpectedConnectionCount == 0)
	{
		return 0.0f;
	}

	// Calculate progress based on both parts and connections
	const float PartProgress = ExpectedPartCount > 0 ? 
		                           static_cast<float>(Parts.Num()) / static_cast<float>(ExpectedPartCount) : 1.0f;

	const float ConnectionProgress = ExpectedConnectionCount > 0 ? 
		                                 static_cast<float>(Connections.Num()) / static_cast<float>(ExpectedConnectionCount) : 1.0f;
    
	// Weight parts and connections equally
	const float TotalProgress = (PartProgress + ConnectionProgress) * 0.5f;
    
	// Clamp to 0-1 range
	return FMath::Clamp(TotalProgress, 0.0f, 1.0f);
}
void AAssemblyActor::UpdateAssemblyState()
{
	EAssemblyState PreviousState = AssemblyState;

	//determine new state
	if (Parts.Num() == 0)
	{
		AssemblyState = EAssemblyState::Empty;
	}
	else if (IsFullyAssembled())
	{
		AssemblyState = EAssemblyState::FullyAssembled;
	}
	else
	{
		AssemblyState = EAssemblyState::PartiallyAssembled;
	}

	// fire event if state changed
	if (AssemblyState != PreviousState)
	{
		OnAssemblyStateChanged.Broadcast(AssemblyState);
		UE_LOG(LogTemp, Log, TEXT("AAssemblyActor::UpdateAssemblyState: State changed to %d. Parts: %d/%d, Connections: %d/%d"), 
			   static_cast<int32>(AssemblyState), Parts.Num(), ExpectedPartCount, 
			   Connections.Num(), ExpectedConnectionCount);
	}
}


// ================== CONNECTION QUERIES ==================
FPartConnection AAssemblyActor::FindConnection(APartActor* PartA, APartActor* PartB)
{
	if (!PartA  || !PartB) return FPartConnection(); //return invalid connection

	for (const FPartConnection& Connection : Connections)
	{
		if ((Connection.PartA == PartA && Connection.PartB == PartB) ||
			(Connection.PartA == PartB && Connection.PartB == PartA))
		{
			return Connection;
		}
	}
	return FPartConnection(); // return invalid connection	
}

bool AAssemblyActor::HasConnection(APartActor* PartA, APartActor* PartB) const
{
	if (!PartA || !PartB) return false;

	for (const FPartConnection& Connection : Connections)
	{
		if ((Connection.PartA == PartA && Connection.PartB == PartB) ||
			(Connection.PartA == PartB && Connection.PartB == PartA))
		{
			return true;
		}
	}
	return false;
}

bool AAssemblyActor::ArePartsConnected(APartActor* PartA, APartActor* PartB) const
{
	return HasConnection(PartA, PartB);
}

TArray<APartActor*> AAssemblyActor::GetConnectedParts(APartActor* Part) const
{
	TArray<APartActor*> ConnectedParts;
	if (!Part) return ConnectedParts;

	for (const FPartConnection& Connection : Connections)
	{
		if (Connection.PartA == Part && Connection.PartB)
		{
			ConnectedParts.Add(Connection.PartB);
		}
		else if (Connection.PartB == Part && Connection.PartA)
		{
			ConnectedParts.Add(Connection.PartA);
		}
	}
	return ConnectedParts;
}

// ================== PHYSICS CONSTRAINT CREATION ==================

UPhysicsConstraintComponent* AAssemblyActor::CreateConstraintBetweenParts(APartActor* PartA, USnapPointComponent* SnapPointA, 
	APartActor* PartB, USnapPointComponent* SnapPointB)
{
	if (!PartA || !PartB || !SnapPointA || !SnapPointB)
	{
		UE_LOG(LogTemp, Error, TEXT("AAssemblyActor::CreateConstraintBetweenParts: Invalid parameters"));
		return nullptr;
	}

	// Create constraint component
	const FString ConstraintName = FString::Printf(TEXT("Constraint_%s_%s"), 
	                                               *PartA->GetName(), *PartB->GetName());
    
	UPhysicsConstraintComponent* Constraint = NewObject<UPhysicsConstraintComponent>(this, 
																				   UPhysicsConstraintComponent::StaticClass(), *ConstraintName);
	// attach to root component
	Constraint->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

	// set constraint bodies
	Constraint->SetConstrainedComponents(PartA->Mesh, NAME_None, PartB->Mesh, NAME_None);

	//configure constraint type based on snap point metadata
	ConfigureConstraintType(Constraint, SnapPointA, SnapPointB);

	// set constraint location (midpoint between snap points)
	const FVector ConstraintLocation = (SnapPointA->GetComponentLocation() + SnapPointB->GetComponentLocation()) * 0.5f;
	Constraint->SetWorldLocation(ConstraintLocation);

	PhysicalConstraints.Add(Constraint);

	UE_LOG(LogTemp, Log, TEXT("AAssemblyActor::CreateConstraintBetweenParts: Created constraint between %s and %s"), 
		   *PartA->GetName(), *PartB->GetName());
    
	return Constraint;

}

void AAssemblyActor::ConfigureConstraintType(UPhysicsConstraintComponent* Constraint, 
											USnapPointComponent* SnapPointA, 
											USnapPointComponent* SnapPointB)
{
	if (!Constraint || !SnapPointA || !SnapPointB) return;

	// parse metadata to determine constraint type
	FString MetadataA = SnapPointA->Metadata.ToLower();
	FString MetadataB = SnapPointB->Metadata.ToLower();

	// Default to fixed constraint
    Constraint->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.0f);
    Constraint->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.0f);
    Constraint->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.0f);
    Constraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);
    Constraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);
    Constraint->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.0f);
    
    // Check for specific constraint types in metadata
    if (MetadataA.Contains("hinge") || MetadataB.Contains("hinge") ||
        MetadataA.Contains("rotate") || MetadataB.Contains("rotate") ||
        MetadataA.Contains("shaft") || MetadataB.Contains("shaft"))
    {
        // Allow rotation around one axis (typically Z for motors)
        Constraint->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Free, 0.0f);
        
        UE_LOG(LogTemp, Log, TEXT("AAssemblyActor::ConfigureConstraintType: Set hinge constraint"));
    }
    else if (MetadataA.Contains("slide") || MetadataB.Contains("slide") ||
             MetadataA.Contains("prismatic") || MetadataB.Contains("prismatic"))
    {
        // Allow linear motion along one axis
        Constraint->SetLinearZLimit(ELinearConstraintMotion::LCM_Free, 0.0f);
        
        UE_LOG(LogTemp, Log, TEXT("AAssemblyActor::ConfigureConstraintType: Set prismatic constraint"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AAssemblyActor::ConfigureConstraintType: Set fixed constraint"));
    }
}

// ================== CLEANUP ==================

void AAssemblyActor::CleanupInvalidConstraints()
{
	// Remove any constraint components that have been destroyed or are invalid
	PhysicalConstraints.RemoveAll([](const TObjectPtr<UPhysicsConstraintComponent>& Constraint)
	{
		return !IsValid(Constraint);
	});
    
	// Remove any connections with invalid constraints
	Connections.RemoveAll([](const FPartConnection& Connection)
	{
		return !IsValid(Connection.Constraint) || !IsValid(Connection.PartA) || !IsValid(Connection.PartB);
	});
}