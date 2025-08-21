// Fill out your copyright notice in the Description page of Project Settings.


#include "MechatronicsVR/Public/PartActor.h"

#include "AssemblyActor.h"
#include "AssemblyComponent.h"
#include "MotionControllerComponent.h"
#include "SnapValidatorComponent.h"

#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

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

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PreviewMaterialFinder(TEXT("/Game/M_GhostPreview.M_GhostPreview"));
	if (PreviewMaterialFinder.Succeeded())
	{
		PreviewMaterial = PreviewMaterialFinder.Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load preview material!"));
	}
	
    
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

USnapPointComponent* APartActor::GetBestSnapPointFor(USnapPointComponent* TargetSnapPoint) const
{
	if (!TargetSnapPoint)
	{
		return nullptr;
	}

	TArray<USnapPointComponent*> SnapPoints = Assembly->GetSnapPoints();
	USnapPointComponent* BestSnapPoint = nullptr;
	float BestDistance = FLT_MAX;

	for (USnapPointComponent* SnapPoint : SnapPoints)
	{
		if (!SnapPoint || SnapPoint->bIsAssembled)
		{
			continue;
		}

		// Check if this snap point can accept the target snap point
		if (!SnapPoint->CanAcceptPoint(TargetSnapPoint))
		{
			continue;
		}
		// Check if target can accept this snap point
		if (!TargetSnapPoint->CanAcceptPoint(SnapPoint))
		{
			continue;
		}

		//find the closest compatible snap point
		float Distance = FVector::Dist(SnapPoint->GetComponentLocation(), TargetSnapPoint->GetComponentLocation());
		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			BestSnapPoint = SnapPoint;
		}
	}
	return BestSnapPoint;
}
void APartActor::UpdatePreviewState()
{
	if (!IsAttachedToMotionController())
	{
		HideSnapPreview();
		CurrentPreviewTarget = nullptr;
		return;
	}
	CurrentPreviewTarget = FindBestPreviewTarget();
}

USnapPointComponent* APartActor::FindBestPreviewTarget() const
{
    // Get all my snap points
    TArray<USnapPointComponent*> MySnapPoints = GetSnapPoints();
    
    // FIRST: Check if we have a specific actor we should assemble onto
    if (PartAssembledOnto && IsValid(PartAssembledOnto))
    {
        TArray<USnapPointComponent*> TargetSnapPoints = PartAssembledOnto->GetSnapPoints();
        
        for (USnapPointComponent* TargetSnapPoint : TargetSnapPoints)
        {
            if (!TargetSnapPoint || TargetSnapPoint->bIsAssembled)
            {
                continue;
            }
            
            // Check each of my snap points for compatibility
            for (USnapPointComponent* OtherSnapPoint : MySnapPoints)
            {
                if (!OtherSnapPoint || OtherSnapPoint->bIsAssembled)
                {
                    continue;
                }
                
                // Check bidirectional compatibility
                if (OtherSnapPoint->CanAcceptPoint(TargetSnapPoint) && 
                    TargetSnapPoint->CanAcceptPoint(OtherSnapPoint))
                {
                    // Found a compatible one - just return it!
                    UE_LOG(LogTemp, Log, TEXT("%s: Found target on specified actor %s"), 
                        *GetName(), *PartAssembledOnto->GetName());
                    return TargetSnapPoint;
                }
            }
        }
    }
    
    // SECOND: Try the assembly base
    TArray<AActor*> FoundAssemblies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAssemblyActor::StaticClass(), FoundAssemblies);
    
    for (AActor* Actor : FoundAssemblies)
    {
        AAssemblyActor* MyAssembly = Cast<AAssemblyActor>(Actor);
        if (!MyAssembly || !MyAssembly->bIsAssemblyActive)
        {
            continue;
        }
        
        // Check assembly's base snap points
        TArray<USnapPointComponent*> BaseSnapPoints = MyAssembly->GetBaseSnapPoints();
        for (USnapPointComponent* BaseSnapPoint : BaseSnapPoints)
        {
            if (!BaseSnapPoint || BaseSnapPoint->bIsAssembled)
            {
                continue;
            }
            
            // Check each of my snap points for compatibility with base
            for (USnapPointComponent* OtherSnapPoint : MySnapPoints)
            {
                if (!OtherSnapPoint || OtherSnapPoint->bIsAssembled)
                {
                    continue;
                }
                
                if (OtherSnapPoint->CanAcceptPoint(BaseSnapPoint) && 
                    BaseSnapPoint->CanAcceptPoint(OtherSnapPoint))
                {
                    // Found compatible base - return it!
                    UE_LOG(LogTemp, Log, TEXT("%s: Found base snap point %s"), 
                        *GetName(), *BaseSnapPoint->GetName());
                    return BaseSnapPoint;
                }
            }
        }
    }
    
    return nullptr;  // No compatible target found
}

bool APartActor::IsAttachedToMotionController() const 
{
	if (USceneComponent* RootComp = GetRootComponent()) 
	{
		// Check if we're attached to anything
		if (const USceneComponent* AttachParent = RootComp->GetAttachParent())
		{
			// Get the owner of what we're attached to
			if (AActor* ParentActor = AttachParent->GetOwner())
			{
				// Check if it has a MotionControllerComponent
				return ParentActor->FindComponentByClass<UMotionControllerComponent>() != nullptr;
			}
		}
	}
	return false;
}

void APartActor::ShowSnapPreview()
{
	// Part figures out which snap points to use
	if (!CurrentPreviewTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowSnapPreview: No CurrentPreviewTarget set, returning early."));
		return;
		
	}
    
	// Find which of MY snap points should connect
	USnapPointComponent* MyBestSnapPoint = GetBestSnapPointFor(CurrentPreviewTarget);
    
	if (MyBestSnapPoint)	
	{
		ShowSnapPreviewInternal(MyBestSnapPoint, CurrentPreviewTarget);
	}
}

