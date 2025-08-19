// Fill out your copyright notice in the Description page of Project Settings.


#include "MechatronicsVR/Public/PartActor.h"
#include "AssemblyComponent.h"
#include "SnapValidatorComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
APartActor::APartActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Set root as mesh so physics can drive movement
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetSimulatePhysics(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);


	// Create preview mesh component
	PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(RootComponent);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMesh->SetVisibility(false);
	PreviewMesh->SetCastShadow(false);
    
	// Configure for VR Template grabbing
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore); // Don't block player movement
    
	// Set mass for realistic feel (adjust per part)
	Mesh->SetMassOverrideInKg(NAME_None, 1.0f, true);
    
	// Attach assembly component
	Assembly = CreateDefaultSubobject<UAssemblyComponent>(TEXT("Assembly"));
	Assembly->SetupAttachment(RootComponent);

	// Attach validator
	SnapValidator = CreateDefaultSubobject<USnapValidatorComponent>(TEXT("SnapValidator"));

	// Set default preview values
	PreviewOpacity = 0.3f;
	PreviewColor = FLinearColor::Green;
	bShowingPreview = false;
    

}

void APartActor::UpdatePreviewState()
{
	if (!GrabComonent || !GrabCompnent->IsGrabbed())
	{
		HideSnapPreview();
		CurrentPreviewTarget = nullptr;
		return;
	}
    
	// Find the best snap point to preview with
	USnapPointComponent* BestTarget = FindBestPreviewTarget();
    
	if (BestTarget && BestTarget != CurrentPreviewTarget)
	{
		// Show preview with new target
		USnapPointComponent* MySnapPoint = GetBestSnapPointFor(BestTarget);
		if (MySnapPoint)
		{
			ShowSnapPreview(MySnapPoint, BestTarget);
			CurrentPreviewTarget = BestTarget;
		}
	}
	else if (!BestTarget && CurrentPreviewTarget)
	{
		// No good target anymore, hide preview
		HideSnapPreview();
		CurrentPreviewTarget = nullptr;
	}
}
USnapPointComponent* APartActor::GetBestSnapPointFor(USnapPointComponent* TargetSnapPoint) const

{
	
}

USnapPointComponent* APartActor::FindBestPreviewTarget() const
{
	USnapPointComponent* BestTarget = nullptr;
	float BestDistance = FLT_MAX;

	// check all of my snap points for nearby compatible snap points
	/* TODO: , this has to check not nearby ones but the ones the part is meant to attatch to, each part has to know which part it is supposed to attach to*/

	TArray<USnapPointComponent*> MySnapPoints = GetSnapPoints();

	for (USnapPointComponent* MySnapPoint : MySnapPoints)
	{
		if (!MySnapPoint || MySnapPoint->bIsAssembled)
		{
			continue; // Skip if null or already assembled
		}

		// check each nearby snap point
		for (USnapPointComponent* NearbySnapPoint : MySnapPoint->NearbySnapPoints)
		{
			if (!NearbySnapPoint || NearbySnapPoint->bIsAssembled)
			{
				continue;
			}
            
			// Check compatibility
			if (!MySnapPoint->CanAcceptPoint(NearbySnapPoint))
			{
				continue;
			}
            
			// TODO: Check lesson system constraints (when implemented)
			// if (!MySnapPoint->IsPartAllowedInCurrentStep(this))
			// {
			// 	continue;
			// }
			// FInd closest compatible snap point
			float Distance = FVector::Dist(MySnapPoint->GetComponentLocation(), NearbySnapPoint->GetComponentLocation());
			if (Distance < BestDistance && Distance <= MaxSnapDistance)
			{
				BestDistance = Distance;
				BestTarget = NearbySnapPoint;
			}
		}
		
	}
	return BestTarget;
}

// const TArray<USnapPointComponent*>& APartActor::GetSnapPoints()
// {
// 	return Assembly->GetSnapPoints();
// }

