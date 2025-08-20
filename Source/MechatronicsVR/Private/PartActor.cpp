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
//TODO: FIX THIS MESS!!



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
	// UpdatePreviewState();
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
	// HideSnapPreview();
	// CurrentPreviewTarget = nullptr;
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

