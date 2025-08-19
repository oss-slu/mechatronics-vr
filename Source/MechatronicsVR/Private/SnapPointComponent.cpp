// Fill out your copyright notice in the Description page of Project Settings.

#include "PartActor.h"
#include "MechatronicsVR/Public/SnapPointComponent.h"

#include "AssemblyComponent.h"

// Sets default values for this component's properties
USnapPointComponent::USnapPointComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	//create snap detection sphere
	SnapDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SnapDetectionSphere"));
	SnapDetectionSphere->SetupAttachment(this);
	SnapDetectionSphere->SetSphereRadius(SnapDetectionRadius);
	SnapDetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SnapDetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	SnapDetectionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap); // For other parts

	
	
	// Bind overlap events
	SnapDetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &USnapPointComponent::OnSnapDetectionBeginOverlap);
	SnapDetectionSphere->OnComponentEndOverlap.AddDynamic(this, &USnapPointComponent::OnSnapDetectionEndOverlap);
	// ...
}
bool USnapPointComponent::CanAcceptSnapID(FName OtherSnapID) const
{
	return CompatibleSnapIDs.Contains(OtherSnapID);
}

// Keep this for convenience, but use SnapID internally
bool USnapPointComponent::CanAcceptPoint(const USnapPointComponent* OtherSnapPoint) const
{
	if (!OtherSnapPoint) return false;
	return CanAcceptSnapID(OtherSnapPoint->SnapID);
}

void USnapPointComponent::OnSnapDetectionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// check if the overlapping component is another snap point
	USnapPointComponent* OtherSnapPoint = Cast<USnapPointComponent>(OtherComp -> GetAttachParent());
	if (!OtherSnapPoint)
	{
		// Check if the component's owner has a SnapPointComponent
		if (OtherComp->GetOwner())
		{
			OtherSnapPoint = OtherComp->GetOwner()->FindComponentByClass<USnapPointComponent>();
		}
	}
	if (!OtherSnapPoint)
	{
		return;
	}

	// Don't detect ourselves
	if (OtherSnapPoint == this)
	{
		return;
	}
	
	// After
	AActor* MyOwner = GetOwner();
	UAssemblyComponent* MyAssembly = MyOwner ? MyOwner->FindComponentByClass<UAssemblyComponent>() : nullptr;
	const APartActor* MyPart = Cast<APartActor>(MyOwner);

	AActor* OtherOwner = OtherSnapPoint ? OtherSnapPoint->GetOwner() : nullptr;
	UAssemblyComponent* OtherAssembly = OtherOwner ? OtherOwner->FindComponentByClass<UAssemblyComponent>() : nullptr;
	if (const APartActor* OtherPart = Cast<APartActor>(OtherOwner); MyPart && OtherPart && MyPart == OtherPart)
	{
		return;
	}

	// Check compatibility
	if (!CanAcceptPoint(OtherSnapPoint))
	{
		return;
	}

	// Add to nearby list if not already there
	if (!NearbySnapPoints.Contains(OtherSnapPoint))
	{
		NearbySnapPoints.Add(OtherSnapPoint);
        
		UE_LOG(LogTemp, Log, TEXT("SnapPoint %s: Detected compatible snap point %s"), 
			   *GetName(), *OtherSnapPoint->GetName());
	}
}

void USnapPointComponent::OnSnapDetectionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	USnapPointComponent* OtherSnapPoint = Cast<USnapPointComponent>(OtherComp->GetAttachParent());
	if (!OtherSnapPoint)
	{
		// Check if the component's owner has a SnapPointComponent
		if (OtherComp->GetOwner())
		{
			OtherSnapPoint = OtherComp->GetOwner()->FindComponentByClass<USnapPointComponent>();
		}
	}

	if (!OtherSnapPoint)
	{
		return;
	}
	// Remove from nearby list
	if (NearbySnapPoints.Contains(OtherSnapPoint))
	{
		NearbySnapPoints.Remove(OtherSnapPoint);
        
		UE_LOG(LogTemp, Log, TEXT("SnapPoint %s: Lost detection of snap point %s"), 
			   *GetName(), *OtherSnapPoint->GetName());
	}

}

USnapPointComponent* USnapPointComponent::GetClosestCompatibleSnapPoint() const
{
	if (NearbySnapPoints.Num() == 0)
	{
		return nullptr; // No nearby snap points
	}

	USnapPointComponent* ClosestSnapPoint = nullptr;
	float ClosestDistance = FLT_MAX;
	FVector MyLocation = GetComponentLocation();

	for (USnapPointComponent* NearbySnapPoint: NearbySnapPoints)
	{
		if (!NearbySnapPoint || !IsValid(NearbySnapPoint))
		{
			continue; // Skip invalid snap points
		}

		// Double-check compatibility
		if (!CanAcceptPoint(NearbySnapPoint))
		{
			continue;
		}
        
		// Check if still not assembled
		if (bIsAssembled || NearbySnapPoint->bIsAssembled)
		{
			continue;
		}

		//Calculate distance
		float Distance = FVector::Dist(MyLocation, NearbySnapPoint->GetComponentLocation());
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestSnapPoint = NearbySnapPoint;
		}
	}
	return ClosestSnapPoint;
}

void USnapPointComponent::CleanupNearbySnapPoints()
{
	// Remove any invalid snap points from the list
	NearbySnapPoints.RemoveAll([](const TObjectPtr<USnapPointComponent>& SnapPoint)
	{
		return !IsValid(SnapPoint) || SnapPoint->bIsAssembled;
	});
}


bool USnapPointComponent::CanAcceptPart(const APartActor* Part) const
{
	if (!Part) return false;
	
	// Check if the part's class is in our compatible parts list
	for (const TSubclassOf<APartActor>& CompatiblePartClass : CompatibleParts)
	{
		if (Part->IsA(CompatiblePartClass))
		{
			return true;
		}
	}
	return false;
}

// Called when the game starts
void USnapPointComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USnapPointComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