void APartActor::ShowSnapPreviewInternal(USnapPointComponent* SourceSnapPoint, USnapPointComponent* TargetSnapPoint)
{
	if (!SourceSnapPoint || !TargetSnapPoint || !Mesh || !PreviewMesh) {
		UE_LOG(LogTemp, Warning, TEXT("ShowSnapPreviewInternal: Early return - SourceSnapPoint: %p, TargetSnapPoint: %p, "), SourceSnapPoint, TargetSnapPoint);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ShowSnapPreviewInternal: Called with SourceSnapPoint: %s, TargetSnapPoint: %s"),
		SourceSnapPoint ? *SourceSnapPoint->GetName() : TEXT("nullptr"),
		TargetSnapPoint ? *TargetSnapPoint->GetName() : TEXT("nullptr"));

	if (Mesh->GetStaticMesh())
	{
		UE_LOG(LogTemp, Log, TEXT("ShowSnapPreviewInternal: Setting PreviewMesh static me+sh to %s"), *Mesh->GetStaticMesh()->GetName());
		PreviewMesh->SetStaticMesh(Mesh->GetStaticMesh());
		
		// DETACH the preview mesh so it doesn't move with the part!
		PreviewMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

		const FTransform SnapTransform = CalculateSnapTransform(SourceSnapPoint, TargetSnapPoint);
		UE_LOG(LogTemp, Log, TEXT("ShowSnapPreviewInternal: Setting PreviewMesh world transform. Location: %s, Rotation: %s"),
			*SnapTransform.GetLocation().ToString(), *SnapTransform.GetRotation().Rotator().ToString());
		PreviewMesh->SetWorldTransform(SnapTransform);

		if (PreviewMaterial)
		{
			UE_LOG(LogTemp, Log, TEXT("ShowSnapPreviewInternal: Using PreviewMaterial: %s"), *PreviewMaterial->GetName());
			UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(PreviewMaterial, this);
			if (DynamicMaterial)
			{
				UE_LOG(LogTemp, Log, TEXT("ShowSnapPreviewInternal: Created dynamic material instance for preview."));
				DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), PreviewOpacity);
				DynamicMaterial->SetVectorParameterValue(TEXT("Color"), PreviewColor);
				for (int32 i = 0; i<PreviewMesh->GetNumMaterials(); i++)
				{
					PreviewMesh->SetMaterial(i, DynamicMaterial);
					UE_LOG(LogTemp, Log, TEXT("ShowSnapPreviewInternal: Set dynamic material on PreviewMesh slot %d"), i);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to create dynamic material instance for preview mesh"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("ShowSnapPreviewInternal: No PreviewMaterial, using fallback."));
			for (int32 i = 0; i < PreviewMesh->GetNumMaterials(); i++)
			{
				UMaterialInterface* OriginalMaterial = Mesh->GetMaterial(i);
				if (OriginalMaterial)
				{
					UE_LOG(LogTemp, Log, TEXT("ShowSnapPreviewInternal: Creating dynamic material from original material %s for slot %d"), *OriginalMaterial->GetName(), i);
					UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(OriginalMaterial, this);
					if (DynamicMaterial)
					{
						DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), PreviewOpacity);
						PreviewMesh->SetMaterial(i, DynamicMaterial);
						UE_LOG(LogTemp, Log, TEXT("ShowSnapPreviewInternal: Set fallback dynamic material on PreviewMesh slot %d"), i);
					}
				}
			}
		}

		PreviewMesh->SetVisibility(true);
		bShowingPreview = true;
		CurrentPreviewTarget = TargetSnapPoint;

		UE_LOG(LogTemp, Log, TEXT("ShowSnapPreview: Showing preview for %s at snap point %s to target %s"), 
		  *GetName(), 
		  *SourceSnapPoint->GetName(), 
		  *TargetSnapPoint->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowSnapPreviewInternal: Mesh has no static mesh assigned!"));
	}
}

void APartActor::HideSnapPreview()
{
	if (!PreviewMesh) return;

	if (bShowingPreview)
	{
		PreviewMesh->SetVisibility(false);
		bShowingPreview = false;
		CurrentPreviewTarget = nullptr;
		UE_LOG(LogTemp, Log, TEXT("HideSnapPreview: Hiding preview for %s"), *GetName());
	}
}

FTransform APartActor::CalculateSnapTransform(USnapPointComponent* SourceSnapPoint, USnapPointComponent* TargetSnapPoint) const
{
	if (!SourceSnapPoint || !TargetSnapPoint)
	{
		return GetActorTransform();
	}
    

    
	// Simple calculation
	FVector MySnapWorldPos = SourceSnapPoint->GetComponentLocation();
	FVector TargetSnapWorldPos = TargetSnapPoint->GetComponentLocation();
	FVector MyActorPos = GetActorLocation();
    
	// Offset from actor center to snap point
	FVector ActorToSnap = MySnapWorldPos - MyActorPos;
    
	// Where actor needs to be
	FVector NewActorPos = TargetSnapWorldPos - ActorToSnap;
    
	FTransform ResultTransform = GetActorTransform();
	ResultTransform.SetLocation(NewActorPos);
	if (AActor* TargetOwner = TargetSnapPoint->GetOwner())
	{
		// Use the rotation of what we're attaching to
		ResultTransform.SetRotation(TargetOwner->GetActorRotation().Quaternion());
	}
	else
	{
		// Fallback: just use world aligned
		ResultTransform.SetRotation(FQuat::Identity);
	}
	
    
	return ResultTransform;
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