void APartActor::ShowSnapPreview(USnapPointComponent* MySnapPoint, USnapPointComponent* TargetSnapPoint)
{
	if (!MySnapPoint || !TargetSnapPoint)
	{
		return;
	}

	// copy the mesh to the preview
	if (Mesh && Mesh->GetStaticMesh())
	{
		PreviewMesh->SetStaticMesh(Mesh->GetStaticMesh());

		//calculate where the preview should be positioned
		const FTransform SnapTransform = CalculateSnapTransform(MySnapPoint, TargetSnapPoint);
		PreviewMesh->SetWorldTransform(SnapTransform);

		// create dynamic material for ghost effect
		if (PreviewMaterial)
		{
			UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(PreviewMaterial, this);
			if (DynamicMaterial)
			{
				DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), PreviewOpacity);
				DynamicMaterial->SetVectorParameterValue(TEXT("Color"), PreviewColor);
				PreviewMesh->SetMaterial(0, DynamicMaterial);
			}
		}
	}
}

void APartActor::OnPartGrabbed()
{
	UE_LOG(LogTemp, Warning, TEXT("🔥 GRABBED: %s"), *GetName());
    
	// Print to screen for easy debugging
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
			FString::Printf(TEXT("GRABBED: %s"), *GetName()));
	}
    
	// Update preview state
	UpdatePreviewState();
}

void APartActor::OnPartReleased()
{
	UE_LOG(LogTemp, Warning, TEXT("🚀 RELEASED: %s"), *GetName());
    
	// Print to screen for easy debugging
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
			FString::Printf(TEXT("RELEASED: %s"), *GetName()));
	}
    
	// Hide preview
	HideSnapPreview();
	CurrentPreviewTarget = nullptr;
}

const TArray<USnapPointComponent*> APartActor::GetSnapPoints() const
{
	return Assembly->GetSnapPoints();
}


void APartActor::ClearSnapHighlight(APartActor* OtherPart)
{
    if (SnapCandidate == OtherPart)
    {
        SnapCandidate = nullptr;
        MySnapPoint = nullptr;
        CandidateSnapPoint = nullptr;
        
        UE_LOG(LogTemp, Log, TEXT("PartActor %s: Cleared snap highlight for %s"), 
               *GetName(), *OtherPart->GetName());
        
        // TODO: Remove visual feedback
    }
}

bool APartActor::FindBestSnapPointPair(APartActor* OtherPart, USnapPointComponent*& OutMySnapPoint, 
                                      USnapPointComponent*& OutOtherSnapPoint, float& OutDistance)
{
    if (!OtherPart)
    {
        return false;
    }
    
    TArray<USnapPointComponent*> MySnapPoints = GetSnapPoints();
    TArray<USnapPointComponent*> OtherSnapPoints = OtherPart->GetSnapPoints();
    
    float BestDistance = MaxSnapDistance + 1.0f; // Start with invalid distance
    USnapPointComponent* BestMySnapPoint = nullptr;
    USnapPointComponent* BestOtherSnapPoint = nullptr;
    
    // Check all combinations of snap points
    for (USnapPointComponent* MySnap : MySnapPoints)
    {
        if (!MySnap || MySnap->bIsAssembled)
        {
            continue; // Skip if null or already assembled
        }
        
        for (USnapPointComponent* OtherSnap : OtherSnapPoints)
        {
            if (!OtherSnap || OtherSnap->bIsAssembled)
            {
                continue; // Skip if null or already assembled
            }
            
            // Check compatibility
            if (!MySnap->CanAcceptPoint(OtherSnap) || !OtherSnap->CanAcceptPoint(MySnap))
            {
                continue; // Not compatible
            }
            
            // Check distance
            float Distance = FVector::Dist(MySnap->GetComponentLocation(), OtherSnap->GetComponentLocation());
            if (Distance <= MaxSnapDistance && Distance < BestDistance)
            {
                BestDistance = Distance;
                BestMySnapPoint = MySnap;
                BestOtherSnapPoint = OtherSnap;
            }
        }
    }
    
    // Return results
    if (BestMySnapPoint && BestOtherSnapPoint)
    {
        OutMySnapPoint = BestMySnapPoint;
        OutOtherSnapPoint = BestOtherSnapPoint;
        OutDistance = BestDistance;
        return true;
    }
    
    return false;
}

// Called when the game starts or when spawned
void APartActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APartActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

